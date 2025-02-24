# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import
import os
import sys
import json

from telemetry.internal.browser import browser_options
from telemetry.internal.platform import android_device
from telemetry.internal.util import binary_manager
from telemetry.testing import browser_test_context
from telemetry.testing import serially_executed_browser_test_case

from py_utils import cloud_storage
from py_utils import discover

import typ
from typ import arg_parser

TEST_SUFFIXES = ['*_test.py', '*_tests.py', '*_unittest.py', '*_unittests.py']


def PrintTelemetryHelp():
  options = browser_options.BrowserFinderOptions()
  options.browser_type = 'any'
  parser = options.CreateParser()
  print('\n\nCommand line arguments handled by Telemetry:')
  parser.print_help()


def ProcessCommandLineOptions(test_class, typ_options, args):
  options = browser_options.BrowserFinderOptions()
  options.browser_type = 'any'
  parser = options.CreateParser(test_class.__doc__)
  test_class.AddCommandlineArgs(parser)
  # Set the default chrome root variable. This is required for the
  # Android browser finder to function properly.
  if typ_options.default_chrome_root:
    parser.set_defaults(chrome_root=typ_options.default_chrome_root)
  finder_options, positional_args = parser.parse_args(args)
  finder_options.positional_args = positional_args
  # Typ parses the "verbose", or "-v", command line arguments which
  # are supposed to control logging verbosity. Carry them over.
  finder_options.verbosity = typ_options.verbose
  return finder_options


def _ValidateDistinctNames(browser_test_classes):
  names_to_test_classes = {}
  for cl in browser_test_classes:
    name = cl.Name()
    if name in names_to_test_classes:
      raise Exception('Test name %s is duplicated between %s and %s' % (
          name, repr(cl), repr(names_to_test_classes[name])))
    names_to_test_classes[name] = cl


def _TestIndicesForShard(total_shards, shard_index, num_tests):
  """Returns indices of tests to run for a given shard.

  This methods returns every Nth index, where N is the number of shards. We
  intentionally avoid picking sequential runs of N tests, since that will pick
  groups of related tests, which can skew runtimes. See
  https://crbug.com/1028298.
  """
  return list(range(shard_index, num_tests, total_shards))

def _MedianTestTime(test_times):
  times = sorted(test_times.values())
  if len(times) == 0:
    return 0
  halfLen = len(times) // 2
  if len(times) % 2:
    return times[halfLen]
  return 0.5 * (times[halfLen - 1] + times[halfLen])


def _TestTime(test, test_times, default_test_time):
  return test_times.get(test.shortName()) or default_test_time


def _DebugShardDistributions(shards, test_times):
  for i, s in enumerate(shards):
    num_tests = len(s)
    if test_times:
      median = _MedianTestTime(test_times)
      shard_time = 0.0
      for t in s:
        shard_time += _TestTime(t, test_times, median)
      print('shard %d: %d seconds (%d tests)' % (i, shard_time, num_tests))
    else:
      print('shard %d: %d tests (unknown duration)' % (i, num_tests))


def _SplitShardsByTime(test_cases, total_shards, test_times,
                       debug_shard_distributions):
  median = _MedianTestTime(test_times)
  shards = []
  for i in range(total_shards):
    shards.append({'total_time': 0.0, 'tests': []})
  # Sort by test time first, falling back to the test name in the case of a tie
  # so that ordering is consistent across all shards.
  test_cases.sort(key=lambda t: (_TestTime(t, test_times, median),
                                 t.shortName()),
                  reverse=True)

  # The greedy algorithm has been empirically tested on the WebGL 2.0
  # conformance tests' times, and results in an essentially perfect
  # shard distribution of 530 seconds per shard. In the same scenario,
  # round-robin scheduling resulted in shard times spread between 502
  # and 592 seconds, and the current alphabetical sharding resulted in
  # shard times spread between 44 and 1591 seconds.

  # Greedy scheduling. O(m*n), where m is the number of shards and n
  # is the number of test cases.
  for t in test_cases:
    min_shard_index = 0
    min_shard_time = None
    for i in range(total_shards):
      if min_shard_time is None or shards[i]['total_time'] < min_shard_time:
        min_shard_index = i
        min_shard_time = shards[i]['total_time']
    shards[min_shard_index]['tests'].append(t)
    shards[min_shard_index]['total_time'] += _TestTime(t, test_times, median)

  res = [s['tests'] for s in shards]
  if debug_shard_distributions:
    _DebugShardDistributions(res, test_times)

  return res


