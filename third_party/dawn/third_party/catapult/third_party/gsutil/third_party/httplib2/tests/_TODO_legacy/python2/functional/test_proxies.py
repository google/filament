from __future__ import print_function
import unittest
import errno
import os
import signal
import subprocess
import tempfile

import nose

import httplib2
from httplib2 import socks
from httplib2.test import miniserver

tinyproxy_cfg = """
User "%(user)s"
Port %(port)s
Listen 127.0.0.1
PidFile "%(pidfile)s"
LogFile "%(logfile)s"
MaxClients 2
StartServers 1
LogLevel Info
"""


class FunctionalProxyHttpTest(unittest.TestCase):
    def setUp(self):
        if not socks:
            raise nose.SkipTest("socks module unavailable")
        if not subprocess:
            raise nose.SkipTest("subprocess module unavailable")

        # start a short-lived miniserver so we can get a likely port
        # for the proxy
        self.httpd, self.proxyport = miniserver.start_server(miniserver.ThisDirHandler)
        self.httpd.shutdown()
        self.httpd, self.port = miniserver.start_server(miniserver.ThisDirHandler)

        self.pidfile = tempfile.mktemp()
        self.logfile = tempfile.mktemp()
        fd, self.conffile = tempfile.mkstemp()
        f = os.fdopen(fd, "w")
        our_cfg = tinyproxy_cfg % {
            "user": os.getlogin(),
            "pidfile": self.pidfile,
            "port": self.proxyport,
            "logfile": self.logfile,
        }
        f.write(our_cfg)
        f.close()
        try:
            # TODO use subprocess.check_call when 2.4 is dropped
            ret = subprocess.call(["tinyproxy", "-c", self.conffile])
            self.assertEqual(0, ret)
        except OSError as e:
            if e.errno == errno.ENOENT:
                raise nose.SkipTest("tinyproxy not available")
            raise

    def tearDown(self):
        self.httpd.shutdown()
        try:
            pid = int(open(self.pidfile).read())
            os.kill(pid, signal.SIGTERM)
        except OSError as e:
            if e.errno == errno.ESRCH:
                print("\n\n\nTinyProxy Failed to start, log follows:")
                print(open(self.logfile).read())
                print("end tinyproxy log\n\n\n")
            raise
        map(os.unlink, (self.pidfile, self.logfile, self.conffile))

    def testSimpleProxy(self):
        proxy_info = httplib2.ProxyInfo(
            socks.PROXY_TYPE_HTTP, "localhost", self.proxyport
        )
        client = httplib2.Http(proxy_info=proxy_info)
        src = "miniserver.py"
        response, body = client.request("http://localhost:%d/%s" % (self.port, src))
        self.assertEqual(response.status, 200)
        self.assertEqual(body, open(os.path.join(miniserver.HERE, src)).read())
        lf = open(self.logfile).read()
        expect = 'Established connection to host "127.0.0.1" ' "using file descriptor"
        self.assertTrue(
            expect in lf, "tinyproxy did not proxy a request for miniserver"
        )
