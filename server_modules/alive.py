import threading
import time

from server_modules.config import Device
from server_modules.constants import *
from server_modules.sockets import *
from server_modules.terminal import print_dbg


class AliveService:
    """
    Genera un nou proces que revisa els temps que fa que no es reb un ALIVE dels diferents dispositius
    Tambe te un metode que permet reiniciar els comptadors d'un dispositiu
    """
    _w = 3
    _x = 3

    def __init__(self, cfg, sock):
        self._sock = sock
        self._cfg = cfg
        self._run()

    def _handler(self):
        print_dbg("Fill creat per atendre els ALIVE")
        while True:
            for device in self._cfg.devices.values():
                now = time.time()
                if device.status == Status.SEND_ALIVE or device.status == Status.REGISTERED:
                    if now - device.lastAlive > self._w:
                        if device.missedAlives >= self._x - 1:
                            print_err(f"No s'han rebut {self._x} alives consequtius de {device.id}")
                            device.change_status(Status.DISCONNECTED)
                        elif device.status == Status.REGISTERED:
                            print_err(f"No s'ha rebut resposta al primer ALIVE del dispositiu {device.id}")
                            device.change_status(Status.DISCONNECTED)
                        else:
                            print_err(f"No s'ha rebut alive de {device.id}, alives perduts {device.missedAlives + 1}")
                            device.missedAlives += 1
                            device.lastAlive = now
            time.sleep(0.25)  # Per no malgastar recursos del ordenador

    def processAlive(self, device: Device):
        if device.status == Status.REGISTERED:
            device.change_status(Status.SEND_ALIVE)

        device.lastAlive = time.time()
        device.missedAlives = 0

        alive_pkt = UDP_PDU(Types.ALIVE, self._cfg.id, device.commId, device.id)
        send_to(self._sock, alive_pkt, device.ip, device.portUDP)

    def _run(self):
        t = threading.Thread(target=self._handler, args=())
        t.start()
