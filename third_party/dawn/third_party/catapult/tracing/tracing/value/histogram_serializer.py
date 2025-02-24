# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json


def Serialize(histograms):
  serializer = HistogramSerializer()
  histograms = [h.Serialize(serializer) for h in histograms]
  diagnostics = serializer._diagnostics_by_type
  for diagnostics_by_name in diagnostics.values():
    for diagnostics_by_id in diagnostics_by_name.values():
      for did, diag in diagnostics_by_id.items():
        diagnostics_by_id[did] = diag.Serialize(serializer)
  return [serializer._objects, diagnostics] + histograms


class HistogramSerializer(object):
  __slots__ = ('_objects', '_ids_by_json', '_diagnostics_by_type',
               '_diagnostic_id')

  def __init__(self):
    self._objects = []
    self._ids_by_json = {}
    self._diagnostics_by_type = {}
    self._diagnostic_id = -1

  def GetOrAllocateId(self, obj):
    if isinstance(obj, (dict, list)):
      obj_json = json.dumps(obj)
      if obj_json in self._ids_by_json:
        return self._ids_by_json[obj_json]
      self._objects.append(obj)
      i = len(self._objects) - 1
      self._ids_by_json[obj_json] = i
      return i

    if obj in self._objects:
      return self._objects.index(obj)
    self._objects.append(obj)
    return len(self._objects) - 1

  def GetOrAllocateDiagnosticId(self, name, diag):
    type_name = diag.__class__.__name__
    diagnostics_by_name = self._diagnostics_by_type.setdefault(type_name, {})
    diagnostics_by_id = diagnostics_by_name.setdefault(name, {})
    for i, other in diagnostics_by_id.items():
      if other is diag or other == diag:
        return i

    self._diagnostic_id += 1
    diagnostics_by_id[self._diagnostic_id] = diag
    return self._diagnostic_id
