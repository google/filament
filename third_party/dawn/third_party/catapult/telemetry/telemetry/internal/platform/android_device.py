# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import os

from telemetry.internal.platform import cros_device
from telemetry.internal.platform import device

from devil.android import device_denylist
from devil.android import device_errors
from devil.android import device_utils
from devil.android.sdk import adb_wrapper


HIGH_PERFORMANCE_MODE = 'high'
NORMAL_PERFORMANCE_MODE = 'normal'
LITTLE_ONLY_PERFORMANCE_MODE = 'little-only'
KEEP_PERFORMANCE_MODE = 'keep'


class AndroidDevice(device.Device):
  """ Class represents information for connecting to an android device.

  Attributes:
    device_id: the device's serial string created by adb to uniquely
      identify an emulator/device instance. This string can be found by running
      'adb devices' command
  """
  def __init__(self, device_id):
    super().__init__(
        name='Android device %s' % device_id, guid=device_id)
    self._device_id = device_id

  @classmethod
  def GetAllConnectedDevices(cls, denylist):
    device_serials = GetDeviceSerials(denylist)
    return [cls(s) for s in device_serials]

  @property
  def device_id(self):
    return self._device_id


def _ListSerialsOfHealthyOnlineDevices(denylist):
  return [d.adb.GetDeviceSerial()
          for d in device_utils.DeviceUtils.HealthyDevices(denylist)]


def GetDeviceSerials(denylist):
  """Return the list of device serials of healthy devices.

  If a preferred device has been set with ANDROID_SERIAL, it will be first in
  the returned list. The arguments specify what devices to include in the list.
  """
  device_serials = _ListSerialsOfHealthyOnlineDevices(denylist)

  preferred_device = os.environ.get('ANDROID_SERIAL')
  if preferred_device in device_serials:
    logging.warning(
        'ANDROID_SERIAL is defined. Put %s in the first of the '
        'discovered devices list.' % preferred_device)
    device_serials.remove(preferred_device)
    device_serials.insert(0, preferred_device)
  return device_serials


def GetDevice(finder_options):
  """Return a Platform instance for the device specified by |finder_options|."""
  android_platform_options = finder_options.remote_platform_options
  if not CanDiscoverDevices():
    logging.info(
        'No adb command found. Will not try searching for Android browsers.')
    return None

  if android_platform_options.android_denylist_file:
    denylist = device_denylist.Denylist(
        android_platform_options.android_denylist_file)
  else:
    denylist = None

  if (android_platform_options.device
      and android_platform_options.device in GetDeviceSerials(denylist)):
    return AndroidDevice(android_platform_options.device)

  devices = AndroidDevice.GetAllConnectedDevices(denylist)
  if len(devices) == 0:
    logging.warning('No android devices found.')
    return None
  if len(devices) > 1:
    logging.warning(
        'Multiple devices attached. Please specify one of the following:\n' +
        '\n'.join(['  --device=%s' % d.device_id for d in devices]))
    return None
  return devices[0]


def _HasValidAdb():
  """Returns true if adb is present.

  Note that this currently will return True even if the adb that's present
  cannot run on this system.
  """
  if os.name != 'posix' or cros_device.IsRunningOnCrOS():
    return False

  try:
    adb_path = adb_wrapper.AdbWrapper.GetAdbPath()
  except device_errors.NoAdbError:
    return False

  if os.path.isabs(adb_path) and not os.path.exists(adb_path):
    return False

  return True


def CanDiscoverDevices():
  """Returns true if devices are discoverable via adb."""
  if not _HasValidAdb():
    return False

  try:
    device_utils.DeviceUtils.HealthyDevices(None)
    return True
  except (device_errors.CommandFailedError, device_errors.CommandTimeoutError,
          device_errors.NoAdbError, OSError):
    return False


def FindAllAvailableDevices(options):
  """Returns a list of available devices.
  """
  # Disable Android device discovery when remote testing a CrOS device
  if options.remote:
    return []

  android_platform_options = options.remote_platform_options
  devices = []
  try:
    if CanDiscoverDevices():
      denylist = None
      if android_platform_options.android_denylist_file:
        denylist = device_denylist.Denylist(
            android_platform_options.android_denylist_file)
      devices = AndroidDevice.GetAllConnectedDevices(denylist)
  finally:
    if not devices and _HasValidAdb():
      try:
        adb_wrapper.AdbWrapper.KillServer()
      except device_errors.NoAdbError as e:
        logging.warning(
            'adb reported as present, but NoAdbError thrown: %s', str(e))

  return devices
