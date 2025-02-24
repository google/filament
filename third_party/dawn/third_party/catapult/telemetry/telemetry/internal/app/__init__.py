# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import time

from telemetry.core import debug_data


class App():
  """ A running application instance that can be controlled in a limited way.

  Be sure to clean up after yourself by calling Close() when you are done with
  the app. Or better yet:
    with possible_app.Create(options) as app:
      ... do all your operations on app here
  """
  def __init__(self, app_backend, platform_backend):
    assert platform_backend.platform is not None
    self._app_backend = app_backend
    self._platform_backend = platform_backend
    self._app_backend.SetApp(self)

  @property
  def app_type(self):
    return self._app_backend.app_type

  @property
  def platform(self):
    return self._platform_backend.platform

  def __enter__(self):
    return self

  def __exit__(self, *args):
    self.Close()

  def Close(self):
    raise NotImplementedError()

  def GetStandardOutput(self):
    return self._app_backend.GetStandardOutput()

  def GetMostRecentMinidumpPath(self):
    return self._app_backend.GetMostRecentMinidumpPath()

  def CollectDebugData(self, log_level):
    """Collect debug data and return it.

    Children should override this to actually collect useful debug data.

    Args:
      log_level: The logging level to use from the logging module, e.g.
          logging.ERROR.

    Returns:
      A debug_data.DebugData object containing the collected data.
    """
    del log_level  # unused
    logging.warning(
        'Crashed app of type %s does not implement the CollectDebugData '
        'method, unable to provide actual debug information', self.app_type)
    return debug_data.DebugData()

  def GetRecentMinidumpPathWithTimeout(self, timeout_s=15, oldest_ts=None):
    """Get a path to a recent minidump, blocking until one is available.

    Similar to GetMostRecentMinidumpPath, but does not assume that any pending
    dumps have been written to disk yet. Instead, waits until a suitably fresh
    minidump is found or the timeout is reached.

    Args:
      timeout_s: The timeout in seconds.
      oldest_ts: The oldest allowable timestamp (in seconds since epoch) that a
          minidump was created at for it to be considered fresh enough to
          return. Defaults to a minute from the current time if not set.

    Return:
      None if the timeout is hit or a str containing the path to the found
      minidump if a suitable one is found.
    """
    if oldest_ts is None:
      oldest_ts = time.time() - 60
    return self._app_backend.GetRecentMinidumpPathWithTimeout(
        timeout_s, oldest_ts)
