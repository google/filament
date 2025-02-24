# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import shutil
import tempfile
import unittest
from unittest import mock

from telemetry import benchmark as benchmark_module
from telemetry import page as page_module
from telemetry.page import legacy_page_test
from telemetry import story as story_module
from telemetry.testing import fakes
from telemetry.testing import options_for_unittests


# pylint: disable=abstract-method
class DummyPageTest(legacy_page_test.LegacyPageTest):

  def __init__(self):
    super().__init__()
    # Without disabling the above warning, this complains that
    # ValidateAndMeasurePage is abstract; but defining it complains
    # that its definition is overridden here.
    self.ValidateAndMeasurePage = mock.Mock() # pylint: disable=invalid-name


# More end-to-end tests of Benchmark, shared_page_state and associated
# classes using telemetry.testing.fakes, to avoid needing to construct
# a real browser instance.


class FakePage(page_module.Page):

  def __init__(self, page_set):
    super().__init__(
        url='http://nonexistentserver.com/nonexistentpage.html',
        name='fake page',
        page_set=page_set,
        shared_page_state_class=fakes.FakeSharedPageState)
    self.RunNavigateSteps = mock.Mock() # pylint: disable=invalid-name
    self.RunPageInteractions = mock.Mock() # pylint: disable=invalid-name


class FakeBenchmark(benchmark_module.Benchmark):

  def __init__(self, max_failures=None):
    super().__init__(max_failures)
    self._fake_pages = []
    self._fake_story_set = story_module.StorySet()
    self._created_story_set = False
    self.validator = DummyPageTest()

  def CreatePageTest(self, options):
    return self.validator

  def GetFakeStorySet(self):
    return self._fake_story_set

  def AddFakePage(self, page):
    if self._created_story_set:
      raise Exception('Can not add any more fake pages')
    self._fake_pages.append(page)

  def CreateStorySet(self, options):
    if self._created_story_set:
      raise Exception('Can only create the story set once per FakeBenchmark')
    for page in self._fake_pages:
      self._fake_story_set.AddStory(page)
    self._created_story_set = True
    return self._fake_story_set


class FailingPage(FakePage):

  def __init__(self, page_set):
    super().__init__(page_set)
    self.RunNavigateSteps.side_effect = Exception('Deliberate exception')


class BenchmarkRunTest(unittest.TestCase):
  def setUp(self):
    self.options = options_for_unittests.GetRunOptions(
        output_dir=tempfile.mkdtemp(),
        benchmark_cls=FakeBenchmark,
        fake_browser=True)
    self.options.browser_options.platform = fakes.FakeLinuxPlatform()
    self.benchmark = FakeBenchmark()

  def tearDown(self):
    shutil.rmtree(self.options.output_dir)

  def testPassingPage(self):
    manager = mock.Mock()
    page = FakePage(self.benchmark.GetFakeStorySet())
    page.RunNavigateSteps = manager.page.RunNavigateSteps
    page.RunPageInteractions = manager.page.RunPageInteractions
    self.benchmark.validator.ValidateAndMeasurePage = (
        manager.validator.ValidateAndMeasurePage)
    self.benchmark.AddFakePage(page)
    self.assertEqual(
        self.benchmark.Run(self.options), 0, 'Test should run with no errors')
    expected = [
        mock.call.page.RunNavigateSteps(mock.ANY),
        mock.call.page.RunPageInteractions(mock.ANY),
        mock.call.validator.ValidateAndMeasurePage(page, mock.ANY, mock.ANY)
    ]
    self.assertTrue(manager.mock_calls == expected)

  def testFailingPage(self):
    page = FailingPage(self.benchmark.GetFakeStorySet())
    self.benchmark.AddFakePage(page)
    self.assertNotEqual(
        self.benchmark.Run(self.options), 0, 'Test should fail')
    self.assertFalse(page.RunPageInteractions.called)
