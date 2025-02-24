# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import six

from py_trace_event import trace_event
from telemetry.util import wpr_modes


class NetworkController(
    six.with_metaclass(trace_event.TracedMetaClass, object)):
  """Control network settings and servers to simulate the Web.

  Network changes include forwarding device ports to host platform ports.
  Web Page Replay is used to record and replay HTTP/HTTPS responses.
  """

  def __init__(self, network_controller_backend):
    self._network_controller_backend = network_controller_backend

  @property
  def is_open(self):
    return self._network_controller_backend.is_open

  def Open(self, wpr_mode=None):
    if wpr_mode is None:
      wpr_mode = wpr_modes.WPR_REPLAY
    self._network_controller_backend.Open(wpr_mode)

  def Close(self):
    self._network_controller_backend.Close()

  def UpdateTrafficSettings(self,
                            round_trip_latency_ms=None,
                            download_bandwidth_kbps=None,
                            upload_bandwidth_kbps=None):
    self._network_controller_backend.ts_proxy_server.UpdateTrafficSettings(
        round_trip_latency_ms, download_bandwidth_kbps, upload_bandwidth_kbps)

  def StartReplay(self, archive_path, make_javascript_deterministic=False,
                  extra_wpr_args=None):
    extra_wpr_args = tuple(extra_wpr_args) if extra_wpr_args else ()
    self._network_controller_backend.StartReplay(
        archive_path, make_javascript_deterministic, extra_wpr_args)

  def StopReplay(self):
    self._network_controller_backend.StopReplay()
