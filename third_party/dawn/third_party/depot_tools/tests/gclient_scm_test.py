#!/usr/bin/env vpython3
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for gclient_scm.py."""

# pylint: disable=E1103

from io import StringIO
import json
import logging
import os
import re
from subprocess import Popen, PIPE, STDOUT
import sys
import tempfile
import unittest
from unittest import mock

import scm_mock

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import scm

import gclient_scm
import gclient_utils
import git_cache
import git_common
import subprocess2
from testing_support import fake_repos
from testing_support import test_case_utils

# TODO: Should fix these warnings.
# pylint: disable=line-too-long

GIT = 'git' if sys.platform != 'win32' else 'git.bat'

# Disable global git cache
git_cache.Mirror.SetCachePath(None)

# Shortcut since this function is used often
join = gclient_scm.os.path.join

TIMESTAMP_RE = re.compile(r'\[[0-9]{1,2}:[0-9]{2}:[0-9]{2}\] (.*)', re.DOTALL)


def strip_timestamps(value):
    lines = value.splitlines(True)
    for i in range(len(lines)):
        m = TIMESTAMP_RE.match(lines[i])
        if m:
            lines[i] = m.group(1)
    return ''.join(lines)


class BasicTests(unittest.TestCase):

    def setUp(self) -> None:
        scm_mock.GIT(self)
        return super().setUp()

    def testGetFirstRemoteUrl(self):
        FAKE_PATH = '/fake/path'
        scm.GIT.SetConfig(FAKE_PATH,
                          'remote.origin.url',
                          'first-value',
                          append=True)
        scm.GIT.SetConfig(FAKE_PATH,
                          'remote.origin.url',
                          'second-value',
                          append=True)

        self.assertEqual(
            gclient_scm.SCMWrapper._get_first_remote_url(FAKE_PATH),
            'first-value')


class BaseGitWrapperTestCase(unittest.TestCase, test_case_utils.TestCaseUtils):
    """This class doesn't use pymox."""
    class OptionsObject(object):
        def __init__(self, verbose=False, revision=None):
            self.auto_rebase = False
            self.verbose = verbose
            self.revision = revision
            self.deps_os = None
            self.force = False
            self.reset = False
            self.nohooks = False
            self.no_history = False
            self.upstream = False
            self.cache_dir = None
            self.merge = False
            self.jobs = 1
            self.break_repo_locks = False
            self.delete_unversioned_trees = False
            self.patch_ref = None
            self.patch_repo = None
            self.rebase_patch_ref = True
            self.reset_patch_ref = True

    sample_git_import = """blob
mark :1
data 6
Hello

blob
mark :2
data 4
Bye

reset refs/heads/main
commit refs/heads/main
mark :3
author Bob <bob@example.com> 1253744361 -0700
committer Bob <bob@example.com> 1253744361 -0700
data 8
A and B
M 100644 :1 a
M 100644 :2 b

blob
mark :4
data 10
Hello
You

blob
mark :5
data 8
Bye
You

commit refs/heads/origin
mark :6
author Alice <alice@example.com> 1253744424 -0700
committer Alice <alice@example.com> 1253744424 -0700
data 13
Personalized
from :3
M 100644 :4 a
M 100644 :5 b
M 160000 1111111111111111111111111111111111111111 submodule

blob
mark :7
data 5
Mooh

commit refs/heads/feature
mark :8
author Bob <bob@example.com> 1390311986 -0000
committer Bob <bob@example.com> 1390311986 -0000
data 6
Add C
from :3
M 100644 :7 c

reset refs/heads/main
from :3
"""

    def Options(self, *args, **kwargs):
        return self.OptionsObject(*args, **kwargs)

    def checkstdout(self, expected):
        # pylint: disable=no-member
        value = sys.stdout.getvalue()
        sys.stdout.close()
        # Check that the expected output appears.
        self.assertIn(expected, strip_timestamps(value))

    @staticmethod
    def CreateGitRepo(git_import, path):
        """Do it for real."""
        try:
            Popen([GIT, 'init', '-q'], stdout=PIPE, stderr=STDOUT,
                  cwd=path).communicate()
        except OSError:
            # git is not available, skip this test.
            return False
        Popen([GIT, 'fast-import', '--quiet'],
              stdin=PIPE,
              stdout=PIPE,
              stderr=STDOUT,
              cwd=path).communicate(input=git_import.encode())
        Popen([GIT, 'checkout', '-q'], stdout=PIPE, stderr=STDOUT,
              cwd=path).communicate()
        Popen([GIT, 'remote', 'add', '-f', 'origin', '.'],
              stdout=PIPE,
              stderr=STDOUT,
              cwd=path).communicate()
        Popen([GIT, 'checkout', '-b', 'new', 'origin/main', '-q'],
              stdout=PIPE,
              stderr=STDOUT,
              cwd=path).communicate()
        Popen([GIT, 'push', 'origin', 'origin/origin:origin/main', '-q'],
              stdout=PIPE,
              stderr=STDOUT,
              cwd=path).communicate()
        Popen([GIT, 'config', '--unset', 'remote.origin.fetch'],
              stdout=PIPE,
              stderr=STDOUT,
              cwd=path).communicate()
        Popen([GIT, 'config', 'user.email', 'someuser@chromium.org'],
              stdout=PIPE,
              stderr=STDOUT,
              cwd=path).communicate()
        Popen([GIT, 'config', 'user.name', 'Some User'],
              stdout=PIPE,
              stderr=STDOUT,
              cwd=path).communicate()
        # Set HEAD back to main
        Popen([GIT, 'checkout', 'main', '-q'],
              stdout=PIPE,
              stderr=STDOUT,
              cwd=path).communicate()
        return True

    def _GetAskForDataCallback(self, expected_prompt, return_value):
        def AskForData(prompt, options):
            self.assertEqual(prompt, expected_prompt)
            return return_value

        return AskForData

    def setUp(self):
        unittest.TestCase.setUp(self)
        test_case_utils.TestCaseUtils.setUp(self)
        self.url = 'git://foo'
        # The .git suffix allows gclient_scm to recognize the dir as a git repo
        # when cloning it locally
        self.root_dir = tempfile.mkdtemp('.git')
        self.relpath = '.'
        self.base_path = join(self.root_dir, self.relpath)
        self.enabled = self.CreateGitRepo(self.sample_git_import,
                                          self.base_path)
        mock.patch('sys.stdout', StringIO()).start()
        self.addCleanup(mock.patch.stopall)
        self.addCleanup(gclient_utils.rmtree, self.root_dir)


