# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Dispatches requests to request handler classes."""

# from flask import Flask, request, make_response

import logging
import google.cloud.logging
google.cloud.logging.Client().setup_logging(log_level=logging.DEBUG)

try:
  import googleclouddebugger
  googleclouddebugger.enable(breakpoint_enable_canary=True)
except ImportError:
  pass

from application import app


APP = app.create_app()

if __name__ == '__main__':
  # This is used when running locally only.
  APP.run(host='127.0.0.1', port=8080, debug=True)
