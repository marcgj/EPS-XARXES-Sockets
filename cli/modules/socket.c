//
// Created by Marc Gaspà Joval on 25/3/22.
//
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/globals.h"
#include "headers/socket.h"
#include "headers/terminal.h"

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

int configure_socket(int port, int type) {
    struct sockaddr_in addr;
    int sock = socket(AF_INET, type, 0);

    if (sock < 0) {
        print_error("Error creant el socket");
        perror("");
        exit(1);
    }

    const int true = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(true));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        print_error("Error bind UDP");
        perror("");
        kill(0, SIGTERM);
    }
    return sock;
}

int configure_udp(int port) {
    print_message("Obrint socket UDP amb port %i\n", port);
    return configure_socket(port, SOCK_DGRAM);
}

int configure_tcp(int port) {
    print_message("Obrint socket TCP amb port %i\n", port);
    return configure_socket(port, SOCK_STREAM);
}
