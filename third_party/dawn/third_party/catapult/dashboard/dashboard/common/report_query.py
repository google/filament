# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging

from google.appengine.ext import ndb

from dashboard.common import descriptor
from dashboard.common import utils
from dashboard.models import graph_data
from tracing.value import histogram as histogram_module


def TableRowDescriptors(table_row):
  for test_suite in table_row['testSuites']:
    for bot in table_row['bots']:
      for case in table_row['testCases']:
        yield descriptor.Descriptor(test_suite, table_row['measurement'], bot,
                                    case)
      if not table_row['testCases']:
        yield descriptor.Descriptor(test_suite, table_row['measurement'], bot)


class ReportQuery:
  """Take a template and revisions. Return a report.

  Templates look like this: {
    statistics: ['avg', 'std'],
    rows: [
      {label, testSuites, measurement, bots, testCases},
    ],
  }

  Reports look like this: {
    statistics: ['avg', 'std'],
    rows: [
      {
        label, testSuites, measurement, bots, testCases, units,
        improvement_direction,
        data: {
          [revision]: {
            statistics: RunningStatisticsDict,
            descriptors: [{suite, bot, case, revision}],
          },
        },
      },
      ...
    ],
  }
  """

  def __init__(self, template, revisions):
    # Clone the template so that we can set table_row['data'] and the client
    # still gets to see the statistics, row testSuites, measurement, etc.
    self._report = dict(template)
    self._revisions = revisions
    self._max_revs = {}

    self._report['rows'] = []
    for row in template['rows']:
      row = dict(row)
      row['data'] = {}
      for rev in self._revisions:
        row['data'][rev] = []
      self._report['rows'].append(row)

  def FetchSync(self):
    return self.FetchAsync().get_result()

  @ndb.tasklet
  def FetchAsync(self):
    # Get data for each descriptor in each table row in parallel.
    futures = []
    for tri, table_row in enumerate(self._report['rows']):
      for desc in TableRowDescriptors(table_row):
        futures.append(self._GetRow(tri, table_row['data'], desc))
    yield futures

    # _GetRow can't know whether a datum will be merged until all the data have
    # been fetched, so post-process.
    for tri, table_row in enumerate(self._report['rows']):
      self._IgnoreStaleData(tri, table_row)
      self._IgnoreIncomparableData(table_row)
      self._SetRowUnits(table_row)
      self._IgnoreDataWithWrongUnits(table_row)
      self._MergeData(table_row)

    raise ndb.Return(self._report)

  def _IgnoreStaleData(self, tri, table_row):
    # Ignore data from test cases that were removed.
    for rev, data in table_row['data'].items():
      new_data = []
      for datum in data:
        max_rev_key = (datum['descriptor'].test_suite, datum['descriptor'].bot,
                       tri, rev)
        if datum['revision'] == self._max_revs[max_rev_key]:
          new_data.append(datum)
      table_row['data'][rev] = new_data

  def _IgnoreIncomparableData(self, table_row):
    # Ignore data from test cases that are not present for every rev.
    for rev, data in table_row['data'].items():
      new_data = []
      for datum in data:
        all_revs = True
        for other_data in table_row['data'].values():
          any_desc = False
          for other_datum in other_data:
            if other_datum['descriptor'] == datum['descriptor']:
              any_desc = True
              break

          if not any_desc:
            all_revs = False
            break

        if all_revs:
          new_data.append(datum)

      table_row['data'][rev] = new_data

  def _SetRowUnits(self, table_row):
    # Copy units from the first datum to the table_row.
    # Sort data first so this is deterministic.
    for rev in self._revisions:
      data = table_row['data'][rev] = sorted(
          table_row['data'][rev], key=lambda datum: datum['descriptor'])
      if data:
        table_row['units'] = data[0]['units']
        table_row['improvement_direction'] = data[0]['improvement_direction']
        break

  def _IgnoreDataWithWrongUnits(self, table_row):
    for rev, data in table_row['data'].items():
      new_data = []
      for datum in data:
        if datum['units'] == table_row['units']:
          new_data.append(datum)
        else:
          logging.warning('Expected units=%r; %r', table_row['units'], datum)
      table_row['data'][rev] = new_data

  def _MergeData(self, table_row):
    for rev, data in table_row['data'].items():
      statistics = histogram_module.RunningStatistics()
      for datum in data:
        statistics = statistics.Merge(datum['statistics'])
      revision = rev
      if data:
        revision = data[0]['revision']
      table_row['data'][rev] = {
          'statistics': statistics.AsDict(),
          'descriptors': [{
              'testSuite': datum['descriptor'].test_suite,
              'bot': datum['descriptor'].bot,
              'testCase': datum['descriptor'].test_case,
          } for datum in data],
          'revision': revision,
      }

  @ndb.tasklet
  def _GetRow(self, tri, table_row, desc):
    # First try to find the unsuffixed test.
    test_paths = yield desc.ToTestPathsAsync()
    logging.info('_GetRow %r', test_paths)
    unsuffixed_tests = yield [
        utils.TestMetadataKey(test_path).get_async() for test_path in test_paths
    ]
    unsuffixed_tests = [t for t in unsuffixed_tests if t]

    if not unsuffixed_tests:
      # Fall back to suffixed tests.
      yield [
          self._GetSuffixedCell(tri, table_row, desc, rev)
          for rev in self._revisions
      ]

    for test in unsuffixed_tests:
      test_path = utils.TestPath(test.key)
      yield [
          self._GetUnsuffixedCell(tri, table_row, desc, test, test_path, rev)
          for rev in self._revisions
      ]

  @ndb.tasklet
  def _GetUnsuffixedCell(self, tri, table_row, desc, test, test_path, rev):
    data_row = yield self._GetDataRow(test_path, rev)
    if data_row is None:
      # Fall back to suffixed tests.
      yield self._GetSuffixedCell(tri, table_row, desc, rev)
      raise ndb.Return()

    statistics = {
        stat: getattr(data_row, 'd_' + stat)
        for stat in descriptor.STATISTICS
        if hasattr(data_row, 'd_' + stat)
    }
    if 'avg' not in statistics:
      statistics['avg'] = data_row.value
    if 'std' not in statistics and data_row.error:
      statistics['std'] = data_row.error
    datum = dict(
        descriptor=desc,
        units=test.units,
        improvement_direction=test.improvement_direction,
        revision=data_row.revision,
        statistics=_MakeRunningStatistics(statistics))
    table_row[rev].append(datum)

    max_rev_key = (desc.test_suite, desc.bot, tri, rev)
    self._max_revs[max_rev_key] = max(
        self._max_revs.get(max_rev_key, 0), data_row.revision)

  @ndb.tasklet
  def _GetSuffixedCell(self, tri, table_row, desc, rev):
    datum = {'descriptor': desc}
    statistics = yield [
        self._GetStatistic(datum, desc, rev, stat)
        for stat in descriptor.STATISTICS
    ]
    statistics = {
        descriptor.STATISTICS[i]: stat
        for i, stat in enumerate(statistics)
        if stat is not None
    }
    if 'avg' not in statistics:
      raise ndb.Return()

    table_row[rev].append(datum)
    datum['statistics'] = _MakeRunningStatistics(statistics)

    max_rev_key = (desc.test_suite, desc.bot, tri, rev)
    self._max_revs[max_rev_key] = max(
        self._max_revs.get(max_rev_key, 0), datum['revision'])

  @ndb.tasklet
  def _GetStatistic(self, datum, desc, rev, stat):
    desc = desc.Clone()
    desc.statistic = stat
    test_paths = yield desc.ToTestPathsAsync()
    logging.info('_GetStatistic %r', test_paths)
    suffixed_tests = yield [
        utils.TestMetadataKey(test_path).get_async() for test_path in test_paths
    ]
    suffixed_tests = [t for t in suffixed_tests if t]
    if not suffixed_tests:
      raise ndb.Return(None)

    last_data_row = None
    for test in suffixed_tests:
      if stat == 'avg':
        datum['units'] = test.units
        datum['improvement_direction'] = test.improvement_direction
      test_path = utils.TestPath(test.key)
      data_row = yield self._GetDataRow(test_path, rev)
      if not last_data_row or data_row.revision > last_data_row.revision:
        last_data_row = data_row
    if not last_data_row:
      raise ndb.Return(None)
    datum['revision'] = last_data_row.revision
    raise ndb.Return(last_data_row.value)

  @ndb.tasklet
  def _GetDataRow(self, test_path, rev):
    entities = yield [
        self._GetDataRowForKey(utils.TestMetadataKey(test_path), rev),
        self._GetDataRowForKey(utils.OldStyleTestKey(test_path), rev)
    ]
    entities = [e for e in entities if e]
    if not entities:
      raise ndb.Return(None)
    if len(entities) > 1:
      logging.warning('Found too many Row entities: %r %r', rev, test_path)
      raise ndb.Return(None)
    raise ndb.Return(entities[0])

  @ndb.tasklet
  def _GetDataRowForKey(self, test_key, rev):
    query = graph_data.Row.query(graph_data.Row.parent_test == test_key)
    if rev != 'latest':
      query = query.filter(graph_data.Row.revision <= rev)
    query = query.order(-graph_data.Row.revision)  # pylint: disable=invalid-unary-operand-type
    data_row = yield query.get_async()
    raise ndb.Return(data_row)


def _MakeRunningStatistics(statistics):
  if statistics.get('avg') is None:
    return None
  count = statistics.get('count', 10)
  std = statistics.get('std', 0)
  return histogram_module.RunningStatistics.FromDict([
      count,
      statistics.get('max', statistics['avg']),
      0,  # meanlogs for geometricMean
      statistics['avg'],
      statistics.get('min', statistics['avg']),
      statistics.get('sum', statistics['avg'] * count),
      std * std * (count - 1)
  ])
