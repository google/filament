#!/usr/bin/env vpython3
# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import platform
import sys
import unittest
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import detect_host_arch


class DetectHostArchTest(unittest.TestCase):
    def setUp(self):
        super(DetectHostArchTest, self).setUp()
        mock.patch('platform.machine').start()
        mock.patch('platform.processor').start()
        mock.patch('platform.architecture').start()
        self.addCleanup(mock.patch.stopall)

    def testHostArch(self):
        test_cases = [
            ('ia86', '', [''], 'x86'),
            ('i86pc', '', [''], 'x86'),
            ('x86_64', '', [''], 'x64'),
            ('amd64', '', [''], 'x64'),
            ('x86_64', '', ['32bit'], 'x86'),
            ('amd64', '', ['32bit'], 'x86'),
            ('arm', '', [''], 'arm'),
            ('aarch64', '', [''], 'arm64'),
            ('aarch64', '', ['32bit'], 'arm'),
            ('arm64', '', [''], 'arm64'),
            ('amd64', 'ARMv8 (64-bit) Family', ['64bit', 'WindowsPE'], 'x64'),
            ('arm64', 'ARMv8 (64-bit) Family', ['32bit', 'WindowsPE'], 'x64'),
            ('mips64', '', [''], 'mips64'),
            ('mips', '', [''], 'mips'),
            ('ppc', '', [''], 'ppc'),
            ('foo', 'powerpc', [''], 'ppc'),
            ('s390', '', [''], 's390'),
        ]

        for machine, processor, arch, expected in test_cases:
            platform.machine.return_value = machine
            platform.processor.return_value = processor
            platform.architecture.return_value = arch
            detect_host_arch.HostArch.cache_clear()
            self.assertEqual(expected, detect_host_arch.HostArch())


if __name__ == '__main__':
    unittest.main()
