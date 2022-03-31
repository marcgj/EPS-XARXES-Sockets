import threading

from srv.modules.constants import *
from srv.modules.sockets import *
from srv.modules.terminal import print_dbg
from srv.modules.register import RegisterProcedure
from srv.modules.alive import AliveService


class UDPService:
    def __init__(self, cfg, sock):
        self.cfg = cfg
        self.sock = sock
        self.run()

    def _handler(self):
        print_dbg("Fill creat per atendre conexions UDP")
        alive = AliveService(self.cfg, self.sock)

        while True:
            rcv_pkt, (ip, port) = recive_from(self.sock)

            if rcv_pkt.type == Types.REG_REQ:
                if rcv_pkt.commId != ZEROS and rcv_pkt.data:
                    print_dbg("Error en el paquet REG_REQ")
                    reg_rej_pkt = UDP_PDU(Types.REG_REJ, self.cfg.id, ZEROS, "")
                    send_to(self.sock, reg_rej_pkt, ip, port)
                    continue

                device = self.cfg.devices[rcv_pkt.txId]
                device.ip = ip
                device.portUDP = port

                procedure = RegisterProcedure(device, self.cfg)
                procedure.run()

            elif rcv_pkt.type == Types.ALIVE:
                device = self.cfg.devices[rcv_pkt.txId]
                if ip != device.ip or rcv_pkt.commId != device.commId:
                    print_dbg(f"Discrepancies en el paquet ALIVE del dispositiu {device.id}")
                    alive_rej_pkg = UDP_PDU(Types.ALIVE_REJ, self.cfg.id, device.commId, device.id)
                    send_to(self.sock, alive_rej_pkg, ip, port)
                    device.change_status(Status.DISCONNECTED)
                    continue

                alive.processAlive(device)

            else:
                print_dbg("Paquet rebut erroni")
                device = self.cfg.devices[rcv_pkt.txId]
                if device:
                    device.change_status(Status.DISCONNECTED)

    def run(self):
        t = threading.Thread(target=self._handler, args=())
        t.start()






