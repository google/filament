# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import absolute_import
import logging
import os
import sys

from telemetry.core import util
from telemetry.core import platform as platform_module
from telemetry import decorators
from telemetry.internal.browser import browser_finder
from telemetry.internal.browser import browser_finder_exceptions
from telemetry.internal.browser import browser_options
from telemetry.internal.platform import android_device
from telemetry.internal.util import binary_manager
from telemetry.internal.util import command_line
from telemetry.internal.util import ps_util
from telemetry.testing import browser_test_case
from telemetry.testing import options_for_unittests

from py_utils import cloud_storage
from py_utils import xvfb

import typ


class RunTestsCommand(command_line.OptparseCommand):
  """Run unit tests"""

  usage = (
      '\n\n  ./run_tests [test_name_1 test_name_2 ...] [<options>]\n\n'
      'You can get a list of potential test names by running\n\n'
      '  ./run_tests --list-only\n\n'
      'For a test that looks like \n'
      '"telemetry.web_perf.timeline_based_measurement_unittest.LegacyTim'
      'elineBasedMetricsTests.testDuplicateRepeatableInteractions",\n'
      'You can use any substring of it, i.e.\n\n'
      '  ./run_tests testDuplicateRepeatableInteractions\n')
  xvfb_process = None

  def __init__(self):
    super().__init__()
    self.stream = sys.stdout

  @classmethod
  def CreateParser(cls):
    options = browser_options.BrowserFinderOptions()
    options.browser_type = 'any'
    parser = options.CreateParser(cls.usage)
    return parser

  @classmethod
  def AddCommandLineArgs(cls, parser, _):
    parser.add_argument('--start-xvfb',
                        action='store_true',
                        help='Start Xvfb display if needed.')
    parser.add_argument('--disable-cloud-storage-io',
                        action='store_true',
                        help='Disable cloud storage IO.')
    parser.add_argument(
        '--no-browser',
        action='store_true',
        help='Don\'t require an actual browser to run the tests.')
    parser.add_argument('-d',
                        '--also-run-disabled-tests',
                        dest='run_disabled_tests',
                        action='store_true',
                        help='Ignore @Disabled and @Enabled restrictions.')
    parser.add_argument('--client-config',
                        dest='client_configs',
                        action='append',
                        default=[])
    parser.add_argument('--disable-logging-config',
                        action='store_true',
                        help='Configure logging (default on)')
    parser.add_argument('--use-persistent-shell',
                        action='store_true',
                        help='Uses a persistent shell adb connection when set.')
    parser.add_argument('-v',
                        '--verbose',
                        action='count',
                        dest='verbosity',
                        default=0,
                        help='Increase verbosity level (repeat as needed)')

    group = parser.add_argument_group('Options for running the tests')
    typ.ArgumentParser.add_arguments_to_parser(group,
                                               running=True,
                                               skip=['-d', '-v', '--verbose'])

    group = parser.add_argument_group('Options for reporting the results')
    typ.ArgumentParser.add_arguments_to_parser(group, reporting=True)

  @classmethod
  def ProcessCommandLineArgs(cls, parser, args, _):
    if args.verbosity >= 2:
      logging.getLogger().setLevel(logging.DEBUG)
    elif args.verbosity:
      logging.getLogger().setLevel(logging.INFO)
    else:
      logging.getLogger().setLevel(logging.WARNING)

    # We retry failures by default unless we're running a list of tests
    # explicitly.
    if not args.retry_limit and not args.positional_args:
      args.retry_limit = 3

    if args.test_filter and args.positional_args:
      parser.error(
          'Cannot specify test names in positional args and use'
          '--test-filter flag at the same time.')

    if args.no_browser:
      return

    if args.start_xvfb and xvfb.ShouldStartXvfb():
      cls.xvfb_process = xvfb.StartXvfb()
      # Work around Mesa issues on Linux. See
      # https://github.com/catapult-project/catapult/issues/3074
      args.browser_options.AppendExtraBrowserArgs('--disable-gpu')

    try:
      possible_browser = browser_finder.FindBrowser(args)
    except browser_finder_exceptions.BrowserFinderException as ex:
      parser.error(ex)

    if not possible_browser:
      parser.error('No browser found of type %s. Cannot run tests.\n'
                   'Re-run with --browser=list to see '
                   'available browser types.' % args.browser_type)

  @classmethod
  def main(cls, args=None, stream=None):  # pylint: disable=arguments-differ
    # We override the superclass so that we can hook in the 'stream' arg.
    parser = cls.CreateParser()
    cls.AddCommandLineArgs(parser, None)
    options, positional_args = parser.parse_args(args)
    options.positional_args = positional_args

    try:
      # Must initialize the DependencyManager before calling
      # browser_finder.FindBrowser(args)
      binary_manager.InitDependencyManager(options.client_configs)
      cls.ProcessCommandLineArgs(parser, options, None)

      obj = cls()
      if stream is not None:
        obj.stream = stream
      return obj.Run(options)
    finally:
      if cls.xvfb_process:
        cls.xvfb_process.kill()

  def Run(self, args):
    runner = typ.Runner()
    if self.stream:
      runner.host.stdout = self.stream
    if hasattr(args, 'disable_resultsink'):
      runner.args.disable_resultsink = args.disable_resultsink
    if hasattr(args, 'rdb_content_output_file'):
      runner.args.rdb_content_output_file = args.rdb_content_output_file
    if hasattr(args, 'use_global_pool'):
      runner.args.use_global_pool = args.use_global_pool

    if args.no_browser:
      possible_browser = None
      platform = platform_module.GetHostPlatform()
    else:
      possible_browser = browser_finder.FindBrowser(args)
      platform = possible_browser.platform

    fetch_reference_chrome_binary = False
    # Fetch all binaries needed by telemetry before we run the benchmark.
    if possible_browser and possible_browser.browser_type == 'reference':
      fetch_reference_chrome_binary = True
    binary_manager.FetchBinaryDependencies(
        platform, args.client_configs, fetch_reference_chrome_binary)

    # Telemetry seems to overload the system if we run one test per core,
    # so we scale things back a fair amount. Many of the telemetry tests
    # are long-running, so there's a limit to how much parallelism we
    # can effectively use for now anyway.
    #
    # It should be possible to handle multiple devices if we adjust the
    # browser_finder code properly, but for now we only handle one on ChromeOS.
    if platform.GetOSName() == 'chromeos':
      runner.args.jobs = 1
    elif platform.GetOSName() == 'android':
      android_devs = android_device.FindAllAvailableDevices(args)
      runner.args.jobs = len(android_devs)
      if runner.args.jobs == 0:
        raise RuntimeError("No Android device found")
      print('Running tests with %d Android device(s).' % runner.args.jobs)
    elif platform.GetOSVersionName() == 'xp':
      # For an undiagnosed reason, XP falls over with more parallelism.
      # See crbug.com/388256
      runner.args.jobs = max(int(args.jobs) // 4, 1)
    else:
      runner.args.jobs = max(int(args.jobs) // 2, 1)
    runner.args.stable_jobs = args.stable_jobs
    runner.args.expectations_files = args.expectations_files
    runner.args.tags = args.tags
    runner.args.skip = args.skip
    runner.args.metadata = args.metadata
    runner.args.passthrough = args.passthrough
    runner.args.path = args.path
    runner.args.retry_limit = args.retry_limit
    runner.args.typ_max_failures = args.typ_max_failures
    runner.args.test_results_server = args.test_results_server
    runner.args.partial_match_filter = args.positional_args
    runner.args.test_filter = args.test_filter
    runner.args.test_type = args.test_type
    runner.args.top_level_dirs = args.top_level_dirs
    runner.args.write_full_results_to = args.write_full_results_to
    runner.args.write_trace_to = args.write_trace_to
    runner.args.all = args.run_disabled_tests
    runner.args.repeat = args.repeat
    runner.args.list_only = args.list_only
    runner.args.shard_index = args.shard_index
    runner.args.test_name_prefix = args.test_name_prefix
    runner.args.total_shards = args.total_shards
    runner.args.repository_absolute_path = args.repository_absolute_path
    runner.args.retry_only_retry_on_failure_tests = (
        args.retry_only_retry_on_failure_tests)
    runner.args.path.append(util.GetUnittestDataDir())
    runner.args.quiet = args.quiet

    # Standard verbosity will only emit output on test failure. Higher verbosity
    # levels spam the output with logging, making it very difficult to figure
    # out what's going on when digging into test failures.
    runner.args.timing = True
    runner.args.verbose = 1

    runner.classifier = GetClassifier(runner, possible_browser)
    runner.context = args
    runner.setup_fn = _SetUpProcess
    runner.teardown_fn = _TearDownProcess
    runner.win_multiprocessing = typ.WinMultiprocessing.importable
    try:
      ret, _, _ = runner.run()
    except KeyboardInterrupt:
      print("interrupted, exiting", file=sys.stderr)
      ret = 130
    return ret


def GetClassifier(typ_runner, possible_browser):

  def ClassifyTestWithoutBrowser(test_set, test):
    if typ_runner.matches_filter(test):
      if typ_runner.should_skip(test):
        test_set.add_test_to_skip(
            test, 'skipped because matched --skip')
        return
      # TODO(telemetry-team): Make sure that all telemetry unittest that invokes
      # actual browser are subclasses of browser_test_case.BrowserTestCase
      # (crbug.com/537428)
      if issubclass(test.__class__, browser_test_case.BrowserTestCase):
        test_set.add_test_to_skip(
            test, 'Skip the test because it requires a browser.')
      else:
        test_set.add_test_to_run_in_parallel(test)

  def ClassifyTestWithBrowser(test_set, test):
    if typ_runner.matches_filter(test):
      if typ_runner.should_skip(test):
        test_set.add_test_to_skip(
            test, 'skipped because matched --skip')
        return
      assert hasattr(test, '_testMethodName')
      method = getattr(
          test, test._testMethodName)  # pylint: disable=protected-access
      should_skip, reason = decorators.ShouldSkip(method, possible_browser)
      if should_skip and not typ_runner.args.all:
        test_set.add_test_to_skip(test, reason)
      elif decorators.ShouldBeIsolated(method, possible_browser):
        test_set.add_test_to_run_isolated(test)
      else:
        test_set.add_test_to_run_in_parallel(test)

  if possible_browser:
    return ClassifyTestWithBrowser
  return ClassifyTestWithoutBrowser


def _SetUpProcess(child, context): # pylint: disable=unused-argument
  ps_util.EnableListingStrayProcessesUponExitHook()
  args = context
  # Make sure that we don't invoke cloud storage I/Os
  if args.disable_cloud_storage_io:
    os.environ[cloud_storage.DISABLE_CLOUD_STORAGE_IO] = '1'
  if binary_manager.NeedsInit():
    # Typ doesn't keep the DependencyManager initialization in the child
    # processes.
    binary_manager.InitDependencyManager(context.client_configs)
  # We need to reset the handlers in case some other parts of telemetry already
  # set it to make this work.
  if not args.disable_logging_config:
    logging.getLogger().handlers = []
    logging.basicConfig(
        level=logging.INFO,
        format='(%(levelname)s) %(asctime)s pid=%(process)d'
               '  %(module)s.%(funcName)s:%(lineno)d'
               '  %(message)s')
  if args.remote_platform_options.device == 'android':
    android_devices = android_device.FindAllAvailableDevices(args)
    if not android_devices:
      raise RuntimeError("No Android device found")
    android_devices.sort(key=lambda device: device.name)
    args.remote_platform_options.device = (
        android_devices[child.worker_num-1].guid)
  options_for_unittests.Push(args)


def _TearDownProcess(child, context): # pylint: disable=unused-argument
  options_for_unittests.Pop()


if __name__ == '__main__':
  ret_code = RunTestsCommand.main()
  sys.exit(ret_code)
