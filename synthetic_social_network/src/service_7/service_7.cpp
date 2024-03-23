#include <cstdint>
#include <execinfo.h>
#include <fstream>
#include <future>
#include <getopt.h>
#include <iostream>
#include <jaegertracing/Tracer.h>
#include <nlohmann/json.hpp>
#include <opentracing/propagation.h>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <thrift/TToString.h>
#include <thrift/concurrency/PlatformThreadFactory.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/server/TServer.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/stdcxx.h>
#include <thrift/transport/TNonblockingServerSocket.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <time.h>
#include <yaml-cpp/yaml.h>

#include "../ClientPool.h"
#include "../MemcachedTCPClient.h"
#include "../MongoTCPClient.h"
#include "../RedisTCPClient.h"
#include "../TCPClientPool.h"
#include "../ThriftClient.h"
#include "../logger.h"
#include "../tracing.h"
#include "utils.h"

#include "../../gen-cpp/service_7.h"

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
static uint64_t *pointer_chasing_mem_data = new uint64_t[4 * 1024 * 1024 / 8];

class service_7Handler : public service_7If {
private:
  TCPClientPool<MongoTCPClient> *_service_8_client_pool;
  uint64_t *_mem_data;
  uint64_t *_curr_mem_addrs;
  uint64_t *_curr_pointer_chasing_mem_addrs;
  uint64_t *_pointer_chasing_mem_data;

public:
  explicit service_7Handler(
      TCPClientPool<MongoTCPClient> *service_8_client_pool,
      uint64_t *pointer_chasing_mem_data) {
    _service_8_client_pool = service_8_client_pool;
    _pointer_chasing_mem_data = pointer_chasing_mem_data;
    _curr_mem_addrs = new uint64_t[17];
    _curr_pointer_chasing_mem_addrs = new uint64_t[17];
    uint64_t cache_init_size = 64;
    uint64_t cache_max_size = 4 * 1024 * 1024;
    _mem_data = new uint64_t[cache_max_size / 8];
    uint64_t curr_idx = 0;
    for (int i = 0; i < 17; i++) {
      _curr_mem_addrs[i] = (uint64_t)&_mem_data[curr_idx];
      _curr_pointer_chasing_mem_addrs[i] =
          (uint64_t)&_pointer_chasing_mem_data[curr_idx];
      curr_idx = (cache_init_size << i) / 8; // 8 bytes per uint64_t
    }
  }

  ~service_7Handler() {
    delete[] _mem_data;
    delete[] _curr_mem_addrs;
    delete[] _curr_pointer_chasing_mem_addrs;
  };

  void rpc_7(const std::map<std::string, std::string> &carrier) override {
    TextMapReader reader(carrier);
    auto parent_span = opentracing::Tracer::Global()->Extract(reader);
    auto span = opentracing::Tracer::Global()->StartSpan(
        "rpc_7_server", {opentracing::ChildOf(parent_span->get())});

    try {
      runAssembly0(_mem_data, _request_id, _curr_mem_addrs,
                   _curr_pointer_chasing_mem_addrs);
    } catch (const std::exception &ex) {
      LOG(error) << ex.what();
    }

    _request_id++;

    span->Finish();
  }
};

class service_7CloneFactory : public service_7IfFactory {
private:
  TCPClientPool<MongoTCPClient> *_service_8_client_pool;
  uint64_t *_pointer_chasing_mem_data;

public:
  explicit service_7CloneFactory(
      TCPClientPool<MongoTCPClient> *service_8_client_pool,
      uint64_t *pointer_chasing_mem_data) {
    _service_8_client_pool = service_8_client_pool;
    _pointer_chasing_mem_data = pointer_chasing_mem_data;
  }

  ~service_7CloneFactory() {}

  service_7If *
  getHandler(const ::apache::thrift::TConnectionInfo &connInfo) override {
    return new service_7Handler(_service_8_client_pool,
                                _pointer_chasing_mem_data);
  }

  void releaseHandler(service_7If *handler) override { delete handler; }
};

void startServer(TServer &server) {
  cout << "Starting the service_7 server..." << endl;
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
  } else {
    cout << "Cannot open services-config.json" << endl;
    return -1;
  }

  uint64_t cache_init_size = 64;
  uint64_t cache_max_size = 4 * 1024 * 1024;

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  uint64_t begin = 0;
  for (uint64_t i = 0; i < 17; i++) {
    uint64_t end = (cache_init_size << i) / 8;
    std::vector<int> x(end - begin);
    std::iota(std::begin(x), std::end(x), begin);
    shuffle(x.begin(), x.end(), std::default_random_engine(seed));
    for (uint64_t j = begin; j < end; j++) {
      pointer_chasing_mem_data[j] =
          (uint64_t)&pointer_chasing_mem_data[x[j - begin]];
    }
    begin = end;
  }

  srand((unsigned)time(NULL));
  SetUpTracer("config/jaeger-config.yml", "service_7");
  std::string service_8_addr = services_json["service_8"]["server_addr"];
  int service_8_port = services_json["service_8"]["server_port"];
  TCPClientPool<MongoTCPClient> service_8_client_pool(
      "service_8", service_8_addr, service_8_port, 0, 512, 10000);

  int port = services_json["service_7"]["server_port"];
  TThreadedServer server(
      stdcxx::make_shared<service_7ProcessorFactory>(
          stdcxx::make_shared<service_7CloneFactory>(&service_8_client_pool,
                                                     pointer_chasing_mem_data)),
      stdcxx::make_shared<TServerSocket>("0.0.0.0", port),
      stdcxx::make_shared<TFramedTransportFactory>(),
      stdcxx::make_shared<TBinaryProtocolFactory>());
  startServer(server);
}