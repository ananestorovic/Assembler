cmake_minimum_required(VERSION 3.20)
project(Assembler)

set(CMAKE_CXX_STANDARD 17)

include_directories(inc)
add_executable(Assembler src/main.cpp inc/assembler.h src/assembler.cpp inc/regexes.h inc/exceptions.h)