def LoadTestCasesToBeRun(
    test_class, finder_options, filter_tests_after_sharding,
    total_shards, shard_index, test_times, debug_shard_distributions,
    typ_runner):
  test_cases = []
  match_everything = lambda _: True
  test_filter_matcher_func = typ_runner.matches_filter
  if filter_tests_after_sharding:
    test_filter_matcher = match_everything
    post_test_filter_matcher = test_filter_matcher_func
  else:
    test_filter_matcher = test_filter_matcher_func
    post_test_filter_matcher = match_everything

  for t in serially_executed_browser_test_case.GenerateTestCases(
      test_class, finder_options):
    if test_filter_matcher(t):
      test_cases.append(t)
  if test_times:
    # Assign tests to shards.
    shards = _SplitShardsByTime(test_cases, total_shards, test_times,
                                debug_shard_distributions)
    return [t for t in shards[shard_index]
            if post_test_filter_matcher(t)]
  test_cases.sort(key=lambda t: t.shortName())
  test_cases = [t for t in test_cases if post_test_filter_matcher(t)]
  test_indices = _TestIndicesForShard(total_shards, shard_index,
                                      len(test_cases))
  if debug_shard_distributions:
    tmp_shards = []
    for i in range(total_shards):
      tmp_indices = _TestIndicesForShard(total_shards, i, len(test_cases))
      tmp_tests = [test_cases[index] for index in tmp_indices]
      tmp_shards.append(tmp_tests)
    # Can edit the code to get 'test_times' passed in here for
    # debugging and comparison purposes.
    _DebugShardDistributions(tmp_shards, None)
  return [test_cases[index] for index in test_indices]


def _RemoveArgFromParser(parser, arg):
  # argparse doesn't have a simple way to remove an argument after it has been
  # added, so use the logic from https://stackoverflow.com/a/49753634.
  # pylint: disable=protected-access
  for action in parser._actions:
    option_strings = action.option_strings
    if (option_strings and option_strings[0] == arg
        or action.dest == arg):
      parser._remove_action(action)
      break
  for action in parser._action_groups:
    for group_action in action._group_actions:
      option_strings = group_action.option_strings
      if (option_strings and option_strings[0] == arg
          or group_action.dest == arg):
        action._group_actions.remove(group_action)
        return
  # pylint: enable=protected-access


def _CreateTestArgParsers():
  parser = typ.ArgumentParser(discovery=True, reporting=True, running=True)
  parser.add_argument('test', type=str, help='Name of the test suite to run')

  # We remove typ's "tests" positional argument because we don't use it/pass it
  # on to typ AND it can result in unexpected behavior, particularly if the
  # underlying test suite does its own argument parsing. As a concrete example,
  # consider the args:
  #   test_suite "--extra-browser-args=--foo --bar"
  # In this case, --extra-browser-args is only known to the underlying suite,
  # not to run_browser_test.py or typ. A user would expect this to run the
  # test_suite suite and pass the --extra-browser-args on to the suite. However,
  # if both "tests" and "test" are added to the parser, then we end up with
  # tests=['test_suite'] and test='--extra-browser-args ...' since
  # --extra-browser-args looks like a positional argument to the parser. This
  # can be worked around by having a known argument immediately after the suite,
  # e.g.
  #   test_suite --jobs 1 "--extra-browser-args=--foo --bar"
  # but simply removing "tests" here works around the issue entirely.
  _RemoveArgFromParser(parser, 'tests')

  parser.add_argument(
      '--filter-tests-after-sharding', default=False, action='store_true',
      help=('Apply the test filter after tests are split for sharding. Useful '
            'for reproducing bugs related to the order in which tests run.'))
  parser.add_argument(
      '--read-abbreviated-json-results-from',
      metavar='FILENAME',
      action='store',
      help=(
          'If specified, reads abbreviated results from that path in json '
          'form. This information is used to more evenly distribute tests '
          'among shards.'))
  parser.add_argument(
      '--debug-shard-distributions',
      action='store_true', default=False,
      help='Print debugging information about the shards\' test distributions')
  parser.add_argument(
      '--disable-cloud-storage-io',
      action='store_true', default=False,
      help=('Disable cloud storage IO.'))

  parser.add_argument('--default-chrome-root', type=str, default=None)
  parser.add_argument(
      '--client-config', dest='client_configs', action='append', default=[])
  parser.add_argument(
      '--start-dir', dest='start_dirs', action='append', default=[])
  return parser


