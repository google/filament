# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json
import logging

from flask import make_response, request

from dashboard.common import cloud_metric
from dashboard.models import anomaly
from dashboard.pinpoint.handlers import new as new_module
from dashboard.pinpoint.models import job as job_module
from dashboard.pinpoint.models import job_state, attempt
from dashboard.pinpoint.models.change import change, commit, commit_cache
from dashboard.pinpoint.models.quest import find_isolate, run_test, read_value
from google.protobuf.timestamp_pb2 import Timestamp

CAS_URL = "https://cas-viewer.appspot.com"
SWARMING_URL = "https://chrome-swarming.appspot.com"

@cloud_metric.APIMetric("pinpoint", "/api/job")
def JobHandlerGet(job_id):
  try:
    job = job_module.JobFromId(job_id)
  except ValueError:
    return make_response(
        json.dumps({'error': 'Invalid job id: %s' % job_id}), 400)

  if not job:
    return make_response(
        json.dumps({'error': 'Unknown job id: %s' % job_id}), 404)

  opts = request.args.getlist('o')
  return make_response(json.dumps(job.AsDict(opts)))


def ParseRepoName(url):
  commit_repo_url = url
  if commit_repo_url.endswith('.git'):
    commit_repo_url = commit_repo_url[:-4]

  last_bit = commit_repo_url.split('/')[-1]
  if last_bit == 'src':
    return 'chromium'
  return last_bit


def MarshalToChange(change_data):
  all_commits = []
  for cm in change_data.get('commits', []):
    url_name = cm.get('repository')
    if url_name:
      url_name = ParseRepoName(url_name)
    gh = cm.get('gitHash', cm.get('git_hash'))
    commit_obj = commit.Commit(
        repository=url_name,
        git_hash=gh,
    )
    # this call ensures that self._repository_url is set from
    # repository datastore.
    _ = commit_obj.repository_url
    all_commits.append(commit_obj)

    created_arg = cm.get('created')
    created_time = Timestamp(
        seconds=created_arg.get('seconds'),
        nanos=created_arg.get('nanos')).ToDatetime()
    # store this commit in the commit cache
    commit_cache.Put(
        commit_obj.id_string,
        url=cm.get('url'),
        author=cm.get('author'),
        created=created_time,
        subject=cm.get('subject'),
        message=cm.get('message'))
  return change.Change(commits=all_commits)


def MarshalToAttempt(quests, change_data, legacy_attempt):
  all_executions = []

  def FindIsolateExecution(find_iso_exec):
    builder_details = find_iso_exec.get('details')[0]
    builder_name = builder_details.get('value')
    # TODO(b/322203189): Include buildbucket build ID in build info.
    iso_exec = find_isolate._FindIsolateExecution(None, builder_name, None,
                                                  None, None, None, None, None,
                                                  None)
    iso_details = find_iso_exec.get('details')[1]
    dig_url = iso_details.get('url', '')
    iso_exec._result_arguments = {
        # digest hash and bytes
        'isolate_hash': iso_details.get('value'),
        # project, or casInstance
        'isolate_server': dig_url[len(CAS_URL):].split('blobs')[0].strip('/'),
    }
    iso_exec._completed = True
    return iso_exec

  def RunTestExecution(run_test_exec):
    bot_details = run_test_exec.get('details')[0]
    task_details = run_test_exec.get('details')[1]
    isolate_details = run_test_exec.get('details')[2]

    test_exec = run_test._RunTestExecution(None, SWARMING_URL, None, None, None,
                                           None, None, None, None)
    test_exec._bot_id = bot_details.get('value', '')
    test_exec._task_id = task_details.get('value', '')

    digest = isolate_details.get('value', '').split('/')
    digest_url = isolate_details.get('url', '')
    # when the swarming task errors, no isolate is returned.
    if len(digest) == 2:
      test_exec._result_arguments = {
          'cas_root_ref': {
              'casInstance':
                  digest_url[len(CAS_URL):].split('blobs')[0].strip('/'),
              'digest': {
                  'sizeBytes': digest[1],
                  'hash': digest[0]
              }
          }
      }
    else:
      logging.debug("swarming task does not have cas output or incorrect output: %s", digest)
    test_exec._completed = True
    return test_exec

  def ReadValueExec(result_values):
    value_exec = read_value.ReadValueExecution(None, None, None, None, None,
                                               None, None, None, None, None)
    value_exec._completed = True
    value_exec._result_values = tuple(result_values)
    return value_exec

  # idx 0 is always find_isolate execution
  executions = legacy_attempt.get('executions', [])
  all_executions.append(FindIsolateExecution(executions[0]))
  all_executions.append(RunTestExecution(executions[1]))
  # read value doesn't need anything from execution details and only requires
  # the results from the attempt.
  result_values = legacy_attempt.get('resultValues',
                                     legacy_attempt.get('result_values', []))
  all_executions.append(ReadValueExec(result_values))

  a = attempt.Attempt(quests, change_data)
  a._executions = all_executions
  return a


