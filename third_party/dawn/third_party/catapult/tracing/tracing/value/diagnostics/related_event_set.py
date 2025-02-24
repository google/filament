# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from six.moves import zip  # pylint: disable=redefined-builtin
from tracing.value.diagnostics import diagnostic


class RelatedEventSet(diagnostic.Diagnostic):
  __slots__ = ('_events_by_stable_id',)

  def __init__(self, events=()):
    super(RelatedEventSet, self).__init__()
    self._events_by_stable_id = {}
    for e in events:
      self.Add(e)

  def __eq__(self, other):
    if len(self) != len(other):
      return False
    for event in self:
      if event['stableId'] not in other._events_by_stable_id:
        return False
    return True

  def Add(self, event):
    self._events_by_stable_id[event['stableId']] = event

  def __len__(self):
    return len(self._events_by_stable_id)

  def __iter__(self):
    for event in self._events_by_stable_id.values():
      yield event

  @staticmethod
  def Deserialize(data, deserializer):
    events = RelatedEventSet()
    for event in data:
      event[1] = deserializer.GetObject(event[1])
      events.Add(
          dict(list(zip(['stableId', 'title', 'start', 'duration'], event))))
    return events

  @staticmethod
  def FromDict(dct):
    result = RelatedEventSet()
    for event in dct['events']:
      result.Add(event)
    return result

  @staticmethod
  def FromProto(d):
    raise NotImplementedError()

  def Serialize(self, serializer):
    return [
        [
            e['stableId'],
            serializer.GetOrAllocateId(e['title']),
            e['start'],
            e['duration'],
        ]
        for e in self]

  def _AsDictInto(self, d):
    d['events'] = list(self)

  def _AsProto(self):
    raise NotImplementedError()
