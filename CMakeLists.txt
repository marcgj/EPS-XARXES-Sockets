cmake_minimum_required(VERSION 3.21)
project(Xarxes_Practica1 C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../cli)

add_executable(client_meu cli/client.c
        cli/modules/config.c cli/modules/headers/config.h
        cli/modules/conexions.c cli/modules/headers/conexions.h
        cli/modules/terminal.c cli/modules/headers/terminal.h
        cli/modules/alive.c cli/modules/headers/alive.h
        cli/modules/pdu.c cli/modules/headers/pdu.h
        cli/modules/register.c cli/modules/headers/register.h
        cli/modules/sendrecive.c cli/modules/headers/sendrecive.h
        cli/modules/socket.c cli/modules/headers/socket.h
        cli/modules/headers/globals.h
        cli/modules/globals.c)

cmake_minimum_required(VERSION 3.21)

add_definitions("-ansi -pedantic -Wall")