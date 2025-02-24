# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import tempfile

from telemetry.core import util
from telemetry.internal import forwarders
from telemetry.internal.forwarders import forwarder_utils


class FuchsiaForwarderFactory(forwarders.ForwarderFactory):

  def __init__(self, command_runner):
    super().__init__()
    self._command_runner = command_runner

  def Create(self, local_port, remote_port, reverse=False):
    return FuchsiaSshForwarder(local_port, remote_port,
                               self._command_runner,
                               port_forward=not reverse)


class FuchsiaSshForwarder(forwarders.Forwarder):

  def __init__(self, local_port, remote_port, command_runner, port_forward):
    """Sets up ssh port forwarding betweeen a Fuchsia device and the host.

    Args:
      local_port: Port on the host.
      remote_port: Port on the Fuchsia device.
      command_runner: Contains information related to ssh configuration.
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
        '-o', 'LogLevel=VERBOSE'
    ]
    ssh_args.extend(forwarder_utils.GetForwardingArgs(
        local_port, remote_port, self.host_ip,
        port_forward))

    with tempfile.NamedTemporaryFile() as stderr_file:
      self._proc = command_runner.RunCommandPiped(ssh_args=ssh_args,
                                                  stderr=stderr_file)
      if not remote_port:
        remote_port = forwarder_utils.ReadRemotePort(stderr_file.name)

    self._StartedForwarding(local_port, remote_port)

  def Close(self):
    if self._proc:
      self._proc.kill()
      self._proc = None
    super().Close()
