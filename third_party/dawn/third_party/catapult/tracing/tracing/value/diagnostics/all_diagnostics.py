# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import importlib
import sys


# TODO(#3613): Flatten this to a list once diagnostics are in their own modules.
_MODULES_BY_DIAGNOSTIC_NAME = {
    'Breakdown': 'diagnostics.breakdown',
    'GenericSet': 'diagnostics.generic_set',
    'UnmergeableDiagnosticSet': 'diagnostics.unmergeable_diagnostic_set',
    'RelatedEventSet': 'diagnostics.related_event_set',
    'DateRange': 'diagnostics.date_range',
    'RelatedNameMap': 'diagnostics.related_name_map',
}


_CLASSES_BY_NAME = {}


def IsDiagnosticTypename(name):
  return name in _MODULES_BY_DIAGNOSTIC_NAME


def GetDiagnosticTypenames():
  return list(_MODULES_BY_DIAGNOSTIC_NAME.keys())


def GetDiagnosticClassForName(name):
  assert IsDiagnosticTypename(name), name

  if name in _CLASSES_BY_NAME:
    return _CLASSES_BY_NAME[name]

  module_name = 'tracing.value.%s' % _MODULES_BY_DIAGNOSTIC_NAME[name]
  importlib.import_module(module_name)

  cls = getattr(sys.modules[module_name], name)
  _CLASSES_BY_NAME[name] = cls
  return cls
