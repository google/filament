# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Provide a TestCase to facilitate testing LegacyPageTest instances."""

from __future__ import absolute_import
import shutil
import tempfile
import unittest

from telemetry.internal.results import results_options
from telemetry.internal import story_runner
from telemetry.page import legacy_page_test
from telemetry.testing import options_for_unittests
from telemetry.testing import test_stories


class LegacyPageTestCase(unittest.TestCase):
  """A helper class to write tests for LegacyPageTest clients."""

  def setUp(self):
    self.options = options_for_unittests.GetRunOptions(
        output_dir=tempfile.mkdtemp())
    self.test_result = None

  def tearDown(self):
    shutil.rmtree(self.options.output_dir)

  @staticmethod
  def CreateStorySetForTest(url):
    # Subclasses can override this method to customize the page used for tests.
    return test_stories.SinglePageStorySet(url)

  def RunPageTest(self, page_test, url, expect_status='PASS'):
    """Run a legacy page_test on a test url and return its measurements.

    Args:
      page_test: A legacy_page_test.LegacyPageTest instance.
      url: A URL for the test page to load, usually a local 'file://' URI to be
        served from telemetry/internal/testing. Clients can override the
        static method CreateStorySetForTestFile to change this behavior.
      expect_status: A string with the expected status of the test run.

    Returns:
      A dictionary with measurements recorded by the legacy_page_test.
    """
    self.assertIsInstance(page_test, legacy_page_test.LegacyPageTest)
    page_test.CustomizeBrowserOptions(self.options.browser_options)
    story_set = self.CreateStorySetForTest(url)
    self.assertEqual(len(story_set), 1)
    with results_options.CreateResults(self.options) as results:
      story_runner.RunStorySet(page_test, story_set, self.options, results)
    test_results = results_options.ReadTestResults(
        self.options.intermediate_dir)
    self.assertEqual(len(test_results), 1)
    self.test_result = test_results[0]
    self.assertEqual(self.test_result['status'], expect_status)
    return results_options.ReadMeasurements(self.test_result)
