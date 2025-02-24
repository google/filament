# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import tempfile
import unittest
from unittest import mock

from py_utils import cloud_storage  # pylint: disable=import-error
from telemetry.testing import fakes
from telemetry.internal.util import file_handle
from telemetry.util import image_util
from telemetry.util import screenshot

class ScreenshotUtilTests(unittest.TestCase):

  def setUp(self):
    self.options = fakes.CreateBrowserFinderOptions()

  def testScreenShotTakenSupportedPlatform(self):
    fake_platform = self.options.fake_possible_browser.returned_browser.platform
    expected_png_base64 = """
      iVBORw0KGgoAAAANSUhEUgAAAAIAAAACCAIAAAD91
      JpzAAAAFklEQVR4Xg3EAQ0AAABAMP1LY3YI7l8l6A
      T8tgwbJAAAAABJRU5ErkJggg==
      """
    fake_platform.screenshot_png_data = expected_png_base64

    fh = screenshot.TryCaptureScreenShot(fake_platform, None)
    screenshot_file_path = fh.GetAbsPath()
    try:
      actual_screenshot_img = image_util.FromPngFile(screenshot_file_path)
      self.assertTrue(
          image_util.AreEqual(
              image_util.FromBase64Png(expected_png_base64),
              actual_screenshot_img))
    finally:  # Must clean up screenshot file if exists.
      os.remove(screenshot_file_path)

  def testScreenshotTimeout(self):
    fake_platform = FakeScreenshotTimeoutPlatform()
    def SetTargetCallCount(target):
      fake_platform.take_screenshot_call_count = 0
      fake_platform.target_screenshot_call_count = target

    # No timeout.
    SetTargetCallCount(None)
    try:
      fh = screenshot.TryCaptureScreenShot(fake_platform, timeout=None)
      self.assertEqual(fake_platform.take_screenshot_call_count, 1)
    finally:
      os.remove(fh.GetAbsPath())

    # Timeout, never succeeds.
    SetTargetCallCount(None)
    try:
      fh = screenshot.TryCaptureScreenShot(fake_platform, timeout=1)
      self.assertTrue(fake_platform.take_screenshot_call_count >= 2)
    finally:
      os.remove(fh.GetAbsPath())

    # Timeout, eventual success.
    SetTargetCallCount(3)
    try:
      fh = screenshot.TryCaptureScreenShot(fake_platform, timeout=10)
      self.assertEqual(fake_platform.take_screenshot_call_count, 3)
    finally:
      os.remove(fh.GetAbsPath())

  def testUploadScreenshotToCloudStorage(self):
    tf = tempfile.NamedTemporaryFile(
        suffix='.png', delete=False)
    fh1 = file_handle.FromTempFile(tf)

    local_path = '123456abcdefg.png'

    with mock.patch('py_utils.cloud_storage.Insert') as mock_insert:
      with mock.patch(
          'telemetry.util.screenshot._GenerateRemotePath',
          return_value=local_path):

        url = screenshot._UploadScreenShotToCloudStorage(fh1)
        mock_insert.assert_called_with(
            cloud_storage.TELEMETRY_OUTPUT,
            local_path,
            fh1.GetAbsPath())
        self.assertTrue(url is not None)


class FakeScreenshotTimeoutPlatform(fakes.FakePlatform):
  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)
    self.take_screenshot_call_count = 0
    self.target_screenshot_call_count = None

  @property
  def is_host_platform(self):
    return True

  def CanTakeScreenshot(self):
    return True

  def TakeScreenshot(self, _):
    self.take_screenshot_call_count += 1
    if self.target_screenshot_call_count is None:
      return False
    return self.take_screenshot_call_count >= self.target_screenshot_call_count
