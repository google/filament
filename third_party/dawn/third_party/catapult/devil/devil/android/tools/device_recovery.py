#!/usr/bin/env vpython3
# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A script to recover devices in a known bad state."""

import argparse
import glob
import logging
import os
import signal
import sys

import psutil

if __name__ == '__main__':
  sys.path.append(
      os.path.abspath(
          os.path.join(os.path.dirname(__file__), '..', '..', '..')))
from devil.android import device_denylist
from devil.android import device_errors
from devil.android import device_utils
from devil.android.sdk import adb_wrapper
from devil.android.tools import device_status
from devil.android.tools import script_common
from devil.utils import logging_common
from devil.utils import lsusb
# TODO(jbudorick): Resolve this after experimenting w/ disabling the USB reset.
from devil.utils import reset_usb  # pylint: disable=unused-import

logger = logging.getLogger(__name__)

from py_utils import modules_util

# Script depends on features from psutil version 2.0 or higher.
modules_util.RequireVersion(psutil, '2.0')


def KillAllAdb():
  def get_all_adb():
    for p in psutil.process_iter():
      try:
        # Retrieve all required process infos at once.
        pinfo = p.as_dict(attrs=['pid', 'name', 'cmdline'])
        if pinfo['name'] == 'adb':
          pinfo['cmdline'] = ' '.join(pinfo['cmdline'])
          yield p, pinfo
      except (psutil.NoSuchProcess, psutil.AccessDenied):
        pass

  for sig in [signal.SIGTERM, signal.SIGQUIT, signal.SIGKILL]:
    for p, pinfo in get_all_adb():
      try:
        pinfo['signal'] = sig
        logger.info('kill %(signal)s %(pid)s (%(name)s [%(cmdline)s])', pinfo)
        p.send_signal(sig)
      except (psutil.NoSuchProcess, psutil.AccessDenied):
        pass
  for _, pinfo in get_all_adb():
    try:
      logger.error('Unable to kill %(pid)s (%(name)s [%(cmdline)s])', pinfo)
    except (psutil.NoSuchProcess, psutil.AccessDenied):
      pass


def TryAuth(device):
  """Uses anything in ~/.android/ that looks like a key to auth with the device.

  Args:
    device: The DeviceUtils device to attempt to auth.

  Returns:
    True if device successfully authed.
  """
  possible_keys = glob.glob(os.path.join(adb_wrapper.ADB_HOST_KEYS_DIR, '*key'))
  if len(possible_keys) <= 1:
    logger.warning('Only %d ADB keys available. Not forcing auth.',
                   len(possible_keys))
    return False

  KillAllAdb()
  adb_wrapper.AdbWrapper.StartServer(keys=possible_keys)
  new_state = device.adb.GetState()
  if new_state != 'device':
    logger.error('Auth failed. Device %s still stuck in %s.', str(device),
                 new_state)
    return False

  # It worked! Now register the host's default ADB key on the device so we don't
  # have to do all that again.
  pub_key = os.path.join(adb_wrapper.ADB_HOST_KEYS_DIR, 'adbkey.pub')
  if not os.path.exists(pub_key):  # This really shouldn't happen.
    logger.error('Default ADB key not available at %s.', pub_key)
    return False

  with open(pub_key) as f:
    pub_key_contents = f.read()
  try:
    device.WriteFile(adb_wrapper.ADB_KEYS_FILE, pub_key_contents, as_root=True)
  except (device_errors.CommandTimeoutError, device_errors.CommandFailedError,
          device_errors.DeviceUnreachableError):
    logger.exception('Unable to write default ADB key to %s.', str(device))
    return False
  return True


