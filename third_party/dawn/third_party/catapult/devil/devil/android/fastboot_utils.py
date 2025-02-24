# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Provides a variety of device interactions based on fastboot."""
# pylint: disable=unused-argument

import collections
import fnmatch
import logging
import os
import re
import subprocess

from devil.android import decorators
from devil.android import device_errors
from devil.android import device_utils
from devil.android.sdk import fastboot
from devil.utils import timeout_retry

logger = logging.getLogger(__name__)

_DEFAULT_TIMEOUT = 30
_DEFAULT_RETRIES = 3
_FASTBOOT_REBOOT_TIMEOUT = 10 * _DEFAULT_TIMEOUT

PartitionInfo = collections.namedtuple('PartitionInfo', [
    'image',     # The image name to look for. Can contain *.
    'optional',  # If this image is optional.
    'restart',   # If to restart the device after flashing the image.
])

# These partitions need to be flashed before calling flashall
# so that the device can pass the requirement check.
_VERIFICATION_PARTITIONS = collections.OrderedDict([
    ('bootloader', PartitionInfo(
        image='bootloader*.img',
        optional=False,
        restart=True,
    )),
    ('radio', PartitionInfo(
        image='radio*.img',
        optional=False,
        restart=True,
    )),
])

# These partitions will be flashed when wipe is set to true.
_WIPE_PARTITIONS = collections.OrderedDict([
    ('cache', PartitionInfo(
        image='cache.img',
        optional=True,
        restart=False,
    )),
])


def _FindAndVerifyPartitionsAndImages(partitions, directory):
  """Validate partitions and images.

  Validate all partition names and partition directories. Cannot stop mid
  flash so its important to validate everything first.

  Args:
    partitions: partitions to be tested.
    directory: directory containing the images.

  Returns:
    Dictionary with exact partition, image name mapping.
  """

  files = os.listdir(directory)
  return_dict = collections.OrderedDict()

  def find_file(pattern):
    for filename in files:
      if fnmatch.fnmatch(filename, pattern):
        return os.path.join(directory, filename)
    return None

  for partition, partition_info in partitions.items():
    image_file = find_file(partition_info.image)
    if image_file:
      return_dict[partition] = image_file
    elif not partition_info.optional:
      raise device_errors.FastbootCommandFailedError(
          [],
          '',
          message='Failed to flash device. Could not find image for %s.' %
          partition_info.image)
  return return_dict


