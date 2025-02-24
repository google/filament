# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from tracing.mre import function_handle
from tracing.mre import map_single_trace
from tracing.mre import failure as failure_module
from tracing.mre import job as job_module
from tracing.mre import mre_result


class MreResultTests(unittest.TestCase):

  def testAsDict(self):
    result = mre_result.MreResult()

    with map_single_trace.TemporaryMapScript("""
      tr.mre.FunctionRegistry.register(
          function MyMapFunction(result, model) {
            var canonicalUrl = model.canonicalUrl;
            result.addPair('result', {
                numProcesses: model.getAllProcesses().length
              });
          });
      """) as map_script:

      module = function_handle.ModuleToLoad(filename=map_script.filename)
      map_handle = function_handle.FunctionHandle(
          modules_to_load=[module], function_name='MyMapFunction')
      job = job_module.Job(map_handle, None)
      failure = failure_module.Failure(job, '2', '3', 'err', 'desc', 'stack')
      result.AddFailure(failure)

      result.AddPair('foo', 'bar')

      result_dict = result.AsDict()

      self.assertEqual(result_dict['failures'], [failure.AsDict()])
      self.assertEqual(result_dict['pairs'], {'foo': 'bar'})

  def testAddingNonFailure(self):
    result = mre_result.MreResult()
    with self.assertRaises(ValueError):
      result.AddFailure('foo')
