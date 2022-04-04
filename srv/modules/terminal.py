import datetime
_debug = False


def _debug_on():
    global _debug
    _debug = True


def print_wrapper(str):
    t = datetime.datetime.now().strftime("%H:%M:%S")
    print(f"{t} | {str}")


def print_dbg(string: str):
    global _debug
    if _debug:
        print_wrapper(f"DEBUG >> {string}")


def print_msg(string: str):
    print_wrapper(f"MESSAGE >> {string}")


def print_err(string: str):
    print_wrapper(f"\033[91mERROR >> {string}\033[0m")