class ManagedGitWrapperTestCase(BaseGitWrapperTestCase):
    def testRevertMissing(self):
        if not self.enabled:
            return
        options = self.Options()
        file_path = join(self.base_path, 'a')
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        file_list = []
        git_wrapper.update(options, None, file_list)
        gclient_scm.os.remove(file_path)
        file_list = []
        git_wrapper.revert(options, self.args, file_list)
        self.assertEqual(file_list, [file_path])
        file_list = []
        git_wrapper.diff(options, self.args, file_list)
        self.assertEqual(file_list, [])
        sys.stdout.close()

    def testRevertNone(self):
        if not self.enabled:
            return
        options = self.Options()
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        file_list = []
        git_wrapper.update(options, None, file_list)
        file_list = []
        git_wrapper.revert(options, self.args, file_list)
        self.assertEqual(file_list, [])
        self.assertEqual(git_wrapper.revinfo(options, self.args, None),
                         '4091c7d010ca99d0f2dd416d4b70b758ae432187')
        sys.stdout.close()

    def testRevertModified(self):
        if not self.enabled:
            return
        options = self.Options()
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        file_list = []
        git_wrapper.update(options, None, file_list)
        file_path = join(self.base_path, 'a')
        with open(file_path, 'a') as f:
            f.writelines('touched\n')
        file_list = []
        git_wrapper.revert(options, self.args, file_list)
        self.assertEqual(file_list, [file_path])
        file_list = []
        git_wrapper.diff(options, self.args, file_list)
        self.assertEqual(file_list, [])
        self.assertEqual(git_wrapper.revinfo(options, self.args, None),
                         '4091c7d010ca99d0f2dd416d4b70b758ae432187')
        sys.stdout.close()

    def testRevertNew(self):
        if not self.enabled:
            return
        options = self.Options()
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        file_list = []
        git_wrapper.update(options, None, file_list)
        file_path = join(self.base_path, 'c')
        with open(file_path, 'w') as f:
            f.writelines('new\n')
        Popen([GIT, 'add', 'c'], stdout=PIPE, stderr=STDOUT,
              cwd=self.base_path).communicate()
        file_list = []
        git_wrapper.revert(options, self.args, file_list)
        self.assertEqual(file_list, [file_path])
        file_list = []
        git_wrapper.diff(options, self.args, file_list)
        self.assertEqual(file_list, [])
        self.assertEqual(git_wrapper.revinfo(options, self.args, None),
                         '4091c7d010ca99d0f2dd416d4b70b758ae432187')
        sys.stdout.close()

    def testStatusRef(self):
        if not self.enabled:
            return
        options = self.Options()
        file_paths = [join(self.base_path, 'a')]
        with open(file_paths[0], 'a') as f:
            f.writelines('touched\n')
        git_wrapper = gclient_scm.GitWrapper(self.url + '@refs/heads/feature',
                                             self.root_dir, self.relpath)
        file_paths.append(join(self.base_path, 'c'))  # feature branch touches c
        file_list = []
        git_wrapper.status(options, self.args, file_list)
        self.assertEqual(file_list, file_paths)
        self.checkstdout((
            '\n________ running \'git -c core.quotePath=false diff --name-status '
            'refs/remotes/origin/feature\' in \'%s\'\n\nM\ta\n') %
                         join(self.root_dir, '.'))

    def testStatusNew(self):
        if not self.enabled:
            return
        options = self.Options()
        file_path = join(self.base_path, 'a')
        with open(file_path, 'a') as f:
            f.writelines('touched\n')
        git_wrapper = gclient_scm.GitWrapper(
            self.url + '@069c602044c5388d2d15c3f875b057c852003458',
            self.root_dir, self.relpath)
        file_list = []
        git_wrapper.status(options, self.args, file_list)
        self.assertEqual(file_list, [file_path])
        self.checkstdout((
            '\n________ running \'git -c core.quotePath=false diff --name-status '
            '069c602044c5388d2d15c3f875b057c852003458\' in \'%s\'\n\nM\ta\n') %
                         join(self.root_dir, '.'))

    def testStatusNewNoBaseRev(self):
        if not self.enabled:
            return
        options = self.Options()
        file_path = join(self.base_path, 'a')
        with open(file_path, 'a') as f:
            f.writelines('touched\n')
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        file_list = []
        git_wrapper.status(options, self.args, file_list)
        self.assertEqual(file_list, [file_path])
        self.checkstdout((
            '\n________ running \'git -c core.quotePath=false diff --name-status'
            '\' in \'%s\'\n\nM\ta\n') % join(self.root_dir, '.'))

    def testStatus2New(self):
        if not self.enabled:
            return
        options = self.Options()
        expected_file_list = []
        for f in ['a', 'b']:
            file_path = join(self.base_path, f)
            with open(file_path, 'a') as f:
                f.writelines('touched\n')
            expected_file_list.extend([file_path])
        git_wrapper = gclient_scm.GitWrapper(
            self.url + '@069c602044c5388d2d15c3f875b057c852003458',
            self.root_dir, self.relpath)
        file_list = []
        git_wrapper.status(options, self.args, file_list)
        expected_file_list = [join(self.base_path, x) for x in ['a', 'b']]
        self.assertEqual(sorted(file_list), expected_file_list)
        self.checkstdout((
            '\n________ running \'git -c core.quotePath=false diff --name-status '
            '069c602044c5388d2d15c3f875b057c852003458\' in \'%s\'\n\nM\ta\nM\tb\n'
        ) % join(self.root_dir, '.'))

    def testUpdateUpdate(self):
        if not self.enabled:
            return
        options = self.Options()
        expected_file_list = [join(self.base_path, x) for x in ['a', 'b']]
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        file_list = []

        # Set diff.ignoreSubmodules to something other than 'dirty'
        git_wrapper._Run(['config', 'diff.ignoreSubmodules', 'all'], options)
        git_wrapper.update(options, (), file_list)
        expected_warning = "diff.ignoreSubmodules is not set to 'dirty'"
        self.assertTrue(
            any(expected_warning in w for w in gclient_utils._WARNINGS),
            f"Expected warning not found. "
            f"New warnings: {gclient_utils._WARNINGS}")
        self.assertEqual(file_list, expected_file_list)
        self.assertEqual(git_wrapper.revinfo(options, (), None),
                         '4091c7d010ca99d0f2dd416d4b70b758ae432187')
        self.assertEqual(
            git_wrapper._Capture(['config', '--get', 'diff.ignoreSubmodules']),
            'all')
        self.assertEqual(
            git_wrapper._Capture(['config', '--get',
                                  'fetch.recurseSubmodules']), 'off')
        self.assertEqual(
            git_wrapper._Capture(['config', '--get', 'push.recurseSubmodules']),
            'off')
        os.environ['GCLIENT_SUPPRESS_SUBMODULE_WARNING'] = '1'
        gclient_utils._WARNINGS.clear()
        git_wrapper.update(options, (), file_list)
        self.assertEqual(len(gclient_utils._WARNINGS), 0,
                         "Warning was added despite being suppressed")
        # Clean up
        del os.environ['GCLIENT_SUPPRESS_SUBMODULE_WARNING']
        sys.stdout.close()

    def testUpdateMerge(self):
        if not self.enabled:
            return
        options = self.Options()
        options.merge = True
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        # This sets correct remote HEAD
        git_wrapper.update(options, (), [])

        git_wrapper._Run(['checkout', '-q', 'feature'], options)
        rev = git_wrapper.revinfo(options, (), None)
        file_list = []
        git_wrapper.update(options, (), file_list)
        self.assertEqual(
            file_list,
            [join(self.base_path, x) for x in ['a', 'b', 'c', 'submodule']])
        # The actual commit that is created is unstable, so we verify its tree
        # and parents instead.
        self.assertEqual(git_wrapper._Capture(['rev-parse', 'HEAD:']),
                         '3a3ba72731fa208d37b06598a129ba93970325df')
        self.assertEqual(git_wrapper._Capture(['rev-parse', 'HEAD^1']), rev)
        self.assertEqual(git_wrapper._Capture(['rev-parse', 'HEAD^2']),
                         git_wrapper._Capture(['rev-parse', 'origin/main']))
        sys.stdout.close()

    def testUpdateRebase(self):
        if not self.enabled:
            return
        options = self.Options()
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        # This sets correct remote HEAD
        git_wrapper.update(options, (), [])

        git_wrapper._Run(['checkout', '-q', 'feature'], options)
        # Fake a 'y' key press.
        git_wrapper._AskForData = self._GetAskForDataCallback(
            'Cannot fast-forward merge, attempt to rebase? '
            '(y)es / (q)uit / (s)kip : ', 'y')
        file_list = []
        git_wrapper.update(options, (), file_list)
        self.assertEqual(
            file_list,
            [join(self.base_path, x) for x in ['a', 'b', 'c', 'submodule']])
        # The actual commit that is created is unstable, so we verify its tree
        # and parent instead.
        self.assertEqual(git_wrapper._Capture(['rev-parse', 'HEAD:']),
                         '3a3ba72731fa208d37b06598a129ba93970325df')
        self.assertEqual(git_wrapper._Capture(['rev-parse', 'HEAD^1']),
                         git_wrapper._Capture(['rev-parse', 'origin/main']))
        sys.stdout.close()

    def testUpdateReset(self):
        if not self.enabled:
            return
        options = self.Options()
        options.reset = True

        dir_path = join(self.base_path, 'c')
        os.mkdir(dir_path)
        with open(join(dir_path, 'nested'), 'w') as f:
            f.writelines('new\n')

        file_path = join(self.base_path, 'file')
        with open(file_path, 'w') as f:
            f.writelines('new\n')

        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        file_list = []
        git_wrapper.update(options, (), file_list)
        self.assert_(gclient_scm.os.path.isdir(dir_path))
        self.assert_(gclient_scm.os.path.isfile(file_path))
        sys.stdout.close()

    def testUpdateResetUnsetsFetchConfig(self):
        if not self.enabled:
            return
        options = self.Options()
        options.reset = True

        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        git_wrapper._Run([
            'config', 'remote.origin.fetch',
            '+refs/heads/bad/ref:refs/remotes/origin/bad/ref'
        ], options)

        file_list = []
        git_wrapper.update(options, (), file_list)
        self.assertEqual(git_wrapper.revinfo(options, (), None),
                         '069c602044c5388d2d15c3f875b057c852003458')
        sys.stdout.close()

    def testUpdateResetDeleteUnversionedTrees(self):
        if not self.enabled:
            return
        options = self.Options()
        options.reset = True
        options.delete_unversioned_trees = True

        dir_path = join(self.base_path, 'dir')
        os.mkdir(dir_path)
        with open(join(dir_path, 'nested'), 'w') as f:
            f.writelines('new\n')

        file_path = join(self.base_path, 'file')
        with open(file_path, 'w') as f:
            f.writelines('new\n')

        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        file_list = []
        git_wrapper.update(options, (), file_list)
        self.assert_(not gclient_scm.os.path.isdir(dir_path))
        self.assert_(gclient_scm.os.path.isfile(file_path))
        sys.stdout.close()

    def testUpdateUnstagedConflict(self):
        if not self.enabled:
            return
        options = self.Options()
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        file_path = join(self.base_path, 'b')
        with open(file_path, 'w') as f:
            f.writelines('conflict\n')
        try:
            git_wrapper.update(options, (), [])
            self.fail()
        except (gclient_scm.gclient_utils.Error,
                subprocess2.CalledProcessError):
            # The exact exception text varies across git versions so it's not
            # worth verifying it. It's fine as long as it throws.
            pass
        # Manually flush stdout since we can't verify it's content accurately
        # across git versions.
        sys.stdout.getvalue()
        sys.stdout.close()

    @unittest.skip('Skipping until crbug.com/670884 is resolved.')
    def testUpdateLocked(self):
        if not self.enabled:
            return
        options = self.Options()
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        file_path = join(self.base_path, '.git', 'index.lock')
        with open(file_path, 'w'):
            pass
        with self.assertRaises(subprocess2.CalledProcessError):
            git_wrapper.update(options, (), [])
        sys.stdout.close()

    def testUpdateLockedBreak(self):
        if not self.enabled:
            return
        options = self.Options()
        options.break_repo_locks = True
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        file_path = join(self.base_path, '.git', 'index.lock')
        with open(file_path, 'w'):
            pass
        git_wrapper.update(options, (), [])
        self.assertRegexpMatches(sys.stdout.getvalue(),
                                 r'breaking lock.*\.git[/|\\]index\.lock')
        self.assertFalse(os.path.exists(file_path))
        sys.stdout.close()

    def testUpdateConflict(self):
        if not self.enabled:
            return
        options = self.Options()
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        file_path = join(self.base_path, 'b')
        with open(file_path, 'w') as f:
            f.writelines('conflict\n')
        git_wrapper._Run(['commit', '-am', 'test'], options)
        git_wrapper._AskForData = self._GetAskForDataCallback(
            'Cannot fast-forward merge, attempt to rebase? '
            '(y)es / (q)uit / (s)kip : ', 'y')

        with self.assertRaises(gclient_scm.gclient_utils.Error) as e:
            git_wrapper.update(options, (), [])
        self.assertEqual(
            e.exception.args[0], 'Conflict while rebasing this branch.\n'
            'Fix the conflict and run gclient again.\n'
            'See \'man git-rebase\' for details.\n')

        with self.assertRaises(gclient_scm.gclient_utils.Error) as e:
            git_wrapper.update(options, (), [])
        self.assertEqual(
            e.exception.args[0], '\n____ . at refs/remotes/origin/main\n'
            '\tYou have uncommitted changes.\n'
            '\tcd into ., run git status to see changes,\n'
            '\tand commit, stash, or reset.\n')

        sys.stdout.close()

    def testRevinfo(self):
        if not self.enabled:
            return
        options = self.Options()
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        rev_info = git_wrapper.revinfo(options, (), None)
        self.assertEqual(rev_info, '069c602044c5388d2d15c3f875b057c852003458')


