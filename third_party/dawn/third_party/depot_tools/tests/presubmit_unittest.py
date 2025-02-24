#!/usr/bin/env vpython3
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for presubmit_support.py and presubmit_canned_checks.py."""

# pylint: disable=no-member,E1103

import functools
import io
import itertools
import logging
import multiprocessing
import os
import random
import re
import sys
import tempfile
import threading
import time
import unittest

from io import StringIO
from unittest import mock
import urllib.request as urllib_request

_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, _ROOT)

from testing_support.test_case_utils import TestCaseUtils

import auth
import gclient_utils
import git_cl
import git_common as git
import json
import owners_client
import presubmit_support as presubmit
import rdb_wrapper
import scm
import subprocess2 as subprocess

# Shortcut.
presubmit_canned_checks = presubmit.presubmit_canned_checks

RUNNING_PY_CHECKS_TEXT = ('Running presubmit upload checks ...\n')

# Access to a protected member XXX of a client class
# pylint: disable=protected-access

# TODO: Should fix these warnings.
# pylint: disable=line-too-long


class MockTemporaryFile(object):
    """Simple mock for files returned by tempfile.NamedTemporaryFile()."""
    def __init__(self, name):
        self.name = name

    def __enter__(self):
        return self

    def __exit__(self, *args):
        pass

    def close(self):
        pass


class PresubmitTestsBase(TestCaseUtils, unittest.TestCase):
    """Sets up and tears down the mocks but doesn't test anything as-is."""
    presubmit_text = """
def CheckChangeOnUpload(input_api, output_api):
  if input_api.change.tags.get('ERROR'):
    return [output_api.PresubmitError("!!")]
  if input_api.change.tags.get('PROMPT_WARNING'):
    return [output_api.PresubmitPromptWarning("??")]
  else:
    return ()

def PostUploadHook(gerrit, change, output_api):
  if change.tags.get('ERROR'):
    return [output_api.PresubmitError("!!")]
  if change.tags.get('PROMPT_WARNING'):
    return [output_api.PresubmitPromptWarning("??")]
  else:
    return ()
"""

    presubmit_trymaster = """
def GetPreferredTryMasters(project, change):
  return %s
"""

    presubmit_diffs = """
diff --git %(filename)s %(filename)s
index fe3de7b..54ae6e1 100755
--- %(filename)s       2011-02-09 10:38:16.517224845 -0800
+++ %(filename)s       2011-02-09 10:38:53.177226516 -0800
@@ -1,6 +1,5 @@
 this is line number 0
 this is line number 1
-this is line number 2 to be deleted
 this is line number 3
 this is line number 4
 this is line number 5
@@ -8,7 +7,7 @@
 this is line number 7
 this is line number 8
 this is line number 9
-this is line number 10 to be modified
+this is line number 10
 this is line number 11
 this is line number 12
 this is line number 13
@@ -21,9 +20,8 @@
 this is line number 20
 this is line number 21
 this is line number 22
-this is line number 23
-this is line number 24
-this is line number 25
+this is line number 23.1
+this is line number 25.1
 this is line number 26
 this is line number 27
 this is line number 28
@@ -31,6 +29,7 @@
 this is line number 30
 this is line number 31
 this is line number 32
+this is line number 32.1
 this is line number 33
 this is line number 34
 this is line number 35
@@ -38,14 +37,14 @@
 this is line number 37
 this is line number 38
 this is line number 39
-
 this is line number 40
-this is line number 41
+this is line number 41.1
 this is line number 42
 this is line number 43
 this is line number 44
 this is line number 45
+
 this is line number 46
 this is line number 47
-this is line number 48
+this is line number 48.1
 this is line number 49
"""

    def setUp(self):
        super(PresubmitTestsBase, self).setUp()
        # Disable string diff max. It's hard to parse assertion error if there's
        # limit set.
        self.maxDiff = None

        class FakeChange(object):
            def __init__(self, obj):
                self._root = obj.fake_root_dir
                self.issue = 0

            def RepositoryRoot(self):
                return self._root

            def UpstreamBranch(self):
                return 'upstream'

        presubmit._ASKED_FOR_FEEDBACK = False
        self.fake_root_dir = self.RootDir()
        self.fake_change = FakeChange(self)
        self.rdb_client = mock.MagicMock()

        mock.patch('gclient_utils.FileRead').start()
        mock.patch('gclient_utils.FileWrite').start()
        mock.patch('json.load').start()
        mock.patch('multiprocessing.cpu_count', lambda: 2)
        mock.patch('gerrit_util.IsCodeOwnersEnabledOnHost').start()
        mock.patch('os.chdir').start()
        mock.patch('os.getcwd', self.RootDir)
        mock.patch('os.listdir').start()
        mock.patch('os.path.abspath', lambda f: f).start()
        mock.patch('os.path.isfile').start()
        mock.patch('os.remove').start()
        mock.patch('presubmit_support._parse_files').start()
        mock.patch('presubmit_support.rdb_wrapper.client',
                   return_value=self.rdb_client).start()
        mock.patch('presubmit_support.sigint_handler').start()
        mock.patch('presubmit_support.time_time', return_value=0).start()
        mock.patch('presubmit_support.warn').start()
        mock.patch('random.randint').start()
        mock.patch('scm.GIT.GenerateDiff').start()
        mock.patch('scm.GIT.GetBranch', lambda x: None).start()
        mock.patch('scm.determine_scm').start()
        mock.patch('subprocess2.Popen').start()
        mock.patch('sys.stderr', StringIO()).start()
        mock.patch('sys.stdout', StringIO()).start()
        mock.patch('tempfile.NamedTemporaryFile').start()
        mock.patch('threading.Timer').start()
        mock.patch('urllib.request.urlopen').start()
        self.addCleanup(mock.patch.stopall)

    def checkstdout(self, value):
        self.assertEqual(sys.stdout.getvalue(), value)


