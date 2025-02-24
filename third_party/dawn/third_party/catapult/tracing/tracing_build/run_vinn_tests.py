#!/usr/bin/env python
# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
import sys

from hooks import install
import tracing_project
import vinn


def _RelPathToUnixPath(p):
  return p.replace(os.sep, '/')


def RunTests():
  project = tracing_project.TracingProject()
  headless_test_module_filenames = [
      '/' + _RelPathToUnixPath(x)
      for x in project.FindAllD8TestModuleRelPaths()]
  headless_test_module_filenames.sort()

  cmd = """
  HTMLImportsLoader.loadHTML('/tracing/base/headless_tests.html');
  tr.b.unittest.loadAndRunTests(sys.argv.slice(1));
  """
  res = vinn.RunJsString(
      cmd, source_paths=list(project.source_paths),
      js_args=headless_test_module_filenames,
      stdout=sys.stdout, stdin=sys.stdin)
  return res.returncode


def Main(argv):
  parser = argparse.ArgumentParser(
      description='Run d8 tests.')
  parser.add_argument(
      '--no-install-hooks', dest='install_hooks', action='store_false')
  parser.set_defaults(install_hooks=True)
  args = parser.parse_args(argv[1:])
  if args.install_hooks:
    install.InstallHooks()

  sys.exit(RunTests())
