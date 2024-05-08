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

#include "../../gen-cpp/service_1.h"
#include "../../gen-cpp/service_2.h"
#include "../../gen-cpp/service_21.h"
#include "../../gen-cpp/service_4.h"

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
static uint64_t *pointer_chasing_mem_data = new uint64_t[256 * 1024 * 1024 / 8];

class service_1Handler : public service_1If {
private:
  ClientPool<ThriftClient<service_2Client>> *_service_2_client_pool;
  ClientPool<ThriftClient<service_21Client>> *_service_21_client_pool;
  ClientPool<ThriftClient<service_4Client>> *_service_4_client_pool;
  uint64_t *_mem_data;
  uint64_t *_curr_mem_addrs;
  uint64_t *_curr_pointer_chasing_mem_addrs;
  uint64_t *_pointer_chasing_mem_data;

public:
  explicit service_1Handler(
      ClientPool<ThriftClient<service_2Client>> *service_2_client_pool,
      ClientPool<ThriftClient<service_21Client>> *service_21_client_pool,
      ClientPool<ThriftClient<service_4Client>> *service_4_client_pool,
      uint64_t *pointer_chasing_mem_data) {
    _service_2_client_pool = service_2_client_pool;
    _service_21_client_pool = service_21_client_pool;
    _service_4_client_pool = service_4_client_pool;
    _pointer_chasing_mem_data = pointer_chasing_mem_data;
    _curr_mem_addrs = new uint64_t[23];
    _curr_pointer_chasing_mem_addrs = new uint64_t[23];
    uint64_t cache_init_size = 64;
    uint64_t cache_max_size = 256 * 1024 * 1024;
    _mem_data = new uint64_t[cache_max_size / 8];
    uint64_t curr_idx = 0;
    for (int i = 0; i < 23; i++) {
      _curr_mem_addrs[i] = (uint64_t)&_mem_data[curr_idx];
      _curr_pointer_chasing_mem_addrs[i] =
          (uint64_t)&_pointer_chasing_mem_data[curr_idx];
      curr_idx += (cache_init_size << i) / 8; // 8 bytes per uint64_t
    }
  }

  ~service_1Handler() {
    delete[] _mem_data;
    delete[] _curr_mem_addrs;
    delete[] _curr_pointer_chasing_mem_addrs;
  };

  void rpc_1(const std::map<std::string, std::string> &carrier) override {
    TextMapReader reader(carrier);
    auto parent_span = opentracing::Tracer::Global()->Extract(reader);
    auto span = opentracing::Tracer::Global()->StartSpan(
        "rpc_1_server", {opentracing::ChildOf(parent_span->get())});

    try {
      runAssembly0(_mem_data, _request_id, _curr_mem_addrs,
                   _curr_pointer_chasing_mem_addrs);
    } catch (const std::exception &ex) {
      LOG(error) << ex.what();
    }

    _request_id++;

    vector<std::shared_future<void>> fuWaitVec[2];
    std::shared_future<void> fuVec[2];

    fuVec[0] = std::async(std::launch::async, [&]() {
      if (!fuWaitVec[0].empty()) {
        for (auto &i : fuWaitVec[0]) {
          i.wait();
        }
      }
      std::map<std::string, std::string> writer_text_map_4;
      TextMapWriter writer_4(writer_text_map_4);

      auto self_span_4 = opentracing::Tracer::Global()->StartSpan(
          "rpc_4_client", {opentracing::ChildOf(&(span->context()))});
      opentracing::Tracer::Global()->Inject(self_span_4->context(), writer_4);
      auto service_4_client_wrapper_4 = _service_4_client_pool->Pop();
      if (!service_4_client_wrapper_4) {
        LOG(error) << "ERROR: Failed to connect to service_4";
        ServiceException se;
        se.errorCode = ErrorCode::SE_THRIFT_CONN_ERROR;
        se.message = "Failed to connect to service_4";
        throw se;
      } else {
        auto service_4_client = service_4_client_wrapper_4->GetClient();
        try {
          service_4_client->rpc_4(writer_text_map_4);
          _service_4_client_pool->Keepalive(service_4_client_wrapper_4);
        } catch (...) {
          LOG(error) << "ERROR: Failed to send rpc.";
          _service_4_client_pool->Remove(service_4_client_wrapper_4);
        }
      }
      self_span_4->Finish();

    });

    fuVec[1] = std::async(std::launch::async, [&]() {
      if (!fuWaitVec[1].empty()) {
        for (auto &i : fuWaitVec[1]) {
          i.wait();
        }
      }
      std::map<std::string, std::string> writer_text_map_2;
      TextMapWriter writer_2(writer_text_map_2);

      auto self_span_2 = opentracing::Tracer::Global()->StartSpan(
          "rpc_2_client", {opentracing::ChildOf(&(span->context()))});
      opentracing::Tracer::Global()->Inject(self_span_2->context(), writer_2);
      auto service_2_client_wrapper_2 = _service_2_client_pool->Pop();
      if (!service_2_client_wrapper_2) {
        LOG(error) << "ERROR: Failed to connect to service_2";
        ServiceException se;
        se.errorCode = ErrorCode::SE_THRIFT_CONN_ERROR;
        se.message = "Failed to connect to service_2";
        throw se;
      } else {
        auto service_2_client = service_2_client_wrapper_2->GetClient();
        try {
          service_2_client->rpc_2(writer_text_map_2);
          _service_2_client_pool->Keepalive(service_2_client_wrapper_2);
        } catch (...) {
          LOG(error) << "ERROR: Failed to send rpc.";
          _service_2_client_pool->Remove(service_2_client_wrapper_2);
        }
      }
      self_span_2->Finish();

      if (!fuWaitVec[1].empty()) {
        for (auto &i : fuWaitVec[1]) {
          i.wait();
        }
      }
      std::map<std::string, std::string> writer_text_map_21;
      TextMapWriter writer_21(writer_text_map_21);

      auto self_span_21 = opentracing::Tracer::Global()->StartSpan(
          "rpc_21_client", {opentracing::ChildOf(&(span->context()))});
      opentracing::Tracer::Global()->Inject(self_span_21->context(), writer_21);
      auto service_21_client_wrapper_21 = _service_21_client_pool->Pop();
      if (!service_21_client_wrapper_21) {
        LOG(error) << "ERROR: Failed to connect to service_21";
        ServiceException se;
        se.errorCode = ErrorCode::SE_THRIFT_CONN_ERROR;
        se.message = "Failed to connect to service_21";
        throw se;
      } else {
        auto service_21_client = service_21_client_wrapper_21->GetClient();
        try {
          service_21_client->rpc_21(writer_text_map_21);
          _service_21_client_pool->Keepalive(service_21_client_wrapper_21);
        } catch (...) {
          LOG(error) << "ERROR: Failed to send rpc.";
          _service_21_client_pool->Remove(service_21_client_wrapper_21);
        }
      }
      self_span_21->Finish();

    });

    for (auto &i : fuVec) {
      i.wait();
    }
    span->Finish();
  }
};

