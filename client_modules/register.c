#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "headers/conexions.h"
#include "headers/register.h"
#include "headers/socket.h"
#include "headers/globals.h"
#include "headers/pdu.h"
#include "headers/terminal.h"

/*
 * Funcio principal per al registre del dispositiu
 */
void register_client(int udpSock) {
    const int o = 3;
    int procedures = 1;
    struct sockaddr_in addr_server = sockaddr_in_generator(cfg.address, cfg.server_UDP);

    while (procedures <= o) {
        print_message("Iniciant proces de registre (%i)\n", procedures);
        if (reg_procedure(udpSock, addr_server) == 1) {
            return;
        }
        ++procedures;
    }
    print_error("No s'ha pogut conectar amb el servidor\n");
    exit(1);
}

/*
 * Aquesta funcio porta a terme un procediment de registre, en el cas de que s'hagi completat amb exit retorna 0, en
 * cas d'error retorna -1
 */
int reg_procedure(int sock, struct sockaddr_in addr_server) {
    int const t = 1, u = 2, n = 8, p = 2, q = 4;
    int packets = 1;
    status = NOT_REGISTERED;

    fd_set fileDesctiptors;
    struct timeval tv = {t, 0};

    // addr_server2 nomes canvia en el port i es fa anar quan es continua el registre per un segon port upd
    struct sockaddr_in addr_rcv, addr_server2 = addr_server;
    socklen_t a_len = sizeof(addr_rcv);


    PDU_UDP reg_req_pkt = generate_PDU_UDP(REG_REQ, cfg.id, ZERO_COMM_ID, "");

    while (status != REGISTERED) {
        FD_ZERO(&fileDesctiptors);
        if (packets > p && tv.tv_sec < q * t) tv.tv_sec += t;
        if (packets > n) {
            sleep(u);
            return -1;
        }

        switch (status) {
            case NOT_REGISTERED:
                print_debug("Estat actual= NOT_REGISTERED\n");

                send_pdu_UDP(sock, reg_req_pkt, addr_server, "REG_REQ");

                status = WAIT_ACK_REG;
                break;

            case WAIT_ACK_REG:
                print_debug("Estat actual= WAIT_ACK_REG\n");

                FD_SET(sock, &fileDesctiptors);
                select(sock + 1, &fileDesctiptors, NULL, NULL, &tv);
                if (FD_ISSET(sock, &fileDesctiptors)) {
                    PDU_UDP rcv_pkt;
                    recvfrom(sock, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *) &addr_rcv, &a_len);

                    switch (rcv_pkt.type) {
                        case REG_ACK:
                            print_PDU_UDP(rcv_pkt, "REBUT REQ_ACK");
                            srv_info.udp_port = (int) strtol(rcv_pkt.data, NULL, 10);
                            srv_info.addr = addr_rcv;
                            strcpy(srv_info.comm_id, rcv_pkt.comm_id);
                            strcpy(srv_info.tx_id, rcv_pkt.tx_id);

                            addr_server2.sin_port = htons(srv_info.udp_port);

                            PDU_UDP reg_info_pkt = generate_PDU_UDP(REG_INFO, cfg.id, srv_info.comm_id, "");
                            sprintf(reg_info_pkt.data, "%i,%s", cfg.local_TCP, cfg.elements_string);

                            send_pdu_UDP(sock, reg_info_pkt, addr_server2, "ENVIAT REG_INFO");

                            status = WAIT_ACK_INFO;
                            break;

                        case REG_NACK:
                            print_PDU_UDP(rcv_pkt, "REBUT REG_NACK");
                            status = NOT_REGISTERED;
                            break;

                        case REG_REJ:
                            print_PDU_UDP(rcv_pkt, "REBUT REG_REJ");
                            return -1;

                        default:
                            print_PDU_UDP(rcv_pkt, "REBUT DESCONEGUT");
                            return -1;
                    }
                } else {
                    send_pdu_UDP(sock, reg_req_pkt, addr_server, "ENVIAT REG_REQ");
                    packets++;
                }
                break;

            case WAIT_ACK_INFO:
                print_debug("Estat actual= WAIT_ACK_INFO\n");
                tv.tv_sec *= 2;

                FD_SET(sock, &fileDesctiptors);
                select(sock + 1, &fileDesctiptors, NULL, NULL, &tv);
                if (FD_ISSET(sock, &fileDesctiptors)) {
                    PDU_UDP rcv_pkt;
                    recvfrom(sock, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *) &addr_rcv, NULL);

                    if (!is_valid_pkg(&rcv_pkt, srv_info) || !is_same_addr(srv_info.addr, addr_rcv)) {
                        print_PDU_UDP(rcv_pkt, "REBUT PAQUET ERRONI");
                        return -1;
                    }

                    switch (rcv_pkt.type) {
                        case INFO_ACK:
                            print_PDU_UDP(rcv_pkt, "REBUT INFO_ACK");
                            srv_info.tcp_port = (int) strtol(rcv_pkt.data, NULL, 10);
                            print_message("Registre completat amb exit\n");

                            status = REGISTERED;
                            return 1;

                        case INFO_NACK:
                            print_PDU_UDP(rcv_pkt, "REBUT INFO_NACK");
                            status = NOT_REGISTERED;
                            break;

                        default:
                            print_PDU_UDP(rcv_pkt, "REBUT DESCONEGUT");
                            return -1;
                    }
                }
                return -1;

            default:
                //do nothing
                break;
        }
    }
    return 0;
}