class PresubmitUnittest(PresubmitTestsBase):
    """General presubmit_support.py tests (excluding InputApi and OutputApi)."""

    _INHERIT_SETTINGS = 'inherit-review-settings-ok'
    fake_root_dir = '/foo/bar'

    def testCannedCheckFilter(self):
        canned = presubmit.presubmit_canned_checks
        orig = canned.CheckOwners
        with presubmit.canned_check_filter(['CheckOwners']):
            self.assertNotEqual(canned.CheckOwners, orig)
            self.assertEqual(canned.CheckOwners(None, None), [])
        self.assertEqual(canned.CheckOwners, orig)

    def testListRelevantPresubmitFiles(self):
        files = [
            'blat.cc',
            os.path.join('foo', 'haspresubmit', 'yodle', 'smart.h'),
            os.path.join('moo', 'mat', 'gat', 'yo.h'),
            os.path.join('foo', 'luck.h'),
        ]

        known_files = [
            os.path.join(self.fake_root_dir, 'PRESUBMIT.py'),
            os.path.join(self.fake_root_dir, 'foo', 'haspresubmit',
                         'PRESUBMIT.py'),
            os.path.join(self.fake_root_dir, 'foo', 'haspresubmit', 'yodle',
                         'PRESUBMIT.py'),
        ]
        os.path.isfile.side_effect = lambda f: f in known_files

        dirs_with_presubmit = [
            self.fake_root_dir,
            os.path.join(self.fake_root_dir, 'foo', 'haspresubmit'),
            os.path.join(self.fake_root_dir, 'foo', 'haspresubmit', 'yodle'),
        ]
        os.listdir.side_effect = (lambda d: ['PRESUBMIT.py']
                                  if d in dirs_with_presubmit else [])

        presubmit_files = presubmit.ListRelevantPresubmitFiles(
            files, self.fake_root_dir)
        self.assertEqual(presubmit_files, known_files)

    def testListUserPresubmitFiles(self):
        files = [
            'blat.cc',
        ]

        os.path.isfile.side_effect = lambda f: 'PRESUBMIT' in f
        os.listdir.return_value = [
            'PRESUBMIT.py', 'PRESUBMIT_test.py', 'PRESUBMIT-user.py'
        ]

        presubmit_files = presubmit.ListRelevantPresubmitFiles(
            files, self.fake_root_dir)
        self.assertEqual(presubmit_files, [
            os.path.join(self.fake_root_dir, 'PRESUBMIT.py'),
            os.path.join(self.fake_root_dir, 'PRESUBMIT-user.py'),
        ])

    def testListRelevantPresubmitFilesInheritSettings(self):
        sys_root_dir = self._OS_SEP
        root_dir = os.path.join(sys_root_dir, 'foo', 'bar')
        inherit_path = os.path.join(root_dir, self._INHERIT_SETTINGS)
        files = [
            'test.cc',
            os.path.join('moo', 'test2.cc'),
            os.path.join('zoo', 'test3.cc')
        ]

        known_files = [
            inherit_path,
            os.path.join(sys_root_dir, 'foo', 'PRESUBMIT.py'),
            os.path.join(sys_root_dir, 'foo', 'bar', 'moo', 'PRESUBMIT.py'),
        ]
        os.path.isfile.side_effect = lambda f: f in known_files

        dirs_with_presubmit = [
            os.path.join(sys_root_dir, 'foo'),
            os.path.join(sys_root_dir, 'foo', 'bar', 'moo'),
        ]
        os.listdir.side_effect = (lambda d: ['PRESUBMIT.py']
                                  if d in dirs_with_presubmit else [])

        presubmit_files = presubmit.ListRelevantPresubmitFiles(files, root_dir)
        self.assertEqual(presubmit_files, [
            os.path.join(sys_root_dir, 'foo', 'PRESUBMIT.py'),
            os.path.join(sys_root_dir, 'foo', 'bar', 'moo', 'PRESUBMIT.py')
        ])

    def testTagLineRe(self):
        m = presubmit.Change.TAG_LINE_RE.match(' BUG =1223, 1445  \t')
        self.assertIsNotNone(m)
        self.assertEqual(m.group('key'), 'BUG')
        self.assertEqual(m.group('value'), '1223, 1445')

    def testGitChange(self):
        description_lines = ('Hello there', 'this is a change', 'BUG=123',
                             'and some more regular text  \t')
        unified_diff = [
            'diff --git binary_a.png binary_a.png', 'new file mode 100644',
            'index 0000000..6fbdd6d',
            'Binary files /dev/null and binary_a.png differ',
            'diff --git binary_d.png binary_d.png', 'deleted file mode 100644',
            'index 6fbdd6d..0000000',
            'Binary files binary_d.png and /dev/null differ',
            'diff --git binary_md.png binary_md.png',
            'index 6fbdd6..be3d5d8 100644', 'GIT binary patch', 'delta 109',
            'zcmeyihjs5>)(Opwi4&WXB~yyi6N|G`(i5|?i<2_a@)OH5N{Um`D-<SM@g!_^W9;SR',
            'zO9b*W5{pxTM0slZ=F42indK9U^MTyVQlJ2s%1BMmEKMv1Q^gtS&9nHn&*Ede;|~CU',
            'CMJxLN', '', 'delta 34',
            'scmV+-0Nww+y#@BX1(1W0gkzIp3}CZh0gVZ>`wGVcgW(Rh;SK@ZPa9GXlK=n!', '',
            'diff --git binary_m.png binary_m.png',
            'index 6fbdd6d..be3d5d8 100644',
            'Binary files binary_m.png and binary_m.png differ',
            'diff --git boo/blat.cc boo/blat.cc', 'new file mode 100644',
            'index 0000000..37d18ad', '--- boo/blat.cc', '+++ boo/blat.cc',
            '@@ -0,0 +1,5 @@', '+This is some text',
            '+which lacks a copyright warning',
            '+but it is nonetheless interesting',
            '+and worthy of your attention.',
            '+Its freshness factor is through the roof.',
            'diff --git floo/delburt.cc floo/delburt.cc',
            'deleted file mode 100644', 'index e06377a..0000000',
            '--- floo/delburt.cc', '+++ /dev/null', '@@ -1,14 +0,0 @@',
            '-This text used to be here', '-but someone, probably you,',
            '-having consumed the text', '-  (absorbed its meaning)',
            '-decided that it should be made to not exist',
            '-that others would not read it.', '-  (What happened here?',
            '-was the author incompetent?',
            '-or is the world today so different from the world',
            '-   the author foresaw', '-and past imaginination',
            '-   amounts to rubble, insignificant,',
            '-something to be tripped over', '-and frustrated by)',
            'diff --git foo/TestExpectations foo/TestExpectations',
            'index c6e12ab..d1c5f23 100644', '--- foo/TestExpectations',
            '+++ foo/TestExpectations', '@@ -1,12 +1,24 @@',
            '-Stranger, behold:', '+Strange to behold:', '   This is a text',
            ' Its contents existed before.', '', '-It is written:',
            '+Weasel words suggest:', '   its contents shall exist after',
            '   and its contents', ' with the progress of time',
            ' will evolve,', '-   snaillike,', '+   erratically,',
            ' into still different texts', '-from this.',
            '\ No newline at end of file', '+from this.', '+',
            '+For the most part,', '+I really think unified diffs',
            '+are elegant: the way you can type',
            '+diff --git inside/a/text inside/a/text',
            '+or something silly like', '+@@ -278,6 +278,10 @@',
            '+and have this not be interpreted', '+as the start of a new file',
            '+or anything messed up like that,',
            '+because you parsed the header', '+correctly.',
            '\ No newline at end of file', ''
        ]
        files = [
            ('A      ', 'binary_a.png'),
            ('D      ', 'binary_d.png'),
            ('M      ', 'binary_m.png'),
            ('M      ', 'binary_md.png'),  # Binary w/ diff
            ('A      ', 'boo/blat.cc'),
            ('D      ', 'floo/delburt.cc'),
            ('M      ', 'foo/TestExpectations')
        ]

        known_files = [
            os.path.join(self.fake_root_dir, *path.split('/'))
            for op, path in files if not op.startswith('D')
        ]
        os.path.isfile.side_effect = lambda f: f in known_files

        scm.GIT.GenerateDiff.return_value = '\n'.join(unified_diff)

        change = presubmit.GitChange('mychange',
                                     '\n'.join(description_lines),
                                     self.fake_root_dir,
                                     files,
                                     0,
                                     0,
                                     None,
                                     upstream='upstream',
                                     end_commit='end_commit')
        self.assertIsNotNone(change.Name() == 'mychange')
        self.assertIsNotNone(change.DescriptionText(
        ) == 'Hello there\nthis is a change\nand some more regular text')
        self.assertIsNotNone(
            change.FullDescriptionText() == '\n'.join(description_lines))

        self.assertIsNotNone(change.BugsFromDescription() == ['123'])

        self.assertIsNotNone(len(change.AffectedFiles()) == 7)
        self.assertIsNotNone(len(change.AffectedFiles()) == 7)
        self.assertIsNotNone(
            len(change.AffectedFiles(include_deletes=False)) == 5)
        self.assertIsNotNone(
            len(change.AffectedFiles(include_deletes=False)) == 5)

        # Note that on git, there's no distinction between binary files and text
        # files; everything that's not a delete is a text file.
        affected_text_files = change.AffectedTestableFiles()
        self.assertIsNotNone(len(affected_text_files) == 5)

        local_paths = change.LocalPaths()
        expected_paths = [os.path.normpath(f) for op, f in files]
        self.assertEqual(local_paths, expected_paths)

        actual_rhs_lines = []
        for f, linenum, line in change.RightHandSideLines():
            actual_rhs_lines.append((f.LocalPath(), linenum, line))
        scm.GIT.GenerateDiff.assert_called_once_with(self.fake_root_dir,
                                                     files=[],
                                                     full_move=True,
                                                     branch='upstream',
                                                     branch_head='end_commit')


        f_blat = os.path.normpath('boo/blat.cc')
        f_test_expectations = os.path.normpath('foo/TestExpectations')
        expected_rhs_lines = [
            (f_blat, 1, 'This is some text'),
            (f_blat, 2, 'which lacks a copyright warning'),
            (f_blat, 3, 'but it is nonetheless interesting'),
            (f_blat, 4, 'and worthy of your attention.'),
            (f_blat, 5, 'Its freshness factor is through the roof.'),
            (f_test_expectations, 1, 'Strange to behold:'),
            (f_test_expectations, 5, 'Weasel words suggest:'),
            (f_test_expectations, 10, '   erratically,'),
            (f_test_expectations, 13, 'from this.'),
            (f_test_expectations, 14, ''),
            (f_test_expectations, 15, 'For the most part,'),
            (f_test_expectations, 16, 'I really think unified diffs'),
            (f_test_expectations, 17, 'are elegant: the way you can type'),
            (f_test_expectations, 18, 'diff --git inside/a/text inside/a/text'),
            (f_test_expectations, 19, 'or something silly like'),
            (f_test_expectations, 20, '@@ -278,6 +278,10 @@'),
            (f_test_expectations, 21, 'and have this not be interpreted'),
            (f_test_expectations, 22, 'as the start of a new file'),
            (f_test_expectations, 23, 'or anything messed up like that,'),
            (f_test_expectations, 24, 'because you parsed the header'),
            (f_test_expectations, 25, 'correctly.')
        ]

        self.assertEqual(expected_rhs_lines, actual_rhs_lines)

    def testInvalidChange(self):
        with self.assertRaises(AssertionError):
            presubmit.GitChange('mychange',
                                'description',
                                self.fake_root_dir, ['foo/blat.cc', 'bar'],
                                0,
                                0,
                                None,
                                upstream='upstream',
                                end_commit='HEAD')

    def testExecPresubmitScript(self):
        description_lines = ('Hello there', 'this is a change', 'BUG=123')
        files = [
            ['A', 'foo\\blat.cc'],
        ]
        fake_presubmit = os.path.join(self.fake_root_dir, 'PRESUBMIT.py')

        change = presubmit.Change('mychange', '\n'.join(description_lines),
                                  self.fake_root_dir, files, 0, 0, None)
        executer = presubmit.PresubmitExecuter(change, False, None,
                                               presubmit.GerritAccessor())
        self.assertFalse(executer.ExecPresubmitScript('', fake_presubmit))
        # No error if no on-upload entry point
        self.assertFalse(
            executer.ExecPresubmitScript(
                ('def CheckChangeOnCommit(input_api, output_api):\n'
                 '  return (output_api.PresubmitError("!!"))\n'),
                fake_presubmit))

        executer = presubmit.PresubmitExecuter(change, True, None,
                                               presubmit.GerritAccessor())
        # No error if no on-commit entry point
        self.assertFalse(
            executer.ExecPresubmitScript(
                ('def CheckChangeOnUpload(input_api, output_api):\n'
                 '  return (output_api.PresubmitError("!!"))\n'),
                fake_presubmit))

        self.assertFalse(
            executer.ExecPresubmitScript(
                ('def CheckChangeOnUpload(input_api, output_api):\n'
                 '  if not input_api.change.BugsFromDescription():\n'
                 '    return (output_api.PresubmitError("!!"))\n'
                 '  else:\n'
                 '    return ()'), fake_presubmit))

        self.assertFalse(
            executer.ExecPresubmitScript(
                'def CheckChangeOnCommit(input_api, output_api):\n'
                '  results = []\n'
                '  results.extend(input_api.canned_checks.CheckChangeHasBugField(\n'
                '    input_api, output_api))\n'
                '  results.extend(input_api.canned_checks.'
                'CheckChangeHasNoUnwantedTags(\n'
                '    input_api, output_api))\n'
                '  results.extend(input_api.canned_checks.'
                'CheckChangeHasDescription(\n'
                '    input_api, output_api))\n'
                '  return results\n', fake_presubmit))

    def testExecPresubmitScriptWithResultDB(self):
        description_lines = ('Hello there', 'this is a change', 'BUG=123')
        files = [['A', 'foo\\blat.cc']]
        fake_presubmit = os.path.join(self.fake_root_dir, 'PRESUBMIT.py')
        change = presubmit.Change('mychange', '\n'.join(description_lines),
                                  self.fake_root_dir, files, 0, 0, None)
        executer = presubmit.PresubmitExecuter(change, True, None,
                                               presubmit.GerritAccessor())
        sink = self.rdb_client.__enter__.return_value = mock.MagicMock()

        # STATUS_PASS on success
        executer.ExecPresubmitScript(
            'def CheckChangeOnCommit(input_api, output_api):\n'
            '  return [output_api.PresubmitResult("test")]\n', fake_presubmit)
        sink.report.assert_called_with('CheckChangeOnCommit',
                                       rdb_wrapper.STATUS_PASS, 0, None)

        # STATUS_FAIL on fatal error
        sink.reset_mock()
        executer.ExecPresubmitScript(
            'def CheckChangeOnCommit(input_api, output_api):\n'
            '  return [output_api.PresubmitError("error")]\n', fake_presubmit)
        sink.report.assert_called_with('CheckChangeOnCommit',
                                       rdb_wrapper.STATUS_FAIL, 0, "error\n")

    def testExecPresubmitScriptTemporaryFilesRemoval(self):
        tempfile.NamedTemporaryFile.side_effect = [
            MockTemporaryFile('baz'),
            MockTemporaryFile('quux'),
        ]

        fake_presubmit = os.path.join(self.fake_root_dir, 'PRESUBMIT.py')
        executer = presubmit.PresubmitExecuter(self.fake_change, False, None,
                                               presubmit.GerritAccessor())

        self.assertEqual(
            [],
            executer.ExecPresubmitScript(
                ('def CheckChangeOnUpload(input_api, output_api):\n'
                 '  if len(input_api._named_temporary_files):\n'
                 '    return (output_api.PresubmitError("!!"),)\n'
                 '  return ()\n'), fake_presubmit))

        result = executer.ExecPresubmitScript(
            ('def CheckChangeOnUpload(input_api, output_api):\n'
             '  with input_api.CreateTemporaryFile():\n'
             '    pass\n'
             '  with input_api.CreateTemporaryFile():\n'
             '    pass\n'
             '  return [output_api.PresubmitResult(\'\', f)\n'
             '          for f in input_api._named_temporary_files]\n'),
            fake_presubmit)
        self.assertEqual(['baz', 'quux'], [r._items for r in result])

        self.assertEqual(os.remove.mock_calls,
                         [mock.call('baz'), mock.call('quux')])

    def testExecPresubmitScriptInSourceDirectory(self):
        """Tests that the presubmits are executed with the current working
        directory (CWD) set to the directory of the source presubmit script.
        """
        orig_dir = os.getcwd()

        fake_presubmit_dir = os.path.join(self.fake_root_dir, 'fake_dir')
        fake_presubmit = os.path.join(fake_presubmit_dir, 'PRESUBMIT.py')
        change = self.ExampleChange()

        executer = presubmit.PresubmitExecuter(change, False, None,
                                               presubmit.GerritAccessor())

        executer.ExecPresubmitScript('', fake_presubmit)

        # Check that the executer switched to the directory of the script and
        # back.
        self.assertEqual(os.chdir.call_args_list, [
            mock.call(fake_presubmit_dir),
            mock.call(orig_dir),
        ])

    def testExecPostUploadHookSourceDirectory(self):
        """Tests that the post upload hooks are executed with the current working
        directory (CWD) set to the directory of the source presubmit script.
        s"""
        orig_dir = os.getcwd()

        fake_presubmit_dir = os.path.join(self.fake_root_dir, 'fake_dir')
        fake_presubmit = os.path.join(fake_presubmit_dir, 'PRESUBMIT.py')
        change = self.ExampleChange()

        executer = presubmit.GetPostUploadExecuter(change,
                                                   presubmit.GerritAccessor())

        executer.ExecPresubmitScript(self.presubmit_text, fake_presubmit)

        # Check that the executer switched to the directory of the script and
        # back.
        self.assertEqual(os.chdir.call_args_list, [
            mock.call(fake_presubmit_dir),
            mock.call(orig_dir),
        ])

    def testDoPostUploadExecuter(self):
        os.path.isfile.side_effect = lambda f: 'PRESUBMIT.py' in f
        os.listdir.return_value = ['PRESUBMIT.py']
        gclient_utils.FileRead.return_value = self.presubmit_text
        change = self.ExampleChange()
        self.assertEqual(
            0,
            presubmit.DoPostUploadExecuter(change=change,
                                           gerrit_obj=None,
                                           verbose=False))
        expected = (r'Running post upload checks \.\.\.\n')
        self.assertRegexpMatches(sys.stdout.getvalue(), expected)

    def testDoPostUploadExecuterWarning(self):
        path = os.path.join(self.fake_root_dir, 'PRESUBMIT.py')
        os.path.isfile.side_effect = lambda f: f == path
        os.listdir.return_value = ['PRESUBMIT.py']
        gclient_utils.FileRead.return_value = self.presubmit_text
        change = self.ExampleChange(extra_lines=['PROMPT_WARNING=yes'])
        self.assertEqual(
            0,
            presubmit.DoPostUploadExecuter(change=change,
                                           gerrit_obj=None,
                                           verbose=False))
        self.assertEqual(
            'Running post upload checks ...\n'
            '\n'
            '** Post Upload Hook Messages **\n'
            '??\n'
            '\n', sys.stdout.getvalue())

    def testDoPostUploadExecuterWarning(self):
        path = os.path.join(self.fake_root_dir, 'PRESUBMIT.py')
        os.path.isfile.side_effect = lambda f: f == path
        os.listdir.return_value = ['PRESUBMIT.py']
        gclient_utils.FileRead.return_value = self.presubmit_text
        change = self.ExampleChange(extra_lines=['ERROR=yes'])
        self.assertEqual(
            1,
            presubmit.DoPostUploadExecuter(change=change,
                                           gerrit_obj=None,
                                           verbose=False))

        expected = ('Running post upload checks \.\.\.\n'
                    '\n'
                    '\*\* Post Upload Hook Messages \*\*\n'
                    '!!\n'
                    '\n')
        self.assertRegexpMatches(sys.stdout.getvalue(), expected)

    def testDoPresubmitChecksNoWarningsOrErrors(self):
        haspresubmit_path = os.path.join(self.fake_root_dir, 'haspresubmit',
                                         'PRESUBMIT.py')
        root_path = os.path.join(self.fake_root_dir, 'PRESUBMIT.py')

        os.path.isfile.side_effect = lambda f: f in [
            root_path, haspresubmit_path
        ]
        os.listdir.return_value = ['PRESUBMIT.py']

        gclient_utils.FileRead.return_value = self.presubmit_text

        # Make a change which will have no warnings.
        change = self.ExampleChange(extra_lines=['STORY=http://tracker/123'])

        self.assertEqual(
            0,
            presubmit.DoPresubmitChecks(change=change,
                                        committing=False,
                                        verbose=True,
                                        default_presubmit=None,
                                        may_prompt=False,
                                        gerrit_obj=None,
                                        json_output=None))
        self.assertEqual(sys.stdout.getvalue().count('!!'), 0)
        self.assertEqual(sys.stdout.getvalue().count('??'), 0)
        self.assertEqual(sys.stdout.getvalue().count(RUNNING_PY_CHECKS_TEXT), 1)

    def testDoPresubmitChecksJsonOutput(self):
        fake_error = 'Missing LGTM'
        fake_error_items = '["!", "!!", "!!!"]'
        fake_error_long_text = "Error long text..."
        fake_error2 = 'This failed was found in file fake.py'
        fake_error2_items = '["!!!", "!!", "!"]'
        fake_error2_long_text = " Error long text" * 3
        fake_warning = 'Line 88 is more than 80 characters.'
        fake_warning_items = '["W", "w"]'
        fake_warning_long_text = 'Warning long text...'
        fake_notify = 'This is a dry run'
        fake_notify_items = '["N"]'
        fake_notify_long_text = 'Notification long text...'
        always_fail_presubmit_script = ("""\n
def CheckChangeOnUpload(input_api, output_api):
  output_api.more_cc = ['me@example.com']
  return [
    output_api.PresubmitError("%s",%s, "%s"),
    output_api.PresubmitError("%s",%s, "%s"),
    output_api.PresubmitPromptWarning("%s",%s, "%s"),
    output_api.PresubmitNotifyResult("%s",%s, "%s")
  ]
def CheckChangeOnCommit(input_api, output_api):
  raise Exception("Test error")
""" % (fake_error, fake_error_items, fake_error_long_text, fake_error2,
        fake_error2_items, fake_error2_long_text, fake_warning,
        fake_warning_items, fake_warning_long_text, fake_notify,
        fake_notify_items, fake_notify_long_text))

        os.path.isfile.return_value = False
        os.listdir.side_effect = [[], ['PRESUBMIT.py']]
        random.randint.return_value = 0

        change = self.ExampleChange(extra_lines=['ERROR=yes'])

        temp_path = 'temp.json'

        fake_result = {
            'notifications': [{
                'message': fake_notify,
                'items': json.loads(fake_notify_items),
                'fatal': False,
                'long_text': fake_notify_long_text
            }],
            'errors': [{
                'message': fake_error,
                'items': json.loads(fake_error_items),
                'fatal': True,
                'long_text': fake_error_long_text
            }, {
                'message': fake_error2,
                'items': json.loads(fake_error2_items),
                'fatal': True,
                'long_text': fake_error2_long_text
            }],
            'warnings': [{
                'message': fake_warning,
                'items': json.loads(fake_warning_items),
                'fatal': False,
                'long_text': fake_warning_long_text
            }],
            'more_cc': ['me@example.com'],
        }

        fake_result_json = json.dumps(fake_result, sort_keys=True)

        self.assertEqual(
            1,
            presubmit.DoPresubmitChecks(
                change=change,
                committing=False,
                verbose=True,
                default_presubmit=always_fail_presubmit_script,
                may_prompt=False,
                gerrit_obj=None,
                json_output=temp_path))

        gclient_utils.FileWrite.assert_called_with(temp_path, fake_result_json)

    def testDoPresubmitChecksPromptsAfterWarnings(self):
        presubmit_path = os.path.join(self.fake_root_dir, 'PRESUBMIT.py')
        haspresubmit_path = os.path.join(self.fake_root_dir, 'haspresubmit',
                                         'PRESUBMIT.py')

        os.path.isfile.side_effect = (
            lambda f: f in [presubmit_path, haspresubmit_path])
        os.listdir.return_value = ['PRESUBMIT.py']
        random.randint.return_value = 1

        gclient_utils.FileRead.return_value = self.presubmit_text

        # Make a change with a single warning.
        change = self.ExampleChange(extra_lines=['PROMPT_WARNING=yes'])

        # say no to the warning
        with mock.patch('sys.stdin', StringIO('n\n')):
            self.assertEqual(
                1,
                presubmit.DoPresubmitChecks(change=change,
                                            committing=False,
                                            verbose=True,
                                            default_presubmit=None,
                                            may_prompt=True,
                                            gerrit_obj=None,
                                            json_output=None))
            self.assertEqual(sys.stdout.getvalue().count('??'), 2)

        sys.stdout.truncate(0)
        # say yes to the warning
        with mock.patch('sys.stdin', StringIO('y\n')):
            self.assertEqual(
                0,
                presubmit.DoPresubmitChecks(change=change,
                                            committing=False,
                                            verbose=True,
                                            default_presubmit=None,
                                            may_prompt=True,
                                            gerrit_obj=None,
                                            json_output=None))
            self.assertEqual(sys.stdout.getvalue().count('??'), 2)
        self.assertEqual(sys.stdout.getvalue().count(RUNNING_PY_CHECKS_TEXT), 1)

    def testDoPresubmitChecksWithWarningsAndNoPrompt(self):
        presubmit_path = os.path.join(self.fake_root_dir, 'PRESUBMIT.py')
        haspresubmit_path = os.path.join(self.fake_root_dir, 'haspresubmit',
                                         'PRESUBMIT.py')

        os.path.isfile.side_effect = (
            lambda f: f in [presubmit_path, haspresubmit_path])
        os.listdir.return_value = ['PRESUBMIT.py']
        gclient_utils.FileRead.return_value = self.presubmit_text
        random.randint.return_value = 1

        change = self.ExampleChange(extra_lines=['PROMPT_WARNING=yes'])

        # There is no input buffer and may_prompt is set to False.
        self.assertEqual(
            0,
            presubmit.DoPresubmitChecks(change=change,
                                        committing=False,
                                        verbose=True,
                                        default_presubmit=None,
                                        may_prompt=False,
                                        gerrit_obj=None,
                                        json_output=None))
        # A warning is printed, and should_continue is True.
        self.assertEqual(sys.stdout.getvalue().count('??'), 2)
        self.assertEqual(sys.stdout.getvalue().count('(y/N)'), 0)
        self.assertEqual(sys.stdout.getvalue().count(RUNNING_PY_CHECKS_TEXT), 1)

    def testDoPresubmitChecksNoWarningPromptIfErrors(self):
        presubmit_path = os.path.join(self.fake_root_dir, 'PRESUBMIT.py')
        haspresubmit_path = os.path.join(self.fake_root_dir, 'haspresubmit',
                                         'PRESUBMIT.py')

        os.path.isfile.side_effect = (
            lambda f: f in [presubmit_path, haspresubmit_path])
        os.listdir.return_value = ['PRESUBMIT.py']
        gclient_utils.FileRead.return_value = self.presubmit_text
        random.randint.return_value = 1

        change = self.ExampleChange(extra_lines=['ERROR=yes'])
        self.assertEqual(
            1,
            presubmit.DoPresubmitChecks(change=change,
                                        committing=False,
                                        verbose=True,
                                        default_presubmit=None,
                                        may_prompt=True,
                                        gerrit_obj=None,
                                        json_output=None))
        self.assertEqual(sys.stdout.getvalue().count('??'), 0)
        self.assertEqual(sys.stdout.getvalue().count('!!'), 2)
        self.assertEqual(sys.stdout.getvalue().count('(y/N)'), 0)
        self.assertEqual(sys.stdout.getvalue().count(RUNNING_PY_CHECKS_TEXT), 1)

    def testDoDefaultPresubmitChecksAndFeedback(self):
        always_fail_presubmit_script = ("""\n
def CheckChangeOnUpload(input_api, output_api):
  return [output_api.PresubmitError("!!")]
def CheckChangeOnCommit(input_api, output_api):
  raise Exception("Test error")
""")

        os.path.isfile.return_value = False
        os.listdir.side_effect = (
            lambda d: [] if d == self.fake_root_dir else ['PRESUBMIT.py'])
        random.randint.return_value = 0

        change = self.ExampleChange(extra_lines=['STORY=http://tracker/123'])
        with mock.patch('sys.stdin', StringIO('y\n')):
            self.assertEqual(
                1,
                presubmit.DoPresubmitChecks(
                    change=change,
                    committing=False,
                    verbose=True,
                    default_presubmit=always_fail_presubmit_script,
                    may_prompt=False,
                    gerrit_obj=None,
                    json_output=None))
            text = (
                RUNNING_PY_CHECKS_TEXT + 'Warning, no PRESUBMIT.py found.\n'
                'Running default presubmit script.\n'
                '** Presubmit ERRORS: 1 **\n!!\n\n'
                'There were presubmit errors.\n'
                'Was the presubmit check useful? If not, run "git cl presubmit -v"\n'
                'to figure out which PRESUBMIT.py was run, then run "git blame"\n'
                'on the file to figure out who to ask for help.\n')
            self.assertEqual(sys.stdout.getvalue(), text)

    def ExampleChange(self, extra_lines=None):
        """Returns an example Change instance for tests."""
        description_lines = [
            'Hello there',
            'This is a change',
        ] + (extra_lines or [])
        files = [
            ['A', os.path.join('haspresubmit', 'blat.cc')],
        ]
        return presubmit.Change(name='mychange',
                                description='\n'.join(description_lines),
                                local_root=self.fake_root_dir,
                                files=files,
                                issue=0,
                                patchset=0,
                                author=None)

    def testMainPostUpload(self):
        os.path.isfile.side_effect = lambda f: 'PRESUBMIT.py' in f
        os.listdir.return_value = ['PRESUBMIT.py']
        gclient_utils.FileRead.return_value = (
            'def PostUploadHook(gerrit, change, output_api):\n'
            '  return ()\n')
        scm.determine_scm.return_value = None
        presubmit._parse_files.return_value = [('M', 'random_file.txt')]
        self.assertEqual(
            0,
            presubmit.main([
                '--root', self.fake_root_dir, 'random_file.txt', '--post_upload'
            ]))

    @mock.patch('presubmit_support.ListRelevantPresubmitFiles')
    def testMainUnversioned(self, *_mocks):
        gclient_utils.FileRead.return_value = ''
        scm.determine_scm.return_value = None
        presubmit.ListRelevantPresubmitFiles.return_value = [
            os.path.join(self.fake_root_dir, 'PRESUBMIT.py')
        ]

        self.assertEqual(
            0, presubmit.main(['--root', self.fake_root_dir,
                               'random_file.txt']))

    @mock.patch('presubmit_support.ListRelevantPresubmitFiles')
    def testMainUnversionedChecksFail(self, *_mocks):
        gclient_utils.FileRead.return_value = (
            'def CheckChangeOnUpload(input_api, output_api):\n'
            '  return [output_api.PresubmitError("!!")]\n')
        scm.determine_scm.return_value = None
        presubmit.ListRelevantPresubmitFiles.return_value = [
            os.path.join(self.fake_root_dir, 'PRESUBMIT.py')
        ]

        self.assertEqual(
            1, presubmit.main(['--root', self.fake_root_dir,
                               'random_file.txt']))

    def testMainUnversionedFail(self):
        scm.determine_scm.return_value = 'diff'

        with self.assertRaises(SystemExit) as e:
            presubmit.main(['--root', self.fake_root_dir])
        self.assertEqual(2, e.exception.code)

        self.assertEqual(
            sys.stderr.getvalue(),
            'usage: presubmit_unittest.py [options] <files...>\n'
            'presubmit_unittest.py: error: unversioned directories must '
            'specify <files>, <all_files>, or <diff_file>.\n')

    @mock.patch('presubmit_support.Change', mock.Mock())
    def testParseChange_Files(self):
        presubmit._parse_files.return_value = [('M', 'random_file.txt')]
        scm.determine_scm.return_value = None
        options = mock.Mock(all_files=False,
                            generate_diff=False,
                            source_controlled_only=False)

        change = presubmit._parse_change(None, options)
        self.assertEqual(presubmit.Change.return_value, change)
        presubmit.Change.assert_called_once_with(
            options.name, options.description, options.root,
            [('M', 'random_file.txt')], options.issue, options.patchset,
            options.author)
        presubmit._parse_files.assert_called_once_with(options.files,
                                                       options.recursive)

    def testParseChange_NoFilesAndDiff(self):
        presubmit._parse_files.return_value = []
        scm.determine_scm.return_value = 'diff'
        parser = mock.Mock()
        parser.error.side_effect = [SystemExit]
        options = mock.Mock(files=[], diff_file='', all_files=False)

        with self.assertRaises(SystemExit):
            presubmit._parse_change(parser, options)
        parser.error.assert_called_once_with(
            'unversioned directories must specify '
            '<files>, <all_files>, or <diff_file>.')

    def testParseChange_FilesAndAllFiles(self):
        parser = mock.Mock()
        parser.error.side_effect = [SystemExit]
        options = mock.Mock(files=['foo'], all_files=True)

        with self.assertRaises(SystemExit):
            presubmit._parse_change(parser, options)
        parser.error.assert_called_once_with(
            '<files> cannot be specified when --all-files is set.')

    def testParseChange_DiffAndAllFiles(self):
        parser = mock.Mock()
        parser.error.side_effect = [SystemExit]
        options = mock.Mock(files=[], all_files=True, diff_file='foo.diff')

        with self.assertRaises(SystemExit):
            presubmit._parse_change(parser, options)
        parser.error.assert_called_once_with(
            '<diff_file> cannot be specified when --all-files is set.')

    def testParseChange_DiffAndGenerateDiff(self):
        parser = mock.Mock()
        parser.error.side_effect = [SystemExit]
        options = mock.Mock(all_files=False,
                            files=[],
                            generate_diff=True,
                            diff_file='foo.diff')

        with self.assertRaises(SystemExit):
            presubmit._parse_change(parser, options)
        parser.error.assert_called_once_with(
            '<diff_file> cannot be specified when <generate_diff> is set.')

    @mock.patch('presubmit_support.GitChange', mock.Mock())
    def testParseChange_FilesAndGit(self):
        scm.determine_scm.return_value = 'git'
        presubmit._parse_files.return_value = [('M', 'random_file.txt')]
        options = mock.Mock(all_files=False,
                            generate_diff=False,
                            source_controlled_only=False)

        change = presubmit._parse_change(None, options)
        self.assertEqual(presubmit.GitChange.return_value, change)
        presubmit.GitChange.assert_called_once_with(
            options.name,
            options.description,
            options.root, [('M', 'random_file.txt')],
            options.issue,
            options.patchset,
            options.author,
            upstream=options.upstream,
            end_commit=options.end_commit)
        presubmit._parse_files.assert_called_once_with(options.files,
                                                       options.recursive)

    @mock.patch('presubmit_support.GitChange', mock.Mock())
    @mock.patch('scm.GIT.CaptureStatus', mock.Mock())
    def testParseChange_NoFilesAndGit(self):
        scm.determine_scm.return_value = 'git'
        scm.GIT.CaptureStatus.return_value = [('A', 'added.txt')]
        options = mock.Mock(all_files=False, files=[], diff_file='')

        change = presubmit._parse_change(None, options)
        self.assertEqual(presubmit.GitChange.return_value, change)
        presubmit.GitChange.assert_called_once_with(
            options.name,
            options.description,
            options.root, [('A', 'added.txt')],
            options.issue,
            options.patchset,
            options.author,
            upstream=options.upstream,
            end_commit=options.end_commit)
        scm.GIT.CaptureStatus.assert_called_once_with(options.root,
                                                      options.upstream,
                                                      ignore_submodules=False)

    @mock.patch('presubmit_support.GitChange', mock.Mock())
    @mock.patch('scm.GIT.GetAllFiles', mock.Mock())
    def testParseChange_AllFilesAndGit(self):
        scm.determine_scm.return_value = 'git'
        scm.GIT.GetAllFiles.return_value = ['foo.txt', 'bar.txt']
        options = mock.Mock(all_files=True, files=[], diff_file='')

        change = presubmit._parse_change(None, options)
        self.assertEqual(presubmit.GitChange.return_value, change)
        presubmit.GitChange.assert_called_once_with(
            options.name,
            options.description,
            options.root, [('M', 'foo.txt'), ('M', 'bar.txt')],
            options.issue,
            options.patchset,
            options.author,
            upstream=options.upstream,
            end_commit=options.end_commit)
        scm.GIT.GetAllFiles.assert_called_once_with(options.root)

    @mock.patch('presubmit_support.ProvidedDiffChange', mock.Mock())
    def testParseChange_ProvidedDiffFile(self):
        diff = """
diff --git a/foo b/foo
new file mode 100644
index 0000000..9daeafb
--- /dev/null
+++ b/foo
@@ -0,0 +1 @@
+test
diff --git a/bar b/bar
deleted file mode 100644
index f675c2a..0000000
--- a/bar
+++ /dev/null
@@ -1,1 +0,0 @@
-bar
diff --git a/baz b/baz
index d7ba659f..b7957f3 100644
--- a/baz
+++ b/baz
@@ -1,1 +1,1 @@
-baz
+bat"""
        gclient_utils.FileRead.return_value = diff
        options = mock.Mock(all_files=False,
                            files=[],
                            generate_diff=False,
                            diff_file='foo.diff')
        change = presubmit._parse_change(None, options)
        self.assertEqual(presubmit.ProvidedDiffChange.return_value, change)
        presubmit.ProvidedDiffChange.assert_called_once_with(
            options.name,
            options.description,
            options.root, [('A', 'foo'), ('D', 'bar'), ('M', 'baz')],
            options.issue,
            options.patchset,
            options.author,
            diff=diff)

    def testParseGerritOptions_NoGerritUrl(self):
        options = mock.Mock(gerrit_url=None,
                            gerrit_fetch=False,
                            author='author',
                            description='description')
        gerrit_obj = presubmit._parse_gerrit_options(None, options)

        self.assertIsNone(gerrit_obj)
        self.assertEqual('author', options.author)
        self.assertEqual('description', options.description)

    def testParseGerritOptions_NoGerritFetch(self):
        options = mock.Mock(
            gerrit_url='https://foo-review.googlesource.com/bar',
            gerrit_project='project',
            gerrit_branch='refs/heads/main',
            gerrit_fetch=False,
            author='author',
            description='description')

        gerrit_obj = presubmit._parse_gerrit_options(None, options)

        self.assertEqual('foo-review.googlesource.com', gerrit_obj.host)
        self.assertEqual('project', gerrit_obj.project)
        self.assertEqual('refs/heads/main', gerrit_obj.branch)
        self.assertEqual('author', options.author)
        self.assertEqual('description', options.description)

    @mock.patch('presubmit_support.GerritAccessor.GetChangeOwner')
    @mock.patch('presubmit_support.GerritAccessor.GetChangeDescription')
    def testParseGerritOptions_GerritFetch(self, mockDescription, mockOwner):
        mockDescription.return_value = 'new description'
        mockOwner.return_value = 'new owner'

        options = mock.Mock(
            gerrit_url='https://foo-review.googlesource.com/bar',
            gerrit_project='project',
            gerrit_branch='refs/heads/main',
            gerrit_fetch=True,
            issue=123,
            patchset=4)

        gerrit_obj = presubmit._parse_gerrit_options(None, options)

        self.assertEqual('foo-review.googlesource.com', gerrit_obj.host)
        self.assertEqual('project', gerrit_obj.project)
        self.assertEqual('refs/heads/main', gerrit_obj.branch)
        self.assertEqual('new owner', options.author)
        self.assertEqual('new description', options.description)

    def testParseGerritOptions_GerritFetchNoUrl(self):
        parser = mock.Mock()
        parser.error.side_effect = [SystemExit]
        options = mock.Mock(gerrit_url=None,
                            gerrit_fetch=True,
                            issue=123,
                            patchset=4)

        with self.assertRaises(SystemExit):
            presubmit._parse_gerrit_options(parser, options)
        parser.error.assert_called_once_with(
            '--gerrit_fetch requires --gerrit_url, --issue and --patchset.')

    def testParseGerritOptions_GerritFetchNoIssue(self):
        parser = mock.Mock()
        parser.error.side_effect = [SystemExit]
        options = mock.Mock(gerrit_url='https://example.com',
                            gerrit_fetch=True,
                            issue=None,
                            patchset=4)

        with self.assertRaises(SystemExit):
            presubmit._parse_gerrit_options(parser, options)
        parser.error.assert_called_once_with(
            '--gerrit_fetch requires --gerrit_url, --issue and --patchset.')

    def testParseGerritOptions_GerritFetchNoPatchset(self):
        parser = mock.Mock()
        parser.error.side_effect = [SystemExit]
        options = mock.Mock(gerrit_url='https://example.com',
                            gerrit_fetch=True,
                            issue=123,
                            patchset=None)

        with self.assertRaises(SystemExit):
            presubmit._parse_gerrit_options(parser, options)
        parser.error.assert_called_once_with(
            '--gerrit_fetch requires --gerrit_url, --issue and --patchset.')


