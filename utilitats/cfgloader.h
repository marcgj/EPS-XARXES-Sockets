//
// Created by Marc Gaspà Joval on 2/3/22.
//

#ifndef SO_PRACTICA1_CFGLOADER_H
#define SO_PRACTICA1_CFGLOADER_H


#include "elemcontroller.h"

typedef struct{
    char id[10];

    Element elements[5];
    int elemc;

    char elements_string[32];

    int local_TCP;
    char address[64];
    int server_UDP;
} ClientCfg;

void load_config(char *filename, ClientCfg *clientConfig);
void print_config(ClientCfg *clientCfg);
void elements_to_string(char * str, int elemc, Element elems[elemc]) ;
#endif //SO_PRACTICA1_CFGLOADER_H
