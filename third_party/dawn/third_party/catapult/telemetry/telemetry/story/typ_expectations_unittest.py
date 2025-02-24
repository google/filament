# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import unittest
from unittest import mock

from telemetry.story import typ_expectations


class TypStoryExpectationsTest(unittest.TestCase):

  def testDisableBenchmark(self):
    expectations = typ_expectations.StoryExpectations('fake_benchmark_name')
    raw_expectations = (
        '# tags: [ all ]\n'
        '# results: [ Skip ]\n'
        'crbug.com/123 [ all ] fake_benchmark_name/* [ Skip ]\n')
    expectations.GetBenchmarkExpectationsFromParser(raw_expectations)
    expectations.SetTags(['All'])
    reason = expectations.IsBenchmarkDisabled()
    self.assertTrue(reason)
    self.assertEqual(reason, 'crbug.com/123')

  def testDisableStory_WithReason(self):
    expectations = typ_expectations.StoryExpectations('fake_benchmark_name')
    raw_expectations = (
        '# tags: [ linux win ]\n'
        '# results: [ Skip ]\n'
        '[ linux ] fake_benchmark_name/one [ Skip ]\n'
        'crbug.com/123 [ win ] fake_benchmark_name/on* [ Skip ]\n')
    expectations.GetBenchmarkExpectationsFromParser(raw_expectations)
    expectations.SetTags(['win'])
    story = mock.MagicMock()
    story.name = 'one'
    reason = expectations.IsStoryDisabled(story)
    self.assertTrue(reason)
    self.assertEqual(reason, 'crbug.com/123')

  def testDisableStory_NoReasonGiven(self):
    expectations = typ_expectations.StoryExpectations('fake_benchmark_name')
    raw_expectations = (
        '# tags: [ linux win ]\n'
        '# results: [ Skip ]\n'
        '[ linux ] fake_benchmark_name/one [ Skip ]\n'
        'crbug.com/123 [ win ] fake_benchmark_name/on* [ Skip ]\n')
    expectations.GetBenchmarkExpectationsFromParser(raw_expectations)
    expectations.SetTags(['linux'])
    story = mock.MagicMock()
    story.name = 'one'
    reason = expectations.IsStoryDisabled(story)
    self.assertTrue(reason)
    self.assertEqual(reason, 'No reason given')
