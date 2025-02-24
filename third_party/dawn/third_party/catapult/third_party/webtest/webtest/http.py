# -*- coding: utf-8 -*-
"""
This module contains some helpers to deal with the real http
world.
"""

import threading
import logging
import select
import socket
import time
import os

import six
import webob
from six.moves import http_client
from waitress.server import TcpWSGIServer


def get_free_port():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(('', 0))
    ip, port = s.getsockname()
    s.close()
    ip = os.environ.get('WEBTEST_SERVER_BIND', '127.0.0.1')
    return ip, port


def check_server(host, port, path_info='/', timeout=3, retries=30):
    """Perform a request until the server reply"""
    if retries < 0:
        return 0
    time.sleep(.3)
    for i in range(retries):
        try:
            conn = http_client.HTTPConnection(host, int(port), timeout=timeout)
            conn.request('GET', path_info)
            res = conn.getresponse()
            return res.status
        except (socket.error, http_client.HTTPException):
            time.sleep(.3)
    return 0


class StopableWSGIServer(TcpWSGIServer):
    """StopableWSGIServer is a TcpWSGIServer which run in a separated thread.
    This allow to use tools like casperjs or selenium.

    Server instance have an ``application_url`` attribute formated with the
    server host and port.
    """

    was_shutdown = False

    def __init__(self, application, *args, **kwargs):
        super(StopableWSGIServer, self).__init__(self.wrapper, *args, **kwargs)
        self.runner = None
        self.test_app = application
        self.application_url = 'http://%s:%s/' % (self.adj.host, self.adj.port)

    def wrapper(self, environ, start_response):
        """Wrap the wsgi application to override some path:

        ``/__application__``: allow to ping the server.

        ``/__file__?__file__={path}``: serve the file found at ``path``
        """
        if '__file__' in environ['PATH_INFO']:
            req = webob.Request(environ)
            resp = webob.Response()
            resp.content_type = 'text/html; charset=UTF-8'
            filename = req.params.get('__file__')
            if os.path.isfile(filename):
                body = open(filename, 'rb').read()
                body = body.replace(six.b('http://localhost/'),
                                    six.b('http://%s/' % req.host))
                resp.body = body
            else:
                resp.status = '404 Not Found'
            return resp(environ, start_response)
        elif '__application__' in environ['PATH_INFO']:
            return webob.Response('server started')(environ, start_response)
        return self.test_app(environ, start_response)

    def run(self):
        """Run the server"""
        try:
            self.asyncore.loop(.5, map=self._map)
        except select.error:  # pragma: no cover
            if not self.was_shutdown:
                raise

    def shutdown(self):
        """Shutdown the server"""
        # avoid showing traceback related to asyncore
        self.was_shutdown = True
        self.logger.setLevel(logging.FATAL)
        while self._map:
            triggers = list(self._map.values())
            for trigger in triggers:
                trigger.handle_close()
        self.maintenance(0)
        self.task_dispatcher.shutdown()
        return True

    @classmethod
    def create(cls, application, **kwargs):
        """Start a server to serve ``application``. Return a server
        instance."""
        host, port = get_free_port()
        if 'port' not in kwargs:
            kwargs['port'] = port
        if 'host' not in kwargs:
            kwargs['host'] = host
        if 'expose_tracebacks' not in kwargs:
            kwargs['expose_tracebacks'] = True
        server = cls(application, **kwargs)
        server.runner = threading.Thread(target=server.run)
        server.runner.daemon = True
        server.runner.start()
        return server

    def wait(self, retries=30):
        """Wait until the server is started"""
        running = check_server(self.adj.host, self.adj.port,
                               '/__application__', retries=retries)
        if running:
            return True
        try:
            self.shutdown()
        finally:
            return False
