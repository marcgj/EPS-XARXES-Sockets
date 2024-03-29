#define _POSIX_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include "headers/alive.h"
#include "headers/pdu.h"
#include "headers/globals.h"
#include "headers/conexions.h"
#include "headers/socket.h"
#include "headers/terminal.h"

int pid_alive = 0;

/*
 * Crea un nou proces encarregat de enviar i rebre els alives, i portarne el control
 */
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

        print_error("Tancant servei ALIVEs\n");
        kill(parent_pid, SIGUSR1);
        exit(1);
    } else if (pid < 0) {
        print_error("Error creant process alives");
        perror("");
        exit(1);
    }
    pid_alive = pid;
}

/*
 * Simplement envia un alive i espera t segons, si es reb una resposta correcta retorna 0,
 * si no reb resposta retorna -1
 * i si hi ha discrepancia en les dades o be es reb un _REJ retorna -2
 */
int send_wait_ALIVE(int sock, int t) {
    PDU_UDP alive_pkt = generate_PDU_UDP(ALIVE, cfg.id, srv_info.comm_id, "");

    PDU_UDP rcv_pkt;
    struct sockaddr_in addr_rcv, addr_srv = sockaddr_in_generator(cfg.address, cfg.server_UDP);
    socklen_t a_len = sizeof(addr_rcv);

    fd_set fileDesctiptors;
    struct timeval tv = {t, 0};

    send_pdu_UDP(sock, alive_pkt, addr_srv, "ENVIAT ALIVE");

    FD_SET(sock, &fileDesctiptors);
    select(sock + 1, &fileDesctiptors, NULL, NULL, &tv);
    if (FD_ISSET(sock, &fileDesctiptors)) {
        recvfrom(sock, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *) &addr_rcv, &a_len);
        if (!is_valid_pkg(&rcv_pkt, srv_info) || !is_same_addr(srv_info.addr, addr_rcv)) {
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
    print_debug("NO s'ha rebut resposta a l'ALIVE en %is\n", t);
    return -1;
}
