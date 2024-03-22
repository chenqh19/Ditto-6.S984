import os
import json
import sys

def main(argv):
  path_id = argv[0]
  image_name = argv[1]
  config = argv[2]

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
    services_port = {}
    services_replica = {}
    with open(config + 'services.json') as load_services_file:
      services_json = json.load(load_services_file)
      for service in services:
        services_port[service] = services_json[service]["server_port"]
        services_replica[service] = services_json[service]["replica"]
    os.system("cp docker-compose_temp.yml docker-compose.yml")
    f = open("docker-compose.yml", "a")
    for service in services:
      service_id = int(service.split('_')[-1])
      print >> f, '  ' + service + ':'
      print >> f, '    image: ' + image_name
      print >> f, '    hostname: ' + service
      # print >> f, '    deploy:'
      # print >> f, '      replicas: ' + str(services_replica[service])
      # print >> f, '      resources:'
      # print >> f, '        limits:'
      # print >> f, "          cpus: '1'"
      # print >> f, '          memory: 1G'
      # print >> f, '    ports:'
      # print >> f, '      - ' + str(services_port[service]) + ":" + str(services_port[service])
      print >> f, '    restart: always'
      print >> f, '    init: true'
      if services_json[service]["server_type"] == "memcached":
        print >> f, '    entrypoint: ' + service + ' -n 4'
      else:
        print >> f, '    entrypoint: ' + service
      print >> f, '    volumes:'
      print >> f, '      - ' + config + ':/auto_microservices/config'
      print >> f, ""
    f.close()
    os.system("mv docker-compose.yml ../docker-compose.yml")

if __name__ == "__main__":
  main(sys.argv[1:])

