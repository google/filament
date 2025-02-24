# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import unittest

from unittest import mock

from py_utils import retry_util


class RetryOnExceptionTest(unittest.TestCase):
  def setUp(self):
    self.num_calls = 0
    # Patch time.sleep to make tests run faster (skip waits) and also check
    # that exponential backoff is implemented correctly.
    patcher = mock.patch('time.sleep')
    self.time_sleep = patcher.start()
    self.addCleanup(patcher.stop)

  def testNoExceptionsReturnImmediately(self):
    @retry_util.RetryOnException(Exception, retries=3)
    def Test(retries=None):
      del retries
      self.num_calls += 1
      return 'OK!'

    # The function is called once and returns the expected value.
    self.assertEqual(Test(), 'OK!')
    self.assertEqual(self.num_calls, 1)

  def testRaisesExceptionIfAlwaysFailing(self):
    @retry_util.RetryOnException(KeyError, retries=5)
    def Test(retries=None):
      del retries
      self.num_calls += 1
      raise KeyError('oops!')

    # The exception is eventually raised.
    with self.assertRaises(KeyError):
      Test()
    # The function is called the expected number of times.
    self.assertEqual(self.num_calls, 6)
    # Waits between retries do follow exponential backoff.
    self.assertEqual(
        self.time_sleep.call_args_list,
        [mock.call(i) for i in (1, 2, 4, 8, 16)])

  def testOtherExceptionsAreNotCaught(self):
    @retry_util.RetryOnException(KeyError, retries=3)
    def Test(retries=None):
      del retries
      self.num_calls += 1
      raise ValueError('oops!')

    # The exception is raised immediately on the first try.
    with self.assertRaises(ValueError):
      Test()
    self.assertEqual(self.num_calls, 1)

  def testCallerMayOverrideRetries(self):
    @retry_util.RetryOnException(KeyError, retries=3)
    def Test(retries=None):
      del retries
      self.num_calls += 1
      raise KeyError('oops!')

    with self.assertRaises(KeyError):
      Test(retries=10)
    # The value on the caller overrides the default on the decorator.
    self.assertEqual(self.num_calls, 11)

  def testCanEventuallySucceed(self):
    @retry_util.RetryOnException(KeyError, retries=5)
    def Test(retries=None):
      del retries
      self.num_calls += 1
      if self.num_calls < 3:
        raise KeyError('oops!')
      return 'OK!'

    # The value is returned after the expected number of calls.
    self.assertEqual(Test(), 'OK!')
    self.assertEqual(self.num_calls, 3)

  def testRetriesCanBeSwitchedOff(self):
    @retry_util.RetryOnException(KeyError, retries=5)
    def Test(retries=None):
      del retries
      self.num_calls += 1
      if self.num_calls < 3:
        raise KeyError('oops!')
      return 'OK!'

    # We fail immediately on the first try.
    with self.assertRaises(KeyError):
      Test(retries=0)
    self.assertEqual(self.num_calls, 1)

  def testCanRetryOnMultipleExceptions(self):
    @retry_util.RetryOnException((KeyError, ValueError), retries=3)
    def Test(retries=None):
      del retries
      self.num_calls += 1
      if self.num_calls == 1:
        raise KeyError('oops!')
      if self.num_calls == 2:
        raise ValueError('uh oh!')
      return 'OK!'

    # Call eventually succeeds after enough tries.
    self.assertEqual(Test(retries=5), 'OK!')
    self.assertEqual(self.num_calls, 3)


if __name__ == '__main__':
  unittest.main()
