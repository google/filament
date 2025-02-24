#!/usr/bin/env python
# Copyright 2011 Google Inc. All Rights Reserved.
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

"""Pipelines for mapreduce library."""



__all__ = [
    "MapperPipeline",
    ]


from mapreduce import control
from mapreduce import model
from mapreduce import parameters
from mapreduce import pipeline_base

# pylint: disable=g-bad-name
# pylint: disable=protected-access


class MapperPipeline(pipeline_base._OutputSlotsMixin,
                     pipeline_base.PipelineBase):
  """Pipeline wrapper for mapper job.

  Args:
    job_name: mapper job name as string
    handler_spec: mapper handler specification as string.
    input_reader_spec: input reader specification as string.
    output_writer_spec: output writer specification as string.
    params: mapper parameters for input reader and output writer as dict.
    shards: number of shards in the job as int.

  Returns:
    default: the list of filenames produced by the mapper if there was any
      output and the map was completed successfully.
    result_status: one of model.MapreduceState._RESULTS.
    job_id: mr id that can be used to query model.MapreduceState. Available
      immediately after run returns.
  """
  async = True

  # TODO(user): we probably want to output counters too.
  # Might also need to double filenames as named output.
  output_names = [
      # Job ID. MapreduceState.get_by_job_id can be used to load
      # mapreduce state.
      "job_id",
      # Dictionary of final counter values. Filled when job is completed.
      "counters"] + pipeline_base._OutputSlotsMixin.output_names

  def run(self,
          job_name,
          handler_spec,
          input_reader_spec,
          output_writer_spec=None,
          params=None,
          shards=None):
    """Start a mapreduce job.

    Args:
      job_name: mapreduce name. Only for display purpose.
      handler_spec: fully qualified name to your map function/class.
      input_reader_spec: fully qualified name to input reader class.
      output_writer_spec: fully qualified name to output writer class.
      params: a dictionary of parameters for input reader and output writer
        initialization.
      shards: number of shards. This provides a guide to mapreduce. The real
        number of shards is determined by how input are splited.
    """
    if shards is None:
      shards = parameters.config.SHARD_COUNT

    mapreduce_id = control.start_map(
        job_name,
        handler_spec,
        input_reader_spec,
        params or {},
        mapreduce_parameters={
            "done_callback": self.get_callback_url(),
            "done_callback_method": "GET",
            "pipeline_id": self.pipeline_id,
        },
        shard_count=shards,
        output_writer_spec=output_writer_spec,
        queue_name=self.queue_name,
        )
    self.fill(self.outputs.job_id, mapreduce_id)
    self.set_status(console_url="%s/detail?mapreduce_id=%s" % (
        (parameters.config.BASE_PATH, mapreduce_id)))

  def try_cancel(self):
    """Always allow mappers to be canceled and retried."""
    return True

  def callback(self):
    """Callback after this async pipeline finishes."""
    if self.was_aborted:
      return

    mapreduce_id = self.outputs.job_id.value
    mapreduce_state = model.MapreduceState.get_by_job_id(mapreduce_id)
    if mapreduce_state.result_status != model.MapreduceState.RESULT_SUCCESS:
      self.retry("Job %s had status %s" % (
          mapreduce_id, mapreduce_state.result_status))
      return

    mapper_spec = mapreduce_state.mapreduce_spec.mapper
    outputs = []
    output_writer_class = mapper_spec.output_writer_class()
    if (output_writer_class and
        mapreduce_state.result_status == model.MapreduceState.RESULT_SUCCESS):
      outputs = output_writer_class.get_filenames(mapreduce_state)

    self.fill(self.outputs.result_status, mapreduce_state.result_status)
    self.fill(self.outputs.counters, mapreduce_state.counters_map.to_dict())
    self.complete(outputs)
