#!/usr/bin/env python3
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import logging
import os
import sys

if __name__ == '__main__':
  sys.path.append(
      os.path.abspath(
          os.path.join(os.path.dirname(__file__), '..', '..', '..')))
from devil.android import device_denylist
from devil.android import device_errors
from devil.android import fastboot_utils
from devil.android.sdk import fastboot
from devil.android.tools import script_common
from devil.constants import exit_codes
from devil.utils import logging_common
from devil.utils import parallelizer

logger = logging.getLogger(__name__)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('build_path', help='Path to android build.')
  parser.add_argument(
      '-w', '--wipe', action='store_true', help='If set, wipes user data')
  parser.add_argument('--wait',
                      action='store_true',
                      help='If set, wait for device to be online after flash')
  logging_common.AddLoggingArguments(parser)
  script_common.AddDeviceArguments(parser)
  args = parser.parse_args()
  logging_common.InitializeLogging(args)

  if args.denylist_file:
    denylist = device_denylist.Denylist(args.denylist_file).Read()
    if denylist:
      logger.critical('Device(s) in denylist, not flashing devices:')
      for key in denylist:
        logger.critical('  %s', key)
      return exit_codes.INFRA

  flashed_devices = []
  failed_devices = []

  def flash(device):
    try:
      device.FlashDevice(args.build_path,
                         wipe=args.wipe,
                         wait_for_reboot=args.wait)
      flashed_devices.append(device)
    except Exception:  # pylint: disable=broad-except
      logger.exception('Device %s failed to flash.', str(device))
      failed_devices.append(device)

  devices = []
  try:
    adb_devices = script_common.GetDevices(args.devices, args.denylist_file)
    devices += [fastboot_utils.FastbootUtils(device=d) for d in adb_devices]
  except device_errors.NoDevicesError:
    # Don't bail out if we're not looking for any particular device and there's
    # at least one sitting in fastboot mode. Note that if we ARE looking for a
    # particular device, and it's in fastboot mode, this will still fail.
    fastboot_devices = fastboot.Fastboot.Devices()
    if args.devices or not fastboot_devices:
      raise
    devices += [
        fastboot_utils.FastbootUtils(fastbooter=d) for d in fastboot_devices
    ]

  parallel_devices = parallelizer.SyncParallelizer(devices)
  parallel_devices.pMap(flash)

  if flashed_devices:
    logger.info('The following devices were flashed:')
    logger.info('  %s', ' '.join(str(d) for d in flashed_devices))
  if failed_devices:
    logger.critical('The following devices failed to flash:')
    logger.critical('  %s', ' '.join(str(d) for d in failed_devices))
    return exit_codes.INFRA
  return 0


if __name__ == '__main__':
  sys.exit(main())
