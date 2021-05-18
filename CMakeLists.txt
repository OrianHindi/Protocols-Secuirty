cmake_minimum_required(VERSION 3.17)
project(Protocols_Security C)

set(CMAKE_C_STANDARD 99)

add_executable(Protocols_Security main.c ipv6_flood.c)