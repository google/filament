# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Translate between test paths and Descriptors.

Test paths describe a timeseries by its path in a tree of timeseries.
Descriptors describe a timeseries semantically by its characteristics.
Descriptors allow users to navigate timeseries use meaningful words like
"measurement" and "test case" instead of meaningless words like "subtest".
Test paths can be arbitrarily long, but there are a fixed number of semantic
characteristics. Multiple test path components may be joined into a single
characteristic.

These are timeseries Descriptors, not test suite descriptors, not line
descriptors, not fetch descriptors.

This translation layer should be temporary until the descriptor concept can be
pushed down into the Model layer.
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import re

from google.appengine.ext import ndb

from dashboard.common import bot_configurations
from dashboard.common import stored_object
from dashboard.common import utils

TEST_BUILD_TYPE = 'test'
REFERENCE_BUILD_TYPE = 'ref'
NO_MITIGATIONS_CASE = 'no-mitigations'
# TODO(fancl): Remove alias.
STATISTICS = utils.STATISTICS
STATISTICS_REGEX = '(.*)_(%s)$' % '|'.join(STATISTICS + [
    r'pct_[\d_]+',
    r'ipr_[\d_]+',
    r'ci_[\d]{3}(?:_lower|_upper)?',
])

# This stored object contains a list of test path component strings that must be
# joined with the subsequent test path component in order to form a composite
# test suite. Some are internal-only, but there's no need to store a separate
# list for external users since this data is not served, just used to parse test
# keys.
PARTIAL_TEST_SUITES_KEY = 'partial_test_suites'

# This stored object contains a list of test suites composed of 2 or more test
# path components. All composite test suites start with a partial test suite,
# but not all test suites that start with a partial test suite are composite.
COMPOSITE_TEST_SUITES_KEY = 'composite_test_suites'

# This stored object contains a list of prefixes of test suites that are
# transformed to allow the UI to group them.
GROUPABLE_TEST_SUITE_PREFIXES_KEY = 'groupable_test_suite_prefixes'

# This stored object contains a list of test suites whose measurements are
# composed of multiple test path components.
POLY_MEASUREMENT_TEST_SUITES_KEY = 'poly_measurement_test_suites'

# This stored object contains a list of test suites whose measurements and test
# cases are each composed of two test path components.
TWO_TWO_TEST_SUITES_KEY = 'two_two_test_suites'

# This stored object contains a list of test suites whose test cases are
# composed of two test path components.
ONE_TWO_TEST_SUITES_KEY = 'one_two_test_suites'

# This stored object contains a list of test suites whose test cases are
# partially duplicated to two test path components like 'prefix/prefix_suffix'.
COMPLEX_CASES_TEST_SUITES_KEY = 'complex_cases_test_suites'


