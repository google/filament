# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Small test to send a put request to buildbucket."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import re


class BisectJob:
  """A buildbot bisect job started and monitored through buildbucket."""

  def __init__(self,
               try_job_id,
               good_revision,
               bad_revision,
               test_command,
               metric,
               repeats,
               timeout_minutes,
               bug_id,
               gs_bucket,
               recipe_tester_name,
               builder_host=None,
               builder_port=None,
               test_type='perf',
               required_initial_confidence=None):
    if not all([
        good_revision, bad_revision, test_command, metric, repeats,
        timeout_minutes, recipe_tester_name
    ]):
      raise ValueError('At least one of the values required for BisectJob '
                       'construction was not given or was given with a None '
                       'value.')
    self.try_job_id = try_job_id
    self.good_revision = good_revision
    self.bad_revision = bad_revision
    self.command = BisectJob.EnsureCommandPath(test_command)
    self.metric = metric
    self.repeat_count = repeats
    self.max_time_minutes = timeout_minutes
    self.bug_id = bug_id
    self.gs_bucket = gs_bucket
    self.builder_host = builder_host
    self.builder_port = builder_port
    self.test_type = test_type
    self.recipe_tester_name = recipe_tester_name
    self.required_initial_confidence = required_initial_confidence

  @staticmethod
  def EnsureCommandPath(command):
    old_perf_path_regex = re.compile(r'(?<!src/)tools/perf')
    if old_perf_path_regex.search(command):
      return old_perf_path_regex.sub('src/tools/perf', command)
    old_perf_path_regex_win = re.compile(r'(?<!src\\)tools\\perf')
    if old_perf_path_regex_win.search(command):
      return old_perf_path_regex_win.sub(r'src\\tools\\perf', command)
    return command

  def GetBuildParameters(self):
    """Prepares a nested dict containing the bisect config."""
    # TODO(robertocn): Some of these should be optional.
    bisect_config = {
        'try_job_id': self.try_job_id,
        'test_type': self.test_type,
        'command': self.command,
        'good_revision': self.good_revision,
        'bad_revision': self.bad_revision,
        'metric': self.metric,
        'repeat_count': self.repeat_count,
        'max_time_minutes': self.max_time_minutes,
        'bug_id': self.bug_id,
        'gs_bucket': self.gs_bucket,
        'builder_host': self.builder_host,
        'builder_port': self.builder_port,
        'recipe_tester_name': self.recipe_tester_name,
    }
    if self.required_initial_confidence:
      bisect_config['required_initial_confidence'] = (
          self.required_initial_confidence)
    properties = {'bisect_config': bisect_config}
    parameters = {
        'builder_name': self.recipe_tester_name,
        'properties': properties,
    }
    return parameters

  # TODO(robertocn): Add methods to query the status of a job form buildbucket.
  # TODO(robertocn): Add static method to get a job by it's buildbucket id.
  # TODO(robertocn): Add appropriate tests.
