import threading

from server_modules.alive import AliveService
from server_modules.constants import *
from server_modules.register import RegisterProcedure
from server_modules.sockets import *
from server_modules.terminal import print_dbg


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
            # Sempre es mira que el paquet rebut estiqui dins de la llista de autoritzats
            if not self._cfg.is_authorized_device(rcv_pkt.txId):
                print_err(f"Dispositiu {rcv_pkt.txId} no esta autoritzat")
                continue

            # En el cas de rebe una peticio de registre fem les comprovacions pertinents i iniciem un nou
            # proces de registre en un nou socket
            if rcv_pkt.type == Types.REG_REQ:
                if rcv_pkt.commId != ZEROS or rcv_pkt.data:
                    print_err(f"Rebut REG_REQ erroni de {rcv_pkt.txId}")
                    reg_rej_pkt = UDP_PDU(Types.REG_REJ, self._cfg.id, ZEROS, "")
                    send_to(self._sock, reg_rej_pkt, ip, port)
                    continue

                device = self._cfg.devices[rcv_pkt.txId]
                if device.status != Status.DISCONNECTED:
                    continue

                device.ip = ip
                device.portUDP = port

                procedure = RegisterProcedure(device, self._cfg)

            # En el cas de que el paquet sigui de tipus alive, comprovem que el dispositiu relacionat estiqui en algun
            # dels dos estats que permeten alives i si els altres camps son correctes, per acabar s'envia a procesar
            # all servei de alives
            elif rcv_pkt.type == Types.ALIVE:
                device = self._cfg.devices.get(rcv_pkt.txId)
                if not (device.status == Status.SEND_ALIVE or device.status == Status.REGISTERED):
                    print_err(f"Rebut alive no esperat")
                    device.change_status(Status.DISCONNECTED)
                    continue

                if not device.validate_pkt(rcv_pkt, ip) or rcv_pkt.data:
                    print_err(f"Discrepancies en el paquet ALIVE del dispositiu {device.id}")
                    alive_rej_pkg = UDP_PDU(Types.ALIVE_REJ, self._cfg.id, device.commId, device.id)
                    send_to(self._sock, alive_rej_pkg, ip, port)
                    device.change_status(Status.DISCONNECTED)
                    continue

                alive.processAlive(device)

            # En cas de rebre un paquet diferent, el dispositiu pasara a DISC.
            else:
                print_dbg("Paquet rebut erroni")
                device = self._cfg.devices.get(rcv_pkt.txId)
                if device:
                    device.change_status(Status.DISCONNECTED)

    def _run(self):
        t = threading.Thread(target=self._handler, args=())
        t.start()
