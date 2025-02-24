# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json
import logging
from dashboard.services import request

PROJECT = 'chromeperf'
LOCATION = 'us-central1'
WORKFLOW_NAME = 'sandwich-verification-workflow-prod'
REGRESSION, CULPRIT, TEST = ('regression', 'culprit', 'test')
BASE_URL = 'https://workflowexecutions.googleapis.com/v1/'
CLOUD_SCOPE = 'https://www.googleapis.com/auth/cloud-platform'

# https://cloud.google.com/workflows/docs/reference/executions/rest/v1beta/projects.locations.workflows.executions#State
EXECUTION_STATE_STATE_UNSPECIFIED = 'STATE_UNSPECIFIED'
EXECUTION_STATE_ACTIVE = 'ACTIVE'
EXECUTION_STATE_SUCCEEDED = 'SUCCEEDED'
EXECUTION_STATE_FAILED = 'FAILED'
EXECUTION_STATE_CANCELLED = 'CANCELLED'

# https://cloud.google.com/workflows/docs/reference/executions/rest/v1beta/projects.locations.workflows.executions#CallLogLevel
WORKFLOW_LOG_ALL_CALLS = 'LOG_ALL_CALLS'

def CreateExecution(anomaly,
                    verification_type=TEST,
                    project=PROJECT,
                    location=LOCATION,
                    workflow_name=WORKFLOW_NAME):
  '''
    An anomaly should be a JSON-serializable object (notably, this means it
    cannot be an instance of models.Anomaly) containing the following properties:
        - benchmark: e.g. speedometer2
        - bot_name: e.g. linux-perf
        - story: e.g. Speedometer2
        - measurement: e.g. VanillaJS-TodoMVC
        - target: e.g. performance_test_suite
        - start_git_hash: git hash of A.
        - end_git_hash: git hash of B.
        - attempt_count: (optional) default 30
    verification_type is either REGRESSION, CULPRIT or TEST. Default is TEST.

    Returns:
        The name of the Workflow execution created. The name is of the form:
        'projects/{...}/locations/{...}/workflows/{...}/executions/{...}'
    Raises:
        request.NotFoundError: The underlying http response was 4xx
        request.RequestError: The underlying http response was not 2xx
    '''

  body = {
      'argument': json.dumps({'anomaly': anomaly}),
      'labels': {
          'type': verification_type,
      },
      'callLogLevel': WORKFLOW_LOG_ALL_CALLS
  }
  workflow_id = 'projects/%s/locations/%s/workflows/%s/' % (project, location,
                                                            workflow_name)
  response = request.RequestJson(
      BASE_URL + workflow_id + 'executions',
      method='POST',
      body=body,
      scope=CLOUD_SCOPE)
  logging.info('Created Sandwich Verification execution: %s (Type: %s).',
               response['name'], verification_type)
  return response['name']


def GetExecution(execution_name):
  '''
    execution_name should be of the form
    'projects/{...}/locations/{...}/workflows/{...}/executions/{...}'

    Returns Execution object which has all metadata we need for Alert Group
    and Culprit Workflow:
        - name: name of the execution.
        - state: one of ACTIVE (1), SUCCEEDED (2), FAILED (3), or CANCELLED (4).
        - result: output of the execution. Only present if state is SUCCEEDED.
        - error: object with error context. Only present if state is FAILED or CANCELLED.
    '''
  return request.RequestJson(
      BASE_URL + execution_name, method='GET', scope=CLOUD_SCOPE)
