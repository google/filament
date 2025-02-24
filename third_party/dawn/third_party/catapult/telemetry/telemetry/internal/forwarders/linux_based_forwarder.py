# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import subprocess
import tempfile

from telemetry.core import util
from telemetry.internal import forwarders
from telemetry.internal.forwarders import do_nothing_forwarder
from telemetry.internal.forwarders import forwarder_utils

import py_utils


class LinuxBasedForwarderFactory(forwarders.ForwarderFactory):

  def __init__(self, interface):
    super().__init__()
    self._interface = interface

  def Create(self, local_port, remote_port, reverse=False):
    if self._interface.local:
      return do_nothing_forwarder.DoNothingForwarder(local_port, remote_port)
    return LinuxBasedSshForwarder(
        self._interface, local_port, remote_port, port_forward=not reverse)


class LinuxBasedSshForwarder(forwarders.Forwarder):

  def __init__(self, interface, local_port, remote_port, port_forward):
    super().__init__()
    self._interface = interface
    self._proc = None

    if port_forward:
      assert local_port, 'Local port must be given'
    else:
      assert remote_port, 'Remote port must be given'
      if not local_port:
        # Choose an available port on the host.
        local_port = util.GetUnreservedAvailableLocalPort()

    forwarding_args = [
        # Ensure SSH is at least verbose enough to print the allocated port
        '-o',
        'LogLevel=INFO'
    ]
    forwarding_args.extend(
        forwarder_utils.GetForwardingArgs(local_port, remote_port, self.host_ip,
                                          port_forward))

    # TODO(crbug.com/793256): Consider avoiding the extra tempfile and
    # read stderr directly from the subprocess instead.
    with tempfile.NamedTemporaryFile() as stderr_file:
      self._proc = subprocess.Popen(
          self._interface.FormSSHCommandLine(['-NT'],
                                             forwarding_args,
                                             port_forward=port_forward),
          stdout=subprocess.PIPE,
          stderr=stderr_file,
          stdin=subprocess.PIPE,
          shell=False)
      if not remote_port:
        remote_port = forwarder_utils.ReadRemotePort(stderr_file.name)

    self._StartedForwarding(local_port, remote_port)
    py_utils.WaitFor(self._IsConnectionReady, timeout=60)

  def _IsConnectionReady(self):
    return self._interface.IsHTTPServerRunningOnPort(self.remote_port)

  def Close(self):
    if self._proc:
      self._proc.kill()
      self._proc = None
    super().Close()
