#define _POSIX_SOURCE
#include <printf.h>
#include <stdarg.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>

#include "headers/config.h"
#include "headers/globals.h"
#include "headers/sendrecive.h"
#include "headers/terminal.h"

/*
 * Funcio encarregada de gestionar les diferentes comandes que es poden introduir per el terminal
 */
void handle_terminal() {
    char buffer[128];
    char *token;

    fgets(buffer, sizeof(buffer), stdin);
    buffer[strlen(buffer) - 1] = '\0';

    if (strlen(buffer) == 0) return;

    token = strtok(buffer, " ");

    if (strcmp(token, "stat") == 0) {
        print_message("---------------------------------------\n");
        print_message("Id= %s\n", cfg.id);
        print_message("Status= %i\n", status);
        print_message("\n");
        print_elements(cfg.elemc, 0, cfg.elements);
        print_message("---------------------------------------\n");
    } else if (strcmp(token, "set") == 0) {
        token = strtok(NULL, " ");
        if (token == NULL) {
            fprintf(stderr, "Error de sintaxis comanda set\n");
            return;
        }
        Element *elem = getElement(&cfg, token);
        if (elem == NULL) {
            fprintf(stderr, "Error el dispositiu %s no exesteix\n", token);
        }

        token = strtok(NULL, " ");
        if (token == NULL) {
            fprintf(stderr, "Error de sintaxis comanda set\n");
            return;
        }
        strcpy(elem->value, token);
    } else if (strcmp(token, "send") == 0) {
        token = strtok(NULL, " ");
        if (token == NULL) {
            fprintf(stderr, "Error de sintaxis comanda set\n");
            return;
        }

        Element *elem = getElement(&cfg, token);

        if (elem == NULL) {
            fprintf(stderr, "Error el dispositiu %s no exesteix\n", token);
            return;
        }

        send_element(*elem);

    } else if (strcmp(token, "quit") == 0) {
        kill(0, SIGTERM);
    } else {
        fprintf(stderr, "La comanda %s no es acceptada\n", token);
    }

}

/*
 * Les seguents funcions simplifiquen el imprimir errors i a part m'han servit per experimentar amb funcions amb nombre
 * d'arguments variable
 */

// Aquesta funcio no es mes que un wrapper del print que hi posa la hora
void print(FILE *fd, char *tag, char *format, va_list args) {
    char buffer[264] = "";
    time_t t1;
    struct tm *t2;
    time(&t1);
    t2 = localtime(&t1);
    strftime(buffer, 128, "%H:%M:%S: ", t2);

    strcat(buffer, tag);
    strcat(buffer, " >> ");

    strcat(buffer, format);
    vfprintf(fd, buffer, args);
}

// Nomes s'utilitza en algun cas puntual on conincideixen prints que poden ser o debug o message
void print_alt(FILE *fd, char *tag, char *format, ...) {
    va_list args;
    va_start(args, format);

    print(fd, tag, format, args);
}

void print_error(char *format, ...) {
    va_list args;
    va_start(args, format);

    print(stderr, "ERROR", format, args);
}


void print_debug(char *format, ...) {
    if (!debug) return;

    va_list args;
    va_start(args, format);

    print(stdout, "DEBUG", format, args);

}

void print_message(char *format, ...) {
    va_list args;
    va_start(args, format);

    print(stdout, "MESSAGE", format, args);
}

