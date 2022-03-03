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
    ClientCfg cfg;
    load_config(DEFAULT_CFG, &cfg);
    print_config(&cfg);

}

