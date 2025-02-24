# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import unittest
from unittest import mock

from telemetry import decorators
from telemetry.core import util
from telemetry.internal.platform import linux_platform_backend


def _PathMatches(wanted_path):
  return lambda path: path == wanted_path


class LinuxPlatformBackendTest(unittest.TestCase):
  @decorators.Enabled('linux')
  def testGetOSVersionNameSaucy(self):
    path = os.path.join(util.GetUnittestDataDir(), 'ubuntu-saucy-lsb-release')
    with open(path) as f:
      unbuntu_saucy_lsb_release_content = f.read()

    with mock.patch.object(os.path, 'exists',
                           side_effect=_PathMatches('/etc/lsb-release')):
      with mock.patch.object(
          linux_platform_backend.LinuxPlatformBackend, 'GetFileContents',
          return_value=unbuntu_saucy_lsb_release_content) as mock_method:
        backend = linux_platform_backend.LinuxPlatformBackend()
        self.assertEqual(backend.GetOSVersionName(), 'saucy')
        mock_method.assert_called_once_with('/etc/lsb-release')

  @decorators.Enabled('linux')
  def testGetOSVersionNameArch(self):
    path = os.path.join(util.GetUnittestDataDir(), 'arch-os-release')
    with open(path) as f:
      arch_os_release_content = f.read()

    with mock.patch.object(os.path, 'exists',
                           side_effect=_PathMatches('/usr/lib/os-release')):
      with mock.patch.object(
          linux_platform_backend.LinuxPlatformBackend, 'GetFileContents',
          return_value=arch_os_release_content) as mock_method:
        backend = linux_platform_backend.LinuxPlatformBackend()
        self.assertEqual(backend.GetOSVersionName(), 'arch')
        mock_method.assert_called_once_with('/usr/lib/os-release')

  @decorators.Enabled('linux')
  def testGetOSVersionNameFedora(self):
    path = os.path.join(util.GetUnittestDataDir(), 'fedora-os-release')
    with open(path) as f:
      fedora_os_release_content = f.read()

    with mock.patch.object(os.path, 'exists',
                           side_effect=_PathMatches('/etc/os-release')):
      with mock.patch.object(
          linux_platform_backend.LinuxPlatformBackend, 'GetFileContents',
          return_value=fedora_os_release_content) as mock_method:
        backend = linux_platform_backend.LinuxPlatformBackend()
        self.assertEqual(backend.GetOSVersionName(), 'fedora')
        mock_method.assert_called_once_with('/etc/os-release')
