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
#include "headers/terminal.h"

int pid_alive = 0;

void start_alive_service(int sock, int t) {
    signal(SIGUSR1, sig_usr);

    if (pid_alive != 0) kill(pid_alive, SIGTERM);

    int parent_pid = getpid();
    int pid = fork();
    if (pid == 0) {
        print_debug("Iniciat servei ALIVES\n");
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

        print_error("No s'han rebut %i ALIVES consecutius tancant servei\n", s);
        kill(parent_pid, SIGUSR1);
        exit(1);
    } else if (pid < 0) {
        print_error("Error creant process alives");
        perror("");
        exit(1);
    }
    pid_alive = pid;
}

int send_wait_ALIVE(int sock, int t) {
    PDU_UDP alive_pkt = generate_PDU_UDP(ALIVE, cfg.id, srv_info.comm_id, "");

    PDU_UDP rcv_pkt;
    struct sockaddr_in addr_srv = sockaddr_in_generator(cfg.address, cfg.server_UDP);

    fd_set fileDesctiptors;
    struct timeval tv = {t, 0};

    send_pdu_UDP(sock, alive_pkt, addr_srv, " ENVIAT ALIVE");

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
                    print_error("El ALIVE rebut no conte la id esperada al camp dades\n");
                    return -2;
                } else return 0;

            case ALIVE_REJ:
                print_PDU_UDP(rcv_pkt, "REBUT ALIVE_REJ");
                return -2;
        }
    }
    print_error("NO s'ha rebut resposta a l'ALIVE en %is\n", t);
    return -1;
}
