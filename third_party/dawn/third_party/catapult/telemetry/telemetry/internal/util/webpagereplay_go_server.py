# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Start and stop Web Page Replay."""

from __future__ import absolute_import
import logging

from telemetry.internal.util import binary_manager

from py_utils import webpagereplay_go_server


class ReplayServerNotStartedError(
    webpagereplay_go_server.ReplayNotStartedError):
  pass

# TODO(crbug/1074047): remove this class entirely.
class ReplayServer():
  """Start and Stop Web Page Replay.

  Web Page Replay is a proxy that can record and "replay" web pages with
  simulated network characteristics -- without having to edit the pages
  by hand. With WPR, tests can use "real" web content, and catch
  performance issues that may result from introducing network delays and
  bandwidth throttling.

  Example:
     with ReplayServer(archive_path):
       self.NavigateToURL(start_url)
       self.WaitUntil(...)
  """

  _go_binary_path = None

  def __init__(self, archive_path, replay_host, http_port, https_port,
               replay_options):
    """Initialize ReplayServer.

    Args:
      archive_path: a path to a specific WPR archive (required).
      replay_host: the hostname to serve traffic.
      http_port: an integer port on which to serve HTTP traffic. May be zero
          to let the OS choose an available port.
      https_port: an integer port on which to serve HTTPS traffic. May be zero
          to let the OS choose an available port.
      replay_options: an iterable of option strings to forward to replay.py
    """
    if binary_manager.NeedsInit():
      binary_manager.InitDependencyManager(None)
    self._wpr_server = webpagereplay_go_server.ReplayServer(
        archive_path, replay_host, http_port, https_port,
        replay_options, binary_manager.FetchPath)

  @property
  def http_port(self):
    return self._wpr_server.http_port

  @property
  def https_port(self):
    return self._wpr_server.https_port

  def StartServer(self):
    """Start Web Page Replay and verify that it started.

    Returns:
      A dictionary mapping the keys 'http', 'https', and (if used) 'dns'
      to the respective ports of the replay server.
    Raises:
      ReplayServerNotStartedError: if Replay start-up fails.
    """
    try:
      return self._wpr_server.StartServer()
    except webpagereplay_go_server.ReplayNotStartedError as e:
      raise ReplayServerNotStartedError(
          'Web Page Replay Server failed to start.') from e

  def StopServer(self, log_level=logging.DEBUG):
    """Stop Web Page Replay.

    This also attempts to return stdout/stderr logs of wpr process if there is
    any. If there is none, '(N/A)' string is returned (see _LogLines()
    implementation).
    """
    return self._wpr_server.StopServer(log_level)

  def __enter__(self):
    """Add support for with-statement."""
    self.StartServer()
    return self

  def __exit__(self, unused_exc_type, unused_exc_val, unused_exc_tb):
    """Add support for with-statement."""
    self.StopServer()
