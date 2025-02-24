# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections

from dashboard.pinpoint import test
from dashboard.pinpoint.models import change as change_module
from dashboard.pinpoint.models import evaluators
from dashboard.pinpoint.models import event as event_module
from dashboard.pinpoint.models import task as task_module
from dashboard.pinpoint.models.tasks import performance_bisection
from dashboard.pinpoint.models.tasks import read_value


class BisectionTestBase(test.TestCase):

  @property
  def start_change(self):
    return change_module.Change.FromDict(
        {'commits': [{
            'repository': 'chromium',
            'git_hash': 'commit_0'
        }]})

  @property
  def end_change(self):
    return change_module.Change.FromDict(
        {'commits': [{
            'repository': 'chromium',
            'git_hash': 'commit_5'
        }]})

  def PopulateSimpleBisectionGraph(self, job):
    """Helper function to populate a task graph representing a bisection.

    This function will populate the following graph on the associated job
    initialised in the setUp function:

    find_culprit
     |   |
     |   +--> read_value(start_cl, [0..min_attempts])
     |          |
     |          +--> run_test(start_cl, [0..min_attempts])
     |                 |
     |                 +--> find_isolate(start_cl)
     |
     +--> read_value(end_cl, [0..min_attempts])
            |
            +--> run_test(end_cl, [0..min_attempts])
                   |
                   +--> find_isolate(end_cl)


    This is the starting point for all bisections on which we expect the
    evaluator implementation will be operating with. In this specific case,
    we're setting min_attempts at 10 and max_attempts at 100, then using the
    special `commit_0` and `commit_5` git hashes as the range to bisect over.
    The test base class sets up special meanings for these pseudo-hashes and all
    infrastructure related to expanding that range.
    """
    # TODO(dberris): Remove the reliance on free-form arguments dict processing.
    sample_args = {
        'browser': 'some_browser',
        'configuration': 'some_bot',
    }
    task_module.PopulateTaskGraph(
        job,
        performance_bisection.CreateGraph(
            performance_bisection.TaskOptions(
                build_option_template=performance_bisection.BuildOptionTemplate(
                    builder='Some Builder',
                    target='performance_telemetry_test',
                    bucket='luci.bucket'),
                test_option_template=performance_bisection.TestOptionTemplate(
                    swarming_server='some_server',
                    dimensions=[],
                    extra_args=[],
                ),
                read_option_template=performance_bisection.ReadOptionTemplate(
                    benchmark='some_benchmark',
                    histogram_options=read_value.HistogramOptions(
                        grouping_label='some_grouping_label',
                        story='some_story',
                        statistic='avg',
                        histogram_name='some_chart',
                    ),
                    graph_json_options=read_value.GraphJsonOptions(
                        chart='some_chart',
                        trace='some_trace',
                    ),
                    mode='histogram_sets'),
                analysis_options=performance_bisection.AnalysisOptions(
                    comparison_magnitude=1.0,
                    min_attempts=10,
                    max_attempts=100,
                ),
                start_change=self.start_change,
                end_change=self.end_change,
                pinned_change=None,
            ), sample_args))

  def BisectionEvaluatorForTesting(self, *seeded_evaluators):
    """Creates an evaluator for bisection with the provided evaluators.

    This is a utility function for creating a bisection evaluator which has the
    list of evaluators nested within the sequence of evaluators.
    """
    return evaluators.SequenceEvaluator([
        evaluators.FilteringEvaluator(
            predicate=evaluators.All(evaluators.TaskStatusIn({'pending'})),
            delegate=evaluators.SequenceEvaluator(
                list(seeded_evaluators) +
                [evaluators.TaskPayloadLiftingEvaluator()])),
        evaluators.SequenceEvaluator([
            performance_bisection.Evaluator(self.job),
            evaluators.TaskPayloadLiftingEvaluator(exclude_keys={'commits'})
        ]),
    ])


