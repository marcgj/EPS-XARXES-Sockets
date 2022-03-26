//
// Created by Marc Gaspà Joval on 25/3/22.
//
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/globals.h"
#include "headers/socket.h"

struct sockaddr_in sockaddr_in_generator(char *address, int port) {
    struct sockaddr_in result;

    struct hostent *ent = gethostbyname(address);

    if (!ent) {
        printf("Error! No trobat: %s \n", address);
        kill(0, SIGTERM);
    }

    result.sin_family = AF_INET;
    result.sin_port = htons(port);
    result.sin_addr.s_addr = (((struct in_addr *) ent->h_addr)->s_addr);

    return result;
}

int configure_udp(int port) {
    if (debug) printf("\nObrint socket UDP amb port %i\n", port);
    struct sockaddr_in addr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        perror("Error creant el socket");
        kill(0, SIGTERM);
    }

    const int true = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(true));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("Error bind UDP");
        kill(0, SIGTERM);
    }

    return sock;
}

int configure_tcp(int port) {
    if (debug) printf("\nObrint socket TCP amb port %i\n", port);

    struct sockaddr_in addr;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creant el socket");
        kill(0, SIGTERM);
    }
    const int true = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(true));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror("Error bind TCP");
        exit(1);
    }

    return sock;
}
