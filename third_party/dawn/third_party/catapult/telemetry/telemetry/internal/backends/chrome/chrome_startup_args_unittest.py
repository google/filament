# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest
from unittest import mock
import six

from telemetry import project_config
from telemetry.internal.backends.chrome import chrome_startup_args
from telemetry.internal.browser import browser_options as browser_options_module
from telemetry.util import wpr_modes


class FakeBrowserOptions(browser_options_module.BrowserOptions):
  def __init__(self, wpr_mode=wpr_modes.WPR_OFF):
    super().__init__()
    self.wpr_mode = wpr_mode
    self.browser_type = 'chrome'
    self.browser_user_agent_type = 'desktop'
    self.disable_background_networking = False
    self.disable_component_extensions_with_background_pages = False
    self.disable_default_apps = False


class FakeProjectConfig(project_config.ProjectConfig):
  def __init__(self):
    super().__init__(top_level_dir=None)

  def AdjustStartupFlags(self, args):
    # Example function that removes '--bar' flags.
    return [arg for arg in args if arg != '--bar']


class StartupArgsTest(unittest.TestCase):
  """Test expected inputs for GetBrowserStartupArgs."""

  def testAdjustStartupFlagsApplied(self):
    browser_options = FakeBrowserOptions()
    browser_options.AppendExtraBrowserArgs(['--foo', '--bar'])
    browser_options.environment = FakeProjectConfig()

    startup_args = chrome_startup_args.GetFromBrowserOptions(browser_options)
    self.assertIn('--foo', startup_args)
    self.assertNotIn('--bar', startup_args)


class ReplayStartupArgsTest(unittest.TestCase):
  """Test expected inputs for GetReplayArgs."""
  def setUp(self):
    if six.PY3:
      self.assertItemsEqual = self.assertCountEqual

  def testReplayOffGivesEmptyArgs(self):
    network_backend = mock.Mock()
    network_backend.is_open = False
    network_backend.forwarder = None

    self.assertEqual([], chrome_startup_args.GetReplayArgs(network_backend))

  def testReplayArgsBasic(self):
    network_backend = mock.Mock()
    network_backend.is_open = True
    network_backend.use_live_traffic = False
    network_backend.forwarder.remote_port = 789

    expected_args = [
        '--proxy-server=socks://127.0.0.1:789',
        '--proxy-bypass-list=<-loopback>',
        '--ignore-certificate-errors-spki-list='
        'PhrPvGIaAMmd29hj8BCZOq096yj7uMpRNHpn5PDxI6I=']
    self.assertItemsEqual(
        expected_args,
        chrome_startup_args.GetReplayArgs(network_backend))

  def testReplayArgsNoSpkiSupport(self):
    network_backend = mock.Mock()
    network_backend.is_open = True
    network_backend.use_live_traffic = False
    network_backend.forwarder.remote_port = 789

    expected_args = [
        '--proxy-server=socks://127.0.0.1:789',
        '--proxy-bypass-list=<-loopback>',
        '--ignore-certificate-errors']
    self.assertItemsEqual(
        expected_args,
        chrome_startup_args.GetReplayArgs(network_backend, False))

  def testReplayArgsUseLiveTrafficWithSpkiSupport(self):
    network_backend = mock.Mock()
    network_backend.is_open = True
    network_backend.use_live_traffic = True
    network_backend.forwarder.remote_port = 789

    expected_args = [
        '--proxy-server=socks://127.0.0.1:789',
        '--proxy-bypass-list=<-loopback>']
    self.assertItemsEqual(
        expected_args,
        chrome_startup_args.GetReplayArgs(network_backend,
                                          supports_spki_list=True))

  def testReplayArgsUseLiveTrafficWithNoSpkiSupport(self):
    network_backend = mock.Mock()
    network_backend.is_open = True
    network_backend.use_live_traffic = True
    network_backend.forwarder.remote_port = 123

    expected_args = [
        '--proxy-server=socks://127.0.0.1:123',
        '--proxy-bypass-list=<-loopback>']
    self.assertItemsEqual(
        expected_args,
        chrome_startup_args.GetReplayArgs(network_backend,
                                          supports_spki_list=False))
