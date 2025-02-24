#!/usr/bin/env vpython3
# coding=utf-8
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for fetch.py."""

import contextlib
import logging
import argparse
import os
import subprocess
import sys
import tempfile
import unittest
from io import StringIO
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import fetch


class SystemExitMock(Exception):
    pass


class TestUtilityFunctions(unittest.TestCase):
    """This test case is against utility functions"""
    def _usage_static_message(self, stdout):
        valid_fetch_config_text = 'Valid fetch configs:'
        self.assertIn(valid_fetch_config_text, stdout)

        # split[0] contains static text, whereas split[1] contains list of
        # configs
        split = stdout.split(valid_fetch_config_text)
        self.assertEqual(2, len(split))

        # verify a few fetch_configs
        self.assertIn('foo', split[1])
        self.assertNotIn('bar', split[1])

    def test_handle_args_valid_usage(self):
        response = fetch.handle_args(['filename', 'foo'])
        self.assertEqual(
            argparse.Namespace(dry_run=False,
                               nohooks=False,
                               nohistory=False,
                               force=False,
                               config='foo',
                               protocol_override=None,
                               props=[]), response)

        response = fetch.handle_args([
            'filename', '-n', '--dry-run', '--nohooks', '--no-history',
            '--force', '--protocol-override', 'sso', 'foo', '--some-param=1',
            '--bar=2'
        ])
        self.assertEqual(
            argparse.Namespace(dry_run=True,
                               nohooks=True,
                               nohistory=True,
                               force=True,
                               config='foo',
                               protocol_override='sso',
                               props=['--some-param=1', '--bar=2']), response)

        response = fetch.handle_args([
            'filename', '-n', '--dry-run', '--no-hooks', '--nohistory',
            '--force', '-p', 'sso', 'foo', '--some-param=1', '--bar=2'
        ])
        self.assertEqual(
            argparse.Namespace(dry_run=True,
                               nohooks=True,
                               nohistory=True,
                               force=True,
                               config='foo',
                               protocol_override='sso',
                               props=['--some-param=1', '--bar=2']), response)

    @mock.patch('os.path.exists', return_value=False)
    @mock.patch('sys.stdout', StringIO())
    @mock.patch('sys.exit', side_effect=SystemExitMock)
    def test_run_config_fetch_not_found(self, exit_mock, exists):
        with self.assertRaises(SystemExitMock):
            fetch.run_config_fetch('foo', [])
        exit_mock.assert_called_with(1)
        exists.assert_called_once()

        self.assertEqual(1, len(exists.call_args[0]))
        self.assertTrue(exists.call_args[0][0].endswith('foo.py'))

        stdout = sys.stdout.getvalue()
        self.assertEqual('Could not find a config for foo\n', stdout)

    def test_run_config_fetch_integration(self):
        config = fetch.run_config_fetch('depot_tools', [])
        url = 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
        spec = {
            'type': 'gclient_git',
            'gclient_git_spec': {
                'solutions': [{
                    'url': url,
                    'managed': False,
                    'name': 'depot_tools',
                    'deps_file': 'DEPS',
                }],
            }
        }
        self.assertEqual((spec, 'depot_tools'), config)

    def test_checkout_factory(self):
        with self.assertRaises(KeyError):
            fetch.CheckoutFactory('invalid', {}, {}, "root")

        gclient = fetch.CheckoutFactory('gclient', {}, {}, "root")
        self.assertTrue(isinstance(gclient, fetch.GclientCheckout))


class TestCheckout(unittest.TestCase):
    def setUp(self):
        mock.patch('sys.stdout', StringIO()).start()
        self.addCleanup(mock.patch.stopall)

        self.opts = argparse.Namespace(dry_run=False)
        self.checkout = fetch.Checkout(self.opts, {}, '')

    @contextlib.contextmanager
    def _temporary_file(self):
        """Creates a temporary file and removes it once it's out of scope"""
        name = tempfile.mktemp()
        try:
            with open(name, 'w+') as f:
                yield f
        finally:
            os.remove(name)

    def test_run_dry(self):
        self.opts.dry_run = True
        self.checkout.run(['foo-not-found'])
        self.assertEqual('Running: foo-not-found\n', sys.stdout.getvalue())

    def test_run_non_existing_command(self):
        with self.assertRaises(OSError):
            self.checkout.run(['foo-not-found'])
        self.assertEqual('Running: foo-not-found\n', sys.stdout.getvalue())

    def test_run_non_existing_command_return_stdout(self):
        with self.assertRaises(OSError):
            self.checkout.run(['foo-not-found'], return_stdout=True)
        self.assertEqual('Running: foo-not-found\n', sys.stdout.getvalue())

    @mock.patch('sys.stderr', StringIO())
    @mock.patch('sys.exit', side_effect=SystemExitMock)
    def test_run_wrong_param(self, exit_mock):
        # mocked version of sys.std* is not passed to subprocess, use temp files
        with self._temporary_file() as f:
            with self.assertRaises(subprocess.CalledProcessError):
                self.checkout.run([sys.executable, '-invalid-param'],
                                  return_stdout=True,
                                  stderr=f)
            f.seek(0)
            # Expect some message to stderr
            self.assertNotEqual('', f.read())
        self.assertEqual('', sys.stderr.getvalue())

        with self._temporary_file() as f:
            with self.assertRaises(SystemExitMock):
                self.checkout.run([sys.executable, '-invalid-param'], stderr=f)
            f.seek(0)
            # Expect some message to stderr
            self.assertNotEqual('', f.read())
        self.assertIn('Subprocess failed with return code',
                      sys.stdout.getvalue())
        exit_mock.assert_called_once()

    def test_run_return_as_value(self):
        cmd = [sys.executable, '-c', 'print("foo")']

        response = self.checkout.run(cmd, return_stdout=True)
        # we expect no response other than information about command
        self.assertNotIn('foo', sys.stdout.getvalue().split('\n'))
        # this file should be included in response
        self.assertEqual('foo', response.strip())

    def test_run_print_to_stdout(self):
        cmd = [sys.executable, '-c', 'print("foo")']

        # mocked version of sys.std* is not passed to subprocess, use temp files
        with self._temporary_file() as stdout:
            with self._temporary_file() as stderr:
                response = self.checkout.run(cmd, stdout=stdout, stderr=stderr)
                stdout.seek(0)
                stderr.seek(0)
                self.assertEqual('foo\n', stdout.read())
                self.assertEqual('', stderr.read())

        stdout = sys.stdout.getvalue()
        self.assertEqual('', response)


