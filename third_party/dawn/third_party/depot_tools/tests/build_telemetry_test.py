#!/usr/bin/env python3
# Copyright (c) 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import os
import subprocess
import sys
import tempfile
import unittest
import unittest.mock

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT_DIR)

import build_telemetry


class BuildTelemetryTest(unittest.TestCase):

    def test_check_auth(self):
        with unittest.mock.patch('subprocess.check_output') as run_mock:
            auth = {'email': 'bob@google.com'}
            run_mock.return_value = json.dumps(auth)
            self.assertEqual(build_telemetry.check_auth(), auth)

        with unittest.mock.patch('subprocess.check_output') as run_mock:
            run_mock.side_effect = subprocess.CalledProcessError(
                1, cmd=['check auth'], stderr='failed')
            self.assertEqual(build_telemetry.check_auth(), {})

        with unittest.mock.patch('subprocess.check_output') as run_mock:
            run_mock.return_value = ''
            self.assertEqual(build_telemetry.check_auth(), {})

    def test_load_and_save_config(self):
        test_countdown = 2
        with tempfile.TemporaryDirectory() as tmpdir:
            cfg_path = os.path.join(tmpdir, "build_telemetry.cfg")
            with unittest.mock.patch(
                    'build_telemetry.check_auth') as check_auth:
                check_auth.return_value = {'email': 'bob@google.com'}

                # Initial config load
                cfg = build_telemetry.load_config(cfg_path, test_countdown)
                self.assertEqual(cfg.path, cfg_path)
                self.assertTrue(cfg.is_googler)
                self.assertEqual(cfg.countdown, test_countdown)
                self.assertEqual(cfg.version, build_telemetry.VERSION)

                cfg.save()

                # 2nd config load
                cfg = build_telemetry.load_config(cfg_path, test_countdown)
                self.assertEqual(cfg.path, cfg_path)
                self.assertTrue(cfg.is_googler)
                self.assertEqual(cfg.countdown, test_countdown)
                self.assertEqual(cfg.version, build_telemetry.VERSION)

                # build_telemetry.check_auth() is an expensive call.
                # The cached result should be reused.
                check_auth.assert_called_once()

    def test_enabled(self):
        test_countdown = 2

        # Googler auto opt-in.
        with tempfile.TemporaryDirectory() as tmpdir:
            cfg_path = os.path.join(tmpdir, "build_telemetry.cfg")
            with unittest.mock.patch(
                    'build_telemetry.check_auth') as check_auth:
                check_auth.return_value = {'email': 'bob@google.com'}

                # Initial config load
                cfg = build_telemetry.load_config(cfg_path, test_countdown)
                cfg._show_notice = unittest.mock.MagicMock()
                self.assertEqual(cfg.countdown, test_countdown)

                # 1st enabled() call should print the notice and
                # change the countdown.
                self.assertTrue(cfg.enabled())
                self.assertEqual(cfg.countdown, test_countdown - 1)
                cfg._show_notice.assert_called_once()
                cfg._show_notice.reset_mock()

                # 2nd enabled() call shouldn't print the notice and
                # change the countdown.
                self.assertTrue(cfg.enabled())
                self.assertEqual(cfg.countdown, test_countdown - 1)
                cfg._show_notice.assert_not_called()

                cfg.save()

                # 2nd config load
                cfg = build_telemetry.load_config(cfg_path)
                cfg._show_notice = unittest.mock.MagicMock()
                self.assertTrue(cfg.enabled())
                self.assertEqual(cfg.countdown, test_countdown - 2)
                cfg._show_notice.assert_called_once()

                cfg.save()

                # 3rd config load
                cfg = build_telemetry.load_config(cfg_path)
                cfg._show_notice = unittest.mock.MagicMock()
                self.assertTrue(cfg.enabled())
                self.assertEqual(cfg.countdown, 0)
                cfg._show_notice.assert_not_called()

        # Googler opt-in/opt-out.
        with tempfile.TemporaryDirectory() as tmpdir:
            cfg_path = os.path.join(tmpdir, "build_telemetry.cfg")
            with unittest.mock.patch(
                    'build_telemetry.check_auth') as check_auth:
                check_auth.return_value = {'email': 'bob@google.com'}
                # After opt-out, it should not display the notice and
                # change the countdown.
                cfg = build_telemetry.load_config(cfg_path, test_countdown)
                cfg.opt_out()

                cfg = build_telemetry.load_config(cfg_path, test_countdown)
                cfg._show_notice = unittest.mock.MagicMock()
                self.assertFalse(cfg.enabled())
                self.assertEqual(cfg.countdown, test_countdown)
                cfg._show_notice.assert_not_called()

                # After opt-in, it should not display the notice and
                # change the countdown.
                cfg = build_telemetry.load_config(cfg_path, test_countdown)
                cfg.opt_in()

                cfg = build_telemetry.load_config(cfg_path, test_countdown)
                cfg._show_notice = unittest.mock.MagicMock()
                self.assertTrue(cfg.enabled())
                self.assertEqual(cfg.countdown, test_countdown)
                cfg._show_notice.assert_not_called()

        # Non-Googler
        with tempfile.TemporaryDirectory() as tmpdir:
            cfg_path = os.path.join(tmpdir, "build_telemetry.cfg")
            with unittest.mock.patch(
                    'build_telemetry.check_auth') as check_auth:
                check_auth.return_value = {'email': 'bob@example.com'}
                cfg = build_telemetry.load_config(cfg_path)
                self.assertFalse(cfg.enabled())


if __name__ == '__main__':
    unittest.main()
