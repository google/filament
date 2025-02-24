# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Finds devices that can be controlled by telemetry."""

from __future__ import absolute_import
from telemetry.internal.platform import android_device
from telemetry.internal.platform import cast_device
from telemetry.internal.platform import cros_device
from telemetry.internal.platform import desktop_device
from telemetry.internal.platform import fuchsia_device

DEVICES = [
    android_device,
    cast_device,
    cros_device,
    desktop_device,
    fuchsia_device,
]


def _GetDeviceFinders(supported_platforms):
  if not supported_platforms or 'all' in supported_platforms:
    return DEVICES
  device_finders = []
  if any(p in supported_platforms for p in ['mac', 'linux', 'win']):
    device_finders.append(desktop_device)
  if 'android' in supported_platforms:
    device_finders.append(android_device)
  if 'chromeos' in supported_platforms:
    device_finders.append(cros_device)
  if 'fuchsia' in supported_platforms:
    device_finders.append(fuchsia_device)
  return device_finders


def _GetAllAvailableDevices(options):
  """Returns a list of all available devices."""
  devices = []
  for finder in _GetDeviceFinders(options.target_platforms):
    devices.extend(finder.FindAllAvailableDevices(options))
  return devices


def GetDevicesMatchingOptions(options):
  """Returns a list of devices matching the options."""
  devices = []
  remote_platform_options = options.remote_platform_options
  if (not remote_platform_options.device or
      remote_platform_options.device == 'list'):
    devices = _GetAllAvailableDevices(options)
  elif remote_platform_options.device == 'android':
    devices = android_device.FindAllAvailableDevices(options)
  else:
    devices = _GetAllAvailableDevices(options)
    devices = [d for d in devices if d.guid ==
               options.remote_platform_options.device]

  devices.sort(key=lambda device: device.name)
  return devices
