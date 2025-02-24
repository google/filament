#!/usr/bin/env python3
# Copyright (c) 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import os.path
import sys
import tempfile
import unittest
import unittest.mock
from unittest.mock import patch

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT_DIR)

from gclient import PRECOMMIT_HOOK_VAR
import gclient_utils
from gclient_eval import SYNC, SUBMODULES
import git_common as git


class HooksTest(unittest.TestCase):
    def setUp(self):
        super(HooksTest, self).setUp()
        self.repo = tempfile.mkdtemp()
        self.env = os.environ.copy()
        self.env['SKIP_GITLINK_PRECOMMIT'] = '0'
        self.env['TESTING_ANSWER'] = 'n'
        self.populate()

    def tearDown(self):
        gclient_utils.rmtree(self.repo)

    def write(self, repo, path, content):
        with open(os.path.join(repo, path), 'w') as f:
            f.write(content)

    def populate(self):
        git.run('init', cwd=self.repo)
        deps_content = '\n'.join((
            f'git_dependencies = "{SYNC}"',
            'deps = {',
            f'    "dep_a": "host://dep_a@{"a"*40}",',
            f'    "dep_b": "host://dep_b@{"b"*40}",',
            '}',
        ))
        self.write(self.repo, 'DEPS', deps_content)

        self.dep_a_repo = os.path.join(self.repo, 'dep_a')
        os.mkdir(self.dep_a_repo)
        git.run('init', cwd=self.dep_a_repo)
        os.mkdir(os.path.join(self.repo, 'dep_b'))
        gitmodules_content = '\n'.join((
            '[submodule "dep_a"]'
            '\tpath = dep_a',
            '\turl = host://dep_a',
            '[submodule "dep_b"]'
            '\tpath = dep_b',
            '\turl = host://dep_b',
        ))
        self.write(self.repo, '.gitmodules', gitmodules_content)
        git.run('update-index',
                '--add',
                '--cacheinfo',
                f'160000,{"a"*40},dep_a',
                cwd=self.repo)
        git.run('update-index',
                '--add',
                '--cacheinfo',
                f'160000,{"b"*40},dep_b',
                cwd=self.repo)

        git.run('add', '.', cwd=self.repo)
        git.run('commit', '-m', 'init', cwd=self.repo)

        # On Windows, this path is written to the file as
        # "root_dir\hooks\pre-commit.py", but it gets interpreted as
        # "root_dirhookspre-commit.py".
        precommit_path = os.path.join(ROOT_DIR, 'hooks',
                                      'pre-commit.py').replace('\\', '\\\\')
        precommit_content = '\n'.join((
            '#!/bin/sh',
            f'{PRECOMMIT_HOOK_VAR}={precommit_path}',
            f'if [ -f "${PRECOMMIT_HOOK_VAR}" ]; then',
            f'    python3 "${PRECOMMIT_HOOK_VAR}" || exit 1',
            'fi',
        ))
        self.write(self.repo, os.path.join('.git', 'hooks', 'pre-commit'),
                   precommit_content)
        os.chmod(os.path.join(self.repo, '.git', 'hooks', 'pre-commit'), 0o755)

    def testPreCommit_NoGitlinkOrDEPS(self):
        # Sanity check. Neither gitlinks nor DEPS are touched.
        self.write(self.repo, 'foo', 'foo')
        git.run('add', '.', cwd=self.repo)
        expected_diff = git.run('diff', '--cached', cwd=self.repo)
        git.run('commit', '-m', 'foo', cwd=self.repo)
        self.assertEqual(expected_diff,
                         git.run('diff', 'HEAD^', 'HEAD', cwd=self.repo))

    def testPreCommit_GitlinkWithoutDEPS(self):
        # Gitlink changes were staged without a corresponding DEPS change.
        self.write(self.repo, 'foo', 'foo')
        git.run('add', '.', cwd=self.repo)
        git.run('update-index',
                '--replace',
                '--cacheinfo',
                f'160000,{"b"*40},dep_a',
                cwd=self.repo)
        git.run('update-index',
                '--replace',
                '--cacheinfo',
                f'160000,{"a"*40},dep_b',
                cwd=self.repo)
        diff_before_commit = git.run('diff',
                                     '--cached',
                                     '--name-only',
                                     cwd=self.repo)
        _, stderr = git.run_with_stderr('commit',
                                        '-m',
                                        'regular file and gitlinks',
                                        cwd=self.repo,
                                        env=self.env)

        self.assertIn('dep_a', diff_before_commit)
        self.assertIn('dep_b', diff_before_commit)
        # Gitlinks should be dropped.
        self.assertIn(
            'Found no change to DEPS, but found staged gitlink(s) in diff',
            stderr)
        diff_after_commit = git.run('diff',
                                    '--name-only',
                                    'HEAD^',
                                    'HEAD',
                                    cwd=self.repo)
        self.assertNotIn('dep_a', diff_after_commit)
        self.assertNotIn('dep_b', diff_after_commit)
        self.assertIn('foo', diff_after_commit)

    def testPreCommit_IntentionalGitlinkWithoutDEPS(self):
        # Intentional Gitlink changes staged without a DEPS change.
        self.write(self.repo, 'foo', 'foo')
        git.run('add', '.', cwd=self.repo)
        git.run('update-index',
                '--replace',
                '--cacheinfo',
                f'160000,{"b"*40},dep_a',
                cwd=self.repo)
        git.run('update-index',
                '--replace',
                '--cacheinfo',
                f'160000,{"a"*40},dep_b',
                cwd=self.repo)
        diff_before_commit = git.run('diff',
                                     '--cached',
                                     '--name-only',
                                     cwd=self.repo)
        self.env['TESTING_ANSWER'] = ''
        _, stderr = git.run_with_stderr('commit',
                                        '-m',
                                        'regular file and gitlinks',
                                        cwd=self.repo,
                                        env=self.env)

        self.assertIn('dep_a', diff_before_commit)
        self.assertIn('dep_b', diff_before_commit)
        # Gitlinks should be dropped.
        self.assertIn(
            'Found no change to DEPS, but found staged gitlink(s) in diff',
            stderr)
        diff_after_commit = git.run('diff',
                                    '--name-only',
                                    'HEAD^',
                                    'HEAD',
                                    cwd=self.repo)
        self.assertIn('dep_a', diff_after_commit)
        self.assertIn('dep_b', diff_after_commit)
        self.assertIn('foo', diff_after_commit)

    def testPreCommit_OnlyGitlinkWithoutDEPS(self):
        # Gitlink changes were staged without a corresponding DEPS change but
        # no other files were included in the commit.
        git.run('update-index',
                '--replace',
                '--cacheinfo',
                f'160000,{"b"*40},dep_a',
                cwd=self.repo)
        diff_before_commit = git.run('diff',
                                     '--cached',
                                     '--name-only',
                                     cwd=self.repo)
        ret = git.run_with_retcode('commit',
                                   '-m',
                                   'gitlink only',
                                   cwd=self.repo,
                                   env=self.env)

        self.assertIn('dep_a', diff_before_commit)
        # Gitlinks should be droppped and the empty commit should be aborted.
        self.assertEqual(ret, 1)
        diff_after_commit = git.run('diff',
                                    '--cached',
                                    '--name-only',
                                    cwd=self.repo)
        self.assertNotIn('dep_a', diff_after_commit)

    def testPreCommit_CommitAll(self):
        self.write(self.repo, 'foo', 'foo')
        git.run('add', '.', cwd=self.repo)
        git.run('commit', '-m', 'add foo', cwd=self.repo)
        self.write(self.repo, 'foo', 'foo2')

        # Create a new commit in dep_a.
        self.write(self.dep_a_repo, 'sub_foo', 'sub_foo')
        git.run('add', '.', cwd=self.dep_a_repo)
        git.run('commit', '-m', 'sub_foo', cwd=self.dep_a_repo)

        diff_before_commit = git.run('status',
                                     cwd=self.repo)
        self.assertIn('foo', diff_before_commit)
        self.assertIn('dep_a', diff_before_commit)
        ret = git.run_with_retcode('commit',
                                   '--all',
                                   '-m',
                                   'commit all',
                                   cwd=self.repo,
                                   env=self.env)

        self.assertIn('dep_a', diff_before_commit)
        self.assertEqual(ret, 0)
        diff_after_commit = git.run('diff',
                                    '--cached',
                                    '--name-only',
                                    cwd=self.repo)
        self.assertNotIn('dep_a', diff_after_commit)
        diff_from_commit = git.run('diff',
                                    '--name-only',
                                    'HEAD^',
                                    'HEAD',
                                    cwd=self.repo)
        self.assertIn('foo', diff_from_commit)

    def testPreCommit_GitlinkWithDEPS(self):
        # A gitlink was staged with a corresponding DEPS change.
        updated_deps = '\n'.join((
            f'git_dependencies = "{SYNC}"',
            'deps = {',
            f'    "dep_a": "host://dep_a@{"b"*40}",',
            f'    "dep_b": "host://dep_b@{"b"*40}",',
            '}',
        ))
        self.write(self.repo, 'DEPS', updated_deps)
        git.run('add', '.', cwd=self.repo)
        git.run('update-index',
                '--replace',
                '--cacheinfo',
                f'160000,{"b"*40},dep_a',
                cwd=self.repo)
        diff_before_commit = git.run('diff', '--cached', cwd=self.repo)
        git.run('commit', '-m', 'gitlink and DEPS', cwd=self.repo)

        # There should be no changes to the commit.
        diff_after_commit = git.run('diff', 'HEAD^', 'HEAD', cwd=self.repo)
        self.assertEqual(diff_before_commit, diff_after_commit)

    def testPreCommit_SkipPrecommit(self):
        # A gitlink was staged without a corresponding DEPS change but the
        # SKIP_GITLINK_PRECOMMIT envvar was set.
        git.run('update-index',
                '--replace',
                '--cacheinfo',
                f'160000,{"b"*40},dep_a',
                cwd=self.repo)
        diff_before_commit = git.run('diff',
                                     '--cached',
                                     '--name-only',
                                     cwd=self.repo)
        self.env['SKIP_GITLINK_PRECOMMIT'] = '1'
        git.run('commit',
                '-m',
                'gitlink only, skipping precommit',
                cwd=self.repo,
                env=self.env)

        # Gitlink should be kept.
        self.assertIn('dep_a', diff_before_commit)
        diff_after_commit = git.run('diff',
                                    '--name-only',
                                    'HEAD^',
                                    'HEAD',
                                    cwd=self.repo)
        self.assertIn('dep_a', diff_after_commit)

    def testPreCommit_OtherDEPSState(self):
        # DEPS is set to a git_dependencies state other than SYNC.
        deps_content = '\n'.join((
            f'git_dependencies = \'{SUBMODULES}\'',
            'deps = {',
            f'    "dep_a": "host://dep_a@{"a"*40}",',
            f'    "dep_b": "host://dep_b@{"b"*40}",',
            '}',
        ))
        self.write(self.repo, 'DEPS', deps_content)
        git.run('add', '.', cwd=self.repo)
        git.run('commit', '-m', 'change git_dependencies', cwd=self.repo)

        git.run('update-index',
                '--replace',
                '--cacheinfo',
                f'160000,{"b"*40},dep_a',
                cwd=self.repo)
        diff_before_commit = git.run('diff', '--cached', cwd=self.repo)
        git.run('commit', '-m', 'update dep_a', cwd=self.repo)

        # There should be no changes to the commit.
        diff_after_commit = git.run('diff', 'HEAD^', 'HEAD', cwd=self.repo)
        self.assertEqual(diff_before_commit, diff_after_commit)


if __name__ == '__main__':
    unittest.main()
