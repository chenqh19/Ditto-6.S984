#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PlatformThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TNonblockingServerSocket.h>
#include <thrift/TToString.h>
#include <thrift/stdcxx.h>
#include <yaml-cpp/yaml.h>
#include <jaegertracing/Tracer.h>
#include <opentracing/propagation.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <getopt.h>
#include <thread>
#include <future>
#include <execinfo.h>
#include <string>
#include <random>
#include <time.h>
#include <cstdint>

#include "../ClientPool.h"
#include "../ThriftClient.h"
#include "../logger.h"
#include "../tracing.h"
#include "../TCPClientPool.h"
#include "../MongoTCPClient.h"
#include "../RedisTCPClient.h"
#include "../MemcachedTCPClient.h"
#include "utils.h"

[[[cog
import cog
import json
import math
from enum import Enum

def reorder_list(rpc_send, rpcs_send_list, rpcs_send_dep):
  if rpc_send in rpcs_send_list:
    return rpcs_send_list.index(rpc_send)
  if len(rpcs_send_dep[rpc_send]) == 0:
    rpcs_send_list.insert(0, rpc_send)
    return 0
  else:
    dep_index_max = 0
    for j in rpcs_send_dep[rpc_send]:
      dep_index = reorder_list(j, rpcs_send_list, rpcs_send_dep)
    for j in rpcs_send_dep[rpc_send]:
      if rpcs_send_list.index(j) > dep_index_max:
        dep_index_max = rpcs_send_list.index(j)
    rpcs_send_list.insert(dep_index_max + 1, rpc_send)
    return dep_index_max + 1

def is_chain(rpcs_send, rpcs_send_dep):
  dep_cnt = {}
  for rpc_send in rpcs_send:
    dep_cnt[rpc_send] = 0
  for rpc_send in rpcs_send:
    if len(rpcs_send_dep[rpc_send]) > 1:
      return False
    for rpc in rpcs_send_dep[rpc_send]:
      dep_cnt[rpc] += 1
  head_cnt = 0
  for rpc_send in rpcs_send:
    if dep_cnt[rpc_send] == 1:
      head_cnt += 1
  if head_cnt != len(rpcs_send) - 1:
    return False
  return True

with open(config + "rpcs.json",'r') as load_rpcs_f:
  rpcs_json = json.load(load_rpcs_f)

with open(config + "paths.json",'r') as load_paths_f:
  paths_json = json.load(load_paths_f)
  edges = paths_json[pathName]["edges"]
  dependency = paths_json[pathName]["dependency"]

with open(config + "services.json",'r') as load_services_f:
  services_json = json.load(load_services_f)
  children_services = []
  for i in services_json[serviceName]['rpcs_send']:
    children_services.append(rpcs_json["rpc_" + str(i)]["server"])
  for service in children_services:
    if services_json[service]["server_type"] == "threaded":
      cog.outl('#include "../../gen-cpp/' + service + '.h"')
  cog.outl('#include "../../gen-cpp/' + serviceName + '.h"')

]]]
[[[end]]]

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using json = nlohmann::json;
using std::chrono::microseconds;
using std::chrono::duration_cast;
using std::chrono::system_clock;

using namespace auto_microservices;

static uint64_t _request_id = 0;
static uint64_t* pointer_chasing_mem_data = new uint64_t[4*1024*1024 / 8];

