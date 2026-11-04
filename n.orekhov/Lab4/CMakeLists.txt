cmake_minimum_required(VERSION 3.13)
project(Lab4 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(prog main.c array.c array.h)
