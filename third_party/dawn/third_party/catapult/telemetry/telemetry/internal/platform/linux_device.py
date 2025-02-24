# Copyright 2022 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import logging

from telemetry.internal.platform import linux_based_device


class LinuxDevice(linux_based_device.LinuxBasedDevice):
  pass


def IsRunningOnLinux():
  return LinuxDevice.PlatformIsRunningOS()


def FindAllAvailableDevices(options):
  logging.debug('Calling linux_device.FindAllAvailableDevices()')
  return LinuxDevice.FindAllAvailableDevices(options)
