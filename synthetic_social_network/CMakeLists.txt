cmake_minimum_required(VERSION 3.5)
project(auto_microservices)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_FLAGS "-O3")
# set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -masm=intel -lpthread")

set(CMAKE_INSTALL_PREFIX /usr/local/bin)
set(Boost_USE_STATIC_LIBS ON)

include("cmake/Findthrift.cmake")
include("cmake/FindLibevent.cmake")

find_package(nlohmann_json 3.5.0 REQUIRED)
find_package(Threads)
find_package(OpenSSL REQUIRED)
find_package(Boost 1.54.0 REQUIRED COMPONENTS log log_setup)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})
endif()

set(THRIFT_GEN_CPP_DIR ../../gen-cpp)
set(TCP_SERVER_CPP_DIR ../tcp_server)

add_subdirectory(src)