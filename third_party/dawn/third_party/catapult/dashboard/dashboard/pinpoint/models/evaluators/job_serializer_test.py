# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from unittest import mock
import logging
import pprint

from dashboard.pinpoint.models import evaluators
from dashboard.pinpoint.models import job as job_module
from dashboard.pinpoint.models import task as task_module
from dashboard.pinpoint.models.tasks import bisection_test_util
from dashboard.pinpoint.models.tasks import performance_bisection


@mock.patch.object(job_module.results2, 'GetCachedResults2',
                   mock.MagicMock(return_value='http://foo'))
class EvaluatorTest(bisection_test_util.BisectionTestBase):

  def setUp(self):
    super().setUp()
    self.maxDiff = None
    with mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
                    mock.MagicMock(return_value=["a"])):
      self.job = job_module.Job.New(
          (), (),
          arguments={
              'configuration': 'some_configuration',
              'target': 'performance_telemetry_test',
              'browser': 'some_browser',
              'bucket': 'some_bucket',
              'builder': 'some_builder',
          },
          comparison_mode=job_module.job_state.PERFORMANCE,
          use_execution_engine=True)

  def testSerializeEmptyJob(self):
    self.PopulateSimpleBisectionGraph(self.job)
    self.assertEqual(
        {
            'arguments': mock.ANY,
            'batch_id': None,
            'bots': ['a'],
            'bug_id': None,
            'project': 'chromium',
            'cancel_reason': None,
            'comparison_mode': 'performance',
            'configuration': mock.ANY,
            'created': mock.ANY,
            'difference_count': None,
            'exception': None,
            'improvement_direction': mock.ANY,
            'job_id': self.job.job_id,
            'name': mock.ANY,
            'quests': ['Build', 'Test'],
            'results_url': mock.ANY,
            'started_time': mock.ANY,
            'state': [mock.ANY, mock.ANY],
            'status': 'Queued',
            'updated': mock.ANY,
            'user': None
        },
        self.job.AsDict(
            options=[job_module.OPTION_STATE, job_module.OPTION_ESTIMATE]))

  def testSerializeJob(self):
    self.PopulateSimpleBisectionGraph(self.job)
    task_module.Evaluate(
        self.job, bisection_test_util.SelectEvent(),
        evaluators.SequenceEvaluator([
            evaluators.DispatchByTaskType({
                'find_isolate':
                    bisection_test_util.FakeFoundIsolate(self.job),
                'run_test':
                    bisection_test_util.FakeSuccessfulRunTest(self.job),
                'read_value':
                    bisection_test_util.FakeReadValueSameResult(self.job, 1.0),
                'find_culprit':
                    performance_bisection.Evaluator(self.job),
            }),
            evaluators.TaskPayloadLiftingEvaluator()
        ]))
    logging.debug('Finished evaluating job state.')
    job_dict = self.job.AsDict(options=[job_module.OPTION_STATE])
    logging.debug('Job = %s', pprint.pformat(job_dict))
    self.assertTrue(self.job.use_execution_engine)
    self.assertEqual(
        {
            'arguments':
                mock.ANY,
            'batch_id':
                None,
            'bots': ['a'],
            'bug_id':
                None,
            'project':
                'chromium',
            'cancel_reason':
                None,
            'comparison_mode':
                'performance',
            'configuration':
                'some_configuration',
            'created':
                mock.ANY,
            'difference_count':
                0,
            'exception':
                None,
            'improvement_direction':
                mock.ANY,
            'job_id':
                mock.ANY,
            'metric':
                'some_benchmark',
            'name':
                mock.ANY,
            'quests': ['Build', 'Test', 'Get results'],
            'results_url':
                mock.ANY,
            'started_time':
                mock.ANY,
            'status':
                mock.ANY,
            'updated':
                mock.ANY,
            'user':
                None,
            # NOTE: Here we're asseessing the structure of the results, not the
            # actual contents. We'll reserve more specific content form testing
            # in other test cases, but for now we're ensuring that we're able to
            # get the shape of the data in a certain way.
            'state': [{
                'attempts': [{
                    'executions': [mock.ANY] * 3
                }] + [{
                    'executions': [None, mock.ANY, mock.ANY]
                }] * 9,
                'change':
                    self.start_change.AsDict(),
                'comparisons': {
                    'prev': None,
                    'next': 'same',
                },
                'result_values': [mock.ANY] * 10,
            }, {
                'attempts': [{
                    'executions': [mock.ANY] * 3
                }] + [{
                    'executions': [None, mock.ANY, mock.ANY]
                }] * 9,
                'change':
                    self.end_change.AsDict(),
                'comparisons': {
                    'prev': 'same',
                    'next': None,
                },
                'result_values': [mock.ANY] * 10,
            }]
        },
        job_dict)