class ManagedGitWrapperTestCaseMock(unittest.TestCase):
    class OptionsObject(object):
        def __init__(self, verbose=False, revision=None, force=False):
            self.verbose = verbose
            self.revision = revision
            self.deps_os = None
            self.force = force
            self.reset = False
            self.nohooks = False
            self.break_repo_locks = False
            # TODO(maruel): Test --jobs > 1.
            self.jobs = 1
            self.patch_ref = None
            self.patch_repo = None
            self.rebase_patch_ref = True

    def Options(self, *args, **kwargs):
        return self.OptionsObject(*args, **kwargs)

    def checkstdout(self, expected):
        # pylint: disable=no-member
        value = sys.stdout.getvalue()
        sys.stdout.close()
        # Check that the expected output appears.
        self.assertIn(expected, strip_timestamps(value))

    def setUp(self):
        self.fake_hash_1 = 't0ta11yf4k3'
        self.fake_hash_2 = '3v3nf4k3r'
        self.url = 'git://foo'
        self.root_dir = '/tmp' if sys.platform != 'win32' else 't:\\tmp'
        self.relpath = 'fake'
        self.base_path = os.path.join(self.root_dir, self.relpath)
        self.backup_base_path = os.path.join(self.root_dir,
                                             'old_%s.git' % self.relpath)
        mock.patch('gclient_scm.scm.GIT.ApplyEnvVars').start()
        mock.patch('gclient_scm.GitWrapper._Fetch').start()
        mock.patch('gclient_scm.GitWrapper._DeleteOrMove').start()
        mock.patch('sys.stdout', StringIO()).start()
        self.addCleanup(mock.patch.stopall)

    @mock.patch('scm.GIT.IsValidRevision')
    @mock.patch('os.path.isdir', lambda _: True)
    def testGetUsableRevGit(self, mockIsValidRevision):
        # pylint: disable=no-member
        options = self.Options(verbose=True)

        mockIsValidRevision.side_effect = lambda cwd, rev: rev != '1'

        git_scm = gclient_scm.GitWrapper(self.url, self.root_dir, self.relpath)
        # A [fake] git sha1 with a git repo should work (this is in the case
        # that the LKGR gets flipped to git sha1's some day).
        self.assertEqual(git_scm.GetUsableRev(self.fake_hash_1, options),
                         self.fake_hash_1)
        # An SVN rev with an existing purely git repo should raise an exception.
        self.assertRaises(gclient_scm.gclient_utils.Error, git_scm.GetUsableRev,
                          '1', options)

    @mock.patch('gclient_scm.GitWrapper._Clone')
    @mock.patch('os.path.isdir')
    @mock.patch('os.path.exists')
    @mock.patch('git_common.run')
    def testUpdateNoDotGit(self, mockRun, mockExists, mockIsdir, mockClone):
        mockIsdir.side_effect = lambda path: path == self.base_path
        mockExists.side_effect = lambda path: path == self.base_path
        mockRun.side_effect = ['refs/remotes/origin/main', '', '']

        options = self.Options()
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        git_wrapper.update(options, None, [])

        env = gclient_scm.scm.GIT.ApplyEnvVars({})
        self.assertEqual(mockRun.mock_calls, [
            mock.call('symbolic-ref',
                      'refs/remotes/origin/HEAD',
                      autostrip=True,
                      cwd=self.base_path,
                      env=env),
            mock.call('-c',
                      'core.quotePath=false',
                      'ls-files',
                      cwd=self.base_path,
                      env=env,
                      stderr=-1),
            mock.call('rev-parse',
                      '--verify',
                      'HEAD',
                      cwd=self.base_path,
                      env=env,
                      stderr=-1),
        ])
        mockClone.assert_called_with('refs/remotes/origin/main', self.url,
                                     options)
        self.checkstdout('\n')

    @mock.patch('gclient_scm.GitWrapper._Clone')
    @mock.patch('os.path.isdir')
    @mock.patch('os.path.exists')
    @mock.patch('git_common.run')
    def testUpdateConflict(self, mockRun, mockExists, mockIsdir, mockClone):
        mockIsdir.side_effect = lambda path: path == self.base_path
        mockExists.side_effect = lambda path: path == self.base_path
        mockRun.side_effect = ['refs/remotes/origin/main', '', '']
        mockClone.side_effect = [
            git_common.subprocess2.CalledProcessError(None, None, None, None,
                                                      None),
            None,
        ]

        options = self.Options()
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        git_wrapper.update(options, None, [])

        env = gclient_scm.scm.GIT.ApplyEnvVars({})
        self.assertEqual(mockRun.mock_calls, [
            mock.call('symbolic-ref',
                      'refs/remotes/origin/HEAD',
                      autostrip=True,
                      cwd=self.base_path,
                      env=env),
            mock.call('-c',
                      'core.quotePath=false',
                      'ls-files',
                      cwd=self.base_path,
                      env=env,
                      stderr=-1),
            mock.call('rev-parse',
                      '--verify',
                      'HEAD',
                      cwd=self.base_path,
                      env=env,
                      stderr=-1),
        ])
        mockClone.assert_called_with('refs/remotes/origin/main', self.url,
                                     options)
        self.checkstdout('\n')