class InputApiUnittest(PresubmitTestsBase):
    """Tests presubmit.InputApi."""
    def testInputApiConstruction(self):
        api = presubmit.InputApi(self.fake_change,
                                 presubmit_path='foo/path/PRESUBMIT.py',
                                 is_committing=False,
                                 gerrit_obj=None,
                                 verbose=False)
        self.assertEqual(api.PresubmitLocalPath(), 'foo/path')
        self.assertEqual(api.change, self.fake_change)

    def testInputApiPresubmitScriptFiltering(self):
        description_lines = ('Hello there', 'this is a change', 'BUG=123',
                             ' STORY =http://foo/  \t',
                             'and some more regular text')
        files = [
            ['A', os.path.join('foo', 'blat.cc'), True],
            ['M', os.path.join('foo', 'blat', 'READ_ME2'), True],
            ['M', os.path.join('foo', 'blat', 'binary.dll'), True],
            ['M', os.path.join('foo', 'blat', 'weird.xyz'), True],
            ['M', os.path.join('foo', 'blat', 'another.h'), True],
            ['M', os.path.join('foo', 'third_party', 'third.cc'), True],
            ['D', os.path.join('foo', 'mat', 'beingdeleted.txt'), False],
            ['M', os.path.join('flop', 'notfound.txt'), False],
            ['A', os.path.join('boo', 'flap.h'), True],
        ]
        diffs = []
        known_files = []
        for _, f, exists in files:
            full_file = os.path.join(self.fake_root_dir, f)
            if exists and f.startswith('foo'):
                known_files.append(full_file)
            diffs.append(self.presubmit_diffs % {'filename': f})

        os.path.isfile.side_effect = lambda f: f in known_files
        presubmit.scm.GIT.GenerateDiff.return_value = '\n'.join(diffs)

        change = presubmit.GitChange('mychange',
                                     '\n'.join(description_lines),
                                     self.fake_root_dir,
                                     [[f[0], f[1]] for f in files],
                                     0,
                                     0,
                                     None,
                                     upstream='upstream',
                                     end_commit='HEAD')
        input_api = presubmit.InputApi(
            change, os.path.join(self.fake_root_dir, 'foo', 'PRESUBMIT.py'),
            False, None, False)
        # Doesn't filter much
        got_files = input_api.AffectedFiles()
        self.assertEqual(len(got_files), 7)
        self.assertEqual(got_files[0].LocalPath(),
                         presubmit.normpath(files[0][1]))
        self.assertEqual(got_files[1].LocalPath(),
                         presubmit.normpath(files[1][1]))
        self.assertEqual(got_files[2].LocalPath(),
                         presubmit.normpath(files[2][1]))
        self.assertEqual(got_files[3].LocalPath(),
                         presubmit.normpath(files[3][1]))
        self.assertEqual(got_files[4].LocalPath(),
                         presubmit.normpath(files[4][1]))
        self.assertEqual(got_files[5].LocalPath(),
                         presubmit.normpath(files[5][1]))
        self.assertEqual(got_files[6].LocalPath(),
                         presubmit.normpath(files[6][1]))
        # Ignores weird because of check_list, third_party because of skip_list,
        # binary isn't a text file and being deleted doesn't exist. The rest is
        # outside foo/.
        rhs_lines = list(input_api.RightHandSideLines(None))
        self.assertEqual(len(rhs_lines), 14)
        self.assertEqual(rhs_lines[0][0].LocalPath(),
                         presubmit.normpath(files[0][1]))
        self.assertEqual(rhs_lines[3][0].LocalPath(),
                         presubmit.normpath(files[0][1]))
        self.assertEqual(rhs_lines[7][0].LocalPath(),
                         presubmit.normpath(files[4][1]))
        self.assertEqual(rhs_lines[13][0].LocalPath(),
                         presubmit.normpath(files[4][1]))

    def testInputApiFilterSourceFile(self):
        files = [
            ['A', os.path.join('foo', 'blat.cc')],
            ['M', os.path.join('foo', 'blat', 'READ_ME2')],
            ['M', os.path.join('foo', 'blat', 'binary.dll')],
            ['M', os.path.join('foo', 'blat', 'weird.xyz')],
            ['M', os.path.join('foo', 'blat', 'another.h')],
            ['M',
             os.path.join('foo', 'third_party', 'WebKit', 'WebKit.cpp')],
            ['M',
             os.path.join('foo', 'third_party', 'WebKit2', 'WebKit2.cpp')],
            ['M', os.path.join('foo', 'third_party', 'blink', 'blink.cc')],
            ['M',
             os.path.join('foo', 'third_party', 'blink1', 'blink1.cc')],
            ['M', os.path.join('foo', 'third_party', 'third', 'third.cc')],
        ]
        known_files = [os.path.join(self.fake_root_dir, f) for _, f in files]
        os.path.isfile.side_effect = lambda f: f in known_files

        change = presubmit.GitChange('mychange',
                                     'description\nlines\n',
                                     self.fake_root_dir,
                                     [[f[0], f[1]] for f in files],
                                     0,
                                     0,
                                     None,
                                     upstream='upstream',
                                     end_commit='HEAD')
        input_api = presubmit.InputApi(
            change, os.path.join(self.fake_root_dir, 'foo', 'PRESUBMIT.py'),
            False, None, False)
        # We'd like to test FilterSourceFile, which is used by
        # AffectedSourceFiles(None).
        got_files = input_api.AffectedSourceFiles(None)
        self.assertEqual(len(got_files), 4)
        # blat.cc, another.h, WebKit.cpp, and blink.cc remain.
        self.assertEqual(got_files[0].LocalPath(),
                         presubmit.normpath(files[0][1]))
        self.assertEqual(got_files[1].LocalPath(),
                         presubmit.normpath(files[4][1]))
        self.assertEqual(got_files[2].LocalPath(),
                         presubmit.normpath(files[5][1]))
        self.assertEqual(got_files[3].LocalPath(),
                         presubmit.normpath(files[7][1]))

    def testDefaultFilesToCheckFilesToSkipFilters(self):
        def f(x):
            return presubmit.AffectedFile(x, 'M', self.fake_root_dir, None)

        files = [
            (
                [
                    # To be tested.
                    f('testing_support/google_appengine/b'),
                    f('testing_support/not_google_appengine/foo.cc'),
                ],
                [
                    # Expected.
                    'testing_support/not_google_appengine/foo.cc',
                ],
            ),
            (
                [
                    # To be tested.
                    f('a/experimental/b'),
                    f('experimental/b'),
                    f('a/experimental'),
                    f('a/experimental.cc'),
                    f('a/experimental.S'),
                ],
                [
                    # Expected.
                    'a/experimental.cc',
                    'a/experimental.S',
                ],
            ),
            (
                [
                    # To be tested.
                    f('a/third_party/b'),
                    f('third_party/b'),
                    f('a/third_party'),
                    f('a/third_party.cc'),
                ],
                [
                    # Expected.
                    'a/third_party.cc',
                ],
            ),
            (
                [
                    # To be tested.
                    f('a/LOL_FILE/b'),
                    f('b.c/LOL_FILE'),
                    f('a/PRESUBMIT.py'),
                    f('a/FOO.json'),
                    f('a/FOO.java'),
                    f('a/FOO.mojom'),
                ],
                [
                    # Expected.
                    'a/PRESUBMIT.py',
                    'a/FOO.java',
                    'a/FOO.mojom',
                ],
            ),
            (
                [
                    # To be tested.
                    f('a/.git'),
                    f('b.c/.git'),
                    f('a/.git/bleh.py'),
                    f('.git/bleh.py'),
                    f('bleh.diff'),
                    f('foo/bleh.patch'),
                ],
                [
                    # Expected.
                ],
            ),
        ]
        input_api = presubmit.InputApi(self.fake_change, './PRESUBMIT.py',
                                       False, None, False)

        for item in files:
            results = list(filter(input_api.FilterSourceFile, item[0]))
            for i in range(len(results)):
                self.assertEqual(results[i].LocalPath(),
                                 presubmit.normpath(item[1][i]))
            # Same number of expected results.
            self.assertEqual(
                sorted([f.LocalPath().replace(os.sep, '/') for f in results]),
                sorted(item[1]))

    def testDefaultOverrides(self):
        input_api = presubmit.InputApi(self.fake_change, './PRESUBMIT.py',
                                       False, None, False)
        self.assertEqual(len(input_api.DEFAULT_FILES_TO_CHECK), 26)
        self.assertEqual(len(input_api.DEFAULT_FILES_TO_SKIP), 12)

        input_api.DEFAULT_FILES_TO_CHECK = (r'.+\.c$', )
        input_api.DEFAULT_FILES_TO_SKIP = (r'.+\.patch$', r'.+\.diff')
        self.assertEqual(len(input_api.DEFAULT_FILES_TO_CHECK), 1)
        self.assertEqual(len(input_api.DEFAULT_FILES_TO_SKIP), 2)

    def testCustomFilter(self):
        def FilterSourceFile(affected_file):
            return 'a' in affected_file.LocalPath()

        files = [('A', 'eeaee'), ('M', 'eeabee'), ('M', 'eebcee')]
        known_files = [
            os.path.join(self.fake_root_dir, item) for _, item in files
        ]
        os.path.isfile.side_effect = lambda f: f in known_files

        change = presubmit.GitChange('mychange',
                                     '',
                                     self.fake_root_dir,
                                     files,
                                     0,
                                     0,
                                     None,
                                     upstream='upstream',
                                     end_commit='HEAD')
        input_api = presubmit.InputApi(
            change, os.path.join(self.fake_root_dir, 'PRESUBMIT.py'), False,
            None, False)
        got_files = input_api.AffectedSourceFiles(FilterSourceFile)
        self.assertEqual(len(got_files), 2)
        self.assertEqual(got_files[0].LocalPath(), 'eeaee')
        self.assertEqual(got_files[1].LocalPath(), 'eeabee')

    def testLambdaFilter(self):
        files_to_check = presubmit.InputApi.DEFAULT_FILES_TO_SKIP + (
            r".*?a.*?", )
        files_to_skip = [r".*?b.*?"]
        files = [('A', 'eeaee'), ('M', 'eeabee'), ('M', 'eebcee'),
                 ('M', 'eecaee')]
        known_files = [
            os.path.join(self.fake_root_dir, item) for _, item in files
        ]
        os.path.isfile.side_effect = lambda f: f in known_files

        change = presubmit.GitChange('mychange',
                                     '',
                                     self.fake_root_dir,
                                     files,
                                     0,
                                     0,
                                     None,
                                     upstream='upstream',
                                     end_commit='HEAD')
        input_api = presubmit.InputApi(
            change, os.path.join(self.fake_root_dir, 'PRESUBMIT.py'), False,
            None, False)
        # Sample usage of overriding the default white and black lists.
        got_files = input_api.AffectedSourceFiles(
            lambda x: input_api.FilterSourceFile(x, files_to_check,
                                                 files_to_skip))

        self.assertEqual(len(got_files), 2)
        self.assertEqual(got_files[0].LocalPath(), 'eeaee')
        self.assertEqual(got_files[1].LocalPath(), 'eecaee')

    def testGetAbsoluteLocalPath(self):
        normpath = presubmit.normpath
        # Regression test for bug of presubmit stuff that relies on invoking
        # SVN (e.g. to get mime type of file) not working unless gcl invoked
        # from the client root (e.g. if you were at 'src' and did 'cd base'
        # before invoking 'gcl upload' it would fail because svn wouldn't find
        # the files the presubmit script was asking about).
        files = [
            ['A', 'isdir'],
            ['A', os.path.join('isdir', 'blat.cc')],
            ['M', os.path.join('elsewhere', 'ouf.cc')],
        ]

        change = presubmit.Change('mychange', '', self.fake_root_dir, files, 0,
                                  0, None)
        affected_files = change.AffectedFiles()
        # Validate that normpath strips trailing path separators.
        self.assertEqual('isdir', normpath('isdir/'))
        # Local paths should remain the same
        self.assertEqual(affected_files[0].LocalPath(), normpath('isdir'))
        self.assertEqual(affected_files[1].LocalPath(),
                         normpath('isdir/blat.cc'))
        # Absolute paths should be prefixed
        self.assertEqual(
            affected_files[0].AbsoluteLocalPath(),
            presubmit.normpath(os.path.join(self.fake_root_dir, 'isdir')))
        self.assertEqual(
            affected_files[1].AbsoluteLocalPath(),
            presubmit.normpath(os.path.join(self.fake_root_dir,
                                            'isdir/blat.cc')))

        # New helper functions need to work
        paths_from_change = change.AbsoluteLocalPaths()
        self.assertEqual(len(paths_from_change), 3)
        presubmit_path = os.path.join(self.fake_root_dir, 'isdir',
                                      'PRESUBMIT.py')
        api = presubmit.InputApi(change=change,
                                 presubmit_path=presubmit_path,
                                 is_committing=True,
                                 gerrit_obj=None,
                                 verbose=False)
        paths_from_api = api.AbsoluteLocalPaths()
        self.assertEqual(len(paths_from_api), 1)
        self.assertEqual(
            paths_from_change[0],
            presubmit.normpath(os.path.join(self.fake_root_dir, 'isdir')))
        self.assertEqual(
            paths_from_change[1],
            presubmit.normpath(
                os.path.join(self.fake_root_dir, 'isdir', 'blat.cc')))
        self.assertEqual(
            paths_from_api[0],
            presubmit.normpath(
                os.path.join(self.fake_root_dir, 'isdir', 'blat.cc')))

    def testDeprecated(self):
        change = presubmit.Change('mychange', '', self.fake_root_dir, [], 0, 0,
                                  None)
        api = presubmit.InputApi(
            change, os.path.join(self.fake_root_dir, 'foo', 'PRESUBMIT.py'),
            True, None, False)
        api.AffectedTestableFiles(include_deletes=False)

    def testReadFileStringDenied(self):

        change = presubmit.Change('foo', 'foo', self.fake_root_dir,
                                  [('M', 'AA')], 0, 0, None)
        input_api = presubmit.InputApi(change,
                                       os.path.join(self.fake_root_dir, '/p'),
                                       False, None, False)
        self.assertRaises(IOError, input_api.ReadFile, 'boo', 'x')

    def testReadFileStringAccepted(self):
        path = os.path.join(self.fake_root_dir, 'AA/boo')
        presubmit.gclient_utils.FileRead.return_code = None

        change = presubmit.Change('foo', 'foo', self.fake_root_dir,
                                  [('M', 'AA')], 0, 0, None)
        input_api = presubmit.InputApi(change,
                                       os.path.join(self.fake_root_dir, '/p'),
                                       False, None, False)
        input_api.ReadFile(path, 'x')

    def testReadFileAffectedFileDenied(self):
        fileobj = presubmit.AffectedFile('boo',
                                         'M',
                                         'Unrelated',
                                         diff_cache=mock.Mock())

        change = presubmit.Change('foo', 'foo', self.fake_root_dir,
                                  [('M', 'AA')], 0, 0, None)
        input_api = presubmit.InputApi(change,
                                       os.path.join(self.fake_root_dir, '/p'),
                                       False, None, False)
        self.assertRaises(IOError, input_api.ReadFile, fileobj, 'x')

    def testReadFileAffectedFileAccepted(self):
        fileobj = presubmit.AffectedFile('AA/boo',
                                         'M',
                                         self.fake_root_dir,
                                         diff_cache=mock.Mock())
        presubmit.gclient_utils.FileRead.return_code = None

        change = presubmit.Change('foo', 'foo', self.fake_root_dir,
                                  [('M', 'AA')], 0, 0, None)
        input_api = presubmit.InputApi(change,
                                       os.path.join(self.fake_root_dir, '/p'),
                                       False, None, False)
        input_api.ReadFile(fileobj, 'x')

    def testCreateTemporaryFile(self):
        input_api = presubmit.InputApi(self.fake_change,
                                       presubmit_path='foo/path/PRESUBMIT.py',
                                       is_committing=False,
                                       gerrit_obj=None,
                                       verbose=False)
        tempfile.NamedTemporaryFile.side_effect = [
            MockTemporaryFile('foo'),
            MockTemporaryFile('bar')
        ]

        self.assertEqual(0, len(input_api._named_temporary_files))
        with input_api.CreateTemporaryFile():
            self.assertEqual(1, len(input_api._named_temporary_files))
        self.assertEqual(['foo'], input_api._named_temporary_files)
        with input_api.CreateTemporaryFile():
            self.assertEqual(2, len(input_api._named_temporary_files))
        self.assertEqual(2, len(input_api._named_temporary_files))
        self.assertEqual(['foo', 'bar'], input_api._named_temporary_files)

        self.assertRaises(TypeError, input_api.CreateTemporaryFile, delete=True)
        self.assertRaises(TypeError,
                          input_api.CreateTemporaryFile,
                          delete=False)
        self.assertEqual(['foo', 'bar'], input_api._named_temporary_files)


