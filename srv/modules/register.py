import threading
from random import randint
from select import select

from srv.modules.constants import *
from srv.modules.sockets import *
from srv.modules.terminal import print_dbg


class RegisterService:

    def __init__(self, cfg, sock):
        self.cfg = cfg
        self.sock = sock
        self.portLliure = cfg.udp + 1

    def _reg_req_handler(self):
        print_dbg("Fill creat per atendre els REG_REQ")

        while True:
            rcv_pkt, (ip, port) = recive_from(self.sock)

            if rcv_pkt.type == Types.REG_REQ.value:
                print_dbg(f"REBUT REG_REQ {rcv_pkt}")

                if rcv_pkt.commId != Types.ZEROS and rcv_pkt.data:
                    print_dbg("Error en el paquet REG_REQ")
                    continue

                device = self.cfg.devices[rcv_pkt.txId]
                device.ip = ip
                device.portUDP = port

                procedure = RegisterProcedure(device, self._get_port(), self.cfg)
                procedure.run()

            else:
                print_dbg(f"REBUT PAQUET ERRONI {rcv_pkt}")
                reg_rej_pkt = UDP_PDU()
                reg_rej_pkt.load_values(Types.REG_REJ.value, self.cfg.id, Types.ZEROS, "")
                send_to(self.sock, reg_rej_pkt, ip, port)

    def _get_port(self):
        p = self.portLliure
        self.portLliure += 1
        return p

    def run(self):
        t = threading.Thread(target=self._reg_req_handler(), args=(self,))
        t.start()


class RegisterProcedure:
    z = 2

    def __init__(self, device, port, cfg):
        self.device = device
        self.port = port
        self.sock = config_UDP(port)  # TODO pregutar si qulasevol port serveix
        self.cfg = cfg
        self.commId = ""
        for _ in range(10):
            self.commId += str(randint(0, 9))

    def _handler(self):
        while self.device.status is not Status.REGISTERED:
            if self.device.status == Status.DISCONNECTED:
                reg_ack = UDP_PDU()
                reg_ack.load_values(Types.REG_ACK.value, self.cfg.id, self.commId, self.port)
                send_to(self.sock, reg_ack, self.device.ip, self.device.portUDP)
                print_dbg(f"ENVIAT REG_ACK {reg_ack}")
                self.device.status = Status.WAIT_INFO
                print_dbg(f"Status del dispositiu {self.device.id} es {self.device.status.name}")

            if self.device.status == Status.WAIT_INFO:
                i, o, e = select([self.sock], [], [], self.z)
                if self.sock in i:
                    rcv_pkt, (ip, port) = recive_from(self.sock)

                    if rcv_pkt.type == Types.REG_INFO.value:
                        print_dbg(f"REBUT REG_INFO {rcv_pkt}")

                        # TODO posar en una funcio a part el comparar
                        if ip != self.device.ip or rcv_pkt.commId != self.commId:
                            print_dbg(f"Discrepancies en el paquet REG_INFO del dispositiu {self.device.id}")
                            # TODO enviar info NACK
                            self.device.status = Status.DISCONNECTED
                            print_dbg(f"Status del dispositiu {self.device.id} es {self.device.status.name}")
                            return

                        # TODO control de errors
                        portTCP, elements = self._process_data(rcv_pkt.data)
                        self.device.portTCP = portTCP
                        self.device.elements = elements

                        info_ack_pkg = UDP_PDU()
                        info_ack_pkg.load_values(Types.INFO_ACK.value, self.cfg.id, self.commId, self.cfg.tcp)
                        send_to(self.sock, info_ack_pkg, ip, port)
                        print_dbg(f"ENVIAT INFO_ACK {info_ack_pkg}")
                        self.device.status = Status.REGISTERED
                        print_dbg(f"Status del dispositiu {self.device.id} es {self.device.status.name}")
                        return
                else:
                    print_dbg(f"No s'ha rebut el REG_INFO del dispositiu {self.device.id}")
                    # TODO moure el canvi de estat a una funcio
                    self.device.status = Status.DISCONNECTED
                    print_dbg(f"Status del dispositiu {self.device.id} es {self.device.status.name}")

                return

    def run(self):
        t = threading.Thread(target=self._handler(), args=(self,))
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
