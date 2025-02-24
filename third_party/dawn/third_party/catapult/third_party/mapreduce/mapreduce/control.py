#!/usr/bin/env python
#
# Copyright 2010 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""API for controlling MapReduce execution outside of MapReduce framework."""



__all__ = ["start_map"]

# pylint: disable=g-bad-name
# pylint: disable=protected-access


import logging

from google.appengine.ext import db
from mapreduce import handlers
from mapreduce import model
from mapreduce import parameters
from mapreduce import util
from mapreduce.api import map_job


def start_map(name,
              handler_spec,
              reader_spec,
              mapper_parameters,
              shard_count=None,
              output_writer_spec=None,
              mapreduce_parameters=None,
              base_path=None,
              queue_name=None,
              eta=None,
              countdown=None,
              hooks_class_name=None,
              _app=None,
              in_xg_transaction=False):
  """Start a new, mapper-only mapreduce.

  Deprecated! Use map_job.start instead.

  If a value can be specified both from an explicit argument and from
  a dictionary, the value from the explicit argument wins.

  Args:
    name: mapreduce name. Used only for display purposes.
    handler_spec: fully qualified name of mapper handler function/class to call.
    reader_spec: fully qualified name of mapper reader to use
    mapper_parameters: dictionary of parameters to pass to mapper. These are
      mapper-specific and also used for reader/writer initialization.
      Should have format {"input_reader": {}, "output_writer":{}}. Old
      deprecated style does not have sub dictionaries.
    shard_count: number of shards to create.
    mapreduce_parameters: dictionary of mapreduce parameters relevant to the
      whole job.
    base_path: base path of mapreduce library handler specified in app.yaml.
      "/mapreduce" by default.
    queue_name: taskqueue queue name to be used for mapreduce tasks.
      see util.get_queue_name.
    eta: absolute time when the MR should execute. May not be specified
      if 'countdown' is also supplied. This may be timezone-aware or
      timezone-naive.
    countdown: time in seconds into the future that this MR should execute.
      Defaults to zero.
    hooks_class_name: fully qualified name of a hooks.Hooks subclass.
    in_xg_transaction: controls what transaction scope to use to start this MR
      job. If True, there has to be an already opened cross-group transaction
      scope. MR will use one entity group from it.
      If False, MR will create an independent transaction to start the job
      regardless of any existing transaction scopes.

  Returns:
    mapreduce id as string.
  """
  if shard_count is None:
    shard_count = parameters.config.SHARD_COUNT

  if mapper_parameters:
    mapper_parameters = dict(mapper_parameters)

  # Make sure this old API fill all parameters with default values.
  mr_params = map_job.JobConfig._get_default_mr_params()
  if mapreduce_parameters:
    mr_params.update(mapreduce_parameters)

  # Override default values if user specified them as arguments.
  if base_path:
    mr_params["base_path"] = base_path
  mr_params["queue_name"] = util.get_queue_name(queue_name)

  mapper_spec = model.MapperSpec(handler_spec,
                                 reader_spec,
                                 mapper_parameters,
                                 shard_count,
                                 output_writer_spec=output_writer_spec)

  if in_xg_transaction and not db.is_in_transaction():
    logging.warning("Expects an opened xg transaction to start mapreduce "
                    "when transactional is True.")

  return handlers.StartJobHandler._start_map(
      name,
      mapper_spec,
      mr_params,
      # TODO(user): Now that "queue_name" is part of mr_params.
      # Remove all the other ways to get queue_name after one release.
      queue_name=mr_params["queue_name"],
      eta=eta,
      countdown=countdown,
      hooks_class_name=hooks_class_name,
      _app=_app,
      in_xg_transaction=in_xg_transaction)
