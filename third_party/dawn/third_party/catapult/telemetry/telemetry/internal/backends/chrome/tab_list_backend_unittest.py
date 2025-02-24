# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from telemetry.core import exceptions
from telemetry import decorators
from telemetry.testing import tab_test_case


class TabListBackendTest(tab_test_case.TabTestCase):

  @classmethod
  def CustomizeBrowserOptions(cls, options):
    options.AppendExtraBrowserArgs(['--disable-popup-blocking'])

  @decorators.Enabled('has tabs')
  def testNewTab(self):
    tabs = set(tab.id for tab in self.tabs)
    for _ in range(10):
      new_tab_id = self.tabs.New().id
      self.assertNotIn(new_tab_id, tabs)
      tabs.add(new_tab_id)
      new_tabs = set(tab.id for tab in self.tabs)
      self.assertEqual(tabs, new_tabs)

  @decorators.Enabled('has tabs')
  def testNewWindow(self):
    already_open_tab_ids = set(tab.id for tab in self.tabs)
    number_already_open_tabs = len(already_open_tab_ids)
    self.assertTrue(number_already_open_tabs > 0)

    new_window = self.tabs.New(in_new_window=True, timeout=1)
    self.assertNotIn(new_window.id, already_open_tab_ids)
    # Now the browser does know about the popup.
    self.assertEqual(len(self.tabs), number_already_open_tabs + 1)
    self.assertTrue(new_window in self.tabs)

  @decorators.Enabled('has tabs')
  def testTabIdMatchesContextId(self):
    # Ensure that there are two tabs.
    while len(self.tabs) < 2:
      self.tabs.New()

    # Check that the tab.id matches context_id.
    tabs = []
    for context_id in self.tabs._tab_list_backend.IterContextIds():
      tab = self.tabs.GetTabById(context_id)
      self.assertEqual(tab.id, context_id)
      tabs.append(self.tabs.GetTabById(context_id))

  # https://github.com/catapult-project/catapult/issues/3099 (Android)
  @decorators.Enabled('has tabs')
  @decorators.Disabled('android')
  @decorators.Disabled('all') # Temporary disabled for Chromium changes
  def testTabIdStableAfterTabCrash(self):
    # Ensure that there are two tabs.
    while len(self.tabs) < 2:
      self.tabs.New()

    tabs = list(self.tabs)

    # Crash the first tab.
    self.assertRaises(exceptions.DevtoolsTargetCrashException,
                      lambda: tabs[0].Navigate('chrome://crash'))

    # Fetching the second tab by id should still work. Fetching the first tab
    # should raise an exception.
    self.assertEqual(tabs[1], self.tabs.GetTabById(tabs[1].id))
    self.assertRaises(KeyError, lambda: self.tabs.GetTabById(tabs[0].id))

  @decorators.Enabled('has tabs')
  def testNewTabWithUrl(self):
    url = 'chrome://version/'
    self.assertEqual(url, self.tabs.New(url=url).url)
