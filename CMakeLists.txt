cmake_minimum_required(VERSION 3.21)
project(Xarxes_Practica1 C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ..)
add_definitions("-ansi -pedantic -Wall")

add_executable(client client.c
        client_modules/config.c client_modules/headers/config.h
        client_modules/conexions.c client_modules/headers/conexions.h
        client_modules/terminal.c client_modules/headers/terminal.h
        client_modules/alive.c client_modules/headers/alive.h
        client_modules/pdu.c client_modules/headers/pdu.h
        client_modules/register.c client_modules/headers/register.h
        client_modules/sendrecive.c client_modules/headers/sendrecive.h
        client_modules/socket.c client_modules/headers/socket.h
        client_modules/headers/globals.h
        client_modules/globals.c)

cmake_minimum_required(VERSION 3.21)

