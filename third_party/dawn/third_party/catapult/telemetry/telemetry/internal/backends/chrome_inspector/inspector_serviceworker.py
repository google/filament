# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.internal.backends.chrome_inspector import inspector_websocket
from telemetry.core import exceptions

class InspectorServiceWorker():
  def __init__(self, inspector_socket, timeout):
    self._websocket = inspector_socket
    self._websocket.RegisterDomain('ServiceWorker', self._OnNotification)
    # ServiceWorker.enable RPC must be called before calling any other methods
    # in ServiceWorker domain.
    res = self._websocket.SyncRequest(
        {'method': 'ServiceWorker.enable'}, timeout)
    if 'error' in res:
      raise exceptions.StoryActionError(res['error']['message'])

  def _OnNotification(self, msg):
    # TODO: track service worker events
    # (https://chromedevtools.github.io/devtools-protocol/tot/ServiceWorker/)
    pass

  def StopAllWorkers(self, timeout):
    res = self._websocket.SyncRequest(
        {'method': 'ServiceWorker.stopAllWorkers'}, timeout)
    if 'error' in res:
      code = res['error']['code']
      if code == inspector_websocket.InspectorWebsocket.METHOD_NOT_FOUND_CODE:
        raise NotImplementedError(
            'DevTools method ServiceWorker.stopAllWorkers is not supported by '
            'this browser.')
      raise exceptions.StoryActionError(res['error']['message'])
