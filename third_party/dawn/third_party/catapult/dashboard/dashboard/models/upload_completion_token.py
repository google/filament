# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""The datastore models for upload tokens and related data."""
from __future__ import absolute_import

import datetime
import logging
import uuid

from google.appengine.ext import ndb

from dashboard.models import internal_only_model
from dateutil.relativedelta import relativedelta

# 10 minutes should be enough for keeping the data in memory because processing
# histograms takes 3.5 minutes in the 90th percentile.
_MEMCACHE_TIMEOUT = 60 * 10


class State:
  PENDING = 0
  PROCESSING = 1
  FAILED = 2
  COMPLETED = 3


def StateToString(state):
  if state == State.PENDING:
    return 'PENDING'
  if state == State.PROCESSING:
    return 'PROCESSING'
  if state == State.FAILED:
    return 'FAILED'
  if state == State.COMPLETED:
    return 'COMPLETED'
  return None


class Token(internal_only_model.InternalOnlyModel):
  """Token is used to get state of request.

  Token can contain multiple Measurement. One per each histogram in the
  request. States of nested Measurements affect state of the Token.

  Even though Token and Measurements contain related data we do not combine
  them into one entity group. Token can contain 1000+ measurements. So doing
  such amount of updates of one entity group is too expencive.
  """
  _use_memcache = True
  _memcache_timeout = _MEMCACHE_TIMEOUT

  internal_only = ndb.BooleanProperty(default=True, indexed=False)

  state_ = ndb.IntegerProperty(
      name='state', default=State.PENDING, indexed=False)

  error_message = ndb.StringProperty(indexed=False, default=None)

  creation_time = ndb.DateTimeProperty(auto_now_add=True, indexed=False)

  update_time = ndb.DateTimeProperty(auto_now=True, indexed=False)

  temporary_staging_file_path = ndb.StringProperty(indexed=False, default=None)

  @property
  def state(self):
    measurements = self.GetMeasurements()
    if not measurements:
      return self.state_

    all_states = [child.state for child in measurements if child is not None]
    all_states.append(self.state_)
    if all(s == State.PENDING for s in all_states):
      return State.PENDING
    if any(s == State.FAILED for s in all_states):
      return State.FAILED
    if any(s in (State.PROCESSING, State.PENDING) for s in all_states):
      return State.PROCESSING
    return State.COMPLETED

  @classmethod
  def UpdateObjectState(cls, obj, state, error_message=None):
    if obj is None:
      return None
    return obj.UpdateState(state, error_message)

  def UpdateState(self, state, error_message=None):
    assert error_message is None or state == State.FAILED

    self.state_ = state
    if error_message is not None:
      # In some cases the error_message (str(e) field) can actually be not
      # a string.
      self.error_message = str(error_message)
    self.put()

    # Note that state here does not reflect the state of upload overall (since
    # "state_" doesn't take measurements into account). Token and Measurements
    # aren't connected by entity group, so the information about final state
    # would be stale.
    logging.info('Upload completion token updated. Token id: %s, state: %s',
                 self.key.id(), StateToString(self.state_))

  @ndb.tasklet
  def AddMeasurement(self, test_path, is_monitored):
    """Creates measurement, associated to the current token."""

    measurement = Measurement(
        id=str(uuid.uuid4()),
        test_path=test_path,
        token=self.key,
        monitored=is_monitored)
    yield measurement.put_async()

    logging.info(
        'Upload completion token measurement created. Token id: %s, '
        'measurement test path: %r', self.key.id(), measurement.test_path)
    raise ndb.Return(measurement)

  def GetMeasurements(self):
    return Measurement.query(Measurement.token == self.key).fetch()


class Measurement(internal_only_model.InternalOnlyModel):
  """Measurement represents state of added histogram.

  Measurement is uniquely defined by the full path to the test (for example
  master/bot/test/metric/page) and parent token key.
  """
  _use_memcache = True
  _memcache_timeout = _MEMCACHE_TIMEOUT

  internal_only = ndb.BooleanProperty(default=True)

  token = ndb.KeyProperty(kind='Token', indexed=True)

  test_path = ndb.StringProperty(indexed=True)

  state = ndb.IntegerProperty(default=State.PROCESSING, indexed=False)

  error_message = ndb.StringProperty(indexed=False, default=None)

  update_time = ndb.DateTimeProperty(auto_now=True, indexed=False)

  monitored = ndb.BooleanProperty(default=False, indexed=False)

  histogram = ndb.KeyProperty(kind='Histogram', indexed=True, default=None)

  @ndb.ComputedProperty
  def expiry(self):  # pylint: disable=invalid-name
    if self.update_time:
      return self.update_time + relativedelta(years=3)

    return datetime.datetime.utcnow() + relativedelta(years=3)

  @classmethod
  def GetByPath(cls, test_path, token_id):
    if test_path is None or token_id is None:
      return None
    # Data here can be a bit stale here.
    return Measurement.query(
        ndb.AND(Measurement.test_path == test_path,
                Measurement.token == ndb.Key('Token', token_id))).get()

  @classmethod
  @ndb.tasklet
  def UpdateStateByPathAsync(cls,
                             test_path,
                             token_id,
                             state,
                             error_message=None):
    assert error_message is None or state == State.FAILED

    obj = cls.GetByPath(test_path, token_id)
    if obj is None:
      if test_path is not None and token_id is not None:
        logging.warning(
            'Upload completion token measurement could not be found. '
            'Token id: %s, measurement test path: %s', token_id, test_path)
      return
    obj.state = state
    if error_message is not None:
      # In some cases the error_message (str(e) field) can actually be not
      # a string.
      obj.error_message = str(error_message)
    yield obj.put_async()
    logging.info(
        'Upload completion token measurement updated. Token id: %s, '
        'measurement test path: %s, state: %s', token_id, test_path,
        StateToString(state))