[[[cog
import cog

rpcs_receive = services_json[serviceName]['rpcs_receive']
cog.outl('class ' + serviceName + 'Handler: public ' + serviceName + 'If {')
cog.outl('private:')
if len(children_services) > 0:
  for children_service in children_services:
    if services_json[children_service]["server_type"] == "threaded": 
      cog.outl('ClientPool<ThriftClient<' + children_service + 'Client>> *_' + children_service + '_client_pool;')
    else:
      if services_json[children_service]["server_type"] == "mongod":
        cog.outl('TCPClientPool<MongoTCPClient> *_' + children_service + '_client_pool;')
      elif services_json[children_service]["server_type"] == "redis":
        cog.outl('TCPClientPool<RedisTCPClient> *_' + children_service + '_client_pool;')
      elif services_json[children_service]["server_type"] == "memcached":
        cog.outl('TCPClientPool<MemcachedTCPClient> *_' + children_service + '_client_pool;')
      else:
        print("Unsupported server type")
cog.outl('uint64_t* _mem_data;')
cog.outl('uint64_t* _curr_mem_addrs;')
cog.outl('uint64_t* _curr_pointer_chasing_mem_addrs;')
cog.outl('uint64_t* _pointer_chasing_mem_data;')

cog.outl('public:')
cog.outl('')
cog.outl('explicit ' + serviceName +'Handler(')
for children_service in children_services:
  if services_json[children_service]["server_type"] == "threaded":
    cog.outl('ClientPool<ThriftClient<' + children_service + 'Client>> *' + children_service + '_client_pool,')
  else:
    if services_json[children_service]["server_type"] == "mongod":
      cog.outl('TCPClientPool<MongoTCPClient> *' + children_service + '_client_pool,')
    elif services_json[children_service]["server_type"] == "redis":
      cog.outl('TCPClientPool<RedisTCPClient> *' + children_service + '_client_pool,')
    elif services_json[children_service]["server_type"] == "memcached":
      cog.outl('TCPClientPool<MemcachedTCPClient> *' + children_service + '_client_pool,')
    else:
      print("Unsupported server type")
cog.outl('uint64_t* pointer_chasing_mem_data')
cog.outl(') {') 
for children_service in children_services:
  cog.outl('_' + children_service + '_client_pool = ' + children_service + '_client_pool;')
cog.outl('_pointer_chasing_mem_data = pointer_chasing_mem_data;')
cog.outl('_curr_mem_addrs = new uint64_t[17];')
cog.outl('_curr_pointer_chasing_mem_addrs = new uint64_t[17];')
cog.outl('uint64_t cache_init_size = 64;')
cog.outl('uint64_t cache_max_size = 4*1024*1024;')
cog.outl('_mem_data = new uint64_t[cache_max_size / 8];')
cog.outl('uint64_t curr_idx = 0;')
cog.outl('for (int i = 0; i < 17; i++) {')
cog.outl('  _curr_mem_addrs[i] = (uint64_t) &_mem_data[curr_idx];')
cog.outl('  _curr_pointer_chasing_mem_addrs[i] = (uint64_t) &_pointer_chasing_mem_data[curr_idx];')
cog.outl('  curr_idx = (cache_init_size << i) / 8; // 8 bytes per uint64_t')
cog.outl('}')
cog.outl('}')

cog.outl('')
cog.outl('~' + serviceName+'Handler() {')
cog.outl('delete [] _mem_data;')
cog.outl('delete [] _curr_mem_addrs;')
cog.outl('delete [] _curr_pointer_chasing_mem_addrs;')
cog.outl('};')
cog.outl('')

for rpc_receive in rpcs_receive:
  rpc_name = 'rpc_' + str(rpc_receive)
  cog.outl('void ' + rpc_name + '(const std::map<std::string, std::string> & carrier) override {')
  cog.outl('TextMapReader reader(carrier);')
  cog.outl('auto parent_span = opentracing::Tracer::Global()->Extract(reader);')
  cog.outl('auto span = opentracing::Tracer::Global()->StartSpan("' + rpc_name + '_server", { opentracing::ChildOf(parent_span->get()) });')
  cog.outl('')

  cog.outl('try {')
  cog.outl('runAssembly0(_mem_data, _request_id, _curr_mem_addrs, _curr_pointer_chasing_mem_addrs);}')
  cog.outl('catch (const std::exception &ex) {')
  cog.outl('LOG(error) << ex.what();')
  cog.outl('}')
  cog.outl('')
  cog.outl('_request_id++;')
  cog.outl('')

  rpcs_send = [] # list of rpcs to send, assign a fake id for each call to handle dependency, real id equals rpc_replica_map[rpc_send]
  max_rpc = 0 # maximum value of real rpc_id, avoid overlapping when assigning fake id to replicas
  rpcs_send_multi = {} # map: rpc_send to its send times if multiple
  rpcs_send_para = {} # map: rpc_send to its intra dependicies (1 for parallel 0 for sequential) if multiple
  rpcs_send_dep = {} # map: rpc_send to its dependencies (fake ids, not real rpc_id)
  rpc_replica_map = {} # map: fake id to its real rpc_id

  for edge in edges:
    if edge[0] == rpc_receive:
      rpcs_send.append(edge[1])
      if len(edge) > 2 :
        rpcs_send_multi[edge[1]] = edge[2]
        # dependency between multiple call of this rpc, 1 for parallel and 0 for sequential
        rpcs_send_para[edge[1]] = edge[3]
  for rpc_send in rpcs_send:
    if rpc_send > max_rpc:
      max_rpc = rpc_send
    rpcs_send_dep[rpc_send] = []
    for dep in dependency:
      if dep[0] == rpc_send:
        rpcs_send_dep[rpc_send].append(dep[1])

  rpcs_ini_num = len(rpcs_send)
  rpcs_current_num = max_rpc + 1

  for i in range(rpcs_ini_num):
    rpc_replica_map[rpcs_send[i]] = rpcs_send[i]
    if rpcs_send[i] in rpcs_send_multi:
      for j in range(rpcs_send_multi[rpcs_send[i]]-1):
        rpc_replica_map[rpcs_current_num] = rpcs_send[i]
        rpcs_send.append(rpcs_current_num)
        rpcs_send_dep[rpcs_current_num] = []
        # parallel: add each replica to dependency of downstream rpcs, add upstream rpcs to dependency of each replica 
        if rpcs_send_para[rpcs_send[i]] == 1:
          chain = False
          if rpcs_send_dep[rpcs_send[i]]:
            for k in rpcs_send_dep[rpcs_send[i]]:
              rpcs_send_dep[rpcs_current_num].append(k)
          for kk in rpcs_send:
            if rpcs_send[i] in rpcs_send_dep[kk]:
              rpcs_send_dep[kk].append(rpcs_current_num)
        # sequential: replace the original one to last replica in dependency of downstream rpcs,
        # add each replica to dependency of its following one
        else:
          if j == 0:
            rpcs_send_dep[rpcs_current_num].append(rpcs_send[i])
          else:
            rpcs_send_dep[rpcs_current_num].append(rpcs_current_num - 1)
          if j == rpcs_send_multi[rpcs_send[i]] - 2:
            for k in rpcs_send:
              if rpcs_send[i] in rpcs_send_dep[k]:
              # replace dependency for rpcs that have/haven't been traversed, notice that don't replce within its own replicas
                if (k not in rpc_replica_map) or rpc_replica_map[k] != rpcs_send[i]:
                  rpcs_send_dep[k].remove(rpcs_send[i])
                  rpcs_send_dep[k].append(rpcs_current_num)
        rpcs_current_num += 1

  print(rpcs_send_dep)

  # check whether call_graph is a chain, if yes dont use async future thread to avoid overhead
  chain = is_chain(rpcs_send, rpcs_send_dep)

  rpcs_send_list = []
  # reorder rpc_send list based on intra child_rpcs dependency, since this list will correspond to the future vector one-to-one
  # to achieve the intra dependency
  for rpc_send in rpcs_send:
    reorder_list(rpc_send, rpcs_send_list, rpcs_send_dep)
  print(rpcs_send_list)

  rpcs_send_map = {} # map: fake id to its index in rpcs_send_list
  for rpc_send in rpcs_send_list:
    rpcs_send_map[rpc_send] = rpcs_send_list.index(rpc_send)
  print(rpcs_send_map)

  rpcs_send_num = len(rpcs_send_list)
  
  if rpcs_send_num <= 0:
    # if no child rpcs, directly simulate processing and return
    cog.outl('span->Finish();')
    cog.outl('}')
    continue

  if not chain:
    cog.outl('vector<std::shared_future<void>> fuWaitVec[' + str(rpcs_send_num) +'];')
    cog.outl('std::shared_future<void> fuVec[' + str(rpcs_send_num) +'];')
  cog.outl('')

  for rpc_send in rpcs_send_list:
    rpc_name = "rpc_" + str(rpc_replica_map[rpc_send])
    rpc_server_name = rpcs_json[rpc_name]['server']
    # if call graph is not a chain, push dependent rpcs in each rpc's wait queue
    if not chain:
      for j in rpcs_send_dep[rpc_send]:
        cog.outl('fuWaitVec[' + str(rpcs_send_map[rpc_send]) + '].push_back(fuVec[' + str(rpcs_send_map[j]) + ']);')

    if not chain:
      # if call graph is not a chain, use future and wait until all dependent rpcs have finished.
      cog.outl('fuVec[' + str(rpcs_send_map[rpc_send]) + '] = std::async(std::launch::async, [&]() {')
      cog.outl('if (!fuWaitVec[' + str(rpcs_send_map[rpc_send]) + '].empty()) {')
      cog.outl('for (auto &i : fuWaitVec[' + str(rpcs_send_map[rpc_send]) + ']) {')
      cog.outl('i.wait(); }')
      cog.outl('}')

    cog.outl('std::map<std::string, std::string> writer_text_map_' + str(rpc_send) + ';')
    cog.outl('TextMapWriter writer_' + str(rpc_send) + '(writer_text_map_' + str(rpc_send) + ');')

    cog.outl('')
    cog.outl('auto self_span_' + str(rpc_send) + ' = opentracing::Tracer::Global()->StartSpan("' + rpc_name + '_client", { opentracing::ChildOf(&(span->context()))});')
    cog.outl('opentracing::Tracer::Global()->Inject(self_span_' + str(rpc_send) + '->context(), writer_' + str(rpc_send) + ');')
    
    client_wrapper = rpc_server_name + '_client_wrapper_' + str(rpc_send)
    cog.outl('auto ' + client_wrapper + ' = _' + rpc_server_name + '_client_pool->Pop();')
    cog.outl('if(!' + client_wrapper + ') {')
    cog.outl('LOG(error) << "ERROR: Failed to connect to ' + rpc_server_name + '";')
    cog.outl('ServiceException se;')
    cog.outl('se.errorCode = ErrorCode::SE_THRIFT_CONN_ERROR;')
    cog.outl('se.message = "Failed to connect to ' + rpc_server_name + '";')
    cog.outl('throw se;')
    cog.outl('}')
    cog.outl('else {')
    if services_json[rpcs_json[rpc_name]["server"]]["server_type"] == "threaded":
      cog.outl('auto ' + rpc_server_name + '_client = ' + client_wrapper + '->GetClient();')
      cog.outl('try {')
      cog.outl(rpc_server_name + '_client->' + rpc_name + '(writer_text_map_' + str(rpc_send) + ');')
      cog.outl('_' + rpc_server_name + '_client_pool->Keepalive(' + client_wrapper + ');')
      cog.outl('} catch (...) {')
      cog.outl('LOG(error) << "ERROR: Failed to send rpc.";')
      cog.outl('_' + rpc_server_name + '_client_pool->Remove(' + client_wrapper + ');')
      cog.outl('}')
    else:
      cog.outl('try {')
      cog.outl(rpc_server_name + '_client_wrapper_' + str(rpc_send) + '->Send("Helloworld\\n");')
      cog.outl('_' + rpc_server_name + '_client_pool->Push(' + client_wrapper + ');')
      cog.outl('} catch (...) {')
      cog.outl('LOG(error) << "ERROR: Failed to send rpc.";')
      cog.outl('_' + rpc_server_name + '_client_pool->Remove(' + client_wrapper + ');')
      cog.outl('}')
    cog.outl('}')
    cog.outl('self_span_' + str(rpc_send) + '->Finish();')
    cog.outl('')

    if not chain:
      cog.outl('});')
    cog.outl('')

  if not chain:
    cog.outl('for (auto &i : fuVec) {')
    cog.outl('i.wait();')
    cog.outl('}')
  
  cog.outl('span->Finish();')
  cog.outl('}')

cog.outl('};')

]]]
[[[end]]]

