FROM ml2585/thrift-microservice-deps:bionic

ARG NUM_CPUS=40

COPY ./ /auto_microservices
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
        wget \
        bzip2 \
        autotools-dev \
        automake \
        build-essential \
        libicu60 \
        git \
    && cd /auto_microservices \
    && mkdir -p build \
    && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Debug .. \
    && make -j${NUM_CPUS} \
    && make install 
    # && cd / \
    # && wget --user-agent="Mozilla" https://software.intel.com/content/dam/develop/external/us/en/documents/downloads/sde-external-8.69.1-2021-07-18-lin.tar.bz2 \
    # && tar -xvf sde-external-8.69.1-2021-07-18-lin.tar.bz2 \
    # && cd / \
    # && git clone https://github.com/gy1005/valgrind.git \
    # && cd valgrind \
    # && mkdir build \
    # && ./autogen.sh \
    # && ./configure --prefix=/valgrind/build \
    # && make -j$(nproc) \
    # && make install

WORKDIR /auto_microservices