#!/usr/bin/python2.4
#
# Copyright 2008 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Run all tests from *_test.py files."""

import os
import unittest


def ModuleName(filename, base_dir):
  """Given a filename, convert to the python module name."""
  filename = filename.replace(base_dir, '')
  filename = filename.lstrip(os.path.sep)
  filename = filename.replace(os.path.sep, '.')
  if filename.endswith('.py'):
    filename = filename[:-3]
  return filename


def FindTestModules():
  """Return names of any test modules (*_test.py)."""
  tests = []
  start_dir = os.path.dirname(os.path.abspath(__file__))
  for dir, subdirs, files in os.walk(start_dir):
    if dir.endswith('/.svn') or '/.svn/' in dir:
      continue
    tests.extend(ModuleName(os.path.join(dir, f), start_dir) for f 
                 in files if f.endswith('_test.py'))
  return tests


def AllTests():
  suites = unittest.defaultTestLoader.loadTestsFromNames(FindTestModules())
  return unittest.TestSuite(suites)


if __name__ == '__main__':
  unittest.main(module=None, defaultTest='__main__.AllTests')
