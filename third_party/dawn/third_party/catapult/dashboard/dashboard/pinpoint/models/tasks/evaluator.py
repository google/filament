# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Consolidated evaluator factory module.

This module consolidates the creation of specific evaluator combinators, used
throughout Pinpoint to evaluate task graphs we support.
"""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from dashboard.pinpoint.models import evaluators
from dashboard.pinpoint.models.tasks import find_isolate
from dashboard.pinpoint.models.tasks import performance_bisection
from dashboard.pinpoint.models.tasks import read_value
from dashboard.pinpoint.models.tasks import run_test

EXCLUDED_PAYLOAD_KEYS = {'commits', 'swarming_request_body'}


class ExecutionEngine(evaluators.SequenceEvaluator):

  def __init__(self, job):
    # We gather all the evaluators from the modules we know.
    super().__init__(evaluators=[
        evaluators.DispatchByTaskType({
            'find_isolate': find_isolate.Evaluator(job),
            'find_culprit': performance_bisection.Evaluator(job),
            'read_value': read_value.Evaluator(job),
            'run_test': run_test.Evaluator(job),
        }),

        # We then always lift the task payload up, skipping some of the
        # larger objects that we know we are not going to need when deciding
        # what the end result is.
        evaluators.TaskPayloadLiftingEvaluator(
            exclude_keys=EXCLUDED_PAYLOAD_KEYS)
    ])
