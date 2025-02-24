#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Launches a daemon to monitor android device temperatures & status.

This script will repeatedly poll the given devices for their temperatures and
status every 60 seconds and dump the stats to file on the host.
"""

import argparse
import collections
import json
import logging
import logging.handlers
import os
import re
import socket
import sys
import time

if __name__ == '__main__':
  sys.path.append(
      os.path.abspath(
          os.path.join(os.path.dirname(__file__), '..', '..', '..')))

from devil.android import battery_utils
from devil.android import device_denylist
from devil.android import device_errors
from devil.android import device_utils
from devil.android.tools import script_common

# Various names of sensors used to measure cpu temp
CPU_TEMP_SENSORS = [
    # most nexus devices
    'tsens_tz_sensor0',
    # android one
    'mtktscpu',
    # nexus 9
    'CPU-therm',
]

DEVICE_FILE_VERSION = 1
DEVICE_FILE = os.path.join(
    os.path.expanduser('~'), '.android',
    '%s__android_device_status.json' % socket.gethostname().split('.')[0])

MEM_INFO_REGEX = re.compile(
    r'.*?\:\s*(\d+)\s*kB')  # ex: 'MemTotal:   185735 kB'


def get_device_status_unsafe(device):
  """Polls the given device for various info.

    Returns: A dict of the following format:
    {
      'battery': {
        'level': 100,
        'temperature': 123
      },
      'build': {
        'build.id': 'ABC12D',
        'product.device': 'chickenofthesea'
      },
      'imei': 123456789,
      'mem': {
        'avail': 1000000,
        'total': 1234567,
      },
      'processes': 123,
      'state': 'good',
      'temp': {
        'some_sensor': 30
      },
      'uptime': 1234.56,
    }
  """
  status = collections.defaultdict(dict)

  # Battery
  battery = battery_utils.BatteryUtils(device)
  battery_info = battery.GetBatteryInfo()
  try:
    level = int(battery_info.get('level'))
  except (KeyError, TypeError, ValueError):
    level = None
  if level and 0 <= level <= 100:
    status['battery']['level'] = level
  try:
    temperature = int(battery_info.get('temperature'))
  except (KeyError, TypeError, ValueError):
    temperature = None
  if temperature:
    status['battery']['temperature'] = temperature

  # Build
  status['build']['build.id'] = device.build_id
  status['build']['product.device'] = device.build_product

  # Memory
  mem_info = ''
  try:
    mem_info = device.ReadFile('/proc/meminfo')
  except device_errors.AdbShellCommandFailedError:
    logging.exception('Unable to read /proc/meminfo')
  for line in mem_info.splitlines():
    match = MEM_INFO_REGEX.match(line)
    if match:
      try:
        value = int(match.group(1))
      except ValueError:
        continue
      key = line.split(':')[0].strip()
      if key == 'MemTotal':
        status['mem']['total'] = value
      elif key == 'MemFree':
        status['mem']['free'] = value

  # Process
  try:
    status['processes'] = len(device.ListProcesses())
  except device_errors.AdbCommandFailedError:
    logging.exception('Unable to count process list.')

  # CPU Temps
  # Find a thermal sensor that matches one in CPU_TEMP_SENSORS and read its
  # temperature.
  files = []
  try:
    files = device.RunShellCommand(
        'grep -lE "%s" /sys/class/thermal/thermal_zone*/type' %
        '|'.join(CPU_TEMP_SENSORS),
        shell=True,
        check_return=True)
  except device_errors.AdbShellCommandFailedError:
    logging.exception('Unable to list thermal sensors.')
  for f in files:
    try:
      sensor_name = device.ReadFile(f).strip()
      temp = float(device.ReadFile(f[:-4] + 'temp').strip())  # s/type^/temp
      status['temp'][sensor_name] = temp
    except (device_errors.AdbShellCommandFailedError, ValueError):
      logging.exception('Unable to read thermal sensor %s', f)

  # Uptime
  try:
    uptimes = device.ReadFile('/proc/uptime').split()
    status['uptime'] = float(uptimes[0])  # Take the first field (actual uptime)
  except (device_errors.AdbShellCommandFailedError, ValueError):
    logging.exception('Unable to read /proc/uptime')

  try:
    status['imei'] = device.GetIMEI()
  except device_errors.CommandFailedError:
    logging.exception('Unable to read IMEI')
    status['imei'] = 'unknown'

  status['state'] = 'available'
  return status


def get_device_status(device):
  try:
    status = get_device_status_unsafe(device)
  except device_errors.DeviceUnreachableError:
    status = collections.defaultdict(dict)
    status['state'] = 'offline'
  return status


def get_all_status(denylist):
  status_dict = {
      'version': DEVICE_FILE_VERSION,
      'devices': {},
  }

  healthy_devices = device_utils.DeviceUtils.HealthyDevices(denylist)
  parallel_devices = device_utils.DeviceUtils.parallel(healthy_devices)
  results = parallel_devices.pMap(get_device_status).pGet(None)

  status_dict['devices'] = {
      device.serial: result
      for device, result in zip(healthy_devices, results)
  }

  if denylist:
    for device, reason in denylist.Read().items():
      status_dict['devices'][device] = {
          'state': reason.get('reason', 'denylisted')
      }

  status_dict['timestamp'] = time.time()
  return status_dict


def main(argv):
  """Launches the device monitor.

  Polls the devices for their battery and cpu temperatures and scans the
  denylist file every 60 seconds and dumps the data to DEVICE_FILE.
  """

  parser = argparse.ArgumentParser(description='Launches the device monitor.')
  script_common.AddEnvironmentArguments(parser)
  parser.add_argument('--denylist-file', help='Path to device denylist file.')
  args = parser.parse_args(argv)

  logger = logging.getLogger()
  logger.setLevel(logging.DEBUG)
  handler = logging.handlers.RotatingFileHandler(
      '/tmp/device_monitor.log', maxBytes=10 * 1024 * 1024, backupCount=5)
  fmt = logging.Formatter(
      '%(asctime)s %(levelname)s %(message)s', datefmt='%y%m%d %H:%M:%S')
  handler.setFormatter(fmt)
  logger.addHandler(handler)
  script_common.InitializeEnvironment(args)

  denylist = (device_denylist.Denylist(args.denylist_file)
              if args.denylist_file else None)

  logging.info('Device monitor running with pid %d, adb: %s, denylist: %s',
               os.getpid(), args.adb_path, args.denylist_file)
  while True:
    start = time.time()
    status_dict = get_all_status(denylist)
    with open(DEVICE_FILE, 'wb') as f:
      json.dump(status_dict, f, indent=2, sort_keys=True)
    logging.info('Got status of all devices in %.2fs.', time.time() - start)
    time.sleep(60)


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