[[[cog
import cog

cog.outl('class ' + serviceName + 'CloneFactory: public ' + serviceName + 'IfFactory {')
cog.outl('private:')
if len(children_services) > 0:
  for children_service in children_services:
    if services_json[children_service]["server_type"] == "threaded": 
      cog.outl('ClientPool<ThriftClient<' + children_service + 'Client>> *_' + children_service + '_client_pool;')
    else:
      if services_json[children_service]["server_type"] == "mongod":
        cog.outl('TCPClientPool<MongoTCPClient> *_' + children_service + '_client_pool;')
      elif services_json[children_service]["server_type"] == "redis":
        cog.outl('TCPClientPool<RedisTCPClient> *_' + children_service + '_client_pool;')
      elif services_json[children_service]["server_type"] == "memcached":
        cog.outl('TCPClientPool<MemcachedTCPClient> *_' + children_service + '_client_pool;')
      else:
        print("Unsupported server type")
cog.outl('uint64_t* _pointer_chasing_mem_data;')

cog.outl('public:')
cog.outl('')
cog.outl('explicit ' + serviceName +'CloneFactory(')
for children_service in children_services:
  if services_json[children_service]["server_type"] == "threaded":
    cog.outl('ClientPool<ThriftClient<' + children_service + 'Client>> *' + children_service + '_client_pool,')
  else:
    if services_json[children_service]["server_type"] == "mongod":
      cog.outl('TCPClientPool<MongoTCPClient> *' + children_service + '_client_pool,')
    elif services_json[children_service]["server_type"] == "redis":
      cog.outl('TCPClientPool<RedisTCPClient> *' + children_service + '_client_pool,')
    elif services_json[children_service]["server_type"] == "memcached":
      cog.outl('TCPClientPool<MemcachedTCPClient> *' + children_service + '_client_pool,')
    else:
      print("Unsupported server type")
cog.outl('uint64_t* pointer_chasing_mem_data')
cog.outl(') {') 
for children_service in children_services:
  cog.outl('_' + children_service + '_client_pool = ' + children_service + '_client_pool;')
cog.outl('_pointer_chasing_mem_data = pointer_chasing_mem_data;')
cog.outl('}')

cog.outl('')
cog.outl('~' + serviceName+'CloneFactory() {')
cog.outl('}')

cog.outl('')
cog.outl(serviceName + 'If* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) override {')
cog.outl('return new ' + serviceName + 'Handler(')
for children_service in children_services:
  cog.outl('_' + children_service + '_client_pool,')
cog.outl('_pointer_chasing_mem_data')
cog.outl(');')
cog.outl('}')

cog.outl('')
cog.outl('void releaseHandler(' + serviceName + 'If* handler) override {')
cog.outl('delete handler;')
cog.outl('}')

cog.outl('};')

    ]]]
