# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import six

from py_trace_event import trace_event

# Export story_test.Failure to this page_test module
from telemetry.web_perf.story_test import Failure


class TestNotSupportedOnPlatformError(Exception):
  """LegacyPageTest Exception raised when a required feature is unavailable.

  The feature required to run the test could be part of the platform,
  hardware configuration, or browser.
  """


class MeasurementFailure(Failure):
  """Exception raised when an undesired but designed-for problem."""


LegacyPageTestBase = six.with_metaclass(trace_event.TracedMetaClass, object)

class LegacyPageTest(LegacyPageTestBase):
  """A class styled on unittest.TestCase for creating page-specific tests.

  Note that this method of measuring browser's performance is obsolete and only
  here for "historical" reason. For your performance measurement need, please
  use TimelineBasedMeasurement instead: https://goo.gl/eMvikK

  For correctness testing, please use
  serially_executed_browser_test_case.SeriallyExecutedBrowserTestCase
  instead. See examples in:
  https://github.com/catapult-project/catapult/tree/master/telemetry/examples/browser_tests

  Test should override ValidateAndMeasurePage to perform test
  validation and page measurement as necessary.

     class BodyChildElementMeasurement(LegacyPageTest):
       def ValidateAndMeasurePage(self, page, tab, results):
         body_child_count = tab.EvaluateJavaScript(
             'document.body.children.length')
         results.AddMeasurement('body_children', 'count', body_child_count)
  """

  if six.PY2:
    __metaclass__ = trace_event.TracedMetaClass

  def __init__(self):
    self.options = None

  def CustomizeBrowserOptions(self, options):
    """Override to add test-specific options to the BrowserOptions object"""

  def WillStartBrowser(self, platform):
    """Override to manipulate the browser environment before it launches."""

  def DidStartBrowser(self, browser):
    """Override to customize the browser right after it has launched."""

  def SetOptions(self, options):
    """Sets the BrowserFinderOptions instance to use."""
    self.options = options

  def WillNavigateToPage(self, page, tab):
    """Override to do operations before the page is navigated, notably Telemetry
    will already have performed the following operations on the browser before
    calling this function:
    * Ensure only one tab is open.
    * Call WaitForDocumentReadyStateToComplete on the tab."""

  def DidNavigateToPage(self, page, tab):
    """Override to do operations right after the page is navigated and after
    all waiting for completion has occurred."""

  def DidRunPage(self, platform):
    """Called after the test run method was run, even if it failed."""

  def ValidateAndMeasurePage(self, page, tab, results):
    """Override to check test assertions and perform measurement.

    Implementations should call results.AddMeasurement(...) to record
    measurements associated with the current page run. Raise an exception or
    call results.Fail upon failure. legacy_page_test.py also provides several
    base exception classes to use.

    Put together:
      def ValidateAndMeasurePage(self, page, tab, results):
        res = tab.EvaluateJavaScript('2 + 2')
        if res != 4:
          raise Exception('Oh, wow.')
        results.AddMeasurement('two_plus_two', 'count', res)

    Args:
      page: A telemetry.page.Page instance.
      tab: A telemetry.core.Tab instance.
      results: A telemetry.results.PageTestResults instance.
    """
    raise NotImplementedError
