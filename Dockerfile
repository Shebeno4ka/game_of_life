FROM ubuntu:22.04

# Установка зависимостей
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libboost-all-dev \
    # libsdl3-dev \
    && rm -rf /var/lib/apt/lists/*

# Создание рабочей директории
WORKDIR /app

# Копирование исходного кода
COPY src/ ./src/
COPY CMakeLists.txt ./

# Сборка приложения
RUN mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc)

# Открытие порта
EXPOSE 8080

# Запуск приложения
CMD ["./build/GameOfLife"]