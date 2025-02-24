# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import traceback

from telemetry.internal.backends.chrome_inspector import inspector_websocket
from telemetry.internal.backends.chrome_inspector import websocket


class NativeProfilingTimeoutException(Exception):
  pass


class NativeProfilingUnrecoverableException(Exception):
  pass


class NativeProfilingUnexpectedResponseException(Exception):
  pass


class NativeProfilingBackend():

  def __init__(self, inspector_socket):
    self._inspector_websocket = inspector_socket

  def DumpProfilingDataOfAllProcesses(self, timeout=120):
    """Causes all profiling data of all Chrome processes to be dumped to disk.
    """
    method = 'NativeProfiling.dumpProfilingDataOfAllProcesses'
    request = {'method': method}
    try:
      logging.warning('Requesting PGO profiles to be dumped')
      unused_response = self._inspector_websocket.SyncRequest(request, timeout)
      logging.warning('PGO profile dump done')
    except inspector_websocket.WebSocketException as err:
      if issubclass(
          err.websocket_error_type, websocket.WebSocketTimeoutException):
        raise NativeProfilingTimeoutException(
            'Exception raised while sending a %s request:\n%s' %
            (method, traceback.format_exc())) from err
      raise NativeProfilingUnrecoverableException(
          'Exception raised while sending a %s request:\n%s' %
          (method, traceback.format_exc())) from err

  def Close(self):
    self._inspector_websocket = None
