# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Presubmit script for changes affecting web_page_replay_go/.

See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details about the presubmit API built into depot_tools.
"""

import os
import tempfile
import shutil

USE_PYTHON3 = True

def _RunArgs(args, input_api, cwd):
  p = input_api.subprocess.Popen(args, stdout=input_api.subprocess.PIPE,
                                 stderr=input_api.subprocess.STDOUT,
                                 cwd=cwd)
  out, _ = p.communicate()
  return (out, p.returncode)


def _CommonChecks(input_api, output_api):
  """Performs common checks."""
  results = []
  if input_api.subprocess.call(
          "go  version",
          shell=True,
          stdout=input_api.subprocess.PIPE,
          stderr=input_api.subprocess.PIPE) != 0:
    results.append(output_api.PresubmitPromptOrNotify(
        'go binary is not found. Make sure to run unit tests if you change any '
        'Go files.'))
    return results

  # Run go test ./webpagereplay
  cwd = os.path.join(input_api.PresubmitLocalPath(), 'src')
  cmd = ['go', 'test', './webpagereplay']
  out, return_code = _RunArgs(cmd, input_api, cwd)
  if return_code:
    results.append(output_api.PresubmitError(
        'webpagereplay tests failed.', long_text=out))
  print(out)

  return results


def CheckChangeOnUpload(input_api, output_api):
  report = []
  report.extend(_CommonChecks(input_api, output_api))
  return report


def CheckChangeOnCommit(input_api, output_api):
  report = []
  report.extend(_CommonChecks(input_api, output_api))
  return report
