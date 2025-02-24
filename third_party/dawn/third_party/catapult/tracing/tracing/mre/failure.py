# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from tracing.mre import job as job_module


class Failure(object):

  def __init__(self, job, function_handle_string, trace_canonical_url,
               failure_type_name, description, stack):
    assert isinstance(job, job_module.Job)

    self.job = job
    self.function_handle_string = function_handle_string
    self.trace_canonical_url = trace_canonical_url
    self.failure_type_name = failure_type_name
    self.description = description
    self.stack = stack

  def __str__(self):
    return (
        'Failure for job %s with function handle %s and trace handle %s:\n'
        'of type "%s" with description "%s". Stack:\n\n%s' % (
            self.job.guid, self.function_handle_string,
            self.trace_canonical_url, self.failure_type_name,
            self.description, self.stack))

  def AsDict(self):
    return {
        'job_guid': str(self.job.guid),
        'function_handle_string': self.function_handle_string,
        'trace_canonical_url': self.trace_canonical_url,
        'type': self.failure_type_name,
        'description': self.description,
        'stack': self.stack
    }

  @staticmethod
  def FromDict(failure_dict, job, failure_names_to_constructors=None):
    if failure_names_to_constructors is None:
      failure_names_to_constructors = {}
    failure_type_name = failure_dict['type']
    if failure_type_name in failure_names_to_constructors:
      cls = failure_names_to_constructors[failure_type_name]
    else:
      cls = Failure

    return cls(job,
               failure_dict['function_handle_string'],
               failure_dict['trace_canonical_url'],
               failure_type_name, failure_dict['description'],
               failure_dict['stack'])
