import os
import json
import sys

def main(argv):
  path_id = argv[0]
  config = argv[1]
  with open(config + 'paths.json') as load_path_file:
    path_json = json.load(load_path_file)
    path = path_json["path_" + str(path_id)]
    edges = path["edges"]
    rpcs = [0] # default include rpc_0 for single tier scenario
    for edge in edges:
      rpcs.append(edge[0])
      rpcs.append(edge[1])
    rpcs = list(set(rpcs))
    services = []
    with open(config + 'rpcs.json') as load_rpcs_file:
      rpcs_json = json.load(load_rpcs_file)
      for rpc in rpcs:
        # services.append(rpcs_json["rpc_" + str(rpc)]["client"])
        services.append(rpcs_json["rpc_" + str(rpc)]["server"])
    services = list(set(services))
    if "nginx" in services:
      services.remove("nginx")
    services.sort()
    services_related = {}
    with open(config + 'services.json') as load_services_file:
      services_json = json.load(load_services_file)
      for service in services:
        related_services = [service]
        for i in services_json[service]["rpcs_send"]:
          related_services.append(rpcs_json["rpc_" + str(i)]["server"])
        services_related[service] = related_services
    f = open("cog.txt", "w")
    for service in services:
      os.system('mkdir -p ../src/' + service)
      if services_json[service]["server_type"] == "threaded":
        print >> f, "template_thrift.cpp -d -D config=" + config + " -D serviceName='" + service + "' -D pathName='path_" + str(path_id) \
          + "' -o ../src/" + service + "/" + service + ".cpp"
      else:
        os.system("cp ../src/tcp_server/main.cpp ../src/" + service + "/" + service + ".cpp")
        os.system("cp ../src/tcp_server/assembly.cpp ../src/" + service + "/assembly.cpp")
      with open("../src/" + service + "/CMakeLists.txt", "w") as cmake_f:
        if services_json[service]["server_type"] != "threaded":
          if services_json[service]["server_type"] == "mongod":
            print >> cmake_f, "set(CMAKE_CXX_FLAGS  \"${CMAKE_CXX_FLAGS} -g -masm=intel -lpthread -DMONGODB\")"
          elif services_json[service]["server_type"] == "memcached":
            print >> cmake_f, "set(CMAKE_CXX_FLAGS  \"${CMAKE_CXX_FLAGS} -g -masm=intel -lpthread -DMEMCACHED\")"
          elif services_json[service]["server_type"] == "nginx":
            print >> cmake_f, "set(CMAKE_CXX_FLAGS  \"${CMAKE_CXX_FLAGS} -g -masm=intel -lpthread -DNGINX\")"
          elif services_json[service]["server_type"] == "redis":
            print >> cmake_f, "set(CMAKE_CXX_FLAGS  \"${CMAKE_CXX_FLAGS} -g -masm=intel -lpthread -DREDIS\")"
          else:
            print >> cmake_f, "set(CMAKE_CXX_FLAGS  \"${CMAKE_CXX_FLAGS} -g -masm=intel -lpthread\")"
          print >> cmake_f, ""
          print >> cmake_f, "add_executable("
          print >> cmake_f, '\t' + service
          print >> cmake_f, '\t' + service + '.cpp'
          print >> cmake_f, '\t' + 'assembly.cpp'
          print >> cmake_f, '\t${TCP_SERVER_CPP_DIR}/connection/tcp_conn.cpp'
          print >> cmake_f, '\t${TCP_SERVER_CPP_DIR}/webserver.cpp'
          print >> cmake_f, '\t${TCP_SERVER_CPP_DIR}/config.cpp'
          print >> cmake_f, '\t${TCP_SERVER_CPP_DIR}/utils.cpp'
          print >> cmake_f, ")"
          print >> cmake_f, ""
          print >> cmake_f, "target_link_libraries("
          print >> cmake_f, '\t' + service
          print >> cmake_f, '\t' + 'pthread'
          print >> cmake_f, ")"
          print >> cmake_f, ""
          print >> cmake_f, 'install(TARGETS ' + service + ' DESTINATION ./)'
        else:
          print >> cmake_f, "set(CMAKE_CXX_FLAGS  \"${CMAKE_CXX_FLAGS} -g -masm=intel -lpthread\")"
          print >> cmake_f, ""
          print >> cmake_f, "add_executable("
          print >> cmake_f, '\t' + service
          print >> cmake_f, '\t' + service + '.cpp'
          print >> cmake_f, '\t' + 'assembly.cpp'
          for i in services_related[service]:
            print >> cmake_f, '\t${THRIFT_GEN_CPP_DIR}/' + i + '.cpp'
          print >> cmake_f, '\t${THRIFT_GEN_CPP_DIR}/auto_microservices_types.cpp'
          print >> cmake_f, ")"
          print >> cmake_f, ""
          print >> cmake_f, "target_include_directories("
          print >> cmake_f, '\t' + service + ' PRIVATE'
          print >> cmake_f, "\t/usr/local/include/jaegertracing"
          print >> cmake_f, ")"
          print >> cmake_f, ""
          print >> cmake_f, "target_link_libraries("
          print >> cmake_f, '\t' + service
          print >> cmake_f, "\t${THRIFT_LIB}"
          print >> cmake_f, "\t${CMAKE_THREAD_LIBS_INIT}"
          print >> cmake_f, "\t${Boost_LIBRARIES}"
          print >> cmake_f, "\tnlohmann_json::nlohmann_json"
          print >> cmake_f, "\tOpenSSL::SSL"
          print >> cmake_f, "\t/usr/local/lib/libthrift.so"
          print >> cmake_f, "\t/usr/local/lib/libthriftnb.so"
          print >> cmake_f, "\tjaegertracing"
          print >> cmake_f, "\tBoost::log"
          print >> cmake_f, "\tBoost::log_setup"
          print >> cmake_f, "\t/usr/local/lib/libjaegertracing.so"
          print >> cmake_f, '\t' + 'pthread'
          print >> cmake_f, ")"
          print >> cmake_f, ""
          print >> cmake_f, 'install(TARGETS ' + service + ' DESTINATION ./)'
    f.close()
    f = open("../src/CMakeLists.txt", "w")
    for service in services:
      print >> f, 'add_subdirectory(' + service + ')'
    f.close()
    os.system('cog @cog.txt')
    for service in services:
      os.system('clang-format -i ../src/' + service + '/' + service + '.cpp -style={"IndentWidth: 2"}')
    os.system('rm cog.txt')
    for service in services:
      with open("../src/" + service + "/utils.h", "w") as utils_f:
        print >> utils_f, "#ifndef UTILS_H"
        print >> utils_f, "#define UTILS_H"
        print >> utils_f, ""
        print >> utils_f, "#include <cstdint>"
        print >> utils_f, ""
        print >> utils_f, "void runAssembly0(uint64_t* mem_data, uint64_t req_id, uint64_t* curr_mem_addrs, uint64_t* curr_pointer_chasing_mem_addrs);"
        print >> utils_f, ""
        print >> utils_f, "#endif"
      with open("../src/" + service + "/assembly.cpp", "w") as assembly_f:
        print >> assembly_f, "#include <cstdint>"
        print >> assembly_f, "using namespace std;"
        print >> assembly_f, "void runAssembly0(uint64_t* mem_data, uint64_t req_id, uint64_t* curr_mem_addrs, uint64_t* curr_pointer_chasing_mem_addrs) {"
        print >> assembly_f, "}"

if __name__ == "__main__":
  main(sys.argv[1:])