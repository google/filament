# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from tracing.proto import histogram_proto
from tracing.value import histogram_deserializer
from tracing.value import histogram_serializer
from tracing.value.diagnostics import diagnostic
from tracing.value.diagnostics import generic_set


class GenericSetUnittest(unittest.TestCase):

  def testRoundtrip(self):
    a_set = generic_set.GenericSet([
        None,
        True,
        False,
        0,
        1,
        42,
        [],
        {},
        [0, False],
        {'a': 1, 'b': True},
    ])
    self.assertEqual(a_set, diagnostic.Diagnostic.FromDict(a_set.AsDict()))

  def testEq(self):
    a_set = generic_set.GenericSet([
        None,
        True,
        False,
        0,
        1,
        42,
        [],
        {},
        [0, False],
        {'a': 1, 'b': True},
    ])
    b_set = generic_set.GenericSet([
        {'b': True, 'a': 1},
        [0, False],
        {},
        [],
        42,
        1,
        0,
        False,
        True,
        None,
    ])
    self.assertEqual(a_set, b_set)

  def testMerge(self):
    a_set = generic_set.GenericSet([
        None,
        True,
        False,
        0,
        1,
        42,
        [],
        {},
        [0, False],
        {'a': 1, 'b': True},
    ])
    b_set = generic_set.GenericSet([
        {'b': True, 'a': 1},
        [0, False],
        {},
        [],
        42,
        1,
        0,
        False,
        True,
        None,
    ])
    self.assertTrue(a_set.CanAddDiagnostic(b_set))
    self.assertTrue(b_set.CanAddDiagnostic(a_set))
    a_set.AddDiagnostic(b_set)
    self.assertEqual(a_set, b_set)
    b_set.AddDiagnostic(a_set)
    self.assertEqual(a_set, b_set)

    c_dict = {'a': 1, 'b': 1}
    c_set = generic_set.GenericSet([c_dict])
    a_set.AddDiagnostic(c_set)
    self.assertEqual(len(a_set), 1 + len(b_set))
    self.assertIn(c_dict, a_set)

  def testGetOnlyElement(self):
    gs = generic_set.GenericSet(['foo'])
    self.assertEqual(gs.GetOnlyElement(), 'foo')

  def testGetOnlyElementRaises(self):
    gs = generic_set.GenericSet([])
    with self.assertRaises(AssertionError):
      gs.GetOnlyElement()

  def testDeserialize(self):
    d = histogram_deserializer.HistogramDeserializer(['aaa', 'bbb'])
    a = generic_set.GenericSet.Deserialize(0, d)
    self.assertEqual(len(a), 1)
    self.assertIn('aaa', a)
    b = generic_set.GenericSet.Deserialize([0, 1], d)
    self.assertEqual(len(b), 2)
    self.assertIn('aaa', b)
    self.assertIn('bbb', b)

  def testSerialize(self):
    s = histogram_serializer.HistogramSerializer()
    g = generic_set.GenericSet(['a', 'b'])
    self.assertEqual(g.Serialize(s), [0, 1])
    g = generic_set.GenericSet(['a'])
    self.assertEqual(g.Serialize(s), 0)

  def testFromProto(self):
    p = histogram_proto.Pb2().GenericSet()
    p.values.append('12345')
    p.values.append('"string"')
    p.values.append('{"attr":1}')

    g = generic_set.GenericSet.FromProto(p)

    values = list(g)
    self.assertEqual([12345, 'string', {"attr": 1}], values)

  def testProtoRoundtrip(self):
    a_set = generic_set.GenericSet([
        None,
        True,
        False,
        0,
        1,
        42,
        [],
        {},
        [0, False],
        {'a': 1, 'b': True},
    ])
    self.assertEqual(a_set, diagnostic.Diagnostic.FromProto(a_set.AsProto()))

  def testInvalidJsonValueInProto(self):
    with self.assertRaises(TypeError):
      p = histogram_proto.Pb2().GenericSet()
      p.values.append('this_is_an_undefined_json_indentifier')
      generic_set.GenericSet.FromProto(p)
