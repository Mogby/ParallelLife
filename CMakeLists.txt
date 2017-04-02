cmake_minimum_required(VERSION 3.4)
project(ParallelLife)

set(CMAKE_C_STANDARD 11)

find_package(Threads)

find_package(Curses REQUIRED)

include_directories(${CURSES_INCLUDE_DIR})

set(SOURCE_FILES src/main.c src/field.c src/life.c src/runner.c)

add_executable(life ${SOURCE_FILES})

target_link_libraries(life ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(life ${CURSES_LIBRARIES})

