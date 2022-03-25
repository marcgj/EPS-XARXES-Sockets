//
// Created by Marc Gaspà Joval on 11/3/22.
//
#include <wait.h>
#include "conexions.h"

extern int debug;
extern unsigned char status;
extern ClientCfg cfg;
extern rcv_info srv_info;

extern int udpSocket;
extern int tcpSock;

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

void register_client(int udpSock) {
    const int o = 3;
    struct sockaddr_in addr_server = sockaddr_in_generator(cfg.address, cfg.server_UDP);

    int procedures = 0;
    while (procedures < o) {
        if (reg_procedure(udpSock, addr_server) == 1) {
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
    while (status != REGISTERED) {
        FD_ZERO(&fileDesctiptors);
        if (packets > p && tv.tv_sec < q * t) tv.tv_sec += t;
        if (packets > n) {
            sleep(u);
            return -1;
        }

        switch (status) {
            case NOT_REGISTERED:
                if (debug) printf("Status -> NOT_REGISTERED\n");

                if (sendto(sock, &reg_req_pkt, sizeof(reg_req_pkt), 0,
                           (struct sockaddr *) &addr_server, sizeof(addr_server)) < 0) {
                    perror("Error send REG_REQ1");
                } else {
                    if (debug) print_PDU_UDP(reg_req_pkt, "ENVIAT REG_REQ");
                }
                status = WAIT_ACK_REG;
                break;

            case WAIT_ACK_REG:
                if (debug) printf("Status -> WAIT_ACK_REG\n");

                FD_SET(sock, &fileDesctiptors);
                select(sock + 1, &fileDesctiptors, NULL, NULL, &tv);
                if (FD_ISSET(sock, &fileDesctiptors)) {
                    PDU_UDP rcv_pkt;
                    recvfrom(sock, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *) &addr_rcv, NULL);

                    switch (rcv_pkt.type) {
                        case REG_ACK:
                            if (debug) print_PDU_UDP(rcv_pkt, "REBUT REQ_ACK");

                            srv_info.udp_port = (int) strtol(rcv_pkt.data, NULL, 10);
                            strcpy(srv_info.comm_id, rcv_pkt.comm_id);
                            strcpy(srv_info.tx_id, rcv_pkt.tx_id);

                            addr_server2.sin_port = htons(srv_info.udp_port);

                            PDU_UDP reg_info_pkt = generate_PDU_UDP(REG_INFO, cfg.id, srv_info.comm_id, "");
                            sprintf(reg_info_pkt.data, "%i,%s", cfg.local_TCP, cfg.elements_string);

                            if (sendto(sock, &reg_info_pkt, sizeof(reg_info_pkt), 0,
                                       (struct sockaddr *) &addr_server2, sizeof(addr_server2)) < 0) {
                                perror("Error send REG_INFO");
                            } else {
                                if (debug) print_PDU_UDP(reg_info_pkt, "ENVIAT REG_INFO");
                            }
                            status = WAIT_ACK_INFO;
                            break;

                        case REG_NACK:
                            if (debug) print_PDU_UDP(rcv_pkt, "REBUT REG_NACK");
                            status = NOT_REGISTERED;
                            break;

                        case REG_REJ:
                            if (debug) print_PDU_UDP(rcv_pkt, "REBUT REG_REJ");
                            return -1;

                        default:
                            if (debug) print_PDU_UDP(rcv_pkt, "REBUT DESCONEGUT");
                            return -1;
                    }
                } else {
                    if (sendto(sock, &reg_req_pkt, sizeof(reg_req_pkt), 0,
                               (struct sockaddr *) &addr_server, sizeof(addr_server)) < 0) {
                        perror("Error send REG_REQ1");
                    } else {
                        if (debug) print_PDU_UDP(reg_req_pkt, "ENVIAT REG_REQ2");
                    }
                    packets++;
                }
                break;

            case WAIT_ACK_INFO:
                if (debug) printf("Status -> WAIT_ACK_INFO\n");
                tv.tv_sec = 2 * t;

                FD_SET(sock, &fileDesctiptors);
                select(sock + 1, &fileDesctiptors, NULL, NULL, &tv);
                if (FD_ISSET(sock, &fileDesctiptors)) {
                    PDU_UDP rcv_pkt;
                    recvfrom(sock, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *) &addr_rcv,
                             NULL);

                    if (strcmp(rcv_pkt.tx_id, srv_info.tx_id) != 0 || strcmp(rcv_pkt.comm_id, srv_info.comm_id) != 0) {
                        if (debug) print_PDU_UDP(rcv_pkt, "REBUT PAQUET ERRONI");
                        return -1;
                    }

                    switch (rcv_pkt.type) {
                        case INFO_ACK:
                            if (debug) print_PDU_UDP(rcv_pkt, "REBUT INFO_ACK");
                            srv_info.tcp_port = (int) strtol(rcv_pkt.data, NULL, 10);

                            if (debug) printf("\nREGISTRE COMPLETAT AMB EXIT\n");

                            status = REGISTERED;
                            return 1;

                        case INFO_NACK:
                            if (debug) print_PDU_UDP(rcv_pkt, "REBUT INFO_NACK");
                            status = NOT_REGISTERED;
                            break;

                        default:
                            if (debug) print_PDU_UDP(rcv_pkt, "REBUT DESCONEGUT");
                            return -1;
                    }
                } else {
                    return -1;
                }
                break;

            default:
                break;
        }
        if (debug)
            printf("/////////////////////////////////////////////////////////////////////////////////////////\n");
    }
    return 0;
}

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

