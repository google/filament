#!/usr/bin/env vpython3
# Copyright (c) 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Smoke tests for gclient.py.

Shell out 'gclient' and run gcs tests.
"""

import logging
import os
import sys
import unittest

from unittest import mock
import gclient_smoketest_base
import subprocess2

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


class GClientSmokeGcs(gclient_smoketest_base.GClientSmokeBase):

    def setUp(self):
        super(GClientSmokeGcs, self).setUp()
        self.enabled = self.FAKE_REPOS.set_up_git()
        if not self.enabled:
            self.skipTest('git fake repos not available')
        self.env['PATH'] = (os.path.join(ROOT_DIR, 'testing_support') +
                            os.pathsep + self.env['PATH'])

    def testSyncGcs(self):
        self.gclient(['config', self.git_base + 'repo_22', '--name', 'src'])
        self.gclient(['sync'])

        tree = self.mangle_git_tree(('repo_22@1', 'src'))
        tree.update({
            'src/another_gcs_dep/extracted_dir/extracted_file':
            'extracted text',
            'src/gcs_dep/extracted_dir/extracted_file':
            'extracted text',
            'src/gcs_dep_with_output_file/clang-format-no-extract':
            'non-extractable file',
        })
        self.assertTree(tree)

    def testRevInfo(self):
        self.gclient(['config', self.git_base + 'repo_22', '--name', 'src'])
        self.gclient(['sync'])
        results = self.gclient(['revinfo'])
        out = ('src: %(base)srepo_22\n'
               'src/another_gcs_dep:Linux/llvmfile.tar.gz: '
               'gs://456bucket/Linux/llvmfile.tar.gz\n'
               'src/gcs_dep:deadbeef: gs://123bucket/deadbeef\n'
               'src/gcs_dep_with_output_file:clang-format-version123: '
               'gs://789bucket/clang-format-version123\n' % {
                   'base': self.git_base,
               })
        self.check((out, '', 0), results)

    def testRevInfoActual(self):
        self.gclient(['config', self.git_base + 'repo_22', '--name', 'src'])
        self.gclient(['sync'])
        results = self.gclient(['revinfo', '--actual'])
        out = ('src: %(base)srepo_22@%(hash1)s\n'
               'src/another_gcs_dep:Linux/llvmfile.tar.gz: '
               'gs://456bucket/Linux/llvmfile.tar.gz\n'
               'src/gcs_dep:deadbeef: gs://123bucket/deadbeef\n'
               'src/gcs_dep_with_output_file:clang-format-version123: '
               'gs://789bucket/clang-format-version123\n' % {
                   'base': self.git_base,
                   'hash1': self.githash('repo_22', 1),
               })
        self.check((out, '', 0), results)


if __name__ == '__main__':
    if '-v' in sys.argv:
        logging.basicConfig(level=logging.DEBUG)
    unittest.main()
