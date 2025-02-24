# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import sys
import six

from telemetry.internal.actions import page_action

import py_utils

class WaitForElementAction(page_action.ElementPageAction):

  def RunAction(self, tab):
    code = 'function(element) { return element != null; }'
    try:
      self.EvaluateCallback(tab, code, wait=True,
                            timeout_in_seconds=self.timeout)
    except py_utils.TimeoutException as e:
      # Rethrow with the original stack trace for better debugging.
      six.reraise(
          py_utils.TimeoutException,
          py_utils.TimeoutException(
              'Timeout while waiting for element.\n' + repr(e)),
          sys.exc_info()[2]
      )
