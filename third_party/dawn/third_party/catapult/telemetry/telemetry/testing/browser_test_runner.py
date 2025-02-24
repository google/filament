# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import print_function
from __future__ import absolute_import
import os
import subprocess
import sys

from telemetry.core import util


def ProcessConfig(project_config, args=None):
  args = args or []
  assert '--top-level-dir' not in args, (
      'Top level directory for running tests should be specified through '
      'the instance of telemetry.project_config.ProjectConfig.')
  assert '--client-config' not in args, (
      'Client config file to be used for telemetry should be specified through '
      'the instance of telemetry.project_config.ProjectConfig.')
  assert project_config.top_level_dir, 'Must specify top level dir for project'
  args.extend(['--top-level-dir', project_config.top_level_dir])
  for c in project_config.client_configs:
    args.extend(['--client-config', c])
  for s in project_config.start_dirs:
    args.extend(['--start-dir', s])
  for e in project_config.expectations_files:
    args.extend(['--expectations-file', e])
  if project_config.default_chrome_root and '--chrome-root' not in args:
    args.extend(['--chrome-root', project_config.default_chrome_root])
  return args


def Run(project_config, args):
  args = ProcessConfig(project_config, args)
  env = os.environ.copy()
  telemetry_dir = util.GetTelemetryDir()
  if 'PYTHONPATH' in env:
    env['PYTHONPATH'] = os.pathsep.join([env['PYTHONPATH'], telemetry_dir])
  else:
    env['PYTHONPATH'] = telemetry_dir

  path_to_run_tests = os.path.join(os.path.abspath(os.path.dirname(__file__)),
                                   'run_browser_tests.py')

  exit_code = subprocess.call([sys.executable, path_to_run_tests] + args,
                              env=env)
  if exit_code:
    print('**Non zero exit code**')
    print('If you don\'t see any error stack, this could have been a '
          'native crash. Consider installing faulthandler '
          '(https://faulthandler.readthedocs.io/) for more useful error '
          'message')
  return exit_code
