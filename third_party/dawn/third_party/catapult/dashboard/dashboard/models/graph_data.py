# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""The datastore models for graph data.

The Chromium project uses Buildbot to run its performance tests, and the
structure of the data for the Performance Dashboard reflects this. Metadata
about tests are structured in Master, Bot, and TestMetadata entities. Master and
Bot entities represent Buildbot masters and builders respectively, and
TestMetadata entities represent groups of results, or individual data series,
keyed by a full path to the test separated by '/' characters.

For example, entities might be structured as follows:

  Master: ChromiumPerf
  Bot: win7
  TestMetadata: ChromiumPerf/win7/page_cycler.moz
  TestMetadata: ChromiumPerf/win7/page_cycler.moz/times
  TestMetadata: ChromiumPerf/win7/page_cycler.moz/times/page_load_time
  TestMetadata: ChromiumPerf/win7/page_cycler.moz/times/page_load_time_ref
  TestMetadata: ChromiumPerf/win7/page_cycler.moz/times/www.amazon.com
  TestMetadata: ChromiumPerf/win7/page_cycler.moz/times/www.bing.com
  TestMetadata: ChromiumPerf/win7/page_cycler.moz/commit_charge
  TestMetadata: ChromiumPerf/win7/page_cycler.moz/commit_charge/ref
  TestMetadata: ChromiumPerf/win7/page_cycler.moz/commit_charge/www.amazon.com
  TestMetadata: ChromiumPerf/win7/page_cycler.moz/commit_charge/www.bing.com

The graph data points are represented by Row entities. Each Row entity contains
a revision and value, which are its X and Y values on a graph, and any other
metadata associated with an individual performance test result.

The keys of the Row entities for a particular data series are start with a
TestContainer key, instead of a TestMetadata key. This way, the Row entities for
each data series are in a different "entity group". This allows a faster rate of
putting data in the datastore for many series at once.

For example, Row entities are organized like this:

  TestContainer: ChromiumPerf/win7/page_cycler.moz/times/page_load_time
    Row: revision 12345, value 2.5
    Row: revision 12346, value 2.0
    Row: revision 12347, value 2.1
  TestContainer: ChromiumPerf/win7/page_cycler.moz/times/page_load_time_ref
    Row: revision 12345, value 2.4
    Row: revision 12346, value 2.0
    Row: revision 12347, value 2.2
  TestContainer: ChromiumPerf/win7/page_cycler.moz/commit_charge
    Row: revision 12345, value 10
    Row: revision 12346, value 12
    Row: revision 12347, value 11


IMPORTANT: If you add new kinds to this file, you must also add them to the
Daily Backup url in cron.yaml in order for them to be properly backed up.
See: https://developers.google.com/appengine/articles/scheduled_backups
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
import json
import logging
import six

from google.appengine.ext import ndb

from dashboard.common import layered_cache
from dashboard.common import utils
from dashboard.models import anomaly
from dashboard.models import anomaly_config
from dashboard.models import internal_only_model

from dateutil.relativedelta import relativedelta

# Maximum level of nested tests.
MAX_TEST_ANCESTORS = 10

# Keys to the datastore-based cache. See stored_object.
LIST_TESTS_SUBTEST_CACHE_KEY = 'list_tests_get_tests_v2_%s_%s_%s'

_MAX_STRING_LENGTH = 500


class Master(ndb.Model):
  """Information about the Buildbot master.

  Masters are keyed by name, e.g. 'ChromiumGPU' or 'ChromiumPerf'.
  All Bot entities that are Buildbot slaves of one master are children of one
  Master entity in the datastore.
  """
  # Master has no properties; the name of the master is the ID.


class Bot(internal_only_model.InternalOnlyModel):
  """Information about a Buildbot slave that runs perf tests.

  Bots are keyed by name, e.g. 'xp-release-dual-core'. A Bot entity contains
  information about whether the tests are only viewable to internal users, and
  each bot has a parent that is a Master entity. To query the tests that run on
  a Bot, check the bot_name and master_name properties of the TestMetadata.
  """
  internal_only = ndb.BooleanProperty(default=False, indexed=True)

  _use_memcache = True

  @classmethod
  @ndb.synctasklet
  def GetInternalOnlySync(cls, master, bot):
    try:
      internal_only = yield cls.GetInternalOnlyAsync(master, bot)
      raise ndb.Return(internal_only)
    except AssertionError as e:
      logging.warning(
          'Failed to get internal_only for (master: %s, bot: %s). Error: %s.',
          master, bot, e)
      six.raise_from(ndb.Return(True), e)

  @staticmethod
  @ndb.tasklet
  def GetInternalOnlyAsync(master, bot):
    bot_entity = yield ndb.Key('Master', master, 'Bot', bot).get_async()
    if bot_entity is None:
      logging.warning('Failed to find Bot Entity with master %s and bot %s.',
                      master, bot)
      raise ndb.Return(True)
    raise ndb.Return(bot_entity.internal_only)


