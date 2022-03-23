//
// Created by Marc Gaspà Joval on 2/3/22.
//

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>


#include "utilitats/cfgloader.h"
#include "utilitats/conexions.h"

#define DEFAULT_CFG "../client.cfg"


int debug = 0;
unsigned char status = DISCONNECTED;
ClientCfg cfg;
rcv_info srv_info;

int main(int argc, char *argv[]){
    char cfgFileName[16] = DEFAULT_CFG;

    process_args(argc, argv, cfgFileName);

    load_config(cfgFileName, &cfg);
    if (debug) print_config(&cfg);

    register_client();
    start_alive_service();

    wait(NULL);
}


void process_args(int argc, char **argv, char * cfgFileName) {
    if(argc > 1){
        for (int i = 1; i < argc;) {
            if (argv[i][0] != '-' || strlen(argv[i]) > 2){
                perror("Mal us dels arguments");
                exit(1);
            }
            switch (argv [i][1]) {
                case 'd':
                    debug = 1;
                    printf("MODE DEBUG ACTIVAT\n");
                    i++;
                    break;
                case 'c':
                    strcpy(cfgFileName, argv[i+1]);
                    i += 2;
                    break;

                default:
                    printf("Parametre no reconegut");
                    exit(1);
            }
        }
        if (debug) printf("Config File Selected: %s\n", cfgFileName);
    }

}






