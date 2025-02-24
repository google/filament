# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import math
import numbers

import six
from six.moves import zip  # pylint: disable=redefined-builtin
from tracing.value.diagnostics import diagnostic


try:
  StringTypes = six.string_types # pylint: disable=invalid-name
except NameError:
  StringTypes = str


class Breakdown(diagnostic.Diagnostic):
  __slots__ = '_values', '_color_scheme'

  def __init__(self):
    super(Breakdown, self).__init__()
    self._values = {}
    self._color_scheme = ''

  def __eq__(self, other):
    if self._color_scheme != other._color_scheme:
      return False
    if len(self._values) != len(other._values):
      return False
    for k, v in self:
      if v != other.Get(k):
        return False
    return True

  @property
  def color_scheme(self):
    return self._color_scheme

  @staticmethod
  def Deserialize(data, deserializer):
    breakdown = Breakdown()
    breakdown._color_scheme = deserializer.GetObject(data[0])
    for key, value in zip(deserializer.GetObject(data[1]), data[2:]):
      if value in ['NaN', 'Infinity', '-Infinity']:
        value = float(value)
      breakdown.Set(deserializer.GetObject(key), value)
    return breakdown

  @staticmethod
  def FromDict(dct):
    result = Breakdown()
    result._color_scheme = dct.get('colorScheme')
    for name, value in dct['values'].items():
      if value in ['NaN', 'Infinity', '-Infinity']:
        value = float(value)
      result.Set(name, value)
    return result

  @staticmethod
  def FromProto(d):
    raise NotImplementedError()

  def _AsProto(self):
    raise NotImplementedError()

  def Serialize(self, serializer):
    keys = list(self._values.keys())
    keys.sort()
    return [
        serializer.GetOrAllocateId(self.color_scheme),
        serializer.GetOrAllocateId([
            serializer.GetOrAllocateId(k) for k in keys]),
    ] + [self.Get(k) for k in keys]

  def _AsDictInto(self, d):
    d['values'] = {}
    for name, value in self:
      # JSON serializes NaN and the infinities as 'null', preventing
      # distinguishing between them. Override that behavior by serializing them
      # as their Javascript string names, not their python string names since
      # the reference implementation is in Javascript.
      if math.isnan(value):
        value = 'NaN'
      elif math.isinf(value):
        if value > 0:
          value = 'Infinity'
        else:
          value = '-Infinity'
      d['values'][name] = value
    if self._color_scheme:
      d['colorScheme'] = self._color_scheme

  @staticmethod
  def FromEntries(entries):
    b = Breakdown()
    for name, value in entries.items():
      b.Set(name, value)
    return b

  def Set(self, name, value):
    assert isinstance(name, StringTypes), (
        'Expected basestring, found %s: "%r"' % (type(name).__name__, name))
    assert isinstance(value, numbers.Number), (
        'Expected number, found %s: "%r"', (type(value).__name__, value))
    self._values[name] = value

  def Get(self, name):
    return self._values.get(name, 0)

  def __iter__(self):
    for name, value in self._values.items():
      yield name, value

  def __len__(self):
    return len(self._values)
