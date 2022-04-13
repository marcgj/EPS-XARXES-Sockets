#ifndef XARXES_PRACTICA1_ALIVE_H
#define XARXES_PRACTICA1_ALIVE_H

#include <signal.h>

void start_alive_service(int sock, int t);

int send_wait_ALIVE(int sock, int t);

#endif //XARXES_PRACTICA1_ALIVE_H
