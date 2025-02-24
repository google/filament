#!/usr/bin/env vpython3
# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys
import unittest

from datetime import datetime

DEPOT_TOOLS_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, DEPOT_TOOLS_ROOT)

import my_activity


class MyActivityTest(unittest.TestCase):
    def test_datetime_to_midnight(self):
        self.assertEqual(
            datetime(2020, 9, 12),
            my_activity.datetime_to_midnight(datetime(2020, 9, 12, 13, 0, 0)))
        self.assertEqual(
            datetime(2020, 12, 31),
            my_activity.datetime_to_midnight(datetime(2020, 12, 31, 23, 59,
                                                      59)))
        self.assertEqual(
            datetime(2020, 12, 31),
            my_activity.datetime_to_midnight(datetime(2020, 12, 31)))

    def test_get_quarter_of(self):
        self.assertEqual((datetime(2020, 7, 1), datetime(2020, 10, 1)),
                         my_activity.get_quarter_of(datetime(2020, 9, 12)))
        # Quarter range includes beginning
        self.assertEqual((datetime(2020, 10, 1), datetime(2021, 1, 1)),
                         my_activity.get_quarter_of(datetime(2020, 10, 1)))
        # Quarter range excludes end
        self.assertEqual((datetime(2021, 1, 1), datetime(2021, 4, 1)),
                         my_activity.get_quarter_of(datetime(2021, 1, 1)))
        self.assertEqual(
            (datetime(2020, 10, 1), datetime(2021, 1, 1)),
            my_activity.get_quarter_of(datetime(2020, 12, 31, 23, 59, 59)))

    def test_get_year_of(self):
        self.assertEqual((datetime(2020, 1, 1), datetime(2021, 1, 1)),
                         my_activity.get_year_of(datetime(2020, 9, 12)))
        # Year range includes beginning
        self.assertEqual((datetime(2020, 1, 1), datetime(2021, 1, 1)),
                         my_activity.get_year_of(datetime(2020, 1, 1)))
        # Year range excludes end
        self.assertEqual((datetime(2021, 1, 1), datetime(2022, 1, 1)),
                         my_activity.get_year_of(datetime(2021, 1, 1)))

    def test_get_week_of(self):
        self.assertEqual((datetime(2020, 9, 7), datetime(2020, 9, 14)),
                         my_activity.get_week_of(datetime(2020, 9, 12)))
        # Week range includes beginning
        self.assertEqual((datetime(2020, 9, 7), datetime(2020, 9, 14)),
                         my_activity.get_week_of(datetime(2020, 9, 7)))
        # Week range excludes beginning
        self.assertEqual((datetime(2020, 9, 14), datetime(2020, 9, 21)),
                         my_activity.get_week_of(datetime(2020, 9, 14)))

    def _get_issue_with_description(self, description):
        return {
            'current_revision': 'rev',
            'revisions': {
                'rev': {
                    'commit': {
                        'message': description
                    }
                }
            },
        }

    def test_extract_bug_numbers_from_description(self):
        issue = self._get_issue_with_description(
            'Title\n'
            '\n'
            'Description\n'
            'A comment:\n'
            '> Bug: 1234, another:5678\n'
            '\n'
            'Bug: another:1234, 5678\n'
            'BUG=project:13141516\n'
            'Fixed: fixed:9101112\n'
            'Change-Id: Iabcdef1234567890\n')
        self.assertEqual([
            'another:1234', 'chromium:5678', 'fixed:9101112', 'project:13141516'
        ], my_activity.extract_bug_numbers_from_description(issue))


if __name__ == '__main__':
    unittest.main()
