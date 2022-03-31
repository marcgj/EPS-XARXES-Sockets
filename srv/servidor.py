#!/usr/bin/env python3
import sys, signal
from srv.modules.terminal import *
from srv.modules.config import ServerCfg
import srv.modules.globals as g
from srv.modules.sockets import config_UDP, config_TCP
from srv.modules.register import RegisterService

cfg_filename = "server.cfg"
authorized_filename = "bbdd_dev.dat"

# Elimina un error de keyboard interrupt al fer cntrl+c
def siging_handler(signal, frame):
    exit(0)


def main():
    signal.signal(signal.SIGINT, siging_handler)
    handle_args()
    g.cfg = ServerCfg(cfg_filename, authorized_filename)
    g.cfg.print_cfg()
    g.cfg.print_devices()

    g.sockTCP = config_TCP(g.cfg.tcp)
    g.sockUDP = config_UDP(g.cfg.udp)
    reg = RegisterService(g.cfg, g.sockUDP)
    reg.run()


def handle_args():
    argc = len(sys.argv)
    if argc == 1:
        return

    i = 1
    while i < argc:
        arg = sys.argv[i]
        if arg == "-c":
            g.cfg_filename = sys.argv[i + 1]
            i += 2
        if arg == '-d':
            g.debug = True
            print_dbg("Mode debug activat")
            i += 1
        else:
            print_err("Error en els arguments")
            exit(1)


if __name__ == '__main__':
    main()





