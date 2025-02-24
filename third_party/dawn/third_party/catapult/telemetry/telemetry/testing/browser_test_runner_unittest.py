# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import string
import sys
import tempfile
import unittest
import json
import six

from telemetry import decorators
from telemetry import project_config
from telemetry.core import util
from telemetry.internal.util import binary_manager
from telemetry.testing import browser_test_context
from telemetry.testing import browser_test_runner
from telemetry.testing import options_for_unittests
from telemetry.testing import run_browser_tests
from telemetry.testing import serially_executed_browser_test_case

_expectations_template = (
    '%s'
    '# results: [ %s ]\n'
    'crbug.com/123 [ %s ] %s [ %s ]')


def _MakeTestExpectations(test_name, tag_list, expectations):
  tag_header = ''.join('# tags: [ %s ]\n' % t for t in tag_list)
  tags = ' '.join(tag_list)
  return _expectations_template % (
      tag_header, expectations, tags, test_name, expectations)


def _MakeTestFilter(tests):
  return '::'.join(tests)


class BrowserTestRunnerTest(unittest.TestCase):
  def setUp(self):
    self._test_result = {}

  def _ExtractTestResults(self, test_result):
    delimiter = test_result['path_delimiter']
    failures = []
    successes = []
    skips = []
    def _IsLeafNode(node):
      test_dict = node[1]
      return ('expected' in test_dict and
              isinstance(test_dict['expected'], six.string_types))
    node_queues = []
    for t in test_result['tests']:
      node_queues.append((t, test_result['tests'][t]))
    while node_queues:
      node = node_queues.pop()
      full_test_name, test_dict = node
      if _IsLeafNode(node):
        if all(res not in test_dict['expected'].split() for res in
               test_dict['actual'].split()):
          failures.append(full_test_name)
        elif test_dict['actual'] == 'SKIP':
          skips.append(full_test_name)
        else:
          successes.append(full_test_name)
      else:
        for k in test_dict:
          node_queues.append(
              ('%s%s%s' % (full_test_name, delimiter, k),
               test_dict[k]))
    return successes, failures, skips

  def _RunTest(
      self, test_filter, expected_failures, expected_successes,
      expected_skips=None, test_name='SimpleTest',
      expectations='', tags=None, extra_args=None):
    expected_skips = expected_skips or []
    tags = tags or []
    extra_args = extra_args or []
    config = project_config.ProjectConfig(
        top_level_dir=os.path.join(util.GetTelemetryDir(), 'examples'),
        client_configs=[],
        benchmark_dirs=[
            os.path.join(util.GetTelemetryDir(), 'examples', 'browser_tests')]
    )
    temp_file = tempfile.NamedTemporaryFile(delete=False, mode='w+')
    temp_file.close()
    temp_file_name = temp_file.name
    if expectations:
      expectations_file = tempfile.NamedTemporaryFile(delete=False, mode='w+')
      expectations_file.write(expectations)
      expectations_file.close()
      extra_args.extend(['-X', expectations_file.name] +
                        ['-x=%s' % tag for tag in tags])
    args = ([test_name,
             '--write-full-results-to=%s' % temp_file_name,
             '--test-filter=%s' % test_filter,
             # We don't want the underlying tests to report their results to
             # ResultDB.
             '--disable-resultsink',
             # These tests currently rely on some information sticking around
              # between tests, so we need to use the older global process pool
              # approach instead of having different pools scoped for
              # parallel/serial execution.
              '--use-global-pool',
            ] + extra_args)
    try:
      args = browser_test_runner.ProcessConfig(config, args)
      with binary_manager.TemporarilyReplaceBinaryManager(None):
        run_browser_tests.RunTests(args)
      with open(temp_file_name) as f:
        self._test_result = json.load(f)
      (actual_successes,
       actual_failures,
       actual_skips) = self._ExtractTestResults(self._test_result)
      self.assertEqual(set(actual_failures), set(expected_failures))
      self.assertEqual(set(actual_successes), set(expected_successes))
      self.assertEqual(set(actual_skips), set(expected_skips))
    finally:
      os.remove(temp_file_name)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testShortenTestNameUsingTestNamePrefixCommandLineArg(self):
    self._RunTest(
        test_filter='', expected_failures=[],
        expected_successes=['FailingTest'],
        test_name='ImplementsGetPlatformTags',
        expectations=_MakeTestExpectations(
            'FailingTest', ['linux', 'release'], 'Failure'),
        extra_args=['--test-name-prefix=browser_tests.browser_test.'
                    'ImplementsGetPlatformTags.'])
    test_result = (
        self._test_result['tests']['FailingTest'])
    self.assertEqual(test_result['expected'], 'FAIL')

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testShortenSkipGlobUsingTestNamePrefixCommandLineArg(self):
    self._RunTest(
        test_filter='', expected_failures=[],
        expected_successes=['a/b/fail-test.html'], expected_skips=[],
        test_name='ImplementsExpectationsFiles',
        extra_args=[
            '-x=foo', '--test-name-prefix='
            'browser_tests.browser_test.ImplementsExpectationsFiles.',
            '--skip=a/b/fail-test.html', '--all'])

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testShortenTestFilterGlobsUsingTestNamePrefixCommandLineArg(self):
    self._RunTest(
        test_filter='FailingTest', expected_failures=[],
        expected_successes=['FailingTest'],
        test_name='ImplementsGetPlatformTags',
        expectations=_MakeTestExpectations(
            'FailingTest', ['linux', 'release'], 'Failure'),
        extra_args=[
            '--test-name-prefix='
            'browser_tests.browser_test.ImplementsGetPlatformTags.'])

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testGetExpectationsFromTypWithoutExpectationsFile(self):
    test_name = ('browser_tests.browser_test.'
                 'GetsExpectationsFromTyp.HasNoExpectationsFile')
    self._RunTest(
        test_filter=test_name, expected_failures=[],
        expected_successes=[test_name], test_name='GetsExpectationsFromTyp')
    test_result = (
        self._test_result['tests']['browser_tests']['browser_test']
        ['GetsExpectationsFromTyp']['HasNoExpectationsFile'])
    self.assertEqual(test_result['expected'], 'PASS')
    self.assertEqual(test_result['actual'], 'PASS')
    self.assertNotIn('is_unexpected', test_result)
    self.assertNotIn('is_regression', test_result)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testGetExpectationsFromTypWithExpectationsFile(self):
    test_name = 'HasExpectationsFile'
    self._RunTest(
        test_filter=test_name, expected_failures=[test_name],
        expected_successes=[], test_name='GetsExpectationsFromTyp',
        expectations=_MakeTestExpectations(
            test_name, ['foo'], 'RetryOnFailure Failure'), tags=['foo'],
        extra_args=[('--test-name-prefix=browser_tests.'
                     'browser_test.GetsExpectationsFromTyp.')])
    test_result = self._test_result['tests']['HasExpectationsFile']
    self.assertEqual(test_result['expected'], 'FAIL')
    self.assertEqual(test_result['actual'], 'PASS')
    self.assertIn('is_unexpected', test_result)
    self.assertNotIn('is_regression', test_result)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testOverrideExpectationsFilesFunction(self):
    test_name = ('a/b/fail-test.html')
    self._RunTest(
        test_filter=test_name, expected_failures=[],
        expected_successes=[test_name],
        test_name='ImplementsExpectationsFiles',
        extra_args=[
            '-x=foo',
            '--test-name-prefix=browser_tests.browser_test.'
            'ImplementsExpectationsFiles.'])
    test_result = (
        self._test_result['tests']['a']['b']['fail-test.html'])
    self.assertEqual(self._test_result['path_delimiter'], '/')
    self.assertEqual(test_result['expected'], 'FAIL')
    self.assertEqual(test_result['actual'], 'FAIL')
    self.assertNotIn('is_unexpected', test_result)
    self.assertNotIn('is_regression', test_result)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testDoesRetryOnFailureRetriesAndEventuallyPasses(self):
    test_name = 'a\\b\\c\\flaky-test.html'
    extra_args = [
        '--retry-limit=3', '--retry-only-retry-on-failure-tests',
        '--test-name-prefix', 'browser_tests.browser_test.FlakyTest.']
    self._RunTest(
        test_filter=test_name, expected_failures=[],
        expected_successes=[test_name], test_name='FlakyTest',
        extra_args=extra_args, expectations=_MakeTestExpectations(
            test_name, ['foo'], 'RetryOnFailure'), tags=['foo'])
    results = (
        self._test_result['tests']['a']['b']['c']['flaky-test.html'])
    self.assertEqual(self._test_result['path_delimiter'], '\\')
    self.assertEqual(results['expected'], 'PASS')
    self.assertEqual(results['actual'], 'FAIL FAIL FAIL PASS')
    self.assertNotIn('is_unexpected', results)
    self.assertNotIn('is_regression', results)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testSkipTestWithSkipExpectation(self):
    test_name = ('browser_tests.browser_test'
                 '.TestsWillBeDisabled.SupposedToPass')
    self._RunTest(
        test_filter=test_name, expected_failures=[], expected_successes=[],
        expected_skips=[test_name], test_name='TestsWillBeDisabled',
        expectations=_MakeTestExpectations(
            test_name, ['foo'], 'Skip'), tags=['foo'])
    test_result = (
        self._test_result['tests']['browser_tests']['browser_test']
        ['TestsWillBeDisabled']['SupposedToPass'])
    self.assertEqual(test_result['expected'], 'SKIP')
    self.assertEqual(test_result['actual'], 'SKIP')
    self.assertNotIn('is_unexpected', test_result)
    self.assertNotIn('is_regression', test_result)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testSkipTestViaCommandlineArgWhilePassingExpectationsFile(self):
    test_name = ('browser_tests.browser_test'
                 '.TestsWillBeDisabled.SupposedToPass')
    self._RunTest(
        test_filter=test_name, expected_failures=[], expected_successes=[],
        expected_skips=[test_name], test_name='TestsWillBeDisabled',
        expectations=_MakeTestExpectations(
            test_name, ['foo'], 'Failure'), tags=['foo'],
        extra_args=['--skip=*SupposedToPass'])
    test_result = (
        self._test_result['tests']['browser_tests']['browser_test']
        ['TestsWillBeDisabled']['SupposedToPass'])
    self.assertEqual(test_result['expected'], 'SKIP')
    self.assertEqual(test_result['actual'], 'SKIP')
    self.assertNotIn('is_unexpected', test_result)
    self.assertNotIn('is_regression', test_result)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testSkipTestViaCommandLineArgWithoutExpectationsFile(self):
    test_name = (
        'browser_tests.browser_test.'
        'TestsWillBeDisabled.SupposedToPass')
    self._RunTest(
        test_filter=test_name, expected_failures=[], expected_successes=[],
        test_name='TestsWillBeDisabled',
        expected_skips=[test_name],
        extra_args=['--skip=*SupposedToPass'])
    test_result = (
        self._test_result['tests']['browser_tests']['browser_test']
        ['TestsWillBeDisabled']['SupposedToPass'])
    self.assertEqual(test_result['expected'], 'SKIP')
    self.assertEqual(test_result['actual'], 'SKIP')
    self.assertNotIn('is_unexpected', test_result)
    self.assertNotIn('is_regression', test_result)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testSkipTestWithoutExpectationsFile(self):
    test_name = ('browser_tests.browser_test.'
                 'TestsWillBeDisabled.ThisTestSkips')
    self._RunTest(
        test_filter=test_name, expected_failures=[], expected_successes=[],
        test_name='TestsWillBeDisabled',
        expected_skips=[test_name])
    test_result = (
        self._test_result['tests']['browser_tests']['browser_test']
        ['TestsWillBeDisabled']['ThisTestSkips'])
    self.assertEqual(test_result['expected'], 'SKIP')
    self.assertEqual(test_result['actual'], 'SKIP')
    self.assertNotIn('is_unexpected', test_result)
    self.assertNotIn('is_regression', test_result)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testOverrideGetPlatformTagsFunctionForFailureExpectations(self):
    test_name = ('browser_tests.browser_test'
                 '.ImplementsGetPlatformTags.FailingTest')
    self._RunTest(
        test_filter=test_name, expected_failures=[],
        expected_successes=[test_name],
        test_name='ImplementsGetPlatformTags',
        expectations=_MakeTestExpectations(
            test_name, ['linux', 'release'], 'Failure'))
    test_result = (
        self._test_result['tests']['browser_tests']['browser_test']
        ['ImplementsGetPlatformTags']['FailingTest'])
    self.assertEqual(test_result['expected'], 'FAIL')
    self.assertEqual(test_result['actual'], 'FAIL')

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testOverrideGetPlatformTagsFunctionForSkipExpectations(self):
    test_name = ('browser_tests.browser_test'
                 '.ImplementsGetPlatformTags.FailingTest')
    self._RunTest(
        test_filter=test_name, expected_failures=[], expected_successes=[],
        expected_skips=[test_name],
        test_name='ImplementsGetPlatformTags',
        expectations=_MakeTestExpectations(
            test_name, ['linux', 'release'], 'Skip'))
    test_result = (
        self._test_result['tests']['browser_tests']['browser_test']
        ['ImplementsGetPlatformTags']['FailingTest'])
    self.assertEqual(test_result['expected'], 'SKIP')
    self.assertEqual(test_result['actual'], 'SKIP')

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testJsonOutputFormatNegativeFilter(self):
    failures = [
        'browser_tests.simple_numeric_test.SimpleTest.add_1_and_2',
        'browser_tests.simple_numeric_test.SimpleTest.add_7_and_3',
        'browser_tests.simple_numeric_test.SimpleTest.multiplier_simple_2']
    successes = [
        'browser_tests.simple_numeric_test.SimpleTest.add_2_and_3',
        'browser_tests.simple_numeric_test.SimpleTest.multiplier_simple',
        'browser_tests.simple_numeric_test.SimpleTest.multiplier_simple_3']
    self._RunTest(
        _MakeTestFilter(failures + successes), failures, successes)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testJsonOutputWhenSetupClassFailed(self):
    failures = [
        'browser_tests.failed_tests.SetUpClassFailedTest.dummy_test_0',
        'browser_tests.failed_tests.SetUpClassFailedTest.dummy_test_1',
        'browser_tests.failed_tests.SetUpClassFailedTest.dummy_test_2']
    self._RunTest(
        _MakeTestFilter(failures), failures, [],
        test_name='SetUpClassFailedTest')

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testJsonOutputWhenTearDownClassFailed(self):
    successes = [
        'browser_tests.failed_tests.TearDownClassFailedTest.dummy_test_0',
        'browser_tests.failed_tests.TearDownClassFailedTest.dummy_test_1',
        'browser_tests.failed_tests.TearDownClassFailedTest.dummy_test_2']
    self._RunTest(
        _MakeTestFilter(successes), successes, [],
        test_name='TearDownClassFailedTest')

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testSetUpProcessCalledOnce(self):
    successes = [
        'browser_tests.process_tests.FailIfSetUpProcessCalledTwice.Dummy_0',
        'browser_tests.process_tests.FailIfSetUpProcessCalledTwice.Dummy_1',
        'browser_tests.process_tests.FailIfSetUpProcessCalledTwice.Dummy_2']
    self._RunTest(
        _MakeTestFilter(successes), [], successes,
        test_name='FailIfSetUpProcessCalledTwice')

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testTearDownProcessCalledOnce(self):
    successes = [
        'browser_tests.process_tests.FailIfTearDownProcessCalledTwice.Dummy_0',
        'browser_tests.process_tests.FailIfTearDownProcessCalledTwice.Dummy_1',
        'browser_tests.process_tests.FailIfTearDownProcessCalledTwice.Dummy_2']
    self._RunTest(
        _MakeTestFilter(successes), [], successes,
        test_name='FailIfTearDownProcessCalledTwice')

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testJsonOutputFormatPositiveFilter(self):
    failures = [
        'browser_tests.simple_numeric_test.SimpleTest.TestException',
        'browser_tests.simple_numeric_test.SimpleTest.TestSimple']
    self._RunTest(
        _MakeTestFilter(failures), failures, [])

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testExecutingTestsInSortedOrder(self):
    alphabetical_tests = []
    prefix = 'browser_tests.simple_numeric_test.SimpleTest.Alphabetical_'
    for i in range(20):
      alphabetical_tests.append(prefix + str(i))
    for c in string.ascii_uppercase[:26]:
      alphabetical_tests.append(prefix + c)
    for c in string.ascii_lowercase[:26]:
      alphabetical_tests.append(prefix + c)
    alphabetical_tests.sort()
    self._RunTest(
        prefix + '*', [], alphabetical_tests)

  def shardingRangeTestHelper(self, total_shards, num_tests):
    shard_indices = []
    for shard_index in range(0, total_shards):
      shard_indices.append(run_browser_tests._TestIndicesForShard(
          total_shards, shard_index, num_tests))
    # Make assertions about ranges
    num_tests_run = 0
    for i, cur_indices in enumerate(shard_indices):
      num_tests_in_shard = len(cur_indices)
      if i < num_tests:
        self.assertGreater(num_tests_in_shard, 0)
        num_tests_run += num_tests_in_shard
      else:
        # Not enough tests to go around all of the shards.
        self.assertEqual(num_tests_in_shard, 0)

    # Assert that we run all of the tests exactly once.
    all_indices = set()
    for cur_indices in shard_indices:
      all_indices.update(cur_indices)
    self.assertEqual(num_tests_run, num_tests)
    self.assertEqual(num_tests_run, len(all_indices))

  def testShardsWithPrimeNumTests(self):
    for total_shards in range(1, 20):
      # Nice non-prime number
      self.shardingRangeTestHelper(total_shards, 101)

  def testShardsWithDivisibleNumTests(self):
    for total_shards in range(1, 6):
      self.shardingRangeTestHelper(total_shards, 8)

  def testShardBoundaryConditions(self):
    self.shardingRangeTestHelper(1, 0)
    self.shardingRangeTestHelper(1, 1)
    self.shardingRangeTestHelper(2, 1)

  def BaseShardingTest(self, total_shards, shard_index, failures, successes,
                       opt_abbr_input_json_file=None,
                       opt_test_filter='',
                       opt_filter_tests_after_sharding=False,
                       opt_test_name_prefix=''):
    config = project_config.ProjectConfig(
        top_level_dir=os.path.join(util.GetTelemetryDir(), 'examples'),
        client_configs=[],
        benchmark_dirs=[
            os.path.join(util.GetTelemetryDir(), 'examples', 'browser_tests')]
    )
    temp_file = tempfile.NamedTemporaryFile(delete=False, mode='w+')
    temp_file.close()
    temp_file_name = temp_file.name
    opt_args = []
    if opt_abbr_input_json_file:
      opt_args += [
          '--read-abbreviated-json-results-from=%s' % opt_abbr_input_json_file]
    if opt_test_filter:
      opt_args += [
          '--test-filter=%s' % opt_test_filter]
    if opt_filter_tests_after_sharding:
      opt_args += ['--filter-tests-after-sharding']
    if opt_test_name_prefix:
      opt_args += ['--test-name-prefix=%s' % opt_test_name_prefix]
    args = (['SimpleShardingTest',
             '--write-full-results-to=%s' % temp_file_name,
             '--total-shards=%d' % total_shards,
             '--shard-index=%d' % shard_index] + opt_args)
    try:
      args = browser_test_runner.ProcessConfig(config, args)
      with binary_manager.TemporarilyReplaceBinaryManager(None):
        run_browser_tests.RunTests(args)
      with open(temp_file_name) as f:
        test_result = json.load(f)
      (actual_successes,
       actual_failures, _) = self._ExtractTestResults(test_result)
      self.assertEqual(set(actual_failures), set(failures))
      self.assertEqual(set(actual_successes), set(successes))
    finally:
      os.remove(temp_file_name)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testShardedTestRun(self):
    self.BaseShardingTest(3, 0, [], [
        'browser_tests.simple_sharding_test.SimpleShardingTest.Test1',
        'browser_tests.simple_sharding_test.SimpleShardingTest.passing_test_0',
        'browser_tests.simple_sharding_test.SimpleShardingTest.passing_test_3',
        'browser_tests.simple_sharding_test.SimpleShardingTest.passing_test_6',
        'browser_tests.simple_sharding_test.SimpleShardingTest.passing_test_9',
    ])
    self.BaseShardingTest(3, 1, [], [
        'browser_tests.simple_sharding_test.SimpleShardingTest.Test2',
        'browser_tests.simple_sharding_test.SimpleShardingTest.passing_test_1',
        'browser_tests.simple_sharding_test.SimpleShardingTest.passing_test_4',
        'browser_tests.simple_sharding_test.SimpleShardingTest.passing_test_7',
    ])
    self.BaseShardingTest(3, 2, [], [
        'browser_tests.simple_sharding_test.SimpleShardingTest.Test3',
        'browser_tests.simple_sharding_test.SimpleShardingTest.passing_test_2',
        'browser_tests.simple_sharding_test.SimpleShardingTest.passing_test_5',
        'browser_tests.simple_sharding_test.SimpleShardingTest.passing_test_8',
    ])

  def writeMockTestResultsFile(self):
    mock_test_results = {
        'passes': [
            'Test1',
            'Test2',
            'Test3',
            'passing_test_0',
            'passing_test_1',
            'passing_test_2',
            'passing_test_3',
            'passing_test_4',
            'passing_test_5',
            'passing_test_6',
            'passing_test_7',
            'passing_test_8',
            'passing_test_9',
        ],
        'failures': [],
        'valid': True,
        'times': {
            'Test1': 3.0,
            'Test2': 3.0,
            'Test3': 3.0,
            'passing_test_0': 3.0,
            'passing_test_1': 2.0,
            'passing_test_2': 2.0,
            'passing_test_3': 2.0,
            'passing_test_4': 2.0,
            'passing_test_5': 1.0,
            'passing_test_6': 1.0,
            'passing_test_7': 1.0,
            'passing_test_8': 1.0,
            'passing_test_9': 0.5,
        }
    }
    temp_file = tempfile.NamedTemporaryFile(delete=False, mode='w+')
    temp_file.close()
    temp_file_name = temp_file.name
    with open(temp_file_name, 'w') as f:
      json.dump(mock_test_results, f)
    return temp_file_name

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testSplittingShardsByTimes(self):
    temp_file_name = self.writeMockTestResultsFile()
    # Tests should be sorted first by runtime then by test name.
    # Thus, the ordered tests to be sharded should be:
    #   passing_test_0
    #   Test3-1
    #   passing_test_4-1
    #   passing_test_8-5
    #   passing_test_9
    try:
      # Shard time: 6.5
      self.BaseShardingTest(4, 0, [], [
          'browser_tests.simple_sharding_test' +
          '.SimpleShardingTest.passing_test_0',
          'browser_tests.simple_sharding_test' +
          '.SimpleShardingTest.passing_test_4',
          'browser_tests.simple_sharding_test' +
          '.SimpleShardingTest.passing_test_8',
          'browser_tests.simple_sharding_test' +
          '.SimpleShardingTest.passing_test_9'
      ], temp_file_name)
      # Shard time: 6
      self.BaseShardingTest(4, 1, [], [
          'browser_tests.simple_sharding_test' +
          '.SimpleShardingTest.Test3',
          'browser_tests.simple_sharding_test' +
          '.SimpleShardingTest.passing_test_3',
          'browser_tests.simple_sharding_test' +
          '.SimpleShardingTest.passing_test_7'
      ], temp_file_name)
      # Shard time: 6
      self.BaseShardingTest(4, 2, [], [
          'browser_tests.simple_sharding_test' +
          '.SimpleShardingTest.Test2',
          'browser_tests.simple_sharding_test' +
          '.SimpleShardingTest.passing_test_2',
          'browser_tests.simple_sharding_test' +
          '.SimpleShardingTest.passing_test_6'
      ], temp_file_name)
      # Shard time: 6
      self.BaseShardingTest(4, 3, [], [
          'browser_tests.simple_sharding_test.SimpleShardingTest.Test1',
          'browser_tests.simple_sharding_test' +
          '.SimpleShardingTest.passing_test_1',
          'browser_tests.simple_sharding_test' +
          '.SimpleShardingTest.passing_test_5'
      ], temp_file_name)
    finally:
      os.remove(temp_file_name)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testFilterTestShortenedNameAfterShardingWithoutTestTimes(self):
    self.BaseShardingTest(
        4, 0, [], ['passing_test_8'],
        opt_test_name_prefix=('browser_tests.'
                              'simple_sharding_test.SimpleShardingTest.'),
        opt_test_filter='passing_test_8',
        opt_filter_tests_after_sharding=True)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testFilterTestShortenedNameAfterShardingWithTestTimes(self):
    temp_file_name = self.writeMockTestResultsFile()
    try:
      self.BaseShardingTest(
          4, 3, [], ['passing_test_5'], temp_file_name,
          opt_test_name_prefix=('browser_tests.'
                                'simple_sharding_test.SimpleShardingTest.'),
          opt_test_filter='passing_test_5',
          opt_filter_tests_after_sharding=True)
    finally:
      os.remove(temp_file_name)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testFilteringAfterSharding(self):
    temp_file_name = self.writeMockTestResultsFile()
    successes = [
        'browser_tests.simple_sharding_test.SimpleShardingTest.Test3',
        'browser_tests.simple_sharding_test.SimpleShardingTest.passing_test_3',
        'browser_tests.simple_sharding_test.SimpleShardingTest.passing_test_7']
    try:
      self.BaseShardingTest(
          4, 1, [], successes, temp_file_name,
          opt_test_filter=_MakeTestFilter(successes),
          opt_filter_tests_after_sharding=True)
    finally:
      os.remove(temp_file_name)

  def testMedianComputation(self):
    self.assertEqual(
        2.0,
        run_browser_tests._MedianTestTime({
            'test1': 2.0,
            'test2': 7.0,
            'test3': 1.0
        }))
    self.assertEqual(2.0, run_browser_tests._MedianTestTime({'test1': 2.0}))
    self.assertEqual(0.0, run_browser_tests._MedianTestTime({}))
    self.assertEqual(4.0, run_browser_tests._MedianTestTime(
        {'test1': 2.0, 'test2': 6.0, 'test3': 1.0, 'test4': 8.0}))


