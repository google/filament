# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import contextlib
import os
import shutil
import tempfile
import unittest

from telemetry.core import util
from telemetry import decorators
from telemetry.internal.browser import browser_finder
from telemetry.internal.browser import extension_to_load
from telemetry.testing import options_for_unittests


class ExtensionTest(unittest.TestCase):
  def setUp(self):
    self._options = options_for_unittests.GetCopy()

  def GetExtensionToLoad(self, ext_path):
    extension_path = os.path.join(util.GetUnittestDataDir(), ext_path)
    return extension_to_load.ExtensionToLoad(
        extension_path, self._options.browser_type)

  @contextlib.contextmanager
  def CreateBrowser(self, extensions_to_load):
    self._options.browser_options.extensions_to_load = extensions_to_load
    # TODO(https://crbug.com/354627706): Migrate extensions to MV3 and remove
    # this flag.
    self._options.browser_options.extra_browser_args.add(
        '--disable-features=ExtensionManifestV2Disabled')
    browser_to_create = browser_finder.FindBrowser(self._options)
    if not browser_to_create:
      self.skipTest("Did not find a browser that supports extensions")

    platform = browser_to_create.platform
    try:
      platform.network_controller.Open()
      with browser_to_create.BrowserSession(
          self._options.browser_options) as browser:
        yield browser
    finally:
      platform.network_controller.Close()

  @contextlib.contextmanager
  def CreateBrowserWithExtension(self, ext_path):
    load_extension = self.GetExtensionToLoad(ext_path)
    with self.CreateBrowser([load_extension]) as browser:
      extension = browser.extensions[load_extension]
      self.assertTrue(extension)
      self.assertEqual(extension.extension_id, load_extension.extension_id)
      yield browser, extension

  def testExtensionBasic(self):
    """Test ExtensionPage's ExecuteJavaScript and EvaluateJavaScript."""
    with self.CreateBrowserWithExtension('simple_extension') as (_, extension):
      self.assertTrue(
          extension.EvaluateJavaScript('chrome.runtime != null'))
      extension.ExecuteJavaScript('setTestVar("abcdef")')
      self.assertEqual('abcdef', extension.EvaluateJavaScript('_testVar'))

  def testExtensionGetByExtensionId(self):
    """Test GetByExtensionId for a simple extension with a background page."""
    with self.CreateBrowserWithExtension('simple_extension') as (
        browser, extension):
      extensions = browser.extensions.GetByExtensionId(extension.extension_id)
      self.assertEqual(1, len(extensions))
      self.assertEqual(extensions[0], extension)
      self.assertTrue(
          extensions[0].EvaluateJavaScript('chrome.runtime != null'))

  @decorators.Disabled('mac')
  def testWebApp(self):
    """Tests GetByExtensionId for a web app with multiple pages."""
    with self.CreateBrowserWithExtension('simple_app') as (browser, extension):
      extensions = browser.extensions.GetByExtensionId(extension.extension_id)
      extension_urls = {
          ext.EvaluateJavaScript('location.href;') for ext in extensions
      }
      expected_urls = {
          'chrome-extension://' + extension.extension_id + '/' + path
          for path in ['main.html', 'second.html',
                       '_generated_background_page.html']
      }
      self.assertEqual(expected_urls, extension_urls)


class NonExistentExtensionTest(ExtensionTest):
  def testNonExistentExtensionPath(self):
    """Test that a non-existent extension path will raise an exception."""
    with self.assertRaises(extension_to_load.ExtensionPathNonExistentException):
      self.GetExtensionToLoad('foo')

  def testExtensionNotLoaded(self):
    """Querying an extension that was not loaded will raise an exception"""
    load_extension = self.GetExtensionToLoad('simple_extension')
    with self.CreateBrowser([]) as browser:
      if browser.supports_extensions:
        with self.assertRaises(KeyError):
          # pylint: disable=pointless-statement
          browser.extensions[load_extension]


class MultipleExtensionTest(ExtensionTest):
  @contextlib.contextmanager
  def ClonedExtensions(self):
    """ Copy the manifest and background.js files of simple_extension to a
    number of temporary directories to load as extensions"""
    extension_dirs = [tempfile.mkdtemp() for _ in range(3)]
    try:
      src_dir = os.path.join(util.GetUnittestDataDir(), 'simple_extension')
      manifest_path = os.path.join(src_dir, 'manifest.json')
      script_path = os.path.join(src_dir, 'background.js')
      for tmp_dir in extension_dirs:
        shutil.copy(manifest_path, tmp_dir)
        shutil.copy(script_path, tmp_dir)

      yield [
          extension_to_load.ExtensionToLoad(d, self._options.browser_type)
          for d in extension_dirs]
    finally:
      for tmp_dir in extension_dirs:
        shutil.rmtree(tmp_dir)

  def testMultipleExtensions(self):
    with self.ClonedExtensions() as extensions_to_load:
      with self.CreateBrowser(extensions_to_load) as browser:
        # Test contains.
        loaded_extensions = [e for e in extensions_to_load
                             if e in browser.extensions]
        self.assertEqual(len(loaded_extensions), len(extensions_to_load))
        for load_extension in extensions_to_load:
          extension = browser.extensions[load_extension]
          self.assertTrue(extension)
          self.assertTrue(
              extension.EvaluateJavaScript('chrome.runtime != null'))
          extension.ExecuteJavaScript('setTestVar("abcdef")')
          self.assertEqual('abcdef', extension.EvaluateJavaScript('_testVar'))


class WebviewInExtensionTest(ExtensionTest):

  # Flaky on windows, hits an exception: http://crbug.com/508325
  # Flaky on macOS too: http://crbug.com/661434
  # ChromeOS: http://crbug.com/674220
  @decorators.Disabled('win', 'linux', 'mac', 'chromeos')
  def testWebviewInExtension(self):
    """Tests GetWebviewContext() for a web app containing <webview> element."""
    with self.CreateBrowserWithExtension('webview_app') as (_, extension):
      webview_contexts = extension.GetWebviewContexts()
      self.assertEqual(1, len(webview_contexts))
      webview_context = webview_contexts[0]
      webview_context.WaitForDocumentReadyStateToBeComplete()
      # Check that the context has the right url from the <webview> element.
      self.assertTrue(webview_context.GetUrl().startswith('data:'))
      # Check |test_input_id| element is accessible from the webview context.
      self.assertTrue(
          webview_context.EvaluateJavaScript(
              'document.getElementById("test_input_id") != null'))
      # Check that |test_input_id| is not accessible from outside of the
      # webview context.
      self.assertFalse(
          extension.EvaluateJavaScript(
              'document.getElementById("test_input_id") != null'))
