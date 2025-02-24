# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging
import math
import unittest
import sys

from dashboard.models import anomaly
from dashboard.pinpoint import test
from dashboard.pinpoint.models import job_state
from dashboard.pinpoint.models.change import change_test
from dashboard.pinpoint.models.quest import quest_test


class ExploreTest(test.TestCase):

  def setUp(self):
    # Intercept the logging messages, so that we can see them when we have test
    # output in failures.
    self.logger = logging.getLogger()
    self.logger.level = logging.DEBUG
    self.stream_handler = logging.StreamHandler(sys.stdout)
    self.logger.addHandler(self.stream_handler)
    self.addCleanup(self.logger.removeHandler, self.stream_handler)
    super().setUp()

  def testDifferentWithMidpoint(self):
    quests = [
        quest_test.QuestByChange({
            change_test.Change(1): quest_test.QuestPass(),
            change_test.Change(9): quest_test.QuestFail(),
        })
    ]
    state = job_state.JobState(quests, comparison_mode=job_state.PERFORMANCE)
    state.AddChange(change_test.Change(1))
    state.AddChange(change_test.Change(9))

    state.ScheduleWork()
    state.Explore()

    # The Changes are different. Add the speculated modpoints in a two-level
    # deep bisection tree, which include 3, 5 (the midpoint between 0 and 9),
    # and 7.
    expected = [
        change_test.Change(1),
        change_test.Change(5),
        change_test.Change(9),
    ]

    # View the whole diff in case of failure.
    self.maxDiff = None
    self.assertEqual(state._changes, expected)
    attempt_count_1 = len(state._attempts[change_test.Change(1)])
    attempt_count_2 = len(state._attempts[change_test.Change(5)])
    attempt_count_3 = len(state._attempts[change_test.Change(9)])
    self.assertEqual(attempt_count_1, attempt_count_2)
    self.assertEqual(attempt_count_2, attempt_count_3)
    self.assertEqual([], state.Differences())

  def testDifferentNoMidpoint(self):
    quests = [
        quest_test.QuestByChange({
            change_test.Change(1): quest_test.QuestPass(),
            change_test.Change(2): quest_test.QuestFail(),
        })
    ]
    state = job_state.JobState(quests, comparison_mode=job_state.PERFORMANCE)
    state.AddChange(change_test.Change(1))
    state.AddChange(change_test.Change(2))

    state.ScheduleWork()
    state.Explore()

    # The Changes are different, but there's no midpoint. We're done.
    self.assertEqual(len(state._changes), 2)
    attempt_count_1 = len(state._attempts[change_test.Change(1)])
    attempt_count_2 = len(state._attempts[change_test.Change(2)])
    self.assertEqual(attempt_count_1, attempt_count_2)
    self.assertEqual([(change_test.Change(1), change_test.Change(2))],
                     state.Differences())

  def testPending(self):
    quests = [quest_test.QuestSpin()]
    state = job_state.JobState(quests, comparison_mode=job_state.PERFORMANCE)
    state.AddChange(change_test.Change(1))
    state.AddChange(change_test.Change(9))

    state.Explore()

    # The results are pending. Do not add any Attempts or Changes.
    self.assertEqual(len(state._changes), 2)
    attempt_count_1 = len(state._attempts[change_test.Change(1)])
    attempt_count_2 = len(state._attempts[change_test.Change(9)])
    self.assertEqual(attempt_count_1, attempt_count_2)
    self.assertEqual([], state.Differences())

  def testSame(self):
    quests = [quest_test.QuestPass()]
    state = job_state.JobState(quests, comparison_mode=job_state.FUNCTIONAL)
    state.AddChange(change_test.Change(1))
    state.AddChange(change_test.Change(9))
    for _ in range(5):
      # More Attempts give more confidence that they are, indeed, the same.
      state.AddAttempts(change_test.Change(1))
      state.AddAttempts(change_test.Change(9))

    state.ScheduleWork()
    state.Explore()

    # The Changes are the same. Do not add any Attempts or Changes.
    self.assertEqual(len(state._changes), 2)
    attempt_count_1 = len(state._attempts[change_test.Change(1)])
    attempt_count_2 = len(state._attempts[change_test.Change(9)])
    self.assertEqual(attempt_count_1, attempt_count_2)
    self.assertEqual([], state.Differences())

  def testUnknown(self):
    quests = [quest_test.QuestPass()]
    state = job_state.JobState(
        quests, comparison_mode=job_state.FUNCTIONAL, comparison_magnitude=0.2)
    state.AddChange(change_test.Change(1))
    state.AddChange(change_test.Change(9))

    state.ScheduleWork()
    state.Explore()

    # Need more information. Add more attempts to one Change.
    self.assertEqual(len(state._changes), 2)
    attempt_count_1 = len(state._attempts[change_test.Change(1)])
    attempt_count_2 = len(state._attempts[change_test.Change(9)])
    self.assertGreaterEqual(attempt_count_1, attempt_count_2)

    state.ScheduleWork()
    state.Explore()

    # We still need more information. Add more attempts to the other Change.
    self.assertEqual(len(state._changes), 2)
    attempt_count_1 = len(state._attempts[change_test.Change(1)])
    attempt_count_2 = len(state._attempts[change_test.Change(9)])
    self.assertEqual(attempt_count_1, attempt_count_2)

  def testDifferences(self):
    quests = [quest_test.QuestPass()]
    state = job_state.JobState(
        quests, comparison_mode=job_state.FUNCTIONAL, comparison_magnitude=0.2)
    change_a = change_test.Change(1)
    change_b = change_test.Change(9)
    state.AddChange(change_a)
    state.AddChange(change_b)
    self.assertEqual([], state.Differences())

  def testNoImprovementDirection(self):
    """This unit test replicates
      if ((mean_diff > 0 and self._improvement_direction == anomaly.UP) ...
      AttributeError: 'JobState' object has no attribute '_improvement_direction'
    """
    quests = [quest_test.QuestPass()]
    state = job_state.JobState(quests, comparison_mode=job_state.PERFORMANCE)

    state.AddChange(change_test.Change(1))
    state.AddChange(change_test.Change(9))

    self.assertEqual(state._improvement_direction, anomaly.UNKNOWN)
    delattr(state, "_improvement_direction")
    self.assertIsNone(getattr(state, "_improvement_direction", None))

    state.ScheduleWork()
    state.Explore()

    self.assertEqual(state._improvement_direction, anomaly.UNKNOWN)


