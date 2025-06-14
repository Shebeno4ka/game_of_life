FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    libboost-all-dev \
    gdb \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*