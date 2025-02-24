# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""This module wraps Android's fastboot tool.

This is a thin wrapper around the fastboot interface. Any additional complexity
should be delegated to a higher level (ex. FastbootUtils).
"""
# pylint: disable=unused-argument

from devil import devil_env
from devil.android import decorators
from devil.android import device_errors
from devil.utils import cmd_helper
from devil.utils import lazy

_DEFAULT_TIMEOUT = 30
_DEFAULT_RETRIES = 3
_FLASH_TIMEOUT = _DEFAULT_TIMEOUT * 10  # 5 mins
_FLASHALL_TIMEOUT = _FLASH_TIMEOUT * 6  # 30 mins


class Fastboot(object):

  _fastboot_path = lazy.WeakConstant(lambda: devil_env.config.FetchPath(
      'fastboot'))

  def __init__(self,
               device_serial,
               default_timeout=_DEFAULT_TIMEOUT,
               default_retries=_DEFAULT_RETRIES):
    """Initializes the FastbootWrapper.

    Args:
      device_serial: The device serial number as a string.
    """
    if not device_serial:
      raise ValueError('A device serial must be specified')
    self._device_serial = str(device_serial)
    self._default_timeout = default_timeout
    self._default_retries = default_retries

  def __str__(self):
    return self._device_serial

  @classmethod
  def _BuildFastbootCommand(cls, args, device_serial):
    if not isinstance(args, list):
      raise TypeError('Command for _BuildFastbootCommand must be a list.')

    cmd = []
    cmd.append(cls._fastboot_path.read())
    if device_serial is not None:
      cmd.extend(['-s', device_serial])
    cmd.extend(args)
    return cmd

  @classmethod
  def _RunFastbootCommand(cls, args, device_serial=None, env=None):
    """Run a generic fastboot command.

    Args:
      args: A list of arguments to fastboot, the first one being the command.
      device_serial: (Optional) the serial of the device.
      env: (Optional) a dict containing environments needed to run the command.

    Returns:
      output of command.

    Raises:
      TypeError: If args is not of type list.
    """
    cmd = cls._BuildFastbootCommand(args, device_serial)
    # fastboot can't be trusted to keep non-error output out of stderr, so
    # capture stderr as part of stdout.
    status, output = cmd_helper.GetCmdStatusAndOutput(cmd,
                                                      env=env,
                                                      merge_stderr=True)
    if int(status) != 0:
      raise device_errors.FastbootCommandFailedError(cmd, output, status)
    return output

  def _RunDeviceFastbootCommand(self, args, env=None):
    """Run a fastboot command on the device associated with this object.

    Args:
      args: A list of arguments to fastboot, the first one being the command.
      env: (Optional) a dict containing environments needed to run the command.

    Returns:
      output of command.

    Raises:
      TypeError: If args is not of type list.
    """
    return self._RunFastbootCommand(args,
                                    device_serial=self._device_serial,
                                    env=env)

  def _IterDeviceFastbootCommand(self, args, env=None):
    """Runs a fastboot command and returns an iterator over its output lines.

    Args:
      args: A list of arguments to fastboot, the first one being the command
      env: (Optional) a dict containing environments needed to run the command.

    Yields:
      The output of the command line by line.
    """
    cmd = self._BuildFastbootCommand(args, self._device_serial)
    return cmd_helper.IterCmdOutputLines(cmd, env=env, check_status=True)

  @decorators.WithTimeoutAndRetriesDefaults(_DEFAULT_TIMEOUT, _DEFAULT_RETRIES)
  def GetVar(self, variable, timeout=None, retries=None):
    args = ['getvar', variable]
    output = self._RunDeviceFastbootCommand(args)
    # getvar returns timing information on the last line of output, so only
    # parse the first line.
    output = output.splitlines()[0]
    # And the first line should match the format '$(var): $(value)'.
    if variable + ': ' not in output:
      raise device_errors.FastbootCommandFailedError(
          args, output, message="Unknown 'getvar' output format.")
    return output.split('%s: ' % variable)[1].strip()

  @decorators.WithTimeoutAndRetriesDefaults(_FLASH_TIMEOUT, 0)
  def Flash(self, partition, image, timeout=None, retries=None):
    """Flash partition with img.

    Args:
      partition: Partition to be flashed.
      image: location of image to flash with.
    """
    self._RunDeviceFastbootCommand(['flash', partition, image])

  @decorators.WithTimeoutAndRetriesDefaults(_FLASHALL_TIMEOUT, 0)
  def FlashAll(self,
               directory,
               force=False,
               skip_reboot=False,
               timeout=None,
               retries=None):
    """Flash all the image files from the given partition.

    Args:
      directory: Directory that contains all the images to flash with.
      force: Force the flash operation, if set to true.
      skip_reboot: Don't reboot the device after flash, if set to true.

    Yields:
      The output of the flashall command line by line.
    """
    env = {'ANDROID_PRODUCT_OUT': directory}
    cmd = []
    if force:
      cmd.append('--force')
    if skip_reboot:
      cmd.append('--skip-reboot')
    cmd.append('flashall')
    return self._IterDeviceFastbootCommand(cmd, env=env)

  @classmethod
  @decorators.WithTimeoutAndRetriesDefaults(_DEFAULT_TIMEOUT, _DEFAULT_RETRIES)
  def Devices(cls, timeout=None, retries=None):
    """Outputs list of devices in fastboot mode.

    Returns:
      List of Fastboot objects, one for each device in fastboot.
    """
    output = cls._RunFastbootCommand(['devices'])
    # fastboot v33.0.3 returns an empty line at the end
    return [Fastboot(line.split()[0]) for line in output.splitlines() if line]

  @decorators.WithTimeoutAndRetriesDefaults(_FLASH_TIMEOUT, 0)
  def Erase(self, partition, timeout=None, retries=None):
    """Erase partition."""
    self._RunDeviceFastbootCommand(['erase', partition])

  @decorators.WithTimeoutAndRetriesFromInstance()
  def RebootBootloader(self, timeout=None, retries=None):
    """Reboot from fastboot, into fastboot."""
    self._RunDeviceFastbootCommand(['reboot-bootloader'])

  @decorators.WithTimeoutAndRetriesDefaults(_FLASH_TIMEOUT, 0)
  def Reboot(self, timeout=None, retries=None):
    """Reboot from fastboot to normal usage"""
    self._RunDeviceFastbootCommand(['reboot'])

  @decorators.WithTimeoutAndRetriesFromInstance()
  def SetOemOffModeCharge(self, value, timeout=None, retries=None):
    """Sets off mode charging

    Args:
      value: boolean value to set off-mode-charging on or off.
    """
    self._RunDeviceFastbootCommand(['oem', 'off-mode-charge', str(int(value))])
