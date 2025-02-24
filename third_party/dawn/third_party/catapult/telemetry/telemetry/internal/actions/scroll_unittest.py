# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import division
from __future__ import absolute_import
from telemetry import decorators
from telemetry.internal.actions import page_action
from telemetry.internal.actions import scroll
from telemetry.internal.actions import utils
from telemetry.testing import tab_test_case


class ScrollActionTest(tab_test_case.TabTestCase):

  def _MakePageVerticallyScrollable(self):
    # Make page taller than window so it's scrollable vertically.
    self._tab.ExecuteJavaScript(
        'document.body.style.height ='
        '(3 * __GestureCommon_GetWindowHeight() + 1) + "px";')

  def _MakePageHorizontallyScrollable(self):
    # Make page wider than window so it's scrollable horizontally.
    self._tab.ExecuteJavaScript(
        'document.body.style.width ='
        '(3 * __GestureCommon_GetWindowWidth() + 1) + "px";')

  def setUp(self):
    tab_test_case.TabTestCase.setUp(self)
    self.Navigate('blank.html')
    utils.InjectJavaScript(self._tab, 'gesture_common.js')

  def _RunScrollDistanceTest(self, distance, speed, source, maxError):
    # TODO(bokan): Distance tests will fail on versions of Chrome that haven't
    # been fixed.  The fixes landed at the same time as the
    # setBrowserControlsShown method was added so only run the test if that's
    # available. Once that rolls into ref builds we can remove this check.
    distanceFixedInChrome = self._tab.EvaluateJavaScript(
        "'setBrowserControlsShown' in chrome.gpuBenchmarking")
    if not distanceFixedInChrome:
      return

    # Hide the URL bar so we can measure scrolled distance without worrying
    # about the URL bar consuming delta.
    self._tab.ExecuteJavaScript(
        'chrome.gpuBenchmarking.setBrowserControlsShown(false);')

    # Make the document tall enough to accomodate the requested distance but
    # also leave enough space so we can tell if the scroll overshoots the
    # target.
    screenHeight = self._tab.EvaluateJavaScript('window.visualViewport.height')
    documentHeight = (screenHeight + distance) * 2

    self._tab.ExecuteJavaScript(
        'document.body.style.height = "' + str(documentHeight) + 'px";')
    self.assertEqual(
        self._tab.EvaluateJavaScript('document.scrollingElement.scrollTop'), 0)

    # Allow for some visual viewport offset. For example, if the test doesn't
    # want any visual viewport offset due to animation handoff error between
    # the two viewports.
    start_offset = self._tab.EvaluateJavaScript('window.visualViewport.pageTop')

    i = scroll.ScrollAction(
        distance=distance,
        direction="down",
        speed_in_pixels_per_second=speed,
        synthetic_gesture_source=source)
    i.WillRunAction(self._tab)
    i.RunAction(self._tab)

    actual = self._tab.EvaluateJavaScript(
        'window.visualViewport.pageTop') - start_offset

    # TODO(bokan): setBrowserControlsShown isn't quite enough. Chrome will hide
    # the browser controls but then they animate in after a timeout. We'll need
    # to add a way to lock them to hidden. Until then, just increase the
    # allowed error.
    urlBarError = 150

    self.assertAlmostEqual(distance, actual, delta=maxError + urlBarError)

  @decorators.Disabled('chromeos', 'linux')  # crbug.com/1006789
  def testScrollDistanceFastTouch(self):
    # Just pass the test on platforms that don't support touch (i.e. Mac)
    if not page_action.IsGestureSourceTypeSupported(self._tab, 'touch'):
      return

    # Scrolling distance for touch will have some error from the excess delta
    # of the event that crosses the slop threshold but isn't applied, also
    # scroll resampling can increase the error amount..
    self._RunScrollDistanceTest(
        500000, 200000, page_action.GESTURE_SOURCE_TOUCH, 200)

  @decorators.Disabled('android-reference')  # crbug.com/934649
  def testScrollDistanceFastWheel(self):
    # Wheel scrolling will have a much greater error than touch. There's 2
    # reasons: 1) synthetic wheel gesture accumulate the sent deltas and use
    # that to determine how much delta to send at each event dispatch time.
    # This assumes that the entire sent delta is applied which is wrong due to
    # physical pixel snapping which accumulates over the gesture.
    # 2) We can only send delta as ticks of the wheel. If the total delta is
    # not a multiple of the tick size, we'll "lose" the remainder.
    self._RunScrollDistanceTest(
        500000, 200000, page_action.GESTURE_SOURCE_MOUSE, 15000)

  def testScrollDistanceSlowTouch(self):
    # Just pass the test on platforms that don't support touch (i.e. Mac)
    if not page_action.IsGestureSourceTypeSupported(self._tab, 'touch'):
      return

    # Scrolling slowly produces larger error since each event will have a
    # smaller delta. Thus error from snapping in each event will be a larger
    # share of the total delta.
    self._RunScrollDistanceTest(
        1000, 300, page_action.GESTURE_SOURCE_TOUCH, 10)

  @decorators.Disabled('android-reference')  # crbug.com/934649
  def testScrollDistanceSlowWheel(self):
    self._RunScrollDistanceTest(
        1000, 300, page_action.GESTURE_SOURCE_MOUSE, 200)

  @decorators.Disabled('android-reference')  # crbug.com/934649
  @decorators.Disabled('win-reference')  # crbug.com/805523
  @decorators.Disabled('chromeos')  # crbug.com/1450766
  def testWheelScrollDistanceWhileZoomed(self):
    # TODO(bokan): This API was added recently so only run the test once it's
    # available. Remove this check once it rolls into stable builds.
    chromeSupportsSetPageScaleFactor = self._tab.EvaluateJavaScript(
        "'setPageScaleFactor' in chrome.gpuBenchmarking")
    if not chromeSupportsSetPageScaleFactor:
      return

    self._tab.EvaluateJavaScript('chrome.gpuBenchmarking.setPageScaleFactor(2)')

    # Wheel scrolling can cause animated scrolls. This is a problem here since
    # Chrome currently doesn't hand off the animation between the visual and
    # layout viewports. To account for this, scroll the visual viewport to it's
    # maximum extent so that the entire scroll goes to the layout viewport.
    screenHeight = self._tab.EvaluateJavaScript('window.visualViewport.height')

    i = scroll.ScrollAction(
        distance=screenHeight*2,
        direction="down",
        speed_in_pixels_per_second=5000,
        synthetic_gesture_source=page_action.GESTURE_SOURCE_MOUSE)
    i.WillRunAction(self._tab)
    i.RunAction(self._tab)

    # Ensure the layout viewport isn't scrolled but the visual is.
    self.assertGreater(
        self._tab.EvaluateJavaScript('window.visualViewport.offsetTop'),
        screenHeight // 2 - 1)
    self.assertEqual(self._tab.EvaluateJavaScript('window.scrollY'), 0)

    self._RunScrollDistanceTest(
        2000, 2000, page_action.GESTURE_SOURCE_MOUSE, 60)

  def testTouchScrollDistanceWhileZoomed(self):
    # Just pass the test on platforms that don't support touch (i.e. Mac)
    if not page_action.IsGestureSourceTypeSupported(self._tab, 'touch'):
      return

    # TODO(bokan): This API was added recently so only run the test once it's
    # available. Remove this check once it rolls into stable builds.
    chromeSupportsSetPageScaleFactor = self._tab.EvaluateJavaScript(
        "'setPageScaleFactor' in chrome.gpuBenchmarking")
    if not chromeSupportsSetPageScaleFactor:
      return

    self._tab.EvaluateJavaScript('chrome.gpuBenchmarking.setPageScaleFactor(2)')
    self._RunScrollDistanceTest(
        2000, 2000, page_action.GESTURE_SOURCE_TOUCH, 20)

  def testScrollAction(self):

    self._MakePageVerticallyScrollable()
    self.assertEqual(
        self._tab.EvaluateJavaScript('document.scrollingElement.scrollTop'), 0)

    i = scroll.ScrollAction()
    i.WillRunAction(self._tab)

    self._tab.ExecuteJavaScript("""
        window.__scrollAction.beginMeasuringHook = function() {
            window.__didBeginMeasuring = true;
        };
        window.__scrollAction.endMeasuringHook = function() {
            window.__didEndMeasuring = true;
        };""")
    i.RunAction(self._tab)

    self.assertTrue(self._tab.EvaluateJavaScript('window.__didBeginMeasuring'))
    self.assertTrue(self._tab.EvaluateJavaScript('window.__didEndMeasuring'))

    scroll_position = self._tab.EvaluateJavaScript(
        'document.scrollingElement.scrollTop')
    self.assertTrue(
        scroll_position != 0, msg='scroll_position=%d;' % (scroll_position))

  # https://github.com/catapult-project/catapult/issues/3099
  @decorators.Disabled('android')
  @decorators.Disabled('chromeos')  # crbug.com/984016
  def testDiagonalScrollAction(self):
    self._MakePageVerticallyScrollable()
    self.assertEqual(
        self._tab.EvaluateJavaScript('document.scrollingElement.scrollTop'), 0)

    self._MakePageHorizontallyScrollable()
    self.assertEqual(
        self._tab.EvaluateJavaScript('document.scrollingElement.scrollLeft'), 0)

    i = scroll.ScrollAction(direction='downright')
    i.WillRunAction(self._tab)

    i.RunAction(self._tab)

    viewport_top = self._tab.EvaluateJavaScript(
        'document.scrollingElement.scrollTop')
    self.assertTrue(viewport_top != 0, msg='viewport_top=%d;' % viewport_top)

    viewport_left = self._tab.EvaluateJavaScript(
        'document.scrollingElement.scrollLeft')
    self.assertTrue(viewport_left != 0, msg='viewport_left=%d;' % viewport_left)

  def testBoundingClientRect(self):
    # Verify that the rect returned by getBoundingVisibleRect() in scroll.js is
    # completely contained within the viewport. Scroll events dispatched by the
    # scrolling API use the center of this rect as their location, and this
    # location needs to be within the viewport bounds to correctly decide
    # between main-thread and impl-thread scroll. If the scrollable area were
    # not clipped to the viewport bounds, then the instance used here (the
    # scrollable area being more than twice as tall as the viewport) would
    # result in a scroll location outside of the viewport bounds.
    self._MakePageVerticallyScrollable()
    self.assertEqual(
        self._tab.EvaluateJavaScript('document.scrollingElement.scrollTop'), 0)

    self._MakePageHorizontallyScrollable()
    self.assertEqual(
        self._tab.EvaluateJavaScript('document.scrollingElement.scrollLeft'), 0)

    self._tab.ExecuteJavaScript("""
        window.scrollTo(__GestureCommon_GetWindowWidth(),
                        __GestureCommon_GetWindowHeight());""")

    rect_top = int(
        self._tab.EvaluateJavaScript(
            '__GestureCommon_GetBoundingVisibleRect(document.body).top'))
    rect_height = int(
        self._tab.EvaluateJavaScript(
            '__GestureCommon_GetBoundingVisibleRect(document.body).height'))
    rect_bottom = rect_top + rect_height

    rect_left = int(
        self._tab.EvaluateJavaScript(
            '__GestureCommon_GetBoundingVisibleRect(document.body).left'))
    rect_width = int(
        self._tab.EvaluateJavaScript(
            '__GestureCommon_GetBoundingVisibleRect(document.body).width'))
    rect_right = rect_left + rect_width

    viewport_height = int(
        self._tab.EvaluateJavaScript('__GestureCommon_GetWindowHeight()'))
    viewport_width = int(
        self._tab.EvaluateJavaScript('__GestureCommon_GetWindowWidth()'))

    self.assertTrue(rect_top >= 0, msg='%s >= %s' % (rect_top, 0))
    self.assertTrue(rect_left >= 0, msg='%s >= %s' % (rect_left, 0))
    self.assertTrue(
        rect_bottom <= viewport_height,
        msg='%s + %s <= %s' % (rect_top, rect_height, viewport_height))
    self.assertTrue(
        rect_right <= viewport_width,
        msg='%s + %s <= %s' % (rect_left, rect_width, viewport_width))
