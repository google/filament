# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
from unittest import mock

from telemetry.testing import browser_backend_test_case
from telemetry import decorators


class CrOSBrowserBackendTest(browser_backend_test_case.BrowserBackendTestCase):

  def setUp(self):
    self.create_artifact_patcher = mock.patch(
        'telemetry.internal.results.artifact_logger.CreateArtifact')
    self.mock_create_artifact = self.create_artifact_patcher.start()
    self.addCleanup(self.create_artifact_patcher.stop)
    self.timestamp_patcher = mock.patch(
        'telemetry.internal.results.artifact_logger.GetTimestampSuffix')
    self.mock_timestamp = self.timestamp_patcher.start()
    self.addCleanup(self.timestamp_patcher.stop)
    self.mock_timestamp.return_value = 'timestamp-suffix'

  @decorators.Enabled('chromeos')
  def testCollectBrowserLogsNoLogs(self):
    """Tests that no artifacts are created if no browser logs are found."""
    with mock.patch.object(
        self._browser_backend._cri, 'GetFileContents') as mock_contents:
      mock_contents.side_effect = OSError('')
      self._browser_backend._CollectBrowserLogs(logging.DEBUG)
      self.mock_create_artifact.assert_not_called()

  @decorators.Enabled('chromeos')
  def testCollectBrowserLogsCurrentLog(self):
    """Tests that an artifact is created if only one browser log is found."""
    def GetContents(filepath):
      if 'PREVIOUS' in filepath:
        raise OSError('')
      return 'log contents'
    with mock.patch.object(
        self._browser_backend._cri, 'GetFileContents') as mock_contents:
      mock_contents.side_effect = GetContents
      self._browser_backend._CollectBrowserLogs(logging.DEBUG)
      artifact_name = 'browser_logs/browser_log-timestamp-suffix'
      merged_log = """\
#### Current Chrome Log ####

log contents

#### Previous Chrome Log ####

Did not find a previous Chrome log."""
      self.mock_create_artifact.assert_called_once_with(artifact_name,
                                                        merged_log)

  @decorators.Enabled('chromeos')
  def testCollectBrowserLogsBothLogs(self):
    """Tests that an artifact is created if both browser logs are found."""
    def GetContents(filepath):
      if 'PREVIOUS' in filepath:
        return 'previous log contents'
      return 'current log contents'
    with mock.patch.object(
        self._browser_backend._cri, 'GetFileContents') as mock_contents:
      mock_contents.side_effect = GetContents
      self._browser_backend._CollectBrowserLogs(logging.DEBUG)
      artifact_name = 'browser_logs/browser_log-timestamp-suffix'
      merged_log = """\
#### Current Chrome Log ####

current log contents

#### Previous Chrome Log ####

previous log contents"""
      self.mock_create_artifact.assert_called_once_with(artifact_name,
                                                        merged_log)

  @decorators.Enabled('chromeos')
  def testCollectBrowserLogsActualFile(self):
    """Tests that we successfully get some sort of browser log normally."""
    self._browser_backend._CollectBrowserLogs(logging.DEBUG)
    artifact_name = 'browser_logs/browser_log-timestamp-suffix'
    self.mock_create_artifact.assert_called_once_with(artifact_name, mock.ANY)

  @decorators.Enabled('chromeos')
  def testCollectUiLogsNoLogs(self):
    """Tests that no artifacts are created if no UI logs are found."""
    with mock.patch.object(
        self._browser_backend._cri, 'GetFileContents') as mock_contents:
      mock_contents.side_effect = OSError('')
      self._browser_backend._CollectUiLogs(logging.DEBUG)
      self.mock_create_artifact.assert_not_called()

  @decorators.Enabled('chromeos')
  def testCollectUiLogsLogFound(self):
    """Tests that an artifact is created if a UI log is found."""
    with mock.patch.object(
        self._browser_backend._cri, 'GetFileContents') as mock_contents:
      contents = 'ui log contents'
      mock_contents.return_value = contents
      self._browser_backend._CollectUiLogs(logging.DEBUG)
      artifact_name = 'ui_logs/ui_log-timestamp-suffix'
      self.mock_create_artifact.assert_called_once_with(artifact_name, contents)

  @decorators.Enabled('chromeos')
  def testCollectUiLogsActualFile(self):
    """Tests that we successfully get some sort of UI log normally."""
    self._browser_backend._CollectUiLogs(logging.DEBUG)
    artifact_name = 'ui_logs/ui_log-timestamp-suffix'
    self.mock_create_artifact.assert_called_once_with(artifact_name, mock.ANY)
