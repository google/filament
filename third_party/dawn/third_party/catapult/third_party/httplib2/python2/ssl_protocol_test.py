"""Tests for SSL handling in httplib2."""

import httplib2
import os
import ssl
import sys
import unittest


class TestSslProtocol(unittest.TestCase):
    def testSslCertValidationWithInvalidCaCert(self):
        if sys.version_info >= (2, 6):
            http = httplib2.Http(ca_certs="/nosuchfile")
            if sys.version_info >= (2, 7):
                with self.assertRaises(IOError):
                    http.request("https://www.google.com/", "GET")
            else:
                self.assertRaises(
                    ssl.SSLError, http.request, "https://www.google.com/", "GET"
                )

    def testSslCertValidationWithSelfSignedCaCert(self):
        if sys.version_info >= (2, 7):
            other_ca_certs = os.path.join(
                os.path.dirname(os.path.abspath(httplib2.__file__)),
                "test",
                "other_cacerts.txt",
            )
            http = httplib2.Http(ca_certs=other_ca_certs)
            if sys.platform != "darwin":
                with self.assertRaises(httplib2.SSLHandshakeError):
                    http.request("https://www.google.com/", "GET")

    def testSslProtocolTlsV1AndShouldPass(self):
        http = httplib2.Http(ssl_version=ssl.PROTOCOL_TLSv1)
        urls = [
            "https://www.amazon.com",
            "https://www.apple.com",
            "https://www.twitter.com",
        ]
        for url in urls:
            if sys.version_info >= (2, 7):
                self.assertIsNotNone(http.request(uri=url))

    def testSslProtocolV3AndShouldFailDueToPoodle(self):
        http = httplib2.Http(ssl_version=ssl.PROTOCOL_SSLv3)
        urls = [
            "https://www.amazon.com",
            "https://www.apple.com",
            "https://www.twitter.com",
        ]
        for url in urls:
            if sys.version_info >= (2, 7):
                with self.assertRaises(httplib2.SSLHandshakeError):
                    http.request(url)
                try:
                    http.request(url)
                except httplib2.SSLHandshakeError as e:
                    self.assertTrue("sslv3 alert handshake failure" in str(e))


if __name__ == "__main__":
    unittest.main()
