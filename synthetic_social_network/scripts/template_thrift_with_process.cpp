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

#include "../ClientPool.h"
#include "../ThriftClient.h"
#include "../logger.h"
#include "../tracing.h"

/*[[[cog
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
  for i in children_services:
    cog.outl('#include "../../gen-cpp/' + i + '.h"')
  cog.outl('#include "../../gen-cpp/' + serviceName + '.h"')

]]]*/
//[[[end]]]

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

enum DistributionType {
  constant,
  log_normal
};

struct rpc_params {
  DistributionType distribution_type;
  double pre_time_mean;
  double pre_time_std;
  double post_time_mean;
  double post_time_std;
  double proc_time_mean;
  double proc_time_std;
};

/*[[[cog
import cog

# different methods to simulate processing time
# Bubble_sort to be improved to better map from defined time to actual time
class ProcessType(Enum):
  Bubble_sort = 1
  Busy_wait = 2
  Sleep = 3

processType = ProcessType.Busy_wait
use_pre = False
use_post = False

if processType == ProcessType.Bubble_sort:
  cog.outl('void bubble_sort(int *a, int len){')
  cog.outl('int max = len - 1;')
  cog.outl('int i, j;')
  cog.outl('for(i = 0; i < max; i++){')
  cog.outl('for(j = 0; j < max - i; j++){')
  cog.outl('if(a[j + 1] < a[j]){')
  cog.outl('int t = a[i];')
  cog.outl('a[i] = a[j];')
  cog.outl('a[j] = t; }}}}')

rpcs_receive = services_json[serviceName]['rpcs_receive']
cog.outl('class ' + serviceName + 'Handler: public ' + serviceName + 'If {')
cog.outl('private:')
cog.outl('std::default_random_engine _gen;')
cog.outl('std::lognormal_distribution<double> _dist;')
cog.outl('double _scaler;')
if len(children_services) > 0:
  for children_service in children_services:
    cog.outl('ClientPool<ThriftClient<' + children_service + 'Client>> *_' + children_service + '_client_pool;')
for i in services_json[serviceName]['rpcs_receive']:
  cog.outl('rpc_params * _rpc_' + str(i) + '_params;')
  cog.outl('std::lognormal_distribution<double> _rpc_' + str(i) + '_proc_dist;')
for i in services_json[serviceName]['rpcs_send']:
  cog.outl('rpc_params * _rpc_' + str(i) + '_params;')
  cog.outl('std::lognormal_distribution<double> _rpc_' + str(i) + '_pre_dist;')
  cog.outl('std::lognormal_distribution<double> _rpc_' + str(i) + '_post_dist;')

cog.outl('public:')
cog.outl('')
cog.outl('explicit ' + serviceName +'Handler(')
for children_service in children_services:
  cog.outl('ClientPool<ThriftClient<' + children_service + 'Client>> *' + children_service + '_client_pool,')
for i in services_json[serviceName]['rpcs_send']:
  cog.outl('rpc_params * rpc_' + str(i) + '_params,') 
for i in services_json[serviceName]['rpcs_receive']:
  cog.outl('rpc_params * rpc_' + str(i) + '_params,') 
cog.outl('double scaler) {') 
for children_service in children_services:
  cog.outl('_' + children_service + '_client_pool = ' + children_service + '_client_pool;')
cog.outl('_scaler = scaler;')
cog.outl('auto seed = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();')
cog.outl('_gen = std::default_random_engine(seed);')

for i in services_json[serviceName]['rpcs_receive']:
  cog.outl('_rpc_' + str(i) + '_params = ' + 'rpc_' + str(i) + '_params;')
  cog.outl('double rpc_' + str(i) + '_proc_time_mean = ' + '_rpc_' + str(i) + '_params->proc_time_mean;')
  cog.outl('if(rpc_' + str(i) + '_proc_time_mean != 0) {')
  cog.outl('double rpc_' + str(i) + '_proc_time_std = ' + '_rpc_' + str(i) + '_params->proc_time_std;')
  cog.outl('double rpc_' + str(i) + '_proc_m = log(' + 'rpc_' + str(i) + '_proc_time_mean / sqrt(1 + pow(rpc_' + \
  str(i) + '_proc_time_std, 2) / pow(rpc_' + str(i) + '_proc_time_mean, 2)));')
  cog.outl('double rpc_' + str(i) + '_proc_s = sqrt(log(1 + pow(rpc_' + str(i) + '_proc_time_std, 2) /  \
  pow(rpc_' + str(i) + '_proc_time_mean, 2)));')
  cog.outl('_rpc_' + str(i) + '_proc_dist = std::lognormal_distribution<double>(rpc_' + str(i) + '_proc_m, rpc_' + \
  str(i) + '_proc_s); }')
for i in services_json[serviceName]['rpcs_send']:
  cog.outl('_rpc_' + str(i) + '_params = ' + 'rpc_' + str(i) + '_params;')
  cog.outl('double rpc_' + str(i) + '_pre_time_mean = ' + '_rpc_' + str(i) + '_params->pre_time_mean;')
  cog.outl('if(rpc_' + str(i) + '_pre_time_mean != 0) {')
  cog.outl('double rpc_' + str(i) + '_pre_time_std = ' + '_rpc_' + str(i) + '_params->pre_time_std;')
  cog.outl('double rpc_' + str(i) + '_pre_m = log(' + 'rpc_' + str(i) + '_pre_time_mean / sqrt(1 + pow(rpc_' + \
  str(i) + '_pre_time_std, 2) / pow(rpc_' + str(i) + '_pre_time_mean, 2)));')
  cog.outl('double rpc_' + str(i) + '_pre_s = sqrt(log(1 + pow(rpc_' + str(i) + '_pre_time_std, 2) /  \
  pow(rpc_' + str(i) + '_pre_time_mean, 2)));')
  cog.outl('_rpc_' + str(i) + '_pre_dist = std::lognormal_distribution<double>(rpc_' + str(i) + '_pre_m, rpc_' + \
  str(i) + '_pre_s); }')
  cog.outl('double rpc_' + str(i) + '_post_time_mean = ' + '_rpc_' + str(i) + '_params->post_time_mean;')
  cog.outl('if(rpc_' + str(i) + '_post_time_mean != 0) {')
  cog.outl('double rpc_' + str(i) + '_post_time_std = ' + '_rpc_' + str(i) + '_params->post_time_std;')
  cog.outl('double rpc_' + str(i) + '_post_m = log(' + 'rpc_' + str(i) + '_post_time_mean / sqrt(1 + pow(rpc_' + \
  str(i) + '_post_time_std, 2) / pow(rpc_' + str(i) + '_post_time_mean, 2)));')
  cog.outl('double rpc_' + str(i) + '_post_s = sqrt(log(1 + pow(rpc_' + str(i) + '_post_time_std, 2) /  \
  pow(rpc_' + str(i) + '_post_time_mean, 2)));')
  cog.outl('_rpc_' + str(i) + '_post_dist = std::lognormal_distribution<double>(rpc_' + str(i) + '_post_m, rpc_' + \
  str(i) + '_post_s); }')
cog.outl('}')

cog.outl('')
cog.outl('~' + serviceName+'Handler() override = default;')
cog.outl('')

for rpc_receive in rpcs_receive:
  rpc_name = 'rpc_' + str(rpc_receive)
  cog.outl('void ' + rpc_name + '(const std::map<std::string, std::string> & carrier) override {')
  cog.outl('TextMapReader reader(carrier);')
  cog.outl('auto parent_span = opentracing::Tracer::Global()->Extract(reader);')
  cog.outl('auto span = opentracing::Tracer::Global()->StartSpan("' + rpc_name + '_server", { opentracing::ChildOf(parent_span->get()) });')
  cog.outl('')

  rpcs_send = [] # list of rpcs to send, assign a fake id for each call to handle dependency, real id equals rpc_replica_map[rpc_send]
  max_rpc = 0 # maximum value of real rpc_id, avoid overlapping when assigning fake id to replicas
  rpcs_send_multi = {} # map: rpc_send to its send times if multiple
  rpcs_send_para = {} # map: rpc_send to its intra dependicies (1 for parallel 0 for sequential) if multiple
  rpcs_send_dep = {} # map: rpc_send to its dependencies (fake ids, not real rpc_id)
  rpc_replica_map = {} # map: fake id to its real rpc_id
  
  # check whether call_graph is a chain, if yes dont use async future thread to avoid overhead
  chain = is_chain(rpcs_send, rpcs_send_dep)

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

  cog.outl('double rpc_' + str(rpc_receive) + '_proc_time;')
  cog.outl('double rpc_' + str(rpc_receive) + '_proc_time_mean = _rpc_' + str(rpc_receive) + '_params->proc_time_mean;')
  cog.outl('if(rpc_' + str(rpc_receive) + '_proc_time_mean != 0) {')
  cog.outl('DistributionType rpc_' + str(rpc_receive) + '_distribution_type = _rpc_' + str(rpc_receive) + '_params->distribution_type;')
  cog.outl('switch (rpc_' + str(rpc_receive) + '_distribution_type) {')
  cog.outl('case constant: rpc_' + str(rpc_receive) + '_proc_time = rpc_' + str(rpc_receive) + '_proc_time_mean;')
  cog.outl('break;')
  cog.outl('case log_normal: rpc_' + str(rpc_receive) + '_proc_time = _rpc_' + str(rpc_receive) + '_proc_dist(_gen);')
  cog.outl('break;')
  cog.outl('default: rpc_' + str(rpc_receive) + '_proc_time = rpc_' + str(rpc_receive) + '_proc_time_mean;')
  cog.outl('}}')
  cog.outl('else rpc_' + str(rpc_receive) + '_proc_time = rpc_' + str(rpc_receive) + '_proc_time_mean;')

  cog.outl('rpc_' + str(rpc_receive) + '_proc_time *= _scaler;')
  if processType == ProcessType.Bubble_sort:
    cog.outl('auto rpc_' + str(rpc_receive) + '_proc_t0 = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();')
    cog.outl('int rpc_' + str(rpc_receive) + '_proc_array_size = sqrt(rpc_' + str(rpc_receive) + '_proc_time / 1000) * 1000;')
    cog.outl('int rpc_' + str(rpc_receive) + '_proc[rpc_' + str(rpc_receive) + '_proc_array_size];')
    cog.outl('int rpc_' + str(rpc_receive) + '_init[rpc_' + str(rpc_receive) + '_proc_array_size];')
    cog.outl('for (int i = 0; i < rpc_' + str(rpc_receive) + '_proc_array_size; i++) {')
    cog.outl('rpc_' + str(rpc_receive) + '_init[i] = rand(); }')
    cog.outl('for (int j = 0; j < 10; j++) {')
    cog.outl('for (int i = 0; i < rpc_' + str(rpc_receive) + '_proc_array_size; i++) {')
    cog.outl('rpc_' + str(rpc_receive) + '_proc[i] = rpc_' + str(rpc_receive) + '_init[i]; }')
    cog.outl('bubble_sort(rpc_' + str(rpc_receive) + '_proc, rpc_' + str(rpc_receive) + '_proc_array_size);}')
    cog.outl('auto rpc_' + str(rpc_receive) + '_proc_t1 = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();')
  elif processType == ProcessType.Busy_wait:
    cog.outl('auto rpc_' + str(rpc_receive) + '_proc_t0 = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();')
    cog.outl('while (true) {')
    cog.outl('auto rpc_' + str(rpc_receive) + '_proc_t1 = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();')
    cog.outl('if (rpc_' + str(rpc_receive) + '_proc_t1 - rpc_' + str(rpc_receive) + '_proc_t0 >= (int) (rpc_' + str(rpc_receive) + '_proc_time))')
    cog.outl('break;')
    cog.outl('}')
  else:
    cog.outl('std::this_thread::sleep_for(std::chrono::microseconds(int(rpc_' + str(rpc_receive) + '_proc_time)));')
  
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
      # if call graph is not a chain, use future
      cog.outl('fuVec[' + str(rpcs_send_map[rpc_send]) + '] = std::async(std::launch::async, [&]() {')
    cog.outl('std::map<std::string, std::string> writer_text_map_' + str(rpc_send) + ';')
    cog.outl('TextMapWriter writer_' + str(rpc_send) + '(writer_text_map_' + str(rpc_send) + ');')

    # if call graph is not a chain, wait until all dependent rpcs have finished
    if not chain:
      cog.outl('if (!fuWaitVec[' + str(rpcs_send_map[rpc_send]) + '].empty()) {')
      cog.outl('for (auto &i : fuWaitVec[' + str(rpcs_send_map[rpc_send]) + ']) {')
      cog.outl('i.wait(); }')
      cog.outl('}')
    
    if use_pre:
      cog.outl('DistributionType ' + rpc_name + '_distribution_type = _' + rpc_name + '_params->distribution_type;')
      cog.outl('double ' + rpc_name + '_pre_time;')
      cog.outl('double ' + rpc_name + '_pre_time_mean = _' + rpc_name + '_params->pre_time_mean;')
      cog.outl('if(' + rpc_name + '_pre_time_mean != 0) {')
      cog.outl('switch (' + rpc_name + '_distribution_type) {')
      cog.outl('case constant: ' + rpc_name + '_pre_time = ' + rpc_name + '_pre_time_mean;')
      cog.outl('break;')
      cog.outl('case log_normal: ' + rpc_name + '_pre_time = _' + rpc_name + '_pre_dist(_gen);')
      cog.outl('break;')
      cog.outl('default: ' + rpc_name + '_pre_time = ' + rpc_name + '_pre_time_mean;')
      cog.outl('}}')
      cog.outl('else ' + rpc_name + '_pre_time = ' + rpc_name + '_pre_time_mean;')
      cog.outl(rpc_name + '_pre_time *= _scaler;')
      
      if processType == ProcessType.Bubble_sort:
        cog.outl('auto ' + rpc_name + '_pre_t0 = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();')
        cog.outl('int ' + rpc_name + '_pre_array_size = sqrt(' + rpc_name + '_pre_time / 1000) * 1000;')
        cog.outl('int ' + rpc_name + '_pre[' + rpc_name + '_pre_array_size];')
        cog.outl('int ' + rpc_name + '_pre_init[' + rpc_name + '_pre_array_size];')
        cog.outl('for (int i = 0; i < ' + rpc_name + '_pre_array_size; i++) {')
        cog.outl('' + rpc_name + '_pre_init[i] = rand(); }')
        cog.outl('for (int j = 0; j < 10; j++) {')
        cog.outl('for (int i = 0; i < ' + rpc_name + '_pre_array_size; i++) {')
        cog.outl(rpc_name + '_pre[i] = ' + rpc_name + '_pre_init[i]; }')
        cog.outl('bubble_sort(' + rpc_name + '_pre, ' + rpc_name + '_pre_array_size);}')
        cog.outl('auto ' + rpc_name + '_pre_t1 = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();')
      elif processType == ProcessType.Busy_wait:
        cog.outl('auto ' + rpc_name + '_pre_t0 = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();')
        cog.outl('while (true) {')
        cog.outl('auto ' + rpc_name + '_pre_t1 = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();')
        cog.outl('if (' + rpc_name + '_pre_t1 - ' + rpc_name + '_pre_t0 >= (int) (' + rpc_name + '_pre_time))')
        cog.outl('break;')
        cog.outl('}')
      else:
        cog.outl('std::this_thread::sleep_for(std::chrono::microseconds(int(' + rpc_name + '_pre_time)));')

    cog.outl('')
    cog.outl('auto self_span_' + str(rpc_send) + ' = opentracing::Tracer::Global()->StartSpan("' + rpc_name + '_client", { opentracing::ChildOf(&(span->context()))});')
    cog.outl('opentracing::Tracer::Global()->Inject(self_span_' + str(rpc_send) + '->context(), writer_' + str(rpc_send) + ');')
    
    client_wrapper = rpc_server_name + '_client_wrapper_' + str(rpc_send)
    cog.outl('auto ' + client_wrapper + ' = _' + rpc_server_name + '_client_pool->Pop();')
    cog.outl('if(!' + client_wrapper + ') {')
    cog.outl('ServiceException se;')
    cog.outl('se.errorCode = ErrorCode::SE_THRIFT_CONN_ERROR;')
    cog.outl('se.message = "Failed to connect to ' + rpc_server_name + '";')
    cog.outl('throw se;')
    cog.outl('}')
    cog.outl('else {')
    cog.outl('auto ' + rpc_server_name + '_client = ' + client_wrapper + '->GetClient();')
    cog.outl('try {')
    cog.outl(rpc_server_name + '_client->' + rpc_name + '(writer_text_map_' + str(rpc_send) + ');')
    cog.outl('} catch (TException& tx) {')
    cog.outl('cout << "ERROR: " << tx.what() << endl;')
    cog.outl('_' + rpc_server_name + '_client_pool->Push(' + client_wrapper + ');')
    cog.outl('}')
    cog.outl('_' + rpc_server_name + '_client_pool->Push(' + client_wrapper + ');')
    cog.outl('}')
    cog.outl('self_span_' + str(rpc_send) + '->Finish();')
    cog.outl('')

    if use_post:
      if not use_pre:
        cog.outl('DistributionType ' + rpc_name + '_distribution_type = _' + rpc_name + '_params->distribution_type;')
      cog.outl('double ' + rpc_name + '_post_time;')
      cog.outl('double ' + rpc_name + '_post_time_mean = _' + rpc_name + '_params->post_time_mean;')
      cog.outl('if(' + rpc_name + '_post_time_mean != 0) {')
      cog.outl('switch (' + rpc_name + '_distribution_type) {')
      cog.outl('case constant: ' + rpc_name + '_post_time = ' + rpc_name + '_post_time_mean;')
      cog.outl('break;')
      cog.outl('case log_normal: ' + rpc_name + '_post_time = _' + rpc_name + '_post_dist(_gen);')
      cog.outl('break;')
      cog.outl('default: ' + rpc_name + '_post_time = ' + rpc_name + '_post_time_mean;')
      cog.outl('}}')
      cog.outl('else ' + rpc_name + '_post_time = ' + rpc_name + '_post_time_mean;')
      cog.outl(rpc_name + '_post_time *= _scaler;')

      if processType == ProcessType.Bubble_sort:
        cog.outl('auto ' + rpc_name + '_post_t0 = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();')
        cog.outl('int ' + rpc_name + '_post_array_size = sqrt(' + rpc_name + '_post_time / 1000) * 1000;')
        cog.outl('int ' + rpc_name + '_post[' + rpc_name + '_post_array_size];')
        cog.outl('int ' + rpc_name + '_post_init[' + rpc_name + '_post_array_size];')
        cog.outl('for (int i = 0; i < ' + rpc_name + '_post_array_size; i++) {')
        cog.outl('' + rpc_name + '_post_init[i] = rand(); }')
        cog.outl('for (int j = 0; j < 10; j++) {')
        cog.outl('for (int i = 0; i < ' + rpc_name + '_post_array_size; i++) {')
        cog.outl('' + rpc_name + '_post[i] = ' + rpc_name + '_post_init[i]; }')
        cog.outl('bubble_sort(' + rpc_name + '_post, ' + rpc_name + '_post_array_size);}')
        cog.outl('auto ' + rpc_name + '_post_t1 = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();')
      elif processType == ProcessType.Busy_wait:
        cog.outl('auto ' + rpc_name + '_post_t0 = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();')
        cog.outl('while (true) {')
        cog.outl('auto ' + rpc_name + '_post_t1 = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();')
        cog.outl('if (' + rpc_name + '_post_t1 - ' + rpc_name + '_post_t0 >= (int) (' + rpc_name + '_post_time))')
        cog.outl('break;')
        cog.outl('}')
      else:
        cog.outl('std::this_thread::sleep_for(std::chrono::microseconds(int(' + rpc_name + '_post_time)));')
    
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

]]]*/
//[[[end]]]

