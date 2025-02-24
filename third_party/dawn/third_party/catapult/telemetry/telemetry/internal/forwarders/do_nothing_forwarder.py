# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
from telemetry.internal import forwarders



class Error(Exception):
  """Base class for exceptions in this module."""


class PortsMismatchError(Error):
  """Raised when local and remote ports are not equal."""


class DoNothingForwarderFactory(forwarders.ForwarderFactory):

  def Create(self, local_port, remote_port, reverse=False):
    del reverse  # Not relevant in DoNothingForwarder.
    return DoNothingForwarder(local_port, remote_port)


class DoNothingForwarder(forwarders.Forwarder):
  """Check that no forwarding is needed for the given port pairs.

  If either the local or remote port is missing, it is made to match its
  counterpart. At least one of the two must be given, though.

  A PortsMismatchError is raised if local and remote ports are not equal.
  Otherwise, the "do nothing" forwarder does not make sense.
  """

  def __init__(self, local_port, remote_port):
    super().__init__()
    local_port, remote_port = _ValidatePorts(local_port, remote_port)
    self._StartedForwarding(local_port, remote_port)

def _ValidatePorts(local_port, remote_port):
  if not local_port:
    assert remote_port, 'Either local or remote ports must be given'
    local_port = remote_port
  elif not remote_port:
    remote_port = local_port
  elif local_port != remote_port:
    raise PortsMismatchError('Local port forwarding is not supported')
  return (local_port, remote_port)
