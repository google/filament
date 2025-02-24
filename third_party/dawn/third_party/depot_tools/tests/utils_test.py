#!/usr/bin/env vpython3
# Copyright 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging
import os
import shutil
import sys
import tempfile
import unittest
from unittest import mock

DEPOT_TOOLS_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, DEPOT_TOOLS_ROOT)

from testing_support import coverage_utils
import utils


class GitCacheTest(unittest.TestCase):
    def setUp(self):
        pass

    @mock.patch('subprocess.check_output', lambda x, **kwargs: b'foo')
    def testVersionWithGit(self):
        version = utils.depot_tools_version()
        self.assertEqual(version, 'git-foo')

    @mock.patch('subprocess.check_output')
    @mock.patch('os.path.getmtime', lambda x: 42)
    def testVersionWithNoGit(self, mock_subprocess):
        mock_subprocess.side_effect = Exception
        version = utils.depot_tools_version()
        self.assertEqual(version, 'recipes.cfg-42')

    @mock.patch('subprocess.check_output')
    @mock.patch('os.path.getmtime')
    def testVersionWithNoGit(self, mock_subprocess, mock_getmtime):
        mock_subprocess.side_effect = Exception
        mock_getmtime.side_effect = Exception
        version = utils.depot_tools_version()
        self.assertEqual(version, 'unknown')


class ConfigDirTest(unittest.TestCase):

    @mock.patch('sys.platform', 'win')
    def testWin(self):
        self.assertEqual(DEPOT_TOOLS_ROOT, utils.depot_tools_config_dir())

    @mock.patch('sys.platform', 'darwin')
    def testMac(self):
        self.assertEqual(DEPOT_TOOLS_ROOT, utils.depot_tools_config_dir())

    @mock.patch('sys.platform', 'foo')
    def testOther(self):
        self.assertEqual(DEPOT_TOOLS_ROOT, utils.depot_tools_config_dir())

    @mock.patch('sys.platform', 'linux')
    @mock.patch.dict('os.environ', {})
    def testLinuxDefault(self):
        self.assertEqual(
            os.path.join(os.path.expanduser('~/.config'), 'depot_tools'),
            utils.depot_tools_config_dir())

    @mock.patch('sys.platform', 'linux')
    @mock.patch.dict('os.environ', {'XDG_CONFIG_HOME': '/my/home'})
    def testLinuxCustom(self):
        self.assertEqual(os.path.join('/my/home', 'depot_tools'),
                         utils.depot_tools_config_dir())


class ConfigPathTest(unittest.TestCase):

    def setUp(self):
        self.temp_dir = tempfile.mkdtemp(prefix='utils_test')
        self.config_dir = os.path.join(self.temp_dir, 'test_files')

        self.isfile = mock.Mock()
        self.move = mock.Mock()

        mock.patch('os.path.isfile', self.isfile).start()
        mock.patch('shutil.move', self.move).start()
        mock.patch('utils.depot_tools_config_dir',
                   lambda: self.config_dir).start()

        self.addCleanup(mock.patch.stopall)
        self.addCleanup(shutil.rmtree, self.temp_dir)

    def testCreatesConfigDir(self):
        # Ensure "legacy path" doesn't exist so that nothing gets moved.
        def EnsureLegacyPathNotExists(path):
            return path != os.path.join(DEPOT_TOOLS_ROOT, 'metrics.cfg')

        self.isfile.side_effect = EnsureLegacyPathNotExists

        self.assertEqual(os.path.join(self.config_dir, 'metrics.cfg'),
                         utils.depot_tools_config_path('metrics.cfg'))
        self.assertTrue(os.path.exists(self.config_dir))
        self.move.assert_not_called()

    def testMovesLegacy(self):
        # Ensure "legacy path" exists so that it gets moved.
        def EnsureLegacyPathExists(path):
            return path == os.path.join(DEPOT_TOOLS_ROOT, 'metrics.cfg')

        self.isfile.side_effect = EnsureLegacyPathExists

        self.assertEqual(os.path.join(self.config_dir, 'metrics.cfg'),
                         utils.depot_tools_config_path('metrics.cfg'))
        self.move.assert_called_once_with(
            os.path.join(DEPOT_TOOLS_ROOT, 'metrics.cfg'),
            os.path.join(self.config_dir, 'metrics.cfg'))


if __name__ == '__main__':
    logging.basicConfig(
        level=logging.DEBUG if '-v' in sys.argv else logging.ERROR)
    sys.exit(
        coverage_utils.covered_main(
            (os.path.join(DEPOT_TOOLS_ROOT, 'git_cache.py')),
            required_percentage=0))
