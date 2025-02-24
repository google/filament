# Copyright 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import re

import pexpect # pylint: disable=import-error

from telemetry.core import cast_interface
from telemetry.core import util
from telemetry.internal import forwarders
from telemetry.internal.forwarders import forwarder_utils


class CastForwarderFactory(forwarders.ForwarderFactory):

  def __init__(self, ip_addr):
    super().__init__()
    self._ip_addr = ip_addr

  def Create(self, local_port, remote_port, reverse=False):
    return CastSshForwarder(local_port, remote_port,
                            self._ip_addr, port_forward=not reverse)


class CastSshForwarder(forwarders.Forwarder):

  def __init__(self, local_port, remote_port, ip_addr, port_forward):
    """Sets up ssh port forwarding betweeen a Cast device and the host.

    Args:
      local_port: Port on the host.
      remote_port: Port on the Cast device.
      ip_addr: IP address of the cast receiver.
      port_forward: Determines the direction of the connection."""
    super().__init__()
    self._proc = None

    if port_forward:
      assert local_port, 'Local port must be given'
    else:
      assert remote_port, 'Remote port must be given'
      if not local_port:
        # Choose an available port on the host.
        local_port = util.GetUnreservedAvailableLocalPort()

    ssh_args = [
        '-N',  # Don't execute command
        '-T',  # Don't allocate terminal.
        # Ensure SSH is at least verbose enough to print the allocated port
        '-o', 'LogLevel=VERBOSE',
        '-o', 'StrictHostKeyChecking=no',
        '-o', 'UserKnownHostsFile=/dev/null',
        '-l', cast_interface.SSH_USER
    ]
    ssh_args.extend(forwarder_utils.GetForwardingArgs(
        local_port, remote_port, self.host_ip,
        port_forward))

    self._proc = pexpect.spawn('ssh %s %s' % (' '.join(ssh_args), ip_addr))
    self._proc.expect('.*password:')
    self._proc.sendline(cast_interface.SSH_PWD)
    if not remote_port:
      self._proc.expect('Allocated port [0-9]+ for.*')
      line = self._proc.match.group(0).decode('utf-8')
      tokens = re.search(r'Allocated port (\d+) for', line)
      remote_port = int(tokens.group(1))

    self._StartedForwarding(local_port, remote_port)

  def Close(self):
    if self._proc:
      self._proc.close(force=True)
      self._proc = None
    super().Close()
