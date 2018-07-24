cmake_minimum_required(VERSION 3.10)
project(simple_database)

set(CMAKE_CXX_STANDARD 11)

add_executable(simple_database src/main.cpp src/statements.cpp include/project/statements.h src/parser.cpp include/project/parser.h src/store.cpp include/project/store.h)