def MarshalToState(args):
  arguments = args.get('arguments', {})

  new_args = new_module._ArgumentsWithConfiguration(arguments)
  quests = new_module._GenerateQuests(new_args)
  all_changes = []
  all_attempts = {}  # map of change to attempts
  for s in args.get('state', []):
    # only map attempt if change is defined.
    if s.get('change'):
      change_data = MarshalToChange(s.get('change'))
      all_changes.append(change_data)

      attempts_for_change = []
      for a in s.get('attempts', []):
        attempt_obj = MarshalToAttempt(quests, change_data, a)
        attempts_for_change.append(attempt_obj)
      all_attempts[change_data] = attempts_for_change

  js = job_state.JobState(
      quests=quests,
      comparison_mode=arguments.get('comparison_mode'),
      # only in arguments are they expected to be in string. elsewhere, they
      # need to be in int/float for calculations.
      # skia default is 1.0.
      comparison_magnitude=float(arguments.get('comparison_magnitude', 1.0)),
      # skia default is 10, but the pinpoint skia service will set to 20
      initial_attempt_count=int(arguments.get('initial_attempt_count', 20)),
  )
  direction_int = int(
      args.get('improvementDirection', args.get('improvement_direction', 4)))
  # see anomaly.py for mapping
  if direction_int == 0:
    js._improvement_direction = anomaly.UP
  elif direction_int == 1:
    js._improvement_direction = anomaly.DOWN
  else:
    js._improvement_direction = anomaly.UNKNOWN

  js._changes = all_changes
  js._attempts = all_attempts
  return js


def MarshalArguments(arguments):
  # all arguments need to be in string:string format.
  ret = {}
  for k, v in arguments.items():
    ret[str(k)] = str(v)

  # some keys passed in are in camel case, but catapult expects them underscore
  # delimited.
  if ret.get('comparisonMagnitude'):
    ret['comparison_magnitude'] = ret.get('comparisonMagnitude')
    del ret['comparisonMagnitude']
  if ret.get('comparisonMode'):
    ret['comparison_mode'] = ret.get('comparisonMode')
    del ret['comparisonMode']
  if ret.get('startGitHash'):
    ret['start_git_hash'] = ret.get('startGitHash')
    del ret['startGitHash']
  if ret.get('endGitHash'):
    ret['end_git_hash'] = ret.get('endGitHash')
    del ret['endGitHash']
  if ret.get('initialAttemptCount'):
    ret['initial_attempt_count'] = ret.get('initialAttemptCount')
    del ret['initialAttemptCount']
  if ret.get('storyTags'):
    ret['story_tags'] = ret.get('storyTags')
    del ret['storyTags']

  return ret


def MarshalToJob(args):
  job = job_module.Job()
  skia_job_id = args.get('job_id', args.get('JobId'))
  if skia_job_id:
    job = job_module.Job(id=skia_job_id)

  created_arg = args.get('created')
  created_timestamp = Timestamp(
      seconds=created_arg.get('seconds'),
      nanos=created_arg.get('nanos')).ToDatetime()
  updated_arg = args.get('updated')
  updated_timestamp = Timestamp(
      seconds=updated_arg.get('seconds'),
      nanos=updated_arg.get('nanos')).ToDatetime()

  arguments = MarshalArguments(args.get('arguments', {}))
  if args.get('skia_workflow_url'):
    arguments['skia_workflow_url'] = args.get('skia_workflow_url')
  job.arguments = arguments
  # rewrite into args so that job state and other sub marshal commands use the
  # same arg set.
  args['arguments'] = arguments

  job.project = args.get('project', 'chromium')
  job.comparison_mode = arguments.get('comparison_mode')
  job.gerrit_server = None
  job.gerrit_change_id = None
  default_name = '[Skia] Performance bisect on {}/{}'.format(
      arguments.get('configuration', ''), arguments.get('benchmark', ''))
  job.name = args.get('name', str(default_name))
  job.tags = args.get('tags', {})
  job.user = args.get('user')

  job.created = created_timestamp
  job.started_time = created_timestamp
  job.updated = updated_timestamp
  job.started = args.get('completed', True)

  job.cancelled = False
  job.cancel_reason = None
  job.done = True
  job.task = None
  job.exception = None
  job.exception_details = None
  job.difference_count = args.get('differenceCount', 0)
  job.retry_count = 0

  benchmark_args = job_module.BenchmarkArguments()
  benchmark_args.benchmark = arguments.get('benchmark')
  benchmark_args.story = arguments.get('story')
  benchmark_args.story_tags = arguments.get('story_tags')
  benchmark_args.chart = arguments.get('chart')
  benchmark_args.statistic = arguments.get('statistic')
  job.benchmark_arguments = benchmark_args

  job.use_execution_engine = False
  job.batch_id = None
  job.bots = args.get('bots', [])
  bug_id = args.get('bug_id')
  if bug_id:
    job.bug_id = int(bug_id)

  job.state = MarshalToState(args)
  k = job.put()

  return {
      'kind': k.kind(),
      'id': k.id(),
  }, job


@cloud_metric.APIMetric("pinpoint", "/api/job")
def JobHandlerPost():
  req_body = request.get_json(force=True)
  logging.info(json.dumps(req_body))
  if not req_body:
    return make_response(json.dumps({'error': 'Body is none'}), 400)
  resp, _ = MarshalToJob(req_body)
  return make_response(json.dumps(resp))
