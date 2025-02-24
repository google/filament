import os
import unittest

import httplib2

from httplib2.test import miniserver


class HttpSmokeTest(unittest.TestCase):
    def setUp(self):
        self.httpd, self.port = miniserver.start_server(miniserver.ThisDirHandler)

    def tearDown(self):
        self.httpd.shutdown()

    def testGetFile(self):
        client = httplib2.Http()
        src = "miniserver.py"
        response, body = client.request("http://localhost:%d/%s" % (self.port, src))
        self.assertEqual(response.status, 200)
        self.assertEqual(body, open(os.path.join(miniserver.HERE, src)).read())
