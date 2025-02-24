# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
import six
from tracing.value.diagnostics import diagnostic
from tracing.value.diagnostics import diagnostic_ref

try:
  StringTypes = six.string_types # pylint: disable=invalid-name
except NameError:
  StringTypes = str


class UnmergeableDiagnosticSet(diagnostic.Diagnostic):
  __slots__ = ('_diagnostics',)

  def __init__(self, diagnostics):
    super(UnmergeableDiagnosticSet, self).__init__()
    self._diagnostics = diagnostics

  def __len__(self):
    return len(self._diagnostics)

  def __iter__(self):
    for diag in self._diagnostics:
      yield diag

  def CanAddDiagnostic(self, unused_other_diagnostic):
    return True

  def AddDiagnostic(self, other_diagnostic):
    if isinstance(other_diagnostic, UnmergeableDiagnosticSet):
      self._diagnostics.extend(other_diagnostic._diagnostics)
      return
    for diag in self:
      if diag.CanAddDiagnostic(other_diagnostic):
        diag.AddDiagnostic(other_diagnostic)
        return
    self._diagnostics.append(other_diagnostic)

  def _AsDictInto(self, d):
    d['diagnostics'] = [d.AsDictOrReference() for d in self]

  def _AsProto(self):
    raise NotImplementedError()

  @staticmethod
  def Deserialize(data, deserializer):
    return UnmergeableDiagnosticSet(
        [list(deserializer.GetDiagnostic(i).values())[0] for i in data])

  @staticmethod
  def FromDict(dct):
    def RefOrDiagnostic(d):
      if isinstance(d, StringTypes):
        return diagnostic_ref.DiagnosticRef(d)
      return diagnostic.Diagnostic.FromDict(d)

    return UnmergeableDiagnosticSet(
        [RefOrDiagnostic(d) for d in dct['diagnostics']])

  @staticmethod
  def FromProto(d):
    raise NotImplementedError()
