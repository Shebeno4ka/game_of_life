FROM ubuntu:22.04

# Установка зависимостей и базовых библиотек
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    libboost-all-dev \
    gdb \
    git \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Сборка и установка spdlog из исходников
RUN git clone --depth=1 https://github.com/gabime/spdlog.git /tmp/spdlog && \
    cd /tmp/spdlog && \
    mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc) && \
    make install && \
    cd / && rm -rf /tmp/spdlog
