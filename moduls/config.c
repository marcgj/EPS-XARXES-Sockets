//
// Created by Marc Gaspà Joval on 2/3/22.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "headers/config.h"

void load_config(char *filename, ClientCfg *clientConfig) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        fprintf(stderr, "Error obrint arxiu %s: ", filename);
        perror("");
        exit(1);
    }

    fscanf(f, "Id = %s\n", clientConfig->id);

    fscanf(f, "Elements = ");
    for (int i = 0, eol = 0; i < 5 && !eol; i++) {
        Element *elem = &clientConfig->elements[i];
        fscanf(f, "%3s-", elem->magnitude);
        fscanf(f, "%d-", &elem->ordinal);
        fscanf(f, "%c", &elem->type);
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

    printf("CONFIGURACIO CARGADA\n");
    fclose(f);
}

void print_config(ClientCfg *cfg) {
    printf("---------- DADES DISPOSITIU ----------\n");
    printf(" Id: %s \n", cfg->id);
    printf(" Address: %s \n", cfg->address);
    printf(" TCP: %i\n", cfg->local_TCP);
    printf(" UDP: %d \n\n", cfg->server_UDP);
    print_elements(cfg->elemc, cfg->elements);
    printf("-----------------------------------");

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

void print_elements(int elemc, Element elements[elemc]) {
    printf("   Parametres \t Valors\n");
    printf("   ----------- \t ------------------\n");
    for (int i = 0; i < elemc; ++i) {
        Element elem = elements[i];
        printf("   %s \t\t %s\n", elem.elem_string, elem.value);
    }
}


