#!/usr/bin/env vpython3
# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import print_function
import argparse
import os
import subprocess
import sys

FAIL_EMOJI = u'\U0001F631'.encode('utf-8')
PASS_EMOJI = u'\U0001F601'.encode('utf-8')

GREEN = '\033[92m'
RED = '\033[91m'
END_CODE = '\033[0m'


def _Color(s, color):
  """Adds ANSI escape codes to color a string printed to the terminal."""
  return color + s + END_CODE


def _RunTest(test, chrome_command):
  if sys.platform in test.get('disabled_platforms', {}):
    return 0
  command = [test['path']]
  if sys.platform == 'win32':
    command = ['python3'] + command
  if test.get('chrome_path_arg') and chrome_command:
    command += ['--chrome_path', chrome_command]
  try:
    return subprocess.call(command)
  except OSError:
    return 1


def Main(name, tests, argv):
  parser = argparse.ArgumentParser(
      description='Run all tests of %s project.' % name)
  parser.add_argument(
      '--chrome_path', type=str,
      help='Path to Chrome browser binary for dev_server tests.')
  args = parser.parse_args(argv[1:])

  exit_code = 0
  errors = []
  for test in tests:
    new_exit_code = _RunTest(test, args.chrome_path)
    if new_exit_code != 0:
      exit_code |= new_exit_code
      errors += '%s failed some tests. Re-run %s script to see those.\n' % (
          os.path.basename(test['path']), test['path'])

  if exit_code:
    print(_Color('Oops! Some tests failed.', RED), FAIL_EMOJI)
    sys.stderr.writelines(errors)
  else:
    print(_Color('Woho! All tests passed.', GREEN), PASS_EMOJI)

  sys.exit(exit_code)
