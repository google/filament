# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Start and stop tsproxy."""

from __future__ import absolute_import
from py_utils import ts_proxy_server


# TODO(crbug/1074098): clean up this file and use py_utils.ts_proxy_server.
class TsProxyServer(ts_proxy_server.TsProxyServer):
  """Start and Stop Tsproxy.

  TsProxy provides basic latency, download and upload traffic shaping. This
  class provides a programming API to the tsproxy script in
  telemetry/third_party/tsproxy/tsproxy.py
  """

  def __enter__(self):
    """Add support for with-statement."""
    self.StartServer()
    return self

  def __exit__(self, unused_exc_type, unused_exc_val, unused_exc_tb):
    """Add support for with-statement."""
    self.StopServer()
