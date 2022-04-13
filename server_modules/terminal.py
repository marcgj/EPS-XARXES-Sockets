import datetime
import os
from threading import Lock
from server_modules.constants import Status, Types

_debug = False


def _debug_on():
    global _debug
    _debug = True


lock = Lock()


# Per aquesta funcio he de fer anar un lock ja que al tindre prints en diferents procesos a vegades
# si escribien tots alhora apareixia tot en una sola linia
def print_wrapper(str):
    lock.acquire()
    t = datetime.datetime.now().strftime("%H:%M:%S")
    print(f"{t}: {str}", end="\n", flush=True)
    lock.release()


def print_dbg(string: str):
    global _debug
    if _debug:
        print_wrapper(f"DEBUG >> {string}")


def print_msg(string: str):
    print_wrapper(f"MESSAGE >> {string}")


def print_err(string: str):
    print_wrapper(f"\033[91mERROR >> {string}\033[0m")


def handle_terminal(text: str, service):
    splitted = text.split(" ")
    cmd = splitted[0]

    if cmd == "set":
        if len(splitted) != 4:
            print_err("La comanda requereix de 3 parametres")
            return

        deviceId = splitted[1]
        device = service.cfg.devices.get(deviceId)

        if not device:
            print_err(f"Dispositiu {deviceId} no exesteix")
            return

        if device.status != Status.SEND_ALIVE:
            print_err(f"El dispositiu {deviceId} no esta en SEND_ALIVE")
            return

        elementId = splitted[2]

        if not device.elements.get(elementId):
            print_err(f"El element {elementId} no exesteix")
            return

        if elementId[-1] != "I":
            print_err(f"El element ha de ser un actuador 'I'")
            return

        newVal = splitted[3]

        service.handle_outgoing_conexion(Types.SET_DATA, device, elementId, newVal)

    elif cmd == "get":
        if len(splitted) != 3:
            print_err("La comanda requereix de 2 parametres")
            return

        deviceId = splitted[1]
        device = service.cfg.devices.get(deviceId)

        if not device:
            print_err(f"Dispositiu {deviceId} no exesteix")
            return

        if device.status != Status.SEND_ALIVE:
            print_err(f"El dispositiu {deviceId} no esta en SEND_ALIVE")
            return

        elementId = splitted[2]

        if not device.elements.get(elementId):
            print_err(f"El element {elementId} no exesteix")
            return

        service.handle_outgoing_conexion(Types.GET_DATA, device, elementId)

    elif cmd == "list":
        service.cfg.print_devices()
    elif cmd == "quit":
        print_msg("Tancant el servidor")
        os._exit(1)
    else:
        if cmd:
            print_err(f"Comanda {cmd} no reconeguda")
