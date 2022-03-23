//
// Created by Marc Gaspà Joval on 11/3/22.
//
#include "conexions.h"

extern int debug;
extern unsigned char status;
extern ClientCfg cfg;
extern rcv_info srv_info;

int configure_udp(int port) {
    if (debug) printf("\nObrint socket UDP amb port %i\n", port);
    struct sockaddr_in addr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(sock < 0){
        perror("Error creant el socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0){
        perror("Error bind");
        exit(-1);
    }

    return sock;
}

void register_client() {
    const int o = 3;
    int udpSock = configure_udp(cfg.local_TCP);
    struct sockaddr_in addr_server = sockaddr_in_generator(cfg.address, cfg.server_UDP);

    int procedures = 0;
    while(procedures < o){
        if(reg_procedure(udpSock, addr_server) == 1){
            return;
        }
        if (debug) printf("Començant nou proces de registre (%i)\n", ++procedures);
    }
    printf("No s'ha pogut conectar amb el servidor\n");
    exit(1);
}

int reg_procedure(int sock, struct sockaddr_in addr_server) {
    int const t = 1, u = 2, n = 8, p = 2, q = 4;
    int packets = 1;

    fd_set fileDesctiptors;
    struct timeval tv;

    struct sockaddr_in addr_rcv;
    struct sockaddr_in addr_server2 = addr_server;

    PDU reg_req_pkt;
    reg_req_pkt.type = REG_REQ;
    strcpy(reg_req_pkt.comm_id, "0000000000");
    strcpy(reg_req_pkt.tx_id, cfg.id);

    tv.tv_sec = t;
    tv.tv_usec = 0;

    if (debug) printf("\nInici proces de registre\n");
    if (debug) printf("/////////////////////////////////////////////////////////////////////////////////////////\n");
    status = NOT_REGISTERED;
    while (status != REGISTERED){
        FD_ZERO(&fileDesctiptors);
        if (packets > p && tv.tv_sec < q*t) tv.tv_sec += t;
        if (packets > n){
            sleep(u);
            return -1;
        }

        switch (status) {
            case NOT_REGISTERED:
                if (debug) printf("Status -> NOT_REGISTERED\n");

                if ( sendto(sock, &reg_req_pkt, sizeof(reg_req_pkt), 0,
                            (struct sockaddr*) &addr_server, sizeof(addr_server)) < 0){
                    perror("Error send REG_REQ1");
                }else{
                    if (debug) print_PDU(reg_req_pkt, "ENVIAT REG_REQ");
                }

                status = WAIT_ACK_REG;
                break;

            case WAIT_ACK_REG:
                if (debug) printf("Status -> WAIT_ACK_REG\n");

                FD_SET(sock, &fileDesctiptors);
                select(sock + 1, &fileDesctiptors, NULL, NULL, &tv);
                if (FD_ISSET(sock, &fileDesctiptors)){
                    PDU rcv_pkt;
                    recvfrom(sock, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *) &addr_rcv, NULL);

                    switch (rcv_pkt.type) {
                        case REG_ACK:
                            if (debug) print_PDU(rcv_pkt, "REBUT REQ_ACK");

                            srv_info.udp_port = (int) strtol(rcv_pkt.data, NULL, 10);
                            strcpy(srv_info.comm_id, rcv_pkt.comm_id);
                            strcpy(srv_info.tx_id, rcv_pkt.tx_id);

                            PDU reg_info_pkt;
                            reg_info_pkt.type = REG_INFO;
                            addr_server2.sin_port = htons(srv_info.udp_port);
                            strcpy(reg_info_pkt.tx_id, cfg.id);
                            strcpy(reg_info_pkt.comm_id, srv_info.comm_id);
                            sprintf(reg_info_pkt.data, "%i,%s", cfg.local_TCP, cfg.elements_string);

                            if (sendto(sock, &reg_info_pkt, sizeof(reg_info_pkt), 0,
                                       (struct sockaddr*) &addr_server2, sizeof(addr_server2)) < 0){
                                perror("Error send REG_INFO");
                            }else{
                                if (debug) print_PDU(reg_info_pkt, "ENVIAT REG_INFO");
                            }

                            status = WAIT_ACK_INFO;
                            break;

                        case REG_NACK:
                            if (debug) print_PDU(rcv_pkt, "REBUT REG_NACK");
                            status = NOT_REGISTERED;
                            break;

                        case REG_REJ:
                            if (debug) print_PDU(rcv_pkt, "REBUT REG_REJ");
                            return -1;

                        default:
                            if (debug) print_PDU(rcv_pkt, "REBUT DESCONEGUT");
                            return -1;
                    }
                }else{
                    if (sendto(sock, &reg_req_pkt, sizeof(reg_req_pkt), 0,
                               (struct sockaddr*) &addr_server, sizeof(addr_server)) < 0){
                        perror("Error send REG_REQ1");
                    }else{
                        if (debug) print_PDU(reg_req_pkt, "ENVIAT REG_REQ2");
                    }
                    packets++;
                }
                break;

            case WAIT_ACK_INFO:
                if (debug) printf("Status -> WAIT_ACK_INFO\n");
                tv.tv_sec = 2 * t;

                FD_SET(sock, &fileDesctiptors);
                select(sock + 1, &fileDesctiptors, NULL, NULL, &tv);
                if (FD_ISSET(sock, &fileDesctiptors)){
                    PDU rcv_pkt;
                    recvfrom(sock, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr*) &addr_rcv,
                             NULL);

                    if (strcmp(rcv_pkt.tx_id, srv_info.tx_id) != 0 || strcmp(rcv_pkt.comm_id, srv_info.comm_id) != 0){
                        if (debug) print_PDU(rcv_pkt, "REBUT PAQUET ERRONI");
                        return -1;
                    }

                    switch (rcv_pkt.type) {
                        case INFO_ACK:
                            if (debug) print_PDU(rcv_pkt, "REBUT INFO_ACK");
                            srv_info.tcp_port = (int) strtol(rcv_pkt.data, NULL, 10);

                            if (debug) printf("\nREGISTRE COMPLETAT AMB EXIT\n");

                            status = REGISTERED;
                            return 1;

                        case INFO_NACK:
                            if (debug) print_PDU(rcv_pkt, "REBUT INFO_NACK");
                            status = NOT_REGISTERED;
                            break;

                        default:
                            if (debug) print_PDU(rcv_pkt, "REBUT DESCONEGUT");
                            return -1;
                    }
                }else {
                    return -1;
                }
                break;

            default:
                break;

        }
        if (debug) printf("/////////////////////////////////////////////////////////////////////////////////////////\n");
    }
    return 0;
}

struct sockaddr_in sockaddr_in_generator(char *address, int port) {
    struct sockaddr_in result;

    struct hostent * ent = gethostbyname(address);

    if (!ent){
        printf("Error! No trobat: %s \n", address);
        exit(-1);
    }

    result.sin_family = AF_INET;
    result.sin_port = htons(port);
    result.sin_addr.s_addr = (((struct in_addr *) ent -> h_addr) -> s_addr);

    return result;
}

void print_PDU(PDU pdu, char * pretext) {
    printf("%s // \t TYPE= %i \t TX_ID= %s\t COMM_ID= %s\t DATA= %s\n", pretext, pdu.type ,pdu.tx_id, pdu.comm_id, pdu.data);
}
