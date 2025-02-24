# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import subprocess

from telemetry.core import util
from telemetry.internal import forwarders

from devil.android import device_errors
try:
  from devil.android import forwarder
except ImportError as exc:
  # Module is not importable e.g. on Windows hosts. Skip the warning printout to
  # to reduce the noise.
  pass


class AndroidForwarderFactory(forwarders.ForwarderFactory):

  def __init__(self, device):
    super().__init__()
    self._device = device

  def Create(self, local_port, remote_port, reverse=False):
    try:
      if not reverse:
        return AndroidForwarder(self._device, local_port, remote_port)
      return AndroidReverseForwarder(self._device, local_port, remote_port)
    except Exception:
      logging.exception(
          'Failed to map local_port=%r to remote_port=%r (reverse=%r).',
          local_port, remote_port, reverse)
      util.LogExtraDebugInformation(
          self._ListCurrentAdbConnections,
          self._ListWebPageReplayInstances,
          self._ListHostTcpPortsInUse,
          self._ListDeviceTcpPortsInUse,
          self._ListDeviceUnixDomainSocketsInUse,
          self._ListDeviceLsofEntries,
      )
      raise

  def _ListCurrentAdbConnections(self):
    """Current adb connections"""
    return self._device.adb.ForwardList().splitlines()

  def _ListWebPageReplayInstances(self):
    """WebPageReplay instances"""
    lines = subprocess.check_output(['ps', '-ef']).splitlines()
    return (line for line in lines if 'webpagereplay' in line)

  def _ListHostTcpPortsInUse(self):
    """Host tcp ports in use"""
    return subprocess.check_output(['netstat', '-t']).splitlines()

  def _ListDeviceTcpPortsInUse(self):
    """Device tcp ports in use"""
    return self._device.ReadFile('/proc/net/tcp', as_root=True,
                                 force_pull=True).splitlines()

  def _ListDeviceUnixDomainSocketsInUse(self):
    """Device unix domain socets in use"""
    return self._device.ReadFile('/proc/net/unix', as_root=True,
                                 force_pull=True).splitlines()

  def _ListDeviceLsofEntries(self):
    """Device lsof entries"""
    return self._device.RunShellCommand(['lsof'], as_root=True,
                                        check_return=True)


class AndroidForwarder(forwarders.Forwarder):
  """Use host_forwarder to map a known local port with a remote (device) port.

  The remote port may be 0, in such case the forwarder will automatically
  choose an available port.

  See:
  - chromium:/src/tools/android/forwarder2
  - catapult:/devil/devil/android/forwarder.py
  """

  def __init__(self, device, local_port, remote_port):
    super().__init__()
    self._device = device
    assert local_port, 'Local port must be given'
    forwarder.Forwarder.Map([(remote_port or 0, local_port)], self._device)
    remote_port = forwarder.Forwarder.DevicePortForHostPort(local_port)
    self._StartedForwarding(local_port, remote_port)

  def Close(self):
    if self.is_forwarding:
      forwarder.Forwarder.UnmapDevicePort(self.remote_port, self._device)
    super().Close()


class AndroidReverseForwarder(forwarders.Forwarder):
  """Use adb forward to map a known remote (device) port with a local port.

  The local port may be 0, in such case the forwarder will automatically
  choose an available port.

  See:
  - catapult:/devil/devil/android/sdk/adb_wrapper.py
  """

  def __init__(self, device, local_port, remote_port):
    super().__init__()
    self._device = device
    assert remote_port, 'Remote port must be given'
    local_port = int(self._device.adb.Forward('tcp:%d' % (local_port or 0),
                                              remote_port))
    self._StartedForwarding(local_port, remote_port)

  def Close(self):
    if self.is_forwarding:
      # This used to run `adb forward --list` to check that the requested
      # port was actually being forwarded to self._device. Unfortunately,
      # starting in adb 1.0.36, a bug (b/31811775) keeps this from working.
      # For now, try to remove the port forwarding and ignore failures.
      local_address = 'tcp:%d' % self.local_port
      try:
        self._device.adb.ForwardRemove(local_address)
      except device_errors.AdbCommandFailedError:
        logging.critical(
            'Attempted to unforward %s but failed.', local_address)
    super().Close()
