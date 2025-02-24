# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import six.moves.BaseHTTPServer # pylint: disable=import-error
import six.moves.SimpleHTTPServer # pylint: disable=import-error

from telemetry import decorators
from telemetry.core import local_server
from telemetry.testing import tab_test_case


class SimpleLocalServerBackendRequestHandler(
    six.moves.SimpleHTTPServer.SimpleHTTPRequestHandler):

  def do_GET(self):
    msg = """<!DOCTYPE html>
<html>
<body>
hello world
</body>
"""

    self.send_response(200)
    self.send_header('Content-Type', 'text/html')
    self.send_header('Content-Length', len(msg))
    self.end_headers()
    self.wfile.write(msg)

  def log_request(self, code='-', size='-'):
    pass


class SimpleLocalServerBackend(six.moves.BaseHTTPServer.HTTPServer,
                               local_server.LocalServerBackend):

  def __init__(self):
    six.moves.BaseHTTPServer.HTTPServer.__init__(
        self, ('127.0.0.1', 0), SimpleLocalServerBackendRequestHandler)
    local_server.LocalServerBackend.__init__(self)

  def StartAndGetNamedPorts(self, args, handler_class=None):
    """See base class for details.

    Make sure the arguments are same as those returned from SimpleLocalServer.

    Args:
      handler_class: None for this test.
    """
    assert 'hello' in args
    assert args['hello'] == 'world'
    return [local_server.NamedPort('http', self.server_address[1])]

  def ServeForever(self):
    self.serve_forever()


class SimpleLocalServer(local_server.LocalServer):

  def __init__(self):
    super().__init__(SimpleLocalServerBackend)

  def GetBackendStartupArgs(self):
    return {'hello': 'world'}


class LocalServerUnittest(tab_test_case.TabTestCase):

  @classmethod
  def setUpClass(cls):
    super(LocalServerUnittest, cls).setUpClass()
    cls._server = SimpleLocalServer()
    cls._platform.StartLocalServer(cls._server)

  @decorators.Disabled('all')  # TODO(crbug.com/799487): Fix and re-enable test.
  def testLocalServer(self):
    self.assertTrue(self._server in self._platform.local_servers)
    self._tab.Navigate(self._server.url)
    self._tab.WaitForDocumentReadyStateToBeComplete()
    body_text = self._tab.EvaluateJavaScript('document.body.textContent')
    body_text = body_text.strip()
    self.assertEqual('hello world', body_text)

  @decorators.Disabled('all')  # TODO(crbug.com/799487): Fix and re-enable test.
  def testStartingAndRestarting(self):
    server2 = SimpleLocalServer()
    self.assertRaises(Exception,
                      lambda: self._platform.StartLocalServer(server2))

    self._server.Close()
    self.assertTrue(self._server not in self._platform.local_servers)

    self._platform.StartLocalServer(server2)
