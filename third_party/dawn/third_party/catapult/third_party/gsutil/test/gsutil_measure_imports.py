# -*- coding: utf-8 -*-
# Copyright 2016 Google Inc. All Rights Reserved.
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
"""Custom importer to handle module load times. Wraps gsutil."""

from __future__ import absolute_import

import six
if six.PY3:
  import builtins as builtin
else:
  import __builtin__ as builtin

import atexit
from collections import OrderedDict
from operator import itemgetter
import os
import sys
import timeit

# This file will function in an identical manner to 'gsutil'. Instead of calling
# 'gsutil some_command' run 'gsutil_measure_imports some_command' from this test
# directory. When used, this file will change the MEASURING_TIME_ACTIVE variable
# in the gsutil.py file to True, signaling that the we are measuring import
# times. In all other cases, this variable will be set to False. This behavior
# will allow gsutil developers to analyze which modules are taking the most time
# to initialize. This is especially important because not all of those modules
# will be used. Therefore it is important to speed up the ones which are not
# used and take a significant amount of time to initialize (e.g. 100ms).
INITIALIZATION_TIMES = {}

real_importer = builtin.__import__


def get_sorted_initialization_times(items=10):
  """Returns a sorted OrderedDict.

  The keys are module names and the values are the corresponding times taken to
  import.

  Args:
    items: The number of items to return in the list.

  Returns:
    An OrderedDict object, sorting initialization times in increasing order.
  """
  return OrderedDict(
      sorted(INITIALIZATION_TIMES.items(), key=itemgetter(1),
             reverse=True)[:items])


def print_sorted_initialization_times():
  """Prints the most expensive imports in descending order."""
  print('\n***Most expensive imports***')
  for item in get_sorted_initialization_times().iteritems():
    print(item)


def timed_importer(name, *args, **kwargs):
  """Wrapper for the default Python import function.

  Args:
    name: The name of the module.
    *args: A list of arguments passed to import.
    **kwargs: A dictionary of arguments to pass to import.

  Returns:
    The value provided by the default import function.
  """
  # TODO: Build an import tree to better understand which areas need more
  # attention.
  import_start_time = timeit.default_timer()
  import_value = real_importer(name, *args, **kwargs)
  import_end_time = timeit.default_timer()
  INITIALIZATION_TIMES[name] = import_end_time - import_start_time
  return import_value


builtin.__import__ = timed_importer


def initialize():
  """Initializes gsutil."""
  sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))
  import gsutil  # pylint: disable=g-import-not-at-top
  atexit.register(print_sorted_initialization_times)
  gsutil.MEASURING_TIME_ACTIVE = True
  gsutil.RunMain()
