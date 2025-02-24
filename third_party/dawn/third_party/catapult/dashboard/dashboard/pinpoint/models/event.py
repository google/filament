# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Consolidated Event model for Task evaluation.

This module defines the canonical 'Event' type used by Pinpoint task evaluator
implementations.

For more about the Execution Engine and the task evaluators, see
dashboard.dashboard.pinpoint.models.task.Evaluate.
"""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections

Event = collections.namedtuple('Event', ('type', 'target_task', 'payload'))


def SelectEvent(target_task=None):
  return Event(type='select', target_task=target_task, payload={})
