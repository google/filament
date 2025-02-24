# coding: utf-8
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import unittest
from unittest import mock

from telemetry.core import exceptions
from telemetry import decorators
from telemetry.internal.actions import action_runner as action_runner_module
from telemetry.internal.actions import page_action
from telemetry.testing import tab_test_case
from telemetry.timeline import chrome_trace_category_filter
from telemetry.timeline import tracing_config
from telemetry.util import trace_processor

import py_utils


class ActionRunnerMeasureMemoryTest(tab_test_case.TabTestCase):

  def setUp(self):
    super().setUp()
    self.action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)
    self.Navigate('blank.html')
    self._REQUESTED_DUMP_COUNT = 3

  def testWithoutTracing(self):
    with mock.patch.object(self._tab.browser, 'DumpMemory') as mock_method:
      self.assertIsNone(self.action_runner.MeasureMemory())
      self.assertFalse(mock_method.called)  # No-op with no tracing.

  def _TestWithTracing(self, deterministic_mode=False):
    trace_memory = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        filter_string='-*,blink.console,disabled-by-default-memory-infra')
    config = tracing_config.TracingConfig()
    config.enable_chrome_trace = True
    config.chrome_trace_config.SetCategoryFilter(trace_memory)
    self._browser.platform.tracing_controller.StartTracing(config)

    expected_dump_ids = []
    try:
      for _ in range(self._REQUESTED_DUMP_COUNT):
        dump_id = self.action_runner.MeasureMemory(deterministic_mode)
        expected_dump_ids.append(dump_id)
    finally:
      trace_data = self._browser.platform.tracing_controller.StopTracing()

    # Check that all dump ids are correct and different from each other.
    self.assertNotIn(None, expected_dump_ids)
    self.assertEqual(len(set(expected_dump_ids)), len(expected_dump_ids))

    actual_dump_ids = trace_processor.ExtractMemoryDumpIds(trace_data)
    self.assertTrue(set(expected_dump_ids).issubset(set(actual_dump_ids)))

  @decorators.Disabled('chromeos')  # crbug.com/1098669
  def testDeterministicMode(self):
    self._TestWithTracing(deterministic_mode=True)

  @decorators.Disabled('chromeos')  # crbug.com/1061321
  def testRealisticMode(self):
    with mock.patch.object(self.action_runner,
                           'ForceGarbageCollection') as mock_method:
      self._TestWithTracing(deterministic_mode=False)
      self.assertFalse(mock_method.called)  # No forced GC in "realistic" mode.

  def testWithFailedDump(self):
    with mock.patch.object(self._tab.browser, 'DumpMemory') as mock_method:
      mock_method.return_value = False  # Dump fails!
      with self.assertRaises(exceptions.Error):
        self._TestWithTracing()


