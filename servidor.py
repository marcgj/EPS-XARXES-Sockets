class ServerCfg:
    # Possible millora: fer anar configparser
    def __init__(self, filename: str):
        f = open(filename, 'r')

        processed = []
        for line in f:
            line = line.rsplit()  # Elimina possible \n del final
            if line:
                processed.append(line[-1])

        f.close()

        self.id = processed[0]
        self.udp = processed[1]
        self.tcp = processed[2]

    def printcfg(self):
        print("---------- Server Config ----------")
        print("      Id: ", self.id)
        print("UDP-port: ", self.udp)
        print("TCP-port: ", self.tcp)
        print("-----------------------------------")


def loadAllowedIds(filename: str):
    f = open(filename, "r")
    lst = []

    for line in f:
        line = line[:-1]
        lst.append(line)

    return lst


# Parametres per defecte
DEFAULT_CGF = "server.cfg"
DEFAULT_AUTH = "bbdd_dev.dat"

if __name__ == '__main__':
    cfg = ServerCfg(DEFAULT_CGF)
    allowedIds = loadAllowedIds(DEFAULT_AUTH)
