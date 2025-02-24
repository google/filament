# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
try:
  from StringIO import StringIO
except ImportError:
  from io import StringIO

from telemetry.internal.backends.chrome_inspector import websocket


class InspectorConsole():
  def __init__(self, inspector_websocket):
    self._inspector_websocket = inspector_websocket
    self._inspector_websocket.RegisterDomain('Console', self._OnNotification)
    self._message_output_stream = None
    self._last_message = None
    self._console_enabled = False

  def _OnNotification(self, msg):
    if msg['method'] == 'Console.messageAdded':
      assert self._message_output_stream
      message = msg['params']['message']
      if message.get('url') == 'chrome://newtab/':
        return
      self._last_message = '(%s) %s:%i: %s' % (
          message.get('level', 'unknown_log_level'),
          message.get('url', 'unknown_url'),
          message.get('line', 0),
          message.get('text', 'no text provided by console message'))
      self._message_output_stream.write(
          '%s\n' % self._last_message)

  def GetCurrentConsoleOutputBuffer(self, timeout=10):
    self._message_output_stream = StringIO()
    self._EnableConsoleOutputStream(timeout)
    try:
      self._inspector_websocket.DispatchNotifications(timeout)
      return self._message_output_stream.getvalue()
    except websocket.WebSocketTimeoutException:
      return self._message_output_stream.getvalue()
    finally:
      self._DisableConsoleOutputStream(timeout)
      self._message_output_stream.close()
      self._message_output_stream = None


  def _EnableConsoleOutputStream(self, timeout):
    self._inspector_websocket.SyncRequest({'method': 'Console.enable'}, timeout)

  def _DisableConsoleOutputStream(self, timeout):
    self._inspector_websocket.SyncRequest(
        {'method': 'Console.disable'}, timeout)
