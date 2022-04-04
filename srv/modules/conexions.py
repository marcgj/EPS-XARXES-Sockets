import threading

from srv.modules.constants import *
from srv.modules.sockets import *
from srv.modules.terminal import print_dbg
from srv.modules.register import RegisterProcedure
from srv.modules.alive import AliveService


class UDPService:
    """
    Crea un proces que gestiona totes les conexions entrats per el port UDP
    definit en el archiu de configuracio

    El servei comprova si el paquet rebut pertany a un element autoritzat, tambe gestiona la
    primera part del registre, i realitza comprovacions per als ALIVE rebuts
    """

    def __init__(self, cfg, sock):
        self._cfg = cfg
        self._sock = sock
        self._run()

    def _handler(self):
        print_dbg("Fill creat per atendre conexions UDP")
        alive = AliveService(self._cfg, self._sock)

        while True:
            rcv_pkt, (ip, port) = recive_from(self._sock)
            if rcv_pkt.txId not in self._cfg.devices.keys():
                print_err(f"Dispositiu {rcv_pkt.txId} no esta autoritzat")
                continue

            if rcv_pkt.type == Types.REG_REQ:
                if rcv_pkt.commId != ZEROS and rcv_pkt.data:
                    print_dbg("Error en el paquet REG_REQ")
                    reg_rej_pkt = UDP_PDU(Types.REG_REJ, self._cfg.id, ZEROS, "")
                    send_to(self._sock, reg_rej_pkt, ip, port)
                    continue

                device = self._cfg.devices[rcv_pkt.txId]
                device.ip = ip
                device.portUDP = port

                procedure = RegisterProcedure(device, self._cfg)
                procedure.run()

            elif rcv_pkt.type == Types.ALIVE:
                device = self._cfg.devices[rcv_pkt.txId]
                if not device.validate_pkt(rcv_pkt, ip):
                    print_dbg(f"Discrepancies en el paquet ALIVE del dispositiu {device.id}")
                    alive_rej_pkg = UDP_PDU(Types.ALIVE_REJ, self._cfg.id, device.commId, device.id)
                    send_to(self._sock, alive_rej_pkg, ip, port)
                    device.change_status(Status.DISCONNECTED)
                    continue

                alive.processAlive(device)

            else:
                print_dbg("Paquet rebut erroni")
                device = self._cfg.devices[rcv_pkt.txId]
                if device:
                    device.change_status(Status.DISCONNECTED)

    def _run(self):
        t = threading.Thread(target=self._handler, args=())
        t.start()






