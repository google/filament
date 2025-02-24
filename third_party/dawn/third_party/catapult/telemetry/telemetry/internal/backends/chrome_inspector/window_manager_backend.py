# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import json

class WindowManagerException(Exception):
  pass


class WindowManagerBackend():

  _WINDOW_MANAGER_DOMAIN = 'WindowManager'

  def __init__(self, inspector_socket):
    self._inspector_websocket = inspector_socket

  def EnterOverviewMode(self, timeout=30):
    request = {'method': self._WINDOW_MANAGER_DOMAIN + '.enterOverviewMode'}
    response = self._inspector_websocket.SyncRequest(request, timeout)
    if 'error' in response:
      raise WindowManagerException('Inspector returned an error: %s' %
                                   json.dumps(response, indent=2))

  def ExitOverviewMode(self, timeout=30):
    request = {'method': self._WINDOW_MANAGER_DOMAIN + '.exitOverviewMode'}
    response = self._inspector_websocket.SyncRequest(request, timeout)
    if 'error' in response:
      raise WindowManagerException('Inspector returned an error: %s' %
                                   json.dumps(response, indent=2))

  def Close(self):
    pass
