# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging
from unittest import mock
import sys
import unittest

from dashboard.pinpoint.models import exploration


def FindMidpoint(a, b):
  offset = (b - a) // 2
  if offset == 0:
    return None
  return a + offset


def ChangeAlwaysDetected(*_):
  return True


class ExplorationTest(unittest.TestCase):

  def setUp(self):
    # Intercept the logging messages, so that we can see them when we have test
    # output in failures.
    self.logger = logging.getLogger()
    self.logger.level = logging.DEBUG
    self.stream_handler = logging.StreamHandler(sys.stdout)
    self.logger.addHandler(self.stream_handler)
    self.addCleanup(self.logger.removeHandler, self.stream_handler)

  def testSpeculateEmpty(self):
    results = exploration.Speculate(
        [],
        change_detected=lambda *_: False,
        on_unknown=lambda *args: self.fail("on_unknown called with %r" %
                                           (args,)),
        midpoint=lambda *_: None,
        levels=100)
    self.assertEqual(results, [])

  def testSpeculateOdd(self):
    changes = [1, 6]

    results = exploration.Speculate(
        changes,
        change_detected=ChangeAlwaysDetected,
        on_unknown=lambda *args: self.fail("on_unknown called with %r" %
                                           (args,)),
        midpoint=FindMidpoint,
        levels=2)
    for index, change in results:
      changes.insert(index, change)
    self.assertEqual(changes, [1, 2, 3, 4, 6])

  def testSpeculateEven(self):
    changes = [0, 100]

    results = exploration.Speculate(
        changes,
        change_detected=ChangeAlwaysDetected,
        on_unknown=lambda *args: self.fail("on_unknown called with %r" %
                                           (args,)),
        midpoint=FindMidpoint,
        levels=2)
    for index, change in results:
      changes.insert(index, change)
    self.assertEqual(changes, [0, 25, 50, 75, 100])

  def testSpeculateUnbalanced(self):
    changes = [0, 9, 100]

    results = exploration.Speculate(
        changes,
        change_detected=ChangeAlwaysDetected,
        on_unknown=lambda *args: self.fail("on_unknown called with %r" %
                                           (args,)),
        midpoint=FindMidpoint,
        levels=2)
    for index, change in results:
      changes.insert(index, change)
    self.assertEqual(changes, [0, 2, 4, 6, 9, 31, 54, 77, 100])

  def testSpeculateIterations(self):
    on_unknown_mock = mock.MagicMock()
    changes = [0, 10]

    results = exploration.Speculate(
        changes,
        change_detected=ChangeAlwaysDetected,
        on_unknown=on_unknown_mock,
        midpoint=FindMidpoint,
        levels=2)
    for index, change in results:
      changes.insert(index, change)
    self.assertEqual(changes, [0, 2, 5, 7, 10])

    # Run the bisection again and get the full range.
    results = exploration.Speculate(
        changes,
        change_detected=ChangeAlwaysDetected,
        on_unknown=lambda *args: self.fail("on_unknown called with %r" %
                                           (args,)),
        midpoint=FindMidpoint,
        levels=2)
    for index, change in results:
      changes.insert(index, change)
    self.assertEqual(changes, list(range(11)))

  def testSpeculateHandleUnknown(self):
    on_unknown_mock = mock.MagicMock()
    changes = [0, 5, 10]

    def ChangeUnknownDetected(a, _):
      if a >= 5:
        return None
      return True

    results = exploration.Speculate(
        changes,
        change_detected=ChangeUnknownDetected,
        on_unknown=on_unknown_mock,
        midpoint=FindMidpoint,
        levels=2)
    for index, change in results:
      changes.insert(index, change)
    self.assertTrue(on_unknown_mock.called)
    self.assertEqual(changes, [0, 1, 2, 3, 5, 10])

  def testSpeculateHandleChangeNeverDetected(self):
    results = exploration.Speculate(
        [0, 1000],
        change_detected=lambda *_: False,
        on_unknown=lambda *args: self.fail("on_unknown called with %r" %
                                           (args,)),
        midpoint=FindMidpoint,
        levels=2)
    self.assertEqual(list(results), [])
