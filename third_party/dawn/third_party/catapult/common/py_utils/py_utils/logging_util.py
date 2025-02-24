# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Logging util functions.

It would be named logging, but other modules in this directory use the default
logging module, so that would break them.
"""

from __future__ import absolute_import
import contextlib
import logging

@contextlib.contextmanager
def CaptureLogs(file_stream):
  if not file_stream:
    # No file stream given, just don't capture logs.
    yield
    return

  fh = logging.StreamHandler(file_stream)

  logger = logging.getLogger()
  # Try to copy the current log format, if one is set.
  if logger.handlers and hasattr(logger.handlers[0], 'formatter'):
    fh.formatter = logger.handlers[0].formatter
  else:
    fh.setFormatter(logging.Formatter(
        '(%(levelname)s) %(asctime)s %(message)s'))
  logger.addHandler(fh)

  try:
    yield
  finally:
    logger = logging.getLogger()
    logger.removeHandler(fh)
