# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os

from six.moves import map  # pylint: disable=redefined-builtin
import tracing_project
import vinn

FORMAT_TO_METHOD = {
    'chartjson': 'compareCharts',
    'buildbot': 'compareBuildbotOutputs'
}

_COMPARE_SAMPLES_CMD_LINE = os.path.join(
    os.path.dirname(__file__), 'compare_samples_cmdline.html')


def CompareSamples(sample_a, sample_b, metric, data_format='chartjson'):
  """Compare the values of a metric from two samples from benchmark output.

  Args:
    sample_a, sample_b (str): comma-separated lists of paths to the benchmark
        output.
    metric (str): Metric name in slash-separated format [2 or 3 part].
    data_format (str): The format the samples are in. Supported values are:
        'chartjson', 'valueset', 'buildbot'.
  Returns:
    JSON encoded dict with the values parsed form the samples and the result of
    the hypothesis testing comparison of the samples under the 'result' key.
    Possible values for the result key are:
      'NEED_MORE_DATA', 'REJECT' and 'FAIL_TO_REJECT'.
    Where the null hypothesis is that the samples belong to the same population.
    i.e. a 'REJECT' result would make it reasonable to conclude that
    there is a significant difference between the samples. (e.g. a perf
    regression).
  """

  method = FORMAT_TO_METHOD[data_format]
  project = tracing_project.TracingProject()
  all_source_paths = list(project.source_paths)

  def MakeAbsPaths(l):
    return ','.join(map(os.path.abspath, l.split(',')))

  return vinn.RunFile(
      _COMPARE_SAMPLES_CMD_LINE,
      source_paths=all_source_paths,
      js_args=[
          method,
          MakeAbsPaths(sample_a),
          MakeAbsPaths(sample_b),
          metric
      ])
