#!/usr/bin/env python
# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Unit tests for decorators.py."""

import unittest

from devil.utils import decorators


class MemoizeDecoratorTest(unittest.TestCase):

  def testFunctionExceptionNotMemoized(self):
    """Tests that |Memoize| decorator does not cache exception results."""

    class ExceptionType1(Exception):
      pass

    class ExceptionType2(Exception):
      pass

    @decorators.Memoize
    def raiseExceptions():
      if raiseExceptions.count == 0:
        raiseExceptions.count += 1
        raise ExceptionType1()

      if raiseExceptions.count == 1:
        raise ExceptionType2()
    raiseExceptions.count = 0

    with self.assertRaises(ExceptionType1):
      raiseExceptions()
    with self.assertRaises(ExceptionType2):
      raiseExceptions()

  def testFunctionResultMemoized(self):
    """Tests that |Memoize| decorator caches results."""

    @decorators.Memoize
    def memoized():
      memoized.count += 1
      return memoized.count
    memoized.count = 0

    def notMemoized():
      notMemoized.count += 1
      return notMemoized.count
    notMemoized.count = 0

    self.assertEqual(memoized(), 1)
    self.assertEqual(memoized(), 1)
    self.assertEqual(memoized(), 1)

    self.assertEqual(notMemoized(), 1)
    self.assertEqual(notMemoized(), 2)
    self.assertEqual(notMemoized(), 3)

  def testFunctionMemoizedBasedOnArgs(self):
    """Tests that |Memoize| caches results based on args and kwargs."""

    @decorators.Memoize
    def returnValueBasedOnArgsKwargs(a, k=0):
      return a + k

    self.assertEqual(returnValueBasedOnArgsKwargs(1, 1), 2)
    self.assertEqual(returnValueBasedOnArgsKwargs(1, 2), 3)
    self.assertEqual(returnValueBasedOnArgsKwargs(2, 1), 3)
    self.assertEqual(returnValueBasedOnArgsKwargs(3, 3), 6)


if __name__ == '__main__':
  unittest.main()
