# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import os
import tempfile
import unittest
import json
import six


from telemetry import project_config
from telemetry import decorators
from telemetry.core import util
from telemetry.internal.util import binary_manager
from telemetry.testing import run_tests
from telemetry.testing import unittest_runner

class MockArgs():
  def __init__(self):
    self.positional_args = []
    self.test_filter = ''
    self.exact_test_filter = True
    self.run_disabled_tests = False
    self.skip = []


class MockPossibleBrowser():
  def __init__(self, browser_type, os_name, os_version_name,
               supports_tab_control):
    self.browser_type = browser_type
    self.platform = MockPlatform(os_name, os_version_name)
    self.supports_tab_control = supports_tab_control

  def GetTypExpectationsTags(self):
    return []


class MockPlatform():
  def __init__(self, os_name, os_version_name):
    self.os_name = os_name
    self.os_version_name = os_version_name

  def GetOSName(self):
    return self.os_name

  def GetOSVersionName(self):
    return self.os_version_name

  def GetOSVersionDetailString(self):
    return ''


def _MakeTestFilter(tests):
  return '::'.join(tests)


class RunTestsUnitTest(unittest.TestCase):
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
    return set(successes), set(failures), set(skips)

  def _RunTest(
      self, expected_failures, expected_successes, expected_skips,
      expected_return_code=0, test_name='', extra_args=None, no_browser=True):
    extra_args = extra_args or []
    config = project_config.ProjectConfig(
        top_level_dir=os.path.join(util.GetTelemetryDir(), 'examples'),
        client_configs=[],
        benchmark_dirs=[
            os.path.join(util.GetTelemetryDir(), 'examples', 'browser_tests')]
    )
    temp_file = tempfile.NamedTemporaryFile(delete=False)
    temp_file.close()
    temp_file_name = temp_file.name
    try:
      passed_args = [
          # We don't want the underlying tests to report their results to
          # ResultDB.
          '--disable-resultsink',
          # These tests currently rely on some information sticking around
            # between tests, so we need to use the older global process pool
            # approach instead of having different pools scoped for
            # parallel/serial execution.
            '--use-global-pool',
      ]
      if test_name:
        passed_args.append(test_name)
      if no_browser:
        passed_args.append('--no-browser')
      passed_args.append('--write-full-results-to=%s' % temp_file_name)
      args = unittest_runner.ProcessConfig(config, passed_args + extra_args)
      test_runner = run_tests.RunTestsCommand()
      with binary_manager.TemporarilyReplaceBinaryManager(None):
        ret = test_runner.main(args=args)
      assert ret == expected_return_code, (
          'actual return code %d, does not equal the expected return code %d' %
          (ret, expected_return_code))
      with open(temp_file_name) as f:
        self._test_result = json.load(f)
      (actual_successes,
       actual_failures,
       actual_skips) = self._ExtractTestResults(self._test_result)

      # leave asserts below because we may miss tests
      # that are running when they are not supposed to
      self.assertEqual(set(actual_failures), set(expected_failures))
      self.assertEqual(set(actual_successes), set(expected_successes))
      self.assertEqual(set(actual_skips), set(expected_skips))
    finally:
      os.remove(temp_file_name)
    return actual_failures, actual_successes, actual_skips

  def _RunTestsWithExpectationsFile(
      self, full_test_name, expectations, test_tags='foo', extra_args=None,
      expected_exit_code=0):
    extra_args = extra_args or []
    test_expectations = (('# tags: [ foo bar mac ]\n'
                          '# results: [ {expectations} ]\n'
                          'crbug.com/123 [ {tags} ] {test} [ {expectations} ]')
                         .format(expectations=expectations, tags=test_tags,
                                 test=full_test_name))
    expectations_file = tempfile.NamedTemporaryFile(delete=False, mode='w+')
    expectations_file.write(test_expectations)
    results = tempfile.NamedTemporaryFile(delete=False)
    results.close()
    expectations_file.close()
    config = project_config.ProjectConfig(
        top_level_dir=os.path.join(util.GetTelemetryDir(), 'examples'),
        client_configs=[],
        expectations_files=[expectations_file.name],
        benchmark_dirs=[
            os.path.join(util.GetTelemetryDir(), 'examples', 'browser_tests')]
    )
    try:
      passed_args = ([full_test_name, '--no-browser',
                      ('--write-full-results-to=%s' % results.name)] +
                     ['--tag=%s' % tag for tag in test_tags.split()])
      args = unittest_runner.ProcessConfig(config, passed_args + extra_args)
      test_runner = run_tests.RunTestsCommand()
      with binary_manager.TemporarilyReplaceBinaryManager(None):
        ret = test_runner.main(args=args)
      self.assertEqual(ret, expected_exit_code)
      with open(results.name) as f:
        self._test_result = json.load(f)
    finally:
      os.remove(expectations_file.name)
      os.remove(results.name)
    return self._test_result

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testIncludeGlobsInTestFilterListWithOutBrowser(self):
    test_prefix = 'unit_tests_test.ExampleTests.%s'
    expected_failure = test_prefix % 'test_fail'
    expected_success = test_prefix % 'test_pass'
    expected_skip = test_prefix % 'test_skip'
    test_filter = _MakeTestFilter(
        [expected_failure, test_prefix % 'test_sk*', expected_success])
    self._RunTest(
        [expected_failure], [expected_success], [expected_skip],
        expected_return_code=1, extra_args=['--skip=*skip',
                                            '--test-filter=%s' % test_filter])

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testIncludeGlobsInTestFilterListWithBrowser(self):
    test_prefix = 'unit_tests_test.ExampleTests.%s'
    expected_failure = test_prefix % 'test_fail'
    expected_success = test_prefix % 'test_pass'
    expected_skip = test_prefix % 'test_skip'
    runner = run_tests.typ.Runner()
    runner.args.test_filter = _MakeTestFilter(
        [expected_failure, test_prefix % 'test_sk*', expected_success])
    runner.top_level_dirs = [os.path.join(util.GetTelemetryDir(), 'examples')]
    possible_browser = MockPossibleBrowser(
        'system', 'mac', 'mavericks', True)
    runner.classifier = run_tests.GetClassifier(runner, possible_browser)
    _, test_set = runner.find_tests(runner.args)
    self.assertEqual(len(test_set.parallel_tests), 3)
    test_names_found = [test.name for test in test_set.parallel_tests]
    self.assertIn(expected_failure, test_names_found)
    self.assertIn(expected_success, test_names_found)
    self.assertIn(expected_skip, test_names_found)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testSkipOnlyWhenTestMatchesTestFilterWithBrowser(self):
    test_name = 'unit_tests_test.ExampleTests.test_also_fail'
    runner = run_tests.typ.Runner()
    runner.args.test_filter = test_name
    runner.args.skip.append('*fail')
    runner.top_level_dirs = [os.path.join(util.GetTelemetryDir(), 'examples')]
    possible_browser = MockPossibleBrowser(
        'system', 'mac', 'mavericks', True)
    runner.classifier = run_tests.GetClassifier(runner, possible_browser)
    _, test_set = runner.find_tests(runner.args)
    self.assertEqual(len(test_set.tests_to_skip), 1)
    self.assertEqual(test_set.tests_to_skip[0].name, test_name)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testSkipOnlyWhenTestMatchesTestFilterWithoutBrowser(self):
    test_name = 'unit_tests_test.ExampleTests.test_also_fail'
    _, _, actual_skips = self._RunTest(
        [], [], [test_name],
        test_name=test_name,
        extra_args=['--skip=*fail'])
    self.assertEqual(actual_skips, {test_name})

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testTestFailsAllRetryOnFailureRetriesAndIsNotARegression(self):
    self._RunTestsWithExpectationsFile(
        'unit_tests_test.ExampleTests.test_fail', 'RetryOnFailure Failure',
        extra_args=['--retry-limit=3', '--retry-only-retry-on-failure-tests'],
        expected_exit_code=0)
    results = (self._test_result['tests']['unit_tests_test']
               ['ExampleTests']['test_fail'])
    self.assertEqual(results['actual'], 'FAIL FAIL FAIL FAIL')
    self.assertEqual(results['expected'], 'FAIL')
    self.assertNotIn('is_unexpected', results)
    self.assertNotIn('is_regression', results)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testDoNotRetryExpectedFailure(self):
    self._RunTestsWithExpectationsFile(
        'unit_tests_test.ExampleTests.test_fail', 'Failure',
        extra_args=['--retry-limit=3'])
    test_result = (self._test_result['tests']['unit_tests_test']['ExampleTests']
                   ['test_fail'])
    self.assertEqual(test_result['actual'], 'FAIL')
    self.assertEqual(test_result['expected'], 'FAIL')
    self.assertNotIn('is_unexpected', test_result)
    self.assertNotIn('is_regression', test_result)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testRetryOnFailureExpectationWithPassingTest(self):
    self._RunTestsWithExpectationsFile(
        'unit_tests_test.ExampleTests.test_pass', 'RetryOnFailure')
    test_result = (self._test_result['tests']['unit_tests_test']['ExampleTests']
                   ['test_pass'])
    self.assertEqual(test_result['actual'], 'PASS')
    self.assertEqual(test_result['expected'], 'PASS')
    self.assertNotIn('is_unexpected', test_result)
    self.assertNotIn('is_regression', test_result)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testSkipTestCmdArgNoExpectationsFile(self):
    test_name = 'unit_tests_test.ExampleTests.test_pass'
    _, _, actual_skips = self._RunTest(
        [], [], ['unit_tests_test.ExampleTests.test_pass'], test_name=test_name,
        extra_args=['--skip=*test_pass'])
    test_result = (self._test_result['tests']['unit_tests_test']
                   ['ExampleTests']['test_pass'])
    self.assertEqual(actual_skips, {test_name})
    self.assertEqual(test_result['expected'], 'SKIP')
    self.assertEqual(test_result['actual'], 'SKIP')
    self.assertNotIn('is_unexpected', test_result)
    self.assertNotIn('is_regression', test_result)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testSkipTestNoExpectationsFile(self):
    test_name = 'unit_tests_test.ExampleTests.test_skip'
    _, _, actual_skips = self._RunTest(
        [], [], [test_name], test_name=test_name)
    result = (self._test_result['tests']['unit_tests_test']
              ['ExampleTests']['test_skip'])
    self.assertEqual(actual_skips, {test_name})
    self.assertEqual(result['actual'], 'SKIP')
    self.assertEqual(result['expected'], 'SKIP')
    self.assertNotIn('is_unexpected', result)
    self.assertNotIn('is_regression', result)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testSkipTestWithExpectationsFileWithSkipExpectation(self):
    self._RunTestsWithExpectationsFile(
        'unit_tests_test.ExampleTests.test_pass', 'Skip Failure Crash')
    test_result = (self._test_result['tests']['unit_tests_test']['ExampleTests']
                   ['test_pass'])
    self.assertEqual(test_result['actual'], 'SKIP')
    self.assertEqual(test_result['expected'], 'SKIP')
    self.assertNotIn('is_unexpected', test_result)
    self.assertNotIn('is_regression', test_result)

  @decorators.Disabled('chromeos')  # crbug.com/696553
  def testSkipTestCmdArgsWithExpectationsFile(self):
    self._RunTestsWithExpectationsFile(
        'unit_tests_test.ExampleTests.test_pass', 'Crash Failure',
        extra_args=['--skip=*test_pass'])
    test_result = (self._test_result['tests']['unit_tests_test']['ExampleTests']
                   ['test_pass'])
    self.assertEqual(test_result['actual'], 'SKIP')
    self.assertEqual(test_result['expected'], 'SKIP')
    self.assertNotIn('is_unexpected', test_result)
    self.assertNotIn('is_regression', test_result)

  def _GetEnabledTests(self, browser_type, os_name, os_version_name,
                       supports_tab_control, runner=None):
    if not runner:
      runner = run_tests.typ.Runner()
    host = runner.host
    runner.top_level_dirs = [util.GetTelemetryDir()]
    runner.args.tests = [
        host.join(util.GetTelemetryDir(), 'telemetry', 'testing',
                  'disabled_cases.py')
    ]
    possible_browser = MockPossibleBrowser(
        browser_type, os_name, os_version_name, supports_tab_control)
    runner.classifier = run_tests.GetClassifier(runner, possible_browser)
    _, test_set = runner.find_tests(runner.args)
    return set(test.name.split('.')[-1] for test in test_set.parallel_tests)

  def testSystemMacMavericks(self):
    self.assertEqual(
        {
            'testAllEnabled', 'testAllEnabledVersion2', 'testMacOnly',
            'testMavericksOnly', 'testNoChromeOS', 'testNoWinLinux',
            'testSystemOnly', 'testHasTabs'
        }, self._GetEnabledTests('system', 'mac', 'mavericks', True))

  def testSystemMacLion(self):
    self.assertEqual(
        {
            'testAllEnabled', 'testAllEnabledVersion2', 'testMacOnly',
            'testNoChromeOS', 'testNoMavericks', 'testNoWinLinux',
            'testSystemOnly', 'testHasTabs'
        }, self._GetEnabledTests('system', 'mac', 'lion', True))

  def testCrosGuestChromeOS(self):
    self.assertEqual(
        {
            'testAllEnabled', 'testAllEnabledVersion2', 'testChromeOSOnly',
            'testNoMac', 'testNoMavericks', 'testNoSystem', 'testNoWinLinux',
            'testHasTabs'
        }, self._GetEnabledTests('cros-guest', 'chromeos', '', True))

  def testCanaryWindowsWin7(self):
    self.assertEqual(
        {
            'testAllEnabled', 'testAllEnabledVersion2', 'testNoChromeOS',
            'testNoMac', 'testNoMavericks', 'testNoSystem',
            'testWinOrLinuxOnly', 'testHasTabs'
        }, self._GetEnabledTests('canary', 'win', 'win7', True))

  def testDoesntHaveTabs(self):
    self.assertEqual(
        {
            'testAllEnabled', 'testAllEnabledVersion2', 'testNoChromeOS',
            'testNoMac', 'testNoMavericks', 'testNoSystem', 'testWinOrLinuxOnly'
        }, self._GetEnabledTests('canary', 'win', 'win7', False))

  def testSkip(self):
    runner = run_tests.typ.Runner()
    runner.args.skip = [
        'telemetry.*testNoMac', '*NoMavericks',
        'telemetry.testing.disabled_cases.DisabledCases.testNoSystem']
    self.assertEqual(
        {
            'testAllEnabled', 'testAllEnabledVersion2', 'testNoChromeOS',
            'testWinOrLinuxOnly', 'testHasTabs'
        }, self._GetEnabledTests('canary', 'win', 'win7', True, runner))

  def testtPostionalArgsTestFiltering(self):
    runner = run_tests.typ.Runner()
    runner.args.partial_match_filter = ['testAllEnabled']
    self.assertEqual({'testAllEnabled', 'testAllEnabledVersion2'},
                     self._GetEnabledTests('system', 'win', 'win7', True,
                                           runner))

  def testPostionalArgsTestFiltering(self):
    runner = run_tests.typ.Runner()
    runner.args.test_filter = (
        'telemetry.testing.disabled_cases.DisabledCases.testAllEnabled::'
        'telemetry.testing.disabled_cases.DisabledCases.testNoMavericks::'
        'testAllEnabledVersion2')  # Partial test name won't match
    self.assertEqual({'testAllEnabled', 'testNoMavericks'},
                     self._GetEnabledTests('system', 'win', 'win7', True,
                                           runner))
