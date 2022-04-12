import datetime
import os

from srv.modules.constants import Status

_debug = False


def _debug_on():
    global _debug
    _debug = True


def print_wrapper(str):
    t = datetime.datetime.now().strftime("%H:%M:%S")
    print(f"{t}: {str}")


def print_dbg(string: str):
    global _debug
    if _debug:
        print_wrapper(f"DEBUG >> {string}")


def print_msg(string: str):
    print_wrapper(f"MESSAGE >> {string}")


def print_err(string: str):
    print_wrapper(f"\033[91mERROR >> {string}\033[0m")


def handle_terminal(text: str, cfg):
    splitted = text.split(" ")
    cmd = splitted[0]

    if cmd == "set":
        if len(splitted) != 4:
            print_err("La comanda requereix de 3 parametres")
            return

        deviceId = splitted[1]
        device = cfg.devices.get(deviceId)

        if not device:
            print_err(f"Dispositiu {deviceId} no exesteix")
            return

        if device.status != Status.SEND_ALIVE:
            print_err(f"El dispositiu {deviceId} no esta en SEND_ALIVE")
            return

        elementId = splitted[2]

        if elementId not in device.elements:
            print_err(f"El element {elementId} no exesteix")
            return

        if elementId[:-1] != "I":
            print_err("El element ha de ser un actuador 'I'")
            return

        newVal = splitted[3]

        # todo tcpshit

    elif cmd == "get":
        if len(splitted) != 3:
            print_err("La comanda requereix de 2 parametres")
            return

        deviceId = splitted[1]
        device = cfg.devices.get(deviceId)

        if not device:
            print_err(f"Dispositiu {deviceId} no exesteix")
            return

        if device.status != Status.SEND_ALIVE:
            print_err(f"El dispositiu {deviceId} no esta en SEND_ALIVE")
            return

        elementId = splitted[2]

        if elementId not in device.elements:
            print_err(f"El element {elementId} no exesteix")
            return

        if elementId[:-1] != "I":
            print_err("El element ha de ser un actuador 'I'")
            return

        # todo tcpshit

    elif cmd == "list":
        cfg.print_devices()
    elif cmd == "quit":
        print_msg("Tancant el servidor")
        os._exit(1)
    else:
        print_err(f"Comanda {cmd} no reconeguda")