class OutputApiUnittest(PresubmitTestsBase):
    """Tests presubmit.OutputApi."""
    def testOutputApiBasics(self):
        self.assertIsNotNone(presubmit.OutputApi.PresubmitError('').fatal)
        self.assertFalse(presubmit.OutputApi.PresubmitError('').should_prompt)

        self.assertFalse(presubmit.OutputApi.PresubmitPromptWarning('').fatal)
        self.assertIsNotNone(
            presubmit.OutputApi.PresubmitPromptWarning('').should_prompt)

        self.assertFalse(presubmit.OutputApi.PresubmitNotifyResult('').fatal)
        self.assertFalse(
            presubmit.OutputApi.PresubmitNotifyResult('').should_prompt)

        # TODO(joi) Test MailTextResult once implemented.

    def testAppendCC(self):
        output_api = presubmit.OutputApi(False)
        output_api.AppendCC('chromium-reviews@chromium.org')
        self.assertEqual(['chromium-reviews@chromium.org'], output_api.more_cc)

    def testAppendCCAndMultipleChecks(self):
        description_lines = ('Hello there', 'this is a change', 'BUG=123')
        files = [
            ['A', 'foo\\blat.cc'],
        ]
        fake_presubmit = os.path.join(self.fake_root_dir, 'PRESUBMIT.py')

        change = presubmit.Change('mychange', '\n'.join(description_lines),
                                  self.fake_root_dir, files, 0, 0, None)

        # Base case: AppendCC from multiple different checks should be reflected
        # in the final result.
        executer = presubmit.PresubmitExecuter(change, True, None,
                                               presubmit.GerritAccessor())
        self.assertFalse(
            executer.ExecPresubmitScript(
                "PRESUBMIT_VERSION = '2.0.0'\n"
                'def CheckChangeAddCC1(input_api, output_api):\n'
                "  output_api.AppendCC('chromium-reviews@chromium.org')\n"
                '  return []\n'
                '\n'
                'def CheckChangeAppendCC2(input_api, output_api):\n'
                "  output_api.AppendCC('ipc-security-reviews@chromium.org')\n"
                '  return []\n', fake_presubmit))
        self.assertEqual([
            'chromium-reviews@chromium.org', 'ipc-security-reviews@chromium.org'
        ], executer.more_cc)

        # Check that if one presubmit check appends a CC, it does not get
        # duplicated into the more CC list by subsequent checks.
        executer = presubmit.PresubmitExecuter(change, True, None,
                                               presubmit.GerritAccessor())
        self.assertFalse(
            executer.ExecPresubmitScript(
                "PRESUBMIT_VERSION = '2.0.0'\n"
                'def CheckChangeAddCC(input_api, output_api):\n'
                "  output_api.AppendCC('chromium-reviews@chromium.org')\n"
                '  return []\n'
                '\n'
                'def CheckChangeDoNothing(input_api, output_api):\n'
                '  return []\n', fake_presubmit))
        self.assertEqual(['chromium-reviews@chromium.org'], executer.more_cc)

        # Check that if multiple presubmit checks append the same CC, it gets
        # deduplicated.
        executer = presubmit.PresubmitExecuter(change, True, None,
                                               presubmit.GerritAccessor())
        self.assertFalse(
            executer.ExecPresubmitScript(
                "PRESUBMIT_VERSION = '2.0.0'\n"
                'def CheckChangeAddCC1(input_api, output_api):\n'
                "  output_api.AppendCC('chromium-reviews@chromium.org')\n"
                '  return []\n'
                '\n'
                'def CheckChangeAppendCC2(input_api, output_api):\n'
                "  output_api.AppendCC('chromium-reviews@chromium.org')\n"
                '  return []\n', fake_presubmit))
        self.assertEqual(['chromium-reviews@chromium.org'], executer.more_cc)

    def testOutputApiHandling(self):
        presubmit.OutputApi.PresubmitError('!!!').handle()
        self.assertIsNotNone(sys.stdout.getvalue().count('!!!'))

        sys.stdout.truncate(0)
        presubmit.OutputApi.PresubmitNotifyResult('?see?').handle()
        self.assertIsNotNone(sys.stdout.getvalue().count('?see?'))

        sys.stdout.truncate(0)
        presubmit.OutputApi.PresubmitPromptWarning('???').handle()
        self.assertIsNotNone(sys.stdout.getvalue().count('???'))

        sys.stdout.truncate(0)
        output_api = presubmit.OutputApi(True)
        output_api.PresubmitPromptOrNotify('???').handle()
        self.assertIsNotNone(sys.stdout.getvalue().count('???'))

        sys.stdout.truncate(0)
        output_api = presubmit.OutputApi(False)
        output_api.PresubmitPromptOrNotify('???').handle()
        self.assertIsNotNone(sys.stdout.getvalue().count('???'))


class AffectedFileUnittest(PresubmitTestsBase):
    def testAffectedFile(self):
        gclient_utils.FileRead.return_value = 'whatever\ncookie'
        af = presubmit.GitAffectedFile('foo/blat.cc', 'M', self.fake_root_dir,
                                       None)
        self.assertEqual(presubmit.normpath('foo/blat.cc'), af.LocalPath())
        self.assertEqual('M', af.Action())
        self.assertEqual(['whatever', 'cookie'], af.NewContents())

    def testAffectedFileNotExists(self):
        notfound = 'notfound.cc'
        gclient_utils.FileRead.side_effect = IOError
        af = presubmit.AffectedFile(notfound, 'A', self.fake_root_dir, None)
        self.assertEqual([], af.NewContents())

    def testIsTestableFile(self):
        files = [
            presubmit.GitAffectedFile('foo/blat.txt', 'M', self.fake_root_dir,
                                      None),
            presubmit.GitAffectedFile('foo/binary.blob', 'M',
                                      self.fake_root_dir, None),
            presubmit.GitAffectedFile('blat/flop.txt', 'D', self.fake_root_dir,
                                      None)
        ]
        blat = os.path.join('foo', 'blat.txt')
        blob = os.path.join('foo', 'binary.blob')
        f_blat = os.path.join(self.fake_root_dir, blat)
        f_blob = os.path.join(self.fake_root_dir, blob)
        os.path.isfile.side_effect = lambda f: f in [f_blat, f_blob]

        output = list(filter(lambda x: x.IsTestableFile(), files))
        self.assertEqual(2, len(output))
        self.assertEqual(files[:2], output[:2])


