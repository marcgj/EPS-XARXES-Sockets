#ifndef XARXES_PRACTICA1_REGISTER_H
#define XARXES_PRACTICA1_REGISTER_H

#include <netinet/in.h>


void register_client(int udpSock);

int reg_procedure(int sock, struct sockaddr_in addr_server);


#endif //XARXES_PRACTICA1_REGISTER_H
