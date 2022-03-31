
def print_dbg(string: str):
    print(f"DEBUG >> {string}")


def print_msg(string: str):
    print(f"MESSAGE >> {string}")


def print_err(string: str):
    print(f"\033[91mERROR >> {string}\033[0m")
