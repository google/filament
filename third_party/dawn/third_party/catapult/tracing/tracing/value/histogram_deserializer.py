# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from tracing.value import histogram
from tracing.value.diagnostics import diagnostic


def Deserialize(data):
  deserializer = HistogramDeserializer(data[0], data[1])
  return {histogram.Histogram.Deserialize(datum, deserializer)
          for datum in data[2:]}


class HistogramDeserializer(object):
  def __init__(self, objects, diagnostics=None):
    self._objects = objects
    self._diagnostics = {}
    if diagnostics:
      for type_name, diagnostics_by_name in diagnostics.items():
        for name, diagnostics_by_id in diagnostics_by_name.items():
          for i, data in diagnostics_by_id.items():
            self._diagnostics[int(i)] = {
                name: diagnostic.Deserialize(type_name, data, self)}

  def GetObject(self, i):
    return self._objects[i]

  def GetDiagnostic(self, i):
    return self._diagnostics[int(i)]
