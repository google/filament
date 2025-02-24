# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging

import json
import functions_framework
from flask import jsonify
from google.protobuf import json_format

from common import pinpoint_service, cabe_service

DEFAULT_ATTEMPT_COUNT = 30

@functions_framework.http
def StartPinpointJob(request):
  """Start a Pinpoint job from an input anomaly.

  Args:
    anomaly: a map with the following attributes:
      benchmark (e.g. "speedometer2")
      story (e.g. "Speedometer2")
      start_git_hash (a valid git hash)
      end_git_hash (a valid git hash)
      bot_name (e.g. "mac-m1_mini_2020-perf")
      target (e.g. "performance_test_suite")
      attempt_count (optional, default 30)
    execution_id: An optional Workflow execution ID to add to the tags.
  Returns:
    Job ID of started Pinpoint Job.
  """
  request_json = request.get_json(silent=True)

  print('Original params: %s' % request_json)
  anomaly = request_json.get('anomaly')
  execution_id = request_json.get('execution_id')

  bot_name = anomaly.get('bot_name')
  benchmark_name = anomaly.get('benchmark')
  measurement = anomaly.get('measurement')

  attempt_count = anomaly.get('attempt_count')
  if not attempt_count:
    attempt_count = DEFAULT_ATTEMPT_COUNT

  name = 'Regression Verification Try job on %s/%s/%s' % (bot_name, benchmark_name, measurement)

  tags = json.dumps({'sandwich_execution_id': execution_id} if execution_id else {})

  pinpoint_params = {
      'benchmark': benchmark_name,
      'story': anomaly.get('story'),
      'base_git_hash': anomaly.get('start_git_hash'),
      'end_git_hash': anomaly.get('end_git_hash'),
      'configuration': bot_name,
      'initial_attempt_count': attempt_count,
      'target': anomaly.get('target'),
      'name': name,
      'comparison_mode': 'try',
      'try': 'on',
      'tags': tags,
      'project': anomaly.get('project', 'chromium')
  }

  print('Starting job with params: %s' % pinpoint_params)

  results = pinpoint_service.NewJob(pinpoint_params)

  print('Starting job response: %s' % results)

  job_id = results.get('jobId')
  if not job_id:
    error_msg = results.get('error')
    return ('Pinpoint could not start the job. Error message: %s' % error_msg), 500

  return jsonify({'job_id': results.get('jobId')})


@functions_framework.http
def PollPinpointJob(request):
  """Poll Pinpoint service for status of a job.

  Args:
    job_id: a valid Pinpoint job id.
  Returns:
    Status from {"Completed", "Queued", "Cancelled", "Failed", "Running"}
  """
  request_json = request.get_json(silent=True)

  print('Original params: %s' % request_json)

  job_id = request_json.get('job_id')

  print("Getting Job: %s" % job_id)

  results = pinpoint_service.GetJob(job_id)

  print('Getting job response: %s' % results)

  return jsonify({'status': results.get('status')})


