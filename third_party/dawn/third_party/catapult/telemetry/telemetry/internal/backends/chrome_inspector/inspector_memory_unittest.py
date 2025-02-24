# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry import decorators
from telemetry.testing import tab_test_case


class InspectorMemoryTest(tab_test_case.TabTestCase):

  @decorators.Enabled('has tabs')
  @decorators.Disabled('chromeos') # crbug.com/1055815 (CrOS)
  def testGetDOMStats(self):
    # Due to an issue with CrOS, we create a new tab here rather than
    # using the existing tab to get a consistent starting page on all platforms.
    self._tab = self._browser.tabs.New()

    self.Navigate('dom_counter_sample.html')

    # Prepare for LeakDetection.
    self._tab.PrepareForLeakDetection()

    # We need a GC here at the even loop in order clean up garbage. The GC call
    # below is only scheduling a GC at the event loop. We cannot nicely await
    # the JS promise here but calling through the inspector to get the DOM
    # statistics anyways pumps the message loop for us.
    self._tab.ExecuteJavaScript('gc({type:"major", execution:"async"});')

    # Document_count > 1 indicates that WebCore::Document loaded in Chrome
    # is leaking! The baseline should exactly match the numbers on:
    # internal/testing/dom_counter_sample.html
    # Please contact kouhei@, hajimehoshi@ when rebaselining.
    counts = self._tab.dom_stats
    self.assertEqual(counts['document_count'], 1, 'Document leak is detected! '
                     + 'The previous document is likely retained unexpectedly.')
    self.assertEqual(counts['node_count'], 14, 'Node leak is detected!')
    self.assertEqual(counts['event_listener_count'], 2,
                     'EventListener leak is detected!')

  @classmethod
  def CustomizeBrowserOptions(cls, options):
    options.AppendExtraBrowserArgs('--js-flags=--expose-gc')

  def testPrepareForLeakDetection(self):
    self._browser.tabs.New().PrepareForLeakDetection()
