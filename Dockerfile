FROM debian:buster

RUN DEBIAN_FRONTEND=noninteractive apt-get update \
 && apt-get install -y --no-install-recommends g++=4:8.3.0-1 make=4.2.1-1.2 \
     cmake=3.13.4-1 clang-format=1:7.0-47 libboost-dev=1.67.0.1 \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*
COPY . /app
WORKDIR /app/build
RUN cmake ../ && make
