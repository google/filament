#!/usr/bin/env vpython3
# Copyright (c) 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import glob
import multiprocessing
import os
import os.path
import io
import sys
import unittest
import contextlib
from unittest import mock

from parameterized import parameterized

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT_DIR)

import autoninja
from testing_support import trial_dir


def write(filename, content):
    """Writes the content of a file and create the directories as needed."""
    filename = os.path.abspath(filename)
    dirname = os.path.dirname(filename)
    if not os.path.isdir(dirname):
        os.makedirs(dirname)
    with open(filename, 'w') as f:
        f.write(content)


class AutoninjaTest(trial_dir.TestCase):
    def setUp(self):
        super(AutoninjaTest, self).setUp()
        self.previous_dir = os.getcwd()
        os.chdir(self.root_dir)

    def tearDown(self):
        os.chdir(self.previous_dir)
        super(AutoninjaTest, self).tearDown()

    def test_autoninja(self):
        """Test that by default (= no GN args) autoninja delegates to ninja."""
        with mock.patch('ninja.main', return_value=0) as ninja_main:
            out_dir = os.path.join('out', 'dir')
            write(os.path.join(out_dir, 'args.gn'), '')
            autoninja.main(['autoninja.py', '-C', out_dir])
            ninja_main.assert_called_once()
            args = ninja_main.call_args.args[0]
        self.assertIn('-C', args)
        self.assertEqual(args[args.index('-C') + 1], out_dir)

    @mock.patch('sys.platform', 'win32')
    def test_autoninja_splits_args_on_windows(self):
        """
        Test that autoninja correctly handles the special case of being
        passed its arguments as a quoted, whitespace-delimited string on
        Windows.
        """
        with mock.patch('ninja.main', return_value=0) as ninja_main:
            out_dir = os.path.join('out', 'dir')
            write(os.path.join(out_dir, 'args.gn'), '')
            autoninja.main([
                'autoninja.py',
                '-C {} base'.format(out_dir),
            ])
            ninja_main.assert_called_once()
            args = ninja_main.call_args.args[0]
        self.assertIn('-C', args)
        self.assertEqual(args[args.index('-C') + 1], out_dir)
        self.assertIn('base', args)

    def test_autoninja_reclient(self):
        """
        Test that when specifying use_remoteexec=true, autoninja delegates to
        reclient_helper.
        """
        with mock.patch('reclient_helper.run_ninja',
                        return_value=0) as run_ninja:
            out_dir = os.path.join('out', 'dir')
            write(os.path.join(out_dir, 'args.gn'), 'use_remoteexec=true')
            write(os.path.join('build', 'toolchain', 'use_reclient_value.py'),
                  """
def use_reclient_value(output_dir):
  return True
""")
            write(os.path.join('buildtools', 'reclient_cfgs', 'reproxy.cfg'),
                  'RBE_v=2')
            write(os.path.join('buildtools', 'reclient', 'version.txt'), '0.0')
            write(
                os.path.join('buildtools', 'reclient_cfgs', 'reproxy.cfg'),
                'instance=projects/rbe-chromium-untrusted-test/'
                'instances/default_instance')
            autoninja.main(['autoninja.py', '-C', out_dir])
            run_ninja.assert_called_once()
            args = run_ninja.call_args.args[0]
        self.assertIn('-C', args)
        self.assertEqual(args[args.index('-C') + 1], out_dir)
        # Check that autoninja correctly calculated the number of jobs to use
        # as required for remote execution, instead of using the value for
        # local execution.
        self.assertIn('-j', args)
        parallel_j = int(args[args.index('-j') + 1])
        self.assertGreater(parallel_j, multiprocessing.cpu_count() * 2)

    def test_autoninja_siso(self):
        """
        Test that when specifying use_siso=true, autoninja delegates to siso.
        """
        with mock.patch('siso.main', return_value=0) as siso_main:
            out_dir = os.path.join('out', 'dir')
            write(os.path.join(out_dir, 'args.gn'), 'use_siso=true')
            write(os.path.join('build', 'config', 'siso', '.sisoenv'),
                  'SISO_PROJECT=rbe-chromium-untrusted-test')
            autoninja.main(['autoninja.py', '-C', out_dir])
            siso_main.assert_called_once()
            args = siso_main.call_args.args[0]
        self.assertIn('-C', args)
        self.assertEqual(args[args.index('-C') + 1], out_dir)

    def test_autoninja_siso_reclient(self):
        """
        Test that when specifying use_siso=true and use_remoteexec=true,
        autoninja starts reproxy using reclient_helper and calls 'siso ninja'.
        """
        reclient_helper_calls = []

        @contextlib.contextmanager
        def reclient_helper_mock(argv, tool, _should_collect_logs):
            reclient_helper_calls.append([argv, tool])
            yield 0

        with mock.patch('reclient_helper.build_context', reclient_helper_mock):
            with mock.patch('siso.main', return_value=0) as siso_main:
                out_dir = os.path.join('out', 'dir')
                write(os.path.join(out_dir, 'args.gn'),
                      'use_siso=true\nuse_remoteexec=true')
                write(
                    os.path.join('build', 'toolchain', 'use_reclient_value.py'),
                    """
def use_reclient_value(output_dir):
  return True
""")
                write(
                    os.path.join('buildtools', 'reclient_cfgs', 'reproxy.cfg'),
                    'instance=projects/rbe-chromium-untrusted-test/'
                    'instances/default_instance\n'
                    'RBE_v=2')
                write(os.path.join('buildtools', 'reclient', 'version.txt'),
                      '0.0')
                autoninja.main(['autoninja.py', '-C', out_dir])
                siso_main.assert_called_once_with([
                    'siso', 'ninja', '-project=', '-reapi_instance=', '-C',
                    out_dir
                ])
        self.assertEqual(len(reclient_helper_calls), 1)
        self.assertEqual(
            reclient_helper_calls[0][0],
            ['siso', 'ninja', '-project=', '-reapi_instance=', '-C', out_dir])
        self.assertEqual(reclient_helper_calls[0][1], 'autosiso')

    @mock.patch('sys.platform', 'win32')
    def test_print_cmd_windows(self):
        args = [
            'C:\\Program Files\\Python 3\\bin\\python3.exe', 'ninja.py', '-C',
            'out\\direc tory\\',
            '../../base/types/expected_macros_unittest.cc^', '-j', '140'
        ]
        with contextlib.redirect_stderr(io.StringIO()) as f:
            autoninja._print_cmd(args)
            self.assertEqual(
                f.getvalue(),
                '"C:\\Program Files\\Python 3\\bin\\python3.exe" ninja.py -C ' +
                '"out\\direc tory\\" ' +
                '../../base/types/expected_macros_unittest.cc^^ -j 140\n')

    @mock.patch('sys.platform', 'linux')
    def test_print_cmd_linux(self):
        args = [
            '/home/user name/bin/python3', 'ninja.py', '-C', 'out/direc tory/',
            '../../base/types/expected_macros_unittest.cc^', '-j', '140'
        ]
        with contextlib.redirect_stderr(io.StringIO()) as f:
            autoninja._print_cmd(args)
            self.assertEqual(
                f.getvalue(),
                "'/home/user name/bin/python3' ninja.py -C 'out/direc tory/' " +
                "'../../base/types/expected_macros_unittest.cc^' -j 140\n")


if __name__ == '__main__':
    unittest.main()
