# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from tracing.value import histogram
from tracing.value import histogram_deserializer
from tracing.value.diagnostics import breakdown
from tracing.value.diagnostics import generic_set
from tracing.value.diagnostics import related_name_map


class HistogramDeserializerUnittest(unittest.TestCase):
  def testObjects(self):
    deserializer = histogram_deserializer.HistogramDeserializer(
        ['a', ['b']], {})
    self.assertEqual('a', deserializer.GetObject(0))
    self.assertEqual(['b'], deserializer.GetObject(1))
    with self.assertRaises(IndexError):
      deserializer.GetObject(2)

  def testDeserialize(self):
    hists = histogram_deserializer.Deserialize([
        [
            'aaa',
            [1, [1, 1000, 20]],
            'a:c',
            'a:d',
            'ccc',
            'ddd',
            [4, 5],
            'ggg',
            'avg',
            'count',
            'max',
            'min',
            'std',
            'sum',
            'lorem ipsum',
            '',
        ],
        {
            'Breakdown': {'bbb': {4: [15, 6, 11, 31]}},
            'GenericSet': {
                'hhh': {1: 7},
                'statisticsNames': {2: [8, 9, 10, 11, 12, 13]},
                'description': {3: 14},
            },
            'RelatedEventSet': {'eee': {5: [['fff', 7, 3, 4]]}},
            'RelatedNameMap': {'bbb': {0: [6, 2, 3]}},
        },
        [
            0,
            'count+',
            1,
            [0, 1, 2, 3],
            [1, 0, None, 0, 0, 0, 0],
            {0: [1, [None, 4, 5]]},
            0,
        ]
    ])
    self.assertEqual(1, len(hists))
    hist = list(hists)[0]
    self.assertIsInstance(hist, histogram.Histogram)
    self.assertEqual('aaa', hist.name)
    self.assertEqual('lorem ipsum', hist.description)
    self.assertEqual('count+', hist.unit)
    self.assertEqual(0, hist.average)
    self.assertEqual(1, hist.num_values)
    self.assertEqual(0, hist.standard_deviation)
    self.assertEqual(0, hist.sum)
    self.assertEqual(0, hist.running.min)
    self.assertEqual(0, hist.running.max)

    names = hist.diagnostics.get('bbb')
    self.assertIsInstance(names, related_name_map.RelatedNameMap)
    self.assertEqual(names.Get('ccc'), 'a:c')
    self.assertEqual(names.Get('ddd'), 'a:d')

    hhh = hist.diagnostics.get('hhh')
    self.assertIsInstance(hhh, generic_set.GenericSet)
    self.assertEqual('ggg', list(hhh)[0])

    self.assertEqual(len(hist.bins), 22)
    b = hist.bins[0]
    self.assertEqual(len(b.diagnostic_maps), 1)
    dm = b.diagnostic_maps[0]
    self.assertEqual(len(dm), 2)
    bd = dm.get('bbb')
    self.assertIsInstance(bd, breakdown.Breakdown)
    self.assertEqual('', bd.color_scheme)
    self.assertEqual(len(bd), 2)
    self.assertEqual(11, bd.Get('ccc'))
    self.assertEqual(31, bd.Get('ddd'))
