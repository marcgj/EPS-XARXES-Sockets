
import threading
import time
from random import randint
from select import select

from srv.modules.constants import *
from srv.modules.config import Device
from srv.modules.sockets import *
from srv.modules.terminal import print_dbg



class RegisterService:
    def __init__(self, cfg, sock):
        self.cfg = cfg
        self.sock = sock

    def _reg_req_handler(self):
        print_dbg("Fill creat per atendre els REG_REQ")
        alive = AliveService(self.sock, self.cfg)
        alive.run()

        while True:
            rcv_pkt, (ip, port) = recive_from(self.sock)

            if rcv_pkt.type == Types.REG_REQ.value:
                print_dbg(f"REBUT REG_REQ {rcv_pkt}")

                if rcv_pkt.commId != Types.ZEROS and rcv_pkt.data:
                    print_dbg("Error en el paquet REG_REQ")
                    reg_rej_pkt = UDP_PDU(Types.REG_REJ.value, self.cfg.id, Types.ZEROS, "")
                    send_to(self.sock, reg_rej_pkt, ip, port)
                    print_dbg(f"ENVIAT REG_REJ {rcv_pkt}")
                    continue

                device = self.cfg.devices[rcv_pkt.txId]
                device.ip = ip
                device.portUDP = port

                procedure = RegisterProcedure(device, self.cfg)
                procedure.run()

            elif rcv_pkt.type == Types.ALIVE.value:
                print_dbg(f"REBUT ALIVE {rcv_pkt}")

                device = self.cfg.devices[rcv_pkt.txId]
                if ip != device.ip or rcv_pkt.commId != device.commId:
                    print_dbg(f"Discrepancies en el paquet ALIVE del dispositiu {device.id}")
                    alive_rej_pkg = UDP_PDU(Types.ALIVE_REJ.value, self.cfg.id, device.commId, device.id)
                    send_to(self.sock, alive_rej_pkg, ip, port)
                    print_dbg(f"ENVIAT ALIVE_REJ {alive_rej_pkg}")
                    device.change_status(Status.DISCONNECTED)
                    continue

                alive.processAlive(device)

            else:
                print_dbg(f"REBUT PAQUET ERRONI {rcv_pkt}")
                device = self.cfg.devices[rcv_pkt.txId]
                if device:
                    device.change_status(Status.DISCONNECTED)

    def run(self):
        t = threading.Thread(target=self._reg_req_handler, args=())
        t.start()


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
                reg_ack = UDP_PDU(Types.REG_ACK.value, self.cfg.id, self.device.commId, self.port)
                send_to(self.sock, reg_ack, self.device.ip, self.device.portUDP)
                print_dbg(f"ENVIAT REG_ACK {reg_ack}")
                self.device.change_status(Status.WAIT_INFO)

            if self.device.status == Status.WAIT_INFO:
                i = select([self.sock], [], [], self.z)[0]
                if self.sock in i:
                    rcv_pkt, (ip, port) = recive_from(self.sock)

                    if rcv_pkt.type == Types.REG_INFO.value:
                        print_dbg(f"REBUT REG_INFO {rcv_pkt}")
                        # TODO posar en una funcio a part el comparar
                        if ip != self.device.ip or rcv_pkt.commId != self.device.commId:
                            print_dbg(f"Discrepancies en el paquet REG_INFO del dispositiu {self.device.id}")
                            info_nack_pkg = UDP_PDU(Types.INFO_NACK.value, self.cfg.id, self.device.commId,
                                                    "Error en els camps d'identificacio del servidor")
                            send_to(self.sock, info_nack_pkg, ip, port)
                            print_dbg(f"ENVIAT INFO_NACK {info_nack_pkg}")
                            self.device.change_status(Status.DISCONNECTED)
                            return

                        # TODO control de errors
                        self.device.portTCP, self.device.elements = self._process_data(rcv_pkt.data)

                        info_ack_pkg = UDP_PDU(Types.INFO_ACK.value, self.cfg.id, self.device.commId, self.cfg.tcp)
                        send_to(self.sock, info_ack_pkg, ip, port)
                        print_dbg(f"ENVIAT INFO_ACK {info_ack_pkg}")
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


class AliveService:
    w = 3
    x = 2

    def __init__(self, sock, cfg):
        self.sock = sock
        self.cfg = cfg

    def _handler(self):
        print_dbg("Fill creat per atendre els alives")
        while True:
            for device in self.cfg.devices.values():
                if device.status != Status.SEND_ALIVE:
                    continue
                now = time.time()
                if now - device.lastAlive > self.w:
                    if device.missedAlives >= self.x:
                        print_err(f"No s'han rebut {self.x} alives consequtius de {device.id}")
                        device.change_status(Status.DISCONNECTED)
                    else:
                        device.missedAlives += 1

            time.sleep(0.5)

    def processAlive(self, device: Device):
        if device.status == Status.REGISTERED:
            device.change_status(Status.SEND_ALIVE)

        device.lastAlive = time.time()
        device.missedAlives = 0

        alive_pkt = UDP_PDU(Types.ALIVE.value, self.cfg.id, device.commId, device.id)
        send_to(self.sock, alive_pkt, device.ip, device.portUDP)
        print_dbg(f"ENVIAT ALIVE {alive_pkt}")

    def run(self):
        t = threading.Thread(target=self._handler, args=())
        t.start()
