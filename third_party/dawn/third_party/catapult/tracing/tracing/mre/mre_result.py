# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from tracing.mre import failure as failure_module

class DuplicateKeyError(Exception):
  """Raised if an attempt is made to set a key more than once."""


class MreResult(object):

  def __init__(self, failures=None, pairs=None):
    if failures is None:
      failures = []
    if pairs is None:
      pairs = {}
    self._failures = failures
    self._pairs = pairs

  @property
  def failures(self):
    return self._failures

  @property
  def pairs(self):
    return self._pairs

  def AsDict(self):
    d = {
        'pairs': self._pairs
    }

    if self.failures:
      d['failures'] = [failure.AsDict() for failure in self._failures]

    return d

  def AddFailure(self, failure):
    if not isinstance(failure, failure_module.Failure):
      raise ValueError('Attempted to add %s as Failure' % failure)

    self._failures.append(failure)

  def AddPair(self, key, value):
    if key in self._pairs:
      raise DuplicateKeyError('Key ' + key + 'already exists in result.')
    self._pairs[key] = value
