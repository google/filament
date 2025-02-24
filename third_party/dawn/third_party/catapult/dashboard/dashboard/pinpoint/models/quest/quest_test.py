# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import itertools
import unittest

from dashboard.pinpoint.models.quest import execution_test
from dashboard.pinpoint.models.quest import quest


class _QuestStub(quest.Quest):

  def __str__(self):
    return 'Quest'

  @classmethod
  def FromDict(cls, arguments):
    return cls()


class QuestCycle(_QuestStub):
  """Cycles through the given Quest classes."""

  def __init__(self, *quests):
    """Creates a QuestCycle.

    Args:
      quests: An iterable of Quest intances.
    """
    self._execution_classes = itertools.cycle(quests)

  def Start(self, change):
    return next(self._execution_classes).Start(change)


class QuestByChange(_QuestStub):
  """Uses a different Quest for each Change."""

  def __init__(self, change_mapping):
    """Creates a QuestByChange.

    Args:
      change_mapping: A dict mapping each Change to
          the Quest instance to use for that Change.
    """
    self._change_mapping = change_mapping

  def Start(self, change):
    return self._change_mapping[change].Start(change)


class QuestException(_QuestStub):

  def Start(self, change):
    del change
    return execution_test.ExecutionException()


class QuestFail(_QuestStub):

  def Start(self, change):
    del change
    return execution_test.ExecutionFail()


class QuestFail2(_QuestStub):

  def Start(self, change):
    del change
    return execution_test.ExecutionFail2()


class QuestPass(_QuestStub):

  def Start(self, change):
    del change
    return execution_test.ExecutionPass()


class QuestSpin(_QuestStub):

  def Start(self, change):
    del change
    return execution_test.ExecutionSpin()


class QuestTest(unittest.TestCase):
  """Unit tests for quest.py, for coverage."""

  def testQuest(self):
    with self.assertRaises(NotImplementedError):
      str(quest.Quest())

    with self.assertRaises(AttributeError):
      quest.Quest().Start()

    with self.assertRaises(NotImplementedError):
      quest.Quest.FromDict({})
