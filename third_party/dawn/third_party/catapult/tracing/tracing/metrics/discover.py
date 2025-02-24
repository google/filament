# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import os

import tracing_project
import vinn


_DISCOVER_CMD_LINE = os.path.join(
    os.path.dirname(__file__), 'discover_cmdline.html')


def DiscoverMetrics(modules_to_load):
  """ Returns a list of registered metrics.

  Args:
    modules_to_load: a list of modules (string) to be loaded before discovering
      the registered metrics.
  """
  assert isinstance(modules_to_load, list)
  project = tracing_project.TracingProject()
  all_source_paths = list(project.source_paths)

  res = vinn.RunFile(
      _DISCOVER_CMD_LINE, source_paths=all_source_paths,
      js_args=modules_to_load)

  if res.returncode != 0:
    raise RuntimeError('Error running metrics_discover_cmdline: ' + res.stdout)
  return [str(m) for m in json.loads(res.stdout)]
