# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import logging


class InspectorLog():
  def __init__(self, inspector_websocket):
    """Enables the Log domain of DevTools protocol.

    This class subscribes to DevTools log entries and forwards error entries
    to python logging system as warnings.

    See https://chromedevtools.github.io/devtools-protocol/1-3/Log
    """

    self._inspector_websocket = inspector_websocket
    self._inspector_websocket.RegisterDomain('Log', self._OnMessage)
    self._Enable()

  def _OnMessage(self, message):
    if message['method'] == 'Log.entryAdded':
      entry = message['params']['entry']
      if entry['level'] == 'error':
        # External log messages may contain non-ASCII characters.
        text = entry['text'].encode('ascii', 'backslashreplace').decode('ascii')
        logging.warning('DevTools console [%s]: %s %s',
                        entry['source'], text, entry.get('url', ''))

  def _Enable(self, timeout=60):
    try:
      self._inspector_websocket.SyncRequest({'method': 'Log.enable'}, timeout)
    except:
      # This is the first DevTools call typically made to a page, so an
      # exception indicates the renderer may be hung. Attempt to crash it so we
      # can see all threads' stacks. (The request comes in on the IO thread,
      # which is usually not blocked.)
      #
      # TODO(crbug.com/917211): consider removing once this bug is diagnosed.
      logging.error('Renderer process seems to have hung during browser start '
                    'and Log.enable DevTools call; forcibly crashing it.')
      self._inspector_websocket.SyncRequest({'method': 'Page.crash'}, timeout)
      raise
