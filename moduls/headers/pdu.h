//
// Created by Marc Gaspà Joval on 25/3/22.
//

#ifndef XARXES_PRACTICA1_PDU_H
#define XARXES_PRACTICA1_PDU_H

#include "config.h"


typedef struct {
    unsigned char type;
    char tx_id[11];
    char comm_id[11];
    char data[61];
} PDU_UDP;

typedef struct {
    unsigned char type;
    char tx_id[11];
    char comm_id[11];
    char element[8];
    char value[16];
    char info[80];
} PDU_TCP;


void print_PDU_UDP(PDU_UDP pdu, char *pretext);

void print_PDU_TCP(PDU_TCP pdu, char *pretext);

PDU_UDP generate_PDU_UDP(unsigned char type, char *tx_id, char *comm_id, char *data);

PDU_TCP generate_PDU_TCP(unsigned char type, char *tx_id, char *comm_id, char *element, char *value, char *info);

PDU_TCP generate_PDU_TCP_Elem(unsigned char type, char *tx_id, char *comm_id, Element elem);

#endif //XARXES_PRACTICA1_PDU_H
