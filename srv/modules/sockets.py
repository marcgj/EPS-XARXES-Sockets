import socket
from srv.modules.terminal import print_msg, print_err
from struct import pack, unpack


class UDP_PDU:
    format = "B11s11s61s"

    def __init__(self):
        self.buffer = None
        self.type = ""
        self.txId = ""
        self.commId = ""
        self.data = ""

    def load_buffer(self, buffer):
        self.buffer = buffer
        self._unpack()

    def load_values(self, type, txId, commId, data):
        self.type = type
        self.txId = str(txId)
        self.commId = str(commId)
        self.data = str(data)
        self._pack()

    def _pack(self):
        self.buffer = pack(self.format, self.type, self.txId.encode("utf-8"),
                           self.commId.encode("utf-8"), self.data.encode("utf-8"))

    def _unpack(self):
        type,  txId,  commId, data = unpack(self.format, self.buffer)
        self.type = type
        self.txId = txId.decode("utf-8")
        self.txId = self.txId.split('\0')[0]
        self.commId = commId.decode("utf-8")
        self.commId = self.commId.split('\0')[0]
        self.data = data.decode("utf-8", errors='ignore')
        self.data = self.data.split('\0')[0]

    def __str__(self):
        return f"type={self.type} \t txId={self.txId} \t commId={self.commId} \t data={self.data}"


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
    print_msg(f"Obert socket UDP amb port {port}")
    return s


def config_TCP(port):
    s = _config_socket(socket.SOCK_STREAM, port)
    print_msg(f"Obert socket TCP amb port {port}")
    return s


def recive_from(s):
    pdu = UDP_PDU()
    (buff, (ip, port)) = s.recvfrom(1024)
    pdu.load_buffer(buff)
    return pdu, (ip, port)


def send_to(s, pdu, ip, port):
    s.sendto(pdu.buffer, (ip, port))




