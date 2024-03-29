#ifndef XARXES_PRACTICA1_TERMINAL_H
#define XARXES_PRACTICA1_TERMINAL_H

#include <stdarg.h>
#include <stdio.h>

void print(FILE *fd, char *tag, char *format, va_list args);

void print_alt(FILE *fd, char *tag, char *format, ...);

void print_error(char *format, ...);

void print_debug(char *format, ...);

void print_message(char *format, ...);

void handle_terminal();

#endif //XARXES_PRACTICA1_TERMINAL_H
