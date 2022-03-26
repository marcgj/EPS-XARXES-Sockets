cmake_minimum_required(VERSION 3.21)
project(Xarxes_Practica1 C)

set(CMAKE_C_STANDARD 17)

add_executable(Client client.c
        moduls/config.c moduls/headers/config.h
        moduls/conexions.c moduls/headers/conexions.h
        moduls/terminal.c moduls/headers/terminal.h
        moduls/alive.c moduls/headers/alive.h
        moduls/pdu.c moduls/headers/pdu.h
        moduls/register.c moduls/headers/register.h
        moduls/sendrecive.c moduls/headers/sendrecive.h
        moduls/socket.c moduls/headers/socket.h
        moduls/globals.h
        )

cmake_minimum_required(VERSION 3.21)

add_definitions("-ansi -pedantic -Wall")