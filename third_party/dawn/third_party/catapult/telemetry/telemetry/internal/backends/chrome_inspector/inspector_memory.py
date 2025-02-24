# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import json

from telemetry.core import exceptions


class InspectorMemoryException(exceptions.Error):
  pass


class InspectorMemory():
  """Communicates with the remote inspector's Memory domain."""

  def __init__(self, inspector_websocket):
    self._inspector_websocket = inspector_websocket
    self._inspector_websocket.RegisterDomain('Memory', self._OnNotification)

  def _OnNotification(self, msg):
    pass

  def GetDOMCounters(self, timeout):
    """Retrieves DOM element counts.

    Args:
      timeout: The number of seconds to wait for the inspector backend to
          service the request before timing out.

    Returns:
      A dictionary containing the counts associated with "nodes", "documents",
      and "jsEventListeners".
    Raises:
      InspectorMemoryException
      inspector_websocket.WebSocketException
      socket.error
      exceptions.WebSocketDisconnected
    """
    res = self._inspector_websocket.SyncRequest({
        'method': 'Memory.getDOMCounters'
    }, timeout)
    if ('result' not in res or
        'nodes' not in res['result'] or
        'documents' not in res['result'] or
        'jsEventListeners' not in res['result']):
      raise InspectorMemoryException(
          'Inspector returned unexpected result for Memory.getDOMCounters:\n' +
          json.dumps(res, indent=2))
    return {
        'nodes': res['result']['nodes'],
        'documents': res['result']['documents'],
        'jsEventListeners': res['result']['jsEventListeners']
    }

  def PrepareForLeakDetection(self, timeout):
    """Prepares for Leak Detection by terminating workers, stopping
       spellcheckers, running garbage collections etc.

    Args:
      timeout: The number of seconds to wait for the inspector backend to
          service the request before timing out.
    """
    self._inspector_websocket.SyncRequest({
        'method': 'Memory.prepareForLeakDetection'
    }, timeout)