def RecoverDevice(device, denylist, should_reboot=lambda device: True):
  if device_status.IsDenylisted(device.adb.GetDeviceSerial(), denylist):
    logger.debug('%s is denylisted, skipping recovery.', str(device))
    return

  if device.adb.GetState() == 'unauthorized' and TryAuth(device):
    logger.info('Successfully authed device %s!', str(device))
    return

  if should_reboot(device):
    should_restore_root = device.HasRoot()
    try:
      device.WaitUntilFullyBooted(retries=0)
    except (device_errors.CommandTimeoutError, device_errors.CommandFailedError,
            device_errors.DeviceUnreachableError):
      logger.exception(
          'Failure while waiting for %s. '
          'Attempting to recover.', str(device))
    try:
      try:
        device.Reboot(block=False, timeout=5, retries=0)
      except device_errors.CommandTimeoutError:
        logger.warning(
            'Timed out while attempting to reboot %s normally.'
            'Attempting alternative reboot.', str(device))
        # The device drops offline before we can grab the exit code, so
        # we don't check for status.
        try:
          device.adb.Root()
        finally:
          # We are already in a failure mode, attempt to reboot regardless of
          # what device.adb.Root() returns. If the sysrq reboot fails an
          # exception willbe thrown at that level.
          device.adb.Shell(
              'echo b > /proc/sysrq-trigger',
              expect_status=None,
              timeout=5,
              retries=0)
    except (device_errors.CommandFailedError,
            device_errors.DeviceUnreachableError):
      logger.exception('Failed to reboot %s.', str(device))
      if denylist:
        denylist.Extend([device.adb.GetDeviceSerial()], reason='reboot_failure')
    except device_errors.CommandTimeoutError:
      logger.exception('Timed out while rebooting %s.', str(device))
      if denylist:
        denylist.Extend([device.adb.GetDeviceSerial()], reason='reboot_timeout')

    try:
      device.WaitUntilFullyBooted(
          retries=0, timeout=device.REBOOT_DEFAULT_TIMEOUT)
      if should_restore_root:
        device.EnableRoot()
    except (device_errors.CommandFailedError,
            device_errors.DeviceUnreachableError):
      logger.exception('Failure while waiting for %s.', str(device))
      if denylist:
        denylist.Extend([device.adb.GetDeviceSerial()], reason='reboot_failure')
    except device_errors.CommandTimeoutError:
      logger.exception('Timed out while waiting for %s.', str(device))
      if denylist:
        denylist.Extend([device.adb.GetDeviceSerial()], reason='reboot_timeout')


def RecoverDevices(devices, denylist, enable_usb_reset=False):
  """Attempts to recover any inoperable devices in the provided list.

  Args:
    devices: The list of devices to attempt to recover.
    denylist: The current device denylist, which will be used then
      reset.
  """

  statuses = device_status.DeviceStatus(devices, denylist)

  should_restart_usb = set(
      status['serial'] for status in statuses
      if (not status['usb_status'] or status['adb_status'] in ('offline',
                                                               'missing')))
  should_restart_adb = should_restart_usb.union(
      set(status['serial'] for status in statuses
          if status['adb_status'] == 'unauthorized'))
  should_reboot_device = should_restart_usb.union(
      set(status['serial'] for status in statuses if status['denylisted']))

  logger.debug('Should restart USB for:')
  for d in should_restart_usb:
    logger.debug('  %s', d)
  logger.debug('Should restart ADB for:')
  for d in should_restart_adb:
    logger.debug('  %s', d)
  logger.debug('Should reboot:')
  for d in should_reboot_device:
    logger.debug('  %s', d)

  if denylist:
    denylist.Reset()

  if should_restart_adb:
    KillAllAdb()
    adb_wrapper.AdbWrapper.StartServer()

  for serial in should_restart_usb:
    try:
      # TODO(crbug.com/642194): Resetting may be causing more harm
      # (specifically, kernel panics) than it does good.
      if enable_usb_reset:
        reset_usb.reset_android_usb(serial)
      else:
        logger.warning('USB reset disabled for %s (crbug.com/642914)', serial)
    except IOError:
      logger.exception('Unable to reset USB for %s.', serial)
      if denylist:
        denylist.Extend([serial], reason='USB failure')
    except device_errors.DeviceUnreachableError:
      logger.exception('Unable to reset USB for %s.', serial)
      if denylist:
        denylist.Extend([serial], reason='offline')

  device_utils.DeviceUtils.parallel(devices).pMap(
      RecoverDevice,
      denylist,
      should_reboot=lambda device: device.serial in should_reboot_device)


def main():
  parser = argparse.ArgumentParser()
  logging_common.AddLoggingArguments(parser)
  script_common.AddEnvironmentArguments(parser)
  parser.add_argument('--denylist-file', help='Device denylist JSON file.')
  parser.add_argument(
      '--known-devices-file',
      action='append',
      default=[],
      dest='known_devices_files',
      help='Path to known device lists.')
  parser.add_argument(
      '--enable-usb-reset', action='store_true', help='Reset USB if necessary.')

  args = parser.parse_args()
  logging_common.InitializeLogging(args)
  script_common.InitializeEnvironment(args)

  denylist = (device_denylist.Denylist(args.denylist_file)
              if args.denylist_file else None)

  expected_devices = device_status.GetExpectedDevices(args.known_devices_files)
  usb_devices = set(lsusb.get_android_devices())
  devices = [
      device_utils.DeviceUtils(s) for s in (expected_devices & usb_devices)
  ]

  RecoverDevices(devices, denylist, enable_usb_reset=args.enable_usb_reset)


if __name__ == '__main__':
  sys.exit(main())
