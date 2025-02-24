# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from tracing.value import histogram
from tracing.value import histogram_grouping
from tracing.value.diagnostics import date_range
from tracing.value.diagnostics import generic_set
from tracing.value.diagnostics import reserved_infos

class HistogramGroupingUnittest(unittest.TestCase):

  def testBooleanTags(self):
    a_hist = histogram.Histogram('', 'count')
    a_hist.diagnostics[reserved_infos.STORY_TAGS.name] = generic_set.GenericSet(
        ['video', 'audio'])
    b_hist = histogram.Histogram('', 'count')
    b_hist.diagnostics[reserved_infos.STORY_TAGS.name] = generic_set.GenericSet(
        ['audio'])
    c_hist = histogram.Histogram('', 'count')
    c_hist.diagnostics[reserved_infos.STORY_TAGS.name] = generic_set.GenericSet(
        ['video'])
    d_hist = histogram.Histogram('', 'count')
    d_hist.diagnostics[reserved_infos.STORY_TAGS.name] = generic_set.GenericSet(
        [])
    groupings = histogram_grouping.BuildFromTags(
        ['audio', 'video'], reserved_infos.STORY_TAGS.name)
    self.assertEqual(len(groupings), 2)
    groupings.sort(key=lambda g: g.key)
    self.assertEqual(groupings[0].key, 'audioTag')
    self.assertEqual(groupings[1].key, 'videoTag')
    self.assertEqual(groupings[0].callback(a_hist), 'audio')
    self.assertEqual(groupings[0].callback(b_hist), 'audio')
    self.assertEqual(groupings[0].callback(c_hist), '~audio')
    self.assertEqual(groupings[0].callback(d_hist), '~audio')
    self.assertEqual(groupings[1].callback(a_hist), 'video')
    self.assertEqual(groupings[1].callback(b_hist), '~video')
    self.assertEqual(groupings[1].callback(c_hist), 'video')
    self.assertEqual(groupings[1].callback(d_hist), '~video')

  def testKeyValueTags(self):
    a_hist = histogram.Histogram('', 'count')
    a_hist.diagnostics[reserved_infos.STORY_TAGS.name] = generic_set.GenericSet(
        ['case:load'])
    b_hist = histogram.Histogram('', 'count')
    b_hist.diagnostics[reserved_infos.STORY_TAGS.name] = generic_set.GenericSet(
        ['case:browse'])
    c_hist = histogram.Histogram('', 'count')
    c_hist.diagnostics[reserved_infos.STORY_TAGS.name] = generic_set.GenericSet(
        [])
    d_hist = histogram.Histogram('', 'count')
    d_hist.diagnostics[reserved_infos.STORY_TAGS.name] = generic_set.GenericSet(
        ['case:load', 'case:browse'])
    groupings = histogram_grouping.BuildFromTags(
        ['case:load', 'case:browse'], reserved_infos.STORY_TAGS.name)
    self.assertEqual(len(groupings), 1)
    self.assertEqual(groupings[0].key, 'caseTag')
    self.assertEqual(groupings[0].callback(a_hist), 'load')
    self.assertEqual(groupings[0].callback(b_hist), 'browse')
    self.assertEqual(groupings[0].callback(c_hist), '~case')
    self.assertEqual(groupings[0].callback(d_hist), 'browse,load')

  def testName(self):
    self.assertEqual(histogram_grouping.HISTOGRAM_NAME.callback(
        histogram.Histogram('test', 'count')), 'test')

  def testDisplayLabel(self):
    hist = histogram.Histogram('test', 'count')
    self.assertEqual(histogram_grouping.DISPLAY_LABEL.callback(hist), 'Value')
    hist.diagnostics[reserved_infos.LABELS.name] = generic_set.GenericSet(['H'])
    self.assertEqual(histogram_grouping.DISPLAY_LABEL.callback(hist), 'H')

  def testGenericSet(self):
    grouping = histogram_grouping.GenericSetGrouping('foo')
    hist = histogram.Histogram('', 'count')
    self.assertEqual(grouping.callback(hist), '')
    hist.diagnostics['foo'] = generic_set.GenericSet(['baz'])
    self.assertEqual(grouping.callback(hist), 'baz')
    hist.diagnostics['foo'] = generic_set.GenericSet(['baz', 'bar'])
    self.assertEqual(grouping.callback(hist), 'bar,baz')

  def testReservedGenericSetGroupings(self):
    self.assertIsInstance(
        histogram_grouping.GROUPINGS_BY_KEY[reserved_infos.ARCHITECTURES.name],
        histogram_grouping.GenericSetGrouping)
    self.assertIsInstance(histogram_grouping.GROUPINGS_BY_KEY[
        reserved_infos.BENCHMARKS.name], histogram_grouping.GenericSetGrouping)
    self.assertIsInstance(histogram_grouping.GROUPINGS_BY_KEY[
        reserved_infos.BOTS.name], histogram_grouping.GenericSetGrouping)
    self.assertIsInstance(histogram_grouping.GROUPINGS_BY_KEY[
        reserved_infos.BUILDS.name], histogram_grouping.GenericSetGrouping)
    self.assertIsInstance(histogram_grouping.GROUPINGS_BY_KEY[
        reserved_infos.MASTERS.name], histogram_grouping.GenericSetGrouping)
    self.assertIsInstance(
        histogram_grouping.GROUPINGS_BY_KEY[reserved_infos.MEMORY_AMOUNTS.name],
        histogram_grouping.GenericSetGrouping)
    self.assertIsInstance(histogram_grouping.GROUPINGS_BY_KEY[
        reserved_infos.OS_NAMES.name], histogram_grouping.GenericSetGrouping)
    self.assertIsInstance(histogram_grouping.GROUPINGS_BY_KEY[
        reserved_infos.OS_VERSIONS.name], histogram_grouping.GenericSetGrouping)
    self.assertIsInstance(
        histogram_grouping.GROUPINGS_BY_KEY[
            reserved_infos.PRODUCT_VERSIONS.name],
        histogram_grouping.GenericSetGrouping)
    self.assertIsInstance(histogram_grouping.GROUPINGS_BY_KEY[
        reserved_infos.STORIES.name], histogram_grouping.GenericSetGrouping)
    self.assertIsInstance(
        histogram_grouping.GROUPINGS_BY_KEY[
            reserved_infos.STORYSET_REPEATS.name],
        histogram_grouping.GenericSetGrouping)

  def testDateRange(self):
    grouping = histogram_grouping.DateRangeGrouping('foo')
    hist = histogram.Histogram('', 'count')
    self.assertEqual(grouping.callback(hist), '')
    hist.diagnostics['foo'] = date_range.DateRange(15e11)
    self.assertEqual(grouping.callback(hist), str(hist.diagnostics['foo']))

  def testReservedDateRangeGroupings(self):
    self.assertIsInstance(
        histogram_grouping.GROUPINGS_BY_KEY[
            reserved_infos.BENCHMARK_START.name],
        histogram_grouping.DateRangeGrouping)
    self.assertIsInstance(histogram_grouping.GROUPINGS_BY_KEY[
        reserved_infos.TRACE_START.name], histogram_grouping.DateRangeGrouping)
