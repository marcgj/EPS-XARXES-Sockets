//
// Created by Marc Gaspà Joval on 2/3/22.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "headers/config.h"
#include "headers/terminal.h"
#include "headers/globals.h"


void load_config(char *filename, ClientCfg *clientConfig) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        print_error("Error obrint arxiu %s: ", filename);
        perror("");
        exit(1);
    }

    fscanf(f, "Id = %s\n", clientConfig->id);

    fscanf(f, "Elements = ");
    for (int i = 0, eol = 0; i < 5 && !eol; i++) {
        Element *elem = &clientConfig->elements[i];
        fscanf(f, "%3s-", elem->magnitude);
        fscanf(f, "%1i-", &elem->ordinal);
        fscanf(f, "%1c", &elem->type);
        strcpy(clientConfig->elements[i].value, "NONE");
        sprintf(elem->elem_string, "%s-%d-%c", elem->magnitude, elem->ordinal, elem->type);

        char c;
        fscanf(f, "%c", &c);

        if (c != ';') {
            eol = 1;
            clientConfig->elemc = i + 1;
        }
    }

    fscanf(f, "Local-TCP = %i\n", &clientConfig->local_TCP);
    fscanf(f, "Server = %s\n", clientConfig->address);
    fscanf(f, "Server-UDP = %i\n", &clientConfig->server_UDP);

    elements_to_string(clientConfig->elements_string, clientConfig->elemc, clientConfig->elements);

    print_message("Configuracio carregada del archiu %s\n", filename);
    fclose(f);
}

void print_config(ClientCfg *cfg) {
    print_debug("---------- DADES DISPOSITIU ----------\n");
    print_debug(" Id: %s \n", cfg->id);
    print_debug(" Address: %s \n", cfg->address);
    print_debug(" TCP: %i\n", cfg->local_TCP);
    print_debug(" UDP: %d \n", cfg->server_UDP);
    print_debug("\n");
    print_elements(cfg->elemc, debug, cfg->elements);
    print_debug("-----------------------------------\n");

}

void elements_to_string(char *str, int elemc, Element elems[elemc]) {
    for (int i = 0; i < elemc; i++) {
        char temp[16];
        Element elem = elems[i];
        sprintf(temp, "%s-%i-%c", elem.magnitude, elem.ordinal, elem.type);
        if (i < elemc - 1) {
            temp[7] = ';';
            temp[8] = '\0';
        }
        strcat(str, temp);
    }
}

Element *getElement(ClientCfg *c, char *str) {
    for (int i = 0; i < c->elemc; i++) {
        Element *elem = &c->elements[i];
        if (strcmp(str, elem->elem_string) == 0) return elem;
    }
    return NULL;
}

void print_elements(int elemc, int dbg, Element elements[elemc]) {
    char tag[16] = "MESSAGE";
    if (dbg) strcpy(tag, "DEBUG");

    print_alt(stdout, tag, "Parametres \t Valors\n");
    print_alt(stdout, tag, "----------- \t ------------------\n");
    for (int i = 0; i < elemc; ++i) {
        Element elem = elements[i];
        print_alt(stdout, tag, "%s \t\t %s\n", elem.elem_string, elem.value);
    }
}