class ActionRunnerTest(tab_test_case.TabTestCase):

  def testExecuteJavaScript(self):
    action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)
    self.Navigate('blank.html')
    action_runner.ExecuteJavaScript('var testing = 42;')
    self.assertEqual(42, self._tab.EvaluateJavaScript('testing'))

  def testWaitForNavigate(self):
    self.Navigate('page_with_link.html')
    action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)
    action_runner.ClickElement('#clickme')
    action_runner.WaitForNavigate()

    self.assertTrue(
        self._tab.EvaluateJavaScript('document.readyState == "interactive" || '
                                     'document.readyState == "complete"'))
    self.assertEqual(
        self._tab.EvaluateJavaScript('document.location.pathname;'),
        '/blank.html')

  def testNavigateBack(self):
    action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)
    self.Navigate('page_with_link.html')
    action_runner.WaitForJavaScriptCondition(
        'document.location.pathname === "/page_with_link.html"')

    # Test that after 3 navigations & 3 back navs, we have to be back at the
    # initial page
    self.Navigate('page_with_swipeables.html')
    action_runner.WaitForJavaScriptCondition(
        'document.location.pathname === "/page_with_swipeables.html"')

    self.Navigate('blank.html')
    action_runner.WaitForJavaScriptCondition(
        'document.location.pathname === "/blank.html"')

    self.Navigate('page_with_swipeables.html')
    action_runner.WaitForJavaScriptCondition(
        'document.location.pathname === "/page_with_swipeables.html"')

    action_runner.NavigateBack()
    action_runner.WaitForJavaScriptCondition(
        'document.location.pathname === "/blank.html"')

    action_runner.NavigateBack()
    action_runner.WaitForJavaScriptCondition(
        'document.location.pathname === "/page_with_swipeables.html"')

    action_runner.NavigateBack()
    action_runner.WaitForJavaScriptCondition(
        'document.location.pathname === "/page_with_link.html"')

  @decorators.Disabled('mac')  # crbug.com/855885
  def testWait(self):
    action_runner = action_runner_module.ActionRunner(self._tab)
    self.Navigate('blank.html')

    action_runner.ExecuteJavaScript(
        'window.setTimeout(function() { window.testing = 101; }, 50);')
    action_runner.Wait(0.1)
    self.assertEqual(101, self._tab.EvaluateJavaScript('window.testing'))

    action_runner.ExecuteJavaScript(
        'window.setTimeout(function() { window.testing = 102; }, 100);')
    action_runner.Wait(0.2)
    self.assertEqual(102, self._tab.EvaluateJavaScript('window.testing'))

  def testWaitForJavaScriptCondition(self):
    action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)
    self.Navigate('blank.html')

    action_runner.ExecuteJavaScript('window.testing = 219;')
    action_runner.WaitForJavaScriptCondition('window.testing == 219')
    action_runner.ExecuteJavaScript(
        'window.setTimeout(function() { window.testing = 220; }, 50);')
    action_runner.WaitForJavaScriptCondition('window.testing == 220')
    self.assertEqual(220, self._tab.EvaluateJavaScript('window.testing'))

  def testWaitForJavaScriptCondition_returnsValue(self):
    action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)
    self.Navigate('blank.html')

    action_runner.ExecuteJavaScript('window.testing = 0;')
    action_runner.WaitForJavaScriptCondition('window.testing == 0')
    action_runner.ExecuteJavaScript(
        'window.setTimeout(function() { window.testing = 42; }, 50);')
    self.assertEqual(42,
                     action_runner.WaitForJavaScriptCondition(
                         'window.testing'))

  def testWaitForElement(self):
    action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)
    self.Navigate('blank.html')

    action_runner.ExecuteJavaScript(
        '(function() {'
        '  var el = document.createElement("div");'
        '  el.id = "test1";'
        '  el.textContent = "foo";'
        '  document.body.appendChild(el);'
        '})()')
    action_runner.WaitForElement('#test1')
    action_runner.WaitForElement(text='foo')
    action_runner.WaitForElement(
        element_function='document.getElementById("test1")')
    action_runner.ExecuteJavaScript(
        'window.setTimeout(function() {'
        '  var el = document.createElement("div");'
        '  el.id = "test2";'
        '  document.body.appendChild(el);'
        '}, 50)')
    action_runner.WaitForElement('#test2')
    action_runner.ExecuteJavaScript(
        'window.setTimeout(function() {'
        '  document.getElementById("test2").textContent = "bar";'
        '}, 50)')
    action_runner.WaitForElement(text='bar')
    action_runner.ExecuteJavaScript(
        'window.setTimeout(function() {'
        '  var el = document.createElement("div");'
        '  el.id = "test3";'
        '  document.body.appendChild(el);'
        '}, 50)')
    action_runner.WaitForElement(
        element_function='document.getElementById("test3")')

  def testWaitForElementWithWrongText(self):
    action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)
    self.Navigate('blank.html')

    action_runner.ExecuteJavaScript(
        '(function() {'
        '  var el = document.createElement("div");'
        '  el.id = "test1";'
        '  el.textContent = "foo";'
        '  document.body.appendChild(el);'
        '})()')
    action_runner.WaitForElement('#test1')

    def WaitForElement():
      action_runner.WaitForElement(text='oo', timeout_in_seconds=0.2)

    self.assertRaises(py_utils.TimeoutException, WaitForElement)

  def testClickElement(self):
    self.Navigate('page_with_clickables.html')
    action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)

    action_runner.ExecuteJavaScript('valueSettableByTest = 1;')
    action_runner.ClickElement('#test')
    self.assertEqual(1, action_runner.EvaluateJavaScript('valueToTest'))

    action_runner.ExecuteJavaScript('valueSettableByTest = 2;')
    action_runner.ClickElement(text='Click/tap me')
    self.assertEqual(2, action_runner.EvaluateJavaScript('valueToTest'))

    action_runner.ExecuteJavaScript('valueSettableByTest = 3;')
    action_runner.ClickElement(
        element_function='document.body.firstElementChild;')
    self.assertEqual(3, action_runner.EvaluateJavaScript('valueToTest'))

    def WillFail():
      action_runner.ClickElement('#notfound')

    self.assertRaises(exceptions.EvaluateException, WillFail)

  @decorators.Disabled(
      'android',
      'debug',  # crbug.com/437068
      'chromeos',  # crbug.com/483212
      'win')  # catapult/issues/2282
  def testTapElement(self):
    self.Navigate('page_with_clickables.html')
    action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)

    action_runner.ExecuteJavaScript('valueSettableByTest = 1;')
    action_runner.TapElement('#test')
    self.assertEqual(1, action_runner.EvaluateJavaScript('valueToTest'))

    action_runner.ExecuteJavaScript('valueSettableByTest = 2;')
    action_runner.TapElement(text='Click/tap me')
    self.assertEqual(2, action_runner.EvaluateJavaScript('valueToTest'))

    action_runner.ExecuteJavaScript('valueSettableByTest = 3;')
    action_runner.TapElement(element_function='document.body.firstElementChild')
    self.assertEqual(3, action_runner.EvaluateJavaScript('valueToTest'))

    def WillFail():
      action_runner.TapElement('#notfound')

    self.assertRaises(exceptions.EvaluateException, WillFail)

  def testScrollToElement(self):
    self.Navigate('page_with_swipeables.html')
    action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)

    off_screen_element = 'document.querySelectorAll("#off-screen")[0]'
    top_bottom_element = 'document.querySelector("#top-bottom")'

    def viewport_comparator(element): # pylint: disable=invalid-name
      return action_runner.EvaluateJavaScript(
          """
          (function(elem) {
            var rect = elem.getBoundingClientRect();

            if (rect.bottom < 0) {
              // The bottom of the element is above the viewport.
              return -1;
            }
            if (rect.top - window.innerHeight > 0) {
              // rect.top provides the pixel offset of the element from the
              // top of the page. Because that exceeds the viewport's height,
              // we know that the element is below the viewport.
              return 1;
            }
            return 0;
          })({{ @element }});
          """,
          element=element)

    self.assertEqual(viewport_comparator(off_screen_element), 1)
    action_runner.ScrollPageToElement(
        selector='#off-screen', speed_in_pixels_per_second=5000)
    self.assertEqual(viewport_comparator(off_screen_element), 0)

    self.assertEqual(viewport_comparator(top_bottom_element), -1)
    action_runner.ScrollPageToElement(
        selector='#top-bottom',
        container_selector='body',
        speed_in_pixels_per_second=5000)
    self.assertEqual(viewport_comparator(top_bottom_element), 0)

  @decorators.Disabled(
      'android',  # crbug.com/437065.
      'chromeos')  # crbug.com/483212.
  def testScroll(self):
    if not page_action.IsGestureSourceTypeSupported(self._tab, 'touch'):
      return

    self.Navigate('page_with_swipeables.html')
    action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)

    action_runner.ScrollElement(
        selector='#left-right', direction='right', left_start_ratio=0.9)
    self.assertTrue(
        action_runner.EvaluateJavaScript(
            'document.querySelector("#left-right").scrollLeft') > 75)
    action_runner.ScrollElement(
        selector='#top-bottom', direction='down', top_start_ratio=0.9)
    self.assertTrue(
        action_runner.EvaluateJavaScript(
            'document.querySelector("#top-bottom").scrollTop') > 75)

    action_runner.ScrollPage(
        direction='right', left_start_ratio=0.9, distance=100)
    self.assertTrue(
        action_runner.EvaluateJavaScript(
            '(document.scrollingElement || document.body).scrollLeft') > 75)

  @decorators.Disabled(
      'android',   # crbug.com/437065.
      'chromeos',  # crbug.com/483212.
      'linux')     # crbug.com/1164657
  def testSwipe(self):
    if not page_action.IsGestureSourceTypeSupported(self._tab, 'touch'):
      return

    self.Navigate('page_with_swipeables.html')
    action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)

    action_runner.SwipeElement(
        selector='#left-right', direction='left', left_start_ratio=0.9)
    self.assertTrue(
        action_runner.EvaluateJavaScript(
            'document.querySelector("#left-right").scrollLeft') > 75)
    action_runner.SwipeElement(
        selector='#top-bottom', direction='up', top_start_ratio=0.9)
    self.assertTrue(
        action_runner.EvaluateJavaScript(
            'document.querySelector("#top-bottom").scrollTop') > 75)

    action_runner.SwipePage(direction='left', left_start_ratio=0.9)
    self.assertTrue(
        action_runner.EvaluateJavaScript(
            '(document.scrollingElement || document.body).scrollLeft') > 75)

  def testWaitForNetworkQuiescenceSmoke(self):
    self.Navigate('blank.html')
    action_runner = action_runner_module.ActionRunner(self._tab)
    action_runner.WaitForNetworkQuiescence()
    self.assertEqual(
        self._tab.EvaluateJavaScript('document.location.pathname;'),
        '/blank.html')

  def testEnterText(self):
    self.Navigate('blank.html')
    self._tab.ExecuteJavaScript(
        '(function() {'
        '  var elem = document.createElement("textarea");'
        '  document.body.appendChild(elem);'
        '  elem.focus();'
        '})();')

    action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)
    action_runner.EnterText('That is boring')  # That is boring|.
    action_runner.PressKey('Home')  # |That is boring.
    action_runner.PressKey('ArrowRight', repeat_count=2)  # Th|at is boring.
    action_runner.PressKey('Delete', repeat_count=2)  # Th| is boring.
    action_runner.EnterText('is')  # This| is boring.
    action_runner.PressKey('End')  # This is boring|.
    action_runner.PressKey('ArrowLeft', repeat_count=3)  # This is bor|ing.
    action_runner.PressKey('Backspace', repeat_count=3)  # This is |ing.
    action_runner.EnterText('interest')  # This is interest|ing.

    # Check that the contents of the textarea is correct. It might take a second
    # until all keystrokes have been handled by the browser (crbug.com/630017).
    self._tab.WaitForJavaScriptCondition(
        'document.querySelector("textarea").value === "This is interesting"',
        timeout=1)

    action_runner.PressKey('End')  # This is interesting|.
    action_runner.EnterText(u'ğҖ⌛XX') # This is interestingğҖ⌛XX|.
    self._tab.WaitForJavaScriptCondition(
        u'document.querySelector("textarea").value ==='
        u'"This is interestingğҖ⌛XX"',
        timeout=1)


  @decorators.Enabled('chromeos')
  def testOverviewMode(self):
    action_runner = action_runner_module.ActionRunner(
        self._tab, skip_waits=True)
    # TODO(chiniforooshan): Currently, there is no easy way to verify that the
    # browser has actually entered to/exited from the overview mode. For now, we
    # just make sure that the actions do not raise exception.
    #
    # One could at least try to capture screenshots before and after entering
    # overview mode and check if they are different, showing that something has
    # happened. But, for some reason taking CrOS screenshots is not properly
    # working.
    #
    # https://github.com/catapult-project/catapult/issues/4043

    action_runner.EnterOverviewMode()
    action_runner.ExitOverviewMode()

  def testCreateInteraction(self):
    action_runner = action_runner_module.ActionRunner(self._tab)
    self.Navigate('interaction_enabled_page.html')
    action_runner.Wait(1)
    config = tracing_config.TracingConfig()
    config.chrome_trace_config.SetLowOverheadFilter()
    config.enable_chrome_trace = True
    self._browser.platform.tracing_controller.StartTracing(config)
    with action_runner.CreateInteraction('InteractionName', repeatable=True):
      pass
    trace_data = self._browser.platform.tracing_controller.StopTracing()
    markers = trace_processor.ExtractTimelineMarkers(trace_data)
    self.assertIn('Interaction.InteractionName/repeatable', markers)


class InteractionTest(unittest.TestCase):

  def setUp(self):
    self.mock_action_runner = mock.Mock(action_runner_module.ActionRunner)

    def expected_js_call(method): # pylint: disable=invalid-name
      return mock.call.ExecuteJavaScript(
          '%s({{ marker }});' % method, marker='Interaction.ABC')

    self.expected_calls = [
        expected_js_call('console.time'),
        expected_js_call('console.timeEnd')
    ]

  def testIssuingInteractionRecordCommand(self):
    with action_runner_module.Interaction(
        self.mock_action_runner, label='ABC', flags=[]):
      pass
    self.assertEqual(self.expected_calls, self.mock_action_runner.mock_calls)

  def testExceptionRaisedInWithInteraction(self):

    class FooException(Exception):
      pass

    # Test that the Foo exception raised in the with block is propagated to the
    # caller.
    with self.assertRaises(FooException):
      with action_runner_module.Interaction(
          self.mock_action_runner, label='ABC', flags=[]):
        raise FooException()

    # Test that the end console.timeEnd(...) isn't called because exception was
    # raised.
    self.assertEqual(self.expected_calls[:1],
                     self.mock_action_runner.mock_calls)
