#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# pylint: disable=protected-access

import logging
import sys  # pylint: disable=unused-import
import unittest

from unittest import mock

from devil import devil_env
from devil.android.ndk import abis


class _MockDeviceUtils(object):
  def __init__(self):
    self.product_cpu_abi = abis.ARM_64


class DevilEnvTest(unittest.TestCase):
  def testGetEnvironmentVariableConfig_configType(self):
    with mock.patch('os.environ.get',
                    mock.Mock(side_effect=lambda _env_var: None)):
      env_config = devil_env._GetEnvironmentVariableConfig()
    self.assertEqual('BaseConfig', env_config.get('config_type'))

  def testGetEnvironmentVariableConfig_noEnv(self):
    with mock.patch('os.environ.get',
                    mock.Mock(side_effect=lambda _env_var: None)):
      env_config = devil_env._GetEnvironmentVariableConfig()
    self.assertEqual({}, env_config.get('dependencies'))

  def testGetEnvironmentVariableConfig_adbPath(self):
    def mock_environment(env_var):
      return '/my/fake/adb/path' if env_var == 'ADB_PATH' else None

    with mock.patch('os.environ.get', mock.Mock(side_effect=mock_environment)):
      env_config = devil_env._GetEnvironmentVariableConfig()
    self.assertEqual(
        {
            'adb': {
                'file_info': {
                    'linux2_x86_64': {
                        'local_paths': ['/my/fake/adb/path'],
                    },
                },
            },
        }, env_config.get('dependencies'))

  def testGetPlatform(self):
    with mock.patch('platform.machine', mock.Mock(return_value='x86_64')):
      with mock.patch('sys.platform', mock.Mock(return_value='linux2')):
        platform = devil_env.GetPlatform()
        self.assertEqual(platform, 'linux2_x86_64')
      with mock.patch('sys.platform', mock.Mock(return_value='linux')):
        platform = devil_env.GetPlatform()
        self.assertEqual(platform, 'linux2_x86_64')

    platform = devil_env.GetPlatform(arch='arm64-v8a')
    self.assertEqual(platform, 'android_arm64-v8a')

    device = _MockDeviceUtils()
    platform = devil_env.GetPlatform(device=device)
    self.assertEqual(platform, 'android_arm64-v8a')


if __name__ == '__main__':
  logging.getLogger().setLevel(logging.DEBUG)
  unittest.main(verbosity=2)
