#ifndef XARXES_PRACTICA1_SOCKET_H
#define XARXES_PRACTICA1_SOCKET_H

#include <netdb.h>

struct sockaddr_in sockaddr_in_generator(char *address, int port);

int configure_udp(int port);

int configure_tcp(int port);

#endif //XARXES_PRACTICA1_SOCKET_H
