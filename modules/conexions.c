//
// Created by Marc Gaspà Joval on 11/3/22.
//
#include <sys/signal.h>
#include <unistd.h>
#include <string.h>

#include "headers/conexions.h"
#include "globals.h"
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


//TODO comprovar a tot arreu on faci falta que la ip del servidor tmb es la correcta