@functions_framework.http
def GetCabeAnalysis(request):
  """Call CABE Analysis API.

  Get a CABE Analysis object from a Pinpoint Job. It'll return a list of
  statistics for multiple workloads.

  (optional) anomaly.measurement input will specify which workload
  we're interested in.

  Args:
    job_id: a valid Completed Pinpoint job id.
    anomaly: (optional) an map with the following attribute:
      measurement: workload (e.g. "AngularJS-TodoMVC")
  Returns:
    A mapping of benchmark, workloads, and confidence intervals.
    If anomaly is specified, only the specified workload will
    be included in the response.
    response = {benchmark: {workload: statistic}}
  """

  print('Original request: %s' % request)
  print('Original request.content_type: %s' % request.content_type)
  req_data = request.get_data(as_text=True)
  print('Original request data: %s' % req_data)

  request_json = request.get_json(silent=True)

  print('Original params: %s' % request_json)

  job_id = request_json.get('job_id')

  benchmark, measurement = None, None
  if request_json.get('anomaly'):
    benchmark = request_json.get('anomaly').get('benchmark')
    measurement = request_json.get('anomaly').get('measurement')

  print("Getting CABE Analysis from Job: %s, %s, %s" % (job_id, benchmark, measurement))
  results = cabe_service.GetAnalysis(job_id, benchmark, measurement)
  print("CABE Analysis response: %s" % results)

  response = {}

  for result in results:
    if len(result.experiment_spec.analysis.benchmark) > 1:
      logging.warning(
          "experiment_spect.analysis returned %d benchmarks instead of 1.",
          len(result.experiment_spec.analysis.benchmark))
    for benchmark in result.experiment_spec.analysis.benchmark:
      # each benchmark and workload is loaded separately so this if statement
      # is necessary to prevent results from being overwritten
      if benchmark.name not in response:
        response[benchmark.name] = {}
      for workload in benchmark.workload:
        if not measurement or measurement == workload:
          # If you don't use json_format.MessageToDict here and try to
          # pass the `statistic` raw proto object, you'll get errors
          # saying is "is not JSON serializable"
          statistic = json_format.MessageToDict(
              result.statistic,
              # This parameter tells it to use the field names as they appear
              # in the orginal proto definition (snake_case) instead of the
              # camelCaseNames you'd get otherwise.
              preserving_proto_field_name=True)
          response[benchmark.name].update({workload: statistic})

  print("CABE Analysis for Job ID %s response: %s" % (job_id, response))
  return jsonify(response)


@functions_framework.http
def RegressionDetection(request):
  """Determine whether an anomaly is statistically significant.

  Given upper and lower confidence intervals, we currently use a simple
  formula to determine whether an anomaly is significant. If upper and lower
  are both negative or both positive with a p-value < 0.05, then we consider
  the anomaly a verified regression.

  Args:
    statistic: map with upper and lower confidence interval values.
  Returns:
    A decision on whether an anomaly is statistically significant.
  """
  request_json = request.get_json(silent=True)

  print('Original params: %s' % request_json)

  statistic = request_json.get('statistic')

  ci_lower = statistic.get('lower')
  ci_upper = statistic.get('upper')
  p_value = statistic.get('p_value')
  control_median = statistic.get('control_median')
  treatment_median = statistic.get('treatment_median')

  # Check improvement, improvement is not regression.
  improvement_direction = request_json.get('anomaly', {}).get('improvement_dir')
  is_improvement = IsImprovement(control_median, treatment_median,
                                 improvement_direction)
  if is_improvement:
    print("Found an improvement: job id - %s, anomaly - %s, statistic - %s." %
          (request_json.get('job_id'), request_json.get('anomaly'), statistic))
    decision = False
    return jsonify({'decision': decision})


  if (ci_lower is None or ci_upper is None) and p_value is None:
    return ("Bad request; ci_upper: %s, ci_lower: %s, p_value: %s" %  ci_lower,
    ci_upper, p_value), 400

  decision = False

  # TODO(crbug/1455502): Define regression threshold based on story
  # specific attributes such as the anomaly's magnitude or the
  # subscription thresholds.
  # CI values might be Infinity if the result changes from zero to none zero
  if p_value is None:
    if (ci_lower != "Infinity" and ci_upper != "Infinity") and (ci_lower * ci_upper > 0):
      decision = True
  elif ci_lower is None or ci_upper is None or ci_lower == "Infinity" or ci_upper == "Infinity":
    if p_value < 0.05:
      decision = True
  elif ci_lower * ci_upper > 0 and p_value < 0.05:
    decision = True

  return jsonify({'decision': decision})


def IsImprovement(control_median, treatment_median, improvement_direction):
  if (improvement_direction is not None and control_median is not None
      and control_median != "infinity" and treatment_median is not None
      and treatment_median != "infinity"):
    if (treatment_median > control_median and improvement_direction
        == 'UP') or (treatment_median < control_median
                     and improvement_direction == 'DOWN'):
      return True

  return False
