from srv.modules.terminal import print_msg, print_err
from srv.modules.constants import Status
from srv.modules.terminal import print_dbg


class Device:
    def __init__(self, deviceid: str):
        self.id = deviceid
        self.status = Status.DISCONNECTED
        self.lastAlive = None
        self.missedAlives = 0
        self.ip = None
        self.portUDP = None
        self.portTCP = None
        self.commId = ""
        self.elements = {}

    def change_status(self, newStatus):
        self.status = newStatus
        print_dbg(f"Status del dispositiu {self.id} es {self.status.name}")


class ServerCfg:
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

    def print_cfg(self):
        print_msg("---------- Server Config ----------")
        print_msg(f"Id: {self.id}")
        print_msg(f"UDP-port: {self.udp}")
        print_msg(f"TCP-port: {self.tcp}")
        print_msg("-----------------------------------")

    def print_devices(self):
        print_msg("------------- Devices -------------")
        print_msg("\t Device ID     \t Status")
        print_msg("\t------------   \t--------------")
        for device in self.devices.values():
            print_msg(f"\t {device.id}  \t {device.status.name}")
        print_msg("-----------------------------------")


def load_allowed_ids(filename: str):
    f = open(filename, "r")
    lst = []

    for line in f:
        line = line[:-1]
        lst.append(line)

    return lst
