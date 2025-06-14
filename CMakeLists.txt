cmake_minimum_required(VERSION 3.16)
project(GameOfLife)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# Включить директории
include_directories(src)

# Исходные файлы
set(SOURCES
    src/core/BitField.cpp
    src/core/GameSimulator.cpp
    src/server/GameServer.cpp
    src/network/WebSocketServer.cpp
)

# Заголовочные файлы
set(HEADERS
    src/core/BitField.h
    src/core/GameSimulator.h
    src/utils/MPSCQueue.h
    src/server/GameServer.h
    src/network/WebSocketServer.h
    src/utils/AsyncUtils.h
    src/utils/Constants.h
        src/core/GameEvent.h
)

# Основное приложение
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} src/main.cpp)

find_package(Boost REQUIRED COMPONENTS system thread)

# Линковка библиотек
if(Boost_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
    
    message(STATUS "Основное приложение собрано с jsoncpp и Boost.Beast")
else()
    message(FATAL_ERROR "Boost не найден")
endif()