class TestMetadata(internal_only_model.CreateHookInternalOnlyModel):
  """A TestMetadata entity is a node in a hierarchy of tests.

  A TestMetadata entity can represent a specific series of results which will be
  plotted on a graph, or it can represent a group of such series of results, or
  both. A TestMetadata entity that the property has_rows set to True corresponds
  to a timeseries on a graph, and the TestMetadata for a group of such tests
  has a path one level less deep, which corresponds to a graph with several
  timeseries. A TestMetadata one level less deep for that test would correspond
  to a group of related graphs. Top-level TestMetadata (also known as test
  suites) are keyed master/bot/test.

  TestMetadata are keyed by the full path to the test (for example
  master/bot/test/metric/page), and they also contain other metadata such as
  description and units.

  NOTE: If you remove any properties from TestMetadata, they should be added to
  the TEST_EXCLUDE_PROPERTIES list in migrate_test_names.py.
  """
  # Enable memcache to reduce Firestore Read Ops cost.
  _use_memcache = True

  internal_only = ndb.BooleanProperty(default=False, indexed=True)

  # There is a default anomaly threshold config (in anomaly.py), and it can
  # be overridden for a group of tests by using /edit_sheriffs.
  overridden_anomaly_config = ndb.KeyProperty(
      kind=anomaly_config.AnomalyConfig, indexed=False)

  # Keep track of what direction is an improvement for this graph so we can
  # filter out alerts on regressions.
  improvement_direction = ndb.IntegerProperty(
      default=anomaly.UNKNOWN,
      choices=[anomaly.UP, anomaly.DOWN, anomaly.UNKNOWN],
      indexed=False)

  # Units of the child Rows of this test, or None if there are no child Rows.
  units = ndb.StringProperty(indexed=False)

  # Whether or not the test has child rows. Set by hook on Row class put.
  has_rows = ndb.BooleanProperty(default=False, indexed=True)

  # A test is marked "deprecated" if no new points have been received for
  # a long time; these tests should usually not be listed.
  deprecated = ndb.BooleanProperty(default=False, indexed=True)

  # Description of what the test measures.
  description = ndb.TextProperty(indexed=True)

  # Story names are escaped (slashes, colons). Store unescaped version
  # for story filter flag.
  unescaped_story_name = ndb.StringProperty(indexed=False)

  # Computed properties are treated like member variables, so they have
  # lowercase names, even though they look like methods to pylint.
  # pylint: disable=invalid-name

  @ndb.ComputedProperty
  def bot(self):  # pylint: disable=invalid-name
    """Immediate parent Bot entity, or None if this is not a test suite."""
    parts = self.key.id().split('/')
    if len(parts) != 3:
      # This is not a test suite.
      return None
    return ndb.Key('Master', parts[0], 'Bot', parts[1])

  @ndb.ComputedProperty
  def parent_test(self):  # pylint: disable=invalid-name
    """Immediate parent TestMetadata entity, or None if this is a test suite."""
    parts = self.key.id().split('/')
    if len(parts) < 4:
      # This is a test suite
      return None
    return ndb.Key('TestMetadata', '/'.join(parts[:-1]))

  @property
  def test_name(self):
    """The name of this specific test, without the test_path preceding."""
    return self.key.id().split('/')[-1]

  @property
  def test_path(self):
    """Slash-separated list of key parts, 'master/bot/suite/chart/...'."""
    return utils.TestPath(self.key)

  @ndb.ComputedProperty
  def master_name(self):
    return self.key.id().split('/')[0]

  @ndb.ComputedProperty
  def bot_name(self):
    return self.key.id().split('/')[1]

  @ndb.ComputedProperty
  def suite_name(self):
    return self.key.id().split('/')[2]

  @ndb.ComputedProperty
  def test_part1_name(self):
    parts = self.key.id().split('/')
    if len(parts) < 4:
      return ''
    return parts[3]

  @ndb.ComputedProperty
  def test_part2_name(self):
    parts = self.key.id().split('/')
    if len(parts) < 5:
      return ''
    return parts[4]

  @ndb.ComputedProperty
  def test_part3_name(self):
    parts = self.key.id().split('/')
    if len(parts) < 6:
      return ''
    return parts[5]

  @ndb.ComputedProperty
  def test_part4_name(self):
    parts = self.key.id().split('/')
    if len(parts) < 7:
      return ''
    return parts[6]

  @ndb.ComputedProperty
  def test_part5_name(self):
    parts = self.key.id().split('/')
    if len(parts) < 8:
      return ''
    return parts[7]

  @classmethod
  def _GetMasterBotSuite(cls, key):
    if not key:
      return None
    return tuple(key.id().split('/')[:3])

  def __init__(self, *args, **kwargs):
    # Indexed StringProperty has a maximum length. If this length is exceeded,
    # then an error will be thrown in ndb.Model.__init__.
    # Truncate the "description" property if necessary.
    description = kwargs.get('description') or ''
    kwargs['description'] = description[:_MAX_STRING_LENGTH]
    super().__init__(*args, **kwargs)

  @ndb.synctasklet
  def UpdateSheriff(self):
    r = yield self.UpdateSheriffAsync()
    raise ndb.Return(r)

  @ndb.tasklet
  def UpdateSheriffAsync(self, anomaly_configs=None):
    """This method is called before a TestMetadata is put into the datastore.

    Here, we check the key to make sure it is valid and check the sheriffs and
    anomaly configs to make sure they are current.
    """
    # Check to make sure the key is valid.
    # TestMetadata should not be an ancestor, so key.pairs() should have length
    # of 1. The id should have at least 3 slashes to represent master/bot/suite.
    assert len(self.key.pairs()) == 1
    path_parts = self.key.id().split('/')
    assert len(path_parts) >= 3

    # Set the anomaly threshold config to the first one that has a test pattern
    # that matches this test, if there is one. Anomaly configs with a pattern
    # that more specifically matches the test are given higher priority.
    # ie. */*/*/foo is chosen over */*/*/*
    old_anomaly_config = self.overridden_anomaly_config
    self.overridden_anomaly_config = None
    if not anomaly_configs:
      anomaly_configs = yield anomaly_config.AnomalyConfig.query().fetch_async()
    anomaly_data_list = []
    for e in anomaly_configs:
      for p in e.patterns:
        anomaly_data_list.append((p, e))
    anomaly_config_to_use = utils.MostSpecificMatchingPattern(
        self, anomaly_data_list)
    if anomaly_config_to_use:
      self.overridden_anomaly_config = anomaly_config_to_use.key

    raise ndb.Return(self.overridden_anomaly_config != old_anomaly_config)

  def CreateCallback(self):
    """Called when the entity is first saved."""
    if len(self.key.id().split('/')) > 3:
      # Since this is not a test suite, the menu cache for the suite must
      # be updated.
      layered_cache.Delete(LIST_TESTS_SUBTEST_CACHE_KEY %
                           self._GetMasterBotSuite(self.key))

  @classmethod
  # pylint: disable=unused-argument
  def _pre_delete_hook(cls, key):
    if len(key.id().split('/')) > 3:
      # Since this is not a test suite, the menu cache for the suite must
      # be updated.
      layered_cache.Delete(LIST_TESTS_SUBTEST_CACHE_KEY %
                           TestMetadata._GetMasterBotSuite(key))


