# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import sys


_CATAPULT_DIR = os.path.abspath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)), '..', '..', '..'))

_PI_PATH = os.path.join(_CATAPULT_DIR, 'perf_insights')

if _PI_PATH not in sys.path:
  sys.path.insert(1, _PI_PATH)