def _GetClassifier(typ_runner):
  def _SeriallyExecutedBrowserTestCaseClassifer(test_set, test):
    # Do not pick up tests that do not inherit from
    # serially_executed_browser_test_case.SeriallyExecutedBrowserTestCase
    # class.
    if not isinstance(
        test,
        serially_executed_browser_test_case.SeriallyExecutedBrowserTestCase):
      return
    if typ_runner.should_skip(test):
      test_set.add_test_to_skip(test, 'skipped because matched --skip')
      return
    # Default to running the test in isolation unless it has specifically opted
    # in to parallel execution.
    if test.CanRunInParallel():
      test_set.add_test_to_run_in_parallel(test)
    else:
      test_set.add_test_to_run_isolated(test)
  return _SeriallyExecutedBrowserTestCaseClassifer


def RunTests(args):
  parser = _CreateTestArgParsers()
  try:
    options, extra_args = parser.parse_known_args(args)
  except arg_parser._Bailout:
    PrintTelemetryHelp()
    return parser.exit_status
  binary_manager.InitDependencyManager(options.client_configs)
  for start_dir in options.start_dirs:
    modules_to_classes = discover.DiscoverClasses(
        start_dir,
        options.top_level_dir,
        base_class=serially_executed_browser_test_case.
        SeriallyExecutedBrowserTestCase)
    browser_test_classes = list(modules_to_classes.values())

  _ValidateDistinctNames(browser_test_classes)

  test_class = None
  for cl in browser_test_classes:
    if cl.Name() == options.test:
      test_class = cl
      break

  if not test_class:
    print('Cannot find test class with name matching %s' % options.test)
    print('Available tests: %s' % '\n'.join(
        cl.Name() for cl in browser_test_classes))
    return 1

  typ_runner = typ.Runner()

  # Create test context.
  typ_runner.context = browser_test_context.TypTestContext()
  for c in options.client_configs:
    typ_runner.context.client_configs.append(c)
  typ_runner.context.finder_options = ProcessCommandLineOptions(
      test_class, options, extra_args)
  typ_runner.context.test_class = test_class
  typ_runner.context.expectations_files = options.expectations_files
  typ_runner.context.disable_cloud_storage_io = options.disable_cloud_storage_io
  test_times = None
  if options.read_abbreviated_json_results_from:
    with open(options.read_abbreviated_json_results_from, 'r') as f:
      abbr_results = json.load(f)
      test_times = abbr_results.get('times')

  # Setup typ.Runner instance.
  typ_runner.args.all = options.all
  typ_runner.args.expectations_files = options.expectations_files
  typ_runner.args.jobs = options.jobs
  typ_runner.args.stable_jobs = options.stable_jobs
  typ_runner.args.list_only = options.list_only
  typ_runner.args.metadata = options.metadata
  typ_runner.args.passthrough = options.passthrough
  typ_runner.args.path = options.path
  typ_runner.args.quiet = options.quiet
  typ_runner.args.repeat = options.repeat
  typ_runner.args.repository_absolute_path = options.repository_absolute_path
  typ_runner.args.retry_limit = options.retry_limit
  typ_runner.args.retry_only_retry_on_failure_tests = (
      options.retry_only_retry_on_failure_tests)
  typ_runner.args.typ_max_failures = options.typ_max_failures
  typ_runner.args.skip = options.skip
  typ_runner.args.suffixes = TEST_SUFFIXES
  typ_runner.args.tags = options.tags
  typ_runner.args.test_name_prefix = options.test_name_prefix
  typ_runner.args.test_filter = options.test_filter
  typ_runner.args.test_results_server = options.test_results_server
  typ_runner.args.test_type = options.test_type
  typ_runner.args.top_level_dir = options.top_level_dir
  typ_runner.args.write_full_results_to = options.write_full_results_to
  typ_runner.args.write_trace_to = options.write_trace_to
  typ_runner.args.disable_resultsink = options.disable_resultsink
  typ_runner.args.rdb_content_output_file = options.rdb_content_output_file
  typ_runner.args.use_global_pool = options.use_global_pool

  typ_runner.classifier = _GetClassifier(typ_runner)
  typ_runner.path_delimiter = test_class.GetJSONResultsDelimiter()
  typ_runner.setup_fn = _SetUpProcess
  typ_runner.teardown_fn = _TearDownProcess
  typ_runner.tag_conflict_checker = test_class.GetTagConflictChecker()

  tests_to_run = LoadTestCasesToBeRun(
      test_class=test_class, finder_options=typ_runner.context.finder_options,
      filter_tests_after_sharding=options.filter_tests_after_sharding,
      total_shards=options.total_shards, shard_index=options.shard_index,
      test_times=test_times,
      debug_shard_distributions=options.debug_shard_distributions,
      typ_runner=typ_runner)
  for t in tests_to_run:
    typ_runner.context.test_case_ids_to_run.add(t.id())
  typ_runner.context.Freeze()
  # pylint: disable=protected-access
  browser_test_context._global_test_context = typ_runner.context
  # pylint: enable=protected-access

  # several class level variables are set for GPU tests  when
  # LoadTestCasesToBeRun is called. Functions line ExpectationsFiles and
  # GenerateTags which use these variables should be called after
  # LoadTestCasesToBeRun

  test_class_expectations_files = test_class.ExpectationsFiles()
  # all file paths in test_class_expectations-files must be absolute
  assert all(os.path.isabs(path) for path in test_class_expectations_files)
  typ_runner.args.expectations_files.extend(
      test_class_expectations_files)
  typ_runner.args.ignored_tags.extend(test_class.IgnoredTags())

  # Since sharding logic is handled by browser_test_runner harness by passing
  # browser_test_context.test_case_ids_to_run to subprocess to indicate test
  # cases to be run, we explicitly disable sharding logic in typ.
  typ_runner.args.total_shards = 1
  typ_runner.args.shard_index = 0

  typ_runner.args.timing = True
  typ_runner.args.verbose = options.verbose
  typ_runner.win_multiprocessing = typ.WinMultiprocessing.importable

  try:
    ret, _, _ = typ_runner.run()
  except KeyboardInterrupt:
    print("interrupted, exiting", file=sys.stderr)
    ret = 130
  return ret