class ChangeUnittest(PresubmitTestsBase):

    def testAffectedFiles(self):
        change = presubmit.Change('', '', self.fake_root_dir, [('Y', 'AA'),
                                                               ('A', 'BB')], 3,
                                  5, '')
        self.assertEqual(2, len(change.AffectedFiles()))
        self.assertEqual('Y', change.AffectedFiles()[0].Action())

    @mock.patch('scm.GIT.ListSubmodules', return_value=['BB'])
    def testAffectedSubmodules(self, mockListSubmodules):
        change = presubmit.GitChange('',
                                     '',
                                     self.fake_root_dir, [('Y', 'AA'),
                                                          ('A', 'BB')],
                                     3,
                                     5,
                                     '',
                                     upstream='upstream',
                                     end_commit='HEAD')
        self.assertEqual(1, len(change.AffectedSubmodules()))
        self.assertEqual('A', change.AffectedSubmodules()[0].Action())

    @mock.patch('scm.GIT.ListSubmodules', return_value=['BB'])
    def testAffectedSubmodulesCachesSubmodules(self, mockListSubmodules):
        change = presubmit.GitChange('',
                                     '',
                                     self.fake_root_dir, [('Y', 'AA'),
                                                          ('A', 'BB')],
                                     3,
                                     5,
                                     '',
                                     upstream='upstream',
                                     end_commit='HEAD')
        change.AffectedSubmodules()
        mockListSubmodules.assert_called_once()
        change.AffectedSubmodules()
        mockListSubmodules.assert_called_once()

    def testSetDescriptionText(self):
        change = presubmit.Change('', 'foo\nDRU=ro', self.fake_root_dir, [], 3,
                                  5, '')
        self.assertEqual('foo', change.DescriptionText())
        self.assertEqual('foo\nDRU=ro', change.FullDescriptionText())
        self.assertEqual({'DRU': 'ro'}, change.tags)

        change.SetDescriptionText('WHIZ=bang\nbar\nFOO=baz')
        self.assertEqual('bar', change.DescriptionText())
        self.assertEqual('WHIZ=bang\nbar\nFOO=baz',
                         change.FullDescriptionText())
        self.assertEqual({'WHIZ': 'bang', 'FOO': 'baz'}, change.tags)

    def testAddDescriptionFooter(self):
        change = presubmit.Change('', 'foo\nDRU=ro\n\nChange-Id: asdf',
                                  self.fake_root_dir, [], 3, 5, '')
        change.AddDescriptionFooter('my-footer', 'my-value')
        self.assertEqual('foo\nDRU=ro\n\nChange-Id: asdf\nMy-Footer: my-value',
                         change.FullDescriptionText())

    def testAddDescriptionFooter_NoPreviousFooters(self):
        change = presubmit.Change('', 'foo\nDRU=ro', self.fake_root_dir, [], 3,
                                  5, '')
        change.AddDescriptionFooter('my-footer', 'my-value')
        self.assertEqual('foo\nDRU=ro\n\nMy-Footer: my-value',
                         change.FullDescriptionText())

    def testAddDescriptionFooter_InvalidFooter(self):
        change = presubmit.Change('', 'foo\nDRU=ro', self.fake_root_dir, [], 3,
                                  5, '')
        with self.assertRaises(ValueError):
            change.AddDescriptionFooter('invalid.characters in:the',
                                        'footer key')

    def testGitFootersFromDescription(self):
        change = presubmit.Change(
            '', 'foo\n\nChange-Id: asdf\nBug: 1\nBug: 2\nNo-Try: True',
            self.fake_root_dir, [], 0, 0, '')
        self.assertEqual(
            {
                'Change-Id': ['asdf'],
                'Bug': ['2', '1'],
                'No-Try': ['True'],
            }, change.GitFootersFromDescription())

    def testGitFootersFromDescription_NoFooters(self):
        change = presubmit.Change('', 'foo', self.fake_root_dir, [], 0, 0, '')
        self.assertEqual({}, change.GitFootersFromDescription())

    def testBugFromDescription_FixedAndBugGetDeduped(self):
        change = presubmit.Change(
            '', 'foo\n\nChange-Id: asdf\nBug: 1, 2\nFixed:2, 1 ',
            self.fake_root_dir, [], 0, 0, '')
        self.assertEqual(['1', '2'], change.BugsFromDescription())
        self.assertEqual('1,2', change.BUG)

    def testBugsFromDescription_MixedTagsAndFooters(self):
        change = presubmit.Change('',
                                  'foo\nBUG=2,1\n\nChange-Id: asdf\nBug: 3, 6',
                                  self.fake_root_dir, [], 0, 0, '')
        self.assertEqual(['1', '2', '3', '6'], change.BugsFromDescription())
        self.assertEqual('1,2,3,6', change.BUG)

    def testBugsFromDescription_MultipleFooters(self):
        change = presubmit.Change(
            '', 'foo\n\nChange-Id: asdf\nBug: 1\nBug:4,  6\nFixed: 7',
            self.fake_root_dir, [], 0, 0, '')
        self.assertEqual(['1', '4', '6', '7'], change.BugsFromDescription())
        self.assertEqual('1,4,6,7', change.BUG)

    def testBugFromDescription_OnlyFixed(self):
        change = presubmit.Change('', 'foo\n\nChange-Id: asdf\nFixed:1, 2',
                                  self.fake_root_dir, [], 0, 0, '')
        self.assertEqual(['1', '2'], change.BugsFromDescription())
        self.assertEqual('1,2', change.BUG)

    def testReviewersFromDescription(self):
        change = presubmit.Change('',
                                  'foo\nR=foo,bar\n\nChange-Id: asdf\nR: baz',
                                  self.fake_root_dir, [], 0, 0, '')
        self.assertEqual(['bar', 'foo'], change.ReviewersFromDescription())
        self.assertEqual('bar,foo', change.R)

    def testTBRsFromDescription(self):
        change = presubmit.Change(
            '', 'foo\nTBR=foo,bar\n\nChange-Id: asdf\nTBR: baz',
            self.fake_root_dir, [], 0, 0, '')
        self.assertEqual(['bar', 'baz', 'foo'], change.TBRsFromDescription())
        self.assertEqual('bar,baz,foo', change.TBR)


