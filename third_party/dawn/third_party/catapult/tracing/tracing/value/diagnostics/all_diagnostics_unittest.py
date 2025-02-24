# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from tracing.value.diagnostics import all_diagnostics


class AllDiagnosticsUnittest(unittest.TestCase):

  def testGetDiagnosticClassForName(self):
    cls0 = all_diagnostics.GetDiagnosticClassForName('GenericSet')
    gs0 = cls0(['foo'])
    gs0_dict = gs0.AsDict()

    # Run twice to ensure that the memoization isn't broken.
    cls1 = all_diagnostics.GetDiagnosticClassForName('GenericSet')
    gs1 = cls1(['foo'])
    gs1_dict = gs1.AsDict()

    self.assertEqual(gs0_dict['type'], 'GenericSet')
    self.assertEqual(gs1_dict['type'], 'GenericSet')

  def testGetDiagnosticClassForName_Bogus(self):
    self.assertRaises(
        AssertionError, all_diagnostics.GetDiagnosticClassForName, 'BogusDiag')
