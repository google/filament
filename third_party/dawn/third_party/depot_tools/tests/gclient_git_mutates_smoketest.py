#!/usr/bin/env vpython3
# Copyright (c) 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Smoke tests for gclient.py.

Shell out 'gclient' and run git tests.
"""

import logging
import os
import sys
import unittest

import gclient_smoketest_base

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT_DIR)

import subprocess2
from testing_support.fake_repos import join, write


class GClientSmokeGITMutates(gclient_smoketest_base.GClientSmokeBase):
    """testRevertAndStatus mutates the git repo so move it to its own suite."""
    def setUp(self):
        super(GClientSmokeGITMutates, self).setUp()
        self.enabled = self.FAKE_REPOS.set_up_git()
        if not self.enabled:
            self.skipTest('git fake repos not available')

    # TODO(crbug.com/1024683): Enable for windows.
    @unittest.skipIf(sys.platform == 'win32', 'not yet fixed on win')
    def testRevertAndStatus(self):
        # Commit new change to repo to make repo_2's hash use a custom_var.
        cur_deps = self.FAKE_REPOS.git_hashes['repo_1'][-1][1]['DEPS']
        repo_2_hash = self.FAKE_REPOS.git_hashes['repo_2'][1][0][:7]
        new_deps = cur_deps.replace('repo_2@%s\'' % repo_2_hash,
                                    'repo_2@\' + Var(\'r2hash\')')
        new_deps = 'vars = {\'r2hash\': \'%s\'}\n%s' % (repo_2_hash, new_deps)
        self.FAKE_REPOS._commit_git('repo_1', {  # pylint: disable=protected-access
          'DEPS': new_deps,
          'origin': 'git/repo_1@3\n',
        })

        config_template = ''.join([
            'solutions = [{'
            '  "name"        : "src",'
            '  "url"         : %(git_base)r + "repo_1",'
            '  "deps_file"   : "DEPS",'
            '  "managed"     : True,'
            '  "custom_vars" : %(custom_vars)s,'
            '}]'
        ])

        self.gclient([
            'config', '--spec', config_template % {
                'git_base': self.git_base,
                'custom_vars': {}
            }
        ])

        # Tested in testSync.
        self.gclient(['sync', '--deps', 'mac'])
        write(join(self.root_dir, 'src', 'repo2', 'hi'), 'Hey!')

        out = self.parseGclient(['status', '--deps', 'mac', '--jobs', '1'], [])
        # TODO(maruel): http://crosbug.com/3584 It should output the unversioned
        # files.
        self.assertEqual(0, len(out))

        # Revert implies --force implies running hooks without looking at
        # pattern matching. For each expected path, 'git reset' and 'git clean'
        # are run, so there should be two results for each. The last two results
        # should reflect writing git_hooked1 and git_hooked2. There's only one
        # result for the third because it is clean and has no output for 'git
        # clean'.
        out = self.parseGclient(['revert', '--deps', 'mac', '--jobs', '1'],
                                ['running', 'running'])
        self.assertEqual(2, len(out))
        tree = self.mangle_git_tree(('repo_1@3', 'src'),
                                    ('repo_2@1', 'src/repo2'),
                                    ('repo_3@2', 'src/repo2/repo_renamed'))
        tree['src/git_hooked1'] = 'git_hooked1'
        tree['src/git_hooked2'] = 'git_hooked2'
        self.assertTree(tree)

        # Make a new commit object in the origin repo, to force reset to fetch.
        self.FAKE_REPOS._commit_git(
            'repo_2',
            {  # pylint: disable=protected-access
                'origin': 'git/repo_2@3\n',
            })

        self.gclient([
            'config', '--spec', config_template % {
                'git_base': self.git_base,
                'custom_vars': {
                    'r2hash': self.FAKE_REPOS.git_hashes['repo_2'][-1][0]
                }
            }
        ])
        out = self.parseGclient(['revert', '--deps', 'mac', '--jobs', '1'],
                                ['running', 'running'])
        self.assertEqual(2, len(out))
        tree = self.mangle_git_tree(('repo_1@3', 'src'),
                                    ('repo_2@3', 'src/repo2'),
                                    ('repo_3@2', 'src/repo2/repo_renamed'))
        tree['src/git_hooked1'] = 'git_hooked1'
        tree['src/git_hooked2'] = 'git_hooked2'
        self.assertTree(tree)

        results = self.gclient(['status', '--deps', 'mac', '--jobs', '1'])
        out = results[0].splitlines(False)
        # TODO(maruel): http://crosbug.com/3584 It should output the unversioned
        # files.
        self.assertEqual(0, len(out))

    def testSyncNoHistory(self):
        # Create an extra commit in repo_2 and point DEPS to its hash.
        cur_deps = self.FAKE_REPOS.git_hashes['repo_1'][-1][1]['DEPS']
        repo_2_hash_old = self.FAKE_REPOS.git_hashes['repo_2'][1][0][:7]
        self.FAKE_REPOS._commit_git('repo_2', {  # pylint: disable=protected-access
          'last_file': 'file created in last commit',
        })
        repo_2_hash_new = self.FAKE_REPOS.git_hashes['repo_2'][-1][0]
        new_deps = cur_deps.replace(repo_2_hash_old, repo_2_hash_new)
        self.assertNotEqual(new_deps, cur_deps)
        self.FAKE_REPOS._commit_git('repo_1', {  # pylint: disable=protected-access
          'DEPS': new_deps,
          'origin': 'git/repo_1@4\n',
        })

        config_template = ''.join([
            'solutions = [{'
            '  "name"        : "src",'
            '  "url"         : %(git_base)r + "repo_1",'
            '  "deps_file"   : "DEPS",'
            '  "managed"     : True,'
            '}]'
        ])

        self.gclient(
            ['config', '--spec', config_template % {
                'git_base': self.git_base
            }])

        self.gclient(['sync', '--no-history', '--deps', 'mac'])
        repo2_root = join(self.root_dir, 'src', 'repo2')

        # Check that repo_2 is actually shallow and its log has only one entry.
        rev_lists = subprocess2.check_output(['git', 'rev-list', 'HEAD'],
                                             cwd=repo2_root).decode('utf-8')
        self.assertEqual(repo_2_hash_new, rev_lists.strip('\r\n'))

        # Check that we have actually checked out the right commit.
        self.assertTrue(os.path.exists(join(repo2_root, 'last_file')))


if __name__ == '__main__':
    if '-v' in sys.argv:
        logging.basicConfig(level=logging.DEBUG)
    unittest.main()
