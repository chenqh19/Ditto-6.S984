import json
import os
from collections import OrderedDict
import sys

def main(argv):
  config = argv[0]
  f = open("auto_microservices.thrift", 'w')
  print >> f, "namespace cpp auto_microservices"
  print >> f, "namespace lua auto_microservices"
  print >> f, ""
  with open(config + "services.json",'r') as load_services_f:
    services_json = json.load(load_services_f, object_pairs_hook=OrderedDict)
    print >> f, "enum ErrorCode {"
    print >> f, "  SE_CONNPOOL_TIMEOUT,"
    print >> f, "  SE_THRIFT_CONN_ERROR,"
    print >> f, "  SE_THRIFT_HANDLER_ERROR"
    print >> f, "}"
    print >> f, "exception ServiceException {"
    print >> f, "  1: ErrorCode errorCode;"
    print >> f, "  2: string message;"
    print >> f, "}"
    for service in services_json:
      if service == "ssl":
        continue
      print >> f, "service " + service
      print >> f, "{"
      for rpc in services_json[service]["rpcs_receive"]:
        print >> f, "  void rpc_" + str(rpc) + "(1: map<string, string> carrier) throws (1: ServiceException se)"
      print >> f, "}"
      print >> f, ""

  f.close()
  os.system("cd ../ && thrift -r --gen cpp scripts/auto_microservices.thrift")
  os.system("cd ../ && thrift -r --gen lua scripts/auto_microservices.thrift")
  os.system("rm auto_microservices.thrift")

if __name__ == "__main__":
  main(sys.argv[1:])