# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import shutil
import tempfile
import unittest

from telemetry.core import exceptions
from telemetry.core import platform as platform_module
from telemetry.internal.browser import browser_finder
from telemetry.internal.results import results_options
from telemetry.internal import story_runner
from telemetry.page import page
from telemetry.page import legacy_page_test
from telemetry.page import shared_page_state
from telemetry import story as story_module
from telemetry.testing import fakes
from telemetry.testing import options_for_unittests
from telemetry.testing import test_stories
from telemetry.util import image_util
from telemetry.util import wpr_modes


class DummyTest(legacy_page_test.LegacyPageTest):

  def ValidateAndMeasurePage(self, *_):
    pass


class SharedPageStateTests(unittest.TestCase):

  def setUp(self):
    self.options = fakes.CreateBrowserFinderOptions()
    self.options.pause = None
    self.options.use_live_sites = False
    self.options.output_formats = ['none']
    self.options.suppress_gtest_report = True
    self.possible_browser = browser_finder.FindBrowser(self.options)

  def testUseLiveSitesFlagSet(self):
    self.options.use_live_sites = True
    test = DummyTest()
    run_state = shared_page_state.SharedPageState(
        test, self.options, story_module.StorySet(), self.possible_browser)
    try:
      self.assertTrue(run_state.platform.network_controller.is_open)
      self.assertEqual(run_state.platform.network_controller.wpr_mode,
                       wpr_modes.WPR_OFF)
      self.assertTrue(run_state.platform.network_controller.use_live_traffic)
    finally:
      run_state.TearDownState()

  def testUseLiveSitesFlagUnset(self):
    test = DummyTest()
    run_state = shared_page_state.SharedPageState(
        test, self.options, story_module.StorySet(), self.possible_browser)
    try:
      self.assertTrue(run_state.platform.network_controller.is_open)
      self.assertEqual(run_state.platform.network_controller.wpr_mode,
                       wpr_modes.WPR_REPLAY)
      self.assertFalse(run_state.platform.network_controller.use_live_traffic)
    finally:
      run_state.TearDownState()

  def testWPRRecordEnable(self):
    self.options.browser_options.wpr_mode = wpr_modes.WPR_RECORD
    test = DummyTest()
    run_state = shared_page_state.SharedPageState(
        test, self.options, story_module.StorySet(), self.possible_browser)
    try:
      self.assertTrue(run_state.platform.network_controller.is_open)
      self.assertEqual(run_state.platform.network_controller.wpr_mode,
                       wpr_modes.WPR_RECORD)
      self.assertFalse(run_state.platform.network_controller.use_live_traffic)
    finally:
      run_state.TearDownState()

  def testConstructorCallsSetOptions(self):
    test = DummyTest()
    run_state = shared_page_state.SharedPageState(
        test, self.options, story_module.StorySet(), self.possible_browser)
    try:
      self.assertEqual(test.options, self.options)
    finally:
      run_state.TearDownState()

  def assertUserAgentSetCorrectly(
      self, shared_page_state_class, expected_user_agent):
    story = page.Page(
        'http://www.google.com',
        shared_page_state_class=shared_page_state_class,
        name='Google')
    test = DummyTest()
    story_set = story_module.StorySet()
    story_set.AddStory(story)
    run_state = story.shared_state_class(
        test, self.options, story_set, self.possible_browser)
    try:
      browser_options = self.options.browser_options
      actual_user_agent = browser_options.browser_user_agent_type
      self.assertEqual(expected_user_agent, actual_user_agent)
    finally:
      run_state.TearDownState()

  def testPageStatesUserAgentType(self):
    self.assertUserAgentSetCorrectly(
        shared_page_state.SharedMobilePageState, 'mobile')
    if platform_module.GetHostPlatform().GetOSName() == 'chromeos':
      self.assertUserAgentSetCorrectly(
          shared_page_state.SharedDesktopPageState, 'chromeos')
    else:
      self.assertUserAgentSetCorrectly(
          shared_page_state.SharedDesktopPageState, 'desktop')
    self.assertUserAgentSetCorrectly(
        shared_page_state.SharedTabletPageState, 'tablet')
    self.assertUserAgentSetCorrectly(
        shared_page_state.Shared10InchTabletPageState, 'tablet_10_inch')
    self.assertUserAgentSetCorrectly(
        shared_page_state.SharedPageState, None)
    self.possible_browser.browser_type = 'web-engine-shell'
    self.assertUserAgentSetCorrectly(
        shared_page_state.SharedDesktopPageState, None)
    self.assertUserAgentSetCorrectly(
        shared_page_state.SharedMobilePageState, None)


class FakeBrowserStorySetRunTests(unittest.TestCase):
  """Tests that involve running story sets on a fake browser."""

  def setUp(self):
    self.options = options_for_unittests.GetRunOptions(
        output_dir=tempfile.mkdtemp(), fake_browser=True)

  def tearDown(self):
    shutil.rmtree(self.options.output_dir)

  @property
  def fake_platform(self):
    """The fake platform used by our fake browser."""
    return self.options.fake_possible_browser.returned_browser.platform

  def RunStorySetAndGetResults(self, story_set):
    dummy_test = test_stories.DummyStoryTest()
    with results_options.CreateResults(self.options) as results:
      story_runner.RunStorySet(dummy_test, story_set, self.options, results)

    test_results = results_options.ReadTestResults(
        self.options.intermediate_dir)
    self.assertEqual(len(test_results), 1)
    return test_results[0]

  def testNoScreenShotTakenForFailedPageDueToNoSupport(self):
    # The default "FakePlatform" does not support taking screenshots.
    self.assertFalse(self.fake_platform.CanTakeScreenshot())
    self.options.browser_options.take_screenshot_for_failed_page = True

    story_set = test_stories.SinglePageStorySet(
        story_run_side_effect=exceptions.AppCrashException(msg='fake crash'))
    results = self.RunStorySetAndGetResults(story_set)

    self.assertEqual(results['status'], 'FAIL')
    self.assertNotIn('screenshot.png', results['outputArtifacts'])

  def testScreenShotTakenForFailedPageOnSupportedPlatform(self):
    expected_png_base64 = ('iVBORw0KGgoAAAANSUhEUgAAAAIAAAACCAIAAAD91'
                           'JpzAAAAFklEQVR4Xg3EAQ0AAABAMP1LY3YI7l8l6A'
                           'T8tgwbJAAAAABJRU5ErkJggg==')
    self.fake_platform.screenshot_png_data = expected_png_base64
    # After setting up some fake data, now the platform supports screenshots.
    self.assertTrue(self.fake_platform.CanTakeScreenshot())
    self.options.browser_options.take_screenshot_for_failed_page = True

    story_set = test_stories.SinglePageStorySet(
        story_run_side_effect=exceptions.AppCrashException(msg='fake crash'))
    results = self.RunStorySetAndGetResults(story_set)

    self.assertEqual(results['status'], 'FAIL')
    self.assertIn('screenshot.png', results['outputArtifacts'])

    actual_screenshot_img = image_util.FromPngFile(
        results['outputArtifacts']['screenshot.png']['filePath'])
    self.assertTrue(
        image_util.AreEqual(
            image_util.FromBase64Png(expected_png_base64),
            actual_screenshot_img))
