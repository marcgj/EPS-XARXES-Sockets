//
// Created by Marc Gaspà Joval on 25/3/22.
//


#ifndef XARXES_PRACTICA1_GLOBALS_H
#define XARXES_PRACTICA1_GLOBALS_H

#include "config.h"
#include "conexions.h"

// TODO preguntar per les macros que no es fan anar

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

#define SEND_DATA 0xc0
#define DATA_ACK 0xc1
#define DATA_NACK 0xc2
#define DATA_REJ 0xc3
#define SET_DATA 0xc4
#define GET_DATA 0xc5

#define ZERO_COMM_ID "0000000000"

int debug;
unsigned char status;
ClientCfg cfg;
ConnexionInfo srv_info;
int udpSocket;
int tcpSock;
int end;

#endif //XARXES_PRACTICA1_GLOBALS_H
