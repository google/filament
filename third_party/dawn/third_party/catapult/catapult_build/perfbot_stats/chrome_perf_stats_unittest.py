#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from perfbot_stats import chrome_perf_stats


class TestChromePerfStats(unittest.TestCase):

  def testUpdateSuccessRatesWithResult(self):
    success_rates = {}
    chrome_perf_stats._UpdateSuccessRatesWithResult(
        success_rates,
        {'count': 0},
        'invalid_date_str',
        'invalid_builder')
    self.assertDictEqual({}, success_rates)
    chrome_perf_stats._UpdateSuccessRatesWithResult(
        success_rates,
        {'count': 5, 'failure_count': 3},
        '20151010',
        'android_nexus_10')
    self.assertDictEqual(
        {'20151010': {'android_nexus_10': {'count': 5, 'success_count': 2}}},
        success_rates)
    chrome_perf_stats._UpdateSuccessRatesWithResult(
        success_rates,
        {'count': 5, 'failure_count': 4},
        '20151010',
        'android_nexus_4')
    self.assertDictEqual(
        {
            '20151010': {
                'android_nexus_10': {'count': 5, 'success_count': 2},
                'android_nexus_4': {'count': 5, 'success_count': 1},
            }
        },
        success_rates)
    chrome_perf_stats._UpdateSuccessRatesWithResult(
        success_rates,
        {'count': 5, 'failure_count': 0},
        '20151009',
        'win_xp')
    self.assertDictEqual(
        {
            '20151010': {
                'android_nexus_10': {'count': 5, 'success_count': 2},
                'android_nexus_4': {'count': 5, 'success_count': 1},
            },
            '20151009': {
                'win_xp': {'count': 5, 'success_count': 5},
            },
        },
        success_rates)

  def testSummarizeSuccessRates(self):
    rates = chrome_perf_stats._SummarizeSuccessRates(
        {
            '20151010': {
                'android_nexus_10': {'count': 5, 'success_count': 2},
                'android_nexus_4': {'count': 5, 'success_count': 3},
            },
            '20151009': {
                'win_xp': {'count': 5, 'success_count': 5},
            },
        })
    self.assertListEqual([['20151010', 0.5], ['20151009', 1.0]], rates)


if __name__ == '__main__':
  unittest.main()
