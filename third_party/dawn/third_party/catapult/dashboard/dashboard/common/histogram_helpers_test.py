# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from dashboard.common import histogram_helpers
from dashboard.common import testing_common
from tracing.value import histogram as histogram_module
from tracing.value import histogram_set
from tracing.value.diagnostics import generic_set
from tracing.value.diagnostics import reserved_infos


class HistogramHelpersTest(testing_common.TestCase):

  def testGetGroupingLabelFromHistogram_NoTags_ReturnsEmpty(self):
    hist = histogram_module.Histogram('hist', 'count')
    self.assertEqual('', histogram_helpers.GetGroupingLabelFromHistogram(hist))

  def testGetGroupingLabelFromHistogram_NoValidTags_ReturnsEmpty(self):
    hist = histogram_module.Histogram('hist', 'count')
    histograms = histogram_set.HistogramSet([hist])
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORY_TAGS.name, generic_set.GenericSet(['foo', 'bar']))
    self.assertEqual('', histogram_helpers.GetGroupingLabelFromHistogram(hist))

  def testGetGroupingLabelFromHistogram_ValidTags_SortsByKey(self):
    hist = histogram_module.Histogram('hist', 'count')
    histograms = histogram_set.HistogramSet([hist])
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORY_TAGS.name,
        generic_set.GenericSet(
            ['z:last', 'ignore', 'a:first', 'me', 'm:middle']))
    self.assertEqual('first_middle_last',
                     histogram_helpers.GetGroupingLabelFromHistogram(hist))

  def testComputeTestPathWithStory(self):
    hist = histogram_module.Histogram('hist', 'count')
    histograms = histogram_set.HistogramSet([hist])
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORIES.name, generic_set.GenericSet(['http://story']))
    hist = histograms.GetFirstHistogram()
    test_path = histogram_helpers.ComputeTestPath(hist)
    self.assertEqual('hist/http___story', test_path)

  def testComputeTestPathWithGroupingLabel(self):
    hist = histogram_module.Histogram('hist', 'count')
    histograms = histogram_set.HistogramSet([hist])
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORIES.name, generic_set.GenericSet(['http://story']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORY_TAGS.name,
        generic_set.GenericSet(['group:media', 'ignored_tag', 'case:browse']))
    hist = histograms.GetFirstHistogram()
    test_path = histogram_helpers.ComputeTestPath(hist)
    self.assertEqual('hist/browse_media/http___story', test_path)

  def testComputeTestPathWithoutStory(self):
    hist = histogram_module.Histogram('hist', 'count')
    histograms = histogram_set.HistogramSet([hist])
    hist = histograms.GetFirstHistogram()
    test_path = histogram_helpers.ComputeTestPath(hist)
    self.assertEqual('hist', test_path)

  def testComputeTestPathWithIsRefWithoutStory(self):
    hist = histogram_module.Histogram('hist', 'count')
    histograms = histogram_set.HistogramSet([hist])
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.IS_REFERENCE_BUILD.name, generic_set.GenericSet([True]))
    hist = histograms.GetFirstHistogram()
    test_path = histogram_helpers.ComputeTestPath(hist)
    self.assertEqual('hist/ref', test_path)

  def testComputeTestPathWithIsRefAndStory(self):
    hist = histogram_module.Histogram('hist', 'count')
    histograms = histogram_set.HistogramSet([hist])
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.STORIES.name, generic_set.GenericSet(['http://story']))
    histograms.AddSharedDiagnosticToAllHistograms(
        reserved_infos.IS_REFERENCE_BUILD.name, generic_set.GenericSet([True]))
    hist = histograms.GetFirstHistogram()
    test_path = histogram_helpers.ComputeTestPath(hist)
    self.assertEqual('hist/http___story_ref', test_path)
