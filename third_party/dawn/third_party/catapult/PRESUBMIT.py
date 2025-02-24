# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Top-level presubmit script for catapult.

See https://www.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details about the presubmit API built into depot_tools.
"""

import re
import sys

USE_PYTHON3 = True

_EXCLUDED_PATHS = (
    r'(.*[\\/])?\.git[\\/].*',
    r'.+\.png$',
    r'.+\.svg$',
    r'.+\.skp$',
    r'.+\.gypi$',
    r'.+\.gyp$',
    r'.+\.gn$',
    r'.*\.gitignore$',
    r'.*codereview.settings$',
    r'.*AUTHOR$',
    r'^CONTRIBUTORS\.md$',
    r'.*LICENSE$',
    r'.*OWNERS$',
    r'.*README\.md$',
    r'^dashboard[\\/]dashboard[\\/]api[\\/]examples[\\/].*.js',
    r'^dashboard[\\/]dashboard[\\/]templates[\\/].*',
    r'^dashboard[\\/]dashboard[\\/]sheriff_config[\\/].*_pb2.py$',
    r'^dashboard[\\/]sandwich_verification[\\/]cabe/proto/v1[\\/].*_pb2.py$',
    r'^dashboard[\\/]sandwich_verification[\\/]cabe/proto/v1[\\/].*_pb2_grpc.py$', # pylint: disable=line-too-long
    r'^experimental[\\/]heatmap[\\/].*',
    r'^experimental[\\/]trace_on_tap[\\/]third_party[\\/].*',
    r'^experimental[\\/]perf_sheriffing_emailer[\\/].*.js',
    r'^perf_insights[\\/]test_data[\\/].*',
    r'^perf_insights[\\/]third_party[\\/].*',
    r'^telemetry[\\/]third_party[\\/].*',
    r'.*third_party[\\/].*',
    r'^tracing[\\/]\.allow-devtools-save$',
    r'^tracing[\\/]bower\.json$',
    r'^tracing[\\/]\.bowerrc$',
    r'^tracing[\\/]tracing_examples[\\/]string_convert\.js$',
    r'^tracing[\\/]test_data[\\/].*',
    r'^tracing[\\/]third_party[\\/].*',
    r'^py_vulcanize[\\/]third_party[\\/].*',
    r'^common/py_vulcanize[\\/].*', # TODO(hjd): Remove after fixing long lines.
)


_GITHUB_BUG_ID_RE = re.compile(r'#[1-9]\d*')
_NUMERAL_BUG_ID_RE = re.compile(r'[1-9]\d*')
_MONORAIL_PROJECT_NAMES = frozenset(
    {'chromium', 'v8', 'angleproject', 'skia', 'dawn'})

def CheckChangeLogBug(input_api, output_api):
  # Show a presubmit message if there is no Bug line or an empty Bug line.
  if not input_api.change.BugsFromDescription():
    return [output_api.PresubmitNotifyResult(
        'If this change has associated bugs on GitHub, Issuetracker or '
        'Monorail, add a "Bug: <bug>(, <bug>)*" line to the patch description '
        'where <bug> can be one of the following: catapult:#NNNN, b:NNNNNN, ' +
        ', '.join('%s:NNNNNN' % n for n in _MONORAIL_PROJECT_NAMES) + '.')]

  # Check that each bug in the BUG= line has the correct format.
  error_messages = []
  catapult_bug_provided = False

  for index, bug in enumerate(input_api.change.BugsFromDescription()):
    # Check if the bug can be split into a repository name and a bug ID (e.g.
    # 'catapult:#1234' -> 'catapult' and '#1234').
    bug_parts = bug.split(':')
    if len(bug_parts) != 2:
      error_messages.append('Invalid bug "%s". Bugs should be provided in the '
                            '"<project-name>:<bug-id>" format.' % bug)
      continue
    project_name, bug_id = bug_parts

    if project_name == 'catapult':
      if not _GITHUB_BUG_ID_RE.match(bug_id):
        error_messages.append('Invalid bug "%s". Bugs in the Catapult '
                              'repository should be provided in the '
                              '"catapult:#NNNN" format.' % bug)
      catapult_bug_provided = True
    elif project_name == 'b':
      if not _NUMERAL_BUG_ID_RE.match(bug_id):
        error_messages.append('Invalid bug "%s". Bugs in the Issuetracker '
                              'should be provided in the '
                              '"b:NNNNNN" format.' % bug)
    elif project_name in _MONORAIL_PROJECT_NAMES:
      if not _NUMERAL_BUG_ID_RE.match(bug_id):
        error_messages.append('Invalid bug "%s". Bugs in the Monorail %s '
                              'project should be provided in the '
                              '"%s:NNNNNN" format.' % (bug, project_name,
                                                       project_name))
    else:
      error_messages.append('Invalid bug "%s". Unknown repository "%s".' % (
          bug, project_name))

  return map(output_api.PresubmitError, error_messages)


def CheckChange(input_api, output_api):
  results = []
  try:
    sys.path += [input_api.PresubmitLocalPath()]

    from catapult_build import bin_checks
    from catapult_build import html_checks
    from catapult_build import js_checks
    from catapult_build import repo_checks

    results += input_api.canned_checks.PanProjectChecks(
        input_api, output_api, excluded_paths=_EXCLUDED_PATHS)
    results += input_api.RunTests(
        input_api.canned_checks.CheckVPythonSpec(input_api, output_api))
    results += CheckChangeLogBug(input_api, output_api)
    results += js_checks.RunChecks(
        input_api, output_api, excluded_paths=_EXCLUDED_PATHS)
    results += input_api.RunTests(
      input_api.canned_checks.CheckPatchFormatted(input_api, output_api,
        check_js=True))
    results += html_checks.RunChecks(
        input_api, output_api, excluded_paths=_EXCLUDED_PATHS)
    results += repo_checks.RunChecks(input_api, output_api)
    results += bin_checks.RunChecks(
        input_api, output_api, excluded_paths=_EXCLUDED_PATHS)
  finally:
    sys.path.remove(input_api.PresubmitLocalPath())
  return results


def CheckChangeOnUpload(input_api, output_api):
  results = CheckChange(input_api, output_api)
  cwd = input_api.PresubmitLocalPath()
  exit_code = input_api.subprocess.call(
      [input_api.python3_executable, 'generate_telemetry_build.py', '--check'],
      cwd=cwd)
  if exit_code != 0:
    results.append(output_api.PresubmitError(
        'BUILD.gn needs to be re-generated. Please run '
        '%s/generate_telemetry_build.py and include the changes in this CL' %
        cwd))
  return results

def CheckChangeOnCommit(input_api, output_api):
  return CheckChange(input_api, output_api)
