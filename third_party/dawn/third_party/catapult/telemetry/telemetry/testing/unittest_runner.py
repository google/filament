# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import absolute_import
import os
import subprocess
import sys

from telemetry.core import util


def ProcessConfig(project_config, args=None, no_browser=False,
                  disable_cloud_storage_io_during_test=False):
  args = args or []
  assert '--top-level-dir' not in args, (
      'Top level directory for running tests should be specified through '
      'the instance of telemetry.project_config.ProjectConfig.')
  assert '--client-config' not in args, (
      'Client config file to be used for telemetry should be specified through '
      'the instance of telemetry.project_config.ProjectConfig.')
  assert project_config.top_level_dir, 'Must specify top level dir for project'
  args.extend(['--top-level-dirs', project_config.top_level_dir])
  for c in project_config.client_configs:
    args.extend(['--client-config', c])
  for e in project_config.expectations_files:
    args.extend(['--expectations-file', e])
  if no_browser and '--no-browser' not in args:
    args.extend(['--no-browser'])

  if project_config.default_chrome_root and '--chrome-root' not in args:
    args.extend(['--chrome-root', project_config.default_chrome_root])

  if disable_cloud_storage_io_during_test:
    args.extend(['--disable-cloud-storage-io'])
  return args


def Run(project_config, no_browser=False,
        disable_cloud_storage_io_during_test=False, passed_args=None):
  args = ProcessConfig(project_config, passed_args or sys.argv[1:], no_browser,
                       disable_cloud_storage_io_during_test)
  env = os.environ.copy()
  telemetry_dir = util.GetTelemetryDir()
  if 'PYTHONPATH' in env:
    env['PYTHONPATH'] = os.pathsep.join([env['PYTHONPATH'], telemetry_dir])
  else:
    env['PYTHONPATH'] = telemetry_dir

  path_to_run_tests = os.path.join(os.path.abspath(os.path.dirname(__file__)),
                                   'run_tests.py')
  exit_code = subprocess.call([sys.executable, path_to_run_tests] + args,
                              env=env)
  if exit_code:
    print('**Non zero exit code**')
    print ('If you don\'t see any error stack, this could have been a '
           'native crash. Consider installing faulthandler '
           '(https://faulthandler.readthedocs.io/) for more useful error '
           'message')
  return exit_code
