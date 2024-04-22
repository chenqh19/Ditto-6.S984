#ifndef UTILS_H
#define UTILS_H

#include <cstdint>

void runAssembly0(uint64_t* mem_data, uint64_t req_id, uint64_t* curr_mem_addrs, uint64_t* curr_pointer_chasing_mem_addrs);
void runAssembly1(uint64_t* mem_data, uint64_t req_id, uint64_t* curr_mem_addrs, uint64_t* curr_pointer_chasing_mem_addrs);
#endif
