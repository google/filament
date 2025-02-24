# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import socket

from telemetry.internal.backends.chrome_inspector import devtools_http
from telemetry.internal.backends.chrome_inspector import inspector_websocket

# These are possible exceptions raised when the DevTools agent is not ready
# to accept incomming connections.
_DEVTOOLS_CONNECTION_ERRORS = (
    devtools_http.DevToolsClientConnectionError,
    inspector_websocket.WebSocketException,
    socket.error)


def GetUIDevtoolsBackend(port, app_backend, browser_target='/0'):
  client = UIDevToolsClientBackend(app_backend)
  try:
    client.Connect(port, browser_target)
    logging.info('DevTools agent connected at %s', client)
    # Enable UI DevTools agents. This is required on Mac so we will be
    # notified about future updates.
    client.Enable()
    client.GetDocument()
  except _DEVTOOLS_CONNECTION_ERRORS as exc:
    logging.info('DevTools agent at %s not ready yet: %s', client, exc)
    client = None
  return client


class UIDevToolsClientBackend():
  """Backend for UIDevTools

  Protocol definition:
  https://source.chromium.org/chromium/chromium/src/+/master:components/ui_devtools/protocol.json
  """
  def __init__(self, app_backend):
    """Create an object able to connect with the DevTools agent.

    Args:
      app_backend: The app that contains the DevTools agent.
    """
    self._app_backend = app_backend
    self._browser_target = None
    self._forwarder = None
    self._devtools_http = None
    self._browser_websocket = None
    self._local_port = None
    self._remote_port = None

  def Connect(self, devtools_port, browser_target):
    try:
      self._Connect(devtools_port, browser_target)
    except:
      self.Close()  # Close any connections made if failed to connect to all.
      raise

  def _Connect(self, devtools_port, browser_target):
    self._browser_target = browser_target or '/devtools/browser'
    self._SetUpPortForwarding(devtools_port)

    # Ensure that the inspector websocket is ready. This may raise a
    # inspector_websocket.WebSocketException or socket.error if not ready.
    self._browser_websocket = inspector_websocket.InspectorWebsocket()
    self._browser_websocket.Connect(self.browser_target_url, timeout=10)

  def _SetUpPortForwarding(self, devtools_port):
    self._forwarder = self.platform_backend.forwarder_factory.Create(
        local_port=None,  # Forwarder will choose an available port.
        remote_port=devtools_port, reverse=True)
    self._local_port = self._forwarder._local_port
    self._remote_port = self._forwarder._remote_port
    self._devtools_http = devtools_http.DevToolsHttp(self.local_port)

  def Close(self):
    # Close the DevTools connections last (in case the backends above still
    # need to interact with them while closing).
    if self._browser_websocket is not None:
      self._browser_websocket.Disconnect()
      self._browser_websocket = None
    if self._devtools_http is not None:
      self._devtools_http.Disconnect()
      self._devtools_http = None
    if self._forwarder is not None:
      self._forwarder.Close()
      self._forwarder = None

  def __str__(self):
    s = self.browser_target_url
    if self.local_port != self.remote_port:
      s = '%s (remote=%s)' % (s, self.remote_port)
    return s

  @property
  def local_port(self):
    return self._local_port

  @property
  def remote_port(self):
    return self._remote_port

  @property
  def browser_target_url(self):
    return 'ws://127.0.0.1:%i%s' % (self._local_port, self._browser_target)

  @property
  def platform_backend(self):
    return self._app_backend.platform_backend

  def QueryNodes(self, query):
    response = self.PerformSearch(query)
    count = response['result']['resultCount']
    logging.info('Found %d results for %s', count, query)
    if count == 0:
      return []
    response = self.GetSearchResults(response['result']['searchId'], 0, count)
    return response['result']['nodeIds']

  def Enable(self):
    request = {
        'method': 'DOM.enable',
    }
    return self._browser_websocket.SyncRequest(request, timeout=30)

  def GetDocument(self):
    request = {
        'method': 'DOM.getDocument',
    }
    return self._browser_websocket.SyncRequest(request, timeout=60)

  def PerformSearch(self, query):
    request = {
        'method': 'DOM.performSearch',
        'params': {
            'query': query,
        }
    }
    return self._browser_websocket.SyncRequest(request, timeout=60)

  def GetSearchResults(self, search_id, from_index, to_index):
    request = {
        'method': 'DOM.getSearchResults',
        'params': {
            'searchId': search_id,
            'fromIndex': from_index,
            'toIndex': to_index,
        }
    }
    return self._browser_websocket.SyncRequest(request, timeout=60)

  # pylint: disable=redefined-builtin
  def DispatchMouseEvent(self,
                         node_id,
                         type,
                         x,
                         y,
                         button,
                         wheel_direction):
    request = {
        'method': 'DOM.dispatchMouseEvent',
        'params': {
            'nodeId': node_id,
            'event': {
                'type': type,
                'x': x,
                'y': y,
                'button': button,
                'wheelDirection': wheel_direction,
            },
        }
    }
    return self._browser_websocket.SyncRequest(request, timeout=60)

  # pylint: disable=redefined-builtin
  def DispatchKeyEvent(self,
                       node_id,
                       type,
                       key_code,
                       code,
                       flags,
                       key,
                       is_char):
    request = {
        'method': 'DOM.dispatchKeyEvent',
        'params': {
            'nodeId': node_id,
            'event': {
                'type': type,
                'keyCode': key_code,
                'code': code,
                'flags': flags,
                'key': key,
                'isChar': is_char,
            },
        }
    }
    return self._browser_websocket.SyncRequest(request, timeout=60)
