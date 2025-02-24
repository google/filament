#!/usr/bin/env vpython3
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for git_number.py"""

import binascii
import os
import sys

DEPOT_TOOLS_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, DEPOT_TOOLS_ROOT)

from testing_support import git_test_utils
from testing_support import coverage_utils


class Basic(git_test_utils.GitRepoReadWriteTestBase):
    REPO_SCHEMA = """
  A B C D E
    B   F E
  X Y     E
  """

    @classmethod
    def setUpClass(cls):
        super(Basic, cls).setUpClass()
        import git_number
        cls.gn = git_number
        cls.old_POOL_KIND = cls.gn.POOL_KIND
        cls.gn.POOL_KIND = 'threads'

    @classmethod
    def tearDownClass(cls):
        cls.gn.POOL_KIND = cls.old_POOL_KIND
        super(Basic, cls).tearDownClass()

    def tearDown(self):
        self.gn.clear_caches()
        super(Basic, self).tearDown()

    def _git_number(self, refs, cache=False):
        refs = [binascii.unhexlify(ref) for ref in refs]
        self.repo.run(self.gn.load_generation_numbers, refs)
        if cache:
            self.repo.run(self.gn.finalize, refs)
        return [self.gn.get_num(ref) for ref in refs]

    def testBasic(self):
        self.assertEqual([0], self._git_number([self.repo['A']]))
        self.assertEqual([2], self._git_number([self.repo['F']]))
        self.assertEqual([0], self._git_number([self.repo['X']]))
        self.assertEqual([4], self._git_number([self.repo['E']]))

    def testInProcessCache(self):
        self.assertEqual(
            None,
            self.repo.run(self.gn.get_num, binascii.unhexlify(self.repo['A'])))
        self.assertEqual([4], self._git_number([self.repo['E']]))
        self.assertEqual(
            0, self.repo.run(self.gn.get_num,
                             binascii.unhexlify(self.repo['A'])))

    def testOnDiskCache(self):
        self.assertEqual(
            None,
            self.repo.run(self.gn.get_num, binascii.unhexlify(self.repo['A'])))
        self.assertEqual([4], self._git_number([self.repo['E']], cache=True))
        self.assertEqual([4], self._git_number([self.repo['E']], cache=True))
        self.gn.clear_caches()
        self.assertEqual(
            0, self.repo.run(self.gn.get_num,
                             binascii.unhexlify(self.repo['A'])))
        self.gn.clear_caches()
        self.repo.run(self.gn.clear_caches, True)
        self.assertEqual(
            None,
            self.repo.run(self.gn.get_num, binascii.unhexlify(self.repo['A'])))


if __name__ == '__main__':
    sys.exit(
        coverage_utils.covered_main(
            os.path.join(DEPOT_TOOLS_ROOT, 'git_number.py'), '3.7'))
