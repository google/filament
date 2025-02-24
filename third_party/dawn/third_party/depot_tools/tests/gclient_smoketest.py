#!/usr/bin/env vpython3
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Smoke tests for gclient.py.

Shell out 'gclient' and run basic conformance tests.
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

# TODO: Should fix these warnings.
# pylint: disable=line-too-long


class GClientSmoke(gclient_smoketest_base.GClientSmokeBase):
    """Doesn't require git-daemon."""
    @property
    def git_base(self):
        return 'git://random.server/git/'

    def testNotConfigured(self):
        res = ('', 'Error: client not configured; see \'gclient config\'\n', 1)
        self.check(res, self.gclient(['diff'], error_ok=True))
        self.check(res, self.gclient(['pack'], error_ok=True))
        self.check(res, self.gclient(['revert'], error_ok=True))
        self.check(res, self.gclient(['revinfo'], error_ok=True))
        self.check(res, self.gclient(['runhooks'], error_ok=True))
        self.check(res, self.gclient(['status'], error_ok=True))
        self.check(res, self.gclient(['sync'], error_ok=True))
        self.check(res, self.gclient(['update'], error_ok=True))

    def testConfig(self):
        # Get any bootstrapping out of the way.
        results = self.gclient(['version'])

        p = join(self.root_dir, '.gclient')

        def test(cmd, expected):
            if os.path.exists(p):
                os.remove(p)
            results = self.gclient(cmd)
            self.check(('', '', 0), results)
            with open(p, 'r') as f:
                actual = {}
                exec(f.read(), {}, actual)
                self.assertEqual(expected, actual)

        test(
            ['config', self.git_base + 'src/'], {
                'solutions': [{
                    'name': 'src',
                    'url': self.git_base + 'src',
                    'deps_file': 'DEPS',
                    'managed': True,
                    'custom_deps': {},
                    'custom_vars': {},
                }],
            })

        test(
            [
                'config', self.git_base + 'repo_1', '--name', 'src',
                '--cache-dir', 'none'
            ], {
                'solutions': [{
                    'name': 'src',
                    'url': self.git_base + 'repo_1',
                    'deps_file': 'DEPS',
                    'managed': True,
                    'custom_deps': {},
                    'custom_vars': {},
                }],
                'cache_dir':
                None
            })

        test(
            [
                'config', 'https://example.com/foo', 'faa', '--cache-dir',
                'something'
            ], {
                'solutions': [{
                    'name': 'foo',
                    'url': 'https://example.com/foo',
                    'deps_file': 'DEPS',
                    'managed': True,
                    'custom_deps': {},
                    'custom_vars': {},
                }],
                'cache_dir':
                'something'
            })

        test(
            ['config', 'https://example.com/foo', '--deps', 'blah'], {
                'solutions': [{
                    'name': 'foo',
                    'url': 'https://example.com/foo',
                    'deps_file': 'blah',
                    'managed': True,
                    'custom_deps': {},
                    'custom_vars': {},
                }]
            })

        test(
            [
                'config', self.git_base + 'src/', '--custom-var',
                'bool_var=True', '--custom-var', 'str_var="abc"'
            ], {
                'solutions': [{
                    'name': 'src',
                    'url': self.git_base + 'src',
                    'deps_file': 'DEPS',
                    'managed': True,
                    'custom_deps': {},
                    'custom_vars': {
                        'bool_var': True,
                        'str_var': 'abc',
                    },
                }]
            })

        test(['config', '--spec', 'bah = ["blah blah"]'],
             {'bah': ["blah blah"]})

        os.remove(p)
        results = self.gclient(['config', 'foo', 'faa', 'fuu'], error_ok=True)
        err = (
            'Usage: gclient.py config [options] [url]\n\n'
            'gclient.py: error: Inconsistent arguments. Use either --spec or one'
            ' or 2 args\n')
        self.check(('', err, 2), results)
        self.assertFalse(os.path.exists(join(self.root_dir, '.gclient')))

    def testSolutionNone(self):
        results = self.gclient(
            ['config', '--spec', 'solutions=[{"name": "./", "url": None}]'])
        self.check(('', '', 0), results)
        results = self.gclient(['sync'])
        self.check(('', '', 0), results)
        self.assertTree({})
        results = self.gclient(['revinfo'])
        self.check(('./: None\n', '', 0), results)
        self.check(('', '', 0), self.gclient(['diff']))
        self.assertTree({})
        self.check(('', '', 0), self.gclient(['pack']))
        self.check(('', '', 0), self.gclient(['revert']))
        self.assertTree({})
        self.check(('', '', 0), self.gclient(['runhooks']))
        self.assertTree({})
        self.check(('', '', 0), self.gclient(['status']))

    def testDifferentTopLevelDirectory(self):
        # Check that even if the .gclient file does not mention the directory
        # src itself, but it is included via dependencies, the .gclient file is
        # used.
        self.gclient(['config', self.git_base + 'src.DEPS'])
        deps = join(self.root_dir, 'src.DEPS')
        os.mkdir(deps)
        subprocess2.check_output(['git', 'init'], cwd=deps)
        write(join(deps, 'DEPS'), 'deps = { "src": "%ssrc" }' % (self.git_base))
        subprocess2.check_output(['git', 'add', 'DEPS'], cwd=deps)
        subprocess2.check_output(['git', 'commit', '-a', '-m', 'DEPS file'],
                                 cwd=deps)
        src = join(self.root_dir, 'src')
        os.mkdir(src)
        subprocess2.check_output(['git', 'init'], cwd=src)
        res = self.gclient(['status', '--jobs', '1', '-v'], src)
        self.checkBlock(res[0], [('running', deps), ('running', src)])


if __name__ == '__main__':
    if '-v' in sys.argv:
        logging.basicConfig(level=logging.DEBUG)
    unittest.main()
