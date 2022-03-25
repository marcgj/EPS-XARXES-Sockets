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
#include "utilitats/elemcontroller.h"

#define DEFAULT_CFG "../client.cfg"


int debug = 0;
unsigned char status = NOT_REGISTERED;

ClientCfg cfg;
rcv_info srv_info;
int end = 0;

int udpSocket;
int tcpSock;

void sigterm(int s) {
    if (s == SIGTERM){
        kill(0, SIGUSR1);
        while (wait(NULL) != -1) printf("Fill tancat\n");
        exit(0);
    }
}
//send LUM-0-O

int main(int argc, char *argv[]) {
    signal(SIGTERM, sigterm);
    char cfgFileName[16] = DEFAULT_CFG;

    handle_args(argc, argv, cfgFileName);

    load_config(cfgFileName, &cfg);

    if (debug) print_config(&cfg);

    udpSocket = configure_udp(cfg.local_TCP);
    fd_set fileDesctiptors;
    struct timeval tv = {0, 0};


    const int v = 2, r = 2;
    while (!end) {
        switch (status) {
            case NOT_REGISTERED:
                register_client(udpSocket);
                if (send_wait_ALIVE(udpSocket, v * r) == 0) {
                    start_alive_service(udpSocket, v);
                    status = SEND_ALIVE;

                    tcpSock = configure_tcp(cfg.local_TCP);
                    if (listen(tcpSock, 16) < 0) {
                        perror("Error listen");
                        status = NOT_REGISTERED;
                    }
                } else status = NOT_REGISTERED;
                break;
            case SEND_ALIVE:
                FD_ZERO(&fileDesctiptors);
                FD_SET(0, &fileDesctiptors);
                FD_SET(tcpSock, &fileDesctiptors);
                int i = select(tcpSock + 1, &fileDesctiptors, NULL, NULL, &tv);
                if (FD_ISSET(0, &fileDesctiptors) && i > 0) {
                    handle_terminal(tcpSock);
                } else if (FD_ISSET(tcpSock, &fileDesctiptors) && i > 0) {
                    handle_incoming_connection(tcpSock);
                }
                break;

            default:
                break;
        }

    }
    wait(NULL);
    wait(NULL);
}


void handle_args(int argc, char **argv, char *cfgFileName) {
    if (argc > 1) {
        for (int i = 1; i < argc;) {
            if (argv[i][0] != '-' || strlen(argv[i]) > 2) {
                perror("Mal us dels arguments");
                kill(0, SIGTERM);
            }
            switch (argv[i][1]) {
                case 'd':
                    debug = 1;
                    printf("MODE DEBUG ACTIVAT\n");
                    i++;
                    break;
                case 'c':
                    strcpy(cfgFileName, argv[i + 1]);
                    i += 2;
                    break;

                default:
                    printf("Parametre no reconegut");
                    kill(0, SIGTERM);
            }
        }
        if (debug) printf("Config File Selected: %s\n", cfgFileName);
    }

}






