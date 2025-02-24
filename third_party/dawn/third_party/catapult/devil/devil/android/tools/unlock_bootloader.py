#!/usr/bin/env python
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A script to open the unlock bootloader on-screen prompt on all devices."""

import argparse
import logging
import os
import subprocess
import sys
import time

if __name__ == '__main__':
  sys.path.append(
      os.path.abspath(
          os.path.join(os.path.dirname(__file__), '..', '..', '..')))

from devil import devil_env
from devil.android import device_errors
from devil.android.sdk import adb_wrapper
from devil.android.sdk import fastboot
from devil.android.tools import script_common
from devil.utils import parallelizer


def reboot_into_bootloader(filter_devices):
  # Reboot all devices into bootloader if they aren't there already.
  rebooted_devices = set()
  for d in adb_wrapper.AdbWrapper.Devices(desired_state=None):
    if filter_devices and str(d) not in filter_devices:
      continue
    state = d.GetState()
    if state == 'device':
      logging.info('Booting %s to bootloader.', d)
      try:
        d.Reboot(to_bootloader=True)
        rebooted_devices.add(str(d))
      except (device_errors.AdbCommandFailedError,
              device_errors.DeviceUnreachableError):
        logging.exception('Unable to reboot device %s', d)
    else:
      logging.error('Unable to reboot device %s: %s', d, state)

  # Wait for the rebooted devices to show up in fastboot.
  if rebooted_devices:
    logging.info('Waiting for devices to reboot...')
    timeout = 60
    start = time.time()
    while True:
      time.sleep(5)
      fastbooted_devices = {str(d) for d in fastboot.Fastboot.Devices()}
      if rebooted_devices <= set(fastbooted_devices):
        logging.info('All devices in fastboot.')
        break
      if time.time() - start > timeout:
        logging.error('Timed out waiting for %s to reboot.',
                      rebooted_devices - set(fastbooted_devices))
        break


def unlock_bootloader(d):
  # Unlock the phones.
  unlocking_processes = []
  logging.info('Unlocking %s...', d)
  # The command to unlock the bootloader could be either of the following
  # depending on the android version and/or oem. Can't really tell which is
  # needed, so just try both.
  # pylint: disable=protected-access
  cmd_old = [d._fastboot_path.read(), '-s', str(d), 'oem', 'unlock']
  cmd_new = [d._fastboot_path.read(), '-s', str(d), 'flashing', 'unlock']
  unlocking_processes.append(
      subprocess.Popen(cmd_old, stdout=subprocess.PIPE, stderr=subprocess.PIPE))
  unlocking_processes.append(
      subprocess.Popen(cmd_new, stdout=subprocess.PIPE, stderr=subprocess.PIPE))

  # Give the unlocking command time to finish and/or open the on-screen prompt.
  logging.info('Sleeping for 5 seconds...')
  time.sleep(5)

  leftover_pids = []
  for p in unlocking_processes:
    p.poll()
    rc = p.returncode
    # If the command succesfully opened the unlock prompt on the screen, the
    # fastboot command process will hang and wait for a response. We still
    # need to read its stdout/stderr, so use os.read so that we don't
    # have to wait for EOF to be written.
    out = os.read(p.stderr.fileno(), 1024).strip().lower()
    if not rc:
      if out in ('...', '< waiting for device >'):
        logging.info('Device %s is waiting for confirmation.', d)
      else:
        logging.error(
            'Device %s is hanging, but not waiting for confirmation: %s', d,
            out)
      leftover_pids.append(p.pid)
    else:
      if 'unknown command' in out:
        # Of the two unlocking commands, this was likely the wrong one.
        continue
      if 'already unlocked' in out:
        logging.info('Device %s already unlocked.', d)
      elif 'unlock is not allowed' in out:
        logging.error("Device %s is oem locked. Can't unlock bootloader.", d)
        return 1
      else:
        logging.error('Device %s in unknown state: "%s"', d, out)
        return 1
    break

  if leftover_pids:
    logging.warning('Processes %s left over after unlocking.', leftover_pids)

  return 0


def main():
  logging.getLogger().setLevel(logging.INFO)

  parser = argparse.ArgumentParser()
  script_common.AddDeviceArguments(parser)
  parser.add_argument(
      '--adb-path', help='Absolute path to the adb binary to use.')
  args = parser.parse_args()

  devil_dynamic_config = devil_env.EmptyConfig()
  if args.adb_path:
    devil_dynamic_config['dependencies'].update(
        devil_env.LocalConfigItem('adb', devil_env.GetPlatform(),
                                  args.adb_path))
  devil_env.config.Initialize(configs=[devil_dynamic_config])

  reboot_into_bootloader(args.devices)
  devices = [
      d for d in fastboot.Fastboot.Devices()
      if not args.devices or str(d) in args.devices
  ]
  parallel_devices = parallelizer.Parallelizer(devices)
  parallel_devices.pMap(unlock_bootloader).pGet(None)
  return 0


if __name__ == '__main__':
  sys.exit(main())