class UnmanagedGitWrapperTestCase(BaseGitWrapperTestCase):
    def checkInStdout(self, expected):
        # pylint: disable=no-member
        value = sys.stdout.getvalue()
        sys.stdout.close()
        self.assertIn(expected, value)

    def checkNotInStdout(self, expected):
        # pylint: disable=no-member
        value = sys.stdout.getvalue()
        sys.stdout.close()
        self.assertNotIn(expected, value)

    def getCurrentBranch(self):
        # Returns name of current branch or HEAD for detached HEAD
        branch = gclient_scm.scm.GIT.Capture(
            ['rev-parse', '--abbrev-ref', 'HEAD'], cwd=self.base_path)
        if branch == 'HEAD':
            return None
        return branch

    def testUpdateClone(self):
        if not self.enabled:
            return
        options = self.Options()

        origin_root_dir = self.root_dir
        self.addCleanup(gclient_utils.rmtree, origin_root_dir)

        self.root_dir = tempfile.mkdtemp()
        self.relpath = '.'
        self.base_path = join(self.root_dir, self.relpath)

        git_wrapper = gclient_scm.GitWrapper(origin_root_dir, self.root_dir,
                                             self.relpath)

        expected_file_list = [
            join(self.base_path, "a"),
            join(self.base_path, "b")
        ]
        file_list = []
        options.revision = 'unmanaged'
        git_wrapper.update(options, (), file_list)

        self.assertEqual(file_list, expected_file_list)
        self.assertEqual(git_wrapper.revinfo(options, (), None),
                         '069c602044c5388d2d15c3f875b057c852003458')
        # indicates detached HEAD
        self.assertEqual(self.getCurrentBranch(), None)
        self.checkInStdout(
            'Checked out refs/remotes/origin/main to a detached HEAD')

    def testUpdateCloneOnCommit(self):
        if not self.enabled:
            return
        options = self.Options()

        origin_root_dir = self.root_dir
        self.addCleanup(gclient_utils.rmtree, origin_root_dir)

        self.root_dir = tempfile.mkdtemp()
        self.relpath = '.'
        self.base_path = join(self.root_dir, self.relpath)
        url_with_commit_ref = origin_root_dir +\
                              '@4091c7d010ca99d0f2dd416d4b70b758ae432187'

        git_wrapper = gclient_scm.GitWrapper(url_with_commit_ref, self.root_dir,
                                             self.relpath)

        expected_file_list = [
            join(self.base_path, "a"),
            join(self.base_path, "b"),
            join(self.base_path, "submodule"),
        ]
        file_list = []
        options.revision = 'unmanaged'
        git_wrapper.update(options, (), file_list)

        self.assertEqual(file_list, expected_file_list)
        self.assertEqual(git_wrapper.revinfo(options, (), None),
                         '4091c7d010ca99d0f2dd416d4b70b758ae432187')
        # indicates detached HEAD
        self.assertEqual(self.getCurrentBranch(), None)
        self.checkInStdout(
            'Checked out 4091c7d010ca99d0f2dd416d4b70b758ae432187 to a detached HEAD'
        )

    def testUpdateCloneOnBranch(self):
        if not self.enabled:
            return
        options = self.Options()

        origin_root_dir = self.root_dir
        self.addCleanup(gclient_utils.rmtree, origin_root_dir)

        self.root_dir = tempfile.mkdtemp()
        self.relpath = '.'
        self.base_path = join(self.root_dir, self.relpath)
        url_with_branch_ref = origin_root_dir + '@feature'

        git_wrapper = gclient_scm.GitWrapper(url_with_branch_ref, self.root_dir,
                                             self.relpath)

        expected_file_list = [
            join(self.base_path, "a"),
            join(self.base_path, "b"),
            join(self.base_path, "c")
        ]
        file_list = []
        options.revision = 'unmanaged'
        git_wrapper.update(options, (), file_list)

        self.assertEqual(file_list, expected_file_list)
        self.assertEqual(git_wrapper.revinfo(options, (), None),
                         '9a51244740b25fa2ded5252ca00a3178d3f665a9')
        # indicates detached HEAD
        self.assertEqual(self.getCurrentBranch(), None)
        self.checkInStdout(
            'Checked out 9a51244740b25fa2ded5252ca00a3178d3f665a9 '
            'to a detached HEAD')

    def testUpdateCloneOnFetchedRemoteBranch(self):
        if not self.enabled:
            return
        options = self.Options()

        origin_root_dir = self.root_dir
        self.addCleanup(gclient_utils.rmtree, origin_root_dir)

        self.root_dir = tempfile.mkdtemp()
        self.relpath = '.'
        self.base_path = join(self.root_dir, self.relpath)
        url_with_branch_ref = origin_root_dir + '@refs/remotes/origin/feature'

        git_wrapper = gclient_scm.GitWrapper(url_with_branch_ref, self.root_dir,
                                             self.relpath)

        expected_file_list = [
            join(self.base_path, "a"),
            join(self.base_path, "b"),
            join(self.base_path, "c")
        ]
        file_list = []
        options.revision = 'unmanaged'
        git_wrapper.update(options, (), file_list)

        self.assertEqual(file_list, expected_file_list)
        self.assertEqual(git_wrapper.revinfo(options, (), None),
                         '9a51244740b25fa2ded5252ca00a3178d3f665a9')
        # indicates detached HEAD
        self.assertEqual(self.getCurrentBranch(), None)
        self.checkInStdout(
            'Checked out refs/remotes/origin/feature to a detached HEAD')

    def testUpdateCloneOnTrueRemoteBranch(self):
        if not self.enabled:
            return
        options = self.Options()

        origin_root_dir = self.root_dir
        self.addCleanup(gclient_utils.rmtree, origin_root_dir)

        self.root_dir = tempfile.mkdtemp()
        self.relpath = '.'
        self.base_path = join(self.root_dir, self.relpath)
        url_with_branch_ref = origin_root_dir + '@refs/heads/feature'

        git_wrapper = gclient_scm.GitWrapper(url_with_branch_ref, self.root_dir,
                                             self.relpath)

        expected_file_list = [
            join(self.base_path, "a"),
            join(self.base_path, "b"),
            join(self.base_path, "c")
        ]
        file_list = []
        options.revision = 'unmanaged'
        git_wrapper.update(options, (), file_list)

        self.assertEqual(file_list, expected_file_list)
        self.assertEqual(git_wrapper.revinfo(options, (), None),
                         '9a51244740b25fa2ded5252ca00a3178d3f665a9')
        # @refs/heads/feature is AKA @refs/remotes/origin/feature in the clone,
        # so should be treated as such by gclient. TODO(mmoss): Though really,
        # we should only allow DEPS to specify branches as they are known in the
        # upstream repo, since the mapping into the local repo can be modified
        # by users (or we might even want to change the gclient defaults at some
        # point). But that will take more work to stop using refs/remotes/
        # everywhere that we do (and to stop assuming a DEPS ref will always
        # resolve locally, like when passing them to show-ref or rev-list).
        self.assertEqual(self.getCurrentBranch(), None)
        self.checkInStdout(
            'Checked out refs/remotes/origin/feature to a detached HEAD')

    def testUpdateUpdate(self):
        if not self.enabled:
            return
        options = self.Options()
        expected_file_list = []
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        file_list = []
        options.revision = 'unmanaged'
        git_wrapper.update(options, (), file_list)
        self.assertEqual(file_list, expected_file_list)
        self.assertEqual(git_wrapper.revinfo(options, (), None),
                         '069c602044c5388d2d15c3f875b057c852003458')
        self.checkstdout('________ unmanaged solution; skipping .\n')


