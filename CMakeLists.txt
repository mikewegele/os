cmake_minimum_required(VERSION 4.0)
project(projectOS C)

set(CMAKE_C_STANDARD 11)

add_executable(projectOS main.c
        task1/serial_demo.c
)
