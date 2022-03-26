//
// Created by Marc Gaspà Joval on 11/3/22.
//

#ifndef SO_PRACTICA1_CONEXIONS_H
#define SO_PRACTICA1_CONEXIONS_H

typedef struct {
    char tx_id[11];
    char comm_id[11];
    int udp_port;
    int tcp_port;
} ConnexionInfo;


int is_valid_pkg(void *pkg, ConnexionInfo info);

void sig_usr(int signo);

#endif //SO_PRACTICA1_CONEXIONS_H
