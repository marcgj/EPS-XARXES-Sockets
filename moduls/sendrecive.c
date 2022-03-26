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

void send_element(Element elem) {
    signal(SIGUSR1, sig_usr);

    int parent_pid = getpid();

    int pid = fork();
    if (pid == 0) {
        const int m = 3;
        const struct sockaddr_in addr_srv = sockaddr_in_generator(cfg.address, srv_info.tcp_port);
        int sock = configure_tcp(cfg.local_TCP + 1);

        if (connect(sock, (struct sockaddr *) &addr_srv, sizeof(addr_srv)) < 0) {
            //TODO mirar si aquest es el comportament
            perror("Error connect");
            exit(1);
        } else {
            if (debug) printf("Conexio establida amb el servidor via TCP\n");
        }

        PDU_TCP send_data_pkg = generate_PDU_TCP_Elem(SEND_DATA, cfg.id, srv_info.comm_id, elem);

        if (send(sock, &send_data_pkg, sizeof(send_data_pkg), 0) < 0) {
            perror("Error SEND_DATA");
            exit(1);
        } else {
            if (debug) print_PDU_TCP(send_data_pkg, "ENVIAT SEND_DATA");
        }

        fd_set fDesc;
        FD_ZERO(&fDesc);
        struct timeval tv = {m, 0};
        PDU_TCP rcv_pkt;

        FD_SET(sock, &fDesc);
        select(sock + 1, &fDesc, NULL, NULL, &tv);
        if (FD_ISSET(sock, &fDesc)) {
            recv(sock, &rcv_pkt, sizeof(rcv_pkt), 0);
            if (!is_valid_pkg(&rcv_pkt, srv_info) ||
                strcmp(rcv_pkt.element, send_data_pkg.element) != 0) {
                if (debug) print_PDU_TCP(rcv_pkt, "REBUT PAQUET ERRONI");
                if (debug) printf("ERROR: DATA_ACK REBUT CAMP ELEMENT ERRONI\n");
                kill(parent_pid, SIGUSR1);
                exit(0);
            }

            switch (rcv_pkt.type) {
                case DATA_ACK:
                    if (debug) print_PDU_TCP(rcv_pkt, "REBUT DATA_ACK");
                    if (strcmp(rcv_pkt.info, cfg.id) != 0) {
                        if (debug) printf("ERROR: DATA_ACK REBUT NO CONTE LA ID\n");
                        kill(parent_pid, SIGUSR1);
                    }
                    break;

                case DATA_NACK:
                    if (debug) print_PDU_TCP(rcv_pkt, "REBUT DATA_NACK");
                    break;

                case DATA_REJ:
                    if (debug) print_PDU_TCP(rcv_pkt, "REBUT DATA_REJ");
                    kill(parent_pid, SIGUSR1);
                    break;
            }
        } else {
            fprintf(stderr, "No s'ha rebut resposta al SEND_DATA\n");
        }
        close(sock);
        exit(0);
    } else if (pid < 0) {
        perror("Error creant process SEND_DATA");
        exit(1);
    }
}

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
            strcmp(rcv_pkt.info, cfg.id) != 0) {
            //TODO preguntar quin es el comportament del test 18

            if (debug) print_PDU_TCP(rcv_pkt, "REBUT PAQUET ERRONI");
            printf("REBUT PAQUET AMB TX_ID O COMM_ID ERRONIS\n");

            PDU_TCP data_rej_pkt =
                    generate_PDU_TCP(DATA_REJ, cfg.id, srv_info.comm_id, rcv_pkt.element,
                                     rcv_pkt.value, "Error paquet amb dades erronies");

            if (send(sock2, &data_rej_pkt, sizeof(data_rej_pkt), 0) < 0) {
                perror("Error DATA_REJ");
                exit(1);
            } else {
                if (debug) print_PDU_TCP(data_rej_pkt, "ENVIAT DATA_REJ");
                status = NOT_REGISTERED;
                return;
            }
        }

        PDU_TCP data_nack_pkt = generate_PDU_TCP(DATA_NACK, cfg.id, srv_info.comm_id, rcv_pkt.element,
                                                 rcv_pkt.value, "Error paquet amb dades erronies");

        Element *elem = getElement(&cfg, rcv_pkt.element);

        if (elem != NULL) {
            switch (rcv_pkt.type) {
                case SET_DATA:
                    if (debug) print_PDU_TCP(rcv_pkt, "REBUT SET_DATA");

                    if (rcv_pkt.element[6] == 'I') {
                        strcpy(elem->value, rcv_pkt.value);
                        PDU_TCP data_ack = generate_PDU_TCP_Elem(DATA_ACK, cfg.id, srv_info.comm_id, *elem);
                        if (send(sock2, &data_ack, sizeof(data_ack), 0) < 0) {
                            perror("Error DATA_ACK");
                            exit(1);
                        } else {
                            if (debug) print_PDU_TCP(data_ack, "ENVIAT DATA_ACK");
                        }
                        return;
                    }
                    strcpy(data_nack_pkt.info, "Error nomes es pot fer set a elements tipus I");
                    break;

                case GET_DATA:
                    if (debug) print_PDU_TCP(rcv_pkt, "REBUT GET_DATA");

                    if (rcv_pkt.element[6] == 'O') {
                        PDU_TCP data_ack = generate_PDU_TCP_Elem(DATA_ACK, cfg.id, srv_info.comm_id, *elem);
                        if (send(sock2, &data_ack, sizeof(data_ack), 0) < 0) {
                            perror("Error DATA_ACK");
                            exit(1);
                        } else {
                            if (debug) print_PDU_TCP(data_ack, "ENVIAT DATA_ACK");
                        }
                        return;
                    }
                    strcpy(data_nack_pkt.info, "Error nomes es pot fer get a elements tipus O");
                    break;
            }
        }

        if (send(sock2, &data_nack_pkt, sizeof(data_nack_pkt), 0) < 0) {
            perror("Error DATA_NACK");
            exit(1);
        } else {
            if (debug) print_PDU_TCP(data_nack_pkt, "ENVIAT DATA_NACK");
            return;
        }

    } else {
        fprintf(stderr, "No s'ha rebut cap paquet\n");
    }
}
