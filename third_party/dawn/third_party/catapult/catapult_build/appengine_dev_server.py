#!/usr/bin/python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import print_function
import argparse
import os
import os.path
import subprocess
import sys

from catapult_build import temp_deployment_dir


def DevAppserver(paths, args, reuse_path=None):
  """Starts a dev server for an App Engine app.

  Args:
    paths: List of paths to files and directories that should be linked (or
      copied) in the deployment directory.
    args: List of additional arguments to pass to the dev server.
    reuse_path: If not None, will re-use an existing path (string) as the
      temporary directory from which the DevAppserver will be run from.
  """
  with temp_deployment_dir.TempDeploymentDir(
      paths, reuse_path=reuse_path) as temp_dir:
    print('Running dev server on "%s".' % temp_dir)

    script_path = _FindScriptInPath('dev_appserver.py')
    if not script_path:
      print('This script requires the App Engine SDK to be in PATH.')
      sys.exit(1)

    subprocess.call([sys.executable, script_path] +
                    _AddTempDirToYamlPathArgs(temp_dir, args))


def _FindScriptInPath(script_name):
  for path in os.environ['PATH'].split(os.pathsep):
    script_path = os.path.join(path, script_name)
    if os.path.exists(script_path):
      return script_path

  return None


def _AddTempDirToYamlPathArgs(temp_dir, args):
  """Join `temp_dir` to the positional args, preserving the other args."""
  parser = argparse.ArgumentParser()
  parser.add_argument('yaml_path', nargs='*')
  parser.add_argument('--run_pinpoint', default=False, action='store_true')
  options, remaining_args = parser.parse_known_args(args)
  yaml_path_args = [
      os.path.join(temp_dir, yaml_path) for yaml_path in options.yaml_path
  ]
  if not yaml_path_args:
    if options.run_pinpoint:
      temp_dir += '/pinpoint.yaml'
    yaml_path_args = [temp_dir]
  return yaml_path_args + remaining_args