[[[end]]]

void startServer(TServer &server) {
  [[[cog
      import cog
      cog.outl('cout << "Starting the '+ serviceName + ' server..." << endl;')
      ]]]
  [[[end]]]
  server.serve();
  cout << "Done." << endl;
}

int main(int argc, char *argv[]) {
  json rpcs_json;
  json services_json;
  std::ifstream json_file;
  json_file.open("config/services.json");
  if (json_file.is_open()) {
    json_file >> services_json;
    json_file.close();
  }
  else {
    cout << "Cannot open services-config.json" << endl;
    return -1;
  }

  uint64_t cache_init_size = 64;
  uint64_t cache_max_size = 4*1024*1024;

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  uint64_t begin = 0;
  for (uint64_t i = 0; i < 17; i++) {
      uint64_t end = (cache_init_size << i) / 8;
      std::vector<int> x(end - begin);
      std::iota(std::begin(x), std::end(x), begin); 
      shuffle (x.begin(), x.end(), std::default_random_engine(seed));
      for (uint64_t j = begin; j < end; j++) {
          pointer_chasing_mem_data[j] = (uint64_t) &pointer_chasing_mem_data[x[j - begin]];
      }
      begin = end;
  }

  [[[cog
  import cog
  cog.outl('srand((unsigned)time(NULL));')
  cog.outl('SetUpTracer("config/jaeger-config.yml", "' + serviceName + '");')
  for children_service in children_services:
    cog.outl('std::string ' + children_service + '_addr = services_json["' + children_service + '"]["server_addr"];')
    cog.outl('int ' + children_service + '_port = services_json["' + children_service + '"]["server_port"];')
    if services_json[children_service]["server_type"] == "threaded":
      cog.outl('ClientPool<ThriftClient<' + children_service + 'Client>> ' + children_service + '_client_pool(')
      cog.outl('"' + children_service + '", ' + children_service + '_addr, ' + children_service + '_port, ' + str(0) + ', ' + str(512) + ', ' + str(10000) + ', ' + str(10000) + ',services_json);')
    else:
      if services_json[children_service]["server_type"] == "mongod":
        cog.outl('TCPClientPool<MongoTCPClient> ' + children_service + '_client_pool(')
      elif services_json[children_service]["server_type"] == "redis":
        cog.outl('TCPClientPool<RedisTCPClient> ' + children_service + '_client_pool(')
      elif services_json[children_service]["server_type"] == "memcached":
        cog.outl('TCPClientPool<MemcachedTCPClient> ' + children_service + '_client_pool(')
      else:
        print("Unsupported server type")
      cog.outl('"' + children_service + '", ' + children_service + '_addr, ' + children_service + '_port, ' + str(0) + ', ' + str(512) + ', ' + str(10000) + ');')
    cog.outl('')
  
  cog.outl('int port = services_json["' + serviceName + '"]["server_port"];')

  serverType = services_json[serviceName]["server_type"]
  if serverType == "threadpool" or serverType == "nonblocking":
    poolSize = services_json[serviceName]["pool_size"]

  # class ServerType(Enum):
  #   TThreaded = 1
  #   TThreadPool = 2
  #   TNonblocking = 3

  # serverType = ServerType.TThreaded
  # if serverType != ServerType.TThreaded:
  #   cog.outl('int poolSize = 50;')

  if serverType == "threaded":
    cog.outl('TThreadedServer server (')
    cog.outl('stdcxx::make_shared<' + serviceName + 'ProcessorFactory>(')
    cog.outl('stdcxx::make_shared<' + serviceName + 'CloneFactory>(')
    for children_service in children_services:
      cog.outl('&' + children_service + '_client_pool,')
    cog.outl('pointer_chasing_mem_data')
    cog.outl(')),')
    cog.outl('stdcxx::make_shared<TServerSocket>(' + '"0.0.0.0", port' + '),')
    cog.outl('stdcxx::make_shared<TFramedTransportFactory>(),')
    cog.outl('stdcxx::make_shared<TBinaryProtocolFactory>());')
  elif serverType == "nonblocking":
    cog.outl('auto trans = make_shared<TNonblockingServerSocket>(port);')
    cog.outl('auto processor = make_shared<' + serviceName + 'ProcessorFactory>(make_shared<' + serviceName + 'CloneFactory>(')
    for children_service in children_services:
      cog.outl('&' + children_service + '_client_pool,')
    cog.outl('pointer_chasing_mem_data')
    cog.outl('));')
    cog.outl('auto protocol = make_shared<TBinaryProtocolFactory>();')
    cog.outl('auto thread_manager = ThreadManager::newSimpleThreadManager(' + str(poolSize) + ');')
    cog.outl('thread_manager->threadFactory(make_shared<PosixThreadFactory>());')
    cog.outl('thread_manager->start();')
    cog.outl('TNonblockingServer server(processor, protocol, trans, thread_manager);')
  elif serverType == "threadpool":
    cog.outl('auto thread_manager = ThreadManager::newSimpleThreadManager(' + str(poolSize) + ');')
    cog.outl('thread_manager->threadFactory(make_shared<PosixThreadFactory>());')
    cog.outl('thread_manager->start();')
    cog.outl('TThreadPoolServer server (')
    cog.outl('stdcxx::make_shared<' + serviceName + 'ProcessorFactory>(')
    cog.outl('stdcxx::make_shared<' + serviceName + 'CloneFactory>(')
    for children_service in children_services:
      cog.outl('&' + children_service + '_client_pool,')
    cog.outl('pointer_chasing_mem_data')
    cog.outl(')),')
    cog.outl('stdcxx::make_shared<TServerSocket>(' + '"0.0.0.0", port' + '),')
    cog.outl('stdcxx::make_shared<TFramedTransportFactory>(),')
    cog.outl('stdcxx::make_shared<TBinaryProtocolFactory>(),')
    cog.outl('thread_manager);')
  else:
    # default SimpleServer, have bugs and don't use.
    cog.outl('TSimpleServer server(')
    cog.outl('stdcxx::make_shared<' + serviceName + 'ProcessorFactory>(')
    cog.outl('stdcxx::make_shared<' + serviceName + 'CloneFactory>(')
    for children_service in children_services:
      cog.outl('&' + children_service + '_client_pool,')
    cog.outl('pointer_chasing_mem_data')
    cog.outl(')),')
    cog.outl('stdcxx::make_shared<TServerSocket>(' + '"0.0.0.0", port' + '),')
    cog.outl('stdcxx::make_shared<TBufferedTransportFactory>(),')
    cog.outl('stdcxx::make_shared<TBinaryProtocolFactory>());')

  ]]]
  //[[[end]]]
  startServer(server);
}