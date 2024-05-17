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

#include <cstring> // For strerror
#include <cerrno> // For errno

#include "../ClientPool.h"
#include "../MemcachedTCPClient.h"
#include "../MongoTCPClient.h"
#include "../RedisTCPClient.h"
#include "../TCPClientPool.h"
#include "../ThriftClient.h"
#include "../logger.h"
#include "../tracing.h"
#include "utils.h"

#include "../../gen-cpp/service_0.h"
#include "../../gen-cpp/service_16.h"

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

class service_0Handler : public service_0If {
private:
  ClientPool<ThriftClient<service_16Client>> *_service_16_client_pool;
  uint64_t *_mem_data;
  uint64_t *_curr_mem_addrs;
  uint64_t *_curr_pointer_chasing_mem_addrs;
  uint64_t *_pointer_chasing_mem_data;

public:
  explicit service_0Handler(
      ClientPool<ThriftClient<service_16Client>> *service_16_client_pool,
      uint64_t *pointer_chasing_mem_data) {
    _service_16_client_pool = service_16_client_pool;
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

  ~service_0Handler() {
    delete[] _mem_data;
    delete[] _curr_mem_addrs;
    delete[] _curr_pointer_chasing_mem_addrs;
  };

  void rpc_0(const std::map<std::string, std::string> &carrier) override {
    TextMapReader reader(carrier);
    auto parent_span = opentracing::Tracer::Global()->Extract(reader);
    auto span = opentracing::Tracer::Global()->StartSpan(
        "rpc_0_server", {opentracing::ChildOf(parent_span->get())});

    try {
      runAssembly0(_mem_data, _request_id, _curr_mem_addrs,
                   _curr_pointer_chasing_mem_addrs);
    } catch (const std::exception &ex) {
      LOG(error) << ex.what();
    }

    _request_id++;

    vector<std::shared_future<void>> fuWaitVec[1];
    std::shared_future<void> fuVec[1];

    fuVec[0] = std::async(std::launch::async, [&]() {
      if (!fuWaitVec[0].empty()) {
        for (auto &i : fuWaitVec[0]) {
          i.wait();
        }
      }
      std::map<std::string, std::string> writer_text_map_16;
      TextMapWriter writer_16(writer_text_map_16);

      auto self_span_16 = opentracing::Tracer::Global()->StartSpan(
          "rpc_16_client", {opentracing::ChildOf(&(span->context()))});
      opentracing::Tracer::Global()->Inject(self_span_16->context(), writer_16);
      auto service_16_client_wrapper_16 = _service_16_client_pool->Pop();
      if (!service_16_client_wrapper_16) {
        LOG(error) << "ERROR: Failed to connect to service_16";
        ServiceException se;
        se.errorCode = ErrorCode::SE_THRIFT_CONN_ERROR;
        se.message = "Failed to connect to service_16";
        throw se;
      } else {
        auto service_16_client = service_16_client_wrapper_16->GetClient();
        try {
          service_16_client->rpc_16(writer_text_map_16);
          _service_16_client_pool->Keepalive(service_16_client_wrapper_16);
        } catch (...) {
          LOG(error) << "ERROR: Failed to send rpc.";
          _service_16_client_pool->Remove(service_16_client_wrapper_16);
        }
      }
      self_span_16->Finish();

    });


    for (auto &i : fuVec) {
      i.wait();
    }
    span->Finish();
  }
};

class service_0CloneFactory : public service_0IfFactory {
private:
  ClientPool<ThriftClient<service_16Client>> *_service_16_client_pool;
  uint64_t *_pointer_chasing_mem_data;

public:
  explicit service_0CloneFactory(
      ClientPool<ThriftClient<service_16Client>> *service_16_client_pool,
      uint64_t *pointer_chasing_mem_data) {
    _service_16_client_pool = service_16_client_pool;
    _pointer_chasing_mem_data = pointer_chasing_mem_data;
  }

  ~service_0CloneFactory() {}

  service_0If *
  getHandler(const ::apache::thrift::TConnectionInfo &connInfo) override {
    return new service_0Handler(
        _service_16_client_pool,
        _pointer_chasing_mem_data);
  }

  void releaseHandler(service_0If *handler) override { delete handler; }
};

void startServer(TServer &server) {
  cout << "Starting the service_0 server..." << endl;
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
    cout << "Cannot open services-config.json " << strerror(errno) << endl;
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
  SetUpTracer("config/jaeger-config.yml", "service_0");

  std::string service_16_addr = services_json["service_16"]["server_addr"];
  int service_16_port = services_json["service_16"]["server_port"];
  ClientPool<ThriftClient<service_16Client>> service_16_client_pool(
      "service_16", service_16_addr, service_16_port, 0, 512, 10000, 10000,
      services_json);

  int port = services_json["service_0"]["server_port"];
  TThreadedServer server(
      stdcxx::make_shared<service_0ProcessorFactory>(
          stdcxx::make_shared<service_0CloneFactory>(
              &service_16_client_pool, pointer_chasing_mem_data)),
      stdcxx::make_shared<TServerSocket>("0.0.0.0", port),
      stdcxx::make_shared<TFramedTransportFactory>(),
      stdcxx::make_shared<TBinaryProtocolFactory>());
  startServer(server);
}
