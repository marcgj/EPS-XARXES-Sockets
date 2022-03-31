
_debug = False


def _debug_on():
    global _debug
    _debug = True


def print_dbg(string: str):
    global _debug
    if _debug:
        print(f"DEBUG >> {string}")


def print_msg(string: str):
    print(f"MESSAGE >> {string}")


def print_err(string: str):
    print(f"\033[91mERROR >> {string}\033[0m")
