# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import unittest

from telemetry.internal.platform import posix_platform_backend


class PosixPlatformBackendTest(unittest.TestCase):

  def testSudoersFileParsing(self):
    binary_path = '/usr/bin/pkill'
    self.assertFalse(
        posix_platform_backend._BinaryExistsInSudoersFiles(binary_path, ''))
    self.assertFalse(
        posix_platform_backend._BinaryExistsInSudoersFiles(
            binary_path, '    (ALL) ALL'))
    self.assertFalse(
        posix_platform_backend._BinaryExistsInSudoersFiles(
            binary_path, '     (root) NOPASSWD: /usr/bin/pkill_DUMMY'))
    self.assertFalse(
        posix_platform_backend._BinaryExistsInSudoersFiles(
            binary_path, '     (root) NOPASSWD: pkill'))


    self.assertTrue(
        posix_platform_backend._BinaryExistsInSudoersFiles(
            binary_path, '(root) NOPASSWD: /usr/bin/pkill'))
    self.assertTrue(
        posix_platform_backend._BinaryExistsInSudoersFiles(
            binary_path, '     (root) NOPASSWD: /usr/bin/pkill'))
    self.assertTrue(
        posix_platform_backend._BinaryExistsInSudoersFiles(
            binary_path, '     (root) NOPASSWD: /usr/bin/pkill arg1 arg2'))
