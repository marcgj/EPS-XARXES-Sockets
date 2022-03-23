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

#include "utilitats/cfgloader.h"
#include "utilitats/conexions.h"

#define DEFAULT_CFG "../client.cfg"


int debug = 0;
unsigned char status = DISCONNECTED;

rcv_info srv_info;

int main(int argc, char *argv[]){
    char cfgFileName[16] = DEFAULT_CFG;

    process_args(argc, argv, cfgFileName);

    ClientCfg cfg;
    load_config(cfgFileName, &cfg);
    if (debug) print_config(&cfg);

    struct sockaddr_in addr_server;

    int udpSock = configure_udp(cfg.address,cfg.server_UDP, &addr_server);

    reg_procedure(udpSock, &addr_server, &cfg);
}


void process_args(int argc, char **argv, char * cfgFileName) {
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
                    perror("Parametre no reconegut\n");
                    exit(1);
                    break;
            }
        }
        if (debug) printf("Config File Selected: %s\n", cfgFileName);
    }

}

int configure_udp(char * address, int port, struct sockaddr_in *addr_server) {
    if (debug) printf("\nObrint socket UDP amb adreça %s i port %i\n", address, port);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(sock < 0){
        perror("Error creant el socket");
        exit(1);
    }

    struct hostent * ent = gethostbyname(address);

    if (!ent){
        printf("Error! No trobat: %s \n", address);
        exit(-1);
    }

    addr_server->sin_family = AF_INET;
    addr_server->sin_port = htons(port);
    addr_server->sin_addr.s_addr = (((struct in_addr *) ent -> h_addr) -> s_addr);

    if(bind(sock,(struct sockaddr*) &addr_server, sizeof(*addr_server)) < 0){
        perror("Error bind");
        exit(-1);
    }

    return sock;
}

int reg_procedure(int sock, struct sockaddr_in *addr_server, ClientCfg *cfg) {
    if (debug) printf("Inici proces de registre\n");

    int const t = 1, u = 2, n = 8, o = 3, p = 2, q = 4;
    int packets = 1, procedures = 0;

    fd_set fileDesctiptors;
    struct timeval tv;

    int sock2;
    struct sockaddr_in addr_server2 = *addr_server;

    PDU reg_req_pkt;
    reg_req_pkt.type = REG_REQ;
    strcpy(reg_req_pkt.tx_id, cfg->id);

    tv.tv_sec = t;
    tv.tv_usec = 0;

    while (status != REGISTERED){
        FD_ZERO(&fileDesctiptors);

        if(procedures > o){
            perror("No s'ha pogut conectar amb el servidor");
            exit(1);
        }

        if (packets > p && tv.tv_sec < q*t) tv.tv_sec += t;
        if (packets > n){
            status = NOT_REGISTERED;
            packets = 1;
            tv.tv_sec = t;
            procedures++;
            sleep(u);
        }

        switch (status) {
            case NOT_REGISTERED:
                sendto(sock, &reg_req_pkt, sizeof(reg_req_pkt), 0,
                       (struct sockaddr*) &addr_server, sizeof(*addr_server));
                status = WAIT_ACK_REG;
                break;

            case WAIT_ACK_REG:
                select(sock + 1, &fileDesctiptors, NULL, NULL, &tv);
                if (FD_ISSET(sock, &fileDesctiptors)){
                    PDU rcv_pkt;
                    recvfrom(sock, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*) &srv_info.server_addr,
                             (socklen_t *) &srv_info.addr_long);

                    switch (rcv_pkt.type) {
                        case REG_ACK:
                            srv_info.udp_port = (int) strtol(rcv_pkt.data, NULL, 10);
                            strcpy(srv_info.comm_id, rcv_pkt.comm_id);
                            strcpy(srv_info.tx_id, rcv_pkt.tx_id);

                            char adress[INET_ADDRSTRLEN];

                            inet_ntop(AF_INET, &srv_info.server_addr.sin_addr, adress, sizeof(adress));

                            sock2 = configure_udp(adress,srv_info.udp_port, &addr_server2);

                            PDU reg_info_pkt;
                            reg_info_pkt.type = REG_INFO;
                            strcpy(reg_info_pkt.tx_id, cfg->id);
                            strcpy(reg_info_pkt.comm_id, srv_info.comm_id);

                            //TODO posar el camp de dades amb el port tcp i els dispositius
                            char data[61];


                            sendto(sock2, &reg_req_pkt, sizeof(reg_req_pkt), 0,
                                   (struct sockaddr*) &addr_server, sizeof(*addr_server));

                            status = WAIT_ACK_INFO;
                            break;

                        case REG_NACK:
                            status = NOT_REGISTERED;
                            break;

                        case REG_REJ:
                            status = NOT_REGISTERED;
                            packets = 1;
                            tv.tv_sec = t;
                            procedures++;
                            break;

                        default:
                                // TODO Preguntar al profe
                            break;
                    }
                }else{
                    sendto(sock, &reg_req_pkt, sizeof(reg_req_pkt), 0,
                           (struct sockaddr*) &addr_server, sizeof(*addr_server));
                    packets++;
                }
                break;

            case WAIT_ACK_INFO:
                tv.tv_sec = 2 * t;
                select(sock2 + 1, &fileDesctiptors, NULL, NULL, &tv);
                if (FD_ISSET(sock2, &fileDesctiptors)){
                    PDU rcv_pkt;
                    rcv_info info;
                    recvfrom(sock2, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*) &info.server_addr,
                             (socklen_t *) &info.addr_long);

                    if (!strcmp(rcv_pkt.tx_id, srv_info.tx_id) && !strcmp(rcv_pkt.comm_id, srv_info.comm_id)){
                        // TODO falta comparar les adreces i preguntar que pasa si no coicideixen
                    }

                    switch (rcv_pkt.type) {
                        case INFO_ACK:
                            srv_info.tcp_port = (int) strtol(rcv_pkt.data, NULL, 10);
                            status = REGISTERED;
                            break;

                        case INFO_NACK:
                            status = NOT_REGISTERED;
                            break;

                        default:

                            break;
                    }
                }else{
                    status = NOT_REGISTERED;
                    packets = 1;
                    tv.tv_sec = t;
                    procedures++;
                }
                    // Si reb un ack estat a registrat i guardar port tcp guardat a dades del paquet

                    // Si rep un nack el camp dades ha de contenir el motiu i sha de tornar a comencar
                break;

            default:
                break;
        }
    }


    return 0;
}




