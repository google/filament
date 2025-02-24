#!/usr/bin/env vpython3
# coding=utf-8
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from io import StringIO
from unittest import mock

import gclient_paths
import gclient_utils
import subprocess2

EXCEPTION = subprocess2.CalledProcessError(128, ['cmd'], 'cwd', 'stdout',
                                           'stderr')


class TestBase(unittest.TestCase):
    def setUp(self):
        super(TestBase, self).setUp()
        self.file_tree = {}
        self.root = 'C:\\' if sys.platform == 'win32' else '/'
        # Use unique roots for each test to avoid cache hits from @lru_cache.
        self.root += self._testMethodName
        self.cwd = self.root
        mock.patch('gclient_utils.FileRead', self.read).start()
        mock.patch('os.environ', {}).start()
        mock.patch('os.getcwd', self.getcwd).start()
        mock.patch('os.path.exists', self.exists).start()
        mock.patch('os.path.realpath', side_effect=lambda path: path).start()
        mock.patch('subprocess2.check_output').start()
        mock.patch('sys.platform', '').start()
        mock.patch('sys.stderr', StringIO()).start()
        self.addCleanup(mock.patch.stopall)

    def getcwd(self):
        return self.cwd

    def exists(self, path):
        return path in self.file_tree

    def read(self, path):
        return self.file_tree[path]

    def make_file_tree(self, file_tree):
        self.file_tree = {
            os.path.join(self.root, path): content
            for path, content in file_tree.items()
        }


class FindGclientRootTest(TestBase):
    def testFindGclientRoot(self):
        self.make_file_tree({'.gclient': ''})
        self.assertEqual(self.root, gclient_paths.FindGclientRoot(self.root))

    def testGclientRootInParentDir(self):
        self.make_file_tree({
            '.gclient': '',
            '.gclient_entries': 'entries = {"foo": "..."}',
        })
        self.assertEqual(
            self.root,
            gclient_paths.FindGclientRoot(os.path.join(self.root, 'foo',
                                                       'bar')))

    def testGclientRootInParentDir_NotInGclientEntries(self):
        self.make_file_tree({
            '.gclient': '',
            '.gclient_entries': 'entries = {"foo": "..."}',
        })
        self.assertIsNone(
            gclient_paths.FindGclientRoot(os.path.join(self.root, 'bar',
                                                       'baz')))

    def testGclientRootInParentDir_NoGclientEntriesFile(self):
        self.make_file_tree({'.gclient': ''})
        self.assertEqual(
            self.root,
            gclient_paths.FindGclientRoot(os.path.join(self.root, 'x', 'y',
                                                       'z')))
        self.assertEqual(
            '%s missing, .gclient file in parent directory %s might not be the '
            'file you want to use.\n' %
            (os.path.join(self.root, '.gclient_entries'), self.root),
            sys.stderr.getvalue())

    def testGclientRootInParentDir_ErrorWhenParsingEntries(self):
        self.make_file_tree({'.gclient': '', '.gclient_entries': ':P'})
        with self.assertRaises(Exception):
            gclient_paths.FindGclientRoot(os.path.join(self.root, 'foo', 'bar'))

    def testRootNotFound(self):
        self.assertIsNone(
            gclient_paths.FindGclientRoot(os.path.join(self.root, 'x', 'y',
                                                       'z')))


class GetGClientPrimarySolutionNameTest(TestBase):
    def testGetGClientPrimarySolutionName(self):
        self.make_file_tree({'.gclient': 'solutions = [{"name": "foo"}]'})
        self.assertEqual('foo',
                         gclient_paths.GetGClientPrimarySolutionName(self.root))

    def testNoSolutionsInGclientFile(self):
        self.make_file_tree({'.gclient': ''})
        self.assertIsNone(gclient_paths.GetGClientPrimarySolutionName(
            self.root))


