# thrift

nginx-web-server contains configuration of openresty to work as the frontend to transfer http requests from clients(wrk2) to rpc call.

src contains some head files needed by the source code of each microservice.

scripts contains scripts to automatically generate codes and files.

### Usage

Make sure you have installed all the dependencies.

```
cd scripts && python auto.py 0(path_id) && cd ../
mkdir build
cd build
cmake ..
make
```