class Algebra(
    serially_executed_browser_test_case.SeriallyExecutedBrowserTestCase):

  @classmethod
  def GenerateTestCases_Simple(cls, options):
    del options  # Unused.
    yield 'testOne', (1, 2)
    yield 'testTwo', (3, 3)

  def Simple(self, x, y):
    self.assertEqual(x, y)

  def TestNumber(self):
    self.assertEqual(0, 1)


class ErrorneousGeometric(
    serially_executed_browser_test_case.SeriallyExecutedBrowserTestCase):

  @classmethod
  def GenerateTestCases_Compare(cls, options):
    del options  # Unused.
    assert False, 'I am a problematic generator'
    yield 'testBasic', ('square', 'circle')

  def Compare(self, x, y):
    self.assertEqual(x, y)

  def TestAngle(self):
    self.assertEqual(90, 450)


class TestLoadAllTestModules(unittest.TestCase):
  def testLoadAllTestsInModule(self):
    context = browser_test_context.TypTestContext()
    context.finder_options = options_for_unittests.GetCopy()
    context.test_class = Algebra
    context.test_case_ids_to_run.add(
        'telemetry.testing.browser_test_runner_unittest.Algebra.TestNumber')
    context.test_case_ids_to_run.add(
        'telemetry.testing.browser_test_runner_unittest.Algebra.testOne')
    context.disable_cloud_storage_io = True
    context.Freeze()
    browser_test_context._global_test_context = context
    try:
      # This should not invoke GenerateTestCases of ErrorneousGeometric class,
      # otherwise that would throw Exception.
      tests = serially_executed_browser_test_case.LoadAllTestsInModule(
          sys.modules[__name__])
      self.assertEqual(sorted([t.id() for t in tests]), [
          'telemetry.testing.browser_test_runner_unittest.Algebra.TestNumber',
          'telemetry.testing.browser_test_runner_unittest.Algebra.testOne'
      ])
    finally:
      browser_test_context._global_test_context = None