class CipdWrapperTestCase(unittest.TestCase):
    def setUp(self):
        # Create this before setting up mocks.
        self._cipd_root_dir = tempfile.mkdtemp()
        self._workdir = tempfile.mkdtemp()

        self._cipd_instance_url = 'https://chrome-infra-packages.appspot.com'
        self._cipd_root = gclient_scm.CipdRoot(self._cipd_root_dir,
                                               self._cipd_instance_url)
        self._cipd_packages = [
            self._cipd_root.add_package('f', 'foo_package', 'foo_version'),
            self._cipd_root.add_package('b', 'bar_package', 'bar_version'),
            self._cipd_root.add_package('b', 'baz_package', 'baz_version'),
        ]
        mock.patch('tempfile.mkdtemp', lambda: self._workdir).start()
        mock.patch('gclient_scm.CipdRoot.add_package').start()
        mock.patch('gclient_scm.CipdRoot.clobber').start()
        mock.patch('gclient_scm.CipdRoot.ensure_file_resolve').start()
        mock.patch('gclient_scm.CipdRoot.ensure').start()
        self.addCleanup(mock.patch.stopall)
        self.addCleanup(gclient_utils.rmtree, self._cipd_root_dir)
        self.addCleanup(gclient_utils.rmtree, self._workdir)

    def createScmWithPackageThatSatisfies(self, condition):
        return gclient_scm.CipdWrapper(
            url=self._cipd_instance_url,
            root_dir=self._cipd_root_dir,
            relpath='fake_relpath',
            root=self._cipd_root,
            package=self.getPackageThatSatisfies(condition))

    def getPackageThatSatisfies(self, condition):
        for p in self._cipd_packages:
            if condition(p):
                return p

        self.fail('Unable to find a satisfactory package.')

    def testRevert(self):
        """Checks that revert does nothing."""
        git_wrapper = self.createScmWithPackageThatSatisfies(lambda _: True)
        git_wrapper.revert(None, (), [])

    @mock.patch('gclient_scm.gclient_utils.CheckCallAndFilter')
    @mock.patch('gclient_scm.gclient_utils.rmtree')
    def testRevinfo(self, mockRmtree, mockCheckCallAndFilter):
        """Checks that revinfo uses the JSON from cipd describe."""
        git_wrapper = self.createScmWithPackageThatSatisfies(lambda _: True)

        expected_revinfo = '0123456789abcdef0123456789abcdef01234567'
        json_contents = {
            'result': {
                'pin': {
                    'instance_id': expected_revinfo,
                }
            }
        }
        describe_json_path = join(self._workdir, 'describe.json')
        with open(describe_json_path, 'w') as describe_json:
            json.dump(json_contents, describe_json)

        revinfo = git_wrapper.revinfo(None, (), [])
        self.assertEqual(revinfo, expected_revinfo)

        mockRmtree.assert_called_with(self._workdir)
        mockCheckCallAndFilter.assert_called_with([
            'cipd',
            'describe',
            'foo_package',
            '-log-level',
            'error',
            '-version',
            'foo_version',
            '-json-output',
            describe_json_path,
        ])

    def testUpdate(self):
        """Checks that update does nothing."""
        git_wrapper = self.createScmWithPackageThatSatisfies(lambda _: True)
        git_wrapper.update(None, (), [])


class GcsWrapperTestCase(unittest.TestCase):

    def setUp(self):
        self.workdir = tempfile.mkdtemp()
        self.url = 'gs://123bucket/path_to_tar.gz'

    def createScm(self):
        return gclient_scm.GcsWrapper(url=self.url,
                                      root_dir=self.workdir,
                                      relpath='fake_relpath')

    def testRevert(self):
        """Checks that revert does nothing."""
        git_wrapper = self.createScm()
        git_wrapper.revert(None, (), [])

    def testRevinfo(self):
        """Checks that revinfo does nothing."""
        git_wrapper = self.createScm()
        git_wrapper.revinfo(None, (), [])

    def testUpdate(self):
        """Checks that update does nothing."""
        git_wrapper = self.createScm()
        git_wrapper.update(None, (), [])


class BranchHeadsFakeRepo(fake_repos.FakeReposBase):
    def populateGit(self):
        # Creates a tree that looks like this:
        #
        #    5 refs/branch-heads/5
        #    |
        #    4
        #    |
        # 1--2--3 refs/heads/main
        self._commit_git('repo_1', {'commit 1': 'touched'})
        self._commit_git('repo_1', {'commit 2': 'touched'})
        self._commit_git('repo_1', {'commit 3': 'touched'})
        self._create_ref('repo_1', 'refs/heads/main', 3)

        self._commit_git('repo_1', {'commit 4': 'touched'}, base=2)
        self._commit_git('repo_1', {'commit 5': 'touched'}, base=2)
        self._create_ref('repo_1', 'refs/branch-heads/5', 5)


