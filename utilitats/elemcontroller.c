//
// Created by fedora on 23/3/22.
//

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wait.h>
#include "elemcontroller.h"
#include "cfgloader.h"

extern int debug;
extern unsigned char status;
extern ClientCfg cfg;

extern int pids[3];

Element *getElement(ClientCfg * c, char * str) {
    for(int i = 0; i < c->elemc; i++){
        Element * elem = &c->elements[i];
        if (strcmp(str, elem->elem_string) == 0) return elem;
    }
    return NULL;
}

void handle_terminal() {
        char buffer[128];
        char * token;

        fgets(buffer, sizeof(buffer), stdin);
        buffer[strlen(buffer) - 1] = '\0';

        if (strlen(buffer) == 0) return;

        token = strtok(buffer, " ");

        if (strcmp(token, "stat") == 0){
            printf("---------------------------------------\n");
            printf("Id= %s\n", cfg.id);
            printf("Status= %i\n\n", status);
            print_elements(cfg.elemc, cfg.elements);
            printf("---------------------------------------\n");
        }else if (strcmp(token, "set") == 0){
            token = strtok(NULL, " ");
            if (token == NULL) {
                fprintf(stderr, "Error de sintaxis comanda set\n");
                return;
            }
            Element * elem = getElement(&cfg, token);
            if (elem != NULL){
                token = strtok(NULL, " ");
                if (token == NULL) {
                    fprintf(stderr, "Error de sintaxis comanda set\n");
                    return;
                }

                strcpy(elem->value, token);
            }else{
                fprintf(stderr, "Error el dispositiu %s no exesteix\n", token);
            }

        }else if (strcmp(token, "send") == 0){

        }else if (strcmp(token, "quit") == 0){
            kill(pids[0], SIGKILL);
            wait(NULL);
            exit(0);
        }else{
            fprintf(stderr, "La comanda %s no es acceptada\n", token);
        }

}
