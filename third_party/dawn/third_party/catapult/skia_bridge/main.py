# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Dispatches requests to request handler classes."""

from __future__ import absolute_import

import logging
import os
import sys
from pathlib import Path
import google.cloud.logging
google.cloud.logging.Client().setup_logging(log_level=logging.DEBUG)

try:
  import googleclouddebugger
  googleclouddebugger.enable(breakpoint_enable_canary=True)
except ImportError:
  pass

dashboard_path = Path(__file__).parent.parent.parent
if str(dashboard_path) not in sys.path:
  sys.path.insert(0, str(dashboard_path))

from application import app
def _AddToPathIfNeeded(path):
  if path not in sys.path:
    sys.path.insert(0, path)
#

APP = app.Create()

if __name__ == '__main__':
  # This is used when running locally only.
  os.environ['DISABLE_METRICS'] = 'True'
  APP.run(host='127.0.0.1', port=8080, debug=True)
