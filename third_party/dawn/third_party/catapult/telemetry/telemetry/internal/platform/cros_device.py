# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import

from telemetry.internal.platform import linux_based_device


class CrOSDevice(linux_based_device.LinuxBasedDevice):
  OS_NAME = 'chromeos'
  OS_PROPER_NAME = 'ChromeOs'
  GUID_NAME = 'cros'


def IsRunningOnCrOS():
  return CrOSDevice.PlatformIsRunningOS()


def FindAllAvailableDevices(options):
  return CrOSDevice.FindAllAvailableDevices(options)
