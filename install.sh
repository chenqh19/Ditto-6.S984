### Prepare for running applications

# build memcached
cd ~/Ditto-6.S984/applications/memcached-1.6.24/
sudo add-apt-repository ppa:ondrej/php
yes | sudo apt-get install libevent-dev
./configure --prefix=/usr/local/memcached
make && make test && sudo make install
# build client
cd ~/Ditto-6.S984/client/mutated/
git submodule update --init
./autogen.sh
./configure
make

# install dependencies
sudo apt install htop
sudo apt install linux-tools-common
yes | sudo apt install linux-tools-5.4.0-164-generic
yes | sudo apt install python3 pip
yes | sudo apt install cmake
pip install jsoncomment
pip install -U scikit-learn scipy matplotlib
pip install wheel
pip install pandas
pip install disjoint-set

### install systap
cd ~/Ditto-6.S984/systemtap-4.4
./configure --prefix=/opt/stap --disable-docs --disable-publican --disable-refdocs CFLAGS="-g -O2"
make -j8
sudo make install

sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys C8CAB6595FDFF622 
codename=$(lsb_release -c | awk  '{print $2}')
sudo tee /etc/apt/sources.list.d/ddebs.list << EOF
deb http://ddebs.ubuntu.com/ ${codename}      main restricted universe multiverse
deb http://ddebs.ubuntu.com/ ${codename}-security main restricted universe multiverse
deb http://ddebs.ubuntu.com/ ${codename}-updates  main restricted universe multiverse
deb http://ddebs.ubuntu.com/ ${codename}-proposed main restricted universe multiverse
EOF

sudo apt-get update
sudo apt-get install linux-image-$(uname -r)-dbgsym