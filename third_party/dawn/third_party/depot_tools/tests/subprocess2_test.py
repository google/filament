#!/usr/bin/env vpython3
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for subprocess2.py."""

import os
import sys
import unittest
from unittest import mock

DEPOT_TOOLS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, DEPOT_TOOLS)

import subprocess
import subprocess2

TEST_FILENAME = 'subprocess2_test_script.py'

TEST_COMMAND = [
    sys.executable,
    os.path.join(DEPOT_TOOLS, 'testing_support', TEST_FILENAME),
]


class DefaultsTest(unittest.TestCase):
    @mock.patch('subprocess2.communicate')
    def test_check_call_defaults(self, mockCommunicate):
        mockCommunicate.return_value = (('stdout', 'stderr'), 0)
        self.assertEqual(('stdout', 'stderr'),
                         subprocess2.check_call_out(['foo'], a=True))
        mockCommunicate.assert_called_with(['foo'], a=True)

    @mock.patch('subprocess2.communicate')
    def test_capture_defaults(self, mockCommunicate):
        mockCommunicate.return_value = (('stdout', 'stderr'), 0)
        self.assertEqual('stdout', subprocess2.capture(['foo'], a=True))
        mockCommunicate.assert_called_with(['foo'],
                                           a=True,
                                           stdin=subprocess2.DEVNULL,
                                           stdout=subprocess2.PIPE)

    @mock.patch('subprocess2.Popen')
    def test_communicate_defaults(self, mockPopen):
        mockPopen().communicate.return_value = ('bar', 'baz')
        mockPopen().returncode = -8
        self.assertEqual((('bar', 'baz'), -8),
                         subprocess2.communicate(['foo'], a=True))
        mockPopen.assert_called_with(['foo'], a=True)

    @mock.patch('os.environ', {})
    @mock.patch('subprocess.Popen.__init__')
    def test_Popen_defaults(self, mockPopen):
        with mock.patch('sys.platform', 'win32'):
            subprocess2.Popen(['foo'], a=True)
            mockPopen.assert_called_with(['foo'], a=True, shell=True)

        with mock.patch('sys.platform', 'non-win32'):
            subprocess2.Popen(['foo'], a=True)
            mockPopen.assert_called_with(['foo'], a=True, shell=False)

    def test_get_english_env(self):
        with mock.patch('sys.platform', 'win32'):
            self.assertIsNone(subprocess2.get_english_env({}))

        with mock.patch('sys.platform', 'non-win32'):
            self.assertIsNone(subprocess2.get_english_env({}))
            self.assertIsNone(
                subprocess2.get_english_env({
                    'LANG': 'en_XX',
                    'LANGUAGE': 'en_YY'
                }))
            self.assertEqual({
                'LANG': 'en_US.UTF-8',
                'LANGUAGE': 'en_US.UTF-8'
            }, subprocess2.get_english_env({
                'LANG': 'bar',
                'LANGUAGE': 'baz'
            }))

    @mock.patch('subprocess2.communicate')
    def test_check_output_defaults(self, mockCommunicate):
        mockCommunicate.return_value = (('stdout', 'stderr'), 0)
        self.assertEqual('stdout', subprocess2.check_output(['foo'], a=True))
        mockCommunicate.assert_called_with(['foo'],
                                           a=True,
                                           stdin=subprocess2.DEVNULL,
                                           stdout=subprocess2.PIPE)

    @mock.patch('subprocess.Popen.__init__')
    def test_env_type(self, mockPopen):
        subprocess2.Popen(['foo'], env={b'key': b'value'})
        mockPopen.assert_called_with(['foo'],
                                     env={'key': 'value'},
                                     shell=mock.ANY)


def _run_test(with_subprocess=True):
    """Runs a tests in 12 combinations:
    - With universal_newlines=True and False.
    - With LF, CR, and CRLF output.
    - With subprocess and subprocess2.
    """
    subps = (subprocess2, subprocess) if with_subprocess else (subprocess2, )
    no_op = lambda s: s
    to_bytes = lambda s: s.encode()
    to_cr_bytes = lambda s: s.replace('\n', '\r').encode()
    to_crlf_bytes = lambda s: s.replace('\n', '\r\n').encode()

    def wrapper(test):
        def inner(self):
            for subp in subps:
                # universal_newlines = False
                test(self, to_bytes, TEST_COMMAND, False, subp)
                test(self, to_cr_bytes, TEST_COMMAND + ['--cr'], False, subp)
                test(self, to_crlf_bytes, TEST_COMMAND + ['--crlf'], False,
                     subp)
                # universal_newlines = True
                test(self, no_op, TEST_COMMAND, True, subp)
                test(self, no_op, TEST_COMMAND + ['--cr'], True, subp)
                test(self, no_op, TEST_COMMAND + ['--crlf'], True, subp)

        return inner

    return wrapper


class SmokeTests(unittest.TestCase):
    # Regression tests to ensure that subprocess and subprocess2 have the same
    # behavior.
    def _check_res(self, res, stdout, stderr, returncode):
        (out, err), code = res
        self.assertEqual(stdout, out)
        self.assertEqual(stderr, err)
        self.assertEqual(returncode, code)

    def _check_exception(self, subp, e, stdout, stderr, returncode):
        """On exception, look if the exception members are set correctly."""
        self.assertEqual(returncode, e.returncode)
        self.assertEqual(stdout, e.stdout)
        self.assertEqual(stderr, e.stderr)

    def test_check_output_no_stdout(self):
        for subp in (subprocess, subprocess2):
            with self.assertRaises(ValueError):
                # pylint: disable=unexpected-keyword-arg
                subp.check_output(TEST_COMMAND, stdout=subp.PIPE)

    def test_print_exception(self):
        with self.assertRaises(subprocess2.CalledProcessError) as e:
            subprocess2.check_output(TEST_COMMAND + ['--fail', '--stdout'])
        exception_str = str(e.exception)
        # Windows escapes backslashes so check only filename
        self.assertIn(TEST_FILENAME + ' --fail --stdout', exception_str)
        self.assertIn(str(e.exception.returncode), exception_str)
        self.assertIn(e.exception.stdout.decode('utf-8', 'ignore'),
                      exception_str)

    @_run_test()
    def test_check_output_throw_stdout(self, c, cmd, un, subp):
        with self.assertRaises(subp.CalledProcessError) as e:
            subp.check_output(cmd + ['--fail', '--stdout'],
                              universal_newlines=un)
        self._check_exception(subp, e.exception, c('A\nBB\nCCC\n'), None, 64)

    @_run_test()
    def test_check_output_throw_no_stderr(self, c, cmd, un, subp):
        with self.assertRaises(subp.CalledProcessError) as e:
            subp.check_output(cmd + ['--fail', '--stderr'],
                              universal_newlines=un)
        self._check_exception(subp, e.exception, c(''), None, 64)

    @_run_test()
    def test_check_output_throw_stderr(self, c, cmd, un, subp):
        with self.assertRaises(subp.CalledProcessError) as e:
            subp.check_output(cmd + ['--fail', '--stderr'],
                              stderr=subp.PIPE,
                              universal_newlines=un)
        self._check_exception(subp, e.exception, c(''), c('a\nbb\nccc\n'), 64)

    @_run_test()
    def test_check_output_throw_stderr_stdout(self, c, cmd, un, subp):
        with self.assertRaises(subp.CalledProcessError) as e:
            subp.check_output(cmd + ['--fail', '--stderr'],
                              stderr=subp.STDOUT,
                              universal_newlines=un)
        self._check_exception(subp, e.exception, c('a\nbb\nccc\n'), None, 64)

    def test_check_call_throw(self):
        for subp in (subprocess, subprocess2):
            with self.assertRaises(subp.CalledProcessError) as e:
                subp.check_call(TEST_COMMAND + ['--fail', '--stderr'])
            self._check_exception(subp, e.exception, None, None, 64)

    @_run_test()
    def test_redirect_stderr_to_stdout_pipe(self, c, cmd, un, subp):
        # stderr output into stdout.
        proc = subp.Popen(cmd + ['--stderr'],
                          stdout=subp.PIPE,
                          stderr=subp.STDOUT,
                          universal_newlines=un)
        res = proc.communicate(), proc.returncode
        self._check_res(res, c('a\nbb\nccc\n'), None, 0)

    @_run_test()
    def test_redirect_stderr_to_stdout(self, c, cmd, un, subp):
        # stderr output into stdout but stdout is not piped.
        proc = subp.Popen(cmd + ['--stderr'],
                          stderr=subprocess2.STDOUT,
                          universal_newlines=un)
        res = proc.communicate(), proc.returncode
        self._check_res(res, None, None, 0)

    @_run_test()
    def test_stderr(self, c, cmd, un, subp):
        cmd = ['expr', '1', '/', '0']
        if sys.platform == 'win32':
            cmd = ['cmd.exe', '/c', 'exit', '1']
        p1 = subprocess.Popen(cmd, stderr=subprocess.PIPE, shell=False)
        p2 = subprocess2.Popen(cmd, stderr=subprocess.PIPE, shell=False)
        r1 = p1.communicate()
        r2 = p2.communicate()
        self.assertEqual(r1, r2)

    @_run_test(with_subprocess=False)
    def test_stdin(self, c, cmd, un, subp):
        stdin = c('0123456789')
        res = subprocess2.communicate(cmd + ['--read'],
                                      stdin=stdin,
                                      universal_newlines=un)
        self._check_res(res, None, None, 10)

    @_run_test(with_subprocess=False)
    def test_stdin_empty(self, c, cmd, un, subp):
        stdin = c('')
        res = subprocess2.communicate(cmd + ['--read'],
                                      stdin=stdin,
                                      universal_newlines=un)
        self._check_res(res, None, None, 0)

    def test_stdin_void(self):
        res = subprocess2.communicate(TEST_COMMAND + ['--read'],
                                      stdin=subprocess2.DEVNULL)
        self._check_res(res, None, None, 0)

    @_run_test(with_subprocess=False)
    def test_stdin_void_stdout(self, c, cmd, un, subp):
        # Make sure a mix ofsubprocess2.DEVNULL andsubprocess2.PIPE works.
        res = subprocess2.communicate(cmd + ['--stdout', '--read'],
                                      stdin=subprocess2.DEVNULL,
                                      stdout=subprocess2.PIPE,
                                      universal_newlines=un,
                                      shell=False)
        self._check_res(res, c('A\nBB\nCCC\n'), None, 0)

    @_run_test(with_subprocess=False)
    def test_stdout_void(self, c, cmd, un, subp):
        res = subprocess2.communicate(cmd + ['--stdout', '--stderr'],
                                      stdout=subprocess2.DEVNULL,
                                      stderr=subprocess2.PIPE,
                                      universal_newlines=un)
        self._check_res(res, None, c('a\nbb\nccc\n'), 0)

    @_run_test(with_subprocess=False)
    def test_stderr_void(self, c, cmd, un, subp):
        res = subprocess2.communicate(cmd + ['--stdout', '--stderr'],
                                      stdout=subprocess2.PIPE,
                                      stderr=subprocess2.DEVNULL,
                                      universal_newlines=un)
        self._check_res(res, c('A\nBB\nCCC\n'), None, 0)

    @_run_test(with_subprocess=False)
    def test_stdout_void_stderr_redirect(self, c, cmd, un, subp):
        res = subprocess2.communicate(cmd + ['--stdout', '--stderr'],
                                      stdout=subprocess2.DEVNULL,
                                      stderr=subprocess2.STDOUT,
                                      universal_newlines=un)
        self._check_res(res, None, None, 0)


if __name__ == '__main__':
    unittest.main()
