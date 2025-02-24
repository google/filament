# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from functools import wraps
import logging
import os
import types
import unittest
import six

from telemetry.internal.browser import browser_finder
from telemetry.internal.util import path
from telemetry.testing import options_for_unittests


class _MetaBrowserTestCase(type):
  """Metaclass for BrowserTestCase.

  The metaclass wraps all test* methods of all subclasses of BrowserTestCase to
  print browser standard output and log upon failure.
  """

  def __new__(cls, name, bases, dct):
    new_dct = {}
    for attributeName, attribute in six.iteritems(dct):
      if (isinstance(attribute, types.FunctionType) and
          attributeName.startswith('test')):
        attribute = cls._PrintBrowserStandardOutputAndLogOnFailure(attribute)
      new_dct[attributeName] = attribute
    return type.__new__(cls, name, bases, new_dct)

  @staticmethod
  def _PrintBrowserStandardOutputAndLogOnFailure(method):
    @wraps(method)
    def WrappedMethod(self):
      try:  # pylint: disable=broad-except
        method(self)
      except Exception: # pylint: disable=broad-except
        if self._browser:
          self._browser.DumpStateUponFailure()
        else:
          logging.warning('Cannot dump browser state: No browser.')
        raise
    return WrappedMethod


class BrowserTestCase(
    six.with_metaclass(_MetaBrowserTestCase, unittest.TestCase)):
  _possible_browser = None
  _platform = None
  _browser = None
  _device = None

  def setUp(self):
    if self._browser:
      self._browser.CleanupUnsymbolizedMinidumps()

  def tearDown(self):
    if self._browser:
      self._browser.CleanupUnsymbolizedMinidumps(fatal=True)

  @classmethod
  def setUpClass(cls):
    try:
      options = options_for_unittests.GetCopy()
      cls.CustomizeBrowserOptions(options.browser_options)
      cls._possible_browser = browser_finder.FindBrowser(options)
      if not cls._possible_browser:
        raise Exception('No browser found, cannot continue test.')
      cls._platform = cls._possible_browser.platform
      cls._platform.network_controller.Open()
      cls._possible_browser.SetUpEnvironment(options.browser_options)
      cls._browser = cls._possible_browser.Create()
      cls._device = options.remote_platform_options.device
    except:
      # Try to tear down the class upon any errors during set up.
      cls.tearDownClass()
      raise

  @classmethod
  def tearDownClass(cls):
    cls._device = None
    if cls._browser is not None:
      cls._browser.Close()
      cls._browser = None
    if cls._possible_browser is not None:
      cls._possible_browser.CleanUpEnvironment()
      cls._possible_browser = None
    if cls._platform is not None:
      cls._platform.StopAllLocalServers()
      cls._platform.network_controller.Close()
      cls._platform = None

  @classmethod
  def CustomizeBrowserOptions(cls, options):
    """Override to add test-specific options to the BrowserOptions object"""

  @classmethod
  def UrlOfUnittestFile(cls, filename, handler_class=None):
    cls._platform.SetHTTPServerDirectories(path.GetUnittestDataDir(),
                                           handler_class)
    file_path = os.path.join(path.GetUnittestDataDir(), filename)
    return cls._platform.http_server.UrlOf(file_path)
