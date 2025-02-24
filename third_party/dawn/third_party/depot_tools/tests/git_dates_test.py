#!/usr/bin/env vpython3
# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Tests for git_dates."""

import datetime
import os
import sys
import unittest

DEPOT_TOOLS_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, DEPOT_TOOLS_ROOT)

from testing_support import coverage_utils


class GitDatesTestBase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        super(GitDatesTestBase, cls).setUpClass()
        import git_dates
        cls.git_dates = git_dates


class GitDatesTest(GitDatesTestBase):
    def testTimestampOffsetToDatetime(self):
        # 2016-01-25 06:25:43 UTC
        timestamp = 1453703143

        offset = '+1100'
        expected_tz = self.git_dates.FixedOffsetTZ(datetime.timedelta(hours=11),
                                                   '')
        expected = datetime.datetime(2016,
                                     1,
                                     25,
                                     17,
                                     25,
                                     43,
                                     tzinfo=expected_tz)
        result = self.git_dates.timestamp_offset_to_datetime(timestamp, offset)
        self.assertEqual(expected, result)
        self.assertEqual(datetime.timedelta(hours=11), result.utcoffset())
        self.assertEqual('+1100', result.tzname())
        self.assertEqual(datetime.timedelta(0), result.dst())

        offset = '-0800'
        expected_tz = self.git_dates.FixedOffsetTZ(datetime.timedelta(hours=-8),
                                                   '')
        expected = datetime.datetime(2016,
                                     1,
                                     24,
                                     22,
                                     25,
                                     43,
                                     tzinfo=expected_tz)
        result = self.git_dates.timestamp_offset_to_datetime(timestamp, offset)
        self.assertEqual(expected, result)
        self.assertEqual(datetime.timedelta(hours=-8), result.utcoffset())
        self.assertEqual('-0800', result.tzname())
        self.assertEqual(datetime.timedelta(0), result.dst())

        # Invalid offset.
        offset = '-08xx'
        expected_tz = self.git_dates.FixedOffsetTZ(datetime.timedelta(hours=0),
                                                   '')
        expected = datetime.datetime(2016, 1, 25, 6, 25, 43, tzinfo=expected_tz)
        result = self.git_dates.timestamp_offset_to_datetime(timestamp, offset)
        self.assertEqual(expected, result)
        self.assertEqual(datetime.timedelta(hours=0), result.utcoffset())
        self.assertEqual('UTC', result.tzname())
        self.assertEqual(datetime.timedelta(0), result.dst())

        # Offset out of range.
        offset = '+2400'
        self.assertRaises(ValueError,
                          self.git_dates.timestamp_offset_to_datetime,
                          timestamp, offset)

    def testDatetimeString(self):
        tz = self.git_dates.FixedOffsetTZ(datetime.timedelta(hours=11), '')
        dt = datetime.datetime(2016, 1, 25, 17, 25, 43, tzinfo=tz)
        self.assertEqual('2016-01-25 17:25:43 +1100',
                         self.git_dates.datetime_string(dt))

        tz = self.git_dates.FixedOffsetTZ(datetime.timedelta(hours=-8), '')
        dt = datetime.datetime(2016, 1, 24, 22, 25, 43, tzinfo=tz)
        self.assertEqual('2016-01-24 22:25:43 -0800',
                         self.git_dates.datetime_string(dt))


if __name__ == '__main__':
    sys.exit(
        coverage_utils.covered_main(
            os.path.join(DEPOT_TOOLS_ROOT, 'git_dates.py')))