class UpdateWrapper(
    collections.namedtuple('UpdateWrapper',
                           ('job', 'task', 'new_state', 'payload'))):
  # We use a named tuple to allow the default string transformation of turning
  # the key-value paired values to be rendered nicely. This aids in debugging
  # tests when the actions are serialised to the log before they are performed
  # by the execution engine.

  def __call__(self, _):
    return task_module.UpdateTask(
        self.job, self.task.id, new_state=self.new_state, payload=self.payload)


class FakeReadValueSameResult(
    collections.namedtuple('FakeReadValueSameResult', (
        'job',
        'result',
    ))):
  __slots__ = ()

  def __call__(self, task, *_):
    if task.task_type != 'read_value' or task.status == 'completed':
      return None

    task.payload.update({'result_values': [self.result]})
    return [UpdateWrapper(self.job, task, 'completed', task.payload)]


class FakeReadValueFails(collections.namedtuple('FakeReadValueFails', ('job'))):
  __slots__ = ()

  def __call__(self, task, *_):
    if task.task_type != 'read_value':
      return None

    task.payload.update({
        'errors': [{
            'reason': 'SomeReason',
            'message': 'This is a message explaining things.',
        }]
    })
    return [UpdateWrapper(self.job, task, 'failed', task.payload)]


class FakeReadValueMapResult(
    collections.namedtuple('FakeReadValueMapResult', ('job', 'value_map'))):
  __slots__ = ()

  def __call__(self, task, *_):
    if task.task_type != 'read_value':
      return None

    task.payload.update({
        'result_values':
            self.value_map[change_module.Change.FromDict(
                task.payload.get('change'))]
    })
    return [UpdateWrapper(self.job, task, 'completed', task.payload)]


def FakeNotFoundIsolate(job, task, *_):
  if task.status == 'completed':
    return None

  return [UpdateWrapper(job, task, 'completed', task.payload)]


class FakeFoundIsolate(collections.namedtuple('FakeFoundIsolate', ('job'))):

  def __call__(self, task, *_):
    if task.task_type != 'find_isolate':
      return None

    if task.status == 'completed':
      return None

    task.payload.update({
        'buildbucket_job_status': {
            'status': 'COMPLETED',
            'result': 'SUCCESS',
            'result_details_json': '{}',
        },
        'buildbucket_result': {
            'build': {
                'id': '345982437987234',
                'url': 'https://builbucket/url',
            }
        },
        'isolate_server': 'https://isolate.server',
        'isolate_hash': '12049adfa129339482234098',
    })
    return [UpdateWrapper(self.job, task, 'completed', task.payload)]


class FakeFindIsolateFailed(
    collections.namedtuple('FakeFoundIsolateFailed', ('job'))):

  def __call__(self, task, *_):
    if task.task_type != 'find_isolate':
      return None

    if task.status == 'failed':
      return None

    task.payload.update({
        'tries': 1,
        'buildbucket_job_status': {
            'status': 'COMPLETED',
            'result': 'FAILURE',
            'result_details_json': '{}',
        }
    })
    return [UpdateWrapper(self.job, task, 'failed', task.payload)]


class FakeSuccessfulRunTest(
    collections.namedtuple('FakeSuccessfulRunTest', ('job'))):

  def __call__(self, task, *_):
    if task.task_type != 'run_test':
      return None

    if task.status == 'completed':
      return None

    task.payload.update({
        'swarming_task_result': {
            'bot_id': 'some_bot',
            'task': 'some_task_id',
        },
        'isolate_server': 'https://isolate.server',
        'isolate_hash': '12334981aad2304ff1243458',
    })
    return [UpdateWrapper(self.job, task, 'completed', task.payload)]


class FakeFailedRunTest(collections.namedtuple('FakeFailedRunTest', ('job'))):

  def __call__(self, task, *_):
    if task.task_type != 'run_test':
      return None

    if task.status == 'failed':
      return None

    task.payload.update({
        'errors': [{
            'reason': 'SomeReason',
            'message': 'There is some message here.',
        }]
    })
    return [UpdateWrapper(self.job, task, 'failed', task.payload)]


def SelectEvent():
  return event_module.Event(type='select', target_task=None, payload={})
