import threading
from random import randint
from select import select

from srv.modules.constants import *
from srv.modules.sockets import *
from srv.modules.terminal import print_dbg


class RegisterProcedure:
    z = 2

    def __init__(self, device, cfg):
        self.device = device
        self.sock = config_UDP(0)
        self.port = self.sock.getsockname()[1]
        self.cfg = cfg
        self.device.commId = ""
        for _ in range(10):
            self.device.commId += str(randint(0, 9))

    def _handler(self):
        print_dbg(f"Fill creat per el registre de {self.device.id}")
        while self.device.status is not Status.REGISTERED:
            if self.device.status == Status.DISCONNECTED:
                reg_ack = UDP_PDU(Types.REG_ACK, self.cfg.id, self.device.commId, self.port)
                send_to(self.sock, reg_ack, self.device.ip, self.device.portUDP)
                self.device.change_status(Status.WAIT_INFO)

            if self.device.status == Status.WAIT_INFO:
                i = select([self.sock], [], [], self.z)[0]
                if self.sock in i:
                    rcv_pkt, (ip, port) = recive_from(self.sock)

                    if rcv_pkt.type == Types.REG_INFO:
                        # TODO posar en una funcio a part el comparar
                        if not self.device.validate_pkt(rcv_pkt, ip) or not rcv_pkt.data:
                            print_dbg(f"Discrepancies en el paquet REG_INFO del dispositiu {self.device.id}")
                            info_nack_pkg = UDP_PDU(Types.INFO_NACK, self.cfg.id, self.device.commId,
                                                    "Error en els camps de la pdu")
                            send_to(self.sock, info_nack_pkg, ip, port)
                            self.device.change_status(Status.DISCONNECTED)
                            return

                        self.device.portTCP, self.device.elements = self._process_data(rcv_pkt.data)

                        info_ack_pkg = UDP_PDU(Types.INFO_ACK, self.cfg.id, self.device.commId, self.cfg.tcp)
                        send_to(self.sock, info_ack_pkg, ip, port)
                        self.device.change_status(Status.REGISTERED)
                        return
                else:
                    print_dbg(f"No s'ha rebut el REG_INFO del dispositiu {self.device.id}")
                    self.device.change_status(Status.DISCONNECTED)
                return

    def run(self):
        t = threading.Thread(target=self._handler, args=())
        t.start()

    @staticmethod
    def _process_data(data):
        elems = {}
        split = data.split(',')
        port = split[0]

        tokenizedElems = split[1].split(';')

        for elem in tokenizedElems:
            elems.update({elem: ""})

        return port, elems
