import socket
from struct import pack, unpack

from srv.modules.constants import enum_from_value, Types
from srv.modules.terminal import print_msg, print_err, print_dbg


class UDP_PDU:
    _format = "B11s11s61s"

    def __init__(self, _type: Types = None, txId="", commId="", data=""):
        self.buffer = b""
        self.type = _type
        self.txId = str(txId)
        self.commId = str(commId)
        self.data = str(data)
        if _type:
            self._pack()

    def load_buffer(self, buffer):
        self.buffer = buffer
        self._unpack()

    def _pack(self):
        self.buffer = pack(self._format, self.type.value, self.txId.encode("utf-8"),
                           self.commId.encode("utf-8"), self.data.encode("utf-8"))

    def _unpack(self):
        _type, txId, commId, data = unpack(self._format, self.buffer)
        self.type = enum_from_value(Types, _type)
        self.txId = txId.decode("utf-8")
        self.txId = self.txId.split('\0')[0]
        self.commId = commId.decode("utf-8")
        self.commId = self.commId.split('\0')[0]
        self.data = data.decode("utf-8", errors="ignore")
        self.data = self.data.split('\0')[0]

    def __str__(self):
        return f"type={self.type.name} \t txId={self.txId} \t commId={self.commId} \t data={self.data}"


class TCP_PDU:
    format = "B11s11s8s16s80s"

    def __init__(self, _type: Types = None, txId="", commId="", element="", valor="", info=""):
        self.buffer = b""
        self.type = _type
        self.txId = str(txId)
        self.commId = str(commId)
        self.element = str(element)
        self.value = str(valor)
        self.info = str(info)
        if _type:
            self._pack()

    def load_buffer(self, buffer):
        self.buffer = buffer
        self._unpack()

    def _pack(self):
        self.buffer = pack(self.format, self.type.value, self.txId.encode("utf-8"), self.commId.encode("utf-8"),
                           self.element.encode("utf-8"), self.value.encode("utf-8"), self.info.encode("utf-8"))

    def _unpack(self):
        _type, txId, commId, element, value, info = unpack(self.format, self.buffer)
        self.type = enum_from_value(Types, _type)
        self.txId = txId.decode("utf-8")
        self.txId = self.txId.split('\0')[0]
        self.commId = commId.decode("utf-8")
        self.commId = self.commId.split('\0')[0]
        self.element = element.decode("utf-8")
        self.element = self.element.split('\0')[0]
        self.value = value.decode("utf-8")
        self.value = self.value.split('\0')[0]
        self.info = info.decode("utf-8")
        self.info = self.info.split('\0')[0]

    def __str__(self):
        return f"type={self.type.name} \t txId={self.txId} \t commId={self.commId} \t element={self.element} " \
               f"\t value={self.value} \t info={self.info}"


def _config_socket(socktype, port):
    try:
        sock = socket.socket(socket.AF_INET, socktype)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind(("", port))
        return sock
    except Exception as e:
        print_err("Error al obrir el socket")
        print(e)
        exit(1)


def config_UDP(port):
    s = _config_socket(socket.SOCK_DGRAM, port)
    print_msg(f"Obert socket UDP amb port {s.getsockname()[1]}")
    return s


def config_TCP(port):
    s = _config_socket(socket.SOCK_STREAM, port)
    print_msg(f"Obert socket TCP amb port {port}")
    return s


def recive_from(s):
    pdu = UDP_PDU()
    (buff, (ip, port)) = s.recvfrom(1024)
    pdu.load_buffer(buff)
    print_dbg(f"REBUT {pdu}")
    return pdu, (ip, port)


def send_to(s, pdu, ip, port):
    s.sendto(pdu.buffer, (ip, port))
    print_dbg(f"ENVIAT {pdu}")
