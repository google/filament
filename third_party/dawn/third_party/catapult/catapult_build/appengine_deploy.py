# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import print_function
import os
import subprocess
import sys

from catapult_build import temp_deployment_dir


def Deploy(paths, args, version=None):
  """Deploys a new version of an App Engine app from a temporary directory.

  Args:
    paths: List of paths to files and directories that should be linked
        (or copied) in the deployment directory.
    args: Arguments passed to "gcloud app deploy".
  """
  if version is None:
    version = _VersionName()
  with temp_deployment_dir.TempDeploymentDir(
      paths, use_symlinks=False) as temp_dir:
    print('Deploying from "%s".' % temp_dir)

    # google-cloud-sdk/bin/gcloud is a shell script, which we can't subprocess
    # on Windows with shell=False. So, execute the Python script directly.
    if os.name == 'nt':
      script_path = _FindScriptInPath('gcloud.cmd')
    else:
      script_path = _FindScriptInPath('gcloud')
    if not script_path:
      print('This script requires the Google Cloud SDK to be in PATH.')
      print('Install at https://cloud.google.com/sdk and then run')
      print('`gcloud components install app-engine-python`')
      sys.exit(1)

    subprocess.check_call([script_path, 'app', 'deploy', '--no-promote',
                           '--quiet', '--version', version] + args,
                          cwd=temp_dir)

def _FindScriptInPath(script_name):
  for path in os.environ['PATH'].split(os.pathsep):
    script_path = os.path.join(path, script_name)
    if os.path.exists(script_path):
      return script_path

  return None


def _VersionName():
  is_synced = not _Run(
      ['git', 'diff', 'origin/master', '--no-ext-diff']).strip()
  deployment_type = 'clean' if is_synced else 'dev'
  email = _Run(['git', 'config', '--get', 'user.email'])
  username = email[0:email.find('@')]
  commit_hash = _Run(['git', 'rev-parse', '--short=8', 'HEAD']).strip()
  return '%s-%s-%s' % (deployment_type, username, commit_hash)


def _Run(command):
  proc = subprocess.Popen(command, stdout=subprocess.PIPE)
  output, _ = proc.communicate()
  return output
