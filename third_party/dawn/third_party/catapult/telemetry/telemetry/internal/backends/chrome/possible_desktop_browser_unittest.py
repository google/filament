# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import shutil
import tempfile
import unittest
from unittest import mock

from pyfakefs import fake_filesystem_unittest

from telemetry import decorators
from telemetry.internal.backends.chrome import desktop_browser_finder
from telemetry.internal.backends.chrome import desktop_browser_backend
from telemetry.internal.browser import browser_options

class PossibleDesktopBrowserTest(unittest.TestCase):
  def setUp(self):
    self._finder_options = browser_options.BrowserFinderOptions()
    self._finder_options.chrome_root = tempfile.mkdtemp()

  def tearDown(self):
    chrome_root = self._finder_options.chrome_root
    if chrome_root and os.path.exists(chrome_root):
      shutil.rmtree(self._finder_options.chrome_root, ignore_errors=True)
    profile_dir = self._finder_options.browser_options.profile_dir
    if profile_dir and os.path.exists(profile_dir):
      shutil.rmtree(self._finder_options.chrome_root, ignore_errors=True)

  @decorators.Disabled('android')
  def testCreate_Retries_DevToolsActivePort(self):
    """Tests that the retries that Create() does delete the devtools file.

    If the retries do not delete this file, then we can have a race condition
    between a second run of Chrome creating overwriting the last run's file
    and Telemetry finding it.
    """
    self.assertGreater(
        desktop_browser_finder._BROWSER_STARTUP_TRIES, 1,
        "This test should be deleted if we turn off retries.")
    fs_patcher = fake_filesystem_unittest.Patcher()
    fs_patcher.setUp()
    try:
      possible_browser = desktop_browser_finder.PossibleDesktopBrowser(
          'exact', None, None, None, None, None)
      fake_options = mock.MagicMock()

      # It is necessary to put the "run" variable inside self.
      # Otherwise I get an UnboundLocalError inside the function when I try
      # to increment it.
      # This can be replaced by using "nonlocal" once we move to python3.
      self.run = 0

      def FakeBrowserInit_FailUntilLastTry(*args, **kwargs):
        del args
        del kwargs
        self.assertTrue(possible_browser._profile_directory)
        self.assertTrue(desktop_browser_backend.DEVTOOLS_ACTIVE_PORT_FILE)
        devtools_file_path = os.path.join(
            possible_browser._profile_directory,
            desktop_browser_backend.DEVTOOLS_ACTIVE_PORT_FILE)
        self.assertFalse(
            os.path.exists(devtools_file_path),
            "SetUpEnvironment should delete the devtools file")
        fs_patcher.fs.CreateFile(devtools_file_path)
        self.assertTrue(
            os.path.exists(devtools_file_path),
            "Smoke check to make sure that CreateFile worked")
        self.run += 1
        if self.run < desktop_browser_finder._BROWSER_STARTUP_TRIES:
          raise Exception

      startup_args = (
          'telemetry.internal.backends.chrome.desktop_browser_finder.'
          'PossibleDesktopBrowser.GetBrowserStartupArgs')
      browser_backend = (
          'telemetry.internal.backends.chrome.desktop_browser_backend.'
          'DesktopBrowserBackend')
      with mock.patch(startup_args), \
          mock.patch(browser_backend):
        fake_options.dont_override_profile = False
        fake_options.profile_dir = None
        possible_browser.SetUpEnvironment(fake_options)
        with mock.patch(
            'telemetry.internal.browser.browser.Browser',
            side_effect=FakeBrowserInit_FailUntilLastTry):
          possible_browser.Create()
        self.assertEqual(self.run,
                         desktop_browser_finder._BROWSER_STARTUP_TRIES)
    finally:
      fs_patcher.tearDown()

  def testCopyProfileFilesSimple(self):
    source_path = os.path.join(self._finder_options.chrome_root, 'AUTHORS')
    with open(source_path, 'w') as f:
      f.write('foo@chromium.org')

    self._finder_options.browser_options.profile_files_to_copy = [
        (source_path, 'AUTHORS')
    ]

    possible_desktop = desktop_browser_finder.PossibleDesktopBrowser(
        'stable', self._finder_options, None, None,
        False, self._finder_options.chrome_root)
    possible_desktop.SetUpEnvironment(self._finder_options.browser_options)

    profile = possible_desktop.profile_directory
    self.assertTrue(os.path.exists(os.path.join(profile, 'AUTHORS')))

  def testExpectationTagsIncludeDebug(self):
    possible_desktop = desktop_browser_finder.PossibleDesktopBrowser(
        'debug_x64', self._finder_options, None, None,
        False, self._finder_options.chrome_root)
    self.assertIn('debug', possible_desktop.GetTypExpectationsTags())

  def testExpectationTagsIncludeRelease(self):
    possible_desktop = desktop_browser_finder.PossibleDesktopBrowser(
        'release_x64', self._finder_options, None, None,
        False, self._finder_options.chrome_root)
    self.assertIn('release', possible_desktop.GetTypExpectationsTags())

  def testCopyProfileFilesRecursive(self):
    """ Ensure copied files can create directories if needed."""

    source_path = os.path.join(self._finder_options.chrome_root, 'AUTHORS')
    with open(source_path, 'w') as f:
      f.write('foo@chromium.org')
    self._finder_options.browser_options.profile_files_to_copy = [
        (source_path, 'path/to/AUTHORS')
    ]

    possible_desktop = desktop_browser_finder.PossibleDesktopBrowser(
        'stable', self._finder_options, None, None,
        False, self._finder_options.chrome_root)
    possible_desktop.SetUpEnvironment(self._finder_options.browser_options)

    profile = possible_desktop.profile_directory
    self.assertTrue(os.path.exists(os.path.join(profile, 'path/to/AUTHORS')))

  def testCopyProfileFilesWithSeedProfile(self):
    """ Ensure copied files can co-exist with a seeded profile."""

    source_path = os.path.join(self._finder_options.chrome_root, 'AUTHORS1')
    with open(source_path, 'w') as f:
      f.write('foo@chromium.org')

    source_profile = tempfile.mkdtemp()
    self._finder_options.browser_options.profile_dir = source_profile

    existing_path = os.path.join(
        self._finder_options.browser_options.profile_dir, 'AUTHORS2')
    with open(existing_path, 'w') as f:
      f.write('bar@chromium.org')

    self._finder_options.browser_options.profile_files_to_copy = [
        (source_path, 'AUTHORS2')
    ]
    possible_desktop = desktop_browser_finder.PossibleDesktopBrowser(
        'stable', self._finder_options, None, None,
        False, self._finder_options.chrome_root)
    possible_desktop.SetUpEnvironment(self._finder_options.browser_options)

    profile = possible_desktop.profile_directory
    self.assertTrue(os.path.join(profile, 'AUTHORS1'))
    self.assertTrue(os.path.join(profile, 'AUTHORS2'))

  def testCopyProfileFilesWithSeedProfileDoesNotOverwrite(self):
    """ Ensure copied files will not overwrite existing profile files."""

    source_path = os.path.join(self._finder_options.chrome_root, 'AUTHORS')
    with open(source_path, 'w') as f:
      f.write('foo@chromium.org')

    source_profile = tempfile.mkdtemp()
    self._finder_options.browser_options.profile_dir = source_profile

    existing_path = os.path.join(
        self._finder_options.browser_options.profile_dir, 'AUTHORS')
    with open(existing_path, 'w') as f:
      f.write('bar@chromium.org')

    self._finder_options.browser_options.profile_files_to_copy = [
        (source_path, 'AUTHORS')
    ]

    possible_desktop = desktop_browser_finder.PossibleDesktopBrowser(
        'stable', self._finder_options, None, None,
        False, self._finder_options.chrome_root)
    possible_desktop.SetUpEnvironment(self._finder_options.browser_options)

    profile = possible_desktop.profile_directory
    with open(os.path.join(profile, 'AUTHORS'), 'r') as f:
      self.assertEqual('bar@chromium.org', f.read())