class TestGClientCheckout(unittest.TestCase):
    def setUp(self):
        self.run = mock.patch('fetch.Checkout.run').start()

        self.opts = argparse.Namespace(dry_run=False)
        self.checkout = fetch.GclientCheckout(self.opts, {}, '/root')

        self.addCleanup(mock.patch.stopall)

    @mock.patch('distutils.spawn.find_executable', return_value=True)
    def test_run_gclient_executable_found(self, find_executable):
        self.checkout.run_gclient('foo', 'bar', baz='qux')
        find_executable.assert_called_once_with('gclient')
        self.run.assert_called_once_with(('gclient', 'foo', 'bar'), baz='qux')

    @mock.patch('distutils.spawn.find_executable', return_value=False)
    def test_run_gclient_executable_not_found(self, find_executable):
        self.checkout.run_gclient('foo', 'bar', baz='qux')
        find_executable.assert_called_once_with('gclient')
        args = self.run.call_args[0][0]
        kargs = self.run.call_args[1]

        self.assertEqual(4, len(args))
        self.assertEqual(sys.executable, args[0])
        self.assertTrue(args[1].endswith('gclient.py'))
        self.assertEqual(('foo', 'bar'), args[2:])
        self.assertEqual({'baz': 'qux'}, kargs)


class TestGclientGitCheckout(unittest.TestCase):
    def setUp(self):
        self.run_gclient = mock.patch(
            'fetch.GclientCheckout.run_gclient').start()
        self.run_git = mock.patch('fetch.GitCheckout.run_git').start()

        self.opts = argparse.Namespace(dry_run=False,
                                       nohooks=True,
                                       nohistory=False)
        specs = {
            'solutions': [{
                'foo': 'bar',
                'baz': 1
            }, {
                'foo': False
            }],
            'with_branch_heads': True,
        }

        self.checkout = fetch.GclientGitCheckout(self.opts, specs, '/root')

        self.addCleanup(mock.patch.stopall)

    def test_init(self):
        self.checkout.init()
        self.assertEqual(2, self.run_gclient.call_count)
        self.assertEqual(2, self.run_git.call_count)

        # Verify only expected commands and ignore arguments to avoid copying
        # commands from fetch.py
        self.assertEqual(['config', 'sync'],
                         [a[0][0] for a in self.run_gclient.call_args_list])
        self.assertEqual(['config', 'config'],
                         [a[0][0] for a in self.run_git.call_args_list])

        # First call to gclient, format spec is expected to be called so "foo"
        # is expected to be present
        args = self.run_gclient.call_args_list[0][0]
        self.assertEqual('config', args[0])
        self.assertIn('foo', args[2])


if __name__ == '__main__':
    logging.basicConfig(
        level=logging.DEBUG if '-v' in sys.argv else logging.ERROR)
    unittest.main()
