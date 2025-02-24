#!/usr/bin/env python
"""Base pipelines."""


import pipeline

from mapreduce import parameters

# pylint: disable=g-bad-name


class PipelineBase(pipeline.Pipeline):
  """Base class for all pipelines within mapreduce framework.

  Rewrites base path to use pipeline library bundled with mapreduce.
  """

  def start(self, **kwargs):
    if "base_path" not in kwargs:
      kwargs["base_path"] = parameters._DEFAULT_PIPELINE_BASE_PATH
    return pipeline.Pipeline.start(self, **kwargs)


class _OutputSlotsMixin(object):
  """Defines common output slots for all MR user facing pipelines.

  result_status: one of model.MapreduceState._RESULTS. When a MR pipeline
    finishes, user should check this for the status of the MR job.
  """

  output_names = ["result_status"]
