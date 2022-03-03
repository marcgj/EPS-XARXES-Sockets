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

#include "cfgloader.h"

#define DEFAULT_CFG "../client.cfg"

int main(int argc, char *argv[]){
    int debug = 0;
    char cfgFileName[16] = DEFAULT_CFG;

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
            }
        }
        if (debug) printf("Config File Selected: %s\n", cfgFileName);
    }

    ClientCfg cfg;
    load_config(cfgFileName, &cfg);
    if (debug) print_config(&cfg);

}


