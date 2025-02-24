# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest

from telemetry.timeline import chrome_trace_category_filter


class ChromeTraceCategoryFilterTest(unittest.TestCase):
  def CheckBasicCategoryFilters(self, cf):
    self.assertEqual({'x'}, set(cf.included_categories))
    self.assertEqual({'y'}, set(cf.excluded_categories))
    self.assertEqual({'disabled-by-default-z'},
                     set(cf.disabled_by_default_categories))
    self.assertEqual({'DELAY(7;foo)'}, set(cf.synthetic_delays))

    self.assertTrue('x' in cf.filter_string)
    self.assertEqual('x,disabled-by-default-z,-y,DELAY(7;foo)',
                     cf.stable_filter_string)

  def testBasic(self):
    cf = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        'x,-y,disabled-by-default-z,DELAY(7;foo)')
    self.CheckBasicCategoryFilters(cf)

  def testBasicWithSpace(self):
    cf = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        ' x ,\n-y\t,disabled-by-default-z ,DELAY(7;foo)')
    self.CheckBasicCategoryFilters(cf)

  def testAddFilter(self):
    cf = chrome_trace_category_filter.ChromeTraceCategoryFilter()
    cf.AddFilter('x')
    cf.AddFilter('-y')
    cf.AddFilter('disabled-by-default-z')
    cf.AddFilter('DELAY(7;foo)')
    self.CheckBasicCategoryFilters(cf)

  def testNoneAndEmptyCategory(self):
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter()
    self.assertEqual(a.stable_filter_string, '')
    self.assertEqual(a.filter_string, '')
    self.assertEqual(str(a.GetDictForChromeTracing()), '{}')

    # Initializing chrome trace category filter with empty string is the same
    # as initialization with None.
    b = chrome_trace_category_filter.ChromeTraceCategoryFilter(filter_string='')
    self.assertEqual(b.stable_filter_string, '')
    self.assertEqual(b.filter_string, '')
    self.assertEqual(str(b.GetDictForChromeTracing()), '{}')

  def testAddIncludedCategory(self):
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter()
    a.AddIncludedCategory('foo')
    a.AddIncludedCategory('bar')
    a.AddIncludedCategory('foo')
    self.assertEqual(a.stable_filter_string, 'bar,foo')

  def testAddExcludedCategory(self):
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter()
    a.AddExcludedCategory('foo')
    a.AddExcludedCategory('bar')
    a.AddExcludedCategory('foo')
    self.assertEqual(a.stable_filter_string, '-bar,-foo')

  def testIncludeAndExcludeCategoryRaisesAssertion(self):
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter()
    a.AddIncludedCategory('foo')
    self.assertRaises(AssertionError, a.AddExcludedCategory, 'foo')

    a = chrome_trace_category_filter.ChromeTraceCategoryFilter()
    a.AddExcludedCategory('foo')
    self.assertRaises(AssertionError, a.AddIncludedCategory, 'foo')

    self.assertRaises(AssertionError,
                      chrome_trace_category_filter.ChromeTraceCategoryFilter,
                      'foo,-foo')

    self.assertRaises(AssertionError,
                      chrome_trace_category_filter.ChromeTraceCategoryFilter,
                      '-foo,foo')


  def testIsSubset(self):
    b = chrome_trace_category_filter.ChromeTraceCategoryFilter()
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter()
    self.assertEqual(a.IsSubset(b), True)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter()
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter("test1,test2")
    self.assertEqual(a.IsSubset(b), True)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter()
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter("-test1,-test2")
    self.assertEqual(a.IsSubset(b), True)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter("test1,test2")
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter()
    self.assertEqual(a.IsSubset(b), None)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter()
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter("test*")
    self.assertEqual(a.IsSubset(b), None)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter("test?")
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter()
    self.assertEqual(a.IsSubset(b), None)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter("test1")
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter("test1,test2")
    self.assertEqual(a.IsSubset(b), False)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter("-test1")
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter("test1")
    self.assertEqual(a.IsSubset(b), False)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter("test1,test2")
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter("test2,test1")
    self.assertEqual(a.IsSubset(b), True)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter("-test1,-test2")
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter("-test2")
    self.assertEqual(a.IsSubset(b), False)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "disabled-by-default-test1")
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "disabled-by-default-test1,disabled-by-default-test2")
    self.assertEqual(a.IsSubset(b), False)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "disabled-by-default-test1")
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "disabled-by-default-test2")
    self.assertEqual(a.IsSubset(b), False)

  def testIsSubsetWithSyntheticDelays(self):
    b = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "DELAY(foo;0.016)")
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "DELAY(foo;0.016)")
    self.assertEqual(a.IsSubset(b), True)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "DELAY(foo;0.016)")
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter()
    self.assertEqual(a.IsSubset(b), True)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter()
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "DELAY(foo;0.016)")
    self.assertEqual(a.IsSubset(b), False)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "DELAY(foo;0.016)")
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "DELAY(foo;0.032)")
    self.assertEqual(a.IsSubset(b), False)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "DELAY(foo;0.016;static)")
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "DELAY(foo;0.016;oneshot)")
    self.assertEqual(a.IsSubset(b), False)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "DELAY(foo;0.016),DELAY(bar;0.1)")
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "DELAY(bar;0.1),DELAY(foo;0.016)")
    self.assertEqual(a.IsSubset(b), True)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "DELAY(foo;0.016),DELAY(bar;0.1)")
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "DELAY(bar;0.1)")
    self.assertEqual(a.IsSubset(b), True)

    b = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "DELAY(foo;0.016),DELAY(bar;0.1)")
    a = chrome_trace_category_filter.ChromeTraceCategoryFilter(
        "DELAY(foo;0.032),DELAY(bar;0.1)")
    self.assertEqual(a.IsSubset(b), False)
