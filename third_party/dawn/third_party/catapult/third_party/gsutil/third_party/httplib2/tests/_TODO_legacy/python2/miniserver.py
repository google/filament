import logging
import os
import select
import SimpleHTTPServer
import socket
import SocketServer
import threading

HERE = os.path.dirname(__file__)
logger = logging.getLogger(__name__)


class ThisDirHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def translate_path(self, path):
        path = path.split("?", 1)[0].split("#", 1)[0]
        return os.path.join(HERE, *filter(None, path.split("/")))

    def log_message(self, s, *args):
        # output via logging so nose can catch it
        logger.info(s, *args)


class ShutdownServer(SocketServer.TCPServer):
    """Mixin that allows serve_forever to be shut down.

    The methods in this mixin are backported from SocketServer.py in the Python
    2.6.4 standard library. The mixin is unnecessary in 2.6 and later, when
    BaseServer supports the shutdown method directly.
    """

    def __init__(self, use_tls, *args, **kwargs):
        self.__use_tls = use_tls
        SocketServer.TCPServer.__init__(self, *args, **kwargs)
        self.__is_shut_down = threading.Event()
        self.__serving = False

    def server_bind(self):
        SocketServer.TCPServer.server_bind(self)
        if self.__use_tls:
            import ssl

            self.socket = ssl.wrap_socket(
                self.socket,
                os.path.join(os.path.dirname(__file__), "server.key"),
                os.path.join(os.path.dirname(__file__), "server.pem"),
                True,
            )

    def serve_forever(self, poll_interval=0.1):
        """Handle one request at a time until shutdown.

        Polls for shutdown every poll_interval seconds. Ignores
        self.timeout. If you need to do periodic tasks, do them in
        another thread.
        """
        self.__serving = True
        self.__is_shut_down.clear()
        while self.__serving:
            r, w, e = select.select([self.socket], [], [], poll_interval)
            if r:
                self._handle_request_noblock()
        self.__is_shut_down.set()

    def shutdown(self):
        """Stops the serve_forever loop.

        Blocks until the loop has finished. This must be called while
        serve_forever() is running in another thread, or it will deadlock.
        """
        self.__serving = False
        self.__is_shut_down.wait()

    def handle_request(self):
        """Handle one request, possibly blocking.

        Respects self.timeout.
        """
        # Support people who used socket.settimeout() to escape
        # handle_request before self.timeout was available.
        timeout = self.socket.gettimeout()
        if timeout is None:
            timeout = self.timeout
        elif self.timeout is not None:
            timeout = min(timeout, self.timeout)
        fd_sets = select.select([self], [], [], timeout)
        if not fd_sets[0]:
            self.handle_timeout()
            return
        self._handle_request_noblock()

    def _handle_request_noblock(self):
        """Handle one request, without blocking.

        I assume that select.select has returned that the socket is
        readable before this function was called, so there should be
        no risk of blocking in get_request().
        """
        try:
            request, client_address = self.get_request()
        except socket.error:
            return
        if self.verify_request(request, client_address):
            try:
                self.process_request(request, client_address)
            except:
                self.handle_error(request, client_address)
                self.close_request(request)


def start_server(handler, use_tls=False):
    httpd = ShutdownServer(use_tls, ("", 0), handler)
    threading.Thread(target=httpd.serve_forever).start()
    _, port = httpd.socket.getsockname()
    return httpd, port