class GetPrimarySolutionPathTest(TestBase):
    def testGetPrimarySolutionPath(self):
        self.make_file_tree({'.gclient': 'solutions = [{"name": "foo"}]'})

        self.assertEqual(os.path.join(self.root, 'foo'),
                         gclient_paths.GetPrimarySolutionPath())

    def testSolutionNameDefaultsToSrc(self):
        self.make_file_tree({'.gclient': ''})

        self.assertEqual(os.path.join(self.root, 'src'),
                         gclient_paths.GetPrimarySolutionPath())

    def testGclientRootNotFound_GitRootHasBuildtools(self):
        self.make_file_tree({os.path.join('foo', 'buildtools'): ''})
        self.cwd = os.path.join(self.root, 'foo', 'bar')
        subprocess2.check_output.return_value = (os.path.join(
            self.root, 'foo').replace(os.sep, '/').encode('utf-8') + b'\n')

        self.assertEqual(os.path.join(self.root, 'foo'),
                         gclient_paths.GetPrimarySolutionPath())

    def testGclientRootNotFound_NoBuildtools(self):
        self.cwd = os.path.join(self.root, 'foo', 'bar')
        subprocess2.check_output.return_value = b'/foo\n'

        self.assertIsNone(gclient_paths.GetPrimarySolutionPath())

    def testGclientRootNotFound_NotInAGitRepo_CurrentDirHasBuildtools(self):
        self.make_file_tree({os.path.join('foo', 'bar', 'buildtools'): ''})
        self.cwd = os.path.join(self.root, 'foo', 'bar')
        subprocess2.check_output.side_effect = EXCEPTION

        self.assertEqual(self.cwd, gclient_paths.GetPrimarySolutionPath())

    def testGclientRootNotFound_NotInAGitRepo_NoBuildtools(self):
        self.cwd = os.path.join(self.root, 'foo')
        subprocess2.check_output.side_effect = EXCEPTION

        self.assertIsNone(gclient_paths.GetPrimarySolutionPath())


class GetBuildtoolsPathTest(TestBase):
    def testEnvVarOverride(self):
        os.environ = {'CHROMIUM_BUILDTOOLS_PATH': 'foo'}

        self.assertEqual('foo', gclient_paths.GetBuildtoolsPath())

    def testNoSolutionsFound(self):
        self.cwd = os.path.join(self.root, 'foo', 'bar')
        subprocess2.check_output.side_effect = EXCEPTION

        self.assertIsNone(gclient_paths.GetBuildtoolsPath())

    def testBuildtoolsInSolution(self):
        self.make_file_tree({
            '.gclient': '',
            os.path.join('src', 'buildtools'): '',
        })
        self.cwd = os.path.join(self.root, 'src', 'foo')

        self.assertEqual(os.path.join(self.root, 'src', 'buildtools'),
                         gclient_paths.GetBuildtoolsPath())

    def testBuildtoolsInGclientRoot(self):
        self.make_file_tree({'.gclient': '', 'buildtools': ''})
        self.cwd = os.path.join(self.root, 'src', 'foo')

        self.assertEqual(os.path.join(self.root, 'buildtools'),
                         gclient_paths.GetBuildtoolsPath())

    def testNoBuildtools(self):
        self.make_file_tree({'.gclient': ''})
        self.cwd = os.path.join(self.root, 'foo', 'bar')

        self.assertIsNone(gclient_paths.GetBuildtoolsPath())


class GetBuildtoolsPlatformBinaryPath(TestBase):
    def testNoBuildtoolsPath(self):
        self.make_file_tree({'.gclient': ''})
        self.cwd = os.path.join(self.root, 'foo', 'bar')
        self.assertIsNone(gclient_paths.GetBuildtoolsPlatformBinaryPath())

    def testWin(self):
        self.make_file_tree({'.gclient': '', 'buildtools': ''})
        sys.platform = 'win'

        self.assertEqual(os.path.join(self.root, 'buildtools', 'win'),
                         gclient_paths.GetBuildtoolsPlatformBinaryPath())

    def testCygwin(self):
        self.make_file_tree({'.gclient': '', 'buildtools': ''})
        sys.platform = 'cygwin'

        self.assertEqual(os.path.join(self.root, 'buildtools', 'win'),
                         gclient_paths.GetBuildtoolsPlatformBinaryPath())

    def testMac(self):
        self.make_file_tree({'.gclient': '', 'buildtools': ''})
        sys.platform = 'darwin'

        self.assertEqual(os.path.join(self.root, 'buildtools', 'mac'),
                         gclient_paths.GetBuildtoolsPlatformBinaryPath())

    def testLinux(self):
        self.make_file_tree({'.gclient': '', 'buildtools': ''})
        sys.platform = 'linux'

        self.assertEqual(os.path.join(self.root, 'buildtools', 'linux64'),
                         gclient_paths.GetBuildtoolsPlatformBinaryPath())

    def testError(self):
        self.make_file_tree({'.gclient': '', 'buildtools': ''})
        sys.platform = 'foo'

        with self.assertRaises(gclient_utils.Error,
                               msg='Unknown platform: foo'):
            gclient_paths.GetBuildtoolsPlatformBinaryPath()


class GetExeSuffixTest(TestBase):
    def testGetExeSuffix(self):
        sys.platform = 'win'
        self.assertEqual('.exe', gclient_paths.GetExeSuffix())

        sys.platform = 'cygwin'
        self.assertEqual('.exe', gclient_paths.GetExeSuffix())

        sys.platform = 'foo'
        self.assertEqual('', gclient_paths.GetExeSuffix())


if __name__ == '__main__':
    unittest.main()
