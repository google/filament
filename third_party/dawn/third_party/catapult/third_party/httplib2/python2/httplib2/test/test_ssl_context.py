#!/usr/bin/env python2
from __future__ import print_function
import BaseHTTPServer
import logging
import os.path
import ssl
import sys
import unittest

import httplib2
from httplib2.test import miniserver

logger = logging.getLogger(__name__)


class KeepAliveHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    """Request handler that keeps the HTTP connection open, so that the test can inspect the resulting SSL connection object

    """

    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-Length", "0")
        self.send_header("Connection", "keep-alive")
        self.end_headers()

        self.close_connection = 0

    def log_message(self, s, *args):
        # output via logging so nose can catch it
        logger.info(s, *args)


class HttpsContextTest(unittest.TestCase):
    def setUp(self):
        if sys.version_info < (2, 7, 9):
            if hasattr(self, "skipTest"):
                self.skipTest("SSLContext requires Python 2.7.9")
            else:
                return

        self.ca_certs_path = os.path.join(os.path.dirname(__file__), "server.pem")
        self.httpd, self.port = miniserver.start_server(KeepAliveHandler, True)

    def tearDown(self):
        self.httpd.shutdown()

    def testHttpsContext(self):
        client = httplib2.Http(ca_certs=self.ca_certs_path)

        # Establish connection to local server
        client.request("https://localhost:%d/" % (self.port))

        # Verify that connection uses a TLS context with the correct hostname
        conn = client.connections["https:localhost:%d" % self.port]

        self.assertIsInstance(conn.sock, ssl.SSLSocket)
        self.assertTrue(hasattr(conn.sock, "context"))
        self.assertIsInstance(conn.sock.context, ssl.SSLContext)
        self.assertTrue(conn.sock.context.check_hostname)
        self.assertEqual(conn.sock.server_hostname, "localhost")
        self.assertEqual(conn.sock.context.verify_mode, ssl.CERT_REQUIRED)
        self.assertEqual(conn.sock.context.protocol, ssl.PROTOCOL_SSLv23)

    def test_ssl_hostname_mismatch_repeat(self):
        # https://github.com/httplib2/httplib2/issues/5

        # FIXME(temoto): as of 2017-01-05 this is only a reference code, not useful test.
        # Because it doesn't provoke described error on my machine.
        # Instead `SSLContext.wrap_socket` raises `ssl.CertificateError`
        # which was also added to original patch.

        # url host is intentionally different, we provoke ssl hostname mismatch error
        url = "https://127.0.0.1:%d/" % (self.port,)
        http = httplib2.Http(ca_certs=self.ca_certs_path, proxy_info=None)

        def once():
            try:
                http.request(url)
                assert False, "expected certificate hostname mismatch error"
            except Exception as e:
                print("%s errno=%s" % (repr(e), getattr(e, "errno", None)))

        once()
        once()
