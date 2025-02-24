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

"""Counters-related operations."""



__all__ = ['Increment']

# Deprecated. Use map_job_context.SliceContext.count instead.

from mapreduce.operation import base

# pylint: disable=protected-access


class Increment(base.Operation):
  """Increment counter operation."""

  def __init__(self, counter_name, delta=1):
    """Constructor.

    Args:
      counter_name: name of the counter as string
      delta: increment delta as int.
    """
    self.counter_name = counter_name
    self.delta = delta

  def __call__(self, context):
    """Execute operation.

    Args:
      context: mapreduce context as context.Context.
    """
    context._counters.increment(self.counter_name, self.delta)
