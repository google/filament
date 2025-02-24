#!/usr/bin/env python3
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""This script ensures that a given directory is an initialized git repo."""

import argparse
import os
import subprocess
import sys

# Import "git_common" from "depot_tools" root.
DEPOT_TOOLS_ROOT = os.path.abspath(os.path.join(
    os.path.dirname(__file__), os.pardir, os.pardir, os.pardir, os.pardir))
sys.path.insert(0, DEPOT_TOOLS_ROOT)
import git_common


def run_git(*cmd, **kwargs):
  kwargs['stdout'] = sys.stdout
  kwargs['stderr'] = sys.stderr
  git_common.run(*cmd, **kwargs)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--path', help='Path to prospective git repo.',
                      required=True)
  parser.add_argument('--url', help='URL of remote to make origin.',
                      required=True)
  parser.add_argument('--remote', help='Name of the git remote.',
                      default='origin')
  opts = parser.parse_args()

  path = opts.path
  remote = opts.remote
  url = opts.url

  if not os.path.exists(path):
    os.makedirs(path)

  if os.path.exists(os.path.join(path, '.git')):
    run_git('config', '--remove-section', 'remote.%s' % remote, cwd=path)
  else:
    run_git('init', cwd=path)
  run_git('remote', 'add', remote, url, cwd=path)
  return 0


if __name__ == '__main__':
  sys.exit(main())
