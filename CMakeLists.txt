cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O2 -g -pg")
set(CMAKE_VERBOSE_MAKEFILE ON)
message(${CMAKE_CXX_FLAGS_RELEASE})

project(2l_busy_beaver)
add_executable(${PROJECT_NAME} main.cpp field.h run.h state.h global.h)
