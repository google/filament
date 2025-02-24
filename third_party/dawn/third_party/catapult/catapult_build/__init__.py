# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys


def _AddToPathIfNeeded(path):
  if path not in sys.path:
    sys.path.insert(0, path)


def _UpdateSysPathIfNeeded():
  catapult_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
  catapult_third_party_path = os.path.abspath(os.path.join(
      catapult_path, 'third_party'))
  _AddToPathIfNeeded(os.path.join(catapult_path, 'common', 'py_utils'))
  _AddToPathIfNeeded(os.path.join(catapult_path, 'common', 'py_vulcanize'))
  _AddToPathIfNeeded(
      os.path.join(catapult_third_party_path, 'beautifulsoup4-4.9.3', 'py3k'))
  _AddToPathIfNeeded(
      os.path.join(catapult_third_party_path, 'html5lib-1.1'))
  _AddToPathIfNeeded(
      os.path.join(catapult_third_party_path, 'webencodings-0.5.1'))
  _AddToPathIfNeeded(os.path.join(catapult_third_party_path, 'six'))
  _AddToPathIfNeeded(os.path.join(catapult_third_party_path, 'webapp2'))
  _AddToPathIfNeeded(os.path.join(catapult_path, 'tracing'))
  _AddToPathIfNeeded(os.path.join(catapult_path, 'perf_insights'))
  _AddToPathIfNeeded(os.path.join(catapult_path, 'dashboard'))
  _AddToPathIfNeeded(os.path.join(catapult_path, 'netlog_viewer'))


_UpdateSysPathIfNeeded()
