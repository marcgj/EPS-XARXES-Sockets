#include <sys/signal.h>
#include <unistd.h>
#include <string.h>

#include "headers/conexions.h"
#include "headers/globals.h"
#include "headers/pdu.h"

/*
 * Farem anar el sig unser per a canviar de estat el dispositiu a NOT_REGISTERED desde els diferents procesos fills
 */
void sig_usr(int signo) {
    if (signo == SIGUSR1) {
        status = NOT_REGISTERED;
        close(tcpSock);
    }
}

/*
 * Comprova si un paquet es correcte
 * En cas que no retorna 0
 */
int is_valid_pkg(void *pkg, ConnexionInfo info) {
    PDU_TCP *pkg2 = (PDU_TCP *) pkg;
    if (strcmp(pkg2->tx_id, info.tx_id) != 0 ||
        strcmp(pkg2->comm_id, info.comm_id) != 0)
        return 0;
    return 1;
}

/*
 * Comprova si dues sockaddr_in son identiques
 */
int is_same_addr(struct sockaddr_in addr1, struct sockaddr_in addr2) {
    if (addr1.sin_addr.s_addr == addr2.sin_addr.s_addr) return 1;
    return 0;
}





