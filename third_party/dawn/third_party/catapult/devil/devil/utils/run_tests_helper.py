# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Helper functions common to native, java and host-driven test runners."""

import collections
import logging

from devil.utils import logging_common

CustomFormatter = logging_common.CustomFormatter

_WrappedLoggingArgs = collections.namedtuple('_WrappedLoggingArgs',
                                             ['verbose', 'quiet'])


def SetLogLevel(verbose_count, add_handler=True):
  """Sets log level as |verbose_count|.

  Args:
    verbose_count: Verbosity level.
    add_handler: If true, adds a handler with |CustomFormatter|.
  """
  logging_common.InitializeLogging(
      _WrappedLoggingArgs(verbose_count, 0),
      handler=None if add_handler else logging.NullHandler())
