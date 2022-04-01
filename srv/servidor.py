#!/usr/bin/env python3
import os
import sys
import signal
from srv.modules.terminal import print_dbg, print_err, _debug_on
from srv.modules.config import ServerCfg
from srv.modules.sockets import config_UDP, config_TCP
from srv.modules.conexions import UDPService

cfg_filename = "server.cfg"
authorized_filename = "bbdd_dev.dat"


# Elimina un error de keyboard interrupt al fer cntrl+c
def siging_handler(s, f):
    exit(0)


def main():
    signal.signal(signal.SIGINT, siging_handler)
    handle_args()
    cfg = ServerCfg(cfg_filename, authorized_filename)
    cfg.print_cfg()
    cfg.print_devices()

    sockTCP = config_TCP(cfg.tcp)
    sockUDP = config_UDP(cfg.udp)

    udpService = UDPService(cfg, sockUDP)


def handle_args():
    argc = len(sys.argv)
    if argc == 1:
        return

    i = 1
    while i < argc:
        arg = sys.argv[i]
        if arg == "-c":
            global cfg_filename
            cfg_filename = sys.argv[i + 1]
            i += 2
        if arg == '-d':
            _debug_on()
            print_dbg("Mode debug activat")
            i += 1
        else:
            print_err("Error en els arguments")
            exit(1)


if __name__ == '__main__':
    main()