void print_PDU_UDP(PDU_UDP pdu, char *pretext) {
    printf("%s // \t TYPE= %i \t TX_ID= %s\t COMM_ID= %s\t DATA= %s\n", pretext, pdu.type, pdu.tx_id, pdu.comm_id,
           pdu.data);
}

void print_PDU_TCP(PDU_TCP pdu, char *pretext) {
    printf("%s // \t TYPE= %i \t TX_ID= %s\t COMM_ID= %s\t ELEM= %s\t VALUE= %s\t DATA= %s\n",
           pretext, pdu.type, pdu.tx_id, pdu.comm_id, pdu.element, pdu.value, pdu.info);
}

PDU_UDP generate_PDU_UDP(unsigned char type, char *tx_id, char *comm_id, char *data) {
    PDU_UDP pdu;
    pdu.type = type;
    strcpy(pdu.tx_id, tx_id);
    strcpy(pdu.comm_id, comm_id);
    strcpy(pdu.data, data);

    return pdu;
}

void sig_usr(int signo) {
    if (signo == SIGUSR1) {
        status = NOT_REGISTERED;
        close(tcpSock);
    }
}

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
    const PDU_UDP alive_pkt = generate_PDU_UDP(ALIVE, cfg.id, srv_info.comm_id, "");

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
        if (strcmp(rcv_pkt.tx_id, srv_info.tx_id) != 0 ||
            strcmp(rcv_pkt.comm_id, srv_info.comm_id) != 0) {
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

PDU_TCP generate_PDU_TCP(unsigned char type, char *tx_id, char *comm_id, Element elem) {
    PDU_TCP pdu;

    pdu.type = type;
    strcpy(pdu.tx_id, tx_id);
    strcpy(pdu.comm_id, comm_id);
    strcpy(pdu.element, elem.elem_string);
    strcpy(pdu.value, elem.value);

    time_t t1;
    struct tm *t2;
    char info_buffer[128];
    time(&t1);
    t2 = localtime(&t1);
    strftime(info_buffer, 128, "%Y-%m-%d;%H:%M:%S", t2);

    strcpy(pdu.info, info_buffer);

    return pdu;
}

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

        const PDU_TCP send_data_pkg = generate_PDU_TCP(SEND_DATA, cfg.id, srv_info.comm_id, elem);

        if (send(sock, &send_data_pkg, sizeof(send_data_pkg), 0) < 0) {
            perror("Error SEND_DATA");
            exit(1);
        } else {
            if (debug) print_PDU_TCP(send_data_pkg, "ENVIAT SEND_DATA");
        }

        fd_set fDesc; FD_ZERO(&fDesc);
        struct timeval tv = {m, 0};
        PDU_TCP rcv_pkt;

        FD_SET(sock, &fDesc);
        select(sock + 1, &fDesc, NULL, NULL, &tv);
        if (FD_ISSET(sock, &fDesc)) {
            recv(sock, &rcv_pkt, sizeof(rcv_pkt), 0);
            //TODO moure el comparar a una funcio auxiliar
            if (strcmp(rcv_pkt.tx_id, srv_info.tx_id) != 0 ||
                strcmp(rcv_pkt.comm_id, srv_info.comm_id) != 0||
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

//get GHXE2LWQ6C LUM-0-O
//set GHXE2LWQ6C LUM-0-I test

void handle_incoming_connection(int sock) {
    int sock2 = accept(sock, NULL, NULL);

    fd_set fDesc;
    PDU_TCP rcv_pkt;

    FD_ZERO(&fDesc);
    FD_SET(sock2, &fDesc);
    select(sock2 + 1, &fDesc, NULL, NULL, 0);
    if (FD_ISSET(sock2, &fDesc)) {
        recv(sock2, &rcv_pkt, sizeof(rcv_pkt), 0);
        //TODO moure el comparar a una funcio auxiliar
        if (strcmp(rcv_pkt.tx_id, srv_info.tx_id) != 0 ||
            strcmp(rcv_pkt.comm_id, srv_info.comm_id) != 0||
            strcmp(rcv_pkt.info, cfg.id) != 0) {
            //TODO preguntar quin es el comportament del test 18

            // TODO arreglar la porqueria de crear un paquet i modificarlo
            if (debug) print_PDU_TCP(rcv_pkt, "REBUT PAQUET ERRONI");
            printf("REBUT PAQUET AMB TX_ID O COMM_ID ERRONIS\n");

            PDU_TCP data_rej_pkt;
            strcpy(data_rej_pkt.tx_id, cfg.id);
            strcpy(data_rej_pkt.comm_id, srv_info.comm_id);
            strcpy(data_rej_pkt.element, rcv_pkt.element);
            strcpy(data_rej_pkt.value, rcv_pkt.value);
            strcpy(data_rej_pkt.info, "Error paquet amb dades erronies");

            if (send(sock2, &data_rej_pkt, sizeof(data_rej_pkt), 0) < 0) {
                perror("Error DATA_REJ");
                exit(1);
            } else {
                if (debug) print_PDU_TCP(data_rej_pkt, "ENVIAT DATA_REJ");
                status = NOT_REGISTERED;
                return;
            }
        }

        PDU_TCP data_nack_pkt;
        data_nack_pkt.type = DATA_NACK;
        strcpy(data_nack_pkt.tx_id, cfg.id);
        strcpy(data_nack_pkt.comm_id, srv_info.comm_id);
        strcpy(data_nack_pkt.element, rcv_pkt.element);
        strcpy(data_nack_pkt.value, rcv_pkt.value);
        strcpy(data_nack_pkt.info, "Error paquet amb dades erronies");

        Element * elem = getElement(&cfg, rcv_pkt.element);

        if (elem != NULL){
            switch (rcv_pkt.type) {
                case SET_DATA:
                    if (debug) print_PDU_TCP(rcv_pkt, "REBUT SET_DATA");

                    if(rcv_pkt.element[6] == 'I'){
                        strcpy(elem->value, rcv_pkt.value);
                        PDU_TCP data_ack = generate_PDU_TCP(DATA_ACK, cfg.id, srv_info.comm_id, *elem);
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

                    if(rcv_pkt.element[6] == 'O'){
                        PDU_TCP data_ack = generate_PDU_TCP(DATA_ACK, cfg.id, srv_info.comm_id, *elem);
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


//TODO comprovar a tot arreu on faci falta que la ip del servidor tmb es la correcta


