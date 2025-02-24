# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import vinn
import tracing_project

def HistogramsToCsv(json_path):
  """Convert HistogramSet JSON to CSV.

  Args:
    json_path: path to a file containing HistogramSet JSON

  Returns:
    a Vinn result object whose 'returncode' indicates whether there was an
    exception, and whose 'stdout' contains CSV.
  """
  return vinn.RunFile(
      os.path.join(os.path.dirname(__file__), 'histograms_to_csv_cmdline.html'),
      source_paths=list(tracing_project.TracingProject().source_paths),
      js_args=[os.path.abspath(json_path)])
