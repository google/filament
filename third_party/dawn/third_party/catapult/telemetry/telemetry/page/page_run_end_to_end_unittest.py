# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import shutil
import tempfile
import time
import unittest

from telemetry import story
from telemetry.core import exceptions
from telemetry.core import util
from telemetry import decorators
from telemetry.internal.browser import user_agent
from telemetry.internal.results import results_options
from telemetry.internal import story_runner
from telemetry.internal.testing.test_page_sets import example_domain
from telemetry.page import page as page_module
from telemetry.page import legacy_page_test
from telemetry.page import shared_page_state
from telemetry.page import traffic_setting as traffic_setting_module
from telemetry.util import image_util
from telemetry.testing import options_for_unittests


class DummyTest(legacy_page_test.LegacyPageTest):
  def ValidateAndMeasurePage(self, *_):
    pass


# TODO(crbug.com/1025765): Remove test cases that use real browsers and replace
# with a story_runner or shared_page_state unittest that tests the same logic.
class ActualPageRunEndToEndTests(unittest.TestCase):
  def setUp(self):
    self.options = options_for_unittests.GetRunOptions(
        output_dir=tempfile.mkdtemp())

  def tearDown(self):
    shutil.rmtree(self.options.output_dir)

  def RunStorySet(self, test, story_set, **kwargs):
    with results_options.CreateResults(self.options) as results:
      story_runner.RunStorySet(test, story_set, self.options, results, **kwargs)
    return results

  def ReadTestResults(self):
    return results_options.ReadTestResults(self.options.intermediate_dir)

  def testBrowserRestartsAfterEachPage(self):
    story_set = story.StorySet()
    story_set.AddStory(page_module.Page(
        'file://blank.html', story_set, base_dir=util.GetUnittestDataDir(),
        name='foo'))
    story_set.AddStory(page_module.Page(
        'file://blank.html', story_set, base_dir=util.GetUnittestDataDir(),
        name='bar'))

    class Test(legacy_page_test.LegacyPageTest):
      def __init__(self):
        super().__init__()
        self.browser_starts = 0
        self.platform_name = None

      def DidStartBrowser(self, browser):
        super().DidStartBrowser(browser)
        self.browser_starts += 1
        self.platform_name = browser.platform.GetOSName()

      def ValidateAndMeasurePage(self, page, tab, results):
        pass

    test = Test()
    results = self.RunStorySet(test, story_set)
    self.assertFalse(results.benchmark_interrupted)
    self.assertEqual(len(story_set), results.num_successful)
    self.assertEqual(len(story_set), test.browser_starts)

  @decorators.Disabled('chromeos')  # crbug.com/483212
  def testUserAgent(self):
    story_set = story.StorySet()
    page = page_module.Page(
        'file://blank.html', story_set, base_dir=util.GetUnittestDataDir(),
        shared_page_state_class=shared_page_state.SharedTabletPageState,
        name='blank.html')
    story_set.AddStory(page)

    class TestUserAgent(legacy_page_test.LegacyPageTest):
      def ValidateAndMeasurePage(self, page, tab, results):
        del page, results  # unused
        actual_user_agent = tab.EvaluateJavaScript(
            'window.navigator.userAgent')
        expected_user_agent = user_agent.UA_TYPE_MAPPING['tablet']
        assert actual_user_agent.strip() == expected_user_agent

        # This is so we can check later that the test actually made it into this
        # function. Previously it was timing out before even getting here, which
        # should fail, but since it skipped all the asserts, it slipped by.
        self.hasRun = True  # pylint: disable=attribute-defined-outside-init

    test = TestUserAgent()
    self.RunStorySet(test, story_set)
    self.assertTrue(hasattr(test, 'hasRun') and test.hasRun)

  # Ensure that story_runner forces exactly 1 tab before running a page.
  @decorators.Enabled('has tabs')
  def testOneTab(self):
    story_set = story.StorySet()
    page = page_module.Page(
        'file://blank.html', story_set, base_dir=util.GetUnittestDataDir(),
        name='blank.html')
    story_set.AddStory(page)

    class TestOneTab(legacy_page_test.LegacyPageTest):
      def DidStartBrowser(self, browser):
        browser.tabs.New()

      def ValidateAndMeasurePage(self, page, tab, results):
        del page, results  # unused
        assert len(tab.browser.tabs) == 1

    test = TestOneTab()
    self.RunStorySet(test, story_set)

  # Flaky crbug.com/1042080, crbug.com/1334472
  @decorators.Disabled('linux', 'mac', 'chromeos')
  def testTrafficSettings(self):
    story_set = story.StorySet()
    slow_page = page_module.Page(
        'file://green_rect.html', story_set, base_dir=util.GetUnittestDataDir(),
        name='slow',
        traffic_setting=traffic_setting_module.GPRS)
    fast_page = page_module.Page(
        'file://green_rect.html', story_set, base_dir=util.GetUnittestDataDir(),
        name='fast',
        traffic_setting=traffic_setting_module.WIFI)
    story_set.AddStory(slow_page)
    story_set.AddStory(fast_page)

    latencies_by_page_in_ms = {}

    class MeasureLatency(legacy_page_test.LegacyPageTest):
      def __init__(self):
        super().__init__()
        self._will_navigate_time = None

      def WillNavigateToPage(self, page, tab):
        del page, tab # unused
        self._will_navigate_time = time.time() * 1000

      def ValidateAndMeasurePage(self, page, tab, results):
        del results  # unused
        latencies_by_page_in_ms[page.name] = (
            time.time() * 1000 - self._will_navigate_time)

    test = MeasureLatency()
    results = self.RunStorySet(test, story_set)
    self.assertFalse(results.had_failures)
    self.assertIn('slow', latencies_by_page_in_ms)
    self.assertIn('fast', latencies_by_page_in_ms)
    slow_page_latency = traffic_setting_module.NETWORK_CONFIGS[
        slow_page.traffic_setting].round_trip_latency_ms
    fast_page_latency = traffic_setting_module.NETWORK_CONFIGS[
        fast_page.traffic_setting].round_trip_latency_ms
    # Slow page should be slower than fast page by at least (roundtrip
    # time of slow network) - (roundtrip time of fast network).
    # We add some safety margin and use |slow_page_latency/2| instead of
    # |slow_page_latency|.
    self.assertGreater(slow_page_latency/2, fast_page_latency)
    self.assertGreater(latencies_by_page_in_ms['slow'],
                       latencies_by_page_in_ms['fast'] +
                       slow_page_latency/2 - fast_page_latency)

  # Ensure that story_runner allows the test to customize the browser
  # before it launches.
  def testBrowserBeforeLaunch(self):
    story_set = story.StorySet()
    page = page_module.Page(
        'file://blank.html', story_set, base_dir=util.GetUnittestDataDir(),
        name='blank.html')
    story_set.AddStory(page)

    class TestBeforeLaunch(legacy_page_test.LegacyPageTest):
      def __init__(self):
        super().__init__()
        self._did_call_will_start = False
        self._did_call_did_start = False

      def WillStartBrowser(self, platform):
        self._did_call_will_start = True
        # TODO(simonjam): Test that the profile is available.

      def DidStartBrowser(self, browser):
        assert self._did_call_will_start
        self._did_call_did_start = True

      def ValidateAndMeasurePage(self, *_):
        assert self._did_call_did_start

    test = TestBeforeLaunch()
    self.RunStorySet(test, story_set)

  # Ensure that story_runner calls cleanUp when a page run fails.
  def testCleanUpPage(self):
    story_set = story.StorySet()
    page = page_module.Page(
        'file://blank.html', story_set, base_dir=util.GetUnittestDataDir(),
        name='blank.html')
    story_set.AddStory(page)

    class Test(legacy_page_test.LegacyPageTest):
      def __init__(self):
        super().__init__()
        self.did_call_clean_up = False

      def ValidateAndMeasurePage(self, *_):
        raise legacy_page_test.Failure

      def DidRunPage(self, platform):
        del platform  # unused
        self.did_call_clean_up = True

    test = Test()
    self.RunStorySet(test, story_set)
    assert test.did_call_clean_up

  # Ensure skipping the test if shared state cannot be run on the browser.
  def testSharedPageStateCannotRunOnBrowser(self):
    story_set = story.StorySet()

    class UnrunnableSharedState(shared_page_state.SharedPageState):
      def CanRunOnBrowser(self, browser_info, page):
        del browser_info, page  # unused
        return False

      def ValidateAndMeasurePage(self, _):
        pass

    story_set.AddStory(page_module.Page(
        url='file://blank.html', page_set=story_set,
        base_dir=util.GetUnittestDataDir(),
        shared_page_state_class=UnrunnableSharedState,
        name='blank.html'))

    class Test(legacy_page_test.LegacyPageTest):
      def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.will_navigate_to_page_called = False

      def ValidateAndMeasurePage(self, *args):
        del args  # unused
        raise Exception('Exception should not be thrown')

      def WillNavigateToPage(self, page, tab):
        del page, tab  # unused
        self.will_navigate_to_page_called = True

    test = Test()
    results = self.RunStorySet(test, story_set)

    self.assertFalse(test.will_navigate_to_page_called)
    self.assertEqual(1, results.num_expected)  # One expected skip.
    self.assertTrue(results.had_skips)
    self.assertFalse(results.had_failures)

  # Verifies that if the browser is not closed between story runs (as happens
  # e.g. on ChromeOS), the page state is reset. The first page scrolls to the
  # end, and the second page loads the same url and checks the scroll position
  # to ensure it is at the top.
  def testPageResetWhenBrowserReusedBetweenStories(self):
    class NoClosingBrowserSharedState(shared_page_state.SharedPageState):
      def ShouldReuseBrowserForAllStoryRuns(self):
        return True

    # Loads a page and scrolls it to the end.
    class ScrollingPage(page_module.Page):
      def __init__(self, url, page_set, base_dir):
        super().__init__(page_set=page_set,
                                            base_dir=base_dir,
                                            shared_page_state_class=
                                            NoClosingBrowserSharedState,
                                            url=url, name='ScrollingPage')

      def RunPageInteractions(self, action_runner):
        action_runner.ScrollPage()

    # Loads same page as ScrollingPage() and records if the scroll position is
    # at the top of the page (in was_page_at_top_on_start).
    class CheckScrollPositionPage(page_module.Page):
      def __init__(self, url, page_set, base_dir):
        super().__init__(
            page_set=page_set, base_dir=base_dir,
            shared_page_state_class=NoClosingBrowserSharedState, url=url,
            name='CheckScroll')
        self.was_page_at_top_on_start = False

      def RunPageInteractions(self, action_runner):
        scroll_y = action_runner.tab.EvaluateJavaScript('window.scrollY')
        self.was_page_at_top_on_start = scroll_y == 0

    story_set = story.StorySet()
    story_set.AddStory(ScrollingPage(
        url='file://page_with_swipeables.html', page_set=story_set,
        base_dir=util.GetUnittestDataDir()))
    test_page = CheckScrollPositionPage(
        url='file://page_with_swipeables.html', page_set=story_set,
        base_dir=util.GetUnittestDataDir())
    story_set.AddStory(test_page)
    self.RunStorySet(DummyTest(), story_set)
    self.assertTrue(test_page.was_page_at_top_on_start)

  def testSingleTabMeansCrashWillCauseFailure(self):
    class TestPage(page_module.Page):
      def RunNavigateSteps(self, _):
        raise exceptions.AppCrashException

    story_set = story.StorySet()
    for i in range(5):
      story_set.AddStory(
          TestPage('file://blank.html', story_set,
                   base_dir=util.GetUnittestDataDir(), name='foo%d' % i))

    results = self.RunStorySet(DummyTest(), story_set, max_failures=1)
    self.assertTrue(results.benchmark_interrupted)
    self.assertEqual(3, results.num_skipped)
    self.assertEqual(2, results.num_failed)  # max_failures + 1

  def testWebPageReplay(self):
    story_set = example_domain.ExampleDomainPageSet()
    body = []

    class TestWpr(legacy_page_test.LegacyPageTest):
      def ValidateAndMeasurePage(self, page, tab, results):
        del page, results  # unused
        body.append(tab.EvaluateJavaScript('document.body.innerText'))

      def DidRunPage(self, platform):
        # Force the replay server to restart between pages; this verifies that
        # the restart mechanism works.
        platform.network_controller.StopReplay()

    test = TestWpr()
    results = self.RunStorySet(test, story_set)

    self.longMessage = True
    self.assertIn('Example Domain', body[0],
                  msg='URL: %s' % story_set.stories[0].url)
    self.assertIn('Example Domain', body[1],
                  msg='URL: %s' % story_set.stories[1].url)

    self.assertEqual(2, results.num_successful)
    self.assertFalse(results.had_failures)

  @decorators.Disabled('chromeos')  # crbug.com/1031074
  @decorators.Disabled('win7')  # crbug.com/1260124
  def testScreenShotTakenForFailedPage(self):
    class FailingTestPage(page_module.Page):
      def RunNavigateSteps(self, action_runner):
        action_runner.Navigate(self._url)
        raise exceptions.AppCrashException

    story_set = story.StorySet()
    story_set.AddStory(page_module.Page('file://blank.html', story_set,
                                        name='blank.html'))
    failing_page = FailingTestPage('chrome://version', story_set,
                                   name='failing')
    story_set.AddStory(failing_page)

    self.options.browser_options.take_screenshot_for_failed_page = True
    results = self.RunStorySet(DummyTest(), story_set, max_failures=2)
    self.assertTrue(results.had_failures)
    # Check that we can find the artifact with a non-empty PNG screenshot.
    failing_run = next(run for run in self.ReadTestResults()
                       if run['testPath'].endswith('/failing'))
    screenshot = image_util.FromPngFile(
        failing_run['outputArtifacts']['screenshot.png']['filePath'])
    self.assertGreater(image_util.Width(screenshot), 0)
    self.assertGreater(image_util.Height(screenshot), 0)
