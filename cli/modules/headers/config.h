//
// Created by Marc Gaspà Joval on 2/3/22.
//

#ifndef SO_PRACTICA1_CFGLOADER_H
#define SO_PRACTICA1_CFGLOADER_H

typedef struct {
    char magnitude[3];
    int ordinal;
    char type;

    char value[15];
    char elem_string[32];
} Element;

typedef struct {
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

Element *getElement(ClientCfg *c, char *str);

void elements_to_string(char *str, int elemc, Element elems[elemc]);

void print_elements(int elemc, int dbg, Element elements[elemc]);

#endif //SO_PRACTICA1_CFGLOADER_H
