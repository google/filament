# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import os
import unittest
from unittest import mock

from telemetry.internal.browser import browser_options


class BrowserOptionsTest(unittest.TestCase):
  def testBrowserMultipleValues_UseLast(self):
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    parser.parse_args(['--browser=stable', '--browser=reference'])
    self.assertEqual(
        options.browser_type, 'reference',
        'Note that this test is needed for run_performance_tests.py.'
        'See crbug.com/928928.')

  def testDefaults(self):
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    parser.add_argument('-x', default=3, type=int)
    parser.parse_args(['--browser', 'any'])
    self.assertEqual(options.x, 3)  # pylint: disable=no-member

  def testDefaultsPlusOverride(self):
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    parser.add_argument('-x', default=3, type=int)
    parser.parse_args(['--browser', 'any', '-x', '10'])
    self.assertEqual(options.x, 10)  # pylint: disable=no-member

  def testDefaultsDontClobberPresetValue(self):
    options = browser_options.BrowserFinderOptions()
    setattr(options, 'x', 7)
    parser = options.CreateParser()
    parser.add_argument('-x', default=3, type=int)
    parser.parse_args(['--browser', 'any'])
    self.assertEqual(options.x, 7)  # pylint: disable=no-member

  def testCount0(self):
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    parser.add_argument('-x', action='count', dest='v')
    parser.parse_args(['--browser', 'any'])
    self.assertEqual(options.v, None)  # pylint: disable=no-member

  def testCount2(self):
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    parser.add_argument('-x', action='count', dest='v')
    parser.parse_args(['--browser', 'any', '-xx'])
    self.assertEqual(options.v, 2)  # pylint: disable=no-member

  def testOptparseMutabilityWhenSpecified(self):
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    parser.add_argument('-x', dest='verbosity', action='store_true')
    options_ret, _ = parser.parse_args(['--browser', 'any', '-x'])
    self.assertEqual(options_ret, options)
    self.assertTrue(options.verbosity)

  def testOptparseMutabilityWhenNotSpecified(self):
    options = browser_options.BrowserFinderOptions()

    parser = options.CreateParser()
    parser.add_argument('-x', dest='verbosity', action='store_true')
    options_ret, _ = parser.parse_args(['--browser', 'any'])
    self.assertEqual(options_ret, options)
    self.assertFalse(options.verbosity)

  def testProfileDirDefault(self):
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    parser.parse_args(['--browser', 'any'])
    self.assertEqual(options.browser_options.profile_dir, None)

  def testProfileDir(self):
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    # Need to use a directory that exists.
    current_dir = os.path.dirname(__file__)
    parser.parse_args(['--browser', 'any', '--profile-dir', current_dir])
    self.assertEqual(options.browser_options.profile_dir, current_dir)

  def testExtraBrowserArgs(self):
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    parser.parse_args(['--extra-browser-args=--foo --bar'])

    self.assertEqual(options.browser_options.extra_browser_args,
                     {'--foo', '--bar'})

  def testEnableSystrace(self):
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    parser.parse_args(['--enable-systrace'])

    self.assertTrue(options.enable_systrace)

  def testIncompatibleIntervalProfilingPeriods(self):
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()

    with self.assertRaises(SystemExit) as context:
      parser.parse_args(['--interval-profiling-period=story_run',
                         '--interval-profiling-period=navigation'])
    self.assertEqual(context.exception.code, 1)

  def testMergeDefaultValues(self):
    options = browser_options.BrowserFinderOptions()
    options.already_true = True
    options.already_false = False
    options.override_to_true = False
    options.override_to_false = True

    parser = options.CreateParser()
    parser.add_argument('--already_true', action='store_true')
    parser.add_argument('--already_false', action='store_true')
    parser.add_argument('--unset', action='store_true')
    parser.add_argument('--default_true', action='store_true', default=True)
    parser.add_argument('--default_false', action='store_true', default=False)
    parser.add_argument('--override_to_true',
                        action='store_true',
                        default=False)
    parser.add_argument('--override_to_false',
                        action='store_true',
                        default=True)

    options.MergeDefaultValues(parser.get_default_values())

    self.assertTrue(options.already_true)
    self.assertFalse(options.already_false)
    self.assertFalse(options.unset)
    self.assertTrue(options.default_true)
    self.assertFalse(options.default_false)
    self.assertFalse(options.override_to_true)
    self.assertTrue(options.override_to_false)

  @mock.patch('socket.getservbyname')
  def testGetServByNameOnlyCalledForRemoteCros(self, serv_mock):
    serv_mock.return_value = 22

    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    parser.parse_args(['--browser=cros-chrome'])
    serv_mock.assert_not_called()

    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    parser.parse_args(['--browser=release', '--remote=localhost'])
    serv_mock.assert_not_called()

    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    parser.parse_args(['--browser=cros-chrome', '--remote=localhost'])
    serv_mock.assert_called()
    self.assertEqual(options.remote_ssh_port, 22)

  @mock.patch('socket.getservbyname')
  def testGetServByNameNotCalledWithPortSpecified(self, serv_mock):
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    parser.parse_args(
        ['--browser=cros-chrome', '--remote=localhost', '--remote-ssh-port=22'])
    serv_mock.assert_not_called()

  @mock.patch('socket.getservbyname')
  def testSshNotAvailableHardFailsCrosRemoteTest(self, serv_mock):
    serv_mock.side_effect = OSError('No SSH here')
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    with self.assertRaises(RuntimeError):
      parser.parse_args(['--browser=cros-chrome', '--remote=localhost'])

  @mock.patch('socket.getservbyname')
  def testOriginalSshErrorIncludedInFailure(self, serv_mock):
    serv_mock.side_effect = OSError('Some unique message')
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    try:
      parser.parse_args(['--browser=cros-chrome', '--remote=localhost'])
      # Shouldn't be hit, but no assertNotReached or similar in unittest.
      self.assertTrue(False)  # pylint: disable=redundant-unittest-assert
    except RuntimeError as e:
      self.assertIn('Some unique message', str(e))

  # Regression test for crbug.com/1056281.
  def testPathsDontDropSlashes(self):
    log_file = '--log-file=%s' % os.path.join('some', 'test', 'path.txt')
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    parser.parse_args([
        '--interval-profiler-options=%s' % log_file,
        '--extra-browser-args=%s' % log_file,
        '--extra-wpr-args=%s' % log_file,
    ])

    self.assertEqual(options.interval_profiler_options, [log_file])
    self.assertEqual(options.browser_options.extra_browser_args, {log_file})
    self.assertEqual(options.browser_options.extra_wpr_args, [log_file])

  def testConsolidateValuesForArgNoOp(self):
    """Tests behavior when there is nothing to consolidate."""
    finder_options = browser_options.BrowserFinderOptions()
    # Empty args.
    finder_options.browser_options.ConsolidateValuesForArg('--to-merge')
    self.assertEqual(finder_options.browser_options.extra_browser_args, set())

    # Non-empty args, but requested arg not present.
    extra_args = ['--foo', 'A', '--bar=B']
    finder_options.AppendExtraBrowserArgs(extra_args)
    finder_options.browser_options.ConsolidateValuesForArg('--to-merge')
    self.assertEqual(finder_options.browser_options.extra_browser_args,
                     set(extra_args))

    # Requested arg is present, but only once.
    extra_args += ['--to-merge=C']
    finder_options.AppendExtraBrowserArgs(extra_args)
    finder_options.browser_options.ConsolidateValuesForArg('--to-merge')
    self.assertEqual(finder_options.browser_options.extra_browser_args,
                     set(extra_args))

  def testConsolidateValuesForArgFlagPresent(self):
    """Tests behavior when there are flag values to consolidate."""
    finder_options = browser_options.BrowserFinderOptions()
    extra_args = ['--foo', 'A', '--to-merge=C', '--bar=B', '--to-merge=D']
    finder_options.AppendExtraBrowserArgs(extra_args)
    finder_options.browser_options.ConsolidateValuesForArg('--to-merge')
    # Due to set order not being consistent, the order of the consolidated args
    # is not guaranteed.
    possible_merged_args = ('--to-merge=C,D', '--to-merge=D,C')
    possible_browser_args = [
        set(['--foo', 'A', '--bar=B'] + [pma]) for pma in possible_merged_args]
    self.assertIn(finder_options.browser_options.extra_browser_args,
                  possible_browser_args)


