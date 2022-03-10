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

#include "cfgloader.h"

#define DEFAULT_CFG "../client.cfg"
#define REG_REQ '0xa0'
#define REG_ACK '0xa1'
#define REG_NACK '0xa2'
#define REG_REJ '0xa3'
#define REG_INFO '0xa4'
#define INFO_ACK '0xa5'
#define INFO_NACK '0xa6'
#define INFO_REJ '0xa7'

#define DISCONNECTED '0xf0'
#define NOT_REGISTERED '0xf1'
#define WAIT_ACK_REG '0xf2'
#define WAIT_INFO '0xf3'
#define WAIT_ACK_INFO '0xf4'
#define REGISTERED '0xf5'
#define SEND_ALIVE '0xf6'

int debug = 0;
unsigned char estat = DISCONNECTED;


void process_args(int argc, char *argv[], char *cfgFileName);
int configure_udp(ClientCfg cfg, struct sockaddr_in *addr_server);
int reg_procedure(int sock, struct sockaddr_in *addr_server, ClientCfg cfg);
typedef struct{
    unsigned char type;
    char tx_id[11];
    char rcv_id[11];
    char data[61];
}PDU;


int main(int argc, char *argv[]){
    char cfgFileName[16] = DEFAULT_CFG;

    process_args(argc, argv, cfgFileName);

    ClientCfg cfg;
    load_config(cfgFileName, &cfg);
    if (debug) print_config(&cfg);

    struct sockaddr_in addr_server;
    int udpSock = configure_udp(cfg, &addr_server);

    reg_procedure(udpSock, &addr_server, cfg);



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
            }
        }
        if (debug) printf("Config File Selected: %s\n", cfgFileName);
    }

}

int configure_udp(ClientCfg cfg, struct sockaddr_in *addr_server) {
    struct hostent *ent;

    ent = gethostbyname(cfg.address);
    if (!ent){
        printf("Error! No trobat: %s \n", cfg.address);
        exit(-1);
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0){
        perror("Error creant el socket");
        exit(-1);
    }


    addr_server->sin_family = AF_INET;
    addr_server->sin_port = htons(cfg.udpPort);
    addr_server->sin_addr.s_addr = (((struct in_addr *) ent -> h_addr) -> s_addr);

    if(bind(sock,(struct sockaddr*) &addr_server, sizeof(*addr_server)) < 0){
        perror("Error bind");
        exit(-1);
    }

    return sock;
}

int reg_procedure(int sock, struct sockaddr_in *addr_server, ClientCfg cfg) {
    PDU pkt;
    pkt.type = REG_REQ;
    strcpy(pkt.tx_id, cfg.id);

    sendto(sock, &pkt, sizeof(pkt), 0,(struct sockaddr*)&addr_server, sizeof(*addr_server));

    return 0;
}




