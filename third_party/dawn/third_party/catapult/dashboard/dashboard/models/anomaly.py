# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""The database model for an "Anomaly", which represents a step up or down."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
import logging
import sys
import time

from google.appengine.ext import ndb

from dashboard.common import timing
from dashboard.common import utils
from dashboard.common import datastore_hooks
from dashboard.models import internal_only_model
from dashboard.models import subscription

from dateutil.relativedelta import relativedelta

# A string to describe the magnitude of a change from zero to non-zero.
FREAKIN_HUGE = 'zero-to-nonzero'

# Possible improvement directions for a change. An Anomaly will always have a
# direction of UP or DOWN, but a test's improvement direction can be UNKNOWN.
UP, DOWN, UNKNOWN = (0, 1, 4)


class Issue(ndb.Model):
  project_id = ndb.StringProperty(default='chromium', indexed=True)
  issue_id = ndb.IntegerProperty(required=True, indexed=True)


class Anomaly(internal_only_model.InternalOnlyModel):
  """Represents a change-point or step found in the data series for a test.

  An Anomaly can be an upward or downward change, and can represent an
  improvement or a regression.
  """
  # Whether the alert should only be viewable by internal users.
  internal_only = ndb.BooleanProperty(indexed=True, default=False)

  # The time the alert fired.
  timestamp = ndb.DateTimeProperty(indexed=True, auto_now_add=True)

  @ndb.ComputedProperty
  def expiry(self):  # pylint: disable=invalid-name
    if self.timestamp:
      return self.timestamp + relativedelta(years=3)

    return datetime.datetime.utcnow() + relativedelta(years=3)

  # TODO(dberris): Remove these after migrating all issues to use the issues
  # repeated field, to allow an anomaly to be represented in multiple issues on
  # different Monorail projects.
  # === DEPRECATED START ===
  # Note: -1 denotes an invalid alert and -2 an ignored alert.
  # By default, this is None, which denotes a non-triaged alert.
  bug_id = ndb.IntegerProperty(indexed=True)

  # This is the project to which an anomaly is associated with, in the issue
  # tracker service.
  project_id = ndb.StringProperty(indexed=True, default='chromium')
  # === DEPRECATED END   ===

  # AlertGroups used for grouping
  groups = ndb.KeyProperty(indexed=True, repeated=True)

  # This is the list of issues associated with the anomaly. We're doing this to
  # allow a single anomaly to be represented in multiple issues in different
  # issue trackers.
  issues = ndb.StructuredProperty(Issue, indexed=True, repeated=True)

  # This field aims to replace the 'bug_id' field serving as a state indicator.
  state = ndb.StringProperty(
      default='untriaged',
      choices=['untriaged', 'triaged', 'ignored', 'invalid'])

  # The subscribers who recieve alerts
  subscriptions = ndb.LocalStructuredProperty(
      subscription.Subscription, repeated=True)
  subscription_names = ndb.StringProperty(indexed=True, repeated=True)

  # The anomaly configuration used to generate this anomaly, associated with the
  # subscription.
  matching_subscription = ndb.LocalStructuredProperty(subscription.Subscription)
  anomaly_config = ndb.JsonProperty()

  # Each Alert is related to one Test.
  test = ndb.KeyProperty(indexed=True)
  statistic = ndb.StringProperty(indexed=True)

  # We'd like to be able to query Alerts by Master, Bot, and Benchmark names.
  master_name = ndb.ComputedProperty(
      lambda self: utils.TestPath(self.test).split('/')[0], indexed=True)
  # This is not the "bot_name" as in "the id of the physical hardware device in the perf lab"
  # but rather the "bot_name" as in "the name of device configuration" e.g. "win-10-perf"
  bot_name = ndb.ComputedProperty(
      lambda self: utils.TestPath(self.test).split('/')[1], indexed=True)
  benchmark_name = ndb.ComputedProperty(
      lambda self: utils.TestPath(self.test).split('/')[2], indexed=True)

  # Each Alert has a revision range it's associated with; however,
  # start_revision and end_revision could be the same.
  start_revision = ndb.IntegerProperty(indexed=True)
  end_revision = ndb.IntegerProperty(indexed=True)

  # The revisions to use for display, if different than point id.
  display_start = ndb.IntegerProperty(indexed=False)
  display_end = ndb.IntegerProperty(indexed=False)

  # Ownership data, mapping e-mails to the benchmark's owners' emails and
  # component as the benchmark's Monorail component
  ownership = ndb.JsonProperty()

  # Alert grouping is used to overide the default alert group (test suite)
  # for auto-triage.
  alert_grouping = ndb.StringProperty(indexed=False, repeated=True)

  # The number of points before and after this anomaly that were looked at
  # when finding this anomaly.
  segment_size_before = ndb.IntegerProperty(indexed=False)
  segment_size_after = ndb.IntegerProperty(indexed=False)

  # The medians of the segments before and after the anomaly.
  median_before_anomaly = ndb.FloatProperty(indexed=False)
  median_after_anomaly = ndb.FloatProperty(indexed=False)

  # The standard deviation of the segments before the anomaly.
  std_dev_before_anomaly = ndb.FloatProperty(indexed=False)

  # The number of points that were used in the before/after segments.
  # This is also  returned by FindAnomalies
  window_end_revision = ndb.IntegerProperty(indexed=False)

  # In order to estimate how likely it is that this anomaly is due to noise,
  # t-test may be performed on the points before and after. The t-statistic,
  # degrees of freedom, and p-value are potentially-useful intermediary results.
  t_statistic = ndb.FloatProperty(indexed=False)
  degrees_of_freedom = ndb.FloatProperty(indexed=False)
  p_value = ndb.FloatProperty(indexed=False)

  # Whether this anomaly represents an improvement; if false, this anomaly is
  # considered to be a regression.
  is_improvement = ndb.BooleanProperty(indexed=True, default=False)

  # Whether this anomaly recovered (i.e. if this is a step down, whether there
  # is a corresponding step up later on, or vice versa.)
  recovered = ndb.BooleanProperty(indexed=True, default=False)

  # If the TestMetadata alerted upon has a ref build, store the ref build.
  ref_test = ndb.KeyProperty(indexed=False)

  # The corresponding units from the TestMetaData entity.
  units = ndb.StringProperty(indexed=False)

  recipe_bisects = ndb.KeyProperty(repeated=True, indexed=False)
  pinpoint_bisects = ndb.StringProperty(repeated=True, indexed=False)

  # Additional Metadata
  # ====
  #
  # Timestamps for the earliest and latest Row we used to determine whether this
  # is an anomaly. We use this to compute time-to-detection.
  earliest_input_timestamp = ndb.DateTimeProperty()
  latest_input_timestamp = ndb.DateTimeProperty()

  # Source generating this anomaly entity (eg: chromeperf, skia)
  source = ndb.StringProperty(indexed=False)

  @property
  def percent_changed(self):
    """The percent change from before the anomaly to after."""
    if self.median_before_anomaly == 0.0:
      return sys.float_info.max
    difference = self.median_after_anomaly - self.median_before_anomaly
    return 100 * difference / self.median_before_anomaly

  @property
  def absolute_delta(self):
    """The absolute change from before the anomaly to after."""
    return self.median_after_anomaly - self.median_before_anomaly

  @property
  def direction(self):
    """Whether the change is numerically an increase or decrease."""
    if self.median_before_anomaly < self.median_after_anomaly:
      return UP
    return DOWN

  def GetDisplayPercentChanged(self):
    """Gets a string showing the percent change."""
    if abs(self.percent_changed) == sys.float_info.max:
      return FREAKIN_HUGE
    return '%.1f%%' % abs(self.percent_changed)

  def GetDisplayAbsoluteChanged(self):
    """Gets a string showing the absolute change."""
    if abs(self.absolute_delta) == sys.float_info.max:
      return FREAKIN_HUGE
    return '%f' % abs(self.absolute_delta)

  def GetRefTestPath(self):
    if not self.ref_test:
      return None
    return utils.TestPath(self.ref_test)

  def SetIsImprovement(self, test=None):
    """Sets whether the alert is an improvement for the given test."""
    if not test:
      test = self.GetTestMetadataKey().get()
    # |self.direction| is never equal to |UNKNOWN| (see the definition above)
    # so when the test improvement direction is |UNKNOWN|, |self.is_improvement|
    # will be False.
    self.is_improvement = (self.direction == test.improvement_direction)

  def GetTestMetadataKey(self):
    """Get the key for the TestMetadata entity of this alert.

    We are in the process of converting from Test entities to TestMetadata.
    Until this is done, it's possible that an alert may store either Test
    or TestMetadata in the 'test' KeyProperty. This gets the TestMetadata key
    regardless of what's stored.
    """
    return utils.TestMetadataKey(self.test)

  @classmethod
  @ndb.tasklet
  def QueryAsync(cls,
                 bot_name=None,
                 bug_id=None,
                 count_limit=0,
                 deadline_seconds=50,
                 inequality_property=None,
                 is_improvement=None,
                 key=None,
                 keys_only=False,
                 limit=100,
                 master_name=None,
                 max_end_revision=None,
                 max_start_revision=None,
                 max_timestamp=None,
                 min_end_revision=None,
                 min_start_revision=None,
                 min_timestamp=None,
                 recovered=None,
                 subscriptions=None,
                 start_cursor=None,
                 test=None,
                 test_keys=None,
                 test_suite_name=None,
                 project_id=None):
    if key:
      # This tasklet isn't allowed to catch the internal_only AssertionError.
      alert = yield ndb.Key(urlsafe=key).get_async()
      raise ndb.Return(([alert], None, 1))

    # post_filters can cause results to be empty, depending on the shape of the
    # data and which filters are applied in the query and which filters are
    # applied after the query. Automatically chase cursors until some results
    # are found, but stay under the request timeout.
    results = []
    deadline = time.time() + deadline_seconds
    while not results and time.time() < deadline:
      query = cls.query()
      equality_properties = []
      if subscriptions:  # Empty subscriptions is not allowed in query
        query = query.filter(cls.subscription_names.IN(subscriptions))
        equality_properties.append('subscription_names')
        inequality_property = 'key'
      if is_improvement is not None:
        query = query.filter(cls.is_improvement == is_improvement)
        equality_properties.append('is_improvement')
        inequality_property = 'key'
      if bug_id is not None:
        if bug_id == '':
          query = query.filter(cls.bug_id == None)
          equality_properties.append('bug_id')
          inequality_property = 'key'
        elif bug_id != '*':
          query = query.filter(cls.bug_id == int(bug_id))
          equality_properties.append('bug_id')
          inequality_property = 'key'
        # bug_id='*' translates to bug_id != None, which is handled with the
        # other inequality filters.
      if recovered is not None:
        query = query.filter(cls.recovered == recovered)
        equality_properties.append('recovered')
        inequality_property = 'key'
      if test or test_keys:
        if not test_keys:
          test_keys = []
        if test:
          test_keys += [
              utils.OldStyleTestKey(test),
              utils.TestMetadataKey(test)
          ]
        query = query.filter(cls.test.IN(test_keys))
        query = query.order(cls.key)
        equality_properties.append('test')
        inequality_property = 'key'
      if master_name:
        query = query.filter(cls.master_name == master_name)
        equality_properties.append('master_name')
        inequality_property = 'key'
      if bot_name:
        query = query.filter(cls.bot_name == bot_name)
        equality_properties.append('bot_name')
        inequality_property = 'key'
      if test_suite_name:
        query = query.filter(cls.benchmark_name == test_suite_name)
        equality_properties.append('benchmark_name')
        inequality_property = 'key'

      query, post_filters = cls._InequalityFilters(
          query, equality_properties, inequality_property, bug_id,
          min_end_revision, max_end_revision, min_start_revision,
          max_start_revision, min_timestamp, max_timestamp)
      if post_filters:
        keys_only = False
      query = query.order(-cls.timestamp, cls.key)

      futures = [
          query.fetch_page_async(
              limit, start_cursor=start_cursor, keys_only=keys_only)
      ]
      if count_limit:
        futures.append(query.count_async(count_limit))
      query_duration = timing.WallTimeLogger('query_duration')
      with query_duration:
        yield futures
      results, start_cursor, more = futures[0].get_result()
      if count_limit:
        count = futures[1].get_result()
      else:
        count = len(results)
      logging.info('query_results_count=%d', len(results))
      if results:
        logging.info('duration_per_result=%f',
                     query_duration.seconds / len(results))
      if post_filters:
        results = [
            alert for alert in results if all(
                post_filter(alert) for post_filter in post_filters)
        ]
      # Temporary treat project_id as a postfilter. This is because some
      # chromium alerts have been booked with empty project_id.
      if project_id is not None:
        results = [
            alert for alert in results if alert.project_id == project_id
            or alert.project_id == '' and project_id == 'chromium'
        ]
      if not more:
        start_cursor = None
      if not start_cursor:
        break
    raise ndb.Return((results, start_cursor, count))

  @classmethod
  def _InequalityFilters(cls, query, equality_properties, inequality_property,
                         bug_id, min_end_revision, max_end_revision,
                         min_start_revision, max_start_revision, min_timestamp,
                         max_timestamp):
    # A query cannot have more than one inequality filter.
    # inequality_property allows users to decide which property to filter in the
    # query, which can significantly affect performance. If other inequalities
    # are specified, they will be handled by post_filters.

    # If callers set inequality_property without actually specifying a
    # corresponding inequality filter, then reset the inequality_property and
    # compute it automatically as if it were not specified.
    if inequality_property == 'start_revision':
      if min_start_revision is None and max_start_revision is None:
        inequality_property = None
    elif inequality_property == 'end_revision':
      if min_end_revision is None and max_end_revision is None:
        inequality_property = None
    elif inequality_property == 'timestamp':
      if min_timestamp is None and max_timestamp is None:
        inequality_property = None
    elif inequality_property == 'bug_id':
      if bug_id != '*':
        inequality_property = None
    elif inequality_property == 'key':
      if equality_properties == [
          'subscription_names'
      ] and (min_start_revision or max_start_revision):
        # Use the composite index (subscription_names, start_revision,
        # -timestamp). See index.yaml.
        inequality_property = 'start_revision'
    else:
      inequality_property = None

    if inequality_property is None:
      # Compute a default inequality_property.
      # We prioritise the 'min' filters first because that lets us limit the
      # amount of data the Datastore instances might handle.
      if min_start_revision:
        inequality_property = 'start_revision'
      elif min_end_revision:
        inequality_property = 'end_revision'
      elif min_timestamp:
        inequality_property = 'timestamp'
      elif max_start_revision:
        inequality_property = 'start_revision'
      elif max_end_revision:
        inequality_property = 'end_revision'
      elif max_timestamp:
        inequality_property = 'timestamp'
      elif bug_id == '*':
        inequality_property = 'bug_id'

    post_filters = []
    if not inequality_property:
      return query, post_filters

    if not datastore_hooks.IsUnalteredQueryPermitted():
      # _DatastorePreHook will filter internal_only=False. index.yaml does not
      # specify indexes for `internal_only, $inequality_property, -timestamp`.
      # Use post_filters for all inequality properties.
      inequality_property = ''

    if bug_id == '*':
      if inequality_property == 'bug_id':
        logging.info('filter:bug_id!=None')
        query = query.filter(cls.bug_id != None).order(cls.bug_id)
      else:
        logging.info('post_filter:bug_id!=None')
        post_filters.append(lambda a: a.bug_id != None)

    # Apply the min filters before the max filters, because that lets us
    # optimise the query application for more recent data, reducing the amount
    # of data post-processing.
    if min_start_revision:
      min_start_revision = int(min_start_revision)
      if inequality_property == 'start_revision':
        logging.info('filter:min_start_revision=%d', min_start_revision)
        query = query.filter(cls.start_revision >= min_start_revision)
        query = query.order(cls.start_revision)
      else:
        logging.info('post_filter:min_start_revision=%d', min_start_revision)
        post_filters.append(lambda a: a.start_revision >= min_start_revision)

    if min_end_revision:
      min_end_revision = int(min_end_revision)
      if inequality_property == 'end_revision':
        logging.info('filter:min_end_revision=%d', min_end_revision)
        query = query.filter(cls.end_revision >= min_end_revision)
        query = query.order(cls.end_revision)
      else:
        logging.info('post_filter:min_end_revision=%d', min_end_revision)
        post_filters.append(lambda a: a.end_revision >= min_end_revision)

    if min_timestamp:
      if inequality_property == 'timestamp':
        logging.info('filter:min_timestamp=%d',
                     time.mktime(min_timestamp.utctimetuple()))
        query = query.filter(cls.timestamp >= min_timestamp)
      else:
        logging.info('post_filter:min_timestamp=%d',
                     time.mktime(min_timestamp.utctimetuple()))
        post_filters.append(lambda a: a.timestamp >= min_timestamp)

    if max_start_revision:
      max_start_revision = int(max_start_revision)
      if inequality_property == 'start_revision':
        logging.info('filter:max_start_revision=%d', max_start_revision)
        query = query.filter(cls.start_revision <= max_start_revision)
        query = query.order(-cls.start_revision)
      else:
        logging.info('post_filter:max_start_revision=%d', max_start_revision)
        post_filters.append(lambda a: a.start_revision <= max_start_revision)

    if max_end_revision:
      max_end_revision = int(max_end_revision)
      if inequality_property == 'end_revision':
        logging.info('filter:max_end_revision=%d', max_end_revision)
        query = query.filter(cls.end_revision <= max_end_revision)
        query = query.order(-cls.end_revision)
      else:
        logging.info('post_filter:max_end_revision=%d', max_end_revision)
        post_filters.append(lambda a: a.end_revision <= max_end_revision)

    if max_timestamp:
      if inequality_property == 'timestamp':
        logging.info('filter:max_timestamp=%d',
                     time.mktime(max_timestamp.utctimetuple()))
        query = query.filter(cls.timestamp <= max_timestamp)
      else:
        logging.info('post_filter:max_timestamp=%d',
                     time.mktime(max_timestamp.utctimetuple()))
        post_filters.append(lambda a: a.timestamp <= max_timestamp)

    return query, post_filters
