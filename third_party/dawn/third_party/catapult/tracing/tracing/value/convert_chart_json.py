# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import tracing_project
import vinn


def ConvertChartJson(chart_json):
  """Convert chart_json to Histograms.

  Args:
    chart_json: path to a file containing chart-json

  Returns:
    a Vinn result object whose 'returncode' indicates whether there was an
    exception, and whose 'stdout' contains HistogramSet json.
  """
  return vinn.RunFile(
      os.path.join(os.path.dirname(__file__),
                   'convert_chart_json_cmdline.html'),
      source_paths=tracing_project.TracingProject().source_paths,
      js_args=[os.path.abspath(chart_json)])
