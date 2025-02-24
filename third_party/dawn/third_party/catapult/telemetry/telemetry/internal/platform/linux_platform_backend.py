# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import os
import platform
import shlex
import subprocess
import sys

from PIL import ImageGrab  # pylint: disable=import-error

from py_utils import cloud_storage  # pylint: disable=import-error

from telemetry.internal.util import binary_manager
from telemetry.core import os_version
from telemetry.core import util
from telemetry import decorators
from telemetry.internal.platform import linux_based_platform_backend
from telemetry.internal.platform import posix_platform_backend


_POSSIBLE_PERFHOST_APPLICATIONS = [
    'perfhost_precise',
    'perfhost_trusty',
]


def _GetOSVersion(value):
  if value == 'rodete':
    return 0.0

  try:
    return float(value)
  except (TypeError, ValueError):
    logging.error('Unrecognizable OS version: %s. Will fallback to 0.0', value)
    return 0.0


class LinuxPlatformBackend(
    posix_platform_backend.PosixPlatformBackend,
    linux_based_platform_backend.LinuxBasedPlatformBackend):
  def __init__(self):
    super().__init__()

  @classmethod
  def IsPlatformBackendForHost(cls):
    return sys.platform.startswith('linux') and not util.IsRunningOnCrosDevice()

  def IsThermallyThrottled(self):
    raise NotImplementedError()

  def HasBeenThermallyThrottled(self):
    raise NotImplementedError()

  @decorators.Cache
  def GetArchName(self):
    return platform.machine()

  def GetOSName(self):
    return 'linux'

  def _ReadReleaseFile(self, file_path):
    if not os.path.exists(file_path):
      return None

    release_data = {}
    for line in self.GetFileContents(file_path).splitlines():
      key, _, value = line.partition('=')
      release_data[key] = ' '.join(shlex.split(value.strip()))
    return release_data

  @decorators.Cache
  def GetOSVersionName(self):
    # First try os-release(5).
    for path in ('/etc/os-release', '/usr/lib/os-release'):
      os_release = self._ReadReleaseFile(path)
      if os_release:
        codename = os_release.get('ID', 'linux')
        version = _GetOSVersion(os_release.get('VERSION_ID'))
        return os_version.OSVersion(codename, version)

    # Use lsb-release as a fallback.
    lsb_release = self._ReadReleaseFile('/etc/lsb-release')
    if lsb_release:
      codename = lsb_release.get('DISTRIB_CODENAME')
      version = _GetOSVersion(lsb_release.get('DISTRIB_RELEASE'))
      return os_version.OSVersion(codename, version)

    raise NotImplementedError('Unknown Linux OS version')

  def GetOSVersionDetailString(self):
    # First try os-release
    for path in ('/etc/os-release', '/usr/lib/os-release'):
      os_release = self._ReadReleaseFile(path)
      if os_release:
        codename = os_release.get('NAME', 'unknown_codename')
        version = os_release.get('VERSION', 'unknown_version')
        return codename + ' ' + version

    # Use lsb-release as a fallback.
    lsb_release = self._ReadReleaseFile('/etc/lsb-release')
    if lsb_release:
      return lsb_release.get('DISTRIB_DESCRIPTION')

    raise NotImplementedError('Missing Linux OS name or version')

  def CanTakeScreenshot(self):
    return True

  def TakeScreenshot(self, file_path):
    image = ImageGrab.grab(xdisplay=os.environ['DISPLAY'])
    with open(file_path, 'wb') as f:
      image.save(f, 'PNG')
    return True

  def CanFlushIndividualFilesFromSystemCache(self):
    return True

  def SupportFlushEntireSystemCache(self):
    return self.HasRootAccess()

  def FlushEntireSystemCache(self):
    p = subprocess.Popen(['/sbin/sysctl', '-w', 'vm.drop_caches=3'])
    p.wait()
    assert p.returncode == 0, 'Failed to flush system cache'

  def CanLaunchApplication(self, application):
    if application == 'ipfw' and not self._IsIpfwKernelModuleInstalled():
      return False
    return super().CanLaunchApplication(application)

  def InstallApplication(self, application):
    if application == 'ipfw':
      self._InstallIpfw()
    elif application == 'avconv':
      self._InstallBinary(application)
    elif application in _POSSIBLE_PERFHOST_APPLICATIONS:
      self._InstallBinary(application)
    else:
      raise NotImplementedError(
          'Please teach Telemetry how to install ' + application)

  def _IsIpfwKernelModuleInstalled(self):
    return 'ipfw_mod' in subprocess.Popen(
        ['lsmod'], stdout=subprocess.PIPE).communicate()[0]

  def _InstallIpfw(self):
    ipfw_bin = binary_manager.FindPath(
        'ipfw', self.GetArchName(), self.GetOSName())
    ipfw_mod = binary_manager.FindPath(
        'ipfw_mod.ko', self.GetArchName(), self.GetOSName())

    try:
      changed = cloud_storage.GetIfChanged(
          ipfw_bin, cloud_storage.INTERNAL_BUCKET)
      changed |= cloud_storage.GetIfChanged(
          ipfw_mod, cloud_storage.INTERNAL_BUCKET)
    except cloud_storage.CloudStorageError as e:
      logging.error(str(e))
      logging.error('You may proceed by manually building and installing'
                    'dummynet for your kernel. See: '
                    'http://info.iet.unipi.it/~luigi/dummynet/')
      sys.exit(1)

    if changed or not self.CanLaunchApplication('ipfw'):
      if not self._IsIpfwKernelModuleInstalled():
        subprocess.check_call(['/usr/bin/sudo', 'insmod', ipfw_mod])
      os.chmod(ipfw_bin, 0o755)
      subprocess.check_call(
          ['/usr/bin/sudo', 'cp', ipfw_bin, '/usr/local/sbin'])

    assert self.CanLaunchApplication('ipfw'), 'Failed to install ipfw. ' \
        'ipfw provided binaries are not supported for linux kernel < 3.13. ' \
        'You may proceed by manually building and installing dummynet for ' \
        'your kernel. See: http://info.iet.unipi.it/~luigi/dummynet/'

  def _InstallBinary(self, bin_name):
    bin_path = binary_manager.FetchPath(
        bin_name, self.GetOSName(), self.GetArchName())
    os.environ['PATH'] += os.pathsep + os.path.dirname(bin_path)
    assert self.CanLaunchApplication(bin_name), 'Failed to install ' + bin_name
