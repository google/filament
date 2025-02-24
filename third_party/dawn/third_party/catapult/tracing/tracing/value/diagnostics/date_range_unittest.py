# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import calendar
import unittest

from tracing.value.diagnostics import date_range
from tracing.value.diagnostics import diagnostic


class DateRangeUnittest(unittest.TestCase):

  def testRoundtrip(self):
    dr = date_range.DateRange(1496693745000)
    dr.AddDiagnostic(date_range.DateRange(1496693746000))
    self.assertEqual(calendar.timegm(dr.min_date.timetuple()), 1496693745)
    self.assertEqual(calendar.timegm(dr.max_date.timetuple()), 1496693746)
    clone = diagnostic.Diagnostic.FromDict(dr.AsDict())
    self.assertEqual(clone.min_date, dr.min_date)
    self.assertEqual(clone.max_date, dr.max_date)

  def testMinTimestamp(self):
    dr = date_range.DateRange(1496693745123)
    dr.AddDiagnostic(date_range.DateRange(1496693746123))
    self.assertEqual(dr.min_timestamp, 1496693745123)

  def testMaxTimestamp(self):
    dr = date_range.DateRange(1496693745123)
    dr.AddDiagnostic(date_range.DateRange(1496693746123))
    self.assertEqual(dr.max_timestamp, 1496693746123)

  def testDeserialize(self):
    dr = date_range.DateRange.Deserialize(1496693746123, None)
    self.assertEqual(dr.min_timestamp, 1496693746123)
    self.assertEqual(dr.max_timestamp, 1496693746123)

    dr = date_range.DateRange.Deserialize([1496693746000, 1496693746123], None)
    self.assertEqual(dr.min_timestamp, 1496693746000)
    self.assertEqual(dr.max_timestamp, 1496693746123)

  def testSerialize(self):
    dr = date_range.DateRange(123)
    self.assertEqual(dr.Serialize(None), 123)
    dr.AddDiagnostic(date_range.DateRange(100))
    self.assertEqual(dr.Serialize(None), [100, 123])
