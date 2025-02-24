# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import uuid

try:
  from py_utils import slots_metaclass
  SlotsMetaclass = slots_metaclass.SlotsMetaclass # pylint: disable=invalid-name
except ImportError:
  # TODO(benjhayden): Figure out why py_utils doesn't work in dev_appserver.py
  SlotsMetaclass = None # pylint: disable=invalid-name

from tracing.value.diagnostics import all_diagnostics


class Diagnostic(object):
  __slots__ = ('_guid',)

  # Ensure that new subclasses remember to specify __slots__ in order to prevent
  # regressing memory consumption:
  if SlotsMetaclass:
    __metaclass__ = SlotsMetaclass

  def __init__(self):
    self._guid = None

  def __ne__(self, other):
    return not self == other

  @property
  def guid(self):
    if self._guid is None:
      self._guid = str(uuid.uuid4())
    return self._guid

  @guid.setter
  def guid(self, g):
    assert self._guid is None
    self._guid = g

  @property
  def has_guid(self):
    return self._guid is not None

  def AsDictOrReference(self):
    if self._guid:
      return self._guid
    return self.AsDict()

  def AsProtoOrReference(self):
    if self._guid:
      return self._guid
    return self.AsProto()

  def AsDict(self):
    dct = {'type': self.__class__.__name__}
    if self._guid:
      dct['guid'] = self._guid
    self._AsDictInto(dct)
    return dct

  def AsProto(self):
    return self._AsProto()

  def _AsDictInto(self, unused_dct):
    raise NotImplementedError()

  def _AsProto(self):
    raise NotImplementedError()

  @staticmethod
  def FromDict(dct):
    cls = all_diagnostics.GetDiagnosticClassForName(dct['type'])
    if not cls:
      raise ValueError('Unrecognized diagnostic type: ' + dct['type'])
    diagnostic = cls.FromDict(dct)
    if 'guid' in dct:
      diagnostic.guid = dct['guid']
    return diagnostic

  @staticmethod
  def FromProto(d):
    # Here we figure out which field is set and downcast to the right diagnostic
    # type. The diagnostic names in the proto must be the same as the class
    # names in the python code, for instance Breakdown.
    attr_name = d.WhichOneof('diagnostic_oneof')
    assert attr_name, 'The diagnostic oneof cannot be empty.'

    d = getattr(d, attr_name)
    assert type(d).__name__ in all_diagnostics.GetDiagnosticTypenames(), (
        'Unrecognized diagnostic type ' + type(d).__name__)

    diag_type = type(d).__name__
    cls = all_diagnostics.GetDiagnosticClassForName(diag_type)

    return cls.FromProto(d)

  def ResetGuid(self, guid=None):
    if guid:
      self._guid = guid
    else:
      self._guid = str(uuid.uuid4())

  def Inline(self):
    """Inlines a shared diagnostic.

    Any diagnostic that has a guid will be serialized as a reference, because it
    is assumed that diagnostics with guids are shared. This method removes the
    guid so that the diagnostic will be serialized by value.

    Inling is used for example in the dashboard, where certain types of shared
    diagnostics that vary on a per-upload basis are inlined for efficiency
    reasons.
    """
    self._guid = None

  def CanAddDiagnostic(self, unused_other_diagnostic):
    return False

  def AddDiagnostic(self, unused_other_diagnostic):
    raise Exception('Abstract virtual method: subclasses must override '
                    'this method if they override canAddDiagnostic')


def Deserialize(type_name, data, deserializer):
  cls = all_diagnostics.GetDiagnosticClassForName(type_name)
  if not cls:
    raise ValueError('Unrecognized diagnostic type: ' + type_name)
  return cls.Deserialize(data, deserializer)
