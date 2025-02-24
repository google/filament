#!/usr/bin/env vpython3
# coding=utf-8
# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Tests for git_map."""

import io
import os
import re
import sys
import unittest
from unittest import mock

DEPOT_TOOLS_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, DEPOT_TOOLS_ROOT)

from testing_support import git_test_utils

import git_map
import git_common

git_common.TEST_MODE = True
GitRepo = git_test_utils.GitRepo


class GitMapTest(git_test_utils.GitRepoReadOnlyTestBase):
    REPO_SCHEMA = """"
  A B C D 😋 F G
    B H I J K
          J L
  """

    def setUp(self):
        # Include branch_K, branch_L to make sure that ABCDEFG all get the
        # same commit hashes as self.repo. Otherwise they get committed with the
        # wrong timestamps, due to commit ordering.
        # TODO(iannucci): Make commit timestamps deterministic in left to right,
        # top to bottom order, not in lexi-topographical order.
        origin_schema = git_test_utils.GitRepoSchema(
            """
    A B C D 😋 F G M N O
      B H I J K
            J L
    """, self.getRepoContent)
        self.origin = origin_schema.reify()
        self.origin.git('checkout', 'main')
        self.origin.git('branch', '-d', *['branch_' + l for l in 'KLG'])

        self.repo.git('remote', 'add', 'origin', self.origin.repo_path)
        self.repo.git('config', '--add', 'remote.origin.fetch',
                      '+refs/tags/*:refs/tags/*')
        self.repo.git('update-ref', 'refs/remotes/origin/main', 'tag_E')
        self.repo.git('branch', '--set-upstream-to', 'branch_G', 'branch_K')
        self.repo.git('branch', '--set-upstream-to', 'branch_K', 'branch_L')

        self.repo.git('fetch', 'origin')
        mock.patch('git_map.RESET', '').start()
        mock.patch('git_map.BLUE_BACK', '').start()
        mock.patch('git_map.BRIGHT_RED', '').start()
        mock.patch('git_map.CYAN', '').start()
        mock.patch('git_map.GREEN', '').start()
        mock.patch('git_map.MAGENTA', '').start()
        mock.patch('git_map.RED', '').start()
        mock.patch('git_map.WHITE', '').start()
        mock.patch('git_map.YELLOW', '').start()
        self.addCleanup(mock.patch.stopall)

    def testHelp(self):
        outbuf = io.BytesIO()
        self.repo.run(git_map.main, ['-h'], outbuf)
        self.assertIn(b'usage: git map [-h] [--help] [<args>]',
                      outbuf.getvalue())

    def testGitMap(self):
        expected = os.linesep.join([
            '* 6e85e877ea	(tag_O, origin/main, origin/branch_O) 1970-01-30 ~ O',
            '* 4705470871	(tag_N) 1970-01-28 ~ N',
            '* 8761b1a94f	(tag_M) 1970-01-26 ~ M',
            '* 5e7ce08691	(tag_G) 1970-01-24 ~ G',
            '* 78543ed411	(tag_F) 1970-01-18 ~ F',
            '* f5c2b77013	(tag_😋) 1970-01-16 ~ 😋',
            '* 5249c43079	(tag_D) 1970-01-10 ~ D',
            '* 072ade676a	(tag_C) 1970-01-06 ~ C',
            '| * e77da937d5	(branch_G) 1970-01-26 ~ G',
            '| * acda9677fd	1970-01-20 ~ F',
            '| * b4bed3c8e1	1970-01-18 ~ 😋',
            '| * 5da071fda9	1970-01-12 ~ D',
            '| * 1ef9b2e4ca	1970-01-08 ~ C',
            '| | * ddd611f619	(branch_L) 1970-01-24 ~ L',
            '| | | * f07cbd8cfc	(branch_K) 1970-01-22 ~ K',
            '| | |/  ',
            '| | * fb7da24708	1970-01-16 ~ J    <(branch_L)',
            '| | * bb168f6d65	1970-01-14 ~ I',
            '| | * ee1032effa	1970-01-10 ~ H',
            '| |/  ',
            '| * db57edd2c0	1970-01-06 ~ B    <(branch_K)',
            '| * e4f775f844	(root_A) 1970-01-04 ~ A',
            '| * 2824d6d8b6	(tag_L, origin/branch_L) 1970-01-22 ~ L',
            '| | * 4e599306f0	(tag_K, origin/branch_K) 1970-01-20 ~ K',
            '| |/  ',
            '| * 332f1b4499	(tag_J) 1970-01-14 ~ J',
            '| * 2fc0bc5ee5	(tag_I) 1970-01-12 ~ I',
            '| * 6e0ab26451	(tag_H) 1970-01-08 ~ H',
            '|/  ',
            '* 315457dbe8	(tag_B) 1970-01-04 ~ B',
            '* cd589e62d8	(tag_A, origin/root_A) 1970-01-02 ~ A',
            '* 7026d3d68e	(tag_", root_", main, branch_") 1970-01-02 ~ "',
        ])
        outbuf = io.BytesIO()
        self.repo.run(git_map.main, [], outbuf)
        output = outbuf.getvalue()
        output = re.sub(br'.\[\d\dm', b'', output)
        output = re.sub(br'.\[m', b'', output)
        self.assertEqual(output.splitlines(),
                         expected.encode('utf-8').splitlines())


if __name__ == '__main__':
    unittest.main()