class CannedChecksUnittest(PresubmitTestsBase):
    """Tests presubmit_canned_checks.py."""

    def MockInputApi(self, change, committing, gerrit=None):
        # pylint: disable=no-self-use
        input_api = mock.MagicMock(presubmit.InputApi)
        input_api.thread_pool = presubmit.ThreadPool()
        input_api.parallel = False
        input_api.json = presubmit.json
        input_api.logging = logging
        input_api.os_listdir = mock.Mock()
        input_api.os_walk = mock.Mock()
        input_api.os_path = os.path
        input_api.re = presubmit.re
        input_api.gerrit = gerrit
        input_api.urllib_request = mock.MagicMock(presubmit.urllib_request)
        input_api.urllib_error = mock.MagicMock(presubmit.urllib_error)
        input_api.unittest = unittest
        input_api.subprocess = subprocess
        input_api.sys = sys

        class fake_CalledProcessError(Exception):
            def __str__(self):
                return 'foo'

        input_api.subprocess.CalledProcessError = fake_CalledProcessError
        input_api.verbose = False
        input_api.is_windows = False
        input_api.no_diffs = False

        input_api.change = change
        input_api.is_committing = committing
        input_api.tbr = False
        input_api.dry_run = None
        input_api.python_executable = 'pyyyyython'
        input_api.python3_executable = 'pyyyyython3'
        input_api.platform = sys.platform
        input_api.cpu_count = 2
        input_api.time = time
        input_api.canned_checks = presubmit_canned_checks
        input_api.Command = presubmit.CommandData
        input_api.RunTests = functools.partial(presubmit.InputApi.RunTests,
                                               input_api)
        return input_api

    def DescriptionTest(self, check, description1, description2, error_type,
                        committing):
        change1 = presubmit.Change('foo1', description1, self.fake_root_dir,
                                   None, 0, 0, None)
        input_api1 = self.MockInputApi(change1, committing)
        change2 = presubmit.Change('foo2', description2, self.fake_root_dir,
                                   None, 0, 0, None)
        input_api2 = self.MockInputApi(change2, committing)

        results1 = check(input_api1, presubmit.OutputApi)
        self.assertEqual(results1, [])
        results2 = check(input_api2, presubmit.OutputApi)
        self.assertEqual(len(results2), 1)
        self.assertTrue(isinstance(results2[0], error_type))

    def ContentTest(self, check, content1, content1_path, content2,
                    content2_path, error_type):
        """Runs a test of a content-checking rule.

        Args:
            check: the check to run.
            content1: content which is expected to pass the check.
            content1_path: file path for content1.
            content2: content which is expected to fail the check.
            content2_path: file path for content2.
            error_type: the type of the error expected for content2.
        """
        change1 = presubmit.Change('foo1', 'foo1\n', self.fake_root_dir, None,
                                   0, 0, None)
        input_api1 = self.MockInputApi(change1, False)
        affected_file1 = mock.MagicMock(presubmit.GitAffectedFile)
        input_api1.AffectedFiles.return_value = [affected_file1]
        affected_file1.LocalPath.return_value = content1_path
        affected_file1.NewContents.return_value = [
            'afoo', content1, 'bfoo', 'cfoo', 'dfoo'
        ]
        # It falls back to ChangedContents when there is a failure. This is an
        # optimization since NewContents() is much faster to execute than
        # ChangedContents().
        affected_file1.ChangedContents.return_value = [(42, content1),
                                                       (43, 'hfoo'),
                                                       (23, 'ifoo')]

        change2 = presubmit.Change('foo2', 'foo2\n', self.fake_root_dir, None,
                                   0, 0, None)
        input_api2 = self.MockInputApi(change2, False)

        affected_file2 = mock.MagicMock(presubmit.GitAffectedFile)
        input_api2.AffectedFiles.return_value = [affected_file2]
        affected_file2.LocalPath.return_value = content2_path
        affected_file2.NewContents.return_value = [
            'dfoo', content2, 'efoo', 'ffoo', 'gfoo'
        ]
        affected_file2.ChangedContents.return_value = [(42, content2),
                                                       (43, 'hfoo'),
                                                       (23, 'ifoo')]

        results1 = check(input_api1, presubmit.OutputApi, None)
        self.assertEqual(results1, [])
        results2 = check(input_api2, presubmit.OutputApi, None)
        self.assertEqual(len(results2), 1)
        self.assertEqual(results2[0].__class__, error_type)

    def PythonLongLineTest(self, maxlen, content, should_pass):
        """Runs a test of Python long-line checking rule.

        Because ContentTest() cannot be used here due to the different code path
        that the implementation of CheckLongLines() uses for Python files.

        Args:
            maxlen: Maximum line length for content.
            content: Python source which is expected to pass or fail the test.
            should_pass: True iff the test should pass, False otherwise.
        """
        change = presubmit.Change('foo1', 'foo1\n', self.fake_root_dir, None, 0,
                                  0, None)
        input_api = self.MockInputApi(change, False)
        affected_file = mock.MagicMock(presubmit.GitAffectedFile)
        input_api.AffectedFiles.return_value = [affected_file]
        affected_file.LocalPath.return_value = 'foo.py'
        affected_file.NewContents.return_value = content.splitlines()

        results = presubmit_canned_checks.CheckLongLines(
            input_api, presubmit.OutputApi, maxlen)
        if should_pass:
            self.assertEqual(results, [])
        else:
            self.assertEqual(len(results), 1)
            self.assertEqual(results[0].__class__,
                             presubmit.OutputApi.PresubmitPromptWarning)

    def ReadFileTest(self, check, content1, content2, error_type):
        change1 = presubmit.Change('foo1', 'foo1\n', self.fake_root_dir, None,
                                   0, 0, None)
        input_api1 = self.MockInputApi(change1, False)
        affected_file1 = mock.MagicMock(presubmit.GitAffectedFile)
        input_api1.AffectedSourceFiles.return_value = [affected_file1]
        input_api1.ReadFile.return_value = content1
        change2 = presubmit.Change('foo2', 'foo2\n', self.fake_root_dir, None,
                                   0, 0, None)
        input_api2 = self.MockInputApi(change2, False)
        affected_file2 = mock.MagicMock(presubmit.GitAffectedFile)
        input_api2.AffectedSourceFiles.return_value = [affected_file2]
        input_api2.ReadFile.return_value = content2
        affected_file2.LocalPath.return_value = 'bar.cc'

        results = check(input_api1, presubmit.OutputApi)
        self.assertEqual(results, [])
        results2 = check(input_api2, presubmit.OutputApi)
        self.assertEqual(len(results2), 1)
        self.assertEqual(results2[0].__class__, error_type)

    def testCannedCheckChangeHasBugField(self):
        self.DescriptionTest(presubmit_canned_checks.CheckChangeHasBugField,
                             'Foo\nBUG=b:1234', 'Foo\n',
                             presubmit.OutputApi.PresubmitNotifyResult, False)

    def testCannedCheckChangeHasBugFieldWithBuganizerSlash(self):
        self.DescriptionTest(presubmit_canned_checks.CheckChangeHasBugField,
                             'Foo\nBUG=b:1234', 'Foo\nBUG=b/1234',
                             presubmit.OutputApi.PresubmitNotifyResult, False)

    def testCannedCheckChangeHasNoUnwantedTags(self):
        self.DescriptionTest(
            presubmit_canned_checks.CheckChangeHasNoUnwantedTags, 'Foo\n',
            'Foo\nFIXED=1234', presubmit.OutputApi.PresubmitError, False)

    def testCheckChangeHasDescription(self):
        self.DescriptionTest(presubmit_canned_checks.CheckChangeHasDescription,
                             'Bleh', '',
                             presubmit.OutputApi.PresubmitNotifyResult, False)
        self.DescriptionTest(presubmit_canned_checks.CheckChangeHasDescription,
                             'Bleh', '', presubmit.OutputApi.PresubmitError,
                             True)

    def testCannedCheckDoNotSubmitInDescription(self):
        self.DescriptionTest(
            presubmit_canned_checks.CheckDoNotSubmitInDescription,
            'Foo\nDO NOTSUBMIT', 'Foo\nDO NOT ' + 'SUBMIT',
            presubmit.OutputApi.PresubmitError, False)

    def testCannedCheckDoNotSubmitInFiles(self):
        self.ContentTest(
            lambda x, y, z: presubmit_canned_checks.CheckDoNotSubmitInFiles(
                x, y), 'DO NOTSUBMIT', None, 'DO NOT ' + 'SUBMIT', None,
            presubmit.OutputApi.PresubmitError)

    def testCannedCheckCorpLinksInDescription(self):
        self.DescriptionTest(
            presubmit_canned_checks.CheckCorpLinksInDescription,
            'chromium.googlesource.com', 'chromium.git.corp.google.com',
            presubmit.OutputApi.PresubmitPromptWarning, False)

    def testCannedCheckCorpLinksInFiles(self):
        self.ContentTest(presubmit_canned_checks.CheckCorpLinksInFiles,
                         'chromium.googlesource.com', None,
                         'chromium.git.corp.google.com', None,
                         presubmit.OutputApi.PresubmitPromptWarning)

    def testCannedCheckLargeScaleChange(self):
        input_api = self.MockInputApi(
            presubmit.Change('foo', 'foo1', self.fake_root_dir, None, 0, 0,
                             None), False)
        affected_files = []
        for i in range(100):
            affected_file = mock.MagicMock(presubmit.GitAffectedFile)
            affected_file.LocalPath.return_value = f'foo{i}.cc'
            affected_files.append(affected_file)
        input_api.AffectedFiles = lambda **_: affected_files

        # Don't warn if less than or equal to 100 files.
        results = presubmit_canned_checks.CheckLargeScaleChange(
            input_api, presubmit.OutputApi)
        self.assertEqual(len(results), 0)

        # Warn if greater than 100 files.
        affected_files.append('bar.cc')

        results = presubmit_canned_checks.CheckLargeScaleChange(
            input_api, presubmit.OutputApi)
        self.assertEqual(len(results), 1)
        result = results[0]
        self.assertEqual(result.__class__,
                         presubmit.OutputApi.PresubmitPromptWarning)
        self.assertIn("large scale change", result.json_format()['message'])

    def testCheckChangeHasNoStrayWhitespace(self):
        self.ContentTest(
            lambda x, y, z: presubmit_canned_checks.
            CheckChangeHasNoStrayWhitespace(x, y), 'Foo', None, 'Foo ', None,
            presubmit.OutputApi.PresubmitPromptWarning)

    def testCheckChangeHasOnlyOneEol(self):
        self.ReadFileTest(presubmit_canned_checks.CheckChangeHasOnlyOneEol,
                          "Hey!\nHo!\n", "Hey!\nHo!\n\n",
                          presubmit.OutputApi.PresubmitPromptWarning)

    def testCheckChangeHasNoCR(self):
        self.ReadFileTest(presubmit_canned_checks.CheckChangeHasNoCR,
                          "Hey!\nHo!\n", "Hey!\r\nHo!\r\n",
                          presubmit.OutputApi.PresubmitPromptWarning)

    def testCheckChangeHasNoCrAndHasOnlyOneEol(self):
        self.ReadFileTest(
            presubmit_canned_checks.CheckChangeHasNoCrAndHasOnlyOneEol,
            "Hey!\nHo!\n", "Hey!\nHo!\n\n",
            presubmit.OutputApi.PresubmitPromptWarning)
        self.ReadFileTest(
            presubmit_canned_checks.CheckChangeHasNoCrAndHasOnlyOneEol,
            "Hey!\nHo!\n", "Hey!\r\nHo!\r\n",
            presubmit.OutputApi.PresubmitPromptWarning)

    def testCheckChangeTodoHasOwner(self):
        self.ContentTest(presubmit_canned_checks.CheckChangeTodoHasOwner,
                         "TODO: foo - bar", None, "TODO: bar", None,
                         presubmit.OutputApi.PresubmitPromptWarning)
        self.ContentTest(presubmit_canned_checks.CheckChangeTodoHasOwner,
                         "TODO(foo): bar", None, "TODO: bar", None,
                         presubmit.OutputApi.PresubmitPromptWarning)

    @mock.patch('git_cl.Changelist')
    @mock.patch('auth.Authenticator')
    def testCannedCheckChangedLUCIConfigsRoot(self, mockGetAuth, mockCl):
        affected_file1 = mock.MagicMock(presubmit.GitAffectedFile)
        affected_file1.LocalPath.return_value = 'foo.cfg'
        affected_file2 = mock.MagicMock(presubmit.GitAffectedFile)
        affected_file2.LocalPath.return_value = 'sub/bar.cfg'

        mockGetAuth().get_id_token().token = 123

        host = 'https://host.com'
        branch = 'branch'
        http_resp = b")]}'\n" + json.dumps({
            'configSets': [{
                'name': 'project/deadbeef',
                'url': '%s/+/%s' % (host, branch)
            }]
        }).encode("utf-8")

        mockCl().GetRemoteBranch.return_value = ('remote', branch)
        mockCl().GetRemoteUrl.return_value = host

        change1 = presubmit.Change('foo', 'foo1', self.fake_root_dir, None, 0,
                                   0, None)
        input_api = self.MockInputApi(change1, False)
        input_api.urllib_request.urlopen().read.return_value = http_resp

        affected_files = (affected_file1, affected_file2)

        input_api.AffectedFiles = lambda **_: affected_files

        proc = mock.Mock()
        proc.communicate.return_value = ('This is STDOUT', 'This is STDERR')
        proc.returncode = 0
        subprocess.Popen.return_value = proc
        input_api.CreateTemporaryFile.return_value = MockTemporaryFile(
            'tmp_file')

        validation_result = {
            'result': {
                'validation': [{
                    'messages': [{
                        'path': 'foo.cfg',
                        'severity': 'ERROR',
                        'text': 'deadbeef',
                    }, {
                        'path': 'sub/bar.cfg',
                        'severity': 'WARNING',
                        'text': 'cafecafe',
                    }]
                }]
            }
        }
        json.load.return_value = validation_result

        results = presubmit_canned_checks.CheckChangedLUCIConfigs(
            input_api, presubmit.OutputApi)
        self.assertEqual(len(results), 2)
        self.assertEqual(results[0].json_format()['message'],
                         "Config validation for file(foo.cfg): deadbeef")
        self.assertEqual(results[1].json_format()['message'],
                         "Config validation for file(sub/bar.cfg): cafecafe")
        subprocess.Popen.assert_called_once_with([
            'lucicfg' + ('.bat' if input_api.is_windows else ''), 'validate',
            '.', '-config-set', 'project/deadbeef', '-log-level',
            'debug' if input_api.verbose else 'warning', '-json-output',
            'tmp_file'
        ],
                                                 cwd=self.fake_root_dir,
                                                 stderr=subprocess.PIPE,
                                                 shell=input_api.is_windows)

    @mock.patch('git_cl.Changelist')
    @mock.patch('auth.Authenticator')
    def testCannedCheckChangedLUCIConfigsNoFile(self, mockGetAuth, mockCl):
        affected_file1 = mock.MagicMock(presubmit.GitAffectedFile)
        affected_file1.LocalPath.return_value = 'foo.cfg'
        affected_file2 = mock.MagicMock(presubmit.GitAffectedFile)
        affected_file2.LocalPath.return_value = 'bar.cfg'

        mockGetAuth().get_id_token().token = 123

        host = 'https://host.com'
        branch = 'branch'
        http_resp = b")]}'\n" + json.dumps({
            'configSets': [{
                'name': 'project/deadbeef',
                'url': '%s/+/%s/generated' % (host, branch)
                # no affected file in generated folder
            }]
        }).encode("utf-8")

        mockCl().GetRemoteBranch.return_value = ('remote', branch)
        mockCl().GetRemoteUrl.return_value = host

        change1 = presubmit.Change('foo', 'foo1', self.fake_root_dir, None, 0,
                                   0, None)
        input_api = self.MockInputApi(change1, False)
        input_api.urllib_request.urlopen().read.return_value = http_resp
        affected_files = (affected_file1, affected_file2)

        input_api.AffectedFiles = lambda **_: affected_files

        results = presubmit_canned_checks.CheckChangedLUCIConfigs(
            input_api, presubmit.OutputApi)
        self.assertEqual(len(results), 0)

    @mock.patch('git_cl.Changelist')
    @mock.patch('auth.Authenticator')
    def testCannedCheckChangedLUCIConfigsNonRoot(self, mockGetAuth, mockCl):
        affected_file1 = mock.MagicMock(presubmit.GitAffectedFile)
        affected_file1.LocalPath.return_value = 'generated/foo.cfg'
        affected_file2 = mock.MagicMock(presubmit.GitAffectedFile)
        affected_file2.LocalPath.return_value = 'generated/bar.cfg'

        mockGetAuth().get_access_token().token = 123

        host = 'https://host.com'
        branch = 'branch'
        http_resp = b")]}'\n" + json.dumps({
            'configSets': [{
                'name': 'project/deadbeef',
                'url': '%s/+/%s/generated' % (host, branch)
            }]
        }).encode("utf-8")

        mockCl().GetRemoteBranch.return_value = ('remote', branch)
        mockCl().GetRemoteUrl.return_value = host

        change1 = presubmit.Change('foo', 'foo1', self.fake_root_dir, None, 0,
                                   0, None)
        input_api = self.MockInputApi(change1, False)
        input_api.urllib_request.urlopen().read.return_value = http_resp
        affected_files = (affected_file1, affected_file2)

        input_api.AffectedFiles = lambda **_: affected_files

        proc = mock.Mock()
        proc.communicate.return_value = ('This is STDOUT', 'This is STDERR')
        proc.returncode = 0
        subprocess.Popen.return_value = proc
        input_api.CreateTemporaryFile.return_value = MockTemporaryFile(
            'tmp_file')

        validation_result = {
            'result': {
                'validation': [{
                    'messages': [
                        {
                            'path': 'bar.cfg',
                            'severity': 'ERROR',
                            'text': 'deadbeef',
                        },
                        {
                            'path': 'sub/baz.cfg',  # not an affected file
                            'severity': 'ERROR',
                            'text': 'cafecafe',
                        }
                    ]
                }]
            }
        }
        json.load.return_value = validation_result

        results = presubmit_canned_checks.CheckChangedLUCIConfigs(
            input_api, presubmit.OutputApi)
        self.assertEqual(len(results), 2)
        self.assertEqual(results[0].json_format()['message'],
                         "Config validation for file(bar.cfg): deadbeef")
        self.assertEqual(
            results[1].json_format()['message'],
            "Found 1 additional errors/warnings in files that are not modified,"
            " run `lucicfg validate %s%sgenerated -config-set "
            "project/deadbeef` to reveal them" %
            (self.fake_root_dir, self._OS_SEP))
        subprocess.Popen.assert_called_once_with([
            'lucicfg' + ('.bat' if input_api.is_windows else ''),
            'validate',
            'generated',
            '-config-set',
            'project/deadbeef',
            '-log-level',
            'debug' if input_api.verbose else 'warning',
            '-json-output',
            'tmp_file',
        ],
                                                 cwd=self.fake_root_dir,
                                                 stderr=subprocess.PIPE,
                                                 shell=input_api.is_windows)

    @mock.patch('git_cl.Changelist')
    @mock.patch('auth.Authenticator')
    def testCannedCheckChangedLUCIConfigsGerritInfo(self, mockGetAuth, mockCl):
        affected_file = mock.MagicMock(presubmit.ProvidedDiffAffectedFile)
        affected_file.LocalPath.return_value = 'foo.cfg'

        mockGetAuth().get_id_token().token = 123

        host = 'host.googlesource.com'
        project = 'project/deadbeef'
        branch = 'branch'
        http_resp = b")]}'\n" + json.dumps({
            'configSets':
            [{
                'name': 'project/deadbeef',
                'url': f'https://{host}/{project}/+/{branch}/generated'
                # no affected file in generated folder
            }]
        }).encode("utf-8")

        gerrit_mock = mock.MagicMock(presubmit.GerritAccessor)
        gerrit_mock.host = host
        gerrit_mock.project = project
        gerrit_mock.branch = branch

        change1 = presubmit.Change('foo', 'foo1', self.fake_root_dir, None, 0,
                                   0, None)
        input_api = self.MockInputApi(change1, False, gerrit_mock)
        input_api.urllib_request.urlopen().read.return_value = http_resp

        input_api.AffectedFiles = lambda **_: (affected_file, )

        results = presubmit_canned_checks.CheckChangedLUCIConfigs(
            input_api, presubmit.OutputApi)
        self.assertEqual(len(results), 0)

    def testCannedCheckChangeHasNoTabs(self):
        self.ContentTest(presubmit_canned_checks.CheckChangeHasNoTabs,
                         'blah blah', None, 'blah\tblah', None,
                         presubmit.OutputApi.PresubmitPromptWarning)

        # Make sure makefiles are ignored.
        change1 = presubmit.Change('foo1', 'foo1\n', self.fake_root_dir, None,
                                   0, 0, None)
        input_api1 = self.MockInputApi(change1, False)
        affected_file1 = mock.MagicMock(presubmit.GitAffectedFile)
        affected_file1.LocalPath.return_value = 'foo.cc'
        affected_file2 = mock.MagicMock(presubmit.GitAffectedFile)
        affected_file2.LocalPath.return_value = 'foo/Makefile'
        affected_file3 = mock.MagicMock(presubmit.GitAffectedFile)
        affected_file3.LocalPath.return_value = 'makefile'
        # Only this one will trigger.
        affected_file4 = mock.MagicMock(presubmit.GitAffectedFile)
        affected_file1.LocalPath.return_value = 'foo.cc'
        affected_file1.NewContents.return_value = ['yo, ']
        affected_file4.LocalPath.return_value = 'makefile.foo'
        affected_file4.LocalPath.return_value = 'makefile.foo'
        affected_file4.NewContents.return_value = ['ye\t']
        affected_file4.ChangedContents.return_value = [(46, 'ye\t')]
        affected_file4.LocalPath.return_value = 'makefile.foo'
        affected_files = (affected_file1, affected_file2, affected_file3,
                          affected_file4)

        def test(include_deletes=True, file_filter=None):
            self.assertFalse(include_deletes)
            for x in affected_files:
                if file_filter(x):
                    yield x

        # Override the mock of these functions.
        input_api1.FilterSourceFile = lambda x: x
        input_api1.AffectedFiles = test

        results1 = presubmit_canned_checks.CheckChangeHasNoTabs(
            input_api1, presubmit.OutputApi, None)
        self.assertEqual(len(results1), 1)
        self.assertEqual(results1[0].__class__,
                         presubmit.OutputApi.PresubmitPromptWarning)
        self.assertEqual(results1[0]._long_text, 'makefile.foo:46')

    def testCannedCheckLongLines(self):
        check = lambda x, y, z: presubmit_canned_checks.CheckLongLines(
            x, y, 10, z)
        self.ContentTest(check, '0123456789', None, '01234567890', None,
                         presubmit.OutputApi.PresubmitPromptWarning)

    def testCannedCheckJavaLongLines(self):
        check = lambda x, y, _: presubmit_canned_checks.CheckLongLines(x, y, 80)
        self.ContentTest(check, 'A ' * 50, 'foo.java', 'A ' * 50 + 'B',
                         'foo.java', presubmit.OutputApi.PresubmitPromptWarning)

    def testCannedCheckSpecialJavaLongLines(self):
        check = lambda x, y, _: presubmit_canned_checks.CheckLongLines(x, y, 80)
        self.ContentTest(check, 'import ' + 'A ' * 150, 'foo.java',
                         'importSomething ' + 'A ' * 50, 'foo.java',
                         presubmit.OutputApi.PresubmitPromptWarning)

    def testCannedCheckPythonLongLines(self):
        # NOTE: Cannot use ContentTest() here because of the different code path
        #       used for Python checks in CheckLongLines().
        passing_cases = [
            r"""
01234568901234589012345689012345689
A short line
""",
            r"""
01234568901234589012345689012345689
This line is too long but should pass # pylint: disable=line-too-long
""",
            r"""
01234568901234589012345689012345689
# pylint: disable=line-too-long
This line is too long but should pass due to global disable
""",
            r"""
01234568901234589012345689012345689
   #pylint: disable=line-too-long
This line is too long but should pass due to global disable.
""",
            r"""
01234568901234589012345689012345689
                       #    pylint: disable=line-too-long
This line is too long but should pass due to global disable.
""",
            r"""
01234568901234589012345689012345689
# import is a valid exception
import some.really.long.package.name.that.should.pass
""",
            r"""
01234568901234589012345689012345689
# from is a valid exception
from some.really.long.package.name import passing.line
""",
            r"""
01234568901234589012345689012345689
                    import some.package
""",
            r"""
01234568901234589012345689012345689
                    from some.package import stuff
""",
        ]
        for content in passing_cases:
            self.PythonLongLineTest(40, content, should_pass=True)

        failing_cases = [
            r"""
01234568901234589012345689012345689
This line is definitely too long and should fail.
""",
            r"""
01234568901234589012345689012345689
# pylint: disable=line-too-long
This line is too long and should pass due to global disable
# pylint: enable=line-too-long
But this line is too long and should still fail now
""",
            r"""
01234568901234589012345689012345689
# pylint: disable=line-too-long
This line is too long and should pass due to global disable
But this line is too long # pylint: enable=line-too-long
""",
            r"""
01234568901234589012345689012345689
This should fail because the global
check is enabled on the next line.
              #         pylint: enable=line-too-long
""",
            r"""
01234568901234589012345689012345689
# pylint: disable=line-too-long
                  # pylint: enable-foo-bar should pass
The following line should fail
since global directives apply to
the current line as well!
                  # pylint: enable-line-too-long should fail
""",
        ]
        for content in failing_cases[0:0]:
            self.PythonLongLineTest(40, content, should_pass=False)

    def testCannedCheckJSLongLines(self):
        check = lambda x, y, _: presubmit_canned_checks.CheckLongLines(x, y, 10)
        self.ContentTest(check, 'GEN(\'#include "c/b/ui/webui/fixture.h"\');',
                         'foo.js', "// GEN('something');", 'foo.js',
                         presubmit.OutputApi.PresubmitPromptWarning)

    def testCannedCheckJSLongImports(self):
        check = lambda x, y, _: presubmit_canned_checks.CheckLongLines(x, y, 10)
        self.ContentTest(check,
                         "import {Name, otherName} from './dir/file.js';",
                         'foo.js', "// We should import something long, eh?",
                         'foo.js', presubmit.OutputApi.PresubmitPromptWarning)

    def testCannedCheckTSLongImports(self):
        check = lambda x, y, _: presubmit_canned_checks.CheckLongLines(x, y, 10)
        self.ContentTest(check, "import {Name, otherName} from './dir/file';",
                         'foo.ts', "// We should import something long, eh?",
                         'foo.ts', presubmit.OutputApi.PresubmitPromptWarning)

    def testCannedCheckObjCExceptionLongLines(self):
        check = lambda x, y, _: presubmit_canned_checks.CheckLongLines(x, y, 80)
        self.ContentTest(check, '#import ' + 'A ' * 150, 'foo.mm',
                         'import' + 'A ' * 150, 'foo.mm',
                         presubmit.OutputApi.PresubmitPromptWarning)

    def testCannedCheckMakefileLongLines(self):
        check = lambda x, y, _: presubmit_canned_checks.CheckLongLines(x, y, 80)
        self.ContentTest(check, 'A ' * 100, 'foo.mk', 'A ' * 100 + 'B',
                         'foo.mk', presubmit.OutputApi.PresubmitPromptWarning)

    def testCannedCheckLongLinesLF(self):
        check = lambda x, y, z: presubmit_canned_checks.CheckLongLines(
            x, y, 10, z)
        self.ContentTest(check, '012345678\n', None, '0123456789\n', None,
                         presubmit.OutputApi.PresubmitPromptWarning)

    def testCannedCheckCppExceptionLongLines(self):
        check = lambda x, y, z: presubmit_canned_checks.CheckLongLines(
            x, y, 10, z)
        self.ContentTest(check, '#if 56 89 12 45 9191919191919', 'foo.cc',
                         '#nif 56 89 12 45 9191919191919', 'foo.cc',
                         presubmit.OutputApi.PresubmitPromptWarning)

    def testCannedCheckLongLinesHttp(self):
        check = lambda x, y, z: presubmit_canned_checks.CheckLongLines(
            x, y, 10, z)
        self.ContentTest(check, ' http:// 0 23 56', None, ' foob:// 0 23 56',
                         None, presubmit.OutputApi.PresubmitPromptWarning)

    def testCannedCheckLongLinesFile(self):
        check = lambda x, y, z: presubmit_canned_checks.CheckLongLines(
            x, y, 10, z)
        self.ContentTest(check, ' file:// 0 23 56', None, ' foob:// 0 23 56',
                         None, presubmit.OutputApi.PresubmitPromptWarning)

    def testCannedCheckLongLinesCssUrl(self):
        check = lambda x, y, z: presubmit_canned_checks.CheckLongLines(
            x, y, 10, z)
        self.ContentTest(check, ' url(some.png)', 'foo.css', ' url(some.png)',
                         'foo.cc', presubmit.OutputApi.PresubmitPromptWarning)

    def testCannedCheckLongLinesLongSymbol(self):
        check = lambda x, y, z: presubmit_canned_checks.CheckLongLines(
            x, y, 10, z)
        self.ContentTest(check, ' TUP5D_LoNG_SY ', None, ' TUP5D_LoNG_SY5 ',
                         None, presubmit.OutputApi.PresubmitPromptWarning)

    def _LicenseCheck(self,
                      text,
                      license_text,
                      committing,
                      expected_result,
                      new_file=False,
                      **kwargs):
        change = mock.MagicMock(presubmit.GitChange)
        change.scm = 'svn'
        input_api = self.MockInputApi(change, committing)
        affected_file = mock.MagicMock(presubmit.GitAffectedFile)
        if new_file:
            affected_file.Action.return_value = 'A'
        input_api.AffectedSourceFiles.return_value = [affected_file]
        input_api.ReadFile.return_value = text
        if expected_result:
            affected_file.LocalPath.return_value = 'bleh'

        result = presubmit_canned_checks.CheckLicense(input_api,
                                                      presubmit.OutputApi,
                                                      license_text,
                                                      source_file_filter=42,
                                                      **kwargs)
        if expected_result:
            self.assertEqual(len(result), 1)
            self.assertEqual(result[0].__class__, expected_result)
        else:
            self.assertEqual(result, [])

    def testCheckLicenseSuccess(self):
        text = ("#!/bin/python\n"
                "# Copyright (c) 2037 Nobody.\n"
                "# All Rights Reserved.\n"
                "print('foo')\n")
        license_text = (r".*? Copyright \(c\) 2037 Nobody.\n"
                        r".*? All Rights Reserved\.\n")
        self._LicenseCheck(text, license_text, True, None)

    def testCheckLicenseSuccessNew(self):
        # Make sure the license check works on new files with custom licenses.
        text = ("#!/bin/python\n"
                "# Copyright (c) 2037 Nobody.\n"
                "# All Rights Reserved.\n"
                "print('foo')\n")
        license_text = (r".*? Copyright \(c\) 2037 Nobody.\n"
                        r".*? All Rights Reserved\.\n")
        self._LicenseCheck(text, license_text, True, None, new_file=True)

    def testCheckLicenseFailCommit(self):
        text = ("#!/bin/python\n"
                "# Copyright (c) 2037 Nobody.\n"
                "# All Rights Reserved.\n"
                "print('foo')\n")
        license_text = (r".*? Copyright \(c\) 0007 Nobody.\n"
                        r".*? All Rights Reserved\.\n")
        self._LicenseCheck(text, license_text, True,
                           presubmit.OutputApi.PresubmitPromptWarning)

    def testCheckLicenseFailUpload(self):
        text = ("#!/bin/python\n"
                "# Copyright (c) 2037 Nobody.\n"
                "# All Rights Reserved.\n"
                "print('foo')\n")
        license_text = (r".*? Copyright \(c\) 0007 Nobody.\n"
                        r".*? All Rights Reserved\.\n")
        self._LicenseCheck(text, license_text, False,
                           presubmit.OutputApi.PresubmitPromptWarning)

    def testCheckLicenseEmptySuccess(self):
        text = ''
        license_text = (r".*? Copyright \(c\) 2037 Nobody.\n"
                        r".*? All Rights Reserved\.\n")
        self._LicenseCheck(text,
                           license_text,
                           True,
                           None,
                           accept_empty_files=True)

    def testCheckLicenseNewFilePass(self):
        text = self._GetLicenseText(int(time.strftime('%Y')))
        license_text = None
        self._LicenseCheck(text, license_text, False, None, new_file=True)

    def testCheckLicenseNewFileFail(self):
        # Check that we fail on new files with the (c) symbol.
        current_year = int(time.strftime('%Y'))
        text = (
            "#!/bin/python\n"
            "# Copyright (c) %d The Chromium Authors\n"
            "# Use of this source code is governed by a BSD-style license that can "
            "be\n"
            "# found in the LICENSE file.\n"
            "print('foo')\n" % current_year)
        license_text = None
        self._LicenseCheck(text,
                           license_text,
                           False,
                           presubmit.OutputApi.PresubmitPromptWarning,
                           new_file=True)

    def _GetLicenseText(self, current_year):
        return (
            "#!/bin/python\n"
            "# Copyright %d The Chromium Authors\n"
            "# Use of this source code is governed by a BSD-style license that can "
            "be\n"
            "# found in the LICENSE file.\n"
            "print('foo')\n" % current_year)

    def testCheckLicenseNewFileWarn(self):
        # Check that we warn on new files with wrong year. Test with first
        # allowed year.
        text = self._GetLicenseText(2006)
        license_text = None
        self._LicenseCheck(text,
                           license_text,
                           False,
                           presubmit.OutputApi.PresubmitPromptWarning,
                           new_file=True)

    def testCheckLicenseNewCSSFilePass(self):
        # Check that CSS-style comments in license text are supported.
        current_year = int(time.strftime('%Y'))
        text = (
            "/* Copyright %d The Chromium Authors\n"
            " * Use of this source code is governed by a BSD-style license that "
            "can be\n"
            "* found in the LICENSE file. */\n"
            "\n"
            "h1 {}\n" % current_year)
        license_text = None
        self._LicenseCheck(text, license_text, False, None, new_file=True)

    def testCannedCheckTreeIsOpenOpen(self):
        input_api = self.MockInputApi(None, True)
        input_api.urllib_request.urlopen(
        ).read.return_value = 'The tree is open'
        results = presubmit_canned_checks.CheckTreeIsOpen(input_api,
                                                          presubmit.OutputApi,
                                                          url='url_to_open',
                                                          closed='.*closed.*')
        self.assertEqual(results, [])

    def testCannedCheckTreeIsOpenClosed(self):
        input_api = self.MockInputApi(None, True)
        input_api.urllib_request.urlopen().read.return_value = (
            'Tree is closed for maintenance')
        results = presubmit_canned_checks.CheckTreeIsOpen(input_api,
                                                          presubmit.OutputApi,
                                                          url='url_to_closed',
                                                          closed='.*closed.*')
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].__class__,
                         presubmit.OutputApi.PresubmitError)

    def testCannedCheckJsonTreeIsOpenOpen(self):
        input_api = self.MockInputApi(None, True)
        status = {
            'can_commit_freely': True,
            'general_state': 'open',
            'message': 'The tree is open'
        }
        input_api.urllib_request.urlopen().read.return_value = json.dumps(
            status)
        results = presubmit_canned_checks.CheckTreeIsOpen(
            input_api, presubmit.OutputApi, json_url='url_to_open')
        self.assertEqual(results, [])

    def testCannedCheckJsonTreeIsOpenClosed(self):
        input_api = self.MockInputApi(None, True)
        status = {
            'can_commit_freely': False,
            'general_state': 'closed',
            'message': 'The tree is close',
        }
        input_api.urllib_request.urlopen().read.return_value = json.dumps(
            status)
        results = presubmit_canned_checks.CheckTreeIsOpen(
            input_api, presubmit.OutputApi, json_url='url_to_closed')
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].__class__,
                         presubmit.OutputApi.PresubmitError)

    def testRunPythonUnitTestsNoTest(self):
        input_api = self.MockInputApi(None, False)
        presubmit_canned_checks.RunPythonUnitTests(input_api,
                                                   presubmit.OutputApi, [])
        results = input_api.thread_pool.RunAsync()
        self.assertEqual(results, [])

    def testRunPythonUnitTestsNonExistentUpload(self):
        input_api = self.MockInputApi(None, False)
        subprocess.Popen().returncode = 1  # pylint: disable=no-value-for-parameter
        presubmit.sigint_handler.wait.return_value = (b'foo', None)

        results = presubmit_canned_checks.RunPythonUnitTests(
            input_api, presubmit.OutputApi, ['_non_existent_module'])
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].__class__,
                         presubmit.OutputApi.PresubmitNotifyResult)

    def testRunPythonUnitTestsNonExistentCommitting(self):
        input_api = self.MockInputApi(None, True)
        subprocess.Popen().returncode = 1  # pylint: disable=no-value-for-parameter
        presubmit.sigint_handler.wait.return_value = (b'foo', None)

        results = presubmit_canned_checks.RunPythonUnitTests(
            input_api, presubmit.OutputApi, ['_non_existent_module'])
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].__class__,
                         presubmit.OutputApi.PresubmitError)

    def testRunPythonUnitTestsFailureUpload(self):
        input_api = self.MockInputApi(None, False)
        input_api.unittest = mock.MagicMock(unittest)
        subprocess.Popen().returncode = 1  # pylint: disable=no-value-for-parameter
        presubmit.sigint_handler.wait.return_value = (b'foo', None)

        results = presubmit_canned_checks.RunPythonUnitTests(
            input_api, presubmit.OutputApi, ['test_module'])
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].__class__,
                         presubmit.OutputApi.PresubmitNotifyResult)
        self.assertEqual(
            'test_module\npyyyyython3 -m test_module (0.00s) failed\nfoo',
            results[0]._message)

    def testRunPythonUnitTestsFailureCommitting(self):
        input_api = self.MockInputApi(None, True)
        subprocess.Popen().returncode = 1  # pylint: disable=no-value-for-parameter
        presubmit.sigint_handler.wait.return_value = (b'foo', None)

        results = presubmit_canned_checks.RunPythonUnitTests(
            input_api, presubmit.OutputApi, ['test_module'])
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].__class__,
                         presubmit.OutputApi.PresubmitError)
        self.assertEqual(
            'test_module\npyyyyython3 -m test_module (0.00s) failed\nfoo',
            results[0]._message)

    def testRunPythonUnitTestsSuccess(self):
        input_api = self.MockInputApi(None, False)
        input_api.unittest = mock.MagicMock(unittest)
        subprocess.Popen().returncode = 0  # pylint: disable=no-value-for-parameter
        presubmit.sigint_handler.wait.return_value = (b'', None)

        presubmit_canned_checks.RunPythonUnitTests(input_api,
                                                   presubmit.OutputApi,
                                                   ['test_module'])
        results = input_api.thread_pool.RunAsync()
        self.assertEqual(results, [])

    def testCannedRunPylint(self):
        change = mock.Mock()
        change.RepositoryRoot.return_value = 'CWD'
        input_api = self.MockInputApi(change, True)
        input_api.environ = mock.MagicMock(os.environ)
        input_api.environ.copy.return_value = {}
        input_api.AffectedSourceFiles.return_value = True
        input_api.PresubmitLocalPath.return_value = 'CWD'
        input_api.os_walk.return_value = [('CWD', [], ['file1.py'])]

        process = mock.Mock()
        process.returncode = 0
        subprocess.Popen.return_value = process
        presubmit.sigint_handler.wait.return_value = (b'', None)

        pylint = os.path.join(_ROOT, 'pylint-2.7')
        pylintrc = os.path.join(_ROOT, 'pylintrc-2.7')
        env = {str('PYTHONPATH'): str('')}
        if sys.platform == 'win32':
            pylint += '.bat'

        results = presubmit_canned_checks.RunPylint(input_api,
                                                    presubmit.OutputApi,
                                                    version='2.7')

        self.assertEqual([], results)
        self.assertEqual(subprocess.Popen.mock_calls, [
            mock.call([pylint, '--args-on-stdin'],
                      env=env,
                      cwd='CWD',
                      stderr=subprocess.STDOUT,
                      stdout=subprocess.PIPE,
                      stdin=subprocess.PIPE),
        ])
        self.assertEqual(presubmit.sigint_handler.wait.mock_calls, [
            mock.call(process,
                      ('--rcfile=%s\nfile1.py' % pylintrc).encode('utf-8')),
        ])

        self.checkstdout('')

    def GetInputApiWithFiles(self, files):
        change = mock.MagicMock(presubmit.Change)
        change.AffectedFiles = lambda *a, **kw: (presubmit.Change.AffectedFiles(
            change, *a, **kw))
        change._affected_files = []
        for path, (action, contents) in files.items():
            affected_file = mock.MagicMock(presubmit.GitAffectedFile)
            affected_file.AbsoluteLocalPath.return_value = path
            affected_file.LocalPath.return_value = path
            affected_file.Action.return_value = action
            affected_file.ChangedContents.return_value = [
                (1, contents or ''),
            ]
            change._affected_files.append(affected_file)

        input_api = self.MockInputApi(None, False)
        input_api.change = change
        input_api.ReadFile = lambda path: files[path][1]
        input_api.basename = os.path.basename
        input_api.is_windows = sys.platform.startswith('win')

        os.path.exists = lambda path: path in files and files[path][0] != 'D'
        os.path.isfile = os.path.exists

        return input_api

    def testCheckDirMetadataFormat(self):
        input_api = self.GetInputApiWithFiles({
            'DIR_METADATA': ('M', ''),
            'a/DIR_METADATA': ('M', ''),
            'a/b/OWNERS': ('M', ''),
            'c/DIR_METADATA': ('D', ''),
            'd/unrelated': ('M', ''),
        })

        dirmd_bin = 'dirmd.bat' if input_api.is_windows else 'dirmd'
        expected_cmd = [
            dirmd_bin, 'validate', 'DIR_METADATA', 'a/DIR_METADATA',
            'a/b/OWNERS'
        ]

        commands = presubmit_canned_checks.CheckDirMetadataFormat(
            input_api, presubmit.OutputApi)
        self.assertEqual(1, len(commands))
        self.assertEqual(expected_cmd, commands[0].cmd)

    def testCheckNoNewMetadataInOwners(self):
        input_api = self.GetInputApiWithFiles({
            'no-new-metadata/OWNERS': ('M', '# WARNING: Blah'),
            'added-no-new-metadata/OWNERS': ('A', '# WARNING: Bleh'),
            'deleted/OWNERS': ('D', None),
        })
        self.assertEqual([],
                         presubmit_canned_checks.CheckNoNewMetadataInOwners(
                             input_api, presubmit.OutputApi))

    def testCheckNoNewMetadataInOwnersFails(self):
        input_api = self.GetInputApiWithFiles({
            'new-metadata/OWNERS': ('M', '# CoMpOnEnT: Monorail>Component'),
        })
        results = presubmit_canned_checks.CheckNoNewMetadataInOwners(
            input_api, presubmit.OutputApi)
        self.assertEqual(1, len(results))
        self.assertIsInstance(results[0], presubmit.OutputApi.PresubmitError)

    def testCheckOwnersDirMetadataExclusiveWorks(self):
        input_api = self.GetInputApiWithFiles({
            'only-owners/OWNERS': ('M', '# COMPONENT: Monorail>Component'),
            'only-dir-metadata/DIR_METADATA': ('M', ''),
            'owners-has-no-metadata/DIR_METADATA': ('M', ''),
            'owners-has-no-metadata/OWNERS': ('M', 'no-metadata'),
            'deleted-owners/OWNERS': ('D', None),
            'deleted-owners/DIR_METADATA': ('M', ''),
            'deleted-dir-metadata/OWNERS':
            ('M', '# COMPONENT: Monorail>Component'),
            'deleted-dir-metadata/DIR_METADATA': ('D', None),
            'non-metadata-comment/OWNERS': ('M', '# WARNING: something.'),
            'non-metadata-comment/DIR_METADATA': ('M', ''),
        })
        self.assertEqual(
            [],
            presubmit_canned_checks.CheckOwnersDirMetadataExclusive(
                input_api, presubmit.OutputApi))

    def testCheckOwnersDirMetadataExclusiveFails(self):
        input_api = self.GetInputApiWithFiles({
            'DIR_METADATA': ('M', ''),
            'OWNERS': ('M', '# COMPONENT: Monorail>Component'),
        })
        results = presubmit_canned_checks.CheckOwnersDirMetadataExclusive(
            input_api, presubmit.OutputApi)
        self.assertEqual(1, len(results))
        self.assertIsInstance(results[0], presubmit.OutputApi.PresubmitError)

    def GetInputApiWithOWNERS(self, owners_content):
        input_api = self.GetInputApiWithFiles({'OWNERS': ('M', owners_content)})
        gerrit_mock = mock.MagicMock(presubmit.GerritAccessor)
        gerrit_mock.IsCodeOwnersEnabledOnRepo = lambda: True
        input_api.gerrit = gerrit_mock

        return input_api

    def testCheckOwnersFormatWorks_CodeOwners(self):
        # If code owners is enabled, we rely on it to check owners format
        # instead of depot tools.
        input_api = self.GetInputApiWithOWNERS('any content')
        self.assertEqual([],
                         presubmit_canned_checks.CheckOwnersFormat(
                             input_api, presubmit.OutputApi))

    def AssertOwnersWorks(self,
                          tbr=False,
                          issue='1',
                          approvers=None,
                          modified_files=None,
                          owners_by_path=None,
                          is_committing=True,
                          response=None,
                          expected_output='',
                          manually_specified_reviewers=None,
                          dry_run=None):
        # The set of people who lgtm'ed a change.
        approvers = approvers or set()
        manually_specified_reviewers = manually_specified_reviewers or []
        modified_files = modified_files or ['foo/xyz.cc']
        owners_by_path = owners_by_path or {'foo/xyz.cc': ['john@example.com']}
        response = response or {
            "owner": {
                "email": 'john@example.com'
            },
            "labels": {
                "Code-Review": {
                    u'all': [{
                        u'email': a,
                        u'value': +1
                    } for a in approvers],
                    u'default_value': 0,
                    u'values': {
                        u' 0': u'No score',
                        u'+1': u'Looks good to me',
                        u'-1': u"I would prefer that you didn't submit this"
                    }
                }
            },
            "reviewers": {"REVIEWER": [{
                u'email': a
            }]
                          for a in approvers},
        }

        change = mock.MagicMock(presubmit.Change)
        change.OriginalOwnersFiles.return_value = {}
        change.RepositoryRoot.return_value = None
        change.ReviewersFromDescription.return_value = manually_specified_reviewers
        change.TBRsFromDescription.return_value = []
        change.author_email = 'john@example.com'
        change.issue = issue

        affected_files = []
        for f in modified_files:
            affected_file = mock.MagicMock(presubmit.GitAffectedFile)
            affected_file.LocalPath.return_value = f
            affected_files.append(affected_file)
        change.AffectedFiles.return_value = affected_files

        input_api = self.MockInputApi(change, False)
        input_api.gerrit = presubmit.GerritAccessor('host')
        input_api.is_committing = is_committing
        input_api.tbr = tbr
        input_api.dry_run = dry_run
        input_api.gerrit._FetchChangeDetail = lambda _: response
        input_api.gerrit.IsCodeOwnersEnabledOnRepo = lambda: True

        input_api.owners_client = owners_client.OwnersClient()

        with mock.patch('owners_client.OwnersClient.ListOwners',
                        side_effect=lambda f: owners_by_path.get(f, [])):
            results = presubmit_canned_checks.CheckOwners(
                input_api, presubmit.OutputApi)
        for result in results:
            result.handle()
        if expected_output:
            self.assertRegexpMatches(sys.stdout.getvalue(), expected_output)
        else:
            self.assertEqual(sys.stdout.getvalue(), expected_output)
        sys.stdout.truncate(0)

    def testCannedCheckOwners_TBRIgnored(self):
        self.AssertOwnersWorks(tbr=True, expected_output='')
        self.AssertOwnersWorks(tbr=True,
                               is_committing=False,
                               expected_output='')

    @mock.patch('io.open', mock.mock_open())
    def testCannedRunUnitTests(self):
        io.open().readline.return_value = ''
        change = presubmit.Change('foo1', 'description1', self.fake_root_dir,
                                  None, 0, 0, None)
        input_api = self.MockInputApi(change, False)
        input_api.verbose = True
        input_api.PresubmitLocalPath.return_value = self.fake_root_dir
        presubmit.sigint_handler.wait.return_value = (b'', None)

        process1 = mock.Mock()
        process1.returncode = 1
        process2 = mock.Mock()
        process2.returncode = 0
        subprocess.Popen.side_effect = [process1, process2]

        unit_tests = ['allo', 'bar.py']
        results = presubmit_canned_checks.RunUnitTests(input_api,
                                                       presubmit.OutputApi,
                                                       unit_tests)
        self.assertEqual(2, len(results))
        self.assertEqual(presubmit.OutputApi.PresubmitNotifyResult,
                         results[1].__class__)
        self.assertEqual(presubmit.OutputApi.PresubmitPromptWarning,
                         results[0].__class__)

        cmd = ['bar.py', '--verbose']
        if input_api.platform == 'win32':
            cmd.insert(0, 'vpython3.bat')
        else:
            cmd.insert(0, 'vpython3')
        self.assertEqual(subprocess.Popen.mock_calls, [
            mock.call(cmd,
                      cwd=self.fake_root_dir,
                      stdout=subprocess.PIPE,
                      stderr=subprocess.STDOUT,
                      stdin=subprocess.PIPE),
            mock.call(['allo', '--verbose'],
                      cwd=self.fake_root_dir,
                      stdout=subprocess.PIPE,
                      stderr=subprocess.STDOUT,
                      stdin=subprocess.PIPE),
        ])

        threading.Timer.assert_not_called()

        self.checkstdout('')

    @mock.patch('io.open', mock.mock_open())
    def testCannedRunUnitTestsWithTimer(self):
        io.open().readline.return_value = ''
        change = presubmit.Change('foo1', 'description1', self.fake_root_dir,
                                  None, 0, 0, None)
        input_api = self.MockInputApi(change, False)
        input_api.verbose = True
        input_api.thread_pool.timeout = 100
        input_api.PresubmitLocalPath.return_value = self.fake_root_dir
        presubmit.sigint_handler.wait.return_value = (b'', None)
        subprocess.Popen.return_value = mock.Mock(returncode=0)

        results = presubmit_canned_checks.RunUnitTests(input_api,
                                                       presubmit.OutputApi,
                                                       ['bar.py'])
        self.assertEqual(presubmit.OutputApi.PresubmitNotifyResult,
                         results[0].__class__)

        threading.Timer.assert_called_once_with(input_api.thread_pool.timeout,
                                                mock.ANY)
        threading.Timer().start.assert_called_once_with()
        threading.Timer().cancel.assert_called_once_with()

        self.checkstdout('')

    @mock.patch('io.open', mock.mock_open())
    def testCannedRunUnitTestsWithTimerTimesOut(self):
        io.open().readline.return_value = ''
        change = presubmit.Change('foo1', 'description1', self.fake_root_dir,
                                  None, 0, 0, None)
        input_api = self.MockInputApi(change, False)
        input_api.verbose = True
        input_api.thread_pool.timeout = 100
        input_api.PresubmitLocalPath.return_value = self.fake_root_dir
        presubmit.sigint_handler.wait.return_value = (b'', None)
        subprocess.Popen.return_value = mock.Mock(returncode=1)

        timer_instance = mock.Mock()

        def mockTimer(_, fn):
            fn()
            return timer_instance

        threading.Timer.side_effect = mockTimer

        results = presubmit_canned_checks.RunUnitTests(input_api,
                                                       presubmit.OutputApi,
                                                       ['bar.py'])
        self.assertEqual(presubmit.OutputApi.PresubmitPromptWarning,
                         results[0].__class__)

        results[0].handle()
        self.assertIn(
            'bar.py --verbose (0.00s) failed\nProcess timed out after 100s',
            sys.stdout.getvalue())

        threading.Timer.assert_called_once_with(input_api.thread_pool.timeout,
                                                mock.ANY)
        timer_instance.start.assert_called_once_with()

    @mock.patch('io.open', mock.mock_open())
    def testCannedRunUnitTestsPython3(self):
        io.open().readline.return_value = '#!/usr/bin/env python3'
        change = presubmit.Change('foo1', 'description1', self.fake_root_dir,
                                  None, 0, 0, None)
        input_api = self.MockInputApi(change, False)
        input_api.verbose = True
        input_api.PresubmitLocalPath.return_value = self.fake_root_dir
        presubmit.sigint_handler.wait.return_value = (b'', None)

        subprocesses = [
            mock.Mock(returncode=1),
            mock.Mock(returncode=0),
            mock.Mock(returncode=0),
        ]
        subprocess.Popen.side_effect = subprocesses

        unit_tests = ['allo', 'bar.py']
        results = presubmit_canned_checks.RunUnitTests(input_api,
                                                       presubmit.OutputApi,
                                                       unit_tests)
        self.assertEqual([result.__class__ for result in results], [
            presubmit.OutputApi.PresubmitPromptWarning,
            presubmit.OutputApi.PresubmitNotifyResult,
        ])

        cmd = ['bar.py', '--verbose']
        vpython3 = 'vpython3'
        if input_api.platform == 'win32':
            vpython3 += '.bat'

        self.assertEqual(subprocess.Popen.mock_calls, [
            mock.call([vpython3] + cmd,
                      cwd=self.fake_root_dir,
                      stdout=subprocess.PIPE,
                      stderr=subprocess.STDOUT,
                      stdin=subprocess.PIPE),
            mock.call(['allo', '--verbose'],
                      cwd=self.fake_root_dir,
                      stdout=subprocess.PIPE,
                      stderr=subprocess.STDOUT,
                      stdin=subprocess.PIPE),
        ])

        self.assertEqual(presubmit.sigint_handler.wait.mock_calls, [
            mock.call(subprocesses[0], None),
            mock.call(subprocesses[1], None),
        ])

        self.checkstdout('')

    @mock.patch('io.open', mock.mock_open())
    def testCannedRunUnitTestsDontRunOnPython2(self):
        io.open().readline.return_value = '#!/usr/bin/env python3'
        change = presubmit.Change('foo1', 'description1', self.fake_root_dir,
                                  None, 0, 0, None)
        input_api = self.MockInputApi(change, False)
        input_api.verbose = True
        input_api.PresubmitLocalPath.return_value = self.fake_root_dir
        presubmit.sigint_handler.wait.return_value = (b'', None)

        subprocess.Popen.side_effect = [
            mock.Mock(returncode=1),
            mock.Mock(returncode=0),
            mock.Mock(returncode=0),
        ]

        unit_tests = ['allo', 'bar.py']
        results = presubmit_canned_checks.RunUnitTests(input_api,
                                                       presubmit.OutputApi,
                                                       unit_tests,
                                                       run_on_python2=False)
        self.assertEqual([result.__class__ for result in results], [
            presubmit.OutputApi.PresubmitPromptWarning,
            presubmit.OutputApi.PresubmitNotifyResult,
        ])

        cmd = ['bar.py', '--verbose']
        vpython3 = 'vpython3'
        if input_api.platform == 'win32':
            vpython3 += '.bat'

        self.assertEqual(subprocess.Popen.mock_calls, [
            mock.call([vpython3] + cmd,
                      cwd=self.fake_root_dir,
                      stdout=subprocess.PIPE,
                      stderr=subprocess.STDOUT,
                      stdin=subprocess.PIPE),
            mock.call(['allo', '--verbose'],
                      cwd=self.fake_root_dir,
                      stdout=subprocess.PIPE,
                      stderr=subprocess.STDOUT,
                      stdin=subprocess.PIPE),
        ])

        self.checkstdout('')

    def testCannedRunUnitTestsInDirectory(self):
        change = presubmit.Change('foo1', 'description1', self.fake_root_dir,
                                  None, 0, 0, None)
        input_api = self.MockInputApi(change, False)
        input_api.verbose = True
        input_api.logging = mock.MagicMock(logging)
        input_api.PresubmitLocalPath.return_value = self.fake_root_dir
        input_api.os_listdir.return_value = ['.', '..', 'a', 'b', 'c']
        input_api.os_path.isfile = lambda x: not x.endswith('.')

        process = mock.Mock()
        process.returncode = 0
        subprocess.Popen.return_value = process
        presubmit.sigint_handler.wait.return_value = (b'', None)

        results = presubmit_canned_checks.RunUnitTestsInDirectory(
            input_api,
            presubmit.OutputApi,
            'random_directory',
            files_to_check=['^a$', '^b$'],
            files_to_skip=['a'])
        self.assertEqual(1, len(results))
        self.assertEqual(presubmit.OutputApi.PresubmitNotifyResult,
                         results[0].__class__)
        self.assertEqual(subprocess.Popen.mock_calls, [
            mock.call([os.path.join('random_directory', 'b'), '--verbose'],
                      cwd=self.fake_root_dir,
                      stdout=subprocess.PIPE,
                      stderr=subprocess.STDOUT,
                      stdin=subprocess.PIPE),
        ])
        self.checkstdout('')

    def testPanProjectChecks(self):
        # Make sure it accepts both list and tuples.
        change = presubmit.Change('foo1', 'description1', self.fake_root_dir,
                                  None, 0, 0, None)
        input_api = self.MockInputApi(change, False)
        affected_file = mock.MagicMock(presubmit.GitAffectedFile)
        input_api.AffectedFiles.return_value = [affected_file]
        affected_file.NewContents.return_value = 'Hey!\nHo!\nHey!\nHo!\n\n'
        # CheckChangeHasNoTabs() calls _FindNewViolationsOfRule() which calls
        # ChangedContents().
        affected_file.ChangedContents.return_value = [
            (0, 'Hey!\n'), (1, 'Ho!\n'), (2, 'Hey!\n'), (3, 'Ho!\n'), (4, '\n')
        ]

        affected_file.LocalPath.return_value = 'hello.py'

        # CheckingLicense() calls AffectedSourceFiles() instead of
        # AffectedFiles().
        input_api.AffectedSourceFiles.return_value = [affected_file]
        input_api.ReadFile.return_value = 'Hey!\nHo!\nHey!\nHo!\n\n'

        results = presubmit_canned_checks.PanProjectChecks(input_api,
                                                           presubmit.OutputApi,
                                                           excluded_paths=None,
                                                           text_files=None,
                                                           license_header=None,
                                                           project_name=None,
                                                           owners_check=False)
        self.assertEqual(2, len(results))
        self.assertEqual('Found line ending with white spaces in:',
                         results[0]._message)
        self.checkstdout('')

    def testCheckCIPDManifest_file(self):
        input_api = self.MockInputApi(None, False)

        command = presubmit_canned_checks.CheckCIPDManifest(input_api,
                                                            presubmit.OutputApi,
                                                            path='/path/to/foo')
        self.assertEqual(
            command.cmd,
            ['cipd', 'ensure-file-verify', '-ensure-file', '/path/to/foo'])
        self.assertEqual(
            command.kwargs, {
                'stdin': subprocess.PIPE,
                'stdout': subprocess.PIPE,
                'stderr': subprocess.STDOUT,
            })

    def testCheckCIPDManifest_content(self):
        input_api = self.MockInputApi(None, False)
        input_api.verbose = True

        command = presubmit_canned_checks.CheckCIPDManifest(
            input_api, presubmit.OutputApi, content='manifest_content')
        self.assertEqual(command.cmd, [
            'cipd', 'ensure-file-verify', '-log-level', 'debug',
            '-ensure-file=-'
        ])
        self.assertEqual(command.stdin, b'manifest_content')
        self.assertEqual(
            command.kwargs, {
                'stdin': subprocess.PIPE,
                'stdout': subprocess.PIPE,
                'stderr': subprocess.STDOUT,
            })

    def testCheckCIPDPackages(self):
        content = '\n'.join([
            '$VerifiedPlatform foo-bar',
            '$VerifiedPlatform baz-qux',
            'foo/bar/baz/${platform} version:ohaithere',
            'qux version:kthxbye',
        ])

        input_api = self.MockInputApi(None, False)

        command = presubmit_canned_checks.CheckCIPDPackages(
            input_api,
            presubmit.OutputApi,
            platforms=['foo-bar', 'baz-qux'],
            packages={
                'foo/bar/baz/${platform}': 'version:ohaithere',
                'qux': 'version:kthxbye',
            })
        self.assertEqual(command.cmd,
                         ['cipd', 'ensure-file-verify', '-ensure-file=-'])
        self.assertEqual(command.stdin, content.encode())
        self.assertEqual(
            command.kwargs, {
                'stdin': subprocess.PIPE,
                'stdout': subprocess.PIPE,
                'stderr': subprocess.STDOUT,
            })

    def testCheckCIPDClientDigests(self):
        input_api = self.MockInputApi(None, False)
        input_api.verbose = True

        command = presubmit_canned_checks.CheckCIPDClientDigests(
            input_api, presubmit.OutputApi, client_version_file='ver')
        self.assertEqual(command.cmd, [
            'cipd',
            'selfupdate-roll',
            '-check',
            '-version-file',
            'ver',
            '-log-level',
            'debug',
        ])

    def testCannedCheckVPythonSpec(self):
        change = presubmit.Change('a', 'b', self.fake_root_dir, None, 0, 0,
                                  None)
        input_api = self.MockInputApi(change, False)
        affected_filenames = ['/path1/to/.vpython', '/path1/to/.vpython3']
        affected_files = []

        for filename in affected_filenames:
            affected_file = mock.MagicMock(presubmit.GitAffectedFile)
            affected_file.AbsoluteLocalPath.return_value = filename
            affected_files.append(affected_file)
        input_api.AffectedTestableFiles.return_value = affected_files

        commands = presubmit_canned_checks.CheckVPythonSpec(
            input_api, presubmit.OutputApi)

        self.assertEqual(len(commands), len(affected_filenames))
        for i in range(0, len(commands)):
            self.assertEqual(commands[i].name,
                             'Verify ' + affected_filenames[i])
            self.assertEqual(commands[i].cmd, [
                input_api.python3_executable, '-vpython-spec',
                affected_filenames[i], '-vpython-tool', 'verify'
            ])
            self.assertDictEqual(
                commands[0].kwargs, {
                    'stderr': input_api.subprocess.STDOUT,
                    'stdout': input_api.subprocess.PIPE,
                    'stdin': input_api.subprocess.PIPE,
                })
            self.assertEqual(commands[0].message,
                             presubmit.OutputApi.PresubmitError)
            self.assertIsNone(commands[0].info)