class BranchHeadsTest(fake_repos.FakeReposTestBase):
    FAKE_REPOS_CLASS = BranchHeadsFakeRepo

    def setUp(self):
        super(BranchHeadsTest, self).setUp()
        self.enabled = self.FAKE_REPOS.set_up_git()
        self.options = BaseGitWrapperTestCase.OptionsObject()
        self.url = self.git_base + 'repo_1'
        self.mirror = None
        mock.patch('sys.stdout', StringIO()).start()
        self.addCleanup(mock.patch.stopall)

    def setUpMirror(self):
        self.mirror = tempfile.mkdtemp('mirror')
        git_cache.Mirror.SetCachePath(self.mirror)
        self.addCleanup(gclient_utils.rmtree, self.mirror)
        self.addCleanup(git_cache.Mirror.SetCachePath, None)

    def testCheckoutBranchHeads(self):
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        file_list = []

        self.options.revision = 'refs/branch-heads/5'
        git_wrapper.update(self.options, None, file_list)
        self.assertEqual(self.githash('repo_1', 5),
                         self.gitrevparse(self.root_dir))

    def testCheckoutUpdatedBranchHeads(self):
        # Travel back in time, and set refs/branch-heads/5 to its parent.
        subprocess2.check_call([
            'git', 'update-ref', 'refs/branch-heads/5',
            self.githash('repo_1', 4)
        ],
                               cwd=self.url)

        # Sync to refs/branch-heads/5
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        self.options.revision = 'refs/branch-heads/5'
        git_wrapper.update(self.options, None, [])

        # Set refs/branch-heads/5 back to its original value.
        subprocess2.check_call([
            'git', 'update-ref', 'refs/branch-heads/5',
            self.githash('repo_1', 5)
        ],
                               cwd=self.url)

        # Attempt to sync to refs/branch-heads/5 again.
        self.testCheckoutBranchHeads()

    def testCheckoutBranchHeadsMirror(self):
        self.setUpMirror()
        self.testCheckoutBranchHeads()

    def testCheckoutUpdatedBranchHeadsMirror(self):
        self.setUpMirror()
        self.testCheckoutUpdatedBranchHeads()


class GerritChangesFakeRepo(fake_repos.FakeReposBase):
    def populateGit(self):
        # Creates a tree that looks like this:
        #
        #       6 refs/changes/35/1235/1
        #       |
        #       5 refs/changes/34/1234/1
        #       |
        # 1--2--3--4 refs/heads/main
        #    |  |
        #    |  11(5)--12 refs/heads/main-with-5
        #    |
        #    7--8--9 refs/heads/feature
        #       |
        #       10 refs/changes/36/1236/1
        #

        self._commit_git('repo_1', {'commit 1': 'touched'})
        self._commit_git('repo_1', {'commit 2': 'touched'})
        self._commit_git('repo_1', {'commit 3': 'touched'})
        self._commit_git('repo_1', {'commit 4': 'touched'})
        self._create_ref('repo_1', 'refs/heads/main', 4)

        # Create a change on top of commit 3 that consists of two commits.
        self._commit_git('repo_1', {
            'commit 5': 'touched',
            'change': '1234'
        },
                         base=3)
        self._create_ref('repo_1', 'refs/changes/34/1234/1', 5)
        self._commit_git('repo_1', {'commit 6': 'touched', 'change': '1235'})
        self._create_ref('repo_1', 'refs/changes/35/1235/1', 6)

        # Create a refs/heads/feature branch on top of commit 2, consisting of
        # three commits.
        self._commit_git('repo_1', {'commit 7': 'touched'}, base=2)
        self._commit_git('repo_1', {'commit 8': 'touched'})
        self._commit_git('repo_1', {'commit 9': 'touched'})
        self._create_ref('repo_1', 'refs/heads/feature', 9)

        # Create a change of top of commit 8.
        self._commit_git('repo_1', {
            'commit 10': 'touched',
            'change': '1236'
        },
                         base=8)
        self._create_ref('repo_1', 'refs/changes/36/1236/1', 10)

        # Create a refs/heads/main-with-5 on top of commit 3 which is a branch
        # where refs/changes/34/1234/1 (commit 5) has already landed as commit
        # 11.
        self._commit_git(
            'repo_1',
            # This is really commit 11, but has the changes of commit 5
            {
                'commit 5': 'touched',
                'change': '1234'
            },
            base=3)
        self._commit_git('repo_1', {'commit 12': 'touched'})
        self._create_ref('repo_1', 'refs/heads/main-with-5', 12)


