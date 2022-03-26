//
// Created by Marc Gaspà Joval on 25/3/22.
//

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "headers/alive.h"
#include "headers/pdu.h"
#include "headers/globals.h"
#include "headers/conexions.h"
#include "headers/socket.h"

int pid_alive = 0;

void start_alive_service(int sock, int t) {
    signal(SIGUSR1, sig_usr);

    if (pid_alive != 0) kill(pid_alive, SIGTERM);

    int parent_pid = getpid();
    int pid = fork();
    if (pid == 0) {
        const int s = 3;
        int missing_alives = 0;

        while (missing_alives < s) {
            int code = send_wait_ALIVE(sock, t);
            if (code == -1) missing_alives++;
            else if (code == -2) break;
            else {
                missing_alives = 0;
                sleep(t);
            }
        }

        kill(parent_pid, SIGUSR1);
        exit(1);
    } else if (pid < 0) {
        perror("Error creant process alives");
        exit(1);
    }
    pid_alive = pid;
}

int send_wait_ALIVE(int sock, int t) {
    PDU_UDP alive_pkt = generate_PDU_UDP(ALIVE, cfg.id, srv_info.comm_id, "");

    PDU_UDP rcv_pkt;
    const struct sockaddr_in addr_srv = sockaddr_in_generator(cfg.address, cfg.server_UDP);

    fd_set fileDesctiptors;
    struct timeval tv = {t, 0};

    if (sendto(sock, &alive_pkt, sizeof(alive_pkt), 0,
               (struct sockaddr *) &addr_srv, sizeof(addr_srv)) < 0) {
        perror("Error send ALIVE");
    } else if (debug) print_PDU_UDP(alive_pkt, "ENVIAT ALIVE");

    FD_SET(sock, &fileDesctiptors);
    select(sock + 1, &fileDesctiptors, NULL, NULL, &tv);
    if (FD_ISSET(sock, &fileDesctiptors)) {
        recvfrom(sock, &rcv_pkt, sizeof(rcv_pkt), 0, NULL, NULL);
        if (!is_valid_pkg(&rcv_pkt, srv_info)) {
            print_PDU_UDP(rcv_pkt, "REBUT PAQUET ERRONI");
            return -2;
        }

        switch (rcv_pkt.type) {
            case ALIVE:
                if (debug) print_PDU_UDP(rcv_pkt, "REBUT ALIVE");
                if (strcmp(rcv_pkt.data, cfg.id) != 0) {
                    if (debug) printf("ERROR: ALIVE REBUT NO CONTE LA ID\n");
                    return -2;
                } else return 0;

            case ALIVE_REJ:
                if (debug) print_PDU_UDP(rcv_pkt, "REBUT ALIVE_REJ");
                return -2;
        }
    }
    if (debug) fprintf(stderr, "NO s'ha rebut l'ALIVE\n");
    return -1;
}
