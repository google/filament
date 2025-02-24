# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging

from py_utils import atexit_with_log


class ForwarderFactory():

  def Create(self, local_port, remote_port, reverse=False):
    """Creates a forwarder to map a local (host) with a remote (device) port.

    By default this means mapping a known local_port with a remote_port. If the
    remote_port is missing (e.g. 0 or None) then the forwarder will choose an
    available port on the device.

    Conversely, when reverse=True, a known remote_port is mapped to a
    local_port and, if this is missing, then the forwarder will choose an
    available port on the host.

    Args:
      local_port: An http port on the local host.
      remote_port: An http port on the remote device.
      reverse: A Boolean indicating the direction of the mapping.
    """
    raise NotImplementedError()

  @property
  def host_ip(self):
    return '127.0.0.1'


class Forwarder():

  def __init__(self):
    self._local_port = None
    self._remote_port = None
    # Prefer atexit_with_log over __del__ to avoid deadlock hangs, see:
    # https://crbug.com/41491803#comment32
    atexit_with_log.Register(self.Close)

  def _StartedForwarding(self, local_port, remote_port):
    assert not self.is_forwarding, 'forwarder has already started'
    assert local_port and remote_port, 'ports should now be determined'

    self._local_port = local_port
    self._remote_port = remote_port
    logging.info('%s started between %s:%s and %s', type(self).__name__,
                 self.host_ip, self.local_port, self.remote_port)

  @property
  def is_forwarding(self):
    return self._local_port is not None

  @property
  def host_ip(self):
    return '127.0.0.1'

  @property
  def local_port(self):
    return self._local_port

  @property
  def remote_port(self):
    return self._remote_port

  def Close(self):
    self._local_port = None
    self._remote_port = None
