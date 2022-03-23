cmake_minimum_required(VERSION 3.21)
project(Xarxes_Practica1 C)

set(CMAKE_C_STANDARD 17)

add_executable(Client client.c utilitats/cfgloader.c utilitats/cfgloader.h utilitats/conexions.c utilitats/conexions.h utilitats/elemcontroller.c utilitats/elemcontroller.h)

cmake_minimum_required(VERSION 3.21)

add_definitions("-ansi -pedantic -Wall")