class FastbootUtils(object):

  _FASTBOOT_WAIT_TIME = 1
  _BOARD_VERIFICATION_FILE = 'android-info.txt'

  def __init__(self,
               device=None,
               fastbooter=None,
               default_timeout=_DEFAULT_TIMEOUT,
               default_retries=_DEFAULT_RETRIES):
    """FastbootUtils constructor.

    Example Usage to flash a device:
      fastboot = fastboot_utils.FastbootUtils(device)
      fastboot.FlashDevice('/path/to/build/directory')

    Args:
      device: A DeviceUtils instance. Optional if a Fastboot instance was
        passed.
      fastbooter: A fastboot.Fastboot instance. Optional if a DeviceUtils
        instance was passed.
      default_timeout: An integer containing the default number of seconds to
        wait for an operation to complete if no explicit value is provided.
      default_retries: An integer containing the default number or times an
        operation should be retried on failure if no explicit value is provided.
    """
    if not device and not fastbooter:
      raise ValueError("One of 'device' or 'fastbooter' must be passed.")

    if device:
      self._device = device
      self._serial = str(device)
      self._board = device.product_board
      if not fastbooter:
        self.fastboot = fastboot.Fastboot(self._serial)

    if fastbooter:
      self._serial = str(fastbooter)
      self.fastboot = fastbooter
      self._board = fastbooter.GetVar('product')
      if not device:
        self._device = device_utils.DeviceUtils(self._serial)

    self._default_timeout = default_timeout
    self._default_retries = default_retries

  def __str__(self):
    return self._serial

  def IsFastbootMode(self):
    return self._serial in (str(d) for d in self.fastboot.Devices())

  @decorators.WithTimeoutAndRetriesFromInstance()
  def WaitForFastbootMode(self, timeout=None, retries=None):
    """Wait for device to boot into fastboot mode.

    This waits for the device serial to show up in fastboot devices output.
    """
    timeout_retry.WaitFor(self.IsFastbootMode,
                          wait_period=self._FASTBOOT_WAIT_TIME)

  @decorators.WithTimeoutAndRetriesFromInstance(
      min_default_timeout=_FASTBOOT_REBOOT_TIMEOUT)
  def EnableFastbootMode(self, timeout=None, retries=None):
    """Reboots phone into fastboot mode.

    Roots phone if needed, then reboots phone into fastboot mode and waits.
    """
    if self.IsFastbootMode():
      return
    self._device.EnableRoot()
    self._device.adb.Reboot(to_bootloader=True)
    self.WaitForFastbootMode()

  @decorators.WithTimeoutAndRetriesFromInstance(
      min_default_timeout=_FASTBOOT_REBOOT_TIMEOUT)
  def Reboot(self,
             bootloader=False,
             wait_for_reboot=True,
             timeout=None,
             retries=None):
    """Reboots out of fastboot mode.

    It reboots the phone either back into fastboot, or to a regular boot. It
    then blocks until the device is ready.

    Args:
      bootloader: If set to True, reboots back into bootloader.
    """
    if bootloader:
      self.fastboot.RebootBootloader()
      self.WaitForFastbootMode()
    else:
      self.fastboot.Reboot()
      if wait_for_reboot:
        self._device.WaitUntilFullyBooted(timeout=_FASTBOOT_REBOOT_TIMEOUT)

  def _VerifyBoard(self, directory):
    """Validate as best as possible that the android build matches the device.

    Goes through build files and checks if the board name is mentioned in the
    |self._BOARD_VERIFICATION_FILE| or in the build archive.

    Args:
      directory: directory where build files are located.
    """
    files = os.listdir(directory)
    board_regex = re.compile(r'require board=([\w|]+)')
    if self._BOARD_VERIFICATION_FILE in files:
      with open(os.path.join(directory, self._BOARD_VERIFICATION_FILE)) as f:
        for line in f:
          m = board_regex.match(line)
          if m and m.group(1):
            return self._board in m.group(1).split('|')

          logger.warning('No board type found in %s.',
                         self._BOARD_VERIFICATION_FILE)
    else:
      logger.warning('%s not found. Unable to use it to verify device.',
                     self._BOARD_VERIFICATION_FILE)

    zip_regex = re.compile(r'.*%s.*\.zip' % re.escape(self._board))
    for f in files:
      if zip_regex.match(f):
        return True

    return False

  def _FlashPartitions(self, partitions, directory, force=False):
    """Flashes all given partitions with all given images.

    Args:
      partitions: An ordered dict of partitions to flash.
      directory: Directory where all partitions can be found.
      force: boolean to decide to ignore board name safety checks.

    Raises:
      device_errors.CommandFailedError(): If image cannot be found or if bad
          partition name is give.
    """
    if not self._VerifyBoard(directory):
      if force:
        logger.warning('Could not verify build is meant to be installed on '
                       'the current device type, but force flag is set. '
                       'Flashing device. Possibly dangerous operation.')
      else:
        raise device_errors.CommandFailedError(
            'Could not verify build is meant to be installed on the current '
            'device type. Run again with force=True to force flashing with an '
            'unverified board.')

    flash_image_files = _FindAndVerifyPartitionsAndImages(partitions, directory)
    for partition, image_files in flash_image_files.items():
      logger.info('Flashing %s with %s', partition, image_files)
      self.fastboot.Flash(partition, image_files)
      if partitions[partition].restart:
        self.Reboot(bootloader=True)

  def FlashDevice(self, directory, wipe=False, wait_for_reboot=False):
    """Flash device with build in |directory|.

    Directory must contain bootloader, radio, and other necessary files from
    an android build. This is a dangerous operation so use with care.

    Note when wipe is set to true, we erase "userdata" partition, and flash the
    partitions in _WIPE_PARTITIONS to achieve the wipe purpose. We don't use
    the flag "-w" in flashall command as it can cause old devices like Nexus 5X
    stuck on the encryption phrase.

    Args:
      directory: Directory with build files.
      wipe: If set to true, wipe the device data by erasing "userdata"
        partitions and flash partitions in _WIPE_PARTITIONS.
      wait_for_reboot: If set to true, wait for the device to be online after
        sending reboot command.
    """

    # If a device is wiped, then it will no longer have adb keys so it cannot be
    # communicated with to verify that it is rebooted. It is up to the user of
    # this script to ensure that the adb keys are set on the device after using
    # this to wipe a device.
    self.EnableFastbootMode()
    self._FlashPartitions(_VERIFICATION_PARTITIONS, directory)
    try:
      logger.info('Flashing all.')
      for line in self.fastboot.FlashAll(directory, skip_reboot=True):
        logger.info('  %s', line)
    except subprocess.CalledProcessError as e:
      raise device_errors.FastbootCommandFailedError(
          [], '', message='Failed to flashall: %s' % str(e))
    if wipe:
      # Flashing the default userdata.img will cause the device to have only
      # 10GB of internal storage. To avoid this, we directly erase it and rely
      # on the recovery to auto format it.
      logger.info('Erasing "userdata" partition.')
      self.fastboot.Erase('userdata')
      self._FlashPartitions(_WIPE_PARTITIONS, directory)
    self.Reboot(wait_for_reboot=wait_for_reboot)
