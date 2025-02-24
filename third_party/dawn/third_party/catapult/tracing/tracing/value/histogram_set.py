# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import collections

from tracing.proto import histogram_proto
from tracing.value import histogram
from tracing.value import histogram_deserializer
from tracing.value.diagnostics import all_diagnostics
from tracing.value.diagnostics import diagnostic
from tracing.value.diagnostics import diagnostic_ref
from tracing.value.diagnostics import generic_set


class HistogramSet(object):
  def __init__(self, histograms=()):
    self._histograms = set()
    self._shared_diagnostics_by_guid = {}
    for hist in histograms:
      self.AddHistogram(hist)

  def CreateHistogram(self, name, unit, samples, **options):
    hist = histogram.Histogram.Create(name, unit, samples, **options)
    self.AddHistogram(hist)
    return hist

  @property
  def shared_diagnostics(self):
    return list(self._shared_diagnostics_by_guid.values())

  def RemoveOrphanedDiagnostics(self):
    orphans = set(self._shared_diagnostics_by_guid.keys())
    for h in self._histograms:
      for d in h.diagnostics.values():
        if d.guid in orphans:
          orphans.remove(d.guid)
    for guid in orphans:
      del self._shared_diagnostics_by_guid[guid]

  def FilterHistograms(self, discard):
    self._histograms = set(
        hist
        for hist in self._histograms
        if not discard(hist))

  def AddHistogram(self, hist, diagnostics=None):
    if diagnostics:
      for name, diag in diagnostics.items():
        hist.diagnostics[name] = diag

    self._histograms.add(hist)

  def AddSharedDiagnostic(self, diag):
    self._shared_diagnostics_by_guid[diag.guid] = diag

  def AddSharedDiagnosticToAllHistograms(self, name, diag):
    self._shared_diagnostics_by_guid[diag.guid] = diag

    for hist in self:
      hist.diagnostics[name] = diag

  def Merge(self, other):
    """Merge another HistogramSet's contents."""
    self._shared_diagnostics_by_guid.update(other._shared_diagnostics_by_guid)
    self._histograms.update(other._histograms)

  def GetFirstHistogram(self):
    for hist in self._histograms:
      return hist

  def GetHistogramsNamed(self, name):
    return [h for h in self if h.name == name]

  def GetHistogramNamed(self, name):
    hs = self.GetHistogramsNamed(name)
    assert len(hs) == 1, 'Found %d Histograms names "%s"' % (len(hs), name)
    return hs[0]

  def GetSharedDiagnosticsOfType(self, typ):
    return [d for d in self.shared_diagnostics if isinstance(d, typ)]

  def LookupDiagnostic(self, guid):
    return self._shared_diagnostics_by_guid.get(guid)

  def __len__(self):
    return len(self._histograms)

  def __iter__(self):
    for hist in self._histograms:
      yield hist

  def Deserialize(self, data):
    for hist in histogram_deserializer.Deserialize(data):
      self.AddHistogram(hist)

  def ImportProto(self, serialized_proto):
    hist_set = histogram_proto.Pb2().HistogramSet()
    hist_set.ParseFromString(serialized_proto)

    for guid, d in hist_set.shared_diagnostics.items():
      diag = diagnostic.Diagnostic.FromProto(d)
      diag.guid = guid
      self._shared_diagnostics_by_guid[guid] = diag

    for h in hist_set.histograms:
      hist = histogram.Histogram.FromProto(h)
      hist.diagnostics.ResolveSharedDiagnostics(self)
      self.AddHistogram(hist)

  def ImportDicts(self, dicts):
    # The new HistogramSet JSON format is an array of at least 3 arrays.
    # This format isn't finished yet and is currently unused.
    if isinstance(dicts, list) and dicts and isinstance(dicts[0], list):
      self.Deserialize(dicts)
      return

    # The original HistogramSet JSON format was a flat array of objects.
    for d in dicts:
      self.ImportLegacyDict(d)

  def ImportLegacyDict(self, d):
    if 'type' in d:
      # TODO(benjhayden): Forget about TagMaps in 2019Q2.
      if d['type'] == 'TagMap':
        return

      assert d['type'] in all_diagnostics.GetDiagnosticTypenames(), (
          'Unrecognized shared diagnostic type ' + d['type'])
      diag = diagnostic.Diagnostic.FromDict(d)
      self._shared_diagnostics_by_guid[d['guid']] = diag
    else:
      hist = histogram.Histogram.FromDict(d)
      hist.diagnostics.ResolveSharedDiagnostics(self)
      self.AddHistogram(hist)

  def AsDicts(self):
    dcts = []
    for d in self._shared_diagnostics_by_guid.values():
      dcts.append(d.AsDict())
    for h in self:
      dcts.append(h.AsDict())
    return dcts

  def AsProto(self):
    proto = histogram_proto.Pb2().HistogramSet()
    for guid, d in self._shared_diagnostics_by_guid.items():
      proto.shared_diagnostics[guid].CopyFrom(d.AsProto())
    for h in self:
      proto.histograms.extend([h.AsProto()])

    return proto

  def ReplaceSharedDiagnostic(self, old_guid, new_diagnostic):
    if not isinstance(new_diagnostic, diagnostic_ref.DiagnosticRef):
      self._shared_diagnostics_by_guid[new_diagnostic.guid] = new_diagnostic

    old_diagnostic = self._shared_diagnostics_by_guid.get(old_guid)

    # Fast path, if they're both generic_sets, we overwrite the contents of the
    # old diagnostic.
    if isinstance(new_diagnostic, generic_set.GenericSet) and (
        isinstance(old_diagnostic, generic_set.GenericSet)):
      old_diagnostic.SetValues(list(new_diagnostic))
      old_diagnostic.ResetGuid(new_diagnostic.guid)

      self._shared_diagnostics_by_guid[new_diagnostic.guid] = old_diagnostic
      del self._shared_diagnostics_by_guid[old_guid]

      return

    for hist in self:
      for name, diag in hist.diagnostics.items():
        if diag.has_guid and diag.guid == old_guid:
          hist.diagnostics[name] = new_diagnostic

  def DeduplicateDiagnostics(self):
    names_to_candidates = {}
    diagnostics_to_histograms = collections.defaultdict(list)

    for hist in self:
      for name, candidate in hist.diagnostics.items():
        diagnostics_to_histograms[candidate].append(hist)

        if name not in names_to_candidates:
          names_to_candidates[name] = set()
        names_to_candidates[name].add(candidate)

    for name, candidates in names_to_candidates.items():
      deduplicated_diagnostics = set()

      for candidate in candidates:
        found = False
        for test in deduplicated_diagnostics:
          if candidate == test:
            hists = diagnostics_to_histograms.get(candidate)
            for h in hists:
              h.diagnostics[name] = test
            found = True
            break
        if not found:
          deduplicated_diagnostics.add(candidate)

        for diag in deduplicated_diagnostics:
          self._shared_diagnostics_by_guid[diag.guid] = diag
