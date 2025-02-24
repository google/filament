#!/usr/bin/env python
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys
import unittest

from unittest import mock

if __name__ == '__main__':
  sys.path.append(
      os.path.abspath(
          os.path.join(os.path.dirname(__file__), '..', '..', '..')))

from devil.android import device_utils
from devil.android.sdk import adb_wrapper
from devil.android.sdk import version_codes
from devil.android.tools import system_app

_PACKAGE_NAME = 'com.android'
_PACKAGE_PATH = '/path/to/com.android.apk'
_PM_LIST_PACKAGES_COMMAND = [
    'pm', 'list', 'packages', '-f', '-u', _PACKAGE_NAME
]
_PM_LIST_PACKAGES_OUTPUT_WITH_PATH = [
    'package:/path/to/other=' + _PACKAGE_NAME + '.other',
    'package:' + _PACKAGE_PATH + '=' + _PACKAGE_NAME
]
_PM_LIST_PACKAGES_OUTPUT_WITHOUT_PATH = [
    'package:/path/to/other=' + _PACKAGE_NAME + '.other'
]


class SystemAppTest(unittest.TestCase):
  def testDoubleEnableModification(self):
    """Ensures that system app modification logic isn't repeated.

    If EnableSystemAppModification uses are nested, inner calls should
    not need to perform any of the expensive modification logic.
    """
    # pylint: disable=no-self-use,protected-access
    mock_device = mock.Mock(spec=device_utils.DeviceUtils)
    mock_device.adb = mock.Mock(spec=adb_wrapper.AdbWrapper)
    type(mock_device).build_version_sdk = mock.PropertyMock(
        return_value=version_codes.LOLLIPOP)

    system_props = {}

    def dict_setprop(prop_name, value):
      system_props[prop_name] = value

    def dict_getprop(prop_name):
      return system_props.get(prop_name, '')

    mock_device.SetProp.side_effect = dict_setprop
    mock_device.GetProp.side_effect = dict_getprop

    with system_app.EnableSystemAppModification(mock_device):
      mock_device.EnableRoot.assert_called_once_with()
      mock_device.GetProp.assert_called_once_with(
          system_app._ENABLE_MODIFICATION_PROP)
      mock_device.SetProp.assert_called_once_with(
          system_app._ENABLE_MODIFICATION_PROP, '1')
      mock_device.reset_mock()

      with system_app.EnableSystemAppModification(mock_device):
        self.assertFalse(mock_device.EnableRoot.mock_calls)  # assert not called
        mock_device.GetProp.assert_called_once_with(
            system_app._ENABLE_MODIFICATION_PROP)
        self.assertFalse(mock_device.SetProp.mock_calls)  # assert not called
        mock_device.reset_mock()

    mock_device.SetProp.assert_called_once_with(
        system_app._ENABLE_MODIFICATION_PROP, '0')

  def test_GetApplicationPaths_found(self):
    """Path found in output along with another package having similar name."""
    # pylint: disable=protected-access
    mock_device = mock.Mock(spec=device_utils.DeviceUtils)
    mock_device.RunShellCommand.configure_mock(
        return_value=_PM_LIST_PACKAGES_OUTPUT_WITH_PATH)

    paths = system_app._GetApplicationPaths(mock_device, _PACKAGE_NAME)

    self.assertEqual([_PACKAGE_PATH], paths)
    mock_device.RunShellCommand.assert_called_once_with(
        _PM_LIST_PACKAGES_COMMAND, check_return=True)

  def test_GetApplicationPaths_notFound(self):
    """Path not found in output, only another package with similar name."""
    # pylint: disable=protected-access
    mock_device = mock.Mock(spec=device_utils.DeviceUtils)
    mock_device.RunShellCommand.configure_mock(
        return_value=_PM_LIST_PACKAGES_OUTPUT_WITHOUT_PATH)

    paths = system_app._GetApplicationPaths(mock_device, _PACKAGE_NAME)

    self.assertEqual([], paths)
    mock_device.RunShellCommand.assert_called_once_with(
        _PM_LIST_PACKAGES_COMMAND, check_return=True)

  def test_GetApplicationPaths_noPaths(self):
    """Nothing containing text of package name found in output."""
    # pylint: disable=protected-access
    mock_device = mock.Mock(spec=device_utils.DeviceUtils)
    mock_device.RunShellCommand.configure_mock(return_value=[])

    paths = system_app._GetApplicationPaths(mock_device, _PACKAGE_NAME)

    self.assertEqual([], paths)
    mock_device.RunShellCommand.assert_called_once_with(
        _PM_LIST_PACKAGES_COMMAND, check_return=True)

  def test_GetApplicationPaths_emptyName(self):
    """Called with empty name, should not return any packages."""
    # pylint: disable=protected-access
    mock_device = mock.Mock(spec=device_utils.DeviceUtils)
    mock_device.RunShellCommand.configure_mock(
        return_value=_PM_LIST_PACKAGES_OUTPUT_WITH_PATH)

    paths = system_app._GetApplicationPaths(mock_device, '')

    self.assertEqual([], paths)
    mock_device.RunShellCommand.assert_called_once_with(
        _PM_LIST_PACKAGES_COMMAND[:-1] + [''], check_return=True)


if __name__ == '__main__':
  unittest.main()
