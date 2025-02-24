# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import hashlib
import os
import shutil
import tempfile
import unittest
from unittest import mock
try:
  from StringIO import StringIO
except ImportError:
  from io import StringIO

from telemetry.internal import snap_page_util
from telemetry.testing import options_for_unittests
from telemetry.testing import tab_test_case
from telemetry.internal.browser import browser_finder
from telemetry.internal.util import path


class FakeResponse():
  def __init__(self):
    self.content = None

  def read(self):
    return self.content


class SnapPageTest(unittest.TestCase):
  def setUp(self):
    self.finder_options = options_for_unittests.GetCopy()
    browser_to_create = browser_finder.FindBrowser(self.finder_options)
    self.platform = browser_to_create.platform
    self.platform.network_controller.Open()

  def tearDown(self):
    self.platform.network_controller.Close()

  def _SnapWithDummyValuesExceptPath(self, snapshot_path):
    snap_page_util.SnapPage(self.finder_options, 'url', interactive=False,
                            snapshot_path=snapshot_path,
                            enable_browser_log=False)

  def testSnappingToInvalidSnapshotPath(self):
    with self.assertRaises(ValueError):
      self._SnapWithDummyValuesExceptPath('nosuffix')
    with self.assertRaises(ValueError):
      self._SnapWithDummyValuesExceptPath('')
    with self.assertRaises(ValueError):
      self._SnapWithDummyValuesExceptPath('foohtml')
    with self.assertRaises(ValueError):
      self._SnapWithDummyValuesExceptPath('foo.svg')
    with self.assertRaises(ValueError):
      self._SnapWithDummyValuesExceptPath('foo.xhtml')

  def testSnappingSimplePage(self):
    self.platform.SetHTTPServerDirectories(path.GetUnittestDataDir())
    html_file_path = os.path.join(path.GetUnittestDataDir(), 'green_rect.html')
    url = self.platform.http_server.UrlOf(html_file_path)
    outfile = StringIO()
    test_dir = tempfile.mkdtemp()
    try:
      snap_page_util._SnapPageToFile(
          self.finder_options, url, interactive=False,
          snapshot_path=os.path.join(test_dir, 'page.html'),
          snapshot_file=outfile, enable_browser_log=False)
      self.assertIn('id="green"', outfile.getvalue())
    finally:
      shutil.rmtree(test_dir)

  @mock.patch('six.moves.urllib.request.urlopen')
  def testSnappingPageWithImage(self, mock_urlopen):
    test_dir = tempfile.mkdtemp()
    try:
      src_html_filename = 'image.html'
      dest_html_path = os.path.join(test_dir, src_html_filename)
      shutil.copyfile(os.path.join(path.GetUnittestDataDir(),
                                   src_html_filename),
                      dest_html_path)
      self.platform.SetHTTPServerDirectories(path.GetUnittestDataDir())
      url = self.platform.http_server.UrlOf(
          os.path.join(path.GetUnittestDataDir(), src_html_filename))
      outfile = StringIO()

      # Load the test image file's content so that we can return it
      # from the mocked url request as if we'd actually fetched the
      # image from an external source.
      request_response = FakeResponse()
      expected_image_path = os.path.join(path.GetUnittestDataDir(), 'image.png')
      with open(expected_image_path, 'rb') as image_file:
        request_response.content = image_file.read()

      # Mock out the external image url fetch to return the loaded
      # test image content.
      mock_urlopen.return_value = request_response

      snap_page_util._SnapPageToFile(
          self.finder_options, url, interactive=False,
          snapshot_path=os.path.join(test_dir, src_html_filename),
          snapshot_file=outfile, enable_browser_log=False)

      self.assertIn('id="target"', outfile.getvalue())

      # Validate that the 'fetched' image was written to the
      # destination local image path.
      expected_fetched_md5 = hashlib.md5(request_response.content).hexdigest()
      actual_fetched_md5 = None
      with open(os.path.join(test_dir, 'image', '0-target.png'), 'rb') as f:
        actual_fetched_md5 = hashlib.md5(f.read()).hexdigest()
      self.assertEqual(expected_fetched_md5, actual_fetched_md5)
    finally:
      shutil.rmtree(test_dir)


class JSONTransmissionTest(tab_test_case.TabTestCase):
  def testTransmittingLargeObject(self):
    # Create a large array of 1 million elements
    json_obj = [1] * 1000000
    snap_page_util._TransmitLargeJSONToTab(
        self._tab, json_obj, 'big_array')
    self.assertEqual(self._tab.EvaluateJavaScript('big_array.length'), 1000000)
