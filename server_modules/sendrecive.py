import sys
import threading
from os import getcwd
from select import select
from socket import socket

from server_modules.constants import *
from server_modules.terminal import print_dbg, print_err, handle_terminal
from server_modules.sockets import send, recive, TCP_PDU, config_TCP
from server_modules.config import Device


def _log_element_change(pdu):
    try:
        f = open(f"{pdu.txId}.data", "a")
        s = f"{pdu.info} type={pdu.type.name} element={pdu.element} value={pdu.value}"
        f.write(s + "\n")
        print_dbg(f"Guardat a {getcwd()}{pdu.txId}.data: {s}")
        f.close()
        return True
    except OSError as e:
        return False


class SendReciveService:
    m = 3

    def __init__(self, cfg, sock: socket):
        self.cfg = cfg
        self.sock = sock
        self._run()

    def _handler(self):
        print_dbg("Fill creat sendrecive")
        self.sock.listen(1)
        while True:
            i = select([self.sock, sys.stdin], [], [], None)[0]
            if self.sock in i:
                self.handle_incoming_connection()
            elif sys.stdin in i:
                text = input()
                handle_terminal(text, self)

    def _run(self):
        t = threading.Thread(target=self._handler, args=())
        t.start()

    def handle_incoming_connection(self):
        sock, (ip, port) = self.sock.accept()

        i = select([sock], [], [], self.m)[0]
        if not i:
            print_err(f"No s'ha rebut cap paquet en {self.m}s")
            sock.close()
            return

        rcv_pkt = recive(sock)

        if rcv_pkt.type == Types.SEND_DATA:
            device: Device = self.cfg.devices.get(rcv_pkt.txId)
            if not device:
                print_err(f"Dispositiu {rcv_pkt.txId} no autoritzat")
                data_rej_pkt = TCP_PDU(Types.DATA_REJ, self.cfg.id, ZEROS, info="Dispositiu no autoritzat")
                send(sock, data_rej_pkt)
                sock.close()
                return

            if not device.validate_pkt(rcv_pkt, ip):
                print_err(f"Error en les dades d'identificacio")
                data_rej_pkt = TCP_PDU(Types.DATA_REJ, self.cfg.id, device.commId, rcv_pkt.element, rcv_pkt.value)
                send(sock, data_rej_pkt)
                device.change_status(Status.DISCONNECTED)
                sock.close()
                return

            element = device.elements.get(rcv_pkt.element)
            if not element:
                print_err(f"Element '{rcv_pkt.element}' no exesteix")
                data_nack_pkt = TCP_PDU(Types.DATA_NACK, self.cfg.id, device.commId, rcv_pkt.element, rcv_pkt.value,
                                        "L'element no exesteix")
                send(sock, data_nack_pkt)
                sock.close()
                return

            if not rcv_pkt.info:
                sock.close()
                return

            if not _log_element_change(rcv_pkt):
                print_err(f"Error al guardar les dades")
                data_nack_pkt = TCP_PDU(Types.DATA_NACK, self.cfg.id, device.commId, rcv_pkt.element, rcv_pkt.value,
                                        "Error al guardar les dades")
                send(sock, data_nack_pkt)
                sock.close()
                return

            data_ack = TCP_PDU(Types.DATA_ACK, self.cfg.id, device.commId, rcv_pkt.element, rcv_pkt.value, device.id)
            send(sock, data_ack)

        else:
            print_err("Paquet erroni rebut")

        sock.close()

    def handle_outgoing_conexion(self, pdu_type, device, element, value=""):
        sock = config_TCP(0)
        try:
            print_dbg(f"Connect amb {device.ip} al port {device.portTCP}")
            sock.connect((device.ip, int(device.portTCP)))
        except Exception as e:
            print_err(f"Error al fer connect amb {device.id}: {e}")
            device.change_status(Status.DISCONNECTED)
            sock.close()
            return

        out_pkt = TCP_PDU(pdu_type, self.cfg.id, device.commId, element, value, device.id)
        send(sock, out_pkt)

        i = select([sock], [], [], self.m)[0]
        if sock in i:
            rcv_pkt = recive(sock)
            if rcv_pkt == None:
                print_err("Rebut buffer buit, segurament el peer ha tancat la conexio")
                return

            if not device.validate_pkt(rcv_pkt, device.ip):
                print_err("Error en les dades d'identificacio del paquet")
                sock.close()
                return

            if rcv_pkt.element != element:
                print_err("Error el camp element")
                sock.close()
                return

            if rcv_pkt.type == Types.DATA_REJ:
                device.change_status(Status.DISCONNECTED)
                sock.close()
                return
            elif rcv_pkt.type == Types.DATA_NACK:
                print_err("Operacio fallida")
                sock.close()
                return
            elif rcv_pkt.type == Types.DATA_ACK:
                rcv_pkt.type = pdu_type
                _log_element_change(rcv_pkt)
            else:
                print_err(f"Rebut paquet erroni de {device.id}")

        else:
            print_err(f"No s'ha rebut cap paquet en {self.m}s")

        sock.close()

# get GHXE2LWQ6C LUM-0-O
# set GHXE2LWQ6C LUM-0-I test
# set LUM-0-I test
# send LUM-0-I