class ThreadPoolTest(unittest.TestCase):
    def setUp(self):
        super(ThreadPoolTest, self).setUp()
        mock.patch('subprocess2.Popen').start()
        mock.patch('presubmit_support.sigint_handler').start()
        mock.patch('presubmit_support.time_time', return_value=0).start()
        presubmit.sigint_handler.wait.return_value = (b'stdout', '')
        self.addCleanup(mock.patch.stopall)

    def testSurfaceExceptions(self):
        def FakePopen(cmd, **kwargs):
            if cmd[0] == '3':
                raise TypeError('TypeError')
            if cmd[0] == '4':
                raise OSError('OSError')
            if cmd[0] == '5':
                return mock.Mock(returncode=1)
            return mock.Mock(returncode=0)

        subprocess.Popen.side_effect = FakePopen

        mock_tests = [
            presubmit.CommandData(
                name=str(i),
                cmd=[str(i)],
                kwargs={},
                message=lambda x, **kwargs: x,
            ) for i in range(10)
        ]

        t = presubmit.ThreadPool(1)
        t.AddTests(mock_tests)
        messages = sorted(t.RunAsync())

        self.assertEqual(3, len(messages))
        self.assertIn(
            '3\n3 exec failure (0.00s)\nTraceback (most recent call last):',
            messages[0])
        self.assertIn(
            '4\n4 exec failure (0.00s)\nTraceback (most recent call last):',
            messages[1])
        self.assertEqual('5\n5 (0.00s) failed\nstdout', messages[2])


if __name__ == '__main__':
    import unittest
    unittest.main()
