//
// Created by Marc Gaspà Joval on 2/3/22.
//

#include <sys/socket.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#include "modules/headers/config.h"
#include "modules/headers/globals.h"
#include "modules/headers/socket.h"
#include "modules/headers/register.h"
#include "modules/headers/alive.h"
#include "modules/headers/terminal.h"
#include "modules/headers/sendrecive.h"

#define DEFAULT_CFG "client.cfg"

// TODO preguntar que pasa amb el t3, i si s-ha denviar algo

// get GHXE2LWQ6C LUM-0-O
// set GHXE2LWQ6C LUM-0-I test
// set LUM-0-O test

void handle_args(int argc, char *argv[], char *cfgFileName);

void sigterm(int s);

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
                    print_debug("Estat actual= SEND_ALIVE\n");

                    tcpSock = configure_tcp(cfg.local_TCP);
                    if (listen(tcpSock, 16) < 0) {
                        print_error("Error al fer listen al port TCP");
                        perror("");
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
}


void handle_args(int argc, char **argv, char *cfgFileName) {
    if (argc > 1) {
        for (int i = 1; i < argc;) {
            if (argv[i][0] != '-' || strlen(argv[i]) > 2) {
                print_error("Mal us dels arguments");
                kill(0, SIGTERM);
            }
            switch (argv[i][1]) {
                case 'd':
                    debug = 1;
                    print_debug("MODE DEBUG ACTIVAT\n");
                    i++;
                    break;
                case 'c':
                    strcpy(cfgFileName, argv[i + 1]);
                    i += 2;
                    break;

                default:
                    print_error("Parametre no reconegut");
                    kill(0, SIGTERM);
            }
        }
    }

}

void sigterm(int s) {
    if (s == SIGTERM) {
        kill(0, SIGUSR1);
        int pid;
        while ((pid = wait(NULL)) != -1) print_debug("Fill amb pid=%i tancat\n", pid);
        exit(0);
    }
}