class LastAddedRevision(ndb.Model):
  """Represents the last added revision for a test path.

  The reason this property is separated from TestMetadata entity is to avoid
  contention issues (Frequent update of entity within the same group).  This
  property is updated very frequent in /add_point.
  """
  # Our access patterns don't generally involve the same LastAddedRevision being
  # accessed again and again across multiple requests. Don't put them into
  # memcache when accessed by default. For more info, see:
  # https://cloud.google.com/appengine/docs/python/ndb/cache
  _use_memcache = False
  revision = ndb.IntegerProperty(indexed=False)


class Row(ndb.Expando):
  """A Row represents one data point.

  A Row has a revision and a value, which are the X and Y values, respectively.
  Each Row belongs to one TestMetadata, along with all of the other Row entities
  that it is plotted with. Rows are keyed by revision.

  In addition to the properties defined below, Row entities may also have other
  properties which specify additional supplemental data. These are called
  "supplemental columns", and should have the following prefixes:
    d_: A data point, such as d_1st_run or d_50th_percentile. FloatProperty.
    r_: Revision such as r_webkit or r_v8. StringProperty, limited to 25
        characters, '0-9' and '.'.
    a_: Annotation such as a_chrome_bugid or a_gasp_anomaly. StringProperty.
  """
  # Our access patterns don't generally involve the same Row being
  # accessed again and again across multiple requests. Don't put them into
  # memcache when accessed by default. For more info, see:
  # https://cloud.google.com/appengine/docs/python/ndb/cache
  _use_memcache = False

  # Don't index by default (only explicitly indexed properties are indexed).
  _default_indexed = False

  # The parent_test is the key of the TestMetadata entity that this Row belongs
  # to.
  @ndb.ComputedProperty
  def parent_test(self):  # pylint: disable=invalid-name
    # The Test entity that a Row belongs to isn't actually its parent in
    # the datastore. Rather, the parent key of each Row contains a test path,
    # which contains the information necessary to get the actual Test
    # key. The Test key will need to be converted back to a new style
    # TestMetadata key to get information back out. This is because we have
    # over 3 trillion Rows in the datastore and cannot convert them all :(
    return utils.OldStyleTestKey(utils.TestKey(self.key.parent().string_id()))

  # Points in each graph are sorted by "revision". This is usually a Chromium
  # SVN version number, but it might also be any other integer, as long as
  # newer points have higher numbers.
  @ndb.ComputedProperty
  def revision(self):  # pylint: disable=invalid-name
    return self.key.integer_id()

  # The time the revision was added to the dashboard is tracked in order
  # to too many points from being added in a short period of time, which would
  # indicate an error or malicious code.
  timestamp = ndb.DateTimeProperty(auto_now_add=True, indexed=True)

  # The Y-value at this point.
  value = ndb.FloatProperty(indexed=True)

  # The standard deviation at this point. Optional.
  error = ndb.FloatProperty(indexed=False)

  # A *single* swarming device ID, or None. Never multiple.
  swarming_bot_id = ndb.StringProperty(indexed=True)

  @ndb.ComputedProperty
  def expiry(self):  # pylint: disable=invalid-name
    if self.timestamp:
      return self.timestamp + relativedelta(years=3)

    return datetime.datetime.utcnow() + relativedelta(years=3)

  @ndb.tasklet
  def UpdateParentAsync(self, **ctx_options):
    parent_test = yield utils.TestMetadataKey(
        self.key.parent().id()).get_async(**ctx_options)

    if not parent_test:
      parent_key = self.key.parent()
      logging.warning('Row put without valid TestMetadata. Parent key: %s',
                      parent_key)
      return

    if not parent_test.has_rows:
      parent_test.has_rows = True
      yield parent_test.UpdateSheriffAsync()
      yield parent_test.put_async(**ctx_options)


