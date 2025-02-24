# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import sys
import time
import unittest

import py_utils
import psutil  # pylint: disable=import-error
from telemetry import decorators
from telemetry.internal.util import ps_util


class PsUtilTest(unittest.TestCase):

  @decorators.Disabled('chromeos')  # crbug.com/939730
  def testListAllSubprocesses_RaceCondition(self):
    """This is to check that crbug.com/934575 stays fixed."""
    class FakeProcess():
      def __init__(self):
        self.pid = '1234'
      def name(self):
        raise psutil.ZombieProcess('this is an error')
    output = ps_util._GetProcessDescription(FakeProcess())
    self.assertIn('ZombieProcess', output)
    self.assertIn('this is an error', output)

  def testWaitForSubProcAndKillFinished(self):
    args = [
        sys.executable,
        '-c',
        'import time; time.sleep(2)'
    ]
    sp = ps_util.RunSubProcWithTimeout(args, 3, 'test')
    self.assertEqual(sp.poll(), 0)
    self.assertTrue(
        sp.pid not in ps_util.GetAllSubprocessIDs()
    )

  def testWaitForSubProcAndKillTimeout(self):
    args = [
        sys.executable,
        '-c',
        'import time; time.sleep(10)'
    ]
    with self.assertRaises(py_utils.TimeoutException) as e:
      ps_util.RunSubProcWithTimeout(args, 1, 'test')
    time.sleep(.5)
    subprocess_ids = ps_util.GetAllSubprocessIDs()
    for subprocess_id in subprocess_ids:
      self.assertFalse(
          str(subprocess_id) in repr(e.exception),
          'The pid %d causing timeout should not exist. Exception: %s' %
          (subprocess_id, repr(e.exception)))
