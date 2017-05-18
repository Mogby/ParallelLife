cmake_minimum_required(VERSION 3.6)
project(ParallelLife)

set(CMAKE_C_STANDARD 11)

find_package(Threads)

set(SOURCE_FILES src/main.c)

add_library(mylib src/mylib.c)

add_executable(life ${SOURCE_FILES})

target_link_libraries(life ${CMAKE_THREAD_LIBS_INIT} mylib)
