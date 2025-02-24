# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import subprocess

from telemetry.core import platform as telemetry_platform
from telemetry.core.fuchsia_interface import CommandRunner
from telemetry.internal.forwarders import fuchsia_forwarder
from telemetry.internal.platform import fuchsia_device
from telemetry.internal.platform import platform_backend


class FuchsiaPlatformBackend(platform_backend.PlatformBackend):
  def __init__(self, device):
    super().__init__(device)
    self._command_runner = CommandRunner(device.target_id)
    self._detailed_os_version = None
    self._device_type = None

  @classmethod
  def SupportsDevice(cls, device):
    return isinstance(device, fuchsia_device.FuchsiaDevice)

  @classmethod
  def CreatePlatformForDevice(cls, device, finder_options):
    assert cls.SupportsDevice(device)
    return telemetry_platform.Platform(FuchsiaPlatformBackend(device))

  @property
  def command_runner(self):
    return self._command_runner

  def GetSystemLog(self):
    return None

  def _CreateForwarderFactory(self):
    return fuchsia_forwarder.FuchsiaForwarderFactory(self._command_runner)

  def IsRemoteDevice(self):
    return True

  def GetArchName(self):
    return 'Arch type of device not yet supported in Fuchsia'

  def GetOSName(self):
    return 'fuchsia'

  def GetDeviceTypeName(self):
    if not self._device_type:
      _, self._device_type, _ = self._command_runner.RunCommand(
          ['cat', '/config/build-info/board'],
          stdout=subprocess.PIPE,
          stderr=subprocess.PIPE)
    # In case anything goes wrong with the above command, make the error sound
    # and clear.
    assert self._device_type

    # Fuchsia changed its qemu-x64 board's name to x64, but for the sake of
    # consistency we will still label it as qemu-x64
    if self._device_type == 'x64':
      self._device_type = 'qemu-x64'
    return 'fuchsia-board-' + self._device_type

  def GetOSVersionName(self):
    return ''  # TODO(crbug.com/1140869): Implement this.

  def GetOSVersionDetailString(self):
    if not self._detailed_os_version:
      _, self._detailed_os_version, _ = self._command_runner.RunCommand(
          ['cat', '/config/build-info/version'],
          stdout=subprocess.PIPE,
          stderr=subprocess.PIPE)
    return self._detailed_os_version

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
