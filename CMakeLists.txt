cmake_minimum_required(VERSION 3.21)
project(Xarxes_Practica1 C)

set(CMAKE_C_STANDARD 17)

add_executable(Client client.c
        modules/config.c modules/headers/config.h
        modules/conexions.c modules/headers/conexions.h
        modules/terminal.c modules/headers/terminal.h
        modules/alive.c modules/headers/alive.h
        modules/pdu.c modules/headers/pdu.h
        modules/register.c modules/headers/register.h
        modules/sendrecive.c modules/headers/sendrecive.h
        modules/socket.c modules/headers/socket.h
        modules/headers/globals.h
        modules/globals.c)

cmake_minimum_required(VERSION 3.21)

add_definitions("-ansi -pedantic -Wall")