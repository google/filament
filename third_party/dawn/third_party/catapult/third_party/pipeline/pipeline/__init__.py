#!/usr/bin/env python
#
# Copyright 2010 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


# pylint: disable=g-import-not-at-top
# pylint: disable=g-bad-name


def _fix_path():
  """Finds the google_appengine directory and fixes Python imports to use it."""
  import os
  import sys
  all_paths = os.environ.get('PYTHONPATH').split(os.pathsep)
  for path_dir in all_paths:
    dev_appserver_path = os.path.join(path_dir, 'dev_appserver.py')
    if os.path.exists(dev_appserver_path):
      logging.debug('Found appengine SDK on path!')
      google_appengine = os.path.dirname(os.path.realpath(dev_appserver_path))
      sys.path.append(google_appengine)
      # Use the next import will fix up sys.path even further to bring in
      # any dependent lib directories that the SDK needs.
      dev_appserver = __import__('dev_appserver')
      sys.path.extend(dev_appserver.EXTRA_PATHS)
      return


try:
  from pipeline import *
except ImportError, e:
  import logging
  logging.warning(
      'Could not load Pipeline API. Will fix path for testing. %s: %s',
      e.__class__.__name__, str(e))
  _fix_path()
  del logging
  from pipeline import *
