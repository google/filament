#!/usr/bin/env vpython3
# Copyright (c) 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Smoke tests for gclient.py.

Shell out 'gclient' and run git tests.
"""

import json
import logging
import os
import sys
import unittest

import gclient_smoketest_base

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT_DIR)

import subprocess2
from testing_support.fake_repos import join, write

# TODO: Should fix these warnings.
# pylint: disable=line-too-long


class GClientSmokeGIT(gclient_smoketest_base.GClientSmokeBase):
    def setUp(self):
        super(GClientSmokeGIT, self).setUp()
        self.env['PATH'] = (os.path.join(ROOT_DIR, 'testing_support') +
                            os.pathsep + self.env['PATH'])
        self.env['GCLIENT_SUPPRESS_GIT_VERSION_WARNING'] = '1'
        self.env['GCLIENT_SUPPRESS_SUBMODULE_WARNING'] = '1'
        self.enabled = self.FAKE_REPOS.set_up_git()
        if not self.enabled:
            self.skipTest('git fake repos not available')

    def testGitmodules_relative(self):
        self.gclient(['config', self.git_base + 'repo_19', '--name', 'dir'],
                     cwd=self.git_base + 'repo_19')
        self.gclient(['sync'], cwd=self.git_base + 'repo_19')
        self.gclient(['gitmodules'],
                     cwd=self.git_base + os.path.join('repo_19', 'dir'))

        gitmodules = os.path.join(self.git_base, 'repo_19', 'dir',
                                  '.gitmodules')
        with open(gitmodules) as f:
            contents = f.read().splitlines()
            self.assertEqual([
                '[submodule "some_repo"]', '\tpath = some_repo',
                '\turl = /repo_2', '\tgclient-condition = not foo_checkout',
                '[submodule "chicken/dickens"]', '\tpath = chicken/dickens',
                '\turl = /repo_3', '\tgclient-recursedeps = true'
            ], contents)

    def testGitmodules_not_relative(self):
        self.gclient(['config', self.git_base + 'repo_20', '--name', 'foo'],
                     cwd=self.git_base + 'repo_20')
        self.gclient(['sync'], cwd=self.git_base + 'repo_20')
        self.gclient(['gitmodules'],
                     cwd=self.git_base + os.path.join('repo_20', 'foo'))

        gitmodules = os.path.join(self.git_base, 'repo_20', 'foo',
                                  '.gitmodules')
        with open(gitmodules) as f:
            contents = f.read().splitlines()
            self.assertEqual([
                '[submodule "some_repo"]', '\tpath = some_repo',
                '\turl = /repo_2', '\tgclient-condition = not foo_checkout',
                '[submodule "chicken/dickens"]', '\tpath = chicken/dickens',
                '\turl = /repo_3', '\tgclient-recursedeps = true'
            ], contents)

    def testGitmodules_migration_no_git_suffix(self):
        self.gclient(['config', self.git_base + 'repo_21', '--name', 'foo'],
                     cwd=self.git_base + 'repo_21')
        # We are not running gclient sync since dependencies don't exist
        self.gclient(['gitmodules'], cwd=self.git_base + 'repo_21')

        gitmodules = os.path.join(self.git_base, 'repo_21', '.gitmodules')
        with open(gitmodules) as f:
            contents = f.read().splitlines()
            self.assertEqual([
                '[submodule "bar"]',
                '\tpath = bar',
                '\turl = https://example.googlesource.com/repo.git',
            ], contents)
        # force migration
        os.remove(gitmodules)
        self.gclient(['gitmodules'], cwd=self.git_base + 'repo_21')

        gitmodules = os.path.join(self.git_base, 'repo_21', '.gitmodules')
        with open(gitmodules) as f:
            contents = f.read().splitlines()
            self.assertEqual([
                '[submodule "bar"]',
                '\tpath = bar',
                '\turl = https://example.googlesource.com/repo',
            ], contents)

    def testGitmodules_migration_recursdeps(self):
        self.gclient(['config', self.git_base + 'repo_24', '--name', 'foo'],
                     cwd=self.git_base + 'repo_24')
        # We are not running gclient sync since dependencies don't exist
        self.gclient(['gitmodules'], cwd=self.git_base + 'repo_24')

        gitmodules = os.path.join(self.git_base, 'repo_24', '.gitmodules')
        with open(gitmodules) as f:
            contents = f.read().splitlines()
            self.assertEqual([
                '[submodule "bar"]',
                '\tpath = bar',
                '\turl = https://example.googlesource.com/repo',
            ], contents)
        # force migration
        os.remove(gitmodules)
        self.gclient(['gitmodules'], cwd=self.git_base + 'repo_24')

        gitmodules = os.path.join(self.git_base, 'repo_24', '.gitmodules')
        with open(gitmodules) as f:
            contents = f.read().splitlines()
            self.assertEqual([
                '[submodule "bar"]',
                '\tpath = bar',
                '\turl = https://example.googlesource.com/repo',
                '\tgclient-recursedeps = true'
            ], contents)

    def testGitmodules_not_in_gclient(self):
        with self.assertRaisesRegex(AssertionError, 'from a gclient workspace'):
            self.gclient(['gitmodules'], cwd=self.root_dir)

    def testSync(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        # Test unversioned checkout.
        self.parseGclient(['sync', '--deps', 'mac', '--jobs', '1'],
                          ['running', 'running', 'running'])
        # TODO(maruel): http://crosbug.com/3582 hooks run even if not matching,
        # must add sync parsing to get the list of updated files.
        tree = self.mangle_git_tree(('repo_1@2', 'src'),
                                    ('repo_2@1', 'src/repo2'),
                                    ('repo_3@2', 'src/repo2/repo_renamed'))
        tree['src/git_hooked1'] = 'git_hooked1'
        tree['src/git_hooked2'] = 'git_hooked2'
        self.assertTree(tree)

        # Manually remove git_hooked1 before syncing to make sure it's not
        # recreated.
        os.remove(join(self.root_dir, 'src', 'git_hooked1'))

        # Test incremental versioned sync: sync backward.
        self.parseGclient([
            'sync', '--jobs', '1', '--revision',
            'src@' + self.githash('repo_1', 1), '--deps', 'mac',
            '--delete_unversioned_trees'
        ], ['deleting'])
        tree = self.mangle_git_tree(
            ('repo_1@1', 'src'), ('repo_2@2', 'src/repo2'),
            ('repo_3@1', 'src/repo2/repo3'), ('repo_4@2', 'src/repo4'))
        tree['src/git_hooked2'] = 'git_hooked2'
        tree['src/repo2/gclient.args'] = '\n'.join([
            '# Generated from \'DEPS\'',
            'false_var = false',
            'false_str_var = false',
            'true_var = true',
            'true_str_var = true',
            'str_var = "abc"',
            'cond_var = false',
        ])
        self.assertTree(tree)
        # Test incremental sync: delete-unversioned_trees isn't there.
        self.parseGclient(['sync', '--deps', 'mac', '--jobs', '1'],
                          ['running', 'running'])
        tree = self.mangle_git_tree(
            ('repo_1@2', 'src'), ('repo_2@1', 'src/repo2'),
            ('repo_3@1', 'src/repo2/repo3'),
            ('repo_3@2', 'src/repo2/repo_renamed'), ('repo_4@2', 'src/repo4'))
        tree['src/git_hooked1'] = 'git_hooked1'
        tree['src/git_hooked2'] = 'git_hooked2'
        tree['src/repo2/gclient.args'] = '\n'.join([
            '# Generated from \'DEPS\'',
            'false_var = false',
            'false_str_var = false',
            'true_var = true',
            'true_str_var = true',
            'str_var = "abc"',
            'cond_var = false',
        ])
        self.assertTree(tree)

    def testSyncJsonOutput(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        output_json = os.path.join(self.root_dir, 'output.json')
        self.gclient(['sync', '--deps', 'mac', '--output-json', output_json])
        with open(output_json) as f:
            output_json = json.load(f)

        out = {
            'solutions': {
                'src/': {
                    'scm': 'git',
                    'url': self.git_base + 'repo_1',
                    'revision': self.githash('repo_1', 2),
                    'was_processed': True,
                    'was_synced': True,
                },
                'src/repo2/': {
                    'scm': 'git',
                    'url':
                    self.git_base + 'repo_2@' + self.githash('repo_2', 1)[:7],
                    'revision': self.githash('repo_2', 1),
                    'was_processed': True,
                    'was_synced': True,
                },
                'src/repo2/repo_renamed/': {
                    'scm': 'git',
                    'url': self.git_base + 'repo_3',
                    'revision': self.githash('repo_3', 2),
                    'was_processed': True,
                    'was_synced': True,
                },
                'src/should_not_process/': {
                    'scm': None,
                    'url': self.git_base + 'repo_4',
                    'revision': None,
                    'was_processed': False,
                    'was_synced': True,
                },
            },
        }
        self.assertEqual(out, output_json)

    def testSyncIgnoredSolutionName(self):
        """TODO(maruel): This will become an error soon."""
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        self.parseGclient(
            [
                'sync', '--deps', 'mac', '--jobs', '1', '--revision',
                'invalid@' + self.githash('repo_1', 1)
            ], ['running', 'running', 'running'],
            'Please fix your script, having invalid --revision flags '
            'will soon be considered an error.\n')
        tree = self.mangle_git_tree(('repo_1@2', 'src'),
                                    ('repo_2@1', 'src/repo2'),
                                    ('repo_3@2', 'src/repo2/repo_renamed'))
        tree['src/git_hooked1'] = 'git_hooked1'
        tree['src/git_hooked2'] = 'git_hooked2'
        self.assertTree(tree)

    def testSyncNoSolutionName(self):
        # When no solution name is provided, gclient uses the first solution
        # listed.
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        self.parseGclient([
            'sync', '--deps', 'mac', '--jobs', '1', '--revision',
            self.githash('repo_1', 1)
        ], ['running'])
        tree = self.mangle_git_tree(
            ('repo_1@1', 'src'), ('repo_2@2', 'src/repo2'),
            ('repo_3@1', 'src/repo2/repo3'), ('repo_4@2', 'src/repo4'))
        tree['src/repo2/gclient.args'] = '\n'.join([
            '# Generated from \'DEPS\'',
            'false_var = false',
            'false_str_var = false',
            'true_var = true',
            'true_str_var = true',
            'str_var = "abc"',
            'cond_var = false',
        ])
        self.assertTree(tree)

    def testSyncJobs(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        # Test unversioned checkout.
        self.parseGclient(['sync', '--deps', 'mac', '--jobs', '8'],
                          ['running', 'running', 'running'],
                          untangle=True)
        # TODO(maruel): http://crosbug.com/3582 hooks run even if not matching,
        # must add sync parsing to get the list of updated files.
        tree = self.mangle_git_tree(('repo_1@2', 'src'),
                                    ('repo_2@1', 'src/repo2'),
                                    ('repo_3@2', 'src/repo2/repo_renamed'))
        tree['src/git_hooked1'] = 'git_hooked1'
        tree['src/git_hooked2'] = 'git_hooked2'
        self.assertTree(tree)

        # Manually remove git_hooked1 before syncing to make sure it's not
        # recreated.
        os.remove(join(self.root_dir, 'src', 'git_hooked1'))

        # Test incremental versioned sync: sync backward.
        # Use --jobs 1 otherwise the order is not deterministic.
        self.parseGclient([
            'sync', '--revision', 'src@' + self.githash('repo_1', 1), '--deps',
            'mac', '--delete_unversioned_trees', '--jobs', '1'
        ], ['deleting'],
                          untangle=True)
        tree = self.mangle_git_tree(
            ('repo_1@1', 'src'), ('repo_2@2', 'src/repo2'),
            ('repo_3@1', 'src/repo2/repo3'), ('repo_4@2', 'src/repo4'))
        tree['src/git_hooked2'] = 'git_hooked2'
        tree['src/repo2/gclient.args'] = '\n'.join([
            '# Generated from \'DEPS\'',
            'false_var = false',
            'false_str_var = false',
            'true_var = true',
            'true_str_var = true',
            'str_var = "abc"',
            'cond_var = false',
        ])
        self.assertTree(tree)
        # Test incremental sync: delete-unversioned_trees isn't there.
        self.parseGclient(['sync', '--deps', 'mac', '--jobs', '8'],
                          ['running', 'running'],
                          untangle=True)
        tree = self.mangle_git_tree(
            ('repo_1@2', 'src'), ('repo_2@1', 'src/repo2'),
            ('repo_3@1', 'src/repo2/repo3'),
            ('repo_3@2', 'src/repo2/repo_renamed'), ('repo_4@2', 'src/repo4'))
        tree['src/git_hooked1'] = 'git_hooked1'
        tree['src/git_hooked2'] = 'git_hooked2'
        tree['src/repo2/gclient.args'] = '\n'.join([
            '# Generated from \'DEPS\'',
            'false_var = false',
            'false_str_var = false',
            'true_var = true',
            'true_str_var = true',
            'str_var = "abc"',
            'cond_var = false',
        ])
        self.assertTree(tree)

    def testSyncFetch(self):
        self.gclient(['config', self.git_base + 'repo_13', '--name', 'src'])
        self.gclient([
            'sync', '-v', '-v', '-v', '--revision',
            self.githash('repo_13', 2)
        ])

    def testSyncFetchUpdate(self):
        self.gclient(['config', self.git_base + 'repo_13', '--name', 'src'])

        # Sync to an earlier revision first, one that doesn't refer to
        # non-standard refs.
        self.gclient([
            'sync', '-v', '-v', '-v', '--revision',
            self.githash('repo_13', 1)
        ])

        # Make sure update that pulls a non-standard ref works.
        self.gclient([
            'sync', '-v', '-v', '-v', '--revision',
            self.githash('repo_13', 2)
        ])

    def testSyncDirect(self):
        self.gclient(['config', self.git_base + 'repo_12', '--name', 'src'])
        self.gclient(
            ['sync', '-v', '-v', '-v', '--revision', 'refs/changes/1212'])

    def testSyncUnmanaged(self):
        self.gclient([
            'config', '--spec',
            'solutions=[{"name":"src", "url": %r, "managed": False}]' %
            (self.git_base + 'repo_5')
        ])
        self.gclient(['sync', '--revision', 'src@' + self.githash('repo_5', 2)])
        self.gclient(
            ['sync', '--revision',
             'src/repo1@%s' % self.githash('repo_1', 1)])
        # src is unmanaged, so gclient shouldn't have updated it. It should've
        # stayed synced at @2
        tree = self.mangle_git_tree(('repo_5@2', 'src'),
                                    ('repo_1@1', 'src/repo1'),
                                    ('repo_2@1', 'src/repo2'))
        tree['src/git_pre_deps_hooked'] = 'git_pre_deps_hooked'
        self.assertTree(tree)

    def testSyncUrl(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        self.gclient([
            'sync', '-v', '-v', '-v', '--revision',
            'src/repo2@%s' % self.githash('repo_2', 1), '--revision',
            '%srepo_2@%s' % (self.git_base, self.githash('repo_2', 2))
        ])
        # repo_2 should've been synced to @2 instead of @1, since URLs override
        # paths.
        tree = self.mangle_git_tree(('repo_1@2', 'src'),
                                    ('repo_2@2', 'src/repo2'),
                                    ('repo_3@2', 'src/repo2/repo_renamed'))
        tree['src/git_hooked1'] = 'git_hooked1'
        tree['src/git_hooked2'] = 'git_hooked2'
        self.assertTree(tree)

    def testSyncPatchRef(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        self.gclient([
            'sync',
            '-v',
            '-v',
            '-v',
            '--revision',
            'src/repo2@%s' % self.githash('repo_2', 1),
            '--patch-ref',
            '%srepo_2@refs/heads/main:%s' %
            (self.git_base, self.githash('repo_2', 2)),
        ])
        # Assert that repo_2 files coincide with revision @2 (the patch ref)
        tree = self.mangle_git_tree(('repo_1@2', 'src'),
                                    ('repo_2@2', 'src/repo2'),
                                    ('repo_3@2', 'src/repo2/repo_renamed'))
        tree['src/git_hooked1'] = 'git_hooked1'
        tree['src/git_hooked2'] = 'git_hooked2'
        self.assertTree(tree)
        # Assert that HEAD revision of repo_2 is @1 (the base we synced to)
        # since we should have done a soft reset.
        self.assertEqual(
            self.githash('repo_2', 1),
            self.gitrevparse(os.path.join(self.root_dir, 'src/repo2')))

    def testSyncPatchRefNoHooks(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        self.gclient([
            'sync',
            '-v',
            '-v',
            '-v',
            '--revision',
            'src/repo2@%s' % self.githash('repo_2', 1),
            '--patch-ref',
            '%srepo_2@refs/heads/main:%s' %
            (self.git_base, self.githash('repo_2', 2)),
            '--nohooks',
        ])
        # Assert that repo_2 files coincide with revision @2 (the patch ref)
        tree = self.mangle_git_tree(('repo_1@2', 'src'),
                                    ('repo_2@2', 'src/repo2'),
                                    ('repo_3@2', 'src/repo2/repo_renamed'))
        self.assertTree(tree)
        # Assert that HEAD revision of repo_2 is @1 (the base we synced to)
        # since we should have done a soft reset.
        self.assertEqual(
            self.githash('repo_2', 1),
            self.gitrevparse(os.path.join(self.root_dir, 'src/repo2')))

    def testInstallHooks_no_existing_hook(self):
        repo = os.path.join(self.git_base, 'repo_18')
        self.gclient(['config', repo, '--name', 'src'], cwd=repo)
        self.gclient(['sync'], cwd=repo)
        self.gclient(['installhooks'], cwd=repo)

        hook = os.path.join(repo, 'src', '.git', 'hooks', 'pre-commit')
        with open(hook) as f:
            contents = f.read()
            self.assertIn('python3 "$GCLIENT_PRECOMMIT"', contents)

        # Later runs should only inform that the hook was already installed.
        stdout, _, _ = self.gclient(['installhooks'], cwd=repo)
        self.assertIn(f'{hook} already contains the gclient pre-commit hook.',
                      stdout)

    def testInstallHooks_existing_hook(self):
        repo = os.path.join(self.git_base, 'repo_19')
        self.gclient(['config', repo, '--name', 'src'], cwd=repo)
        self.gclient(['sync'], cwd=repo)

        # Create an existing pre-commit hook.
        hook = os.path.join(repo, 'src', '.git', 'hooks', 'pre-commit')
        hook_contents = ['#!/bin/sh', 'echo hook']
        with open(hook, 'w') as f:
            f.write('\n'.join(hook_contents))

        stdout, _, _ = self.gclient(['installhooks'], cwd=repo)
        self.assertIn('Please append the following lines to the hook', stdout)

        # The orignal hook is left unchanged.
        with open(hook) as f:
            contents = f.read().splitlines()
            self.assertEqual(hook_contents, contents)

    def testRunHooks(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        self.gclient(['sync', '--deps', 'mac'])
        tree = self.mangle_git_tree(('repo_1@2', 'src'),
                                    ('repo_2@1', 'src/repo2'),
                                    ('repo_3@2', 'src/repo2/repo_renamed'))
        tree['src/git_hooked1'] = 'git_hooked1'
        tree['src/git_hooked2'] = 'git_hooked2'
        self.assertTree(tree)

        os.remove(join(self.root_dir, 'src', 'git_hooked1'))
        os.remove(join(self.root_dir, 'src', 'git_hooked2'))
        # runhooks runs all hooks even if not matching by design.
        out = self.parseGclient(['runhooks', '--deps', 'mac'],
                                ['running', 'running'])
        self.assertEqual(1, len(out[0]))
        self.assertEqual(1, len(out[1]))
        tree = self.mangle_git_tree(('repo_1@2', 'src'),
                                    ('repo_2@1', 'src/repo2'),
                                    ('repo_3@2', 'src/repo2/repo_renamed'))
        tree['src/git_hooked1'] = 'git_hooked1'
        tree['src/git_hooked2'] = 'git_hooked2'
        self.assertTree(tree)

    def testRunHooksCondition(self):
        self.gclient(['config', self.git_base + 'repo_7', '--name', 'src'])
        self.gclient(['sync', '--deps', 'mac'])
        tree = self.mangle_git_tree(('repo_7@1', 'src'))
        tree['src/should_run'] = 'should_run'
        self.assertTree(tree)

    def testPreDepsHooks(self):
        self.gclient(['config', self.git_base + 'repo_5', '--name', 'src'])
        expectation = [
            ('running', self.root_dir),  # git clone
            ('running', self.root_dir),  # pre-deps hook
        ]
        out = self.parseGclient([
            'sync', '--deps', 'mac', '--jobs=1', '--revision',
            'src@' + self.githash('repo_5', 2)
        ], expectation)
        self.assertEqual('Cloning into ', out[0][1][:13])
        # parseGClient may produce hook slowness warning, so we expect either 2
        # or 3 blocks.
        self.assertIn(len(out[1]), [2, 3], out[1])
        self.assertEqual('pre-deps hook', out[1][1])
        tree = self.mangle_git_tree(('repo_5@2', 'src'),
                                    ('repo_1@2', 'src/repo1'),
                                    ('repo_2@1', 'src/repo2'))
        tree['src/git_pre_deps_hooked'] = 'git_pre_deps_hooked'
        self.assertTree(tree)

        os.remove(join(self.root_dir, 'src', 'git_pre_deps_hooked'))

        # Pre-DEPS hooks don't run with runhooks.
        self.gclient(['runhooks', '--deps', 'mac'])
        tree = self.mangle_git_tree(('repo_5@2', 'src'),
                                    ('repo_1@2', 'src/repo1'),
                                    ('repo_2@1', 'src/repo2'))
        self.assertTree(tree)

        # Pre-DEPS hooks run when syncing with --nohooks.
        self.gclient([
            'sync', '--deps', 'mac', '--nohooks', '--revision',
            'src@' + self.githash('repo_5', 2)
        ])
        tree = self.mangle_git_tree(('repo_5@2', 'src'),
                                    ('repo_1@2', 'src/repo1'),
                                    ('repo_2@1', 'src/repo2'))
        tree['src/git_pre_deps_hooked'] = 'git_pre_deps_hooked'
        self.assertTree(tree)

        os.remove(join(self.root_dir, 'src', 'git_pre_deps_hooked'))

        # Pre-DEPS hooks don't run with --noprehooks
        self.gclient([
            'sync', '--deps', 'mac', '--noprehooks', '--revision',
            'src@' + self.githash('repo_5', 2)
        ])
        tree = self.mangle_git_tree(('repo_5@2', 'src'),
                                    ('repo_1@2', 'src/repo1'),
                                    ('repo_2@1', 'src/repo2'))
        self.assertTree(tree)

    def testPreDepsHooksError(self):
        self.gclient(['config', self.git_base + 'repo_5', '--name', 'src'])
        expectated_stdout = [
            ('running', self.root_dir),  # git clone
            ('running', self.root_dir),  # pre-deps hook
            ('running', self.root_dir),  # pre-deps hook (fails)
        ]
        expected_stderr = (
            "Error: Command 'python3 -c import sys; "
            "sys.exit(1)' returned non-zero exit status 1 in %s\n" %
            (self.root_dir))
        stdout, stderr, retcode = self.gclient([
            'sync', '--deps', 'mac', '--jobs=1', '--revision',
            'src@' + self.githash('repo_5', 3)
        ],
                                               error_ok=True)
        self.assertEqual(stderr, expected_stderr)
        self.assertEqual(2, retcode)
        self.checkBlock(stdout, expectated_stdout)

    def testRevInfo(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        self.gclient(['sync', '--deps', 'mac'])
        results = self.gclient(['revinfo', '--deps', 'mac'])
        out = ('src: %(base)srepo_1\n'
               'src/repo2: %(base)srepo_2@%(hash2)s\n'
               'src/repo2/repo_renamed: %(base)srepo_3\n' % {
                   'base': self.git_base,
                   'hash2': self.githash('repo_2', 1)[:7],
               })
        self.check((out, '', 0), results)

    def testRevInfoActual(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        self.gclient(['sync', '--deps', 'mac'])
        results = self.gclient(['revinfo', '--deps', 'mac', '--actual'])
        out = ('src: %(base)srepo_1@%(hash1)s\n'
               'src/repo2: %(base)srepo_2@%(hash2)s\n'
               'src/repo2/repo_renamed: %(base)srepo_3@%(hash3)s\n' % {
                   'base': self.git_base,
                   'hash1': self.githash('repo_1', 2),
                   'hash2': self.githash('repo_2', 1),
                   'hash3': self.githash('repo_3', 2),
               })
        self.check((out, '', 0), results)

    def testRevInfoFilterPath(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        self.gclient(['sync', '--deps', 'mac'])
        results = self.gclient(['revinfo', '--deps', 'mac', '--filter', 'src'])
        out = ('src: %(base)srepo_1\n' % {
            'base': self.git_base,
        })
        self.check((out, '', 0), results)

    def testRevInfoFilterURL(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        self.gclient(['sync', '--deps', 'mac'])
        results = self.gclient([
            'revinfo', '--deps', 'mac', '--filter',
            '%srepo_2' % self.git_base
        ])
        out = ('src/repo2: %(base)srepo_2@%(hash2)s\n' % {
            'base': self.git_base,
            'hash2': self.githash('repo_2', 1)[:7],
        })
        self.check((out, '', 0), results)

    def testRevInfoFilterURLOrPath(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        self.gclient(['sync', '--deps', 'mac'])
        results = self.gclient([
            'revinfo', '--deps', 'mac', '--filter', 'src', '--filter',
            '%srepo_2' % self.git_base
        ])
        out = ('src: %(base)srepo_1\n'
               'src/repo2: %(base)srepo_2@%(hash2)s\n' % {
                   'base': self.git_base,
                   'hash2': self.githash('repo_2', 1)[:7],
               })
        self.check((out, '', 0), results)

    def testRevInfoJsonOutput(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        self.gclient(['sync', '--deps', 'mac'])
        output_json = os.path.join(self.root_dir, 'output.json')
        self.gclient(['revinfo', '--deps', 'mac', '--output-json', output_json])
        with open(output_json) as f:
            output_json = json.load(f)

        out = {
            'src': {
                'url': self.git_base + 'repo_1',
                'rev': None,
            },
            'src/repo2': {
                'url': self.git_base + 'repo_2',
                'rev': self.githash('repo_2', 1)[:7],
            },
            'src/repo2/repo_renamed': {
                'url': self.git_base + 'repo_3',
                'rev': None,
            },
        }
        self.assertEqual(out, output_json)

    def testRevInfoJsonOutputSnapshot(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        self.gclient(['sync', '--deps', 'mac'])
        output_json = os.path.join(self.root_dir, 'output.json')
        self.gclient([
            'revinfo', '--deps', 'mac', '--snapshot', '--output-json',
            output_json
        ])
        with open(output_json) as f:
            output_json = json.load(f)

        out = [{
            'solution_url': self.git_base + 'repo_1',
            'managed': True,
            'name': 'src',
            'deps_file': 'DEPS',
            'custom_deps': {
                'src/repo2':
                '%srepo_2@%s' % (self.git_base, self.githash('repo_2', 1)),
                'src/repo2/repo_renamed':
                '%srepo_3@%s' % (self.git_base, self.githash('repo_3', 2)),
                'src/should_not_process':
                None,
            },
        }]
        self.assertEqual(out, output_json)

    def testSetDep(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        fake_deps = os.path.join(self.git_base, 'repo_1', 'DEPS')
        with open(fake_deps, 'w') as f:
            f.write('\n'.join([
                'vars = { ',
                '  "foo_var": "foo_val",',
                '  "foo_rev": "foo_rev",',
                '}',
                'deps = {',
                '  "foo": {',
                '    "url": "url@{foo_rev}",',
                '  },',
                '  "bar": "url@bar_rev",',
                '}',
            ]))

        self.gclient([
            'setdep', '-r', 'foo@new_foo', '-r', 'bar@new_bar', '--var',
            'foo_var=new_val', '--deps-file', fake_deps
        ],
                     cwd=self.git_base + 'repo_1')

        with open(fake_deps) as f:
            contents = f.read().splitlines()

        self.assertEqual([
            'vars = { ',
            '  "foo_var": "new_val",',
            '  "foo_rev": "new_foo",',
            '}',
            'deps = {',
            '  "foo": {',
            '    "url": "url@{foo_rev}",',
            '  },',
            '  "bar": "url@new_bar",',
            '}',
        ], contents)

    def testSetDep_Submodules(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        with open(os.path.join(self.git_base, '.gclient'), 'w') as f:
            f.write('')

        fake_deps = os.path.join(self.git_base, 'repo_1', 'DEPS')
        gitmodules = os.path.join(self.git_base, 'repo_1', '.gitmodules')
        with open(fake_deps, 'w') as f:
            f.write('\n'.join([
                'git_dependencies = "SYNC"',
                'vars = { ',
                '  "foo_var": "foo_val",',
                '  "foo_rev": "foo_rev",',
                '}',
                'deps = { ',
                '  "repo_1/foo": "https://foo" + Var("foo_rev"),',
                '  "repo_1/bar": "https://bar@barrev",',
                '}',
            ]))

        with open(gitmodules, 'w') as f:
            f.write('\n'.join([
                '[submodule "foo"]', '  url = https://foo', '  path = foo',
                '[submodule "bar"]', '  url = https://bar', '  path = bar'
            ]))

        subprocess2.call([
            'git', 'update-index', '--add', '--cacheinfo',
            '160000,aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa,foo'
        ],
                         cwd=self.git_base + 'repo_1')
        subprocess2.call([
            'git', 'update-index', '--add', '--cacheinfo',
            '160000,aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa,bar'
        ],
                         cwd=self.git_base + 'repo_1')

        self.gclient([
            'setdep',
            '-r',
            'repo_1/foo@new_foo',
            '--var',
            'foo_var=new_val',
            '-r',
            'repo_1/bar@bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb',
        ],
                     cwd=self.git_base + 'repo_1')

        with open(fake_deps) as f:
            contents = f.read().splitlines()

        self.assertEqual([
            'git_dependencies = "SYNC"',
            'vars = { ',
            '  "foo_var": "new_val",',
            '  "foo_rev": "new_foo",',
            '}',
            'deps = { ',
            '  "repo_1/foo": "https://foo" + Var("foo_rev"),',
            '  "repo_1/bar": '
            '"https://bar@bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",',
            '}',
        ], contents)

    def testSetDep_Submodules_relative(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        fake_deps = os.path.join(self.git_base, 'repo_1', 'DEPS')
        gitmodules = os.path.join(self.git_base, 'repo_1', '.gitmodules')
        with open(fake_deps, 'w') as f:
            f.write('\n'.join([
                'git_dependencies = "SUBMODULES"',
                'use_relative_paths = True',
                'vars = { ',
                '  "foo_var": "foo_val",',
                '  "foo_rev": "foo_rev",',
                '}',
            ]))

        with open(gitmodules, 'w') as f:
            f.write('\n'.join(
                ['[submodule "foo"]', '  url = https://foo', '  path = foo']))

        subprocess2.call([
            'git', 'update-index', '--add', '--cacheinfo',
            '160000,aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa,foo'
        ],
                         cwd=self.git_base + 'repo_1')

        self.gclient([
            'setdep', '-r', 'foo@new_foo', '--var', 'foo_var=new_val',
            '--deps-file', fake_deps
        ],
                     cwd=self.git_base + 'repo_1')

        with open(fake_deps) as f:
            contents = f.read().splitlines()

        self.assertEqual([
            'git_dependencies = "SUBMODULES"',
            'use_relative_paths = True',
            'vars = { ',
            '  "foo_var": "new_val",',
            '  "foo_rev": "foo_rev",',
            '}',
        ], contents)

    def testSetDep_BuiltinVariables(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'],
                     cwd=self.git_base)

        fake_deps = os.path.join(self.root_dir, 'DEPS')
        with open(fake_deps, 'w') as f:
            f.write('\n'.join([
                'vars = { ',
                '  "foo_var": "foo_val",',
                '  "foo_rev": "foo_rev",',
                '}',
                'deps = {',
                '  "foo": {',
                '    "url": "url@{foo_rev}",',
                '  },',
                '  "bar": "url@bar_rev",',
                '}',
                'hooks = [{',
                '  "name": "uses_builtin_var",',
                '  "pattern": ".",',
                '  "action": ["python3", "fake.py",',
                '             "--with-android={checkout_android}"],',
                '}]',
            ]))

        self.gclient([
            'setdep', '-r', 'foo@new_foo', '-r', 'bar@new_bar', '--var',
            'foo_var=new_val', '--deps-file', fake_deps
        ],
                     cwd=self.git_base + 'repo_1')

        with open(fake_deps) as f:
            contents = f.read().splitlines()

        self.assertEqual([
            'vars = { ',
            '  "foo_var": "new_val",',
            '  "foo_rev": "new_foo",',
            '}',
            'deps = {',
            '  "foo": {',
            '    "url": "url@{foo_rev}",',
            '  },',
            '  "bar": "url@new_bar",',
            '}',
            'hooks = [{',
            '  "name": "uses_builtin_var",',
            '  "pattern": ".",',
            '  "action": ["python3", "fake.py",',
            '             "--with-android={checkout_android}"],',
            '}]',
        ], contents)

    def testGetDep(self):
        fake_deps = os.path.join(self.root_dir, 'DEPS.fake')
        with open(fake_deps, 'w') as f:
            f.write('\n'.join([
                'vars = { ',
                '  "foo_var": "foo_val",',
                '  "foo_rev": "foo_rev",',
                '}',
                'deps = {',
                '  "foo": {',
                '    "url": "url@{foo_rev}",',
                '  },',
                '  "bar": "url@bar_rev",',
                '}',
            ]))

        results = self.gclient([
            'getdep', '-r', 'foo', '-r', 'bar', '--var', 'foo_var',
            '--deps-file', fake_deps
        ])

        self.assertEqual([
            'foo_val',
            'foo_rev',
            'bar_rev',
        ], results[0].splitlines())

    def testGetDep_Submodule(self):
        self.gclient(['config', self.git_base + 'repo_20', '--name', 'src'])
        subprocess2.call([
            'git', 'update-index', '--add', '--cacheinfo',
            '160000,aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa,bar'
        ],
                         cwd=self.git_base + 'repo_20')

        results = self.gclient([
            'getdep', '-r', 'foo/bar:lemur', '-r', 'bar', '--var',
            'foo_checkout'
        ],
                               cwd=self.git_base + 'repo_20')

        self.assertEqual([
            'True', 'version:1234', 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa'
        ], results[0].splitlines())

    def testGetDep_BuiltinVariables(self):
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])
        fake_deps = os.path.join(self.root_dir, 'DEPS.fake')
        with open(fake_deps, 'w') as f:
            f.write('\n'.join([
                'vars = { ',
                '  "foo_var": "foo_val",',
                '  "foo_rev": "foo_rev",',
                '}',
                'deps = {',
                '  "foo": {',
                '    "url": "url@{foo_rev}",',
                '  },',
                '  "bar": "url@bar_rev",',
                '}',
                'hooks = [{',
                '  "name": "uses_builtin_var",',
                '  "pattern": ".",',
                '  "action": ["python3", "fake.py",',
                '             "--with-android={checkout_android}"],',
                '}]',
            ]))

        results = self.gclient([
            'getdep', '-r', 'foo', '-r', 'bar', '--var', 'foo_var',
            '--deps-file', fake_deps
        ])

        self.assertEqual([
            'foo_val',
            'foo_rev',
            'bar_rev',
        ], results[0].splitlines())

    # TODO(crbug.com/1024683): Enable for windows.
    @unittest.skipIf(sys.platform == 'win32', 'not yet fixed on win')
    def testFlatten(self):
        output_deps = os.path.join(self.root_dir, 'DEPS.flattened')
        self.assertFalse(os.path.exists(output_deps))

        self.gclient([
            'config',
            self.git_base + 'repo_6',
            '--name',
            'src',
            # This should be ignored because 'custom_true_var' isn't
            # defined in the DEPS.
            '--custom-var',
            'custom_true_var=True',
            # This should override 'true_var=True' from the DEPS.
            '--custom-var',
            'true_var="False"'
        ])
        self.gclient(['sync'])
        self.gclient(
            ['flatten', '-v', '-v', '-v', '--output-deps', output_deps])

        # Assert we can sync to the flattened DEPS we just wrote.
        solutions = [{
            "url": self.git_base + 'repo_6',
            'name': 'src',
            'deps_file': output_deps
        }]
        self.gclient(['sync', '--spec=solutions=%s' % solutions])

        with open(output_deps) as f:
            deps_contents = f.read()

        self.assertEqual([
            'gclient_gn_args_file = "src/repo2/gclient.args"',
            'gclient_gn_args = [\'false_var\', \'false_str_var\', \'true_var\', '
            '\'true_str_var\', \'str_var\', \'cond_var\']',
            'allowed_hosts = [',
            '  "' + self.git_base + '",',
            ']',
            '',
            'deps = {',
            '  # "src" -> "src/repo2" -> "foo/bar"',
            '  "foo/bar": {',
            '    "url": "' + self.git_base + 'repo_3",',
            '    "condition": \'(repo2_false_var) and (true_str_var)\',',
            '  },',
            '',
            '  # "src"',
            '  "src": {',
            '    "url": "' + self.git_base + 'repo_6",',
            '  },',
            '',
            '  # "src" -> "src/mac_repo"',
            '  "src/mac_repo": {',
            '    "url": "' + self.git_base + 'repo_5",',
            '    "condition": \'checkout_mac\',',
            '  },',
            '',
            '  # "src" -> "src/repo8" -> "src/recursed_os_repo"',
            '  "src/recursed_os_repo": {',
            '    "url": "' + self.git_base + 'repo_5",',
            '    "condition": \'(checkout_linux) or (checkout_mac)\',',
            '  },',
            '',
            '  # "src" -> "src/repo15"',
            '  "src/repo15": {',
            '    "url": "' + self.git_base + 'repo_15",',
            '  },',
            '',
            '  # "src" -> "src/repo16"',
            '  "src/repo16": {',
            '    "url": "' + self.git_base + 'repo_16",',
            '  },',
            '',
            '  # "src" -> "src/repo2"',
            '  "src/repo2": {',
            '    "url": "' + self.git_base + 'repo_2@%s",' %
            (self.githash('repo_2', 1)[:7]),
            '    "condition": \'true_str_var\',',
            '  },',
            '',
            '  # "src" -> "src/repo4"',
            '  "src/repo4": {',
            '    "url": "' + self.git_base + 'repo_4",',
            '    "condition": \'False\',',
            '  },',
            '',
            '  # "src" -> "src/repo8"',
            '  "src/repo8": {',
            '    "url": "' + self.git_base + 'repo_8",',
            '  },',
            '',
            '  # "src" -> "src/unix_repo"',
            '  "src/unix_repo": {',
            '    "url": "' + self.git_base + 'repo_5",',
            '    "condition": \'checkout_linux\',',
            '  },',
            '',
            '  # "src" -> "src/win_repo"',
            '  "src/win_repo": {',
            '    "url": "' + self.git_base + 'repo_5",',
            '    "condition": \'checkout_win\',',
            '  },',
            '',
            '}',
            '',
            'hooks = [',
            '  # "src"',
            '  {',
            '    "pattern": ".",',
            '    "condition": \'True\',',
            '    "cwd": ".",',
            '    "action": [',
            '        "python3",',
            '        "-c",',
            '        "open(\'src/git_hooked1\', \'w\')'
            '.write(\'git_hooked1\')",',
            '    ]',
            '  },',
            '',
            '  # "src"',
            '  {',
            '    "pattern": "nonexistent",',
            '    "cwd": ".",',
            '    "action": [',
            '        "python3",',
            '        "-c",',
            '        "open(\'src/git_hooked2\', \'w\').write(\'git_hooked2\')",',
            '    ]',
            '  },',
            '',
            '  # "src"',
            '  {',
            '    "pattern": ".",',
            '    "condition": \'checkout_mac\',',
            '    "cwd": ".",',
            '    "action": [',
            '        "python3",',
            '        "-c",',
            '        "open(\'src/git_hooked_mac\', \'w\').write('
            '\'git_hooked_mac\')",',
            '    ]',
            '  },',
            '',
            '  # "src" -> "src/repo15"',
            '  {',
            '    "name": "absolute_cwd",',
            '    "pattern": ".",',
            '    "cwd": ".",',
            '    "action": [',
            '        "python3",',
            '        "-c",',
            '        "pass",',
            '    ]',
            '  },',
            '',
            '  # "src" -> "src/repo16"',
            '  {',
            '    "name": "relative_cwd",',
            '    "pattern": ".",',
            '    "cwd": "src/repo16",',
            '    "action": [',
            '        "python3",',
            '        "relative.py",',
            '    ]',
            '  },',
            '',
            ']',
            '',
            'vars = {',
            '  # "src"',
            '  "DummyVariable": \'repo\',',
            '',
            '  # "src"',
            '  "cond_var": \'false_str_var and true_var\',',
            '',
            '  # "src"',
            '  "false_str_var": \'False\',',
            '',
            '  # "src"',
            '  "false_var": False,',
            '',
            '  # "src"',
            '  "git_base": \'' + self.git_base + '\',',
            '',
            '  # "src"',
            '  "hook1_contents": \'git_hooked1\',',
            '',
            '  # "src" -> "src/repo2"',
            '  "repo2_false_var": \'False\',',
            '',
            '  # "src"',
            '  "repo5_var": \'/repo_5\',',
            '',
            '  # "src"',
            '  "str_var": \'abc\',',
            '',
            '  # "src"',
            '  "true_str_var": \'True\',',
            '',
            '  # "src" [custom_var override]',
            '  "true_var": \'False\',',
            '',
            '}',
            '',
            '# ' + self.git_base + 'repo_15, DEPS',
            '# ' + self.git_base + 'repo_16, DEPS',
            '# ' + self.git_base + 'repo_2@%s, DEPS' %
            (self.githash('repo_2', 1)[:7]),
            '# ' + self.git_base + 'repo_6, DEPS',
            '# ' + self.git_base + 'repo_8, DEPS',
        ], deps_contents.splitlines())

    # TODO(crbug.com/1024683): Enable for windows.
    @unittest.skipIf(sys.platform == 'win32', 'not yet fixed on win')
    def testFlattenPinAllDeps(self):
        output_deps = os.path.join(self.root_dir, 'DEPS.flattened')
        self.assertFalse(os.path.exists(output_deps))

        self.gclient(['config', self.git_base + 'repo_6', '--name', 'src'])
        self.gclient(['sync', '--process-all-deps'])
        self.gclient([
            'flatten', '-v', '-v', '-v', '--output-deps', output_deps,
            '--pin-all-deps'
        ])

        with open(output_deps) as f:
            deps_contents = f.read()

        self.assertEqual([
            'gclient_gn_args_file = "src/repo2/gclient.args"',
            'gclient_gn_args = [\'false_var\', \'false_str_var\', \'true_var\', '
            '\'true_str_var\', \'str_var\', \'cond_var\']',
            'allowed_hosts = [',
            '  "' + self.git_base + '",',
            ']',
            '',
            'deps = {',
            '  # "src" -> "src/repo2" -> "foo/bar"',
            '  "foo/bar": {',
            '    "url": "' + self.git_base + 'repo_3@%s",' %
            (self.githash('repo_3', 2)),
            '    "condition": \'(repo2_false_var) and (true_str_var)\',',
            '  },',
            '',
            '  # "src"',
            '  "src": {',
            '    "url": "' + self.git_base + 'repo_6@%s",' %
            (self.githash('repo_6', 1)),
            '  },',
            '',
            '  # "src" -> "src/mac_repo"',
            '  "src/mac_repo": {',
            '    "url": "' + self.git_base + 'repo_5@%s",' %
            (self.githash('repo_5', 3)),
            '    "condition": \'checkout_mac\',',
            '  },',
            '',
            '  # "src" -> "src/repo8" -> "src/recursed_os_repo"',
            '  "src/recursed_os_repo": {',
            '    "url": "' + self.git_base + 'repo_5@%s",' %
            (self.githash('repo_5', 3)),
            '    "condition": \'(checkout_linux) or (checkout_mac)\',',
            '  },',
            '',
            '  # "src" -> "src/repo15"',
            '  "src/repo15": {',
            '    "url": "' + self.git_base + 'repo_15@%s",' %
            (self.githash('repo_15', 1)),
            '  },',
            '',
            '  # "src" -> "src/repo16"',
            '  "src/repo16": {',
            '    "url": "' + self.git_base + 'repo_16@%s",' %
            (self.githash('repo_16', 1)),
            '  },',
            '',
            '  # "src" -> "src/repo2"',
            '  "src/repo2": {',
            '    "url": "' + self.git_base + 'repo_2@%s",' %
            (self.githash('repo_2', 1)),
            '    "condition": \'true_str_var\',',
            '  },',
            '',
            '  # "src" -> "src/repo4"',
            '  "src/repo4": {',
            '    "url": "' + self.git_base + 'repo_4@%s",' %
            (self.githash('repo_4', 2)),
            '    "condition": \'False\',',
            '  },',
            '',
            '  # "src" -> "src/repo8"',
            '  "src/repo8": {',
            '    "url": "' + self.git_base + 'repo_8@%s",' %
            (self.githash('repo_8', 1)),
            '  },',
            '',
            '  # "src" -> "src/unix_repo"',
            '  "src/unix_repo": {',
            '    "url": "' + self.git_base + 'repo_5@%s",' %
            (self.githash('repo_5', 3)),
            '    "condition": \'checkout_linux\',',
            '  },',
            '',
            '  # "src" -> "src/win_repo"',
            '  "src/win_repo": {',
            '    "url": "' + self.git_base + 'repo_5@%s",' %
            (self.githash('repo_5', 3)),
            '    "condition": \'checkout_win\',',
            '  },',
            '',
            '}',
            '',
            'hooks = [',
            '  # "src"',
            '  {',
            '    "pattern": ".",',
            '    "condition": \'True\',',
            '    "cwd": ".",',
            '    "action": [',
            '        "python3",',
            '        "-c",',
            '        "open(\'src/git_hooked1\', \'w\')'
            '.write(\'git_hooked1\')",',
            '    ]',
            '  },',
            '',
            '  # "src"',
            '  {',
            '    "pattern": "nonexistent",',
            '    "cwd": ".",',
            '    "action": [',
            '        "python3",',
            '        "-c",',
            '        "open(\'src/git_hooked2\', \'w\').write(\'git_hooked2\')",',
            '    ]',
            '  },',
            '',
            '  # "src"',
            '  {',
            '    "pattern": ".",',
            '    "condition": \'checkout_mac\',',
            '    "cwd": ".",',
            '    "action": [',
            '        "python3",',
            '        "-c",',
            '        "open(\'src/git_hooked_mac\', \'w\').write('
            '\'git_hooked_mac\')",',
            '    ]',
            '  },',
            '',
            '  # "src" -> "src/repo15"',
            '  {',
            '    "name": "absolute_cwd",',
            '    "pattern": ".",',
            '    "cwd": ".",',
            '    "action": [',
            '        "python3",',
            '        "-c",',
            '        "pass",',
            '    ]',
            '  },',
            '',
            '  # "src" -> "src/repo16"',
            '  {',
            '    "name": "relative_cwd",',
            '    "pattern": ".",',
            '    "cwd": "src/repo16",',
            '    "action": [',
            '        "python3",',
            '        "relative.py",',
            '    ]',
            '  },',
            '',
            ']',
            '',
            'vars = {',
            '  # "src"',
            '  "DummyVariable": \'repo\',',
            '',
            '  # "src"',
            '  "cond_var": \'false_str_var and true_var\',',
            '',
            '  # "src"',
            '  "false_str_var": \'False\',',
            '',
            '  # "src"',
            '  "false_var": False,',
            '',
            '  # "src"',
            '  "git_base": \'' + self.git_base + '\',',
            '',
            '  # "src"',
            '  "hook1_contents": \'git_hooked1\',',
            '',
            '  # "src" -> "src/repo2"',
            '  "repo2_false_var": \'False\',',
            '',
            '  # "src"',
            '  "repo5_var": \'/repo_5\',',
            '',
            '  # "src"',
            '  "str_var": \'abc\',',
            '',
            '  # "src"',
            '  "true_str_var": \'True\',',
            '',
            '  # "src"',
            '  "true_var": True,',
            '',
            '}',
            '',
            '# ' + self.git_base + 'repo_15@%s, DEPS' %
            (self.githash('repo_15', 1)),
            '# ' + self.git_base + 'repo_16@%s, DEPS' %
            (self.githash('repo_16', 1)),
            '# ' + self.git_base + 'repo_2@%s, DEPS' %
            (self.githash('repo_2', 1)),
            '# ' + self.git_base + 'repo_6@%s, DEPS' %
            (self.githash('repo_6', 1)),
            '# ' + self.git_base + 'repo_8@%s, DEPS' %
            (self.githash('repo_8', 1)),
        ], deps_contents.splitlines())

    # TODO(crbug.com/1024683): Enable for windows.
    @unittest.skipIf(sys.platform == 'win32', 'not yet fixed on win')
    def testFlattenRecursedeps(self):
        output_deps = os.path.join(self.root_dir, 'DEPS.flattened')
        self.assertFalse(os.path.exists(output_deps))

        output_deps_files = os.path.join(self.root_dir, 'DEPS.files')
        self.assertFalse(os.path.exists(output_deps_files))

        self.gclient(['config', self.git_base + 'repo_10', '--name', 'src'])
        self.gclient(['sync', '--process-all-deps'])
        self.gclient([
            'flatten', '-v', '-v', '-v', '--output-deps', output_deps,
            '--output-deps-files', output_deps_files
        ])

        with open(output_deps) as f:
            deps_contents = f.read()

        self.assertEqual([
            'gclient_gn_args_file = "src/repo8/gclient.args"',
            "gclient_gn_args = ['str_var']",
            'deps = {',
            '  # "src"',
            '  "src": {',
            '    "url": "' + self.git_base + 'repo_10",',
            '  },',
            '',
            '  # "src" -> "src/repo9" -> "src/repo8" -> "src/recursed_os_repo"',
            '  "src/recursed_os_repo": {',
            '    "url": "' + self.git_base + 'repo_5",',
            '    "condition": \'(checkout_linux) or (checkout_mac)\',',
            '  },',
            '',
            '  # "src" -> "src/repo11"',
            '  "src/repo11": {',
            '    "url": "' + self.git_base + 'repo_11",',
            '    "condition": \'(checkout_ios) or (checkout_mac)\',',
            '  },',
            '',
            '  # "src" -> "src/repo11" -> "src/repo12"',
            '  "src/repo12": {',
            '    "url": "' + self.git_base + 'repo_12",',
            '    "condition": \'(checkout_ios) or (checkout_mac)\',',
            '  },',
            '',
            '  # "src" -> "src/repo9" -> "src/repo4"',
            '  "src/repo4": {',
            '    "url": "' + self.git_base + 'repo_4",',
            '    "condition": \'checkout_android\',',
            '  },',
            '',
            '  # "src" -> "src/repo6"',
            '  "src/repo6": {',
            '    "url": "' + self.git_base + 'repo_6",',
            '  },',
            '',
            '  # "src" -> "src/repo9" -> "src/repo7"',
            '  "src/repo7": {',
            '    "url": "' + self.git_base + 'repo_7",',
            '  },',
            '',
            '  # "src" -> "src/repo9" -> "src/repo8"',
            '  "src/repo8": {',
            '    "url": "' + self.git_base + 'repo_8",',
            '  },',
            '',
            '  # "src" -> "src/repo9"',
            '  "src/repo9": {',
            '    "url": "' + self.git_base + 'repo_9",',
            '  },',
            '',
            '}',
            '',
            'vars = {',
            '  # "src" -> "src/repo9"',
            '  "str_var": \'xyz\',',
            '',
            '}',
            '',
            '# ' + self.git_base + 'repo_10, DEPS',
            '# ' + self.git_base + 'repo_11, DEPS',
            '# ' + self.git_base + 'repo_8, DEPS',
            '# ' + self.git_base + 'repo_9, DEPS',
        ], deps_contents.splitlines())

        with open(output_deps_files) as f:
            deps_files_contents = json.load(f)

        self.assertEqual([
            {
                'url': self.git_base + 'repo_10',
                'deps_file': 'DEPS',
                'hierarchy': [['src', self.git_base + 'repo_10']]
            },
            {
                'url':
                self.git_base + 'repo_11',
                'deps_file':
                'DEPS',
                'hierarchy': [['src', self.git_base + 'repo_10'],
                              ['src/repo11', self.git_base + 'repo_11']]
            },
            {
                'url':
                self.git_base + 'repo_8',
                'deps_file':
                'DEPS',
                'hierarchy': [['src', self.git_base + 'repo_10'],
                              ['src/repo9', self.git_base + 'repo_9'],
                              ['src/repo8', self.git_base + 'repo_8']]
            },
            {
                'url':
                self.git_base + 'repo_9',
                'deps_file':
                'DEPS',
                'hierarchy': [['src', self.git_base + 'repo_10'],
                              ['src/repo9', self.git_base + 'repo_9']]
            },
        ], deps_files_contents)

    # TODO(crbug.com/1024683): Enable for windows.
    @unittest.skipIf(sys.platform == 'win32', 'not yet fixed on win')
    def testFlattenCipd(self):
        output_deps = os.path.join(self.root_dir, 'DEPS.flattened')
        self.assertFalse(os.path.exists(output_deps))

        self.gclient(['config', self.git_base + 'repo_14', '--name', 'src'])
        self.gclient(['sync'])
        self.gclient(
            ['flatten', '-v', '-v', '-v', '--output-deps', output_deps])

        with open(output_deps) as f:
            deps_contents = f.read()

        self.assertEqual([
            'deps = {',
            '  # "src"',
            '  "src": {',
            '    "url": "' + self.git_base + 'repo_14",',
            '  },',
            '',
            '  # "src" -> src/another_cipd_dep',
            '  "src/another_cipd_dep": {',
            '    "packages": [',
            '      {',
            '        "package": "package1",',
            '        "version": "1.1-cr0",',
            '      },',
            '      {',
            '        "package": "package2",',
            '        "version": "1.13",',
            '      },',
            '    ],',
            '    "dep_type": "cipd",',
            '  },',
            '',
            '  # "src" -> src/cipd_dep',
            '  "src/cipd_dep": {',
            '    "packages": [',
            '      {',
            '        "package": "package0",',
            '        "version": "0.1",',
            '      },',
            '    ],',
            '    "dep_type": "cipd",',
            '  },',
            '',
            '  # "src" -> src/cipd_dep_with_cipd_variable',
            '  "src/cipd_dep_with_cipd_variable": {',
            '    "packages": [',
            '      {',
            '        "package": "package3/${{platform}}",',
            '        "version": "1.2",',
            '      },',
            '    ],',
            '    "dep_type": "cipd",',
            '  },',
            '',
            '}',
            '',
            '# ' + self.git_base + 'repo_14, DEPS',
        ], deps_contents.splitlines())

    def testRelativeGNArgsFile(self):
        self.gclient(['config', self.git_base + 'repo_17', '--name', 'src'])
        self.gclient([
            'sync',
        ])

        tree = self.mangle_git_tree(('repo_17@1', 'src'))
        tree['src/repo17_gclient.args'] = '\n'.join([
            '# Generated from \'DEPS\'',
            'toto = "tata"',
        ])
        self.assertTree(tree)


if __name__ == '__main__':
    if '-v' in sys.argv:
        logging.basicConfig(level=logging.DEBUG)
    unittest.main()
