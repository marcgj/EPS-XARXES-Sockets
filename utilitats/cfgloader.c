//
// Created by Marc Gaspà Joval on 2/3/22.
//

#include "cfgloader.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void load_config(char *filename, ClientCfg *clientConfig) {
    system("pwd");

    FILE *f = fopen(filename, "r");
    if (f == NULL){
        fprintf(stderr, "Error obrint arxiu %s: ", filename);
        perror("");
        exit(1);
    }

    fscanf(f, "Id = %s\n", clientConfig->id);

    fscanf(f, "Elements = ");
    for(int elem = 0, eol = 0; elem < 5 && !eol; elem++){
        fscanf(f, "%3s-", clientConfig->elements[elem].magnitude);
        fscanf(f, "%d-", &clientConfig->elements[elem].ordinal);
        fscanf(f, "%c", &clientConfig->elements[elem].type);

        char c;
        fscanf(f, "%c", &c);
        if (c != ';') {
            eol = 1;
            clientConfig->elemc = elem+1;
        }
    }

    fscanf(f, "Local-TCP = %i\n", &clientConfig->local_TCP);
    fscanf(f, "Server = %s\n", clientConfig->address);
    fscanf(f, "Server-UDP = %i\n", &clientConfig->server_UDP);

    printf("Conf Loaded\n");
    fclose(f);
}

void print_config(ClientCfg *cfg) {
    printf("---------- Client Config ----------\n");
    printf("Id: %s \n", cfg->id);
    printf("Address: %s \n", cfg->address);
    printf("TCP: %i\n", cfg->local_TCP);
    printf("UDP: %d \n\n", cfg->server_UDP);
    for (int i = 0; i < cfg->elemc; ++i) {
        printf("Mag: %s ", cfg->elements[i].magnitude);
        printf("Ord: %i ", cfg->elements[i].ordinal);
        printf("Type: %c \n", cfg->elements[i].type);
    }
    printf("-----------------------------------");

}


