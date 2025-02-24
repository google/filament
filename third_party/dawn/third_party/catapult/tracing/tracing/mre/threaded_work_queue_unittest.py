# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import unittest

from tracing.mre import threaded_work_queue


class ThreadedWorkQueueTests(unittest.TestCase):

  def testSingleThreaded(self):
    wq = threaded_work_queue.ThreadedWorkQueue(num_threads=1)
    self._RunSimpleDecrementingTest(wq)

  def testMultiThreaded(self):
    wq = threaded_work_queue.ThreadedWorkQueue(num_threads=4)
    self._RunSimpleDecrementingTest(wq)

  def testSingleThreadedWithException(self):
    def Ex():
      raise Exception("abort")

    wq = threaded_work_queue.ThreadedWorkQueue(num_threads=1)
    wq.PostAnyThreadTask(Ex)
    res = wq.Run()
    self.assertEqual(res, None)

  def _RunSimpleDecrementingTest(self, wq):

    remaining = [10]

    def Decrement():
      remaining[0] -= 1
      if remaining[0]:
        wq.PostMainThreadTask(Done)

    def Done():
      wq.Stop(314)

    wq.PostAnyThreadTask(Decrement)
    res = wq.Run()
    self.assertEqual(res, 314)
