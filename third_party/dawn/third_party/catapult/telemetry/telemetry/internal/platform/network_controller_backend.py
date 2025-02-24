# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os

from telemetry.internal.util import webpagereplay_go_server
from telemetry.internal.util import ts_proxy_server
from telemetry.util import wpr_modes


class ArchiveDoesNotExistError(Exception):
  """Raised when the archive path does not exist for replay mode."""


class ReplayAndBrowserPortsError(Exception):
  """Raised an existing browser would get different remote replay ports."""


class NetworkControllerBackend():
  """Control network settings and servers to simulate the Web.

  Network changes include forwarding device ports to host platform ports.
  Web Page Replay is used to record and replay HTTP/HTTPS responses.
  """

  def __init__(self, platform_backend):
    self._platform_backend = platform_backend
    # Controller options --- bracketed by Open/Close
    self._wpr_mode = None
    # Replay options --- bracketed by StartReplay/StopReplay
    self._archive_path = None
    self._make_javascript_deterministic = None
    self._extra_wpr_args = None
    # Network control services
    self._ts_proxy_server = None
    self._forwarder = None
    self._wpr_server = None
    self._open_attempted = False
    self._previous_open_successful = False

  def Open(self, wpr_mode):
    """Get the target platform ready for network control.

    This will both start a TsProxy server and set up a forwarder to it.

    If options are compatible and the controller is already open, it will
    try to re-use the existing server and forwarder.

    After network interactions are over, clients should call the Close method.

    Args:
      wpr_mode: a mode for web page replay; available modes are
          wpr_modes.WPR_OFF, wpr_modes.APPEND, wpr_modes.WPR_REPLAY, or
          wpr_modes.WPR_RECORD. Setting wpr_modes.WPR_OFF configures the
          network controller to use live traffic.
    """
    self._open_attempted = True
    self._previous_open_successful = False
    if self.is_open:
      use_live_traffic = wpr_mode == wpr_modes.WPR_OFF
      if self.use_live_traffic != use_live_traffic:
        self.Close()  # Need to restart the current TsProxy and forwarder.
      else:
        if self._wpr_mode != wpr_mode:
          self.StopReplay()  # Need to restart the WPR server, if any.
          self._wpr_mode = wpr_mode
        self._previous_open_successful = True
        return

    self._wpr_mode = wpr_mode
    try:
      local_port = self._StartTsProxyServer()
      self._forwarder = self._platform_backend.forwarder_factory.Create(
          local_port=local_port, remote_port=None)
    except Exception:
      self.Close()
      raise
    self._previous_open_successful = True

  @property
  def is_open(self):
    return self._ts_proxy_server is not None

  @property
  def is_intentionally_closed(self):
    # We consider the server to be intentionally closed if it isn't open and
    # either we've never attempted to open the server before or the previous
    # open attempt was successful, in which case we assume that the reason we
    # are not currently open is because something explicitly told the server to
    # close and nothing has tried to open it since.
    return (not self.is_open and (not self._open_attempted
                                  or self._previous_open_successful))

  @property
  def use_live_traffic(self):
    return self._wpr_mode == wpr_modes.WPR_OFF

  @property
  def host_ip(self):
    return self._platform_backend.forwarder_factory.host_ip

  @property
  def wpr_mode(self):
    return self._wpr_mode

  def Close(self):
    """Undo changes in the target platform used for network control.

    Implicitly stops replay if currently active.
    """
    self.StopReplay()
    self._StopForwarder()
    self._StopTsProxyServer()
    self._wpr_mode = None

  def StartReplay(self, archive_path, make_javascript_deterministic,
                  extra_wpr_args):
    """Start web page replay from a given replay archive.

    Starts as needed, and reuses if possible, the replay server on the host.

    Implementation details
    ----------------------

    The local host is where Telemetry is run. The remote is host where
    the target application is run. The local and remote hosts may be
    the same (e.g., testing a desktop browser) or different (e.g., testing
    an android browser).

    A replay server is started on the local host using the local ports, while
    a forwarder ties the local to the remote ports.

    Both local and remote ports may be zero. In that case they are determined
    by the replay server and the forwarder respectively. Setting dns to None
    disables DNS traffic.

    Args:
      archive_path: a path to a specific WPR archive.
      make_javascript_deterministic: True if replay should inject a script
          to make JavaScript behave deterministically (e.g., override Date()).
      extra_wpr_args: a tuple with any extra args to send to the WPR server.
    """
    assert self.is_open, 'Network controller is not open'
    if self.use_live_traffic:
      return
    if not archive_path:
      # TODO(slamm, tonyg): Ideally, replay mode should be stopped when there is
      # no archive path. However, if the replay server already started, and
      # a file URL is tested with the
      # telemetry.core.local_server.LocalServerController, then the
      # replay server forwards requests to it. (Chrome is configured to use
      # fixed ports fo all HTTP/HTTPS requests.)
      return
    if (self._wpr_mode == wpr_modes.WPR_REPLAY and
        not os.path.exists(archive_path)):
      raise ArchiveDoesNotExistError(
          'Archive path does not exist: %s' % archive_path)
    if (self._wpr_server is not None and
        self._archive_path == archive_path and
        self._make_javascript_deterministic == make_javascript_deterministic and
        self._extra_wpr_args == extra_wpr_args):
      return  # We may reuse the existing replay server.

    self._archive_path = archive_path
    self._make_javascript_deterministic = make_javascript_deterministic
    self._extra_wpr_args = extra_wpr_args
    local_ports = self._StartReplayServer()
    self._ts_proxy_server.UpdateOutboundPorts(
        http_port=local_ports['http'], https_port=local_ports['https'])

  def StopReplay(self):
    """Stop web page replay.

    Stops the replay server if currently active.
    """
    self._StopReplayServer()
    self._archive_path = None
    self._make_javascript_deterministic = None
    self._extra_wpr_args = None

  def _StartReplayServer(self):
    """Start the replay server and return the started local_ports."""
    self._StopReplayServer()  # In case it was already running.
    self._wpr_server = webpagereplay_go_server.ReplayServer(
        self._archive_path,
        self.host_ip,
        http_port=0,
        https_port=0,
        replay_options=self._ReplayCommandLineArgs())
    return self._wpr_server.StartServer()

  def _StopReplayServer(self):
    """Stop the replay server only."""
    if self._wpr_server:
      self._wpr_server.StopServer()
      self._wpr_server = None

  def _StopForwarder(self):
    if self._forwarder:
      self._forwarder.Close()
      self._forwarder = None

  def _StopTsProxyServer(self):
    """Stop the replay server only."""
    if self._ts_proxy_server:
      self._ts_proxy_server.StopServer()
      self._ts_proxy_server = None

  def _ReplayCommandLineArgs(self):
    wpr_args = list(self._extra_wpr_args)
    if self._wpr_mode == wpr_modes.WPR_APPEND:
      wpr_args.append('--append')
    elif self._wpr_mode == wpr_modes.WPR_RECORD:
      wpr_args.append('--record')
    if not self._make_javascript_deterministic:
      wpr_args.append('--inject_scripts=')
    return wpr_args

  def _StartTsProxyServer(self):
    assert not self._ts_proxy_server, 'ts_proxy_server is already started'
    host_ip = None if self.use_live_traffic else self.host_ip
    self._ts_proxy_server = ts_proxy_server.TsProxyServer(host_ip=host_ip)
    self._ts_proxy_server.StartServer()
    return self._ts_proxy_server.port

  @property
  def forwarder(self):
    return self._forwarder

  @property
  def ts_proxy_server(self):
    return self._ts_proxy_server
