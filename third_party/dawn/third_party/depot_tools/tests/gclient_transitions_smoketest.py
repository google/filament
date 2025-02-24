#!/usr/bin/env vpython3
# Copyright (c) 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Smoke tests for gclient.py.

Shell out 'gclient' and simulate the behavior of bisect bots as they transition
across DEPS changes.
"""

import logging
import os
import sys
import unittest

import gclient_smoketest_base

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT_DIR)

import scm
from testing_support import fake_repos

# TODO: Should fix these warnings.
# pylint: disable=line-too-long


class SkiaDEPSTransitionSmokeTest(gclient_smoketest_base.GClientSmokeBase):
    """Simulate the behavior of bisect bots as they transition across the Skia
    DEPS change."""

    FAKE_REPOS_CLASS = fake_repos.FakeRepoSkiaDEPS

    def setUp(self):
        super(SkiaDEPSTransitionSmokeTest, self).setUp()
        self.enabled = self.FAKE_REPOS.set_up_git()
        if not self.enabled:
            self.skipTest('git fake repos not available')

    def testSkiaDEPSChangeGit(self):
        # Create an initial checkout:
        # - Single checkout at the root.
        # - Multiple checkouts in a shared subdirectory.
        self.gclient([
            'config', '--spec', 'solutions=['
            '{"name": "src",'
            ' "url": ' + repr(self.git_base) + '+ "repo_2",'
            '}]'
        ])

        checkout_path = os.path.join(self.root_dir, 'src')
        skia = os.path.join(checkout_path, 'third_party', 'skia')
        skia_gyp = os.path.join(skia, 'gyp')
        skia_include = os.path.join(skia, 'include')
        skia_src = os.path.join(skia, 'src')

        gyp_git_url = self.git_base + 'repo_3'
        include_git_url = self.git_base + 'repo_4'
        src_git_url = self.git_base + 'repo_5'
        skia_git_url = self.FAKE_REPOS.git_base + 'repo_1'

        pre_hash = self.githash('repo_2', 1)
        post_hash = self.githash('repo_2', 2)

        # Initial sync. Verify that we get the expected checkout.
        res = self.gclient(
            ['sync', '--deps', 'mac', '--revision',
             'src@%s' % pre_hash])
        self.assertEqual(res[2], 0, 'Initial sync failed.')
        self.assertEqual(
            scm.GIT.Capture(['config', 'remote.origin.url'], skia_gyp),
            gyp_git_url)
        self.assertEqual(
            scm.GIT.Capture(['config', 'remote.origin.url'], skia_include),
            include_git_url)
        self.assertEqual(
            scm.GIT.Capture(['config', 'remote.origin.url'], skia_src),
            src_git_url)

        # Verify that the sync succeeds. Verify that we have the  expected
        # merged checkout.
        res = self.gclient(
            ['sync', '--deps', 'mac', '--revision',
             'src@%s' % post_hash])
        self.assertEqual(res[2], 0, 'DEPS change sync failed.')
        self.assertEqual(scm.GIT.Capture(['config', 'remote.origin.url'], skia),
                         skia_git_url)

        # Sync again. Verify that we still have the expected merged checkout.
        res = self.gclient(
            ['sync', '--deps', 'mac', '--revision',
             'src@%s' % post_hash])
        self.assertEqual(res[2], 0, 'Subsequent sync failed.')
        self.assertEqual(scm.GIT.Capture(['config', 'remote.origin.url'], skia),
                         skia_git_url)

        # Sync back to the original DEPS. Verify that we get the original
        # structure.
        res = self.gclient(
            ['sync', '--deps', 'mac', '--revision',
             'src@%s' % pre_hash])
        self.assertEqual(res[2], 0, 'Reverse sync failed.')
        self.assertEqual(
            scm.GIT.Capture(['config', 'remote.origin.url'], skia_gyp),
            gyp_git_url)
        self.assertEqual(
            scm.GIT.Capture(['config', 'remote.origin.url'], skia_include),
            include_git_url)
        self.assertEqual(
            scm.GIT.Capture(['config', 'remote.origin.url'], skia_src),
            src_git_url)

        # Sync again. Verify that we still have the original structure.
        res = self.gclient(
            ['sync', '--deps', 'mac', '--revision',
             'src@%s' % pre_hash])
        self.assertEqual(res[2], 0, 'Subsequent sync #2 failed.')
        self.assertEqual(
            scm.GIT.Capture(['config', 'remote.origin.url'], skia_gyp),
            gyp_git_url)
        self.assertEqual(
            scm.GIT.Capture(['config', 'remote.origin.url'], skia_include),
            include_git_url)
        self.assertEqual(
            scm.GIT.Capture(['config', 'remote.origin.url'], skia_src),
            src_git_url)


class BlinkDEPSTransitionSmokeTest(gclient_smoketest_base.GClientSmokeBase):
    """Simulate the behavior of bisect bots as they transition across the Blink
    DEPS change."""

    FAKE_REPOS_CLASS = fake_repos.FakeRepoBlinkDEPS

    def setUp(self):
        super(BlinkDEPSTransitionSmokeTest, self).setUp()
        self.enabled = self.FAKE_REPOS.set_up_git()
        if not self.enabled:
            self.skipTest('git fake repos not available')
        self.checkout_path = os.path.join(self.root_dir, 'src')
        self.blink = os.path.join(self.checkout_path, 'third_party', 'WebKit')
        self.blink_git_url = self.FAKE_REPOS.git_base + 'repo_2'
        self.pre_merge_sha = self.githash('repo_1', 1)
        self.post_merge_sha = self.githash('repo_1', 2)

    def CheckStatusPreMergePoint(self):
        self.assertEqual(
            scm.GIT.Capture(['config', 'remote.origin.url'], self.blink),
            self.blink_git_url)
        self.assertTrue(os.path.exists(join(self.blink, '.git')))
        self.assertTrue(os.path.exists(join(self.blink, 'OWNERS')))
        with open(join(self.blink, 'OWNERS')) as f:
            owners_content = f.read()
            self.assertEqual('OWNERS-pre', owners_content, 'OWNERS not updated')
        self.assertTrue(
            os.path.exists(join(self.blink, 'Source', 'exists_always')))
        self.assertTrue(
            os.path.exists(
                join(self.blink, 'Source', 'exists_before_but_not_after')))
        self.assertFalse(
            os.path.exists(
                join(self.blink, 'Source', 'exists_after_but_not_before')))

    def CheckStatusPostMergePoint(self):
        # Check that the contents still exists
        self.assertTrue(os.path.exists(join(self.blink, 'OWNERS')))
        with open(join(self.blink, 'OWNERS')) as f:
            owners_content = f.read()
            self.assertEqual('OWNERS-post', owners_content,
                             'OWNERS not updated')
        self.assertTrue(
            os.path.exists(join(self.blink, 'Source', 'exists_always')))
        # Check that file removed between the branch point are actually deleted.
        self.assertTrue(
            os.path.exists(
                join(self.blink, 'Source', 'exists_after_but_not_before')))
        self.assertFalse(
            os.path.exists(
                join(self.blink, 'Source', 'exists_before_but_not_after')))
        # But not the .git folder
        self.assertFalse(os.path.exists(join(self.blink, '.git')))

    @unittest.skip('flaky')
    def testBlinkDEPSChangeUsingGclient(self):
        """Checks that {src,blink} repos are consistent when syncing going back and
        forth using gclient sync src@revision."""
        self.gclient([
            'config', '--spec', 'solutions=['
            '{"name": "src",'
            ' "url": "' + self.git_base + 'repo_1",'
            '}]'
        ])

        # Go back and forth two times.
        for _ in range(2):
            res = self.gclient([
                'sync', '--jobs', '1', '--revision',
                'src@%s' % self.pre_merge_sha
            ])
            self.assertEqual(res[2], 0, 'DEPS change sync failed.')
            self.CheckStatusPreMergePoint()

            res = self.gclient([
                'sync', '--jobs', '1', '--revision',
                'src@%s' % self.post_merge_sha
            ])
            self.assertEqual(res[2], 0, 'DEPS change sync failed.')
            self.CheckStatusPostMergePoint()

    @unittest.skip('flaky')
    def testBlinkDEPSChangeUsingGit(self):
        """Like testBlinkDEPSChangeUsingGclient, but move the main project using
        directly git and not gclient sync."""
        self.gclient([
            'config', '--spec', 'solutions=['
            '{"name": "src",'
            ' "url": "' + self.git_base + 'repo_1",'
            ' "managed": False,'
            '}]'
        ])

        # Perform an initial sync to bootstrap the repo.
        res = self.gclient(['sync', '--jobs', '1'])
        self.assertEqual(res[2], 0, 'Initial gclient sync failed.')

        # Go back and forth two times.
        for _ in range(2):
            subprocess2.check_call(
                ['git', 'checkout', '-q', self.pre_merge_sha],
                cwd=self.checkout_path)
            res = self.gclient(['sync', '--jobs', '1'])
            self.assertEqual(res[2], 0, 'gclient sync failed.')
            self.CheckStatusPreMergePoint()

            subprocess2.check_call(
                ['git', 'checkout', '-q', self.post_merge_sha],
                cwd=self.checkout_path)
            res = self.gclient(['sync', '--jobs', '1'])
            self.assertEqual(res[2], 0, 'DEPS change sync failed.')
            self.CheckStatusPostMergePoint()

    @unittest.skip('flaky')
    def testBlinkLocalBranchesArePreserved(self):
        """Checks that the state of local git branches are effectively preserved
        when going back and forth."""
        self.gclient([
            'config', '--spec', 'solutions=['
            '{"name": "src",'
            ' "url": "' + self.git_base + 'repo_1",'
            '}]'
        ])

        # Initialize to pre-merge point.
        self.gclient(['sync', '--revision', 'src@%s' % self.pre_merge_sha])
        self.CheckStatusPreMergePoint()

        # Create a branch named "foo".
        subprocess2.check_call(['git', 'checkout', '-qB', 'foo'],
                               cwd=self.blink)

        # Cross the pre-merge point.
        self.gclient(['sync', '--revision', 'src@%s' % self.post_merge_sha])
        self.CheckStatusPostMergePoint()

        # Go backwards and check that we still have the foo branch.
        self.gclient(['sync', '--revision', 'src@%s' % self.pre_merge_sha])
        self.CheckStatusPreMergePoint()
        subprocess2.check_call(
            ['git', 'show-ref', '-q', '--verify', 'refs/heads/foo'],
            cwd=self.blink)


if __name__ == '__main__':
    if '-v' in sys.argv:
        logging.basicConfig(level=logging.DEBUG)
    unittest.main()