void startServer(TServer &server) {
  /*[[[cog
      import cog
      cog.outl('cout << "Starting the '+ serviceName + ' server..." << endl;')
      ]]]*/
  //[[[end]]]
  server.serve();
  cout << "Done." << endl;
}

int main(int argc, char *argv[]) {
  json rpcs_json;
  json services_json;
  std::ifstream json_file;
  json_file.open("config/rpcs.json");
  if (json_file.is_open()) {
    json_file >> rpcs_json;
    json_file.close();
  }
  else {
    cout << "Cannot open rpcs-config.json" << endl;
    return -1;
  }
  json_file.open("config/services.json");
  if (json_file.is_open()) {
    json_file >> services_json;
    json_file.close();
  }
  else {
    cout << "Cannot open services-config.json" << endl;
    return -1;
  }

  /*[[[cog
  import cog
  cog.outl('srand((unsigned)time(NULL));')
  cog.outl('SetUpTracer("config/jaeger-config.yml", "' + serviceName + '");')
  for children_service in children_services:
    cog.outl('std::string ' + children_service + '_addr = services_json["' + children_service + '"]["server_addr"];')
    cog.outl('int ' + children_service + '_port = services_json["' + children_service + '"]["server_port"];')
    cog.outl('ClientPool<ThriftClient<' + children_service + 'Client>> ' + children_service + '_client_pool(')
    cog.outl('"' + children_service + '", ' + children_service + '_addr, ' + children_service + '_port, ' + str(0) + ', ' + str(1024) + ', ' + str(5000) + ');')
    cog.outl('')
  
  for i in services_json[serviceName]['rpcs_send']:
    cog.outl('string tmp_rpc_' + str(i) + '_distribution_type = rpcs_json["rpc_' + str(i) + '"]["distribution_type"];')
    cog.outl('DistributionType rpc_' + str(i) + '_distribution_type;')
    cog.outl('if (tmp_rpc_' + str(i) + '_distribution_type == "log_normal")')
    cog.outl('rpc_' + str(i) + '_distribution_type = log_normal;')
    cog.outl('else')
    cog.outl('rpc_' + str(i) + '_distribution_type = constant;')
    cog.outl('rpc_params rpc_' + str(i) + '_params = {')
    cog.outl('rpc_' + str(i) + '_distribution_type,')
    cog.outl('rpcs_json["rpc_' + str(i) + '"]["pre_time_mean"],')
    cog.outl('rpcs_json["rpc_' + str(i) + '"]["pre_time_std"],')
    cog.outl('rpcs_json["rpc_' + str(i) + '"]["post_time_mean"],')
    cog.outl('rpcs_json["rpc_' + str(i) + '"]["post_time_std"],')
    cog.outl('rpcs_json["rpc_' + str(i) + '"]["proc_time_mean"],')
    cog.outl('rpcs_json["rpc_' + str(i) + '"]["proc_time_std"],')
    cog.outl('};')
    cog.outl('')

  for i in services_json[serviceName]['rpcs_receive']:
    cog.outl('string tmp_rpc_' + str(i) + '_distribution_type = rpcs_json["rpc_' + str(i) + '"]["distribution_type"];')
    cog.outl('DistributionType rpc_' + str(i) + '_distribution_type;')
    cog.outl('if (tmp_rpc_' + str(i) + '_distribution_type == "log_normal")')
    cog.outl('rpc_' + str(i) + '_distribution_type = log_normal;')
    cog.outl('else')
    cog.outl('rpc_' + str(i) + '_distribution_type = constant;')
    cog.outl('rpc_params rpc_' + str(i) + '_params = {')
    cog.outl('rpc_' + str(i) + '_distribution_type,')
    cog.outl('rpcs_json["rpc_' + str(i) + '"]["pre_time_mean"],')
    cog.outl('rpcs_json["rpc_' + str(i) + '"]["pre_time_std"],')
    cog.outl('rpcs_json["rpc_' + str(i) + '"]["post_time_mean"],')
    cog.outl('rpcs_json["rpc_' + str(i) + '"]["post_time_std"],')
    cog.outl('rpcs_json["rpc_' + str(i) + '"]["proc_time_mean"],')
    cog.outl('rpcs_json["rpc_' + str(i) + '"]["proc_time_std"],')
    cog.outl('};')
    cog.outl('')

  cog.outl('double scaler = services_json["' + serviceName + '"]["scaler"];')
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
    cog.outl('stdcxx::make_shared<' + serviceName + 'Processor>(')
    cog.outl('stdcxx::make_shared<' + serviceName + 'Handler>(')
    for i in children_services:
      cog.outl('&' + i + '_client_pool,')
    for i in services_json[serviceName]['rpcs_send']:
      cog.outl('&rpc_' + str(i) + '_params,')
    for i in services_json[serviceName]['rpcs_receive']:
      cog.outl('&rpc_' + str(i) + '_params,')
    cog.outl(' scaler)),')
    cog.outl('stdcxx::make_shared<TServerSocket>(' + '"0.0.0.0", port' + '),')
    cog.outl('stdcxx::make_shared<TFramedTransportFactory>(),')
    cog.outl('stdcxx::make_shared<TBinaryProtocolFactory>());')
  elif serverType == "nonblocking":
    cog.outl('auto trans = make_shared<TNonblockingServerSocket>(port);')
    cog.outl('auto processor = make_shared<' + serviceName + 'Processor>(make_shared<' + serviceName + 'Handler>(')
    for i in children_services:
      cog.outl('&' + i + '_client_pool,')
    for i in services_json[serviceName]['rpcs_send']:
      cog.outl('&rpc_' + str(i) + '_params,')
    for i in services_json[serviceName]['rpcs_receive']:
      cog.outl('&rpc_' + str(i) + '_params,')
    cog.outl(' scaler));')
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
    cog.outl('stdcxx::make_shared<' + serviceName + 'Processor>(')
    cog.outl('stdcxx::make_shared<' + serviceName + 'Handler>(')
    for i in children_services:
      cog.outl('&' + i + '_client_pool,')
    for i in services_json[serviceName]['rpcs_send']:
      cog.outl('&rpc_' + str(i) + '_params,')
    for i in services_json[serviceName]['rpcs_receive']:
      cog.outl('&rpc_' + str(i) + '_params,')
    cog.outl(' scaler)),')
    cog.outl('stdcxx::make_shared<TServerSocket>(' + '"0.0.0.0", port' + '),')
    cog.outl('stdcxx::make_shared<TFramedTransportFactory>(),')
    cog.outl('stdcxx::make_shared<TBinaryProtocolFactory>(),')
    cog.outl('thread_manager);')
  else:
    # default SimpleServer, have bugs and don't use.
    cog.outl('TSimpleServer server(')
    cog.outl('stdcxx::make_shared<' + serviceName + 'Processor>(')
    cog.outl('stdcxx::make_shared<' + serviceName + 'Handler>(')
    for i in children_services:
      cog.outl('&' + i + '_client_pool,')
    for i in services_json[serviceName]['rpcs_send']:
      cog.outl('&rpc_' + str(i) + '_params,')
    for i in services_json[serviceName]['rpcs_receive']:
      cog.outl('&rpc_' + str(i) + '_params,')
    cog.outl(' scaler)),')
    cog.outl('stdcxx::make_shared<TServerSocket>(' + '"0.0.0.0", port' + '),')
    cog.outl('stdcxx::make_shared<TBufferedTransportFactory>(),')
    cog.outl('stdcxx::make_shared<TBinaryProtocolFactory>());')

  ]]]*/
  //[[[end]]]
  startServer(server);
}
