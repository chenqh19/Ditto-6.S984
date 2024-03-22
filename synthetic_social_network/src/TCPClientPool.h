#ifndef AUTO_MICROSERVICES_SRC_TCPCLIENTPOOL_H_
#define AUTO_MICROSERVICES_SRC_TCPCLIENTPOOL_H_

#include <vector>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <chrono>
#include <string>
#include "logger.h"

using namespace std;

namespace auto_microservices {

template<class TCPClient>
class TCPClientPool {
 public:
  TCPClientPool(const std::string &client_type, const std::string &addr,
      int port, int min_size, int max_size, int timeout_ms);
  ~TCPClientPool();

  TCPClientPool(const TCPClientPool&) = delete;
  TCPClientPool& operator=(const TCPClientPool&) = delete;
  TCPClientPool(TCPClientPool&&) = default;
  TCPClientPool& operator=(TCPClientPool&&) = default;

  TCPClient* Pop();
  void Push(TCPClient*);
  void Keepalive(TCPClient*);
  void Remove(TCPClient*);

 private:
  std::deque<TCPClient*> _pool;
  std::string _addr;
  std::string _client_type;
  int _port;
  int _min_pool_size{};
  int _max_pool_size{};
  int _curr_pool_size{};
  int _timeout_ms;
  std::mutex _mtx;
  std::condition_variable _cv;
};

template<class TCPClient>
TCPClientPool<TCPClient>::TCPClientPool(const std::string &client_type,
    const std::string &addr, int port, int min_pool_size, int max_pool_size, int timeout_ms) {
  _addr = addr;
  _port = port;
  _min_pool_size = min_pool_size;
  _max_pool_size = max_pool_size;
  _timeout_ms = timeout_ms;
  _client_type = client_type;

  for (int i = 0; i < min_pool_size; ++i) {
    TCPClient* client = new TCPClient(addr, port);
    _pool.emplace_back(client);
  }
  _curr_pool_size = min_pool_size;
}

template<class TCPClient>
TCPClientPool<TCPClient>::~TCPClientPool() {
  while (!_pool.empty()) {
    delete _pool.front();
    _pool.pop_front();
  }
}

template<class TCPClient>
TCPClient* TCPClientPool<TCPClient>::Pop() {
  TCPClient* client = nullptr;
  {
    std::unique_lock<std::mutex> cv_lock(_mtx);
    while (_pool.size() == 0 && _curr_pool_size == _max_pool_size) {
      // Create a new a client if current pool size is less than
      // the max pool size.
      auto wait_time = std::chrono::system_clock::now() +
          std::chrono::milliseconds(_timeout_ms);
      bool wait_success = _cv.wait_until(cv_lock, wait_time,
            [this] { return _pool.size() > 0 || _curr_pool_size < _max_pool_size; });
      if (!wait_success) {
        LOG(warning) << "ClientPool pop timeout";
        LOG(info) << _pool.size() << " " << _curr_pool_size;
        cv_lock.unlock();
        return nullptr;
      }
    }
    if (_pool.size() > 0) {
      client = _pool.front();
      _pool.pop_front();
    } else {
      client = new TCPClient(_addr, _port);
      _curr_pool_size++;
    }
  cv_lock.unlock();
  }

  return client;
}

template<class TCPClient>
void TCPClientPool<TCPClient>::Push(TCPClient* client) {
  std::unique_lock<std::mutex> cv_lock(_mtx);
  _pool.push_back(client);
  cv_lock.unlock();
  _cv.notify_one();
}

template<class TCPClient>
void TCPClientPool<TCPClient>::Remove(TCPClient *client) {
  // No need to delete it from _pool because the *client has been poped out
  delete client;
  std::unique_lock<std::mutex> cv_lock(_mtx);
  _curr_pool_size--;
  cv_lock.unlock();
  _cv.notify_one();
}

} // namespace auto_microservices

#endif //AUTO_MICROSERVICES_SRC_TCPCLIENTPOOL_H_
