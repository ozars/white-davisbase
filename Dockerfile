FROM debian:buster

RUN DEBIAN_FRONTEND=noninteractive apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
    g++ make cmake clang-format libboost-dev
ADD . /app
WORKDIR /app
RUN mkdir -p build && cd build && cmake ../ && make
WORKDIR /app/build
