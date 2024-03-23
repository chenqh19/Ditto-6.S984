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
#include "../../gen-cpp/service_1.h"
#include "../../gen-cpp/service_10.h"
#include "../../gen-cpp/service_11.h"
#include "../../gen-cpp/service_13.h"
#include "../../gen-cpp/service_16.h"
#include "../../gen-cpp/service_7.h"
#include "../../gen-cpp/service_9.h"

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
  ClientPool<ThriftClient<service_1Client>> *_service_1_client_pool;
  ClientPool<ThriftClient<service_7Client>> *_service_7_client_pool;
  ClientPool<ThriftClient<service_9Client>> *_service_9_client_pool;
  ClientPool<ThriftClient<service_10Client>> *_service_10_client_pool;
  ClientPool<ThriftClient<service_11Client>> *_service_11_client_pool;
  ClientPool<ThriftClient<service_13Client>> *_service_13_client_pool;
  ClientPool<ThriftClient<service_16Client>> *_service_16_client_pool;
  uint64_t *_mem_data;
  uint64_t *_curr_mem_addrs;
  uint64_t *_curr_pointer_chasing_mem_addrs;
  uint64_t *_pointer_chasing_mem_data;

public:
  explicit service_0Handler(
      ClientPool<ThriftClient<service_1Client>> *service_1_client_pool,
      ClientPool<ThriftClient<service_7Client>> *service_7_client_pool,
      ClientPool<ThriftClient<service_9Client>> *service_9_client_pool,
      ClientPool<ThriftClient<service_10Client>> *service_10_client_pool,
      ClientPool<ThriftClient<service_11Client>> *service_11_client_pool,
      ClientPool<ThriftClient<service_13Client>> *service_13_client_pool,
      ClientPool<ThriftClient<service_16Client>> *service_16_client_pool,
      uint64_t *pointer_chasing_mem_data) {
    _service_1_client_pool = service_1_client_pool;
    _service_7_client_pool = service_7_client_pool;
    _service_9_client_pool = service_9_client_pool;
    _service_10_client_pool = service_10_client_pool;
    _service_11_client_pool = service_11_client_pool;
    _service_13_client_pool = service_13_client_pool;
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

    vector<std::shared_future<void>> fuWaitVec[7];
    std::shared_future<void> fuVec[7];

    fuVec[0] = std::async(std::launch::async, [&]() {
      if (!fuWaitVec[0].empty()) {
        for (auto &i : fuWaitVec[0]) {
          i.wait();
        }
      }
      std::map<std::string, std::string> writer_text_map_10;
      TextMapWriter writer_10(writer_text_map_10);

      auto self_span_10 = opentracing::Tracer::Global()->StartSpan(
          "rpc_10_client", {opentracing::ChildOf(&(span->context()))});
      opentracing::Tracer::Global()->Inject(self_span_10->context(), writer_10);
      auto service_10_client_wrapper_10 = _service_10_client_pool->Pop();
      if (!service_10_client_wrapper_10) {
        LOG(error) << "ERROR: Failed to connect to service_10";
        ServiceException se;
        se.errorCode = ErrorCode::SE_THRIFT_CONN_ERROR;
        se.message = "Failed to connect to service_10";
        throw se;
      } else {
        auto service_10_client = service_10_client_wrapper_10->GetClient();
        try {
          service_10_client->rpc_10(writer_text_map_10);
          _service_10_client_pool->Keepalive(service_10_client_wrapper_10);
        } catch (...) {
          LOG(error) << "ERROR: Failed to send rpc.";
          _service_10_client_pool->Remove(service_10_client_wrapper_10);
        }
      }
      self_span_10->Finish();

    });

    fuVec[1] = std::async(std::launch::async, [&]() {
      if (!fuWaitVec[1].empty()) {
        for (auto &i : fuWaitVec[1]) {
          i.wait();
        }
      }
      std::map<std::string, std::string> writer_text_map_9;
      TextMapWriter writer_9(writer_text_map_9);

      auto self_span_9 = opentracing::Tracer::Global()->StartSpan(
          "rpc_9_client", {opentracing::ChildOf(&(span->context()))});
      opentracing::Tracer::Global()->Inject(self_span_9->context(), writer_9);
      auto service_9_client_wrapper_9 = _service_9_client_pool->Pop();
      if (!service_9_client_wrapper_9) {
        LOG(error) << "ERROR: Failed to connect to service_9";
        ServiceException se;
        se.errorCode = ErrorCode::SE_THRIFT_CONN_ERROR;
        se.message = "Failed to connect to service_9";
        throw se;
      } else {
        auto service_9_client = service_9_client_wrapper_9->GetClient();
        try {
          service_9_client->rpc_9(writer_text_map_9);
          _service_9_client_pool->Keepalive(service_9_client_wrapper_9);
        } catch (...) {
          LOG(error) << "ERROR: Failed to send rpc.";
          _service_9_client_pool->Remove(service_9_client_wrapper_9);
        }
      }
      self_span_9->Finish();

    });

    fuVec[2] = std::async(std::launch::async, [&]() {
      if (!fuWaitVec[2].empty()) {
        for (auto &i : fuWaitVec[2]) {
          i.wait();
        }
      }
      std::map<std::string, std::string> writer_text_map_7;
      TextMapWriter writer_7(writer_text_map_7);

      auto self_span_7 = opentracing::Tracer::Global()->StartSpan(
          "rpc_7_client", {opentracing::ChildOf(&(span->context()))});
      opentracing::Tracer::Global()->Inject(self_span_7->context(), writer_7);
      auto service_7_client_wrapper_7 = _service_7_client_pool->Pop();
      if (!service_7_client_wrapper_7) {
        LOG(error) << "ERROR: Failed to connect to service_7";
        ServiceException se;
        se.errorCode = ErrorCode::SE_THRIFT_CONN_ERROR;
        se.message = "Failed to connect to service_7";
        throw se;
      } else {
        auto service_7_client = service_7_client_wrapper_7->GetClient();
        try {
          service_7_client->rpc_7(writer_text_map_7);
          _service_7_client_pool->Keepalive(service_7_client_wrapper_7);
        } catch (...) {
          LOG(error) << "ERROR: Failed to send rpc.";
          _service_7_client_pool->Remove(service_7_client_wrapper_7);
        }
      }
      self_span_7->Finish();

    });

    fuVec[3] = std::async(std::launch::async, [&]() {
      if (!fuWaitVec[3].empty()) {
        for (auto &i : fuWaitVec[3]) {
          i.wait();
        }
      }
      std::map<std::string, std::string> writer_text_map_1;
      TextMapWriter writer_1(writer_text_map_1);

      auto self_span_1 = opentracing::Tracer::Global()->StartSpan(
          "rpc_1_client", {opentracing::ChildOf(&(span->context()))});
      opentracing::Tracer::Global()->Inject(self_span_1->context(), writer_1);
      auto service_1_client_wrapper_1 = _service_1_client_pool->Pop();
      if (!service_1_client_wrapper_1) {
        LOG(error) << "ERROR: Failed to connect to service_1";
        ServiceException se;
        se.errorCode = ErrorCode::SE_THRIFT_CONN_ERROR;
        se.message = "Failed to connect to service_1";
        throw se;
      } else {
        auto service_1_client = service_1_client_wrapper_1->GetClient();
        try {
          service_1_client->rpc_1(writer_text_map_1);
          _service_1_client_pool->Keepalive(service_1_client_wrapper_1);
        } catch (...) {
          LOG(error) << "ERROR: Failed to send rpc.";
          _service_1_client_pool->Remove(service_1_client_wrapper_1);
        }
      }
      self_span_1->Finish();

    });

    fuWaitVec[4].push_back(fuVec[3]);
    fuWaitVec[4].push_back(fuVec[2]);
    fuWaitVec[4].push_back(fuVec[1]);
    fuWaitVec[4].push_back(fuVec[0]);
    fuVec[4] = std::async(std::launch::async, [&]() {
      if (!fuWaitVec[4].empty()) {
        for (auto &i : fuWaitVec[4]) {
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

    fuWaitVec[5].push_back(fuVec[3]);
    fuWaitVec[5].push_back(fuVec[2]);
    fuWaitVec[5].push_back(fuVec[1]);
    fuWaitVec[5].push_back(fuVec[0]);
    fuVec[5] = std::async(std::launch::async, [&]() {
      if (!fuWaitVec[5].empty()) {
        for (auto &i : fuWaitVec[5]) {
          i.wait();
        }
      }
      std::map<std::string, std::string> writer_text_map_13;
      TextMapWriter writer_13(writer_text_map_13);

      auto self_span_13 = opentracing::Tracer::Global()->StartSpan(
          "rpc_13_client", {opentracing::ChildOf(&(span->context()))});
      opentracing::Tracer::Global()->Inject(self_span_13->context(), writer_13);
      auto service_13_client_wrapper_13 = _service_13_client_pool->Pop();
      if (!service_13_client_wrapper_13) {
        LOG(error) << "ERROR: Failed to connect to service_13";
        ServiceException se;
        se.errorCode = ErrorCode::SE_THRIFT_CONN_ERROR;
        se.message = "Failed to connect to service_13";
        throw se;
      } else {
        auto service_13_client = service_13_client_wrapper_13->GetClient();
        try {
          service_13_client->rpc_13(writer_text_map_13);
          _service_13_client_pool->Keepalive(service_13_client_wrapper_13);
        } catch (...) {
          LOG(error) << "ERROR: Failed to send rpc.";
          _service_13_client_pool->Remove(service_13_client_wrapper_13);
        }
      }
      self_span_13->Finish();

    });

    fuWaitVec[6].push_back(fuVec[3]);
    fuWaitVec[6].push_back(fuVec[2]);
    fuWaitVec[6].push_back(fuVec[1]);
    fuWaitVec[6].push_back(fuVec[0]);
    fuVec[6] = std::async(std::launch::async, [&]() {
      if (!fuWaitVec[6].empty()) {
        for (auto &i : fuWaitVec[6]) {
          i.wait();
        }
      }
      std::map<std::string, std::string> writer_text_map_11;
      TextMapWriter writer_11(writer_text_map_11);

      auto self_span_11 = opentracing::Tracer::Global()->StartSpan(
          "rpc_11_client", {opentracing::ChildOf(&(span->context()))});
      opentracing::Tracer::Global()->Inject(self_span_11->context(), writer_11);
      auto service_11_client_wrapper_11 = _service_11_client_pool->Pop();
      if (!service_11_client_wrapper_11) {
        LOG(error) << "ERROR: Failed to connect to service_11";
        ServiceException se;
        se.errorCode = ErrorCode::SE_THRIFT_CONN_ERROR;
        se.message = "Failed to connect to service_11";
        throw se;
      } else {
        auto service_11_client = service_11_client_wrapper_11->GetClient();
        try {
          service_11_client->rpc_11(writer_text_map_11);
          _service_11_client_pool->Keepalive(service_11_client_wrapper_11);
        } catch (...) {
          LOG(error) << "ERROR: Failed to send rpc.";
          _service_11_client_pool->Remove(service_11_client_wrapper_11);
        }
      }
      self_span_11->Finish();

    });

    for (auto &i : fuVec) {
      i.wait();
    }
    span->Finish();
  }
};

class service_0CloneFactory : public service_0IfFactory {
private:
  ClientPool<ThriftClient<service_1Client>> *_service_1_client_pool;
  ClientPool<ThriftClient<service_7Client>> *_service_7_client_pool;
  ClientPool<ThriftClient<service_9Client>> *_service_9_client_pool;
  ClientPool<ThriftClient<service_10Client>> *_service_10_client_pool;
  ClientPool<ThriftClient<service_11Client>> *_service_11_client_pool;
  ClientPool<ThriftClient<service_13Client>> *_service_13_client_pool;
  ClientPool<ThriftClient<service_16Client>> *_service_16_client_pool;
  uint64_t *_pointer_chasing_mem_data;

public:
  explicit service_0CloneFactory(
      ClientPool<ThriftClient<service_1Client>> *service_1_client_pool,
      ClientPool<ThriftClient<service_7Client>> *service_7_client_pool,
      ClientPool<ThriftClient<service_9Client>> *service_9_client_pool,
      ClientPool<ThriftClient<service_10Client>> *service_10_client_pool,
      ClientPool<ThriftClient<service_11Client>> *service_11_client_pool,
      ClientPool<ThriftClient<service_13Client>> *service_13_client_pool,
      ClientPool<ThriftClient<service_16Client>> *service_16_client_pool,
      uint64_t *pointer_chasing_mem_data) {
    _service_1_client_pool = service_1_client_pool;
    _service_7_client_pool = service_7_client_pool;
    _service_9_client_pool = service_9_client_pool;
    _service_10_client_pool = service_10_client_pool;
    _service_11_client_pool = service_11_client_pool;
    _service_13_client_pool = service_13_client_pool;
    _service_16_client_pool = service_16_client_pool;
    _pointer_chasing_mem_data = pointer_chasing_mem_data;
  }

  ~service_0CloneFactory() {}

  service_0If *
  getHandler(const ::apache::thrift::TConnectionInfo &connInfo) override {
    return new service_0Handler(
        _service_1_client_pool, _service_7_client_pool, _service_9_client_pool,
        _service_10_client_pool, _service_11_client_pool,
        _service_13_client_pool, _service_16_client_pool,
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
  std::string service_1_addr = services_json["service_1"]["server_addr"];
  int service_1_port = services_json["service_1"]["server_port"];
  ClientPool<ThriftClient<service_1Client>> service_1_client_pool(
      "service_1", service_1_addr, service_1_port, 0, 512, 10000, 10000,
      services_json);

  std::string service_7_addr = services_json["service_7"]["server_addr"];
  int service_7_port = services_json["service_7"]["server_port"];
  ClientPool<ThriftClient<service_7Client>> service_7_client_pool(
      "service_7", service_7_addr, service_7_port, 0, 512, 10000, 10000,
      services_json);

  std::string service_9_addr = services_json["service_9"]["server_addr"];
  int service_9_port = services_json["service_9"]["server_port"];
  ClientPool<ThriftClient<service_9Client>> service_9_client_pool(
      "service_9", service_9_addr, service_9_port, 0, 512, 10000, 10000,
      services_json);

  std::string service_10_addr = services_json["service_10"]["server_addr"];
  int service_10_port = services_json["service_10"]["server_port"];
  ClientPool<ThriftClient<service_10Client>> service_10_client_pool(
      "service_10", service_10_addr, service_10_port, 0, 512, 10000, 10000,
      services_json);

  std::string service_11_addr = services_json["service_11"]["server_addr"];
  int service_11_port = services_json["service_11"]["server_port"];
  ClientPool<ThriftClient<service_11Client>> service_11_client_pool(
      "service_11", service_11_addr, service_11_port, 0, 512, 10000, 10000,
      services_json);

  std::string service_13_addr = services_json["service_13"]["server_addr"];
  int service_13_port = services_json["service_13"]["server_port"];
  ClientPool<ThriftClient<service_13Client>> service_13_client_pool(
      "service_13", service_13_addr, service_13_port, 0, 512, 10000, 10000,
      services_json);

  std::string service_16_addr = services_json["service_16"]["server_addr"];
  int service_16_port = services_json["service_16"]["server_port"];
  ClientPool<ThriftClient<service_16Client>> service_16_client_pool(
      "service_16", service_16_addr, service_16_port, 0, 512, 10000, 10000,
      services_json);

  int port = services_json["service_0"]["server_port"];
  TThreadedServer server(
      stdcxx::make_shared<service_0ProcessorFactory>(
          stdcxx::make_shared<service_0CloneFactory>(
              &service_1_client_pool, &service_7_client_pool,
              &service_9_client_pool, &service_10_client_pool,
              &service_11_client_pool, &service_13_client_pool,
              &service_16_client_pool, pointer_chasing_mem_data)),
      stdcxx::make_shared<TServerSocket>("0.0.0.0", port),
      stdcxx::make_shared<TFramedTransportFactory>(),
      stdcxx::make_shared<TBinaryProtocolFactory>());
  startServer(server);
}
