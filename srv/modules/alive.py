import threading
import time

from srv.modules.constants import *
from srv.modules.config import Device
from srv.modules.sockets import *
from srv.modules.terminal import print_dbg


class AliveService:
    w = 3
    x = 3

    def __init__(self, cfg, sock):
        self.sock = sock
        self.cfg = cfg
        self.run()

    def _handler(self):
        print_dbg("Fill creat per atendre els ALIVE")
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

        alive_pkt = UDP_PDU(Types.ALIVE, self.cfg.id, device.commId, device.id)
        send_to(self.sock, alive_pkt, device.ip, device.portUDP)

    def run(self):
        t = threading.Thread(target=self._handler, args=())
        t.start()

