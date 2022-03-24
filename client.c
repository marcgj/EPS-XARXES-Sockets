//
// Created by Marc Gaspà Joval on 2/3/22.
//

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>


#include "utilitats/cfgloader.h"
#include "utilitats/conexions.h"

#define DEFAULT_CFG "../client.cfg"


int debug = 0;
unsigned char status = NOT_REGISTERED;

int pids[3];

void print_elements(int elemc, Element *elements) {
    printf("   Parametres \t Valors\n");
    printf("   ----------- \t ------------------\n");
    for (int i = 0; i < elemc; ++i) {
        Element elem = elements[i];
        printf("   %s \t\t %s\n", elem.elem_string, elem.value);
    }
}

ClientCfg cfg;
rcv_info srv_info;
int end = 0;

int main(int argc, char *argv[]){
    char cfgFileName[16] = DEFAULT_CFG;

    handle_args(argc, argv, cfgFileName);

    load_config(cfgFileName, &cfg);
    if (debug) print_config(&cfg);

    int udpSock = configure_udp(cfg.local_TCP);
    fd_set fileDesctiptors;

    const int v = 2, r = 2;
    while(!end){
        switch (status) {
            case NOT_REGISTERED:
                register_client(udpSock);
                if(send_wait_ALIVE(udpSock, v*r) == 0) {
                    start_alive_service(udpSock, v);
                    status = SEND_ALIVE;
                }
                else status = NOT_REGISTERED;
                break;
            case SEND_ALIVE:
                FD_SET(STDIN_FILENO, &fileDesctiptors);
                select(STDIN_FILENO + 1, &fileDesctiptors, NULL, NULL, NULL);
                if (FD_ISSET(STDIN_FILENO, &fileDesctiptors)){
                    handle_terminal();
                    FD_CLR(STDIN_FILENO, &fileDesctiptors);
                }
                break;

            default:
                break;
        }

    }
    wait(NULL);
    wait(NULL);
}


void handle_args(int argc, char **argv, char * cfgFileName) {
    if(argc > 1){
        for (int i = 1; i < argc;) {
            if (argv[i][0] != '-' || strlen(argv[i]) > 2){
                perror("Mal us dels arguments");
                exit(1);
            }
            switch (argv [i][1]) {
                case 'd':
                    debug = 1;
                    printf("MODE DEBUG ACTIVAT\n");
                    i++;
                    break;
                case 'c':
                    strcpy(cfgFileName, argv[i+1]);
                    i += 2;
                    break;

                default:
                    printf("Parametre no reconegut");
                    exit(1);
            }
        }
        if (debug) printf("Config File Selected: %s\n", cfgFileName);
    }

}






