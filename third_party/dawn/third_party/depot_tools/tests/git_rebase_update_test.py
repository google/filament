#!/usr/bin/env vpython3
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for git_rebase_update.py"""

import os
import sys

DEPOT_TOOLS_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, DEPOT_TOOLS_ROOT)

from testing_support import coverage_utils
from testing_support import git_test_utils

# TODO: Should fix these warnings.
# pylint: disable=line-too-long


class GitRebaseUpdateTest(git_test_utils.GitRepoReadWriteTestBase):
    REPO_SCHEMA = """
  A B C D E F G
    B H I J K
          J L
  """

    @classmethod
    def getRepoContent(cls, commit):
        # Every commit X gets a file X with the content X
        return {commit: {'data': commit.encode('utf-8')}}

    @classmethod
    def setUpClass(cls):
        super(GitRebaseUpdateTest, cls).setUpClass()
        import git_rebase_update, git_new_branch, git_reparent_branch, git_common
        import git_rename_branch
        cls.reup = git_rebase_update
        cls.rp = git_reparent_branch
        cls.nb = git_new_branch
        cls.mv = git_rename_branch
        cls.gc = git_common
        cls.gc.TEST_MODE = True

    def setUp(self):
        super(GitRebaseUpdateTest, self).setUp()
        # Include branch_K, branch_L to make sure that ABCDEFG all get the
        # same commit hashes as self.repo. Otherwise they get committed with the
        # wrong timestamps, due to commit ordering.
        # TODO(iannucci): Make commit timestamps deterministic in left to right,
        # top to bottom order, not in lexi-topographical order.
        origin_schema = git_test_utils.GitRepoSchema(
            """
    A B C D E F G M N O
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
        self.repo.git('branch', '--set-upstream-to', 'origin/main', 'branch_G')

        self.repo.to_schema_refs += ['origin/main']

    def tearDown(self):
        self.origin.nuke()
        super(GitRebaseUpdateTest, self).tearDown()

    def testRebaseUpdate(self):
        self.repo.git('checkout', 'branch_K')

        self.repo.run(self.nb.main, ['foobar'])
        self.assertEqual(
            self.repo.git('rev-parse', 'HEAD').stdout,
            self.repo.git('rev-parse', 'origin/main').stdout)

        with self.repo.open('foobar', 'w') as f:
            f.write('this is the foobar file')
        self.repo.git('add', 'foobar')
        self.repo.git_commit('foobar1')

        with self.repo.open('foobar', 'w') as f:
            f.write('totes the Foobar file')
        self.repo.git_commit('foobar2')

        self.repo.run(self.nb.main, ['--upstream-current', 'int1_foobar'])
        self.repo.run(self.nb.main, ['--upstream-current', 'int2_foobar'])
        self.repo.run(self.nb.main, ['--upstream-current', 'sub_foobar'])
        with self.repo.open('foobar', 'w') as f:
            f.write('some more foobaring')
        self.repo.git('add', 'foobar')
        self.repo.git_commit('foobar3')

        self.repo.git('checkout', 'branch_K')
        self.repo.run(self.nb.main, ['--upstream-current', 'sub_K'])
        with self.repo.open('K', 'w') as f:
            f.write('This depends on K')
        self.repo.git_commit('sub_K')

        self.repo.run(self.nb.main, ['old_branch'])
        self.repo.git('reset', '--hard', self.repo['A'])
        with self.repo.open('old_file', 'w') as f:
            f.write('old_files we want to keep around')
        self.repo.git('add', 'old_file')
        self.repo.git_commit('old_file')
        self.repo.git('config', 'branch.old_branch.dormant', 'true')

        self.repo.git('checkout', 'origin/main')

        self.assertSchema("""
    A B H I J K sub_K
            J L
      B C D E foobar1 foobar2 foobar3
            E F G
    A old_file
    """)
        self.assertEqual(self.repo['A'], self.origin['A'])
        self.assertEqual(self.repo['E'], self.origin['E'])

        with self.repo.open('bob', 'wb') as f:
            f.write(b'testing auto-freeze/thaw')

        output, _ = self.repo.capture_stdio(self.reup.main)
        self.assertIn('Cannot rebase-update', output)

        self.repo.run(self.nb.main, ['empty_branch'])
        self.repo.run(self.nb.main, ['--upstream-current', 'empty_branch2'])

        self.repo.git('checkout', 'branch_K')

        output, _ = self.repo.capture_stdio(self.reup.main)

        self.assertIn('Rebasing: branch_G', output)
        self.assertIn('Rebasing: branch_K', output)
        self.assertIn('Rebasing: branch_L', output)
        self.assertIn('Rebasing: foobar', output)
        self.assertIn('Rebasing: sub_K', output)
        self.assertIn('Deleted branch branch_G', output)
        self.assertIn('Deleted branch empty_branch', output)
        self.assertIn('Deleted branch empty_branch2', output)
        self.assertIn('Deleted branch int1_foobar', output)
        self.assertIn('Deleted branch int2_foobar', output)
        self.assertIn('Reparented branch_K to track origin/main', output)
        self.assertIn('Reparented sub_foobar to track foobar', output)

        self.assertSchema("""
    A B C D E F G M N O H I J K sub_K
                              K L
                      O foobar1 foobar2 foobar3
    A old_file
    """)

        output, _ = self.repo.capture_stdio(self.reup.main)
        self.assertIn('branch_K up-to-date', output)
        self.assertIn('branch_L up-to-date', output)
        self.assertIn('foobar up-to-date', output)
        self.assertIn('sub_K up-to-date', output)

        with self.repo.open('bob') as f:
            self.assertEqual(b'testing auto-freeze/thaw', f.read())

        self.assertEqual(
            self.repo.git('status', '--porcelain').stdout, '?? bob\n')

        self.repo.git('checkout', 'origin/main')
        _, err = self.repo.capture_stdio(self.rp.main, [])
        self.assertIn('Must specify new parent somehow', err)
        _, err = self.repo.capture_stdio(self.rp.main, ['foobar'])
        self.assertIn('Must be on the branch', err)

        self.repo.git('checkout', 'branch_K')
        _, err = self.repo.capture_stdio(self.rp.main, ['origin/main'])
        self.assertIn('Cannot reparent a branch to its existing parent', err)
        output, _ = self.repo.capture_stdio(self.rp.main, ['foobar'])
        self.assertIn('Rebasing: branch_K', output)
        self.assertIn('Rebasing: sub_K', output)
        self.assertIn('Rebasing: branch_L', output)

        self.assertSchema("""
    A B C D E F G M N O foobar1 foobar2 H I J K L
                                foobar2 foobar3
                                              K sub_K
    A old_file
    """)

        self.repo.git('checkout', 'sub_K')
        output, _ = self.repo.capture_stdio(self.rp.main, ['foobar'])
        self.assertIn('try squashing your branch first', output)

        self.assertTrue(self.repo.run(self.gc.in_rebase))

        self.repo.git('rebase', '--abort')
        self.assertIsNone(self.repo.run(self.gc.thaw))

        self.assertSchema("""
    A B C D E F G M N O foobar1 foobar2 H I J K L
                                foobar2 foobar3
    A old_file
                                              K sub_K
    """)

        self.assertEqual(
            self.repo.git('status', '--porcelain').stdout, '?? bob\n')

        branches = self.repo.run(set, self.gc.branches())
        self.assertEqual(
            branches, {
                'branch_K', 'main', 'sub_K', 'root_A', 'branch_L', 'old_branch',
                'foobar', 'sub_foobar'
            })

        self.repo.git('checkout', 'branch_K')
        self.repo.run(self.mv.main, ['special_K'])

        branches = self.repo.run(set, self.gc.branches())
        self.assertEqual(
            branches, {
                'special_K', 'main', 'sub_K', 'root_A', 'branch_L',
                'old_branch', 'foobar', 'sub_foobar'
            })

        self.repo.git('checkout', 'origin/main')
        _, err = self.repo.capture_stdio(self.mv.main,
                                         ['special_K', 'cool branch'])
        self.assertIn('fatal: \'cool branch\' is not a valid branch name', err)

        self.repo.run(self.mv.main, ['special_K', 'cool_branch'])
        branches = self.repo.run(set, self.gc.branches())
        # This check fails with git 2.4 (see crbug.com/487172)
        self.assertEqual(
            branches, {
                'cool_branch', 'main', 'sub_K', 'root_A', 'branch_L',
                'old_branch', 'foobar', 'sub_foobar'
            })

        _, branch_tree = self.repo.run(self.gc.get_branch_tree)
        self.assertEqual(branch_tree['sub_K'], 'foobar')

    def testRebaseConflicts(self):
        # Pretend that branch_L landed
        self.origin.git('checkout', 'main')
        with self.origin.open('L', 'w') as f:
            f.write('L')
        self.origin.git('add', 'L')
        self.origin.git_commit('L')

        # Add a commit to branch_K so that things fail
        self.repo.git('checkout', 'branch_K')
        with self.repo.open('M', 'w') as f:
            f.write('NOPE')
        self.repo.git('add', 'M')
        self.repo.git_commit('K NOPE')

        # Add a commits to branch_L which would work if squashed
        self.repo.git('checkout', 'branch_L')
        self.repo.git('reset', 'branch_L~')
        with self.repo.open('L', 'w') as f:
            f.write('NOPE')
        self.repo.git('add', 'L')
        self.repo.git_commit('L NOPE')
        with self.repo.open('L', 'w') as f:
            f.write('L')
        self.repo.git('add', 'L')
        self.repo.git_commit('L YUP')

        # start on a branch which will be deleted
        self.repo.git('checkout', 'branch_G')

        output, _ = self.repo.capture_stdio(self.reup.main)
        self.assertIn('branch.branch_K.dormant true', output)

        output, _ = self.repo.capture_stdio(self.reup.main)
        self.assertIn('Rebase in progress', output)

        self.repo.git('checkout', '--theirs', 'M')
        self.repo.git('rebase', '--skip')

        output, _ = self.repo.capture_stdio(self.reup.main)
        self.assertIn('try squashing your branch first', output)

        # manually squash the branch
        self.repo.git('rebase', '--abort')
        self.repo.git('squash-branch',)

        # Try the rebase again
        self.repo.git('rebase', '--skip')

        output, _ = self.repo.capture_stdio(self.reup.main)
        self.assertIn('Deleted branch branch_G', output)
        self.assertIn('Deleted branch branch_L', output)
        self.assertIn('\'branch_G\' was merged', output)
        self.assertIn('checking out \'origin/main\'', output)

    def testRebaseConflictsKeepGoing(self):
        # Pretend that branch_L landed
        self.origin.git('checkout', 'main')
        with self.origin.open('L', 'w') as f:
            f.write('L')
        self.origin.git('add', 'L')
        self.origin.git_commit('L')

        # Add a commit to branch_K so that things fail
        self.repo.git('checkout', 'branch_K')
        with self.repo.open('M', 'w') as f:
            f.write('NOPE')
        self.repo.git('add', 'M')
        self.repo.git_commit('K NOPE')

        # Add a commits to branch_L which will work when squashed
        self.repo.git('checkout', 'branch_L')
        self.repo.git('reset', 'branch_L~')
        with self.repo.open('L', 'w') as f:
            f.write('NOPE')
        self.repo.git('add', 'L')
        self.repo.git_commit('L NOPE')
        with self.repo.open('L', 'w') as f:
            f.write('L')
        self.repo.git('add', 'L')
        self.repo.git_commit('L YUP')

        # start on a branch which will be deleted
        self.repo.git('checkout', 'branch_G')

        self.repo.git('config', 'branch.branch_K.dormant', 'false')
        output, _ = self.repo.capture_stdio(self.reup.main, ['-k'])
        self.assertIn('--keep-going set, continuing with next branch.', output)
        self.assertIn('could not be cleanly rebased:', output)
        self.assertIn('  branch_K', output)

    def testTrackTag(self):
        self.origin.git('tag', 'tag-to-track', self.origin['M'])
        self.repo.git('tag', 'tag-to-track', self.repo['D'])

        self.repo.git('config', 'branch.branch_G.remote', '.')
        self.repo.git('config', 'branch.branch_G.merge',
                      'refs/tags/tag-to-track')

        self.assertIn(
            'fatal: \'foo bar\' is not a valid branch name',
            self.repo.capture_stdio(
                self.nb.main,
                ['--upstream', 'tags/tag-to-track', 'foo bar'])[1])

        self.repo.run(self.nb.main,
                      ['--upstream', 'tags/tag-to-track', 'foobar'])

        with self.repo.open('foobar', 'w') as f:
            f.write('this is the foobar file')
        self.repo.git('add', 'foobar')
        self.repo.git_commit('foobar1')

        with self.repo.open('foobar', 'w') as f:
            f.write('totes the Foobar file')
        self.repo.git_commit('foobar2')

        self.assertSchema("""
    A B H I J K
            J L
      B C D E F G
          D foobar1 foobar2
    """)
        self.assertEqual(self.repo['A'], self.origin['A'])
        self.assertEqual(self.repo['G'], self.origin['G'])

        output, _ = self.repo.capture_stdio(self.reup.main)
        self.assertIn('Rebasing: branch_G', output)
        self.assertIn('Rebasing: branch_K', output)
        self.assertIn('Rebasing: branch_L', output)
        self.assertIn('Rebasing: foobar', output)
        self.assertEqual(
            self.repo.git('rev-parse', 'tags/tag-to-track').stdout.strip(),
            self.origin['M'])

        self.assertSchema("""
    A B C D E F G M N O
                  M H I J K L
                  M foobar1 foobar2
    """)

        _, err = self.repo.capture_stdio(self.rp.main, ['tag F'])
        self.assertIn('fatal: invalid reference', err)

        output, _ = self.repo.capture_stdio(self.rp.main, ['tag_F'])
        self.assertIn('to track tag_F [tag] (was tag-to-track [tag])', output)

        self.assertSchema("""
    A B C D E F G M N O
                  M H I J K L
              F foobar1 foobar2
    """)

        output, _ = self.repo.capture_stdio(self.rp.main, ['tag-to-track'])
        self.assertIn('to track tag-to-track [tag] (was tag_F [tag])', output)

        self.assertSchema("""
    A B C D E F G M N O
                  M H I J K L
                  M foobar1 foobar2
    """)

        output, _ = self.repo.capture_stdio(self.rp.main, ['--root'])
        self.assertIn('to track origin/main (was tag-to-track [tag])', output)

        self.assertSchema("""
    A B C D E F G M N O foobar1 foobar2
                  M H I J K L
    """)

    def testReparentBranchWithoutUpstream(self):
        self.repo.git('branch', 'nerp')
        self.repo.git('checkout', 'nerp')

        _, err = self.repo.capture_stdio(self.rp.main, ['branch_K'])

        self.assertIn('Unable to determine nerp@{upstream}', err)


if __name__ == '__main__':
    sys.exit(
        coverage_utils.covered_main(
            (os.path.join(DEPOT_TOOLS_ROOT, 'git_rebase_update.py'),
             os.path.join(DEPOT_TOOLS_ROOT, 'git_new_branch.py'),
             os.path.join(DEPOT_TOOLS_ROOT, 'git_reparent_branch.py'),
             os.path.join(DEPOT_TOOLS_ROOT, 'git_rename_branch.py'))))
