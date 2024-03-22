
#include "../connection/tcp_conn.h"
#include "../lock/locker.h"
#include "../utils.h"

using namespace std;

class WorkerThread {
  public:
    WorkerThread(uint64_t* pointer_chasing_mem_data);
    ~WorkerThread();
  private:
    uint64_t* pointer_chasing_mem_data_;
    uint64_t* mem_data_;
    uint64_t* curr_mem_addrs_;
    uint64_t* curr_pointer_chasing_mem_addrs_;
    uint64_t request_id_;
};

WorkerThread::WorkerThread(uint64_t* pointer_chasing_mem_data) {
  uint64_t cache_init_size = 64;
  uint64_t cache_max_size = 256*1024*1024;
  request_id_ = 0;

  pointer_chasing_mem_data_ = pointer_chasing_mem_data;
  mem_data_ = new uint64_t[cache_max_size / 8];
  curr_mem_addrs_ = new uint64_t[23];
  curr_pointer_chasing_mem_addrs_ = new uint64_t[23];
  
  for (int i = 0; i < 23; i++) {
    curr_mem_addrs_[i] = (uint64_t) &mem_data_[curr_idx];
    curr_pointer_chasing_mem_addrs_[i] = (uint64_t) &pointer_chasing_mem_data_[curr_idx];
    curr_idx = (cache_init_size << i) / 8; // 8 bytes per uint64_t
  }
}

WorkerThread::~WorkerThread() {
  delete[] mem_data_;
  delete[] curr_mem_addrs_;
  delete[] curr_pointer_chasing_mem_addrs_;
}