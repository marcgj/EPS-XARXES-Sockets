//
// Created by Marc Gaspà Joval on 11/3/22.
//

#ifndef SO_PRACTICA1_CONEXIONS_H
#define SO_PRACTICA1_CONEXIONS_H

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "cfgloader.h"

#define REG_REQ 0xa0
#define REG_ACK 0xa1
#define REG_NACK 0xa2
#define REG_REJ 0xa3
#define REG_INFO 0xa4
#define INFO_ACK 0xa5
#define INFO_NACK 0xa6
#define INFO_REJ 0xa7

#define DISCONNECTED 0xf0
#define NOT_REGISTERED 0xf1
#define WAIT_ACK_REG 0xf2
#define WAIT_INFO 0xf3
#define WAIT_ACK_INFO 0xf4
#define REGISTERED 0xf5
#define SEND_ALIVE 0xf6

#define ALIVE 0xb0
#define ALIVE_NACK 0xb1
#define ALIVE_REJ 0xb2

#define ZERO_COMM_ID "0000000000"


void process_args(int argc, char *argv[], char *cfgFileName);
int configure_udp(int port);
struct sockaddr_in sockaddr_in_generator(char * address, int port);
int reg_procedure(int sock, struct sockaddr_in addr_server);
void register_client();

int send_wait_ALIVE(int t, int sock);
void start_alive_service();

typedef struct{
    unsigned char type;
    char tx_id[11];
    char comm_id[11];
    char data[61];
}PDU;

PDU generate_PDU(unsigned char type, char tx_id[11], char comm_id[11], char data[61]);
void print_PDU(PDU pdu, char * pretext);

typedef struct {
    char tx_id[11];
    char comm_id[11];
    int udp_port;
    int tcp_port;
} rcv_info;

#endif //SO_PRACTICA1_CONEXIONS_H
