cmake_minimum_required(VERSION 3.10)

# Название проекта
project(GameOfLife)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Добавление исполняемого файла
add_executable(GameOfLife src/main.cpp src/game_state.cpp src/game_state.hpp src/game_runner.cpp src/game_renderer.cpp src/sdl_context.cpp src/thread_safe_game_state.cpp)

# Подключение библиотеки SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(GameOfLife ${SDL2_LIBRARIES})