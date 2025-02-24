# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import platform

try:
  from pexpect import pxssh # pylint: disable=import-error
except ImportError as e:
  if platform.system() == 'Windows':
    logging.info('pxssh not supported on Windows')
  pxssh = None

from telemetry.core import cast_interface
from telemetry.core import platform as telemetry_platform
from telemetry.internal.forwarders import cast_forwarder
from telemetry.internal.platform import cast_device
from telemetry.internal.platform import platform_backend


class CastPlatformBackend(platform_backend.PlatformBackend):
  def __init__(self, device):
    super().__init__(device)
    self._ip_addr = None
    self._output_dir = device.output_dir
    self._runtime_exe = device.runtime_exe
    if device.ip_addr:
      self._ip_addr = device.ip_addr

  @classmethod
  def SupportsDevice(cls, device):
    return isinstance(device, cast_device.CastDevice)

  @classmethod
  def CreatePlatformForDevice(cls, device, finder_options):
    assert cls.SupportsDevice(device)
    return telemetry_platform.Platform(CastPlatformBackend(device))

  @property
  def output_dir(self):
    return self._output_dir

  @property
  def runtime_exe(self):
    return self._runtime_exe

  @property
  def ip_addr(self):
    return self._ip_addr

  def _CreateForwarderFactory(self):
    if self._ip_addr:
      return cast_forwarder.CastForwarderFactory(self._ip_addr)
    return super()._CreateForwarderFactory()

  def GetSSHSession(self):
    ssh = pxssh.pxssh(options={
        'StrictHostKeyChecking': 'no',
        'UserKnownHostsFile': '/dev/null',})
    ssh.login(
        self._ip_addr,
        username=cast_interface.SSH_USER,
        password=cast_interface.SSH_PWD)
    return ssh

  def IsRemoteDevice(self):
    return False

  def GetArchName(self):
    return 'Arch type of device not yet supported in Cast'

  def GetOSName(self):
    return 'castos'

  def GetDeviceTypeName(self):
    return 'Cast Device'

  def GetOSVersionName(self):
    return ''

  def GetOSVersionDetailString(self):
    return 'CastOS'

  def GetSystemTotalPhysicalMemory(self):
    raise NotImplementedError()

  def HasBeenThermallyThrottled(self):
    return False

  def IsThermallyThrottled(self):
    return False

  def InstallApplication(self, application):
    raise NotImplementedError()

  def LaunchApplication(self, application, parameters=None,
                        elevate_privilege=False):
    raise NotImplementedError()

  def PathExists(self, path, timeout=None, retries=None):
    raise NotImplementedError()

  def CanFlushIndividualFilesFromSystemCache(self):
    return False

  def FlushEntireSystemCache(self):
    return None

  def FlushSystemCacheForDirectory(self, directory):
    return None

  def StartActivity(self, intent, blocking):
    raise NotImplementedError()

  def CooperativelyShutdown(self, proc, app_name):
    return False

  def SupportFlushEntireSystemCache(self):
    return False

  def StartDisplayTracing(self):
    raise NotImplementedError()

  def StopDisplayTracing(self):
    raise NotImplementedError()

  def TakeScreenshot(self, file_path):
    return None

  def GetTypExpectationsTags(self):
    tags = super().GetTypExpectationsTags()
    tags.append(self.GetDeviceTypeName())
    return tags
