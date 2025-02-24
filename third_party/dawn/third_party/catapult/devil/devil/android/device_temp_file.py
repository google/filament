# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A temp file that automatically gets pushed and deleted from a device."""

# pylint: disable=W0622

import logging
import posixpath
import random
import threading

from devil import base_error
from devil.utils import cmd_helper

logger = logging.getLogger(__name__)


def _GenerateName(prefix, suffix, dir):
  random_hex = hex(random.randint(0, 2**52))[2:]
  return posixpath.join(dir, '%s-%s%s' % (prefix, random_hex, suffix))


class DeviceTempFile(object):
  """A named temporary file on a device.

  Behaves like tempfile.NamedTemporaryFile.
  """

  # TODO(b/293175593): Migrate to use device_utils to delete the file.
  def __init__(self,
               adb,
               suffix='',
               prefix='temp_file',
               dir='/data/local/tmp',
               device_utils=None):
    """Find an unused temporary file path on the device.

    When this object is closed, the file will be deleted on the device.

    Args:
      adb: An instance of AdbWrapper
      suffix: The suffix of the name of the temporary file.
      prefix: The prefix of the name of the temporary file.
      dir: The directory on the device in which the temporary file should be
        placed.
      device_utils: An instance of DeviceUtils.
    Raises:
      ValueError if any of suffix, prefix, or dir are None.
    """
    if None in (dir, prefix, suffix):
      m = 'Provided None path component. (dir: %s, prefix: %s, suffix: %s)' % (
          dir, prefix, suffix)
      raise ValueError(m)

    self._adb = adb
    # Python's random module use 52-bit numbers according to its docs.
    self.name = _GenerateName(prefix, suffix, dir)
    self.name_quoted = cmd_helper.SingleQuote(self.name)
    self._device_utils = device_utils

  def close(self):
    """Deletes the temporary file from the device."""

    # ignore exception if the file is already gone.
    def delete_temporary_file():
      try:
        if self._device_utils:
          device_path = self.name
          target_user = self._device_utils.target_user
          if target_user is not None:
            device_path = self._device_utils.ResolveSpecialPath(device_path)
          self._device_utils.RunShellCommand(
              ['rm', '-rf', device_path],
              check_return=False,
              as_root=target_user is not None,
              retries=0,
          )
        else:
          self._adb.Shell('rm -f %s' % self.name_quoted,
                          expect_status=None,
                          retries=0)
      except base_error.BaseError as e:
        # We don't really care, and stack traces clog up the log.
        # Log a warning and move on.
        logger.warning('Failed to delete temporary file %s: %s', self.name,
                       str(e))

    # It shouldn't matter when the temp file gets deleted, so do so
    # asynchronously.
    threading.Thread(
        target=delete_temporary_file,
        name='delete_temporary_file(%s)' % self._adb.GetDeviceSerial()).start()

  def __enter__(self):
    return self

  def __exit__(self, type, value, traceback):
    self.close()


class NamedDeviceTemporaryDirectory(object):
  """A named temporary directory on a device."""

  # TODO(b/293175593): Migrate to use device_utils to delete the directory.
  def __init__(self,
               adb,
               suffix='',
               prefix='tmp',
               dir='/data/local/tmp',
               device_utils=None):
    """Find an unused temporary directory path on the device. The directory is
    not created until it is used with a 'with' statement.

    When this object is closed, the directory will be deleted on the device.

    Args:
      adb: An instance of AdbWrapper
      suffix: The suffix of the name of the temporary directory.
      prefix: The prefix of the name of the temporary directory.
      dir: The directory on the device where to place the temporary directory.
      device_utils: An instance of DeviceUtils.
    Raises:
      ValueError if any of suffix, prefix, or dir are None.
    """
    self._adb = adb
    self.name = _GenerateName(prefix, suffix, dir)
    self.name_quoted = cmd_helper.SingleQuote(self.name)
    self._device_utils = device_utils

  def close(self):
    """Deletes the temporary directory from the device."""

    def delete_temporary_dir():
      try:
        if self._device_utils:
          device_path = self.name
          target_user = self._device_utils.target_user
          if target_user is not None:
            device_path = self._device_utils.ResolveSpecialPath(device_path)
          self._device_utils.RunShellCommand(
              ['rm', '-rf', device_path],
              check_return=False,
              as_root=target_user is not None,
              retries=0,
          )
        else:
          self._adb.Shell('rm -f %s' % self.name_quoted,
                          expect_status=None,
                          retries=0)
      except base_error.BaseError as e:
        # We don't really care, and stack traces clog up the log.
        # Log a warning and move on.
        logger.warning('Failed to delete temporary dir %s: %s', self.name,
                       str(e))

    threading.Thread(
        target=delete_temporary_dir,
        name='delete_temporary_dir(%s)' % self._adb.GetDeviceSerial()).start()

  def __enter__(self):
    if self._device_utils:
      device_path = self.name
      target_user = self._device_utils.target_user
      if target_user is not None:
        device_path = self._device_utils.ResolveSpecialPath(device_path)
      self._device_utils.RunShellCommand(
          ['mkdir', '-p', device_path],
          check_return=True,
          as_root=target_user is not None,
      )
    else:
      self._adb.Shell('mkdir -p %s' % self.name)
    return self

  def __exit__(self, exc_type, exc_val, exc_tb):
    self.close()
