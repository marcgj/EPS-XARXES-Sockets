from srv.modules.constants import Status
from srv.modules.terminal import print_dbg
from srv.modules.terminal import print_msg, print_err


class Device:
    """
    Clase que encapsula tota la informacio referent a cada dispositiu
    i conte un metode per  canviar el estat, i comprovar si les dades d'un paquet es corresponen
    amb les corresponents/assignades al dispositiu
    """

    def __init__(self, deviceid: str):
        self.id = deviceid
        self.status = Status.DISCONNECTED

        self.ip = "############"
        self.commId = "############"

        self.portUDP = None
        self.portTCP = None

        self.elements = {}

        self.lastAlive = None
        self.missedAlives = 0

    def change_status(self, newStatus):
        self.status = newStatus
        print_dbg(f"Status del dispositiu {self.id} es {self.status.name}")

    def validate_pkt(self, pkt, ip):
        return ip == self.ip and pkt.commId == self.commId


def _elements_to_str(elems: dict):
    result = ""
    for s in elems.keys():
        result += s + " "
    return result


class ServerCfg:
    """
    Classe que carrega d'un fixter i encapsula tota la informacio referent a la configuraio del servidor i
    els dispositius autoritzats

    Ta
    """

    # Possible millora: fer anar configparse
    def __init__(self, cfgfilename: str, allowedsfilename: str):
        try:
            cfg = open(cfgfilename, 'r')
            f = open(allowedsfilename, "r")

            processed = []

            for line in cfg:
                line = line.rsplit()  # Elimina possible \n del final
                if line:
                    processed.append(line[-1])

            cfg.close()

            self.id = processed[0]
            self.udp = int(processed[1])
            self.tcp = int(processed[2])

            self.devices = {}

            for line in f:
                line = line[:-1]
                d = Device(line)
                self.devices.update({d.id: d})

            print_msg(f"Configuracio carregada de '{cfgfilename}' i '{allowedsfilename}'")
        except OSError as e:
            print_err("Error al carregar les configuracions")
            print(e)
            exit(1)

    # TODO Canviar aquesta porqueria
    def __str__(self):
        print_msg("---------- Server Config ----------")
        print_msg(f"Id: {self.id}")
        print_msg(f"UDP-port: {self.udp}")
        print_msg(f"TCP-port: {self.tcp}")
        print_msg("-----------------------------------")
        return ""

    def print_devices(self):
        print_msg("------------- Devices -------------")
        print_msg("\t Device ID    \t Status         \t CommId         \t IP             \t Elements")
        print_msg("\t------------  \t--------------  \t--------------  \t--------------  \t--------------")
        for device in self.devices.values():
            print_msg(f"\t {device.id: <13} \t {device.status.name: <13} \t\t {device.commId: <13} "
                      f"\t\t {device.ip: <13} \t\t {_elements_to_str(device.elements): <13} ")
        print_msg("-----------------------------------")
