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

    int udpSock = configure_udp(cfg.local_TCP);

    struct sockaddr_in addr_server = sockaddr_in_generator(cfg.address, cfg.server_UDP);

    reg_procedure(udpSock, addr_server, &cfg);
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
                    printf("Parametre no reconegut");
                    exit(1);
            }
        }
        if (debug) printf("Config File Selected: %s\n", cfgFileName);
    }

}

int configure_udp(int port) {
    if (debug) printf("\nObrint socket UDP amb port %i\n", port);
    struct sockaddr_in addr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(sock < 0){
        perror("Error creant el socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0){
        perror("Error bind");
        exit(-1);
    }

    return sock;
}

int reg_procedure(int sock, struct sockaddr_in addr_server, ClientCfg *cfg) {

    int const t = 1, u = 2, n = 8, o = 3, p = 2, q = 4;
    int packets = 1, procedures = 0;

    fd_set fileDesctiptors;
    struct timeval tv;

    int sock2;
    struct sockaddr_in addr_server2 = addr_server;

    PDU reg_req_pkt;
    reg_req_pkt.type = REG_REQ;
    strcpy(reg_req_pkt.comm_id, "0000000000");
    strcpy(reg_req_pkt.tx_id, cfg->id);

    tv.tv_sec = t;
    tv.tv_usec = 0;

    if (debug) printf("Inici proces de registre\n");
    status = NOT_REGISTERED;
    while (status != REGISTERED){
        if (debug) printf("\nPackets enviats = %i, Procesos de registre = %i | Temps entre paquets = %i\n", packets, procedures, (int) tv.tv_sec);

        FD_ZERO(&fileDesctiptors);

        if(procedures > o){
            printf("No s'ha pogut conectar amb el servidor\n");
            exit(1);
        }

        if (packets > p && tv.tv_sec < q*t) tv.tv_sec += t;
        if (packets > n){
            status = NOT_REGISTERED;
            packets = 1;
            tv.tv_sec = t;
            procedures++;

            if (debug) printf("Començant nou proces de registre (%i), i esperant %is\n", procedures, u);

            sleep(u);
        }

        switch (status) {
            case NOT_REGISTERED:
                if (debug) printf("Status -> NOT_REGISTERED\n");

                if ( sendto(sock, &reg_req_pkt, sizeof(reg_req_pkt), 0,
                       (struct sockaddr*) &addr_server, sizeof(addr_server)) < 0){
                    perror("Error send REG_REQ1");
                }else{
                    if (debug) printf("Paquet enviat -> REG_REQ\n");
                }

                status = WAIT_ACK_REG;
                break;

            case WAIT_ACK_REG:
                if (debug) printf("Status -> WAIT_ACK_REG\n");

                FD_SET(sock, &fileDesctiptors);
                select(sock + 1, &fileDesctiptors, NULL, NULL, &tv);
                if (FD_ISSET(sock, &fileDesctiptors)){
                    PDU rcv_pkt;
                    recvfrom(sock, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*) &srv_info.server_addr,
                             (socklen_t *) &srv_info.addr_long);

                    switch (rcv_pkt.type) {
                        case REG_ACK:
                            if (debug) printf("Paquet rebut -> REG_ACK\n");
                            srv_info.udp_port = (int) strtol(rcv_pkt.data, NULL, 10);
                            strcpy(srv_info.comm_id, rcv_pkt.comm_id);
                            strcpy(srv_info.tx_id, rcv_pkt.tx_id);

                            addr_server2.sin_port = htons(srv_info.udp_port);

                            PDU reg_info_pkt;
                            reg_info_pkt.type = REG_INFO;
                            strcpy(reg_info_pkt.tx_id, cfg->id);
                            strcpy(reg_info_pkt.comm_id, srv_info.comm_id);

                            char data_temp[61];
                            char elements[32];
                            elements_to_string(elements, cfg->elemc, cfg->elements);
                            sprintf(data_temp, "%i,%s", cfg->local_TCP, elements);

                            strcpy(reg_info_pkt.data, data_temp);
                            if (debug) printf("Enviant paquet REG_INFO comm_id: %s, dades: %s\n", reg_info_pkt.comm_id, reg_info_pkt.data);

                            sendto(sock, &reg_info_pkt, sizeof(reg_info_pkt), 0,
                                   (struct sockaddr*) &addr_server2, sizeof(addr_server2));

                            if (debug) printf("Paquet enviat -> REG_INFO\n");

                            status = WAIT_ACK_INFO;
                            break;

                        case REG_NACK:
                            if (debug) printf("Paquet rebut -> REG_NACK\n");
                            status = NOT_REGISTERED;
                            break;

                        case REG_REJ:
                            if (debug) printf("Paquet rebut -> REG_REJ\n");
                            status = NOT_REGISTERED;
                            packets = 1;
                            tv.tv_sec = t;
                            procedures++;
                            if (debug) printf("Començant nou proces de registre (%i)\n", procedures, u);

                            break;

                        default:
                            if (debug) printf("Paquet rebut -> DESCONEGUT\n");
                            break;
                    }
                }else{
                    if (sendto(sock, &reg_req_pkt, sizeof(reg_req_pkt), 0,
                                (struct sockaddr*) &addr_server, sizeof(addr_server)) < 0){
                        perror("Error send REG_REQ1");
                    }else{
                        if (debug) printf("Paquet enviat -> REG_REQ\n");
                    }
                    packets++;
                }
                break;

            case WAIT_ACK_INFO:
                if (debug) printf("Status -> WAIT_ACK_INFO\n");
                tv.tv_sec = 2 * t;

                FD_SET(sock, &fileDesctiptors);
                select(sock + 1, &fileDesctiptors, NULL, NULL, &tv);
                if (FD_ISSET(sock, &fileDesctiptors)){
                    PDU rcv_pkt;
                    rcv_info info;
                    recvfrom(sock, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*) &info.server_addr,
                             (socklen_t *) &info.addr_long);

                    if (!strcmp(rcv_pkt.tx_id, srv_info.tx_id) && !strcmp(rcv_pkt.comm_id, srv_info.comm_id)){
                        // TODO falta comparar les adreces i preguntar que pasa si no coicideixen
                        if (debug) printf("PAQUET REBUT ERRONI: comm_id o adreça no esperades\n");
                        continue;
                    }

                    switch (rcv_pkt.type) {
                        case INFO_ACK:
                            if (debug) printf("Paquet rebut -> INFO_ACK\n");
                            srv_info.tcp_port = (int) strtol(rcv_pkt.data, NULL, 10);
                            status = REGISTERED;
                            break;

                        case INFO_NACK:
                            if (debug) printf("Paquet rebut -> INFO_NACK\n");
                            status = NOT_REGISTERED;
                            break;

                        default:
                            if (debug) printf("Paquet rebut -> DESCONEGUT\n");
                            break;
                    }
                }else {
                    status = NOT_REGISTERED;
                    packets = 1;
                    tv.tv_sec = t;
                    procedures++;
                }
                break;

            default:
                break;
        }
    }


    return 0;
}

struct sockaddr_in sockaddr_in_generator(char *address, int port) {
    struct sockaddr_in result;

    struct hostent * ent = gethostbyname(address);

    if (!ent){
        printf("Error! No trobat: %s \n", address);
        exit(-1);
    }

    result.sin_family = AF_INET;
    result.sin_port = htons(port);
    result.sin_addr.s_addr = (((struct in_addr *) ent -> h_addr) -> s_addr);

    return result;
}




