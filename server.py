#!/usr/bin/env python3
import sys

from server_modules.conexions import UDPService
from server_modules.config import ServerCfg
from server_modules.sendrecive import SendReciveService
from server_modules.sockets import config_UDP, config_TCP
from server_modules.terminal import print_dbg, print_err, _debug_on

cfg_filename = "server.cfg"
authorized_filename = "bbdd_dev.dat"


def main():
    handle_args()
    cfg = ServerCfg(cfg_filename, authorized_filename)
    print(cfg)
    cfg.print_devices()

    sockTCP = config_TCP(cfg.tcp)
    sockUDP = config_UDP(cfg.udp)

    UDPService(cfg, sockUDP)
    SendReciveService(cfg, sockTCP)


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
