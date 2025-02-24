# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from tracing.value.diagnostics import all_diagnostics


class DiagnosticUnittest(unittest.TestCase):

  def testEqualityForSmoke(self):
    for name in all_diagnostics.GetDiagnosticTypenames():
      ctor = all_diagnostics.GetDiagnosticClassForName(name)
      self.assertTrue(hasattr(ctor, '__eq__'))
