cmake_minimum_required(VERSION 3.21)
project(SO_Practica1 C)

set(CMAKE_C_STANDARD 17)

add_executable(Client client.c cfgloader.c cfgloader.h)

cmake_minimum_required(VERSION 3.21)

add_definitions("-ansi -pedantic -Wall")