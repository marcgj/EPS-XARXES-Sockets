//
// Created by Marc Gaspà Joval on 11/3/22.
//
#include <sys/signal.h>
#include <unistd.h>
#include <string.h>
#include <printf.h>

#include "headers/conexions.h"
#include "headers/globals.h"
#include "headers/pdu.h"


void sig_usr(int signo) {
    if (signo == SIGUSR1) {
        status = NOT_REGISTERED;
        close(tcpSock);
    }
}

int is_valid_pkg(void *pkg, ConnexionInfo info) {
    PDU_TCP *pkg2 = (PDU_TCP *) pkg;
    if (strcmp(pkg2->tx_id, info.tx_id) != 0 ||
        strcmp(pkg2->comm_id, info.comm_id) != 0)
        return 0;
    return 1;
}

// TODO preguntar si comprovar les adreces aixi pot suposar algun problema
int is_same_addr(struct sockaddr_in addr1, struct sockaddr_in addr2) {
    if (addr1.sin_addr.s_addr == addr2.sin_addr.s_addr) return 1;
    printf("%d %d\n", addr1.sin_addr.s_addr, addr2.sin_addr.s_addr);
    return 0;
}