def _SetUpProcess(child, context):
  args = context.finder_options
  # Make sure that we don't invoke cloud storage I/Os
  if context.disable_cloud_storage_io:
    os.environ[cloud_storage.DISABLE_CLOUD_STORAGE_IO] = '1'
  if binary_manager.NeedsInit():
    # On windows, typ doesn't keep the DependencyManager initialization in the
    # child processes.
    binary_manager.InitDependencyManager(context.client_configs)
  if args.remote_platform_options.device == 'android':
    android_devices = android_device.FindAllAvailableDevices(args)
    if not android_devices:
      raise RuntimeError("No Android device found")
    android_devices.sort(key=lambda device: device.name)
    args.remote_platform_options.device = (
        android_devices[child.worker_num-1].guid)
  # pylint: disable=protected-access
  browser_test_context._global_test_context = context
  # pylint: enable=protected-access
  # typ will set this later as well, but set it earlier so that it's available
  # in the test class process setup.
  context.test_class.child = child
  context.test_class.SetUpProcess()


def _TearDownProcess(child, context):
  del child, context  # Unused.
  # pylint: disable=protected-access
  browser_test_context._global_test_context.test_class.TearDownProcess()
  browser_test_context._global_test_context = None
  # pylint: enable=protected-access


if __name__ == '__main__':
  ret_code = RunTests(sys.argv[1:])
  sys.exit(ret_code)
