# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging
import time


class WallTimeLogger:

  def __init__(self, label):
    """Initialize a context manager labeled `label` that measures the wall time
    between enter and exit.
    If description is truthy, the measurement is reported to ts mon.
    """
    self._label = label
    self._start = None
    self.seconds = 0

  def _Now(self):
    return time.time()

  def _Suffix(self):
    return 'wall'

  def __enter__(self):
    self._start = self._Now()

  def __exit__(self, *unused_args):
    self.seconds = self._Now() - self._start
    logging.info('%s:%s=%f', self._label, self._Suffix(), self.seconds)


class CpuTimeLogger(WallTimeLogger):

  def _Now(self):
    return time.process_time()

  def _Suffix(self):
    return 'cpu'


def TimeWall(label):

  def Decorator(wrapped):

    def Wrapper(*a, **kw):
      with WallTimeLogger(label):
        return wrapped(*a, **kw)

    return Wrapper

  return Decorator


def TimeCpu(label):

  def Decorator(wrapped):

    def Wrapper(*a, **kw):
      with CpuTimeLogger(label):
        return wrapped(*a, **kw)

    return Wrapper

  return Decorator
