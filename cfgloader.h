//
// Created by Marc Gaspà Joval on 2/3/22.
//

#ifndef SO_PRACTICA1_CFGLOADER_H
#define SO_PRACTICA1_CFGLOADER_H

typedef struct{
    char magnitude[3];
    int ordinal;
    char type;
} Element;

typedef struct{
    char id[10];

    Element elements[5];
    int elemc;

    int tcpPort;
    char address[64];
    int udpPort;
} ClientCfg;

void load_config(char *filename, ClientCfg *clientConfig);
void print_config(ClientCfg *clientCfg);

#endif //SO_PRACTICA1_CFGLOADER_H
