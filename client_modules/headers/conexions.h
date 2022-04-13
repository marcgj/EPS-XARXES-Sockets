#ifndef SO_PRACTICA1_CONEXIONS_H
#define SO_PRACTICA1_CONEXIONS_H


#include <netinet/in.h>

typedef struct {
    char tx_id[11];
    char comm_id[11];
    int udp_port;
    int tcp_port;
    struct sockaddr_in addr;
} ConnexionInfo;

int is_same_addr(struct sockaddr_in addr1, struct sockaddr_in addr2);

int is_valid_pkg(void *pkg, ConnexionInfo info);

void sig_usr(int signo);

#endif //SO_PRACTICA1_CONEXIONS_H
