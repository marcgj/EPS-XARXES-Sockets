//
// Created by fedora on 23/3/22.
//

#ifndef XARXES_PRACTICA1_ELEMCONTROLLER_H
#define XARXES_PRACTICA1_ELEMCONTROLLER_H

typedef struct{
    char magnitude[3];
    int ordinal;
    char type;

    char value[15];
    char elem_string[32];
} Element;

void handle_terminal();
void print_elements(int elemc, Element elements[elemc]);


#endif //XARXES_PRACTICA1_ELEMCONTROLLER_H
