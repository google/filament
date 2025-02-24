# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import platform
import subprocess
import sys
import logging

from PIL import ImageGrab  # pylint: disable=import-error

from telemetry.core import os_version as os_version_module
from telemetry import decorators
from telemetry.internal.platform import posix_platform_backend


class MacPlatformBackend(posix_platform_backend.PosixPlatformBackend):
  def __init__(self):
    super().__init__()

  def GetSystemLog(self):
    try:
      # Since the log file can be very large, only show the last 200 lines.
      return subprocess.check_output(
          ['tail', '-n', '200', '/var/log/system.log']).decode('utf-8')
    except subprocess.CalledProcessError as e:
      return 'Failed to collect system log: %s\nOutput:%s' % (e, e.output)

  @classmethod
  def IsPlatformBackendForHost(cls):
    return sys.platform == 'darwin'

  def IsThermallyThrottled(self):
    raise NotImplementedError()

  def HasBeenThermallyThrottled(self):
    raise NotImplementedError()

  @decorators.Cache
  def GetSystemTotalPhysicalMemory(self):
    return int(self.RunCommand(['sysctl', '-n', 'hw.memsize']))

  @decorators.Cache
  def GetArchName(self):
    return platform.machine()

  def GetOSName(self):
    return 'mac'

  @decorators.Cache
  def GetOSVersionName(self):
    os_version = os.uname()[2]

    if os_version.startswith('9.'):
      return os_version_module.LEOPARD
    if os_version.startswith('10.'):
      return os_version_module.SNOWLEOPARD
    if os_version.startswith('11.'):
      return os_version_module.LION
    if os_version.startswith('12.'):
      return os_version_module.MOUNTAINLION
    if os_version.startswith('13.'):
      return os_version_module.MAVERICKS
    if os_version.startswith('14.'):
      return os_version_module.YOSEMITE
    if os_version.startswith('15.'):
      return os_version_module.ELCAPITAN
    if os_version.startswith('16.'):
      return os_version_module.SIERRA
    if os_version.startswith('17.'):
      return os_version_module.HIGHSIERRA
    if os_version.startswith('18.'):
      return os_version_module.MOJAVE
    if os_version.startswith('19.'):
      return os_version_module.CATALINA
    if os_version.startswith('20.'):
      return os_version_module.BIGSUR
    if os_version.startswith('21.'):
      return os_version_module.MONTEREY
    if os_version.startswith('22.'):
      return os_version_module.VENTURA
    if os_version.startswith('23.'):
      return os_version_module.SONOMA

    raise NotImplementedError('Unknown mac version %s.' % os_version)

  def GetTypExpectationsTags(self):
    # telemetry benchmarks expectations need to know if the version number
    # of the operating system is 10.12 or 10.13
    tags = super().GetTypExpectationsTags()
    detail_string = self.GetOSVersionDetailString()
    if detail_string.startswith('10.11'):
      tags.append('mac-10.11')
    elif detail_string.startswith('10.12'):
      tags.append('mac-10.12')
    tags.append('mac-' + os.uname()[4])
    return tags

  @decorators.Cache
  def GetOSVersionDetailString(self):
    product = subprocess.check_output(['sw_vers', '-productVersion'],
                                      universal_newlines=True).strip()
    build = subprocess.check_output(['sw_vers', '-buildVersion'],
                                    universal_newlines=True).strip()
    return product + ' ' + build

  def CanTakeScreenshot(self):
    return True

  def TakeScreenshot(self, file_path):
    image = ImageGrab.grab()
    with open(file_path, 'wb') as f:
      image.save(f, 'PNG')
    return True

  def CanFlushIndividualFilesFromSystemCache(self):
    return False

  def SupportFlushEntireSystemCache(self):
    return self.HasRootAccess()

  def FlushEntireSystemCache(self):
    mavericks_or_later = self.GetOSVersionName() >= os_version_module.MAVERICKS
    p = self.LaunchApplication('purge', elevate_privilege=mavericks_or_later)
    p.communicate()
    assert p.returncode == 0, 'Failed to flush system cache'

  def GetIntelPowerGadgetPath(self):
    gadget_path = '/Applications/Intel Power Gadget/PowerLog'
    if not os.path.isfile(gadget_path):
      logging.debug('Cannot locate Intel Power Gadget at ' + gadget_path)
      return None
    return gadget_path

  def SupportsIntelPowerGadget(self):
    gadget_path = self.GetIntelPowerGadgetPath()
    return gadget_path is not None
