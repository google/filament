#! /usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for the contents of parallelizer.py."""

# pylint: disable=protected-access
# pylint: disable=unused-argument

import contextlib
import os
import tempfile
import time
import sys
import unittest

if __name__ == '__main__':
  sys.path.append(
      os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..')))

from devil.utils import parallelizer
from devil.base_error import BaseError


class ParallelizerTestObject(object):
  """Class used to test parallelizer.Parallelizer."""

  parallel = parallelizer.Parallelizer

  def __init__(self, thing, completion_file_name=None):
    self._thing = thing
    self._completion_file_name = completion_file_name
    self.helper = ParallelizerTestObjectHelper(thing)

  @staticmethod
  def doReturn(what):
    return what

  @classmethod
  def doRaise(cls, what):
    raise what

  def doSetTheThing(self, new_thing):
    self._thing = new_thing

  def doReturnTheThing(self):
    return self._thing

  def doRaiseTheThing(self):
    raise self._thing

  def doRaiseIfExceptionElseSleepFor(self, sleep_duration):
    if isinstance(self._thing, Exception):
      raise self._thing
    time.sleep(sleep_duration)
    self._write_completion_file()
    return self._thing

  def _write_completion_file(self):
    if self._completion_file_name and len(self._completion_file_name):
      with open(self._completion_file_name, 'w+b') as completion_file:
        completion_file.write(b'complete')

  def __getitem__(self, index):
    return self._thing[index]

  def __str__(self):
    return type(self).__name__


class ParallelizerTestObjectHelper(object):
  def __init__(self, thing):
    self._thing = thing

  def doReturnStringThing(self):
    return str(self._thing)


class ParallelizerTest(unittest.TestCase):
  def testInitEmptyList(self):
    r = parallelizer.Parallelizer([]).replace('a', 'b').pGet(0.1)
    self.assertEqual([], r)

  def testMethodCall(self):
    test_data = ['abc_foo', 'def_foo', 'ghi_foo']
    expected = ['abc_bar', 'def_bar', 'ghi_bar']
    r = parallelizer.Parallelizer(test_data).replace('_foo', '_bar').pGet(0.1)
    self.assertEqual(expected, r)

  def testMutate(self):
    devices = [ParallelizerTestObject(True) for _ in range(0, 10)]
    self.assertTrue(all(d.doReturnTheThing() for d in devices))
    ParallelizerTestObject.parallel(devices).doSetTheThing(False).pFinish(1)
    self.assertTrue(not any(d.doReturnTheThing() for d in devices))

  def testAllReturn(self):
    devices = [ParallelizerTestObject(True) for _ in range(0, 10)]
    results = ParallelizerTestObject.parallel(devices).doReturnTheThing().pGet(
        1)
    self.assertTrue(isinstance(results, list))
    self.assertEqual(10, len(results))
    self.assertTrue(all(results))

  def testAllRaise(self):
    devices = [
        ParallelizerTestObject(Exception('thing %d' % i))
        for i in range(0, 10)
    ]
    p = ParallelizerTestObject.parallel(devices).doRaiseTheThing()
    with self.assertRaises(Exception):
      p.pGet(1)

  def testOneFailOthersComplete(self):
    parallel_device_count = 10
    exception_index = 7
    exception_msg = 'thing %d' % exception_index

    try:
      completion_files = [
          tempfile.NamedTemporaryFile(delete=False)
          for _ in range(0, parallel_device_count)
      ]
      devices = [
          ParallelizerTestObject(
              i if i != exception_index else BaseError(exception_msg),
              completion_files[i].name)
          for i in range(0, parallel_device_count)
      ]
      for f in completion_files:
        f.close()
      p = ParallelizerTestObject.parallel(devices)
      with self.assertRaises(BaseError) as e:
        p.doRaiseIfExceptionElseSleepFor(2).pGet(3)
      self.assertTrue(exception_msg in str(e.exception))
      for i in range(0, parallel_device_count):
        with open(completion_files[i].name) as f:
          if i == exception_index:
            self.assertEqual('', f.read())
          else:
            self.assertEqual('complete', f.read())
    finally:
      for f in completion_files:
        os.remove(f.name)

  def testReusable(self):
    devices = [ParallelizerTestObject(True) for _ in range(0, 10)]
    p = ParallelizerTestObject.parallel(devices)
    results = p.doReturn(True).pGet(1)
    self.assertTrue(all(results))
    results = p.doReturn(True).pGet(1)
    self.assertTrue(all(results))
    with self.assertRaises(Exception):
      results = p.doRaise(Exception('reusableTest')).pGet(1)

  def testContained(self):
    devices = [ParallelizerTestObject(i) for i in range(0, 10)]
    results = (ParallelizerTestObject.parallel(devices).helper.
               doReturnStringThing().pGet(1))
    self.assertTrue(isinstance(results, list))
    self.assertEqual(10, len(results))
    for i in range(0, 10):
      self.assertEqual(str(i), results[i])

  def testGetItem(self):
    devices = [ParallelizerTestObject(range(i, i + 10)) for i in range(0, 10)]
    results = ParallelizerTestObject.parallel(devices)[9].pGet(1)
    self.assertEqual(list(range(9, 19)), results)


class SyncParallelizerTest(unittest.TestCase):
  def testContextManager(self):
    in_context = [False for i in range(10)]

    @contextlib.contextmanager
    def enter_into_context(i):
      in_context[i] = True
      try:
        yield
      finally:
        in_context[i] = False

    parallelized_context = parallelizer.SyncParallelizer(
        [enter_into_context(i) for i in range(10)])

    with parallelized_context:
      self.assertTrue(all(in_context))
    self.assertFalse(any(in_context))


if __name__ == '__main__':
  unittest.main(verbosity=2)
