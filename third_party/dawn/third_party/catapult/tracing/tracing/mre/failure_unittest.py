# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from tracing.mre import function_handle
from tracing.mre import failure as failure_module
from tracing.mre import job as job_module


def _SingleFileFunctionHandle(filename, function_name, guid):
  return function_handle.FunctionHandle(
      modules_to_load=[function_handle.ModuleToLoad(filename=filename)],
      function_name=function_name, guid=guid)


class FailureTests(unittest.TestCase):

  def testAsDict(self):
    map_function_handle = _SingleFileFunctionHandle('foo.html', 'Foo', '2')
    job = job_module.Job(map_function_handle, '1')
    failure = failure_module.Failure(job, 'foo.html:Foo',
                                     'file://foo.html',
                                     'err', 'desc', 'stack')

    self.assertEqual(failure.AsDict(), {
        'job_guid': '1',
        'function_handle_string': 'foo.html:Foo',
        'trace_canonical_url': 'file://foo.html',
        'type': 'err',
        'description': 'desc',
        'stack': 'stack'
    })

  def testFromDict(self):
    map_function_handle = _SingleFileFunctionHandle('foo.html', 'Foo', '2')
    job = job_module.Job(map_function_handle, '1')

    failure_dict = {
        'job_guid': '1',
        'function_handle_string': 'foo.html:Foo',
        'trace_canonical_url': 'file://foo.html',
        'type': 'err',
        'description': 'desc',
        'stack': 'stack'
    }

    failure = failure_module.Failure.FromDict(failure_dict, job)

    self.assertEqual(failure.job.guid, '1')
    self.assertEqual(failure.function_handle_string, 'foo.html:Foo')
    self.assertEqual(failure.trace_canonical_url, 'file://foo.html')
    self.assertEqual(failure.failure_type_name, 'err')
    self.assertEqual(failure.description, 'desc')
    self.assertEqual(failure.stack, 'stack')
