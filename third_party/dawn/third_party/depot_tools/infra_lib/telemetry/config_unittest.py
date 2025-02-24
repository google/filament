# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Test the telemetry config."""

from . import config
import configparser
import pathlib
import tempfile
import unittest


class ConfigTest(unittest.TestCase):
    """Test Config class."""

    def test_create_missing_config_file(self) -> None:
        """Test Config to create missing config file."""

        with tempfile.TemporaryDirectory() as temp:
            path = pathlib.Path(temp) / "telemetry.cfg"
            cfg = config.Config(path)

            with open(path, 'r') as f:
                self.assertEqual(
                    f.read(), "[root]\nnotice_countdown = 10\n\n[trace]\n\n")
            self.assertFalse(cfg.trace_config.enabled)
            self.assertFalse(cfg.trace_config.has_enabled())
            self.assertEqual("AUTO", cfg.trace_config.enabled_reason)
            self.assertEqual(10, cfg.root_config.notice_countdown)

    def test_load_config_file(self) -> None:
        """Test Config to load config file."""

        with tempfile.TemporaryDirectory() as temp:
            path = pathlib.Path(temp) / "telemetry.cfg"
            with open(path, 'w') as f:
                f.write(
                    "[root]\nnotice_countdown = 3\n\n[trace]\nenabled = True\n\n"
                )

            cfg = config.Config(path)

            self.assertTrue(cfg.trace_config.enabled)
            self.assertEqual(3, cfg.root_config.notice_countdown)

    def test_flush_config_file_with_updates(self) -> None:
        """Test Config to write the config changes to file."""

        with tempfile.TemporaryDirectory() as temp:
            path = pathlib.Path(temp) / "telemetry.cfg"
            with open(path, 'w') as f:
                f.write(
                    "[root]\nnotice_countdown = 7\n\n[trace]\nenabled = True\n\n"
                )

            cfg = config.Config(path)

            cfg.trace_config.update(enabled=False, reason="AUTO")
            cfg.root_config.update(notice_countdown=9)
            cfg.flush()

            with open(path, 'r') as f:
                self.assertEqual(
                    f.read(),
                    "\n".join([
                        "[root]",
                        "notice_countdown = 9",
                        "",
                        "[trace]",
                        "enabled = False",
                        "enabled_reason = AUTO",
                        "",
                        "",
                    ]),
                )


def test_default_trace_config() -> None:
    """Test TraceConfig to load default values."""
    cfg = configparser.ConfigParser()
    cfg[config.TRACE_SECTION_KEY] = {}
    trace_config = config.TraceConfig(cfg)

    assert not trace_config.has_enabled()


def test_trace_config_update() -> None:
    """Test TraceConfig to update values."""
    cfg = configparser.ConfigParser()
    cfg[config.TRACE_SECTION_KEY] = {config.ENABLED_KEY: True}
    trace_config = config.TraceConfig(cfg)
    trace_config.update(enabled=False, reason="AUTO")
    assert not trace_config.enabled
    assert trace_config.enabled_reason == "AUTO"


def test_trace_config() -> None:
    """Test TraceConfig to instantiate from passed dict."""
    cfg = configparser.ConfigParser()
    cfg[config.TRACE_SECTION_KEY] = {config.ENABLED_KEY: True}
    trace_config = config.TraceConfig(cfg)

    assert trace_config.enabled
    assert trace_config.has_enabled()
    assert trace_config.enabled_reason == "AUTO"


def test_default_root_config() -> None:
    """Test RootConfig to load default values."""
    cfg = configparser.ConfigParser()
    cfg[config.ROOT_SECTION_KEY] = {}
    root_config = config.RootConfig(cfg)

    assert root_config.notice_countdown == 10


def test_root_config_update() -> None:
    """Test RootConfig to update values."""
    cfg = configparser.ConfigParser()
    cfg[config.ROOT_SECTION_KEY] = {config.NOTICE_COUNTDOWN_KEY: True}
    root_config = config.RootConfig(cfg)
    root_config.update(notice_countdown=8)
    assert root_config.notice_countdown == 8


def test_root_config() -> None:
    """Test RootConfig to instantiate from passed dict."""
    cfg = configparser.ConfigParser()
    cfg[config.ROOT_SECTION_KEY] = {config.NOTICE_COUNTDOWN_KEY: 9}
    root_config = config.RootConfig(cfg)

    assert root_config.notice_countdown == 9


if __name__ == '__main__':
    unittest.main()