class ScheduleWorkTest(unittest.TestCase):

  def testNoAttempts(self):
    state = job_state.JobState(())
    self.assertFalse(state.ScheduleWork())

  def testWorkLeft(self):
    quests = [
        quest_test.QuestCycle(quest_test.QuestPass(), quest_test.QuestSpin())
    ]
    state = job_state.JobState(quests)
    state.AddChange(change_test.Change(123))
    self.assertTrue(state.ScheduleWork())

  def testNoWorkLeft(self):
    quests = [quest_test.QuestPass()]
    state = job_state.JobState(quests)
    state.AddChange(change_test.Change(123))
    self.assertTrue(state.ScheduleWork())
    self.assertFalse(state.ScheduleWork())

  def testAllAttemptsFail(self):
    quests = [
        quest_test.QuestCycle(quest_test.QuestFail(), quest_test.QuestFail(),
                              quest_test.QuestFail2())
    ]
    state = job_state.JobState(quests)
    state.AddChange(change_test.Change(123))
    exception_name = 'dashboard.pinpoint.models.errors.InformationalError'
    expected_regexp = ('.*7/10.*\n%s: Expected error for testing.$' %
                       exception_name)
    self.assertTrue(state.ScheduleWork())
    with self.assertRaisesRegex(Exception, expected_regexp):
      self.assertFalse(state.ScheduleWork())

  def testAbortAfterNChanges(self):
    quests = [quest_test.QuestSpin()]
    state = job_state.JobState(quests, comparison_mode=job_state.PERFORMANCE)
    n = job_state.MAX_BUILDS
    for i in range(n):
      state.AddChange(change_test.Change(i))
    self.assertEqual(len(state._changes), n)
    expected_regexp = ('Bisected max number of times: %d.+' % n)
    with self.assertRaisesRegex(Exception, expected_regexp):
      state.AddChange(change_test.Change(123))


class MeanTest(unittest.TestCase):

  def testValidValues(self):
    self.assertEqual(2, job_state.Mean([1, 2, 3]))

  def testInvalidValues(self):
    self.assertEqual(2, job_state.Mean([1, 2, 3, None]))

  def testNoValues(self):
    self.assertTrue(math.isnan(job_state.Mean([None])))


class FirstOrLastChangeFailedTest(unittest.TestCase):

  def testNoChanges(self):
    state = job_state.JobState(())
    self.assertFalse(state.FirstOrLastChangeFailed())

  def testNoFailedAttempts(self):
    quests = [quest_test.QuestPass()]
    state = job_state.JobState(quests)
    state.AddChange(change_test.Change(1))
    state.AddChange(change_test.Change(2))
    state.ScheduleWork()
    self.assertFalse(state.FirstOrLastChangeFailed())

  def testFailedAttempt(self):
    quests = [quest_test.QuestPass()]
    state = job_state.JobState(quests)
    state.AddChange(change_test.Change(1))
    state.AddChange(change_test.Change(2))
    state.ScheduleWork()
    for attempt in state._attempts[change_test.Change(1)]:
      attempt._last_execution._exception = "Failed"
    self.assertTrue(state.FirstOrLastChangeFailed())

  def testNoAttempts(self):
    quests = [quest_test.QuestPass()]
    state = job_state.JobState(quests)
    state.AddChange(change_test.Change(1))
    state.AddChange(change_test.Change(2))
    state.ScheduleWork()
    # It shouldn't happen that a change has no attempts, but we should cope
    # gracefully if that somehow happens.
    del state._attempts[change_test.Change(1)][:]
    del state._attempts[change_test.Change(2)][:]
    self.assertFalse(state.FirstOrLastChangeFailed())
