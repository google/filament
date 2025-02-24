# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Sheriff Config Service

This service implements the requirements for supporting sheriff configuration
file validation.
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

try:
  import googleclouddebugger
  googleclouddebugger.enable()
except ImportError:
  pass

import logging
import google.cloud.logging

google.cloud.logging.Client().setup_logging(log_level=logging.DEBUG)

import service

APP = service.CreateApp()
