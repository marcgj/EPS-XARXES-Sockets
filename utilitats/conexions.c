//
// Created by Marc Gaspà Joval on 11/3/22.
//
#include "conexions.h"

extern int debug;
extern unsigned char status;
extern ClientCfg cfg;
extern rcv_info srv_info;
extern int pids[3];

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

void register_client(int udpSock) {
    const int o = 3;
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

    PDU_UDP reg_req_pkt = generate_PDU_UDP(REG_REQ, cfg.id, ZERO_COMM_ID, "");

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
                    PDU_UDP rcv_pkt;
                    recvfrom(sock, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *) &addr_rcv, NULL);

                    switch (rcv_pkt.type) {
                        case REG_ACK:
                            if (debug) print_PDU(rcv_pkt, "REBUT REQ_ACK");

                            srv_info.udp_port = (int) strtol(rcv_pkt.data, NULL, 10);
                            strcpy(srv_info.comm_id, rcv_pkt.comm_id);
                            strcpy(srv_info.tx_id, rcv_pkt.tx_id);

                            addr_server2.sin_port = htons(srv_info.udp_port);

                            PDU_UDP reg_info_pkt = generate_PDU_UDP(REG_INFO, cfg.id, srv_info.comm_id, "");
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
                    PDU_UDP rcv_pkt;
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

void print_PDU(PDU_UDP pdu, char * pretext) {
    printf("%s // \t TYPE= %i \t TX_ID= %s\t COMM_ID= %s\t DATA= %s\n", pretext, pdu.type ,pdu.tx_id, pdu.comm_id, pdu.data);
}

PDU_UDP generate_PDU_UDP(unsigned char type, char tx_id[11], char comm_id[11], char data[61]) {
    PDU_UDP pdu;
    pdu.type = type;
    strcpy(pdu.tx_id, tx_id);
    strcpy(pdu.comm_id, comm_id);
    strcpy(pdu.data, data);

    return pdu;
}

void sig_usr(int signo){
    if(signo == SIGUSR1){
        status = NOT_REGISTERED;
    }
}

void start_alive_service(int sock, int t) {
    signal(SIGUSR1,sig_usr);
    int parent_pid = getpid();
    int pid = fork();

    if(pid == 0){
        const int s = 3;
        int missing_alives = 0;

        while (missing_alives < s){
            sleep(t);
            int code = send_wait_ALIVE(sock, t);
            if (code == -1) missing_alives++;
            else if (code == -2) break;
            else missing_alives = 0;
        }

        close(sock);
        kill(parent_pid, SIGUSR1);
        exit(1);
    }else if(pid < 0){
        perror("Error creant process alives");
        exit(1);
    }
    pids[0] = pid;
}

int send_wait_ALIVE(int sock, int t){
    const PDU_UDP alive_pkt = generate_PDU_UDP(ALIVE, cfg.id, srv_info.comm_id, "");

    PDU_UDP rcv_pkt;
    const struct sockaddr_in addr_srv = sockaddr_in_generator(cfg.address, cfg.server_UDP);

    fd_set fileDesctiptors;
    struct timeval tv = {t, 0};

    if (sendto(sock, &alive_pkt, sizeof(alive_pkt), 0,
               (struct sockaddr*) &addr_srv, sizeof(addr_srv)) < 0)
    {
        perror("Error send ALIVE");
    }else{
        if (debug) print_PDU(alive_pkt, "ENVIAT ALIVE");
    }

    FD_SET(sock, &fileDesctiptors);
    select(sock + 1, &fileDesctiptors, NULL, NULL, &tv);
    if (FD_ISSET(sock, &fileDesctiptors)) {
        recvfrom(sock, &rcv_pkt, sizeof(rcv_pkt), 0, NULL, NULL);
        if (strcmp(rcv_pkt.tx_id, srv_info.tx_id) != 0 ||
            strcmp(rcv_pkt.comm_id, srv_info.comm_id) != 0) {
            print_PDU(rcv_pkt, "REBUT PAQUET ERRONI");
            return -2;
        }

        switch (rcv_pkt.type) {
            case ALIVE:
                if (debug) print_PDU(rcv_pkt, "REBUT ALIVE");
                if (strcmp(rcv_pkt.data, cfg.id) != 0) {
                    if (debug) printf("ERROR: ALIVE REBUT NO CONTE LA ID\n");
                    return -2;
                }else return 0;

            case ALIVE_REJ:
                if (debug) print_PDU(rcv_pkt, "REBUT ALIVE_REJ");
                return -2;
        }
    }else{
        if (debug) fprintf(stderr, "NO s'ha rebut l'ALIVE\n");
        return -1;
    }
}




