cmake_minimum_required(VERSION 3.10)

project(GameOfLife)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
    add_compile_options(/W4 /WX /permissive-)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wconversion
        -Wshadow
        -Wunused
        -Wcast-align
        -Wcast-qual
        -Wformat=2
        -Wno-unused-parameter
    )
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)

add_executable(GameOfLife
        src/main.cpp
        src/game_state.cpp
        src/game_runner.cpp
        src/game_renderer.cpp
        src/thread_safe_game_state.cpp
)

target_include_directories(GameOfLife PRIVATE ${SDL3_INCLUDE_DIRS})
target_link_libraries(GameOfLife PRIVATE ${SDL3_LIBRARIES})

target_compile_options(GameOfLife PRIVATE ${SDL3_CFLAGS_OTHER})