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

import logging
import os

version = os.environ.get('CURRENT_VERSION_ID', '').split('.')[0]

if (__name__ == 'google.appengine.ext.mapreduce'
    and version != 'ah-builtin-python-bundle'):
  msg = ('You should not use the mapreduce library that is bundled with the'
         ' SDK. You can use the PyPi package at'
         ' https://pypi.python.org/pypi/GoogleAppEngineMapReduce or use the '
         'source at https://github.com/GoogleCloudPlatform/appengine-mapreduce '
         'instead.')
  logging.warn(msg)