class MigrationJob(ndb.Model):
  job_id = ndb.StringProperty()
  entry_type = ndb.StringProperty()
  entry_data = ndb.StringProperty()
  status = ndb.StringProperty()
  start_time = ndb.DateTimeProperty(auto_now_add=True)
  end_time = ndb.DateTimeProperty(auto_now=True)

  def GetEntryData(self, cls):
    json_dict = json.loads(self.entry_data)
    return cls(json_dict)

  def SetEntryData(self, entry_data):
    self.entry_data = json.dumps(entry_data,
                                 default=lambda entry_data: entry_data.__dict__)


def GetRowsForTestInRange(test_key, start_rev, end_rev):
  """Gets all the Row entities for a test between a given start and end."""
  test_key = utils.OldStyleTestKey(test_key)
  query = Row.query(Row.parent_test == test_key, Row.revision >= start_rev,
                    Row.revision <= end_rev)
  return query.fetch(batch_size=100)


def GetRowsForTestAroundRev(test_key, rev, num_points):
  """Gets up to |num_points| Row entities for a test centered on a revision."""
  test_key = utils.OldStyleTestKey(test_key)
  num_rows_before = int(num_points / 2) + 1
  num_rows_after = int(num_points / 2)

  return GetRowsForTestBeforeAfterRev(test_key, rev, num_rows_before,
                                      num_rows_after)


def GetRowsForTestBeforeAfterRev(test_key, rev, num_rows_before,
    num_rows_after):
  """Gets up to |num_points| Row entities for a test centered on a revision."""
  test_key = utils.OldStyleTestKey(test_key)

  query_up_to_rev = Row.query(Row.parent_test == test_key, Row.revision <= rev)
  query_up_to_rev = query_up_to_rev.order(-Row.revision)  # pylint: disable=invalid-unary-operand-type
  rows_up_to_rev = list(
      reversed(query_up_to_rev.fetch(limit=num_rows_before, batch_size=100)))

  query_after_rev = Row.query(Row.parent_test == test_key, Row.revision > rev)
  query_after_rev = query_after_rev.order(Row.revision)
  rows_after_rev = query_after_rev.fetch(limit=num_rows_after, batch_size=100)

  return rows_up_to_rev + rows_after_rev


def GetLatestRowsForTest(test_key, num_points, keys_only=False):
  """Gets the latest num_points Row entities for a test."""
  test_key = utils.OldStyleTestKey(test_key)
  query = Row.query(Row.parent_test == test_key)
  query = query.order(-Row.revision)  # pylint: disable=invalid-unary-operand-type

  return query.fetch(limit=num_points, batch_size=100, keys_only=keys_only)
