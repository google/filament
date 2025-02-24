# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import datetime
import logging
import os
import random
import tempfile

import py_utils
from py_utils import cloud_storage  # pylint: disable=import-error
from telemetry.util import image_util
from telemetry.internal.util import file_handle


def TryCaptureScreenShot(platform, tab=None, timeout=None):
  """ If the platform or tab supports screenshot, attempt to take a screenshot
  of the current browser.

  Args:
    platform: current platform
    tab: browser tab if available
    timeout: An float denoting the number of seconds to wait for a successful
        screenshot. If set to None, only attempts once.

  Returns:
    file handle of the tempoerary file path for the screenshot if
    present, None otherwise.
  """
  try:
    if platform.CanTakeScreenshot():
      tf = tempfile.NamedTemporaryFile(delete=False, suffix='.png')
      tf.close()
      try:
        py_utils.WaitFor(lambda: platform.TakeScreenshot(tf.name), timeout or 0)
      except py_utils.TimeoutException:
        logging.warning('Did not succeed in screenshot capture')
      return file_handle.FromTempFile(tf)
    if tab and tab.IsAlive() and tab.screenshot_supported:
      tf = tempfile.NamedTemporaryFile(delete=False, suffix='.png')
      tf.close()
      image = tab.Screenshot()
      image_util.WritePngFile(image, tf.name)
      return file_handle.FromTempFile(tf)
    logging.warning(
        'Either tab has crashed or browser does not support taking tab '
        'screenshot. Skip taking screenshot on failure.')
    return None
  except Exception as e: # pylint: disable=broad-except
    logging.warning('Exception when trying to capture screenshot: %s', repr(e))
    return None


def TryCaptureScreenShotAndUploadToCloudStorage(platform, tab=None):
  """ If the platform or tab supports screenshot, attempt to take a screenshot
  of the current browser.  If present it uploads this local path to cloud
  storage and returns the URL of the cloud storage path.

  Args:
    platform: current platform
    tab: browser tab if available

  Returns:
    url of the cloud storage path if screenshot is present, None otherwise
  """
  fh = TryCaptureScreenShot(platform, tab)
  if fh is not None:
    return _UploadScreenShotToCloudStorage(fh)

  return None

def _GenerateRemotePath(fh):
  return ('browser-screenshot_%s-%s%-d%s' %
          (fh.id, datetime.datetime.now().strftime('%Y-%m-%d_%H-%M-%S'),
           random.randint(1, 100000), fh.extension))

def _UploadScreenShotToCloudStorage(fh):
  """ Upload the given screenshot image to cloud storage and return the
    cloud storage url if successful.
  """
  try:
    return cloud_storage.Insert(cloud_storage.TELEMETRY_OUTPUT,
                                _GenerateRemotePath(fh), fh.GetAbsPath())
  except cloud_storage.CloudStorageError as err:
    logging.error('Cloud storage error while trying to upload screenshot: %s',
                  repr(err))
    return '<Missing link>'
  finally:  # Must clean up screenshot file if exists.
    os.remove(fh.GetAbsPath())
