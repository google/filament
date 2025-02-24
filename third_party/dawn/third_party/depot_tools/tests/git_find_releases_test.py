#!/usr/bin/env vpython3
# coding=utf-8
# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for git_find_releases.py."""

from io import StringIO
import logging
import os
import sys
import unittest
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import gclient_utils
import git_find_releases


@unittest.skipIf(gclient_utils.IsEnvCog(),
                'not supported in non-git environment')
class TestGitFindReleases(unittest.TestCase):
    @mock.patch('sys.stdout', StringIO())
    @mock.patch('git_common.run', return_value='')
    def test_invalid_commit(self, git_run):
        result = git_find_releases.main(['foo'])
        self.assertEqual(1, result)
        self.assertEqual('foo not found', sys.stdout.getvalue().strip())
        git_run.assert_called_once_with('name-rev', '--tags', '--name-only',
                                        'foo')

    @mock.patch('sys.stdout', StringIO())
    @mock.patch('git_common.run')
    def test_no_merge(self, git_run):
        def git_run_function(*args):
            assert len(args) > 1
            if args[0] == 'name-rev' and args[1] == '--tags':
                return 'undefined'

            if args[0] == 'name-rev' and args[1] == '--refs':
                return '1.0.0'

            if args[0] == 'log':
                return ''
            assert False, "Unexpected arguments for git.run"

        git_run.side_effect = git_run_function
        result = git_find_releases.main(['foo'])
        self.assertEqual(0, result)
        stdout = sys.stdout.getvalue().strip()
        self.assertIn('commit foo was', stdout)
        self.assertIn('No merges found', stdout)
        self.assertEqual(3, git_run.call_count)


if __name__ == '__main__':
    logging.basicConfig(
        level=logging.DEBUG if '-v' in sys.argv else logging.ERROR)
    unittest.main()
