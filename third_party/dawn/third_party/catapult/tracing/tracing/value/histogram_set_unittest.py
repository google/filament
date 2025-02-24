# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import math
import unittest

from tracing.proto import histogram_proto
from tracing.value import histogram
from tracing.value import histogram_set
from tracing.value.diagnostics import date_range
from tracing.value.diagnostics import diagnostic_ref
from tracing.value.diagnostics import generic_set
from six.moves import zip


def _AddHist(hist_set, name=None, unit=None):
  hist = hist_set.histograms.add()
  hist.name = name or '_'
  hist.unit.unit = unit or histogram_proto.Pb2().MS
  return hist


class HistogramSetUnittest(unittest.TestCase):

  def testGetSharedDiagnosticsOfType(self):
    d0 = generic_set.GenericSet(['foo'])
    d1 = date_range.DateRange(0)
    hs = histogram_set.HistogramSet()
    hs.AddSharedDiagnosticToAllHistograms('generic', d0)
    hs.AddSharedDiagnosticToAllHistograms('generic', d1)
    diagnostics = hs.GetSharedDiagnosticsOfType(generic_set.GenericSet)
    self.assertEqual(len(diagnostics), 1)
    self.assertIsInstance(diagnostics[0], generic_set.GenericSet)

  def testImportDicts(self):
    hist = histogram.Histogram('', 'unitless')
    hists = histogram_set.HistogramSet([hist])
    hists2 = histogram_set.HistogramSet()
    hists2.ImportDicts(hists.AsDicts())
    self.assertEqual(len(hists), len(hists2))

  def testAssertType(self):
    hs = histogram_set.HistogramSet()
    with self.assertRaises(AssertionError):
      hs.ImportDicts([{'type': ''}])

  def testIgnoreTagMap(self):
    histogram_set.HistogramSet().ImportDicts([{'type': 'TagMap'}])

  def testFilterHistogram(self):
    a = histogram.Histogram('a', 'unitless')
    b = histogram.Histogram('b', 'unitless')
    c = histogram.Histogram('c', 'unitless')
    hs = histogram_set.HistogramSet([a, b, c])
    hs.FilterHistograms(lambda h: h.name == 'b')

    names = set(['a', 'c'])
    for h in hs:
      self.assertIn(h.name, names)
      names.remove(h.name)
    self.assertEqual(0, len(names))

  def testRemoveOrphanedDiagnostics(self):
    da = generic_set.GenericSet(['a'])
    db = generic_set.GenericSet(['b'])
    a = histogram.Histogram('a', 'unitless')
    b = histogram.Histogram('b', 'unitless')
    hs = histogram_set.HistogramSet([a])
    hs.AddSharedDiagnosticToAllHistograms('a', da)
    hs.AddHistogram(b)
    hs.AddSharedDiagnosticToAllHistograms('b', db)
    hs.FilterHistograms(lambda h: h.name == 'a')

    dicts = hs.AsDicts()
    self.assertEqual(3, len(dicts))

    hs.RemoveOrphanedDiagnostics()
    dicts = hs.AsDicts()
    self.assertEqual(2, len(dicts))

  def testAddSharedDiagnostic(self):
    diags = {}
    da = generic_set.GenericSet(['a'])
    db = generic_set.GenericSet(['b'])
    diags['da'] = da
    diags['db'] = db
    a = histogram.Histogram('a', 'unitless')
    b = histogram.Histogram('b', 'unitless')
    hs = histogram_set.HistogramSet()
    hs.AddSharedDiagnostic(da)
    hs.AddHistogram(a, {'da': da})
    hs.AddHistogram(b, {'db': db})

    # This should produce one shared diagnostic and 2 histograms.
    dicts = hs.AsDicts()
    self.assertEqual(3, len(dicts))
    self.assertEqual(da.AsDict(), dicts[0])


    # Assert that you only see the shared diagnostic once.
    seen_once = False
    for idx, val in enumerate(dicts):
      if idx == 0:
        continue
      if 'da' in val['diagnostics']:
        self.assertFalse(seen_once)
        self.assertEqual(val['diagnostics']['da'], da.guid)
        seen_once = True

  def testMerge(self):
    hs1 = histogram_set.HistogramSet([histogram.Histogram('a', 'unitless')])
    hs1.AddSharedDiagnosticToAllHistograms('name',
                                           generic_set.GenericSet(['diag1']))

    hs2 = histogram_set.HistogramSet([histogram.Histogram('b', 'unitless')])
    hs2.AddSharedDiagnosticToAllHistograms('name',
                                           generic_set.GenericSet(['diag2']))

    hs1.Merge(hs2)

    self.assertEqual(len(hs1), 2)
    self.assertEqual(len(hs1.shared_diagnostics), 2)
    self.assertEqual(hs1.GetHistogramNamed('a').diagnostics['name'],
                     generic_set.GenericSet(['diag1']))
    self.assertEqual(hs1.GetHistogramNamed('b').diagnostics['name'],
                     generic_set.GenericSet(['diag2']))

  def testSharedDiagnostic(self):
    hist = histogram.Histogram('', 'unitless')
    hists = histogram_set.HistogramSet([hist])
    diag = generic_set.GenericSet(['shared'])
    hists.AddSharedDiagnosticToAllHistograms('generic', diag)

    # Serializing a single Histogram with a single shared diagnostic should
    # produce 2 dicts.
    ds = hists.AsDicts()
    self.assertEqual(len(ds), 2)
    self.assertEqual(diag.AsDict(), ds[0])

    # The serialized Histogram should refer to the shared diagnostic by its
    # guid.
    self.assertEqual(ds[1]['diagnostics']['generic'], diag.guid)

    # Deserialize ds.
    hists2 = histogram_set.HistogramSet()
    hists2.ImportDicts(ds)
    self.assertEqual(len(hists2), 1)
    hist2 = list(hists2)[0]

    self.assertIsInstance(
        hist2.diagnostics.get('generic'), generic_set.GenericSet)
    self.assertEqual(list(diag), list(hist2.diagnostics.get('generic')))

  def testReplaceSharedDiagnostic(self):
    hist = histogram.Histogram('', 'unitless')
    hists = histogram_set.HistogramSet([hist])
    diag0 = generic_set.GenericSet(['shared0'])
    diag1 = generic_set.GenericSet(['shared1'])
    hists.AddSharedDiagnosticToAllHistograms('generic0', diag0)
    hists.AddSharedDiagnosticToAllHistograms('generic1', diag1)

    guid0 = diag0.guid
    guid1 = diag1.guid

    hists.ReplaceSharedDiagnostic(
        guid0, diagnostic_ref.DiagnosticRef('fakeGuid'))

    self.assertEqual(hist.diagnostics['generic0'].guid, 'fakeGuid')
    self.assertEqual(hist.diagnostics['generic1'].guid, guid1)

  def testReplaceSharedDiagnostic_NonRefAddsToMap(self):
    hist = histogram.Histogram('', 'unitless')
    hists = histogram_set.HistogramSet([hist])
    diag0 = generic_set.GenericSet(['shared0'])
    diag1 = generic_set.GenericSet(['shared1'])
    hists.AddSharedDiagnosticToAllHistograms('generic0', diag0)

    guid0 = diag0.guid
    guid1 = diag1.guid

    hists.ReplaceSharedDiagnostic(guid0, diag1)

    self.assertIsNotNone(hists.LookupDiagnostic(guid1))

  def testDeduplicateDiagnostics(self):
    generic_a = generic_set.GenericSet(['A'])
    generic_b = generic_set.GenericSet(['B'])
    date_a = date_range.DateRange(42)
    date_b = date_range.DateRange(57)

    a_hist = histogram.Histogram('a', 'unitless')
    generic0 = generic_set.GenericSet.FromDict(generic_a.AsDict())
    generic0.AddDiagnostic(generic_b)
    a_hist.diagnostics['generic'] = generic0
    date0 = date_range.DateRange.FromDict(date_a.AsDict())
    date0.AddDiagnostic(date_b)
    a_hist.diagnostics['date'] = date0

    b_hist = histogram.Histogram('b', 'unitless')
    generic1 = generic_set.GenericSet.FromDict(generic_a.AsDict())
    generic1.AddDiagnostic(generic_b)
    b_hist.diagnostics['generic'] = generic1
    date1 = date_range.DateRange.FromDict(date_a.AsDict())
    date1.AddDiagnostic(date_b)
    b_hist.diagnostics['date'] = date1

    c_hist = histogram.Histogram('c', 'unitless')
    c_hist.diagnostics['generic'] = generic1

    histograms = histogram_set.HistogramSet([a_hist, b_hist, c_hist])
    self.assertNotEqual(
        a_hist.diagnostics['generic'].guid, b_hist.diagnostics['generic'].guid)
    self.assertEqual(
        b_hist.diagnostics['generic'].guid, c_hist.diagnostics['generic'].guid)
    self.assertEqual(
        a_hist.diagnostics['generic'], b_hist.diagnostics['generic'])
    self.assertNotEqual(
        a_hist.diagnostics['date'].guid, b_hist.diagnostics['date'].guid)
    self.assertEqual(
        a_hist.diagnostics['date'], b_hist.diagnostics['date'])

    histograms.DeduplicateDiagnostics()

    self.assertEqual(
        a_hist.diagnostics['generic'].guid, b_hist.diagnostics['generic'].guid)
    self.assertEqual(
        b_hist.diagnostics['generic'].guid, c_hist.diagnostics['generic'].guid)
    self.assertEqual(
        a_hist.diagnostics['generic'], b_hist.diagnostics['generic'])
    self.assertEqual(
        a_hist.diagnostics['date'].guid, b_hist.diagnostics['date'].guid)
    self.assertEqual(
        a_hist.diagnostics['date'], b_hist.diagnostics['date'])

    histogram_dicts = histograms.AsDicts()

    # All diagnostics should have been serialized as DiagnosticRefs.
    for d in histogram_dicts:
      if 'type' not in d:
        for diagnostic_dict in d['diagnostics'].values():
          self.assertIsInstance(diagnostic_dict, str)

    histograms2 = histogram_set.HistogramSet()
    histograms2.ImportDicts(histograms.AsDicts())
    a_hists = histograms2.GetHistogramsNamed('a')
    self.assertEqual(len(a_hists), 1)
    a_hist2 = a_hists[0]
    b_hists = histograms2.GetHistogramsNamed('b')
    self.assertEqual(len(b_hists), 1)
    b_hist2 = b_hists[0]

    self.assertEqual(
        a_hist2.diagnostics['generic'].guid,
        b_hist2.diagnostics['generic'].guid)
    self.assertEqual(
        a_hist2.diagnostics['generic'],
        b_hist2.diagnostics['generic'])
    self.assertEqual(
        a_hist2.diagnostics['date'].guid,
        b_hist2.diagnostics['date'].guid)
    self.assertEqual(
        a_hist2.diagnostics['date'],
        b_hist2.diagnostics['date'])

  def testBasicImportFromProto(self):
    hist_set = histogram_proto.Pb2().HistogramSet()

    hist = hist_set.histograms.add()
    hist.name = 'metric1'
    hist.unit.unit = histogram_proto.Pb2().TS_MS

    hist = hist_set.histograms.add()
    hist.name = 'metric2'
    hist.unit.unit = histogram_proto.Pb2().SIGMA
    hist.unit.improvement_direction = histogram_proto.Pb2().BIGGER_IS_BETTER

    parsed = histogram_set.HistogramSet()
    parsed.ImportProto(hist_set.SerializeToString())
    hists = list(parsed)

    # The order of the histograms isn't guaranteed.
    self.assertEqual(len(hists), 2)
    self.assertCountEqual([hists[0].name, hists[1].name],
                          ['metric1', 'metric2'])
    self.assertCountEqual([hists[0].unit, hists[1].unit],
                          ['tsMs', 'sigma_biggerIsBetter'])

  def testSimpleFieldsFromProto(self):
    hist_set = histogram_proto.Pb2().HistogramSet()

    hist = _AddHist(hist_set)
    hist.description = 'description!'
    hist.sample_values.append(21)
    hist.sample_values.append(22)
    hist.sample_values.append(23)
    hist.max_num_sample_values = 3
    hist.num_nans = 1

    parsed = histogram_set.HistogramSet()
    parsed.ImportProto(hist_set.SerializeToString())
    parsed_hist = parsed.GetFirstHistogram()

    self.assertEqual(parsed_hist.description, 'description!')
    self.assertEqual(parsed_hist.sample_values, [21, 22, 23])
    self.assertEqual(parsed_hist.max_num_sample_values, 3)
    self.assertEqual(parsed_hist.num_nans, 1)

  def testRaisesOnMissingMandatoryFieldsInProto(self):
    hist_set = histogram_proto.Pb2().HistogramSet()
    hist = hist_set.histograms.add()

    with self.assertRaises(ValueError):
      # Missing name.
      parsed = histogram_set.HistogramSet()
      parsed.ImportProto(hist_set.SerializeToString())

    with self.assertRaises(ValueError):
      # Missing unit.
      hist.name = "eh"
      parsed.ImportProto(hist_set.SerializeToString())

  def testMinimalBinBoundsInProto(self):
    hist_set = histogram_proto.Pb2().HistogramSet()
    hist = _AddHist(hist_set)

    hist.bin_boundaries.first_bin_boundary = 1

    parsed = histogram_set.HistogramSet()
    parsed.ImportProto(hist_set.SerializeToString())
    parsed_hist = parsed.GetFirstHistogram()

    # The transport format for bins is relatively easily understood, whereas
    # how bins are generated is very complex, so use the former for the bin
    # bounds tests. See the histogram spec in docs/histogram-set-json-format.md.
    dict_format = parsed_hist.AsDict()['binBoundaries']

    self.assertEqual(dict_format, [1])

  def testComplexBinBounds(self):
    hist_set = histogram_proto.Pb2().HistogramSet()
    hist = _AddHist(hist_set)

    hist.bin_boundaries.first_bin_boundary = 17
    spec1 = hist.bin_boundaries.bin_specs.add()
    spec1.bin_boundary = 18
    spec2 = hist.bin_boundaries.bin_specs.add()
    spec2.bin_spec.boundary_type = (
        histogram_proto.Pb2().BinBoundaryDetailedSpec.EXPONENTIAL)
    spec2.bin_spec.maximum_bin_boundary = 19
    spec2.bin_spec.num_bin_boundaries = 20
    spec3 = hist.bin_boundaries.bin_specs.add()
    spec3.bin_spec.boundary_type = (
        histogram_proto.Pb2().BinBoundaryDetailedSpec.LINEAR)
    spec3.bin_spec.maximum_bin_boundary = 21
    spec3.bin_spec.num_bin_boundaries = 22

    parsed = histogram_set.HistogramSet()
    parsed.ImportProto(hist_set.SerializeToString())
    parsed_hist = parsed.GetFirstHistogram()

    dict_format = parsed_hist.AsDict()['binBoundaries']

    self.assertEqual(dict_format, [17, 18, [1, 19, 20], [0, 21, 22]])

  def testImportRunningStatisticsFromProto(self):
    hist_set = histogram_proto.Pb2().HistogramSet()
    hist = _AddHist(hist_set)

    hist.running.count = 4
    hist.running.max = 23
    hist.running.meanlogs = 1
    hist.running.mean = 22
    hist.running.min = 21
    hist.running.sum = 66
    hist.running.variance = 1

    parsed = histogram_set.HistogramSet()
    parsed.ImportProto(hist_set.SerializeToString())
    parsed_hist = parsed.GetFirstHistogram()

    # We get at meanlogs through geometric_mean. Variance is after Bessel's
    # correction has been applied.
    self.assertEqual(parsed_hist.running.count, 4)
    self.assertEqual(parsed_hist.running.max, 23)
    self.assertEqual(parsed_hist.running.geometric_mean, math.exp(1))
    self.assertEqual(parsed_hist.running.mean, 22)
    self.assertEqual(parsed_hist.running.min, 21)
    self.assertEqual(parsed_hist.running.sum, 66)
    self.assertAlmostEqual(parsed_hist.running.variance, 0.3333333333)

  def testImportAllBinsFromProto(self):
    hist_set = histogram_proto.Pb2().HistogramSet()
    hist = _AddHist(hist_set)
    hist.all_bins[0].bin_count = 24
    map1 = hist.all_bins[0].diagnostic_maps.add().diagnostic_map
    map1['some bin diagnostic'].generic_set.values.append('"some value"')
    map2 = hist.all_bins[0].diagnostic_maps.add().diagnostic_map
    map2['other bin diagnostic'].generic_set.values.append('"some other value"')

    parsed = histogram_set.HistogramSet()
    parsed.ImportProto(hist_set.SerializeToString())
    parsed_hist = parsed.GetFirstHistogram()

    self.assertGreater(len(parsed_hist.bins), 1)
    self.assertEqual(len(parsed_hist.bins[0].diagnostic_maps), 2)
    self.assertEqual(len(parsed_hist.bins[0].diagnostic_maps[0]), 1)
    self.assertEqual(len(parsed_hist.bins[0].diagnostic_maps[1]), 1)
    self.assertEqual(
        parsed_hist.bins[0].diagnostic_maps[0]['some bin diagnostic'],
        generic_set.GenericSet(values=['some value']))
    self.assertEqual(
        parsed_hist.bins[0].diagnostic_maps[1]['other bin diagnostic'],
        generic_set.GenericSet(values=['some other value']))

  def testSummaryOptionsFromProto(self):
    hist_set = histogram_proto.Pb2().HistogramSet()
    hist = _AddHist(hist_set)
    hist.summary_options.avg = True
    hist.summary_options.nans = True
    hist.summary_options.geometric_mean = True
    hist.summary_options.percentile.append(0.90)
    hist.summary_options.percentile.append(0.95)
    hist.summary_options.percentile.append(0.99)

    parsed = histogram_set.HistogramSet()
    parsed.ImportProto(hist_set.SerializeToString())
    parsed_hist = parsed.GetFirstHistogram()

    # See the histogram spec in docs/histogram-set-json-format.md.
    # Serializing to proto leads to funny rounding errors.
    # The rounding works different from Python 2 and Python 3.
    actual_statistics_names = sorted(parsed_hist.statistics_names)
    expected_statistics_names = sorted(
        ['pct_099_0000009537', 'pct_089_9999976158', 'pct_094_9999988079',
         'nans', 'avg', 'geometricMean']
    )
    self.assertTrue(
        all(actual[:17] == expected[:17]
            for (actual, expected)
            in zip(actual_statistics_names, expected_statistics_names)),
        msg='Statistics names are different. Expected: %s. Actual: %s ' % (
            expected_statistics_names, actual_statistics_names)
    )

  def testImportSharedDiagnosticsFromProto(self):
    guid1 = 'f7f17394-fa4a-481e-86bd-a82cd55935a7'
    guid2 = '88ea36c7-6dcb-4ba8-ba56-1979de05e16f'
    hist_set = histogram_proto.Pb2().HistogramSet()

    hist_set.shared_diagnostics[guid1].generic_set.values.append(
        '"webrtc_perf_tests"')
    hist_set.shared_diagnostics[guid2].generic_set.values.append('123456')
    hist_set.shared_diagnostics['whatever'].generic_set.values.append('2')

    hist = hist_set.histograms.add()
    hist.name = "_"
    hist.unit.unit = histogram_proto.Pb2().MS
    hist.diagnostics.diagnostic_map['bots'].shared_diagnostic_guid = guid1
    hist.diagnostics.diagnostic_map['pointId'].shared_diagnostic_guid = guid2

    parsed = histogram_set.HistogramSet()
    parsed.ImportProto(hist_set.SerializeToString())

    parsed_hist = parsed.GetFirstHistogram()

    self.assertIsNotNone(parsed_hist)
    self.assertEqual(len(parsed_hist.diagnostics), 2)

    self.assertEqual(parsed_hist.diagnostics['pointId'],
                     generic_set.GenericSet(values=[123456]))
    self.assertEqual(parsed_hist.diagnostics['bots'],
                     generic_set.GenericSet(values=['webrtc_perf_tests']))