class GerritChangesTest(fake_repos.FakeReposTestBase):
    FAKE_REPOS_CLASS = GerritChangesFakeRepo

    def setUp(self):
        super(GerritChangesTest, self).setUp()
        self.enabled = self.FAKE_REPOS.set_up_git()
        self.options = BaseGitWrapperTestCase.OptionsObject()
        self.url = self.git_base + 'repo_1'
        self.mirror = None
        mock.patch('sys.stdout', StringIO()).start()
        self.addCleanup(mock.patch.stopall)

    def setUpMirror(self):
        self.mirror = tempfile.mkdtemp()
        git_cache.Mirror.SetCachePath(self.mirror)
        self.addCleanup(gclient_utils.rmtree, self.mirror)
        self.addCleanup(git_cache.Mirror.SetCachePath, None)

    def assertCommits(self, commits):
        """Check that all, and only |commits| are present in the current checkout.
        """
        for i in commits:
            name = os.path.join(self.root_dir, 'commit ' + str(i))
            self.assertTrue(os.path.exists(name), 'Commit not found: %s' % name)

        all_commits = set(range(1, len(self.FAKE_REPOS.git_hashes['repo_1'])))
        for i in all_commits - set(commits):
            name = os.path.join(self.root_dir, 'commit ' + str(i))
            self.assertFalse(os.path.exists(name),
                             'Unexpected commit: %s' % name)

    def testCanCloneGerritChange(self):
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        file_list = []

        self.options.revision = 'refs/changes/35/1235/1'
        git_wrapper.update(self.options, None, file_list)
        self.assertEqual(self.githash('repo_1', 6),
                         self.gitrevparse(self.root_dir))

    def testCanSyncToGerritChange(self):
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        file_list = []

        self.options.revision = self.githash('repo_1', 1)
        git_wrapper.update(self.options, None, file_list)
        self.assertEqual(self.githash('repo_1', 1),
                         self.gitrevparse(self.root_dir))

        self.options.revision = 'refs/changes/35/1235/1'
        git_wrapper.update(self.options, None, file_list)
        self.assertEqual(self.githash('repo_1', 6),
                         self.gitrevparse(self.root_dir))

    def testCanCloneGerritChangeMirror(self):
        self.setUpMirror()
        self.testCanCloneGerritChange()

    def testCanSyncToGerritChangeMirror(self):
        self.setUpMirror()
        self.testCanSyncToGerritChange()

    def testMirrorPushUrl(self):
        self.setUpMirror()

        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        file_list = []
        self.assertIsNotNone(git_wrapper._GetMirror(self.url, self.options))

        git_wrapper.update(self.options, None, file_list)

        fetch_url = git_wrapper._Capture(['remote', 'get-url', 'origin'])
        self.assertTrue(
            fetch_url.startswith(self.mirror),
            msg='\n'.join([
                'Repository fetch url should be in the git cache mirror directory.',
                '  fetch_url: %s' % fetch_url,
                '  mirror:    %s' % self.mirror
            ]))
        push_url = git_wrapper._Capture(
            ['remote', 'get-url', '--push', 'origin'])
        self.assertEqual(push_url, self.url)

    def testAppliesPatchOnTopOfMasterByDefault(self):
        """Test the default case, where we apply a patch on top of main."""
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        file_list = []

        # Make sure we don't specify a revision.
        self.options.revision = None
        git_wrapper.update(self.options, None, file_list)
        self.assertEqual(self.githash('repo_1', 4),
                         self.gitrevparse(self.root_dir))

        git_wrapper.apply_patch_ref(self.url, 'refs/changes/35/1235/1',
                                    'refs/heads/main', self.options, file_list)

        self.assertCommits([1, 2, 3, 4, 5, 6])
        self.assertEqual(self.githash('repo_1', 4),
                         self.gitrevparse(self.root_dir))

    def testCheckoutOlderThanPatchBase(self):
        """Test applying a patch on an old checkout.

        We first checkout commit 1, and try to patch refs/changes/35/1235/1, which
        contains commits 5 and 6, and is based on top of commit 3.
        The final result should contain commits 1, 5 and 6, but not commits 2 or 3.
        """
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        file_list = []

        # Sync to commit 1
        self.options.revision = self.githash('repo_1', 1)
        git_wrapper.update(self.options, None, file_list)
        self.assertEqual(self.githash('repo_1', 1),
                         self.gitrevparse(self.root_dir))

        # Apply the change on top of that.
        git_wrapper.apply_patch_ref(self.url, 'refs/changes/35/1235/1',
                                    'refs/heads/main', self.options, file_list)

        self.assertCommits([1, 5, 6])
        self.assertEqual(self.githash('repo_1', 1),
                         self.gitrevparse(self.root_dir))

    def testCheckoutOriginFeature(self):
        """Tests that we can apply a patch on a branch other than main."""
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        file_list = []

        # Sync to remote's refs/heads/feature
        self.options.revision = 'refs/heads/feature'
        git_wrapper.update(self.options, None, file_list)
        self.assertEqual(self.githash('repo_1', 9),
                         self.gitrevparse(self.root_dir))

        # Apply the change on top of that.
        git_wrapper.apply_patch_ref(self.url, 'refs/changes/36/1236/1',
                                    'refs/heads/feature', self.options,
                                    file_list)

        self.assertCommits([1, 2, 7, 8, 9, 10])
        self.assertEqual(self.githash('repo_1', 9),
                         self.gitrevparse(self.root_dir))

    def testCheckoutOriginFeatureOnOldRevision(self):
        """Tests that we can apply a patch on an old checkout, on a branch other
    than main."""
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        file_list = []

        # Sync to remote's refs/heads/feature on an old revision
        self.options.revision = self.githash('repo_1', 7)
        git_wrapper.update(self.options, None, file_list)
        self.assertEqual(self.githash('repo_1', 7),
                         self.gitrevparse(self.root_dir))

        # Apply the change on top of that.
        git_wrapper.apply_patch_ref(self.url, 'refs/changes/36/1236/1',
                                    'refs/heads/feature', self.options,
                                    file_list)

        # We shouldn't have rebased on top of 2 (which is the merge base between
        # remote's main branch and the change) but on top of 7 (which is the
        # merge base between remote's feature branch and the change).
        self.assertCommits([1, 2, 7, 10])
        self.assertEqual(self.githash('repo_1', 7),
                         self.gitrevparse(self.root_dir))

    def testCheckoutOriginFeaturePatchBranch(self):
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        file_list = []

        # Sync to the hash instead of remote's refs/heads/feature.
        self.options.revision = self.githash('repo_1', 9)
        git_wrapper.update(self.options, None, file_list)
        self.assertEqual(self.githash('repo_1', 9),
                         self.gitrevparse(self.root_dir))

        # Apply refs/changes/34/1234/1, created for remote's main branch on top
        # of remote's feature branch.
        git_wrapper.apply_patch_ref(self.url, 'refs/changes/35/1235/1',
                                    'refs/heads/main', self.options, file_list)

        # Commits 5 and 6 are part of the patch, and commits 1, 2, 7, 8 and 9
        # are part of remote's feature branch.
        self.assertCommits([1, 2, 5, 6, 7, 8, 9])
        self.assertEqual(self.githash('repo_1', 9),
                         self.gitrevparse(self.root_dir))

    def testDoesntRebasePatchMaster(self):
        """Tests that we can apply a patch without rebasing it."""
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        file_list = []

        self.options.rebase_patch_ref = False
        git_wrapper.update(self.options, None, file_list)
        self.assertEqual(self.githash('repo_1', 4),
                         self.gitrevparse(self.root_dir))

        # Apply the change on top of that.
        git_wrapper.apply_patch_ref(self.url, 'refs/changes/35/1235/1',
                                    'refs/heads/main', self.options, file_list)

        self.assertCommits([1, 2, 3, 5, 6])
        self.assertEqual(self.githash('repo_1', 5),
                         self.gitrevparse(self.root_dir))

    def testDoesntRebasePatchOldCheckout(self):
        """Tests that we can apply a patch without rebasing it on an old checkout.
        """
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        file_list = []

        # Sync to commit 1
        self.options.revision = self.githash('repo_1', 1)
        self.options.rebase_patch_ref = False
        git_wrapper.update(self.options, None, file_list)
        self.assertEqual(self.githash('repo_1', 1),
                         self.gitrevparse(self.root_dir))

        # Apply the change on top of that.
        git_wrapper.apply_patch_ref(self.url, 'refs/changes/35/1235/1',
                                    'refs/heads/main', self.options, file_list)

        self.assertCommits([1, 2, 3, 5, 6])
        self.assertEqual(self.githash('repo_1', 5),
                         self.gitrevparse(self.root_dir))

    def testDoesntSoftResetIfNotAskedTo(self):
        """Test that we can apply a patch without doing a soft reset."""
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        file_list = []

        self.options.reset_patch_ref = False
        git_wrapper.update(self.options, None, file_list)
        self.assertEqual(self.githash('repo_1', 4),
                         self.gitrevparse(self.root_dir))

        git_wrapper.apply_patch_ref(self.url, 'refs/changes/35/1235/1',
                                    'refs/heads/main', self.options, file_list)

        self.assertCommits([1, 2, 3, 4, 5, 6])
        # The commit hash after cherry-picking is not known, but it must be
        # different from what the repo was synced at before patching.
        self.assertNotEqual(self.githash('repo_1', 4),
                            self.gitrevparse(self.root_dir))

    @mock.patch('gerrit_util.GetChange', return_value={'topic': 'test_topic'})
    @mock.patch('gerrit_util.QueryChanges',
                return_value=[{
                    '_number': 1234
                }, {
                    '_number': 1235,
                    'current_revision': 'abc',
                    'revisions': {
                        'abc': {
                            'ref': 'refs/changes/35/1235/1'
                        }
                    }
                }])
    def testDownloadTopics(self, query_changes_mock, get_change_mock):
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        file_list = []

        self.options.revision = 'refs/changes/34/1234/1'
        git_wrapper.update(self.options, None, file_list)
        self.assertEqual(self.githash('repo_1', 5),
                         self.gitrevparse(self.root_dir))

        # pylint: disable=attribute-defined-outside-init
        self.options.download_topics = True
        git_wrapper.url = 'https://test-repo.googlesource.com/repo_1.git'
        git_wrapper.apply_patch_ref(self.url, 'refs/changes/34/1234/1',
                                    'refs/heads/main', self.options, file_list)

        get_change_mock.assert_called_once_with(mock.ANY, '1234')
        query_changes_mock.assert_called_once_with(mock.ANY,
                                                   [('topic', 'test_topic'),
                                                    ('status', 'open'),
                                                    ('repo', 'repo_1')],
                                                   o_params=['ALL_REVISIONS'])

        self.assertCommits([1, 2, 3, 5, 6])
        # The commit hash after the two cherry-picks is not known, but it must
        # be different from what the repo was synced at before patching.
        self.assertNotEqual(self.githash('repo_1', 4),
                            self.gitrevparse(self.root_dir))

    def testRecoversAfterPatchFailure(self):
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        file_list = []

        self.options.revision = 'refs/changes/34/1234/1'
        git_wrapper.update(self.options, None, file_list)
        self.assertEqual(self.githash('repo_1', 5),
                         self.gitrevparse(self.root_dir))

        # Checkout 'refs/changes/34/1234/1' modifies the 'change' file, so
        # trying to patch 'refs/changes/36/1236/1' creates a patch failure.
        with self.assertRaises(subprocess2.CalledProcessError) as cm:
            git_wrapper.apply_patch_ref(self.url, 'refs/changes/36/1236/1',
                                        'refs/heads/main', self.options,
                                        file_list)
        self.assertEqual(cm.exception.cmd[3], 'cherry-pick')
        self.assertIn(b'error: could not apply', cm.exception.stderr)

        # Try to apply 'refs/changes/35/1235/1', which doesn't have a merge
        # conflict.
        git_wrapper.apply_patch_ref(self.url, 'refs/changes/35/1235/1',
                                    'refs/heads/main', self.options, file_list)
        self.assertCommits([1, 2, 3, 5, 6])
        self.assertEqual(self.githash('repo_1', 5),
                         self.gitrevparse(self.root_dir))

    def testIgnoresAlreadyMergedCommits(self):
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        file_list = []

        self.options.revision = 'refs/heads/main-with-5'
        git_wrapper.update(self.options, None, file_list)
        self.assertEqual(self.githash('repo_1', 12),
                         self.gitrevparse(self.root_dir))

        # When we try 'refs/changes/35/1235/1' on top of 'refs/heads/feature',
        # 'refs/changes/34/1234/1' will be an empty commit, since the changes
        # were already present in the tree as commit 11. Make sure we deal with
        # this gracefully.
        git_wrapper.apply_patch_ref(self.url, 'refs/changes/35/1235/1',
                                    'refs/heads/feature', self.options,
                                    file_list)
        self.assertCommits([1, 2, 3, 5, 6, 12])
        self.assertEqual(self.githash('repo_1', 12),
                         self.gitrevparse(self.root_dir))

    def testRecoversFromExistingCherryPick(self):
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        file_list = []

        self.options.revision = 'refs/changes/34/1234/1'
        git_wrapper.update(self.options, None, file_list)
        self.assertEqual(self.githash('repo_1', 5),
                         self.gitrevparse(self.root_dir))

        # Checkout 'refs/changes/34/1234/1' modifies the 'change' file, so
        # trying to cherry-pick 'refs/changes/36/1236/1' raises an error.
        git_wrapper._Run(['fetch', 'origin', 'refs/changes/36/1236/1'],
                         self.options)
        with self.assertRaises(subprocess2.CalledProcessError) as cm:
            git_wrapper._Run(['cherry-pick', 'FETCH_HEAD'], self.options)
        self.assertEqual(cm.exception.cmd[:2], ['git', 'cherry-pick'])

        # Try to apply 'refs/changes/35/1235/1', which doesn't have a merge
        # conflict.
        git_wrapper.apply_patch_ref(self.url, 'refs/changes/35/1235/1',
                                    'refs/heads/main', self.options, file_list)
        self.assertCommits([1, 2, 3, 5, 6])
        self.assertEqual(self.githash('repo_1', 5),
                         self.gitrevparse(self.root_dir))


