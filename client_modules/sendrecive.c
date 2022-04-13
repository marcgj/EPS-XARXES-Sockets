//
// Created by Marc Gaspà Joval on 25/3/22.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "headers/conexions.h"
#include "headers/sendrecive.h"
#include "headers/socket.h"
#include "headers/pdu.h"
#include "headers/globals.h"
#include "headers/terminal.h"

/*
 * S'utilitza per mirar i el element que has rebut es el mateix que has enviat en el send
 */
int is_valid_element(PDU_TCP inpkt, PDU_TCP outpkg) {
    if (strcmp(inpkt.element, outpkg.element) != 0) return 0;
    return 1;
}

int id_in_info(PDU_TCP inpkt, char *id) {
    if (strcmp(inpkt.info, id) != 0) return 0;
    return 1;
}

/*
 * Gestiona tot el proces de enviar un element i el seu valor al servidor
 * Ho fa en un proces a part per a tal de que el pare no es quedi bloquejat durant aquest
 */
void send_element(Element elem) {
    signal(SIGUSR1, sig_usr);

    int parent_pid = getpid();

    int pid = fork();
    if (pid == 0) {
        print_debug("Inici proces enviament de dades\n");
        const int m = 3;
        const struct sockaddr_in addr_srv = sockaddr_in_generator(cfg.address, srv_info.tcp_port);
        int sock = configure_tcp(cfg.local_TCP + 1);

        if (connect(sock, (struct sockaddr *) &addr_srv, sizeof(addr_srv)) < 0) {
            print_error("Ha fallat el connect del enviament de dades");
            perror("");
            exit(1);
        }
        print_message("Conexio establida amb el servidor via TCP\n");

        PDU_TCP rcv_pkt, send_data_pkg = generate_PDU_TCP_Elem(SEND_DATA, cfg.id, srv_info.comm_id, elem);

        send_pdu_TCP(sock, send_data_pkg, "SEND_DATA");

        fd_set fDesc;
        FD_ZERO(&fDesc);
        struct timeval tv = {m, 0};

        FD_SET(sock, &fDesc);
        select(sock + 1, &fDesc, NULL, NULL, &tv);
        if (FD_ISSET(sock, &fDesc)) {
            recv(sock, &rcv_pkt, sizeof(rcv_pkt), 0);
            if (!is_valid_pkg(&rcv_pkt, srv_info) ||
                !is_valid_element(rcv_pkt, send_data_pkg)) {
                print_PDU_TCP(rcv_pkt, "REBUT PAQUET ERRONI");
                print_error("S'ha rebut un paquet amb dades erronies\n");
                kill(parent_pid, SIGUSR1);
                exit(0);
            }

            switch (rcv_pkt.type) {
                case DATA_ACK:
                    if (debug) print_PDU_TCP(rcv_pkt, "REBUT DATA_ACK");
                    if (strcmp(rcv_pkt.info, cfg.id) != 0) {
                        perror("El DATA_ACK rebut conte dades erronies\n");
                        kill(parent_pid, SIGUSR1);
                    }
                    break;

                case DATA_NACK:
                    print_PDU_TCP(rcv_pkt, "REBUT DATA_NACK");
                    print_error("Rebut DATA_NACK\n");
                    break;

                case DATA_REJ:
                    print_PDU_TCP(rcv_pkt, "REBUT DATA_REJ");
                    print_error("Rebut DATA_REJ\n");
                    kill(parent_pid, SIGUSR1);
                    break;
            }
        } else {
            print_error("No s'ha rebut resposta al SEND_DATA\n");
        }
        close(sock);
        exit(0);
    } else if (pid < 0) {
        print_error("Error creant process SEND_DATA");
        perror("");
        exit(1);
    }
}

/*
 * Funcio encarregada de gestionar els GET&SET_DATA que es poden rebre
 */
void handle_incoming_connection(int sock) {
    int sock2 = accept(sock, NULL, NULL);

    fd_set fDesc;
    PDU_TCP rcv_pkt;

    FD_ZERO(&fDesc);
    FD_SET(sock2, &fDesc);
    select(sock2 + 1, &fDesc, NULL, NULL, 0);
    if (FD_ISSET(sock2, &fDesc)) {
        recv(sock2, &rcv_pkt, sizeof(rcv_pkt), 0);
        if (!is_valid_pkg(&rcv_pkt, srv_info) ||
            !id_in_info(rcv_pkt, cfg.id)) {
            //TODO preguntar quin es el comportament del test 18

            print_PDU_TCP(rcv_pkt, "REBUT PAQUET ERRONI");
            print_error("Rebut paquet amb tx_id o comm_id erronis\n");

            PDU_TCP data_rej_pkt =
                    generate_PDU_TCP(DATA_REJ, cfg.id, srv_info.comm_id, rcv_pkt.element,
                                     rcv_pkt.value, "Error paquet amb dades erronies");

            send_pdu_TCP(sock2, data_rej_pkt, "DATA_REJ");
            status = NOT_REGISTERED;
            return;
        }

        PDU_TCP data_nack_pkt = generate_PDU_TCP(DATA_NACK, cfg.id, srv_info.comm_id, rcv_pkt.element,
                                                 rcv_pkt.value, "Error paquet amb dades erronies");

        Element *elem = getElement(&cfg, rcv_pkt.element);

        if (elem != NULL) {
            switch (rcv_pkt.type) {
                case SET_DATA:
                    print_PDU_TCP(rcv_pkt, "REBUT SET_DATA");
                    if (rcv_pkt.element[6] == 'I') {
                        strcpy(elem->value, rcv_pkt.value);
                        PDU_TCP data_ack = generate_PDU_TCP_Elem(DATA_ACK, cfg.id, srv_info.comm_id, *elem);
                        send_pdu_TCP(sock2, data_ack, "ENVIAT DATA_ACK");
                        return;
                    }
                    strcpy(data_nack_pkt.info, "Error nomes es pot fer set a elements tipus I");
                    break;

                case GET_DATA:
                    print_PDU_TCP(rcv_pkt, "REBUT GET_DATA");
                    PDU_TCP data_ack = generate_PDU_TCP_Elem(DATA_ACK, cfg.id, srv_info.comm_id, *elem);
                    send_pdu_TCP(sock2, data_ack, "ENVIAT DATA_ACK");
                    return;
            }
        }

        send_pdu_TCP(sock2, data_nack_pkt, "DATA_NACK");
        return;
    } else {
        perror("No s'ha rebut cap paquet\n");
    }
}
