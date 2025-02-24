#!/usr/bin/env vpython3
# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for lockfile.py"""

import logging
import os
import shutil
import sys
import tempfile
import threading
import unittest
from unittest import mock
import queue

DEPOT_TOOLS_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, DEPOT_TOOLS_ROOT)

from testing_support import coverage_utils

import lockfile

# TODO: Should fix these warnings.
# pylint: disable=line-too-long


class LockTest(unittest.TestCase):
    def setUp(self):
        self.cache_dir = tempfile.mkdtemp(prefix='lockfile')
        self.addCleanup(shutil.rmtree, self.cache_dir, ignore_errors=True)

    def testLock(self):
        with lockfile.lock(self.cache_dir):
            # cached dir locked, attempt to lock it again
            with self.assertRaises(lockfile.LockError):
                with lockfile.lock(self.cache_dir):
                    pass

        with lockfile.lock(self.cache_dir):
            pass

    @mock.patch('time.sleep')
    def testLockConcurrent(self, sleep_mock):
        '''testLockConcurrent simulates what happens when two separate processes try
    to acquire the same file lock with timeout.'''
        # Queues q_f1 and q_sleep are used to controll execution of individual
        # threads.
        q_f1 = queue.Queue()
        q_sleep = queue.Queue()
        results = queue.Queue()

        def side_effect(arg):
            '''side_effect is called when with l.lock is blocked. In this unit test
      case, it comes from f2.'''
            logging.debug('sleep: started')
            q_sleep.put(True)
            logging.debug('sleep: waiting for q_sleep to be consumed')
            q_sleep.join()
            logging.debug('sleep: waiting for result before exiting')
            results.get(timeout=1)
            logging.debug('sleep: exiting')

        sleep_mock.side_effect = side_effect

        def f1():
            '''f1 enters first in l.lock (controlled via q_f1). It then waits for
      side_effect to put a message in queue q_sleep.'''
            logging.debug('f1 started, locking')

            with lockfile.lock(self.cache_dir, timeout=1):
                logging.debug('f1: locked')
                q_f1.put(True)
                logging.debug('f1: waiting on q_f1 to be consumed')
                q_f1.join()
                logging.debug('f1: done waiting on q_f1, getting q_sleep')
                q_sleep.get(timeout=1)
                results.put(True)

            logging.debug('f1: lock released')
            q_sleep.task_done()
            logging.debug('f1: exiting')

        def f2():
            '''f2 enters second in l.lock (controlled by q_f1).'''
            logging.debug('f2: started, consuming q_f1')
            q_f1.get(timeout=1)  # wait for f1 to execute lock
            q_f1.task_done()
            logging.debug('f2: done waiting for q_f1, locking')

            with lockfile.lock(self.cache_dir, timeout=1):
                logging.debug('f2: locked')
                results.put(True)

        t1 = threading.Thread(target=f1)
        t1.start()
        t2 = threading.Thread(target=f2)
        t2.start()
        t1.join()
        t2.join()

        # One result was consumed by side_effect, we expect only one in the
        # queue.
        self.assertEqual(1, results.qsize())
        sleep_mock.assert_called_with(0.1)


if __name__ == '__main__':
    logging.basicConfig(
        level=logging.DEBUG if '-v' in sys.argv else logging.ERROR)
    sys.exit(
        coverage_utils.covered_main(
            (os.path.join(DEPOT_TOOLS_ROOT, 'git_cache.py')),
            required_percentage=0))