class service_1CloneFactory : public service_1IfFactory {
private:
  ClientPool<ThriftClient<service_2Client>> *_service_2_client_pool;
  ClientPool<ThriftClient<service_21Client>> *_service_21_client_pool;
  ClientPool<ThriftClient<service_4Client>> *_service_4_client_pool;
  uint64_t *_pointer_chasing_mem_data;

public:
  explicit service_1CloneFactory(
      ClientPool<ThriftClient<service_2Client>> *service_2_client_pool,
      ClientPool<ThriftClient<service_21Client>> *service_21_client_pool,
      ClientPool<ThriftClient<service_4Client>> *service_4_client_pool,
      uint64_t *pointer_chasing_mem_data) {
    _service_2_client_pool = service_2_client_pool;
    _service_21_client_pool = service_21_client_pool;
    _service_4_client_pool = service_4_client_pool;
    _pointer_chasing_mem_data = pointer_chasing_mem_data;
  }

  ~service_1CloneFactory() {}

  service_1If *
  getHandler(const ::apache::thrift::TConnectionInfo &connInfo) override {
    return new service_1Handler(_service_2_client_pool, _service_21_client_pool, _service_4_client_pool,
                                _pointer_chasing_mem_data);
  }

  void releaseHandler(service_1If *handler) override { delete handler; }
};

void startServer(TServer &server) {
  cout << "Starting the service_1 server..." << endl;
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
  uint64_t cache_max_size = 256 * 1024 * 1024;

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  uint64_t begin = 0;
  for (uint64_t i = 0; i < 23; i++) {
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
  SetUpTracer("config/jaeger-config.yml", "service_1");
  std::string service_2_addr = services_json["service_2"]["server_addr"];
  int service_2_port = services_json["service_2"]["server_port"];
  ClientPool<ThriftClient<service_2Client>> service_2_client_pool(
      "service_2", service_2_addr, service_2_port, 0, 512, 10000, 10000,
      services_json);

  std::string service_21_addr = services_json["service_21"]["server_addr"];
  int service_21_port = services_json["service_21"]["server_port"];
  ClientPool<ThriftClient<service_21Client>> service_21_client_pool(
      "service_21", service_21_addr, service_21_port, 0, 512, 10000, 10000,
      services_json);

  std::string service_4_addr = services_json["service_4"]["server_addr"];
  int service_4_port = services_json["service_4"]["server_port"];
  ClientPool<ThriftClient<service_4Client>> service_4_client_pool(
      "service_4", service_4_addr, service_4_port, 0, 512, 10000, 10000,
      services_json);

  int port = services_json["service_1"]["server_port"];
  TThreadedServer server(stdcxx::make_shared<service_1ProcessorFactory>(
                             stdcxx::make_shared<service_1CloneFactory>(
                                 &service_2_client_pool, &service_21_client_pool, &service_4_client_pool,
                                 pointer_chasing_mem_data)),
                         stdcxx::make_shared<TServerSocket>("0.0.0.0", port),
                         stdcxx::make_shared<TFramedTransportFactory>(),
                         stdcxx::make_shared<TBinaryProtocolFactory>());
  startServer(server);
}