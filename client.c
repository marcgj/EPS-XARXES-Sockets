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

#include "client_modules/headers/config.h"
#include "client_modules/headers/globals.h"
#include "client_modules/headers/socket.h"
#include "client_modules/headers/register.h"
#include "client_modules/headers/alive.h"
#include "client_modules/headers/terminal.h"
#include "client_modules/headers/sendrecive.h"

#define DEFAULT_CFG "client.cfg"

void handle_args(int argc, char *argv[], char *cfgFileName);

int main(int argc, char *argv[]) {
    char cfgFileName[16] = DEFAULT_CFG;

    handle_args(argc, argv, cfgFileName);

    load_config(cfgFileName, &cfg);

    print_config(&cfg);

    // Obrim el port udp encarregat de iniciar el registre i dur a terme els alives
    udpSocket = configure_udp(cfg.local_TCP);

    fd_set fileDesctiptors;
    const int v = 2, r = 2;

    while (!end) {
        switch (status) {
            /*
             * En el cas de que el disposotiu no estigui registrat el registra, envia el primer alive, inicia el servei
             * dels alives i obre el port tcp i el posa en mode escolta
            */
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

            /*
             * Quan ja esta en SEND_ALIVE espera o a be una conexio tcp entrant o a una entrada del usuari per terminal
             * en cada cas ho gestiona
             */
            case SEND_ALIVE:
                FD_ZERO(&fileDesctiptors);
                FD_SET(0, &fileDesctiptors);
                FD_SET(tcpSock, &fileDesctiptors);
                int i = select(tcpSock + 1, &fileDesctiptors, NULL, NULL, 0);
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

// S'encarrega de procesar els arguments
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


