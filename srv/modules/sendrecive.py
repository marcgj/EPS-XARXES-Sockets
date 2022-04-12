import sys
import threading
from select import select
from srv.modules.terminal import handle_terminal

from srv.modules.terminal import print_dbg, print_err


class SendReciveService:
    def __init__(self, cfg, sock):
        self.cfg = cfg
        self.sock = sock
        self._run()

    def _handler(self):
        print_dbg("Fill creat sendrecive")
        while True:
            i = select([self.sock, sys.stdin], [], [], None)[0]
            if self.sock in i:
                pass
            elif sys.stdin in i:
                text = input()
                handle_terminal(text, self.cfg)


    def _run(self):
        t = threading.Thread(target=self._handler, args=())
        t.start()


