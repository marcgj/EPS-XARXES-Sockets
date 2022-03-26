//
// Created by fedora on 23/3/22.
//

#include <printf.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include "headers/config.h"
#include "headers/globals.h"
#include "headers/sendrecive.h"
#include "headers/terminal.h"


void handle_terminal() {
    char buffer[128];
    char *token;

    fgets(buffer, sizeof(buffer), stdin);
    buffer[strlen(buffer) - 1] = '\0';

    if (strlen(buffer) == 0) return;

    token = strtok(buffer, " ");

    if (strcmp(token, "stat") == 0) {
        printf("---------------------------------------\n");
        printf("Id= %s\n", cfg.id);
        printf("Status= %i\n\n", status);
        print_elements(cfg.elemc, cfg.elements);
        printf("---------------------------------------\n");
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

void print(FILE * fd, char *tag, char * format, va_list args, ...){
    if(args == NULL){
        va_start(args, args);
    }
    char buffer[264] = "";

    strcpy(buffer, tag);
    strcat(buffer, " >> ");

    strcat(buffer, format);
    vfprintf(fd, buffer, args);
}


//https://www.cplusplus.com/reference/cstdio/vprintf/
void print_error(char *format, ...) {
    va_list args;
    va_start(args, format);

    print(stderr, "ERROR", format, args);
}


void print_debug(char *format, ...) {
    if(!debug) return;

    va_list args;
    va_start(args, format);

    print(stdout, "DEBUG", format, args);

}

void print_message(char *format, ...) {
    va_list args;
    va_start(args, format);

    print(stdout, "MESSAGE", format, args);
}
