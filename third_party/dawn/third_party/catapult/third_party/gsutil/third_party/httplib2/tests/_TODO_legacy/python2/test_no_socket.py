"""Tests for httplib2 when the socket module is missing.

This helps ensure compatibility with environments such as AppEngine.
"""
import os
import sys
import unittest

import httplib2


class MissingSocketTest(unittest.TestCase):
    def setUp(self):
        self._oldsocks = httplib2.socks
        httplib2.socks = None

    def tearDown(self):
        httplib2.socks = self._oldsocks

    def testProxyDisabled(self):
        proxy_info = httplib2.ProxyInfo("blah", "localhost", 0)
        client = httplib2.Http(proxy_info=proxy_info)
        self.assertRaises(
            httplib2.ProxiesUnavailableError, client.request, "http://localhost:-1/"
        )