class DepsChangesFakeRepo(fake_repos.FakeReposBase):
    def populateGit(self):
        self._commit_git('repo_1', {'DEPS': 'versionA', 'doesnotmatter': 'B'})
        self._commit_git('repo_1', {'DEPS': 'versionA', 'doesnotmatter': 'C'})

        self._commit_git('repo_1', {'DEPS': 'versionB'})
        self._commit_git('repo_1', {'DEPS': 'versionA', 'doesnotmatter': 'C'})
        self._create_ref('repo_1', 'refs/heads/main', 4)


class CheckDiffTest(fake_repos.FakeReposTestBase):
    FAKE_REPOS_CLASS = DepsChangesFakeRepo

    def setUp(self):
        super(CheckDiffTest, self).setUp()
        self.enabled = self.FAKE_REPOS.set_up_git()
        self.options = BaseGitWrapperTestCase.OptionsObject()
        self.url = self.git_base + 'repo_1'
        self.mirror = None
        mock.patch('sys.stdout', StringIO()).start()
        self.addCleanup(mock.patch.stopall)

    def setUpMirror(self):
        self.mirror = tempfile.mkdtemp()
        git_cache.Mirror.SetCachePath(self.mirror)
        self.addCleanup(gclient_utils.rmtree, self.mirror)
        self.addCleanup(git_cache.Mirror.SetCachePath, None)

    def testCheckDiff(self):
        """Correctly check for diffs."""
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir, '.')
        file_list = []

        # Make sure we don't specify a revision.
        self.options.revision = None
        git_wrapper.update(self.options, None, file_list)
        self.assertEqual(self.githash('repo_1', 4),
                         self.gitrevparse(self.root_dir))

        self.assertFalse(
            git_wrapper.check_diff(self.githash('repo_1', 1), files=['DEPS']))
        self.assertTrue(git_wrapper.check_diff(self.githash('repo_1', 1)))
        self.assertTrue(
            git_wrapper.check_diff(self.githash('repo_1', 3), files=['DEPS']))

        self.assertFalse(
            git_wrapper.check_diff(self.githash('repo_1', 2),
                                   files=['DEPS', 'doesnotmatter']))
        self.assertFalse(git_wrapper.check_diff(self.githash('repo_1', 2)))


class Submodules(BaseGitWrapperTestCase):
    submodule_hash = '1111111111111111111111111111111111111111'

    def testGetSubmoduleClean(self):
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        options = self.Options()
        git_wrapper.update(options, None, [])
        self.assertEqual(git_wrapper.GetSubmoduleStateFromIndex(),
                         {'submodule': self.submodule_hash})
        self.assertEqual(git_wrapper.GetSubmoduleDiff(), {})

    def testGetSubmoduleModified(self):
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        options = self.Options()
        git_wrapper.update(options, None, [])

        # Create submodule diff
        submodule_dir = os.path.join(self.root_dir, 'submodule')
        subprocess2.check_output(['git', '-C', submodule_dir, 'init'])
        subprocess2.check_output([
            'git', '-C', submodule_dir, 'commit', '-m', 'foo', '--allow-empty'
        ])
        new_rev = subprocess2.check_output(
            ['git', '-C', submodule_dir, 'rev-parse',
             'HEAD']).decode('utf-8').strip()

        # And file diff
        with open(os.path.join(self.root_dir, 'a'), 'w') as f:
            f.write('foo')

        self.assertEqual(git_wrapper.GetSubmoduleStateFromIndex(),
                         {'submodule': self.submodule_hash})

        self.assertEqual(git_wrapper.GetSubmoduleDiff(),
                         {'submodule': (self.submodule_hash, new_rev)})

    def testGetSubmoduleDeleted(self):
        git_wrapper = gclient_scm.GitWrapper(self.url, self.root_dir,
                                             self.relpath)
        options = self.Options()
        git_wrapper.update(options, None, [])
        subprocess2.check_output(
            ['git', '-C', self.root_dir, 'rm', 'submodule'])

        # When git removes submodule, it's autmatically staged and content is
        # unavailable. Therefore, the index shouldn't have any entries and diff
        # should be empty.
        self.assertEqual(git_wrapper.GetSubmoduleStateFromIndex(), {})

        self.assertEqual(git_wrapper.GetSubmoduleDiff(), {})


if 'unittest.util' in __import__('sys').modules:
    # Show full diff in self.assertEqual.
    __import__('sys').modules['unittest.util']._MAX_LENGTH = 999999999

if __name__ == '__main__':
    level = logging.DEBUG if '-v' in sys.argv else logging.FATAL
    logging.basicConfig(level=level,
                        format='%(asctime).19s %(levelname)s %(filename)s:'
                        '%(lineno)s %(message)s')
    unittest.main()

# vim: ts=2:sw=2:tw=80:et:
