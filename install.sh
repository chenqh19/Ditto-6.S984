### Prepare for running applications

# build memcached
cd applications/memcached-1.6.24/
sudo add-apt-repository ppa:ondrej/php
apt-get install libevent-dev
./configure --prefix=/usr/local/memcached
make && make test && sudo make install
# build client
cd ../../client/mutated/
git submodule update --init
./autogen.sh
./configure
make


