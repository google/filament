# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import math
import unittest

from tracing.value import histogram_serializer
from tracing.value.diagnostics import breakdown
from tracing.value.diagnostics import diagnostic
from tracing.value import histogram_deserializer


class BreakdownUnittest(unittest.TestCase):

  def testRoundtrip(self):
    bd = breakdown.Breakdown()
    bd.Set('one', 1)
    bd.Set('m1', -1)
    bd.Set('inf', float('inf'))
    bd.Set('nun', float('nan'))
    bd.Set('ninf', float('-inf'))
    bd.Set('long', 2**65)
    d = bd.AsDict()
    clone = diagnostic.Diagnostic.FromDict(d)
    self.assertEqual(json.dumps(d), json.dumps(clone.AsDict()))
    self.assertEqual(clone.Get('one'), 1)
    self.assertEqual(clone.Get('m1'), -1)
    self.assertEqual(clone.Get('inf'), float('inf'))
    self.assertTrue(math.isnan(clone.Get('nun')))
    self.assertEqual(clone.Get('ninf'), float('-inf'))
    self.assertEqual(clone.Get('long'), 2**65)

  def testDeserialize(self):
    d = histogram_deserializer.HistogramDeserializer([
        'a', 'b', 'c', [0, 1, 2], 'colors'])
    b = breakdown.Breakdown.Deserialize([4, 3, 1, 2, 3], d)
    self.assertEqual(b.color_scheme, 'colors')
    self.assertEqual(b.Get('a'), 1)
    self.assertEqual(b.Get('b'), 2)
    self.assertEqual(b.Get('c'), 3)

  def testSerialize(self):
    s = histogram_serializer.HistogramSerializer()
    b = breakdown.Breakdown.FromEntries({'a': 10, 'b': 20})
    self.assertEqual(b.Serialize(s), [0, 3, 10, 20])
    self.assertEqual(s.GetOrAllocateId(''), 0)
    self.assertEqual(s.GetOrAllocateId('a'), 1)
    self.assertEqual(s.GetOrAllocateId('b'), 2)
    self.assertEqual(s.GetOrAllocateId([1, 2]), 3)