class ParseAndroidEmulatorOptionsTest(unittest.TestCase):
  class EarlyExitException(Exception):
    pass

  def setUp(self):
    self._old_emulator_environment =\
        browser_options.BrowserFinderOptions.emulator_environment
    browser_options.BrowserFinderOptions.emulator_environment = None
    patcher = mock.patch.object(browser_options.BrowserFinderOptions,
                                '_NoOpFunctionForTesting')
    self.addCleanup(patcher.stop)
    self.breakpoint_mock = patcher.start()
    self.breakpoint_mock.side_effect =\
        ParseAndroidEmulatorOptionsTest.EarlyExitException

  def tearDown(self):
    browser_options.BrowserFinderOptions.emulator_environment =\
        self._old_emulator_environment

  def testNoConfig(self):
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    parser.parse_args([])

  def testExistingEnvironment(self):
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    browser_options.BrowserFinderOptions.emulator_environment = True
    parser.parse_args(['--avd-config=foo'])

  @mock.patch('os.path.exists')
  def testNoBuildAndroidDir(self, exists_mock):
    exists_mock.return_value = False
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    with self.assertRaises(RuntimeError):
      parser.parse_args(['--avd-config=foo'])

  @mock.patch('os.path.exists')
  def testAllPrerequisites(self, exists_mock):
    exists_mock.return_value = True
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser()
    with self.assertRaises(ParseAndroidEmulatorOptionsTest.EarlyExitException):
      parser.parse_args(['--avd-config=foo'])