class Descriptor:
  """Describe a timeseries by its characteristics.

  Supports partial test paths (e.g. test suite paths) by allowing some
  characteristics to be None.
  """

  def __init__(self,
               test_suite=None,
               measurement=None,
               bot=None,
               test_case=None,
               statistic=None,
               build_type=None):
    self.test_suite = test_suite
    self.measurement = measurement
    self.bot = bot
    self.test_case = test_case
    self.statistic = statistic
    self.build_type = build_type

  def Clone(self):
    return Descriptor(self.test_suite, self.measurement, self.bot,
                      self.test_case, self.statistic, self.build_type)

  def __repr__(self):
    return 'Descriptor(%r, %r, %r, %r, %r, %r)' % (
        self.test_suite, self.measurement, self.bot, self.test_case,
        self.statistic, self.build_type)

  def __hash__(self):
    return hash(self.test_suite, self.measurement, self.bot, self.test_case,
                self.statistic, self.build_type)

  def __eq__(self, other):
    return repr(self) == repr(other)

  def __lt__(self, other):
    return repr(self) < repr(other)

  CONFIGURATION = {}

  @classmethod
  @ndb.tasklet
  def _GetConfiguration(cls, key, default=None):
    if key not in cls.CONFIGURATION:
      cls.CONFIGURATION[key] = (yield stored_object.GetAsync(key)) or default
    raise ndb.Return(cls.CONFIGURATION[key])

  @classmethod
  def ResetMemoizedConfigurationForTesting(cls):
    cls.CONFIGURATION = {}

  @classmethod
  @ndb.tasklet
  def _MeasurementCase(cls, test_suite, path):
    if len(path) == 1:
      raise ndb.Return((path.pop(0), None))

    if test_suite.startswith('loading.'):
      measurement = path.pop(0)
      parts, path[:] = path[:], []
      if len(parts) > 1 and parts[1].endswith('_' + parts[0]):
        parts[1] = parts[1][:-(len(parts[0]) + 1)]
      raise ndb.Return((measurement, ':'.join(parts)))

    if test_suite.startswith('resource_sizes'):
      parts, path[:] = path[:], []
      raise ndb.Return((':'.join(parts), None))

    if test_suite == 'sizes':
      parts, path[:] = path[:], []
      raise ndb.Return((':'.join(parts[:6]), ':'.join(parts[6:])))

    complex_cases_test_suites = yield cls._GetConfiguration(
        COMPLEX_CASES_TEST_SUITES_KEY, [])
    if (test_suite.startswith('system_health')
        or (test_suite in complex_cases_test_suites)):
      measurement = path.pop(0)
      prefix = path.pop(0)
      if len(path) == 0:
        raise ndb.Return((measurement, prefix.replace('_', ':')))
      raise ndb.Return(
          (measurement,
           path.pop(0).replace('_', ':').replace('long:running:tools',
                                                 'long_running_tools')))

    one_two_test_suites = yield cls._GetConfiguration(ONE_TWO_TEST_SUITES_KEY,
                                                      [])
    if test_suite in one_two_test_suites:
      parts, path[:] = path[:], []
      raise ndb.Return(parts[0], ':'.join(parts[1:]))

    two_two_test_suites = yield cls._GetConfiguration(TWO_TWO_TEST_SUITES_KEY,
                                                      [])
    if test_suite in two_two_test_suites:
      parts, path[:] = path[:], []
      raise ndb.Return(':'.join(parts[:2]), ':'.join(parts[2:]))

    if test_suite in [
        'memory.dual_browser_test', 'memory.top_10_mobile',
        'v8:runtime_stats.top_25'
    ]:
      measurement = path.pop(0)
      case = path.pop(0)
      if len(path) == 0:
        raise ndb.Return((measurement, None))
      raise ndb.Return((measurement, case + ':' + path.pop(0)))

    if test_suite in (yield
                      cls._GetConfiguration(POLY_MEASUREMENT_TEST_SUITES_KEY,
                                            [])):
      parts, path[:] = path[:], []
      case = None
      if parts[-1] == NO_MITIGATIONS_CASE:
        case = parts.pop()
      raise ndb.Return((':'.join(parts), case))

    raise ndb.Return((path.pop(0), path.pop(0)))

  @classmethod
  def FromTestPathSync(cls, test_path):
    return cls.FromTestPathAsync(test_path).get_result()

  @classmethod
  @ndb.tasklet
  def FromTestPathAsync(cls, test_path):
    """Parse a test path into a Descriptor.

    Args:
      path: Array of strings of any length.

    Returns:
      Descriptor
    """
    path = test_path.split('/')
    if len(path) < 2:
      raise ndb.Return(cls())

    bot = path.pop(0) + ':' + path.pop(0)
    if len(path) == 0:
      raise ndb.Return(cls(bot=bot))

    test_suite = path.pop(0)

    if test_suite in (yield cls._GetConfiguration(PARTIAL_TEST_SUITES_KEY, [])):
      if len(path) == 0:
        raise ndb.Return(cls(bot=bot))
      test_suite += ':' + path.pop(0)

    if test_suite.startswith('resource_sizes '):
      test_suite = 'resource_sizes:' + test_suite[16:-1]
    else:
      for prefix in (yield
                     cls._GetConfiguration(GROUPABLE_TEST_SUITE_PREFIXES_KEY,
                                           [])):
        if test_suite.startswith(prefix):
          test_suite = prefix[:-1] + ':' + test_suite[len(prefix):]
          break

    if len(path) == 0:
      raise ndb.Return(cls(test_suite=test_suite, bot=bot))

    build_type = TEST_BUILD_TYPE
    if path[-1] == 'ref':
      path.pop()
      build_type = REFERENCE_BUILD_TYPE
    elif path[-1].endswith('_ref'):
      build_type = REFERENCE_BUILD_TYPE
      path[-1] = path[-1][:-4]

    if len(path) == 0:
      raise ndb.Return(
          cls(test_suite=test_suite, bot=bot, build_type=build_type))

    measurement, test_case = yield cls._MeasurementCase(test_suite, path)

    statistic = None
    if measurement not in ['jank_count', 'exp_jank_count']:
      stat_match = re.match(STATISTICS_REGEX, measurement)
      if stat_match:
        measurement, statistic = stat_match.groups()

    if test_suite != 'graphics:GLBench' and path:
      raise ValueError('Unable to parse %r' % test_path)

    raise ndb.Return(
        cls(test_suite=test_suite,
            bot=bot,
            measurement=measurement,
            statistic=statistic,
            test_case=test_case,
            build_type=build_type))

  def ToTestPathsSync(self):
    return self.ToTestPathsAsync().get_result()

  @ndb.tasklet
  def ToTestPathsAsync(self):
    # There may be multiple possible test paths for a given Descriptor.

    if not self.bot:
      raise ndb.Return(set())

    test_paths = yield self._BotTestPaths()
    if not self.test_suite:
      raise ndb.Return(test_paths)

    test_paths = yield self._AppendTestSuite(test_paths)
    if not self.measurement:
      raise ndb.Return(test_paths)

    test_paths = yield self._AppendMeasurement(test_paths)
    if self.statistic:
      test_paths = {p + '_' + self.statistic for p in test_paths}

    if self.test_case:
      test_paths = yield self._AppendTestCase(test_paths)

    if self.build_type == REFERENCE_BUILD_TYPE:
      test_paths = self._AppendRef(test_paths)

    raise ndb.Return(test_paths)

  @ndb.tasklet
  def _BotTestPaths(self):
    master, slave = self.bot.split(':')
    aliases = yield bot_configurations.GetAliasesAsync(slave)
    raise ndb.Return({master + '/' + alias for alias in aliases})

  @ndb.tasklet
  def _AppendTestSuite(self, test_paths):
    if self.test_suite.startswith('resource_sizes:'):
      raise ndb.Return({
          p + '/resource_sizes (%s)' % self.test_suite[15:] for p in test_paths
      })

    composite_test_suites = yield self._GetConfiguration(
        COMPOSITE_TEST_SUITES_KEY, [])
    if self.test_suite in composite_test_suites:
      raise ndb.Return(
          {p + '/' + self.test_suite.replace(':', '/') for p in test_paths})

    first_part = self.test_suite.split(':')[0]
    groupable_prefixes = yield self._GetConfiguration(
        GROUPABLE_TEST_SUITE_PREFIXES_KEY, [])
    for prefix in groupable_prefixes:
      if prefix[:-1] == first_part:
        raise ndb.Return({
            p + '/' + prefix + self.test_suite[len(first_part) + 1:]
            for p in test_paths
        })

    raise ndb.Return({p + '/' + self.test_suite for p in test_paths})

  @ndb.tasklet
  def _AppendMeasurement(self, test_paths):
    poly_measurement_test_suites = yield self._GetConfiguration(
        POLY_MEASUREMENT_TEST_SUITES_KEY, [])
    poly_measurement_test_suites += yield self._GetConfiguration(
        TWO_TWO_TEST_SUITES_KEY, [])
    if self.test_suite in poly_measurement_test_suites:
      raise ndb.Return(
          {p + '/' + self.measurement.replace(':', '/') for p in test_paths})

    raise ndb.Return({p + '/' + self.measurement for p in test_paths})

  @ndb.tasklet
  def _AppendTestCase(self, test_paths):
    complex_cases_test_suites = yield self._GetConfiguration(
        COMPLEX_CASES_TEST_SUITES_KEY, [])
    if (self.test_suite.startswith('system_health')
        or (self.test_suite in complex_cases_test_suites)):
      test_case = self.test_case.split(':')
      if test_case[0] == 'long_running_tools':
        test_paths = {p + '/' + test_case[0] for p in test_paths}
      else:
        test_paths = {p + '/' + '_'.join(test_case[:2]) for p in test_paths}
      raise ndb.Return({p + '/' + '_'.join(test_case) for p in test_paths})

    if self.test_suite.startswith('loading.'):
      raise ndb.Return({
          p + '/' + self.test_case.replace(':', '/') + extra
          for p in test_paths
          for extra in ['', '_' + self.test_case.split(':')[0]]
      })

    poly_case_test_suites = [
        'sizes',
        'memory.dual_browser_test',
        'memory.top_10_mobile',
        'v8:runtime_stats.top_25',
    ]
    poly_case_test_suites += yield self._GetConfiguration(
        ONE_TWO_TEST_SUITES_KEY, [])
    poly_case_test_suites += yield self._GetConfiguration(
        TWO_TWO_TEST_SUITES_KEY, [])
    if self.test_suite in poly_case_test_suites:
      raise ndb.Return(
          {p + '/' + self.test_case.replace(':', '/') for p in test_paths})

    raise ndb.Return({p + '/' + self.test_case for p in test_paths})

  def _AppendRef(self, test_paths):
    ref_test_paths = set()
    for p in test_paths:
      # A given test path will only use one of these suffixes, but there's no
      # way to know which.
      ref_test_paths.add(p + '_ref')
      ref_test_paths.add(p + '/ref')
    return ref_test_paths
