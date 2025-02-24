# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import six.moves.urllib.parse # pylint: disable=import-error

from telemetry.page import legacy_page_test
from telemetry.testing import legacy_page_test_case


class LegacyPageTestTests(legacy_page_test_case.LegacyPageTestCase):

  def testPageWasLoaded(self):
    class ExamplePageTest(legacy_page_test.LegacyPageTest):
      def ValidateAndMeasurePage(self, page, tab, results):
        del page, results  # Unused.
        contents = tab.EvaluateJavaScript('document.body.textContent')
        if contents.strip() != 'Hello world':
          raise legacy_page_test.MeasurementFailure(
              'Page contents were: %r' % contents)

    page_test = ExamplePageTest()
    measurements = self.RunPageTest(page_test, 'file://blank.html')
    self.assertFalse(measurements)  # No measurements are recorded

  def testPageWithQueryParamsAsMeasurements(self):
    class PageTestWithMeasurements(legacy_page_test.LegacyPageTest):
      def ValidateAndMeasurePage(self, page, tab, results):
        del page  # Unused.
        query = tab.EvaluateJavaScript('window.location.search').lstrip('?')
        for name, value in six.moves.urllib.parse.parse_qsl(query):
          results.AddMeasurement(name, 'count', int(value))

    page_test = PageTestWithMeasurements()
    measurements = self.RunPageTest(page_test, 'file://blank.html?foo=42')
    self.assertEqual(measurements['foo']['samples'], [42])

  def testPageWithFailure(self):
    class PageTestThatFails(legacy_page_test.LegacyPageTest):
      def ValidateAndMeasurePage(self, page, tab, results):
        del page, tab, results  # Unused.
        raise legacy_page_test.Failure

    page_test = PageTestThatFails()
    self.RunPageTest(page_test, 'file://blank.html', expect_status='FAIL')
