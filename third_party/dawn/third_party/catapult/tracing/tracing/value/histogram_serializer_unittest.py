# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from tracing.value import histogram
from tracing.value import histogram_serializer
from tracing.value.diagnostics import breakdown
from tracing.value.diagnostics import generic_set
from tracing.value.diagnostics import related_event_set
from tracing.value.diagnostics import related_name_map


class HistogramSerializerUnittest(unittest.TestCase):
  def testObjects(self):
    serializer = histogram_serializer.HistogramSerializer()
    self.assertEqual(0, serializer.GetOrAllocateId('a'))
    self.assertEqual(1, serializer.GetOrAllocateId(['b']))
    self.assertEqual(0, serializer.GetOrAllocateId('a'))
    self.assertEqual(1, serializer.GetOrAllocateId(['b']))

  def testDiagnostics(self):
    serializer = histogram_serializer.HistogramSerializer()
    self.assertEqual(0, serializer.GetOrAllocateDiagnosticId(
        'a', generic_set.GenericSet(['b'])))
    self.assertEqual(1, serializer.GetOrAllocateDiagnosticId(
        'a', generic_set.GenericSet(['c'])))
    self.assertEqual(0, serializer.GetOrAllocateDiagnosticId(
        'a', generic_set.GenericSet(['b'])))
    self.assertEqual(1, serializer.GetOrAllocateDiagnosticId(
        'a', generic_set.GenericSet(['c'])))

  def testSerialize(self):
    hist = histogram.Histogram('aaa', 'count_biggerIsBetter')
    hist.description = 'lorem ipsum'
    hist.diagnostics['bbb'] = related_name_map.RelatedNameMap({
        'ccc': 'a:c',
        'ddd': 'a:d',
    })
    hist.diagnostics['hhh'] = generic_set.GenericSet(['ggg'])
    hist.AddSample(0, {
        'bbb': breakdown.Breakdown.FromEntries({
            'ccc': 100,
            'ddd': 200,
        }),
        'eee': related_event_set.RelatedEventSet([{
            'stableId': 'fff',
            'title': 'ggg',
            'start': 500,
            'duration': 600,
        }]),
    })
    data = histogram_serializer.Serialize([hist])

    serializer_objects = data[0]
    diagnostics = data[1]

    self.assertIn('Breakdown', diagnostics)
    self.assertIn('bbb', diagnostics['Breakdown'])
    _, breakdown_indexes = list(
        diagnostics['Breakdown']['bbb'].items())[0]
    name_indexes = serializer_objects[breakdown_indexes[1]]
    self.assertEqual(
        [
            serializer_objects[breakdown_indexes[0]],
            [serializer_objects[i] for i in name_indexes],
            breakdown_indexes[2],
            breakdown_indexes[3]
        ],
        ['', ['ccc', 'ddd'], 100, 200]
    )

    self.assertIn('GenericSet', diagnostics)
    self.assertIn('hhh', diagnostics['GenericSet'])
    _, added_diagnostic = list(
        diagnostics['GenericSet']['hhh'].items())[0]
    self.assertEqual(serializer_objects[added_diagnostic], 'ggg')
    self.assertIn('statisticsNames', diagnostics['GenericSet'])
    _, stat_indexes = list(
        diagnostics['GenericSet']['statisticsNames'].items())[0]
    self.assertEqual(
        ['avg', 'count', 'max', 'min', 'std', 'sum'],
        [serializer_objects[i] for i in stat_indexes]
    )
    self.assertIn('description', diagnostics['GenericSet'])
    _, description_index = list(
        diagnostics['GenericSet']['description'].items())[0]
    self.assertEqual(serializer_objects[description_index], 'lorem ipsum')

    self.assertIn('RelatedEventSet', diagnostics)
    self.assertIn('eee', diagnostics['RelatedEventSet'])
    _, event_set_values = list(
        diagnostics['RelatedEventSet']['eee'].items())[0]
    self.assertEqual(
        [
            event_set_values[0][0],
            serializer_objects[event_set_values[0][1]],
            event_set_values[0][2],
            event_set_values[0][3]
        ],
        ['fff', 'ggg', 500, 600]
    )

    self.assertIn('RelatedNameMap', diagnostics)
    self.assertIn('bbb', diagnostics['RelatedNameMap'])
    _, relative_name_values = list(
        diagnostics['RelatedNameMap']['bbb'].items())[0]
    name_indexes = relative_name_values[0]
    self.assertEqual(
        [
            [serializer_objects[i] for i in serializer_objects[name_indexes]],
            serializer_objects[relative_name_values[1]],
            serializer_objects[relative_name_values[2]]
        ],
        [['ccc', 'ddd'], 'a:c', 'a:d']
    )

    histograms = data[2:]
    self.assertEqual(
        histograms[0],
        [
            0,
            'count+',
            1,
            [0, 1, 2, 3],
            [1, 0, None, 0, 0, 0, 0],
            {0: [1, [None, 4, 5]]},
            0,
        ]
    )
