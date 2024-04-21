import json
import os
import sys

image_name = "ml2585/synthetic_social_network:tcp"
config = "/users/chenqh23/Ditto-6.S984/synthetic_social_network/config/"

def main(argv):
  path_id = argv[0]
  # with open(config + 'services.json') as load_services_file:
  #   services_json = json.load(load_services_file)
  #   for service in services_json:
  #     if os.path.exists("../src/" + service):
  #       os.system('rm -rf ../src/' + service)
  # os.system('docker rmi ' + image_name)
  os.system('rm -rf ../gen-cpp')
  os.system('rm -rf ../gen-lua')

  os.system('python2 thrift_gen.py ' + config)
  os.system('python2 lua_rewrite.py')
  # os.system('python2 src_files_gen.py ' + path_id + ' ' + config)
  os.system('python2 docker-compose_gen.py ' + path_id + ' ' + image_name + ' ' + config)

if __name__ == "__main__":
  main(sys.argv[1:])
