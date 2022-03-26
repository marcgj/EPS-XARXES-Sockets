//
// Created by Marc Gaspà Joval on 25/3/22.
//

#include <time.h>
#include <printf.h>
#include <string.h>

#include "headers/pdu.h"

void print_PDU_UDP(PDU_UDP pdu, char *pretext) {
    printf("%s // \t TYPE= %i \t TX_ID= %s\t COMM_ID= %s\t DATA= %s\n", pretext, pdu.type, pdu.tx_id, pdu.comm_id,
           pdu.data);
}

void print_PDU_TCP(PDU_TCP pdu, char *pretext) {
    printf("%s // \t TYPE= %i \t TX_ID= %s\t COMM_ID= %s\t ELEM= %s\t VALUE= %s\t DATA= %s\n",
           pretext, pdu.type, pdu.tx_id, pdu.comm_id, pdu.element, pdu.value, pdu.info);
}

PDU_UDP generate_PDU_UDP(unsigned char type, char *tx_id, char *comm_id, char *data) {
    PDU_UDP pdu;
    pdu.type = type;
    strcpy(pdu.tx_id, tx_id);
    strcpy(pdu.comm_id, comm_id);
    strcpy(pdu.data, data);

    return pdu;
}

PDU_TCP generate_PDU_TCP(unsigned char type, char *tx_id, char *comm_id, char *element, char *value, char *info) {
    PDU_TCP pdu;

    pdu.type = type;
    strcpy(pdu.tx_id, tx_id);
    strcpy(pdu.comm_id, comm_id);
    strcpy(pdu.element, element);
    strcpy(pdu.value, value);
    strcpy(pdu.info, info);

    return pdu;
}

PDU_TCP generate_PDU_TCP_Elem(unsigned char type, char *tx_id, char *comm_id, Element elem) {
    time_t t1;
    struct tm *t2;
    char info_buffer[128];
    time(&t1);
    t2 = localtime(&t1);
    strftime(info_buffer, 128, "%Y-%m-%d;%H:%M:%S", t2);

    return generate_PDU_TCP(type, tx_id, comm_id, elem.elem_string, elem.value, info_buffer);
}
