# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import argparse
import sys
import unittest
from unittest import mock

from telemetry.command_line import parser
from telemetry.core import optparse_argparse_migration as oam
from telemetry import benchmark
from telemetry import project_config


class ParserExit(Exception):
  pass


class ParserError(Exception):
  pass


class ExampleBenchmark(benchmark.Benchmark):
  @classmethod
  def Name(cls):
    return 'example_benchmark'


class ParseArgsTests(unittest.TestCase):
  def setUp(self):
    # TODO(crbug.com/981349): Ideally parsing args should not have any side
    # effects; for now we need to mock out calls to set up logging, binary
    # manager, and browser finding logic.
    mock.patch('telemetry.command_line.parser.logging').start()
    mock.patch('telemetry.command_line.parser.binary_manager').start()
    mock.patch('telemetry.command_line.commands.browser_finder').start()

    mock.patch.object(
        argparse.ArgumentParser, 'exit', side_effect=ParserExit).start()
    mock.patch.object(
        oam.ArgumentParser, 'exit', side_effect=ParserExit).start()
    self._argparse_error = mock.patch.object(
        argparse.ArgumentParser, 'error', side_effect=ParserError).start()
    self._optparse_error = mock.patch.object(
        oam.ArgumentParser, 'error', side_effect=ParserError).start()

    self.benchmarks = [ExampleBenchmark]
    def find_by_name(name):
      return next((b for b in self.benchmarks if b.Name() == name), None)

    self.mock_config = mock.Mock(spec=project_config.ProjectConfig)
    self.mock_config.GetBenchmarks.return_value = self.benchmarks
    self.mock_config.GetBenchmarkByName.side_effect = find_by_name
    self.mock_config.expectations_files = []

  def tearDown(self):
    mock.patch.stopall()

  def testHelpFlag(self):
    with self.assertRaises(ParserExit):
      parser.ParseArgs(self.mock_config, ['--help'])
    self.assertIn('Command line tool to run performance benchmarks.',
                  sys.stdout.getvalue())

  def testHelpCommand(self):
    with self.assertRaises(ParserExit):
      parser.ParseArgs(self.mock_config, ['help', 'run'])
    self.assertIn('To get help about a command use', sys.stdout.getvalue())

  def testRunHelp(self):
    with self.assertRaises(ParserExit):
      parser.ParseArgs(self.mock_config, ['run', '--help'])
    self.assertIn('--browser {list,any,', sys.stdout.getvalue())

  def testRunBenchmarkHelp(self):
    with self.assertRaises(ParserExit):
      parser.ParseArgs(self.mock_config, ['example_benchmark', '--help'])
    self.assertIn('--browser {list,any,', sys.stdout.getvalue())

  def testListBenchmarks(self):
    args = parser.ParseArgs(self.mock_config, ['list', '--json', 'output.json'])
    self.assertEqual(args.command, 'list')
    self.assertEqual(args.json_filename, 'output.json')

  def testRunBenchmark(self):
    args = parser.ParseArgs(self.mock_config, [
        'run', 'example_benchmark', '--browser=stable'])
    self.assertEqual(args.command, 'run')
    self.assertEqual(args.positional_args, ['example_benchmark'])
    self.assertEqual(args.browser_type, 'stable')

  def testRunCommandIsDefault(self):
    args = parser.ParseArgs(self.mock_config, [
        'example_benchmark', '--browser', 'stable'])
    self.assertEqual(args.command, 'run')
    self.assertEqual(args.positional_args, ['example_benchmark'])
    self.assertEqual(args.browser_type, 'stable')

  def testRunCommandBenchmarkNameAtEnd(self):
    args = parser.ParseArgs(self.mock_config, [
        '--browser', 'stable', 'example_benchmark'])
    self.assertEqual(args.command, 'run')
    self.assertEqual(args.positional_args, ['example_benchmark'])
    self.assertEqual(args.browser_type, 'stable')

  def testRunBenchmark_UnknownBenchmark(self):
    with self.assertRaises(ParserError):
      parser.ParseArgs(self.mock_config, [
          'run', 'foo.benchmark', '--browser=stable'])
    self._optparse_error.assert_called_with(
        'no such benchmark: foo.benchmark')

  def testRunBenchmark_MissingBenchmark(self):
    with self.assertRaises(ParserError):
      parser.ParseArgs(self.mock_config, ['run', '--browser=stable'])
    self._optparse_error.assert_called_with(
        'missing required argument: benchmark_name')

  def testRunBenchmark_TooManyArgs(self):
    with self.assertRaises(ParserError):
      parser.ParseArgs(self.mock_config, [
          'run', 'example_benchmark', 'other', '--browser=beta', 'args'])
    self._optparse_error.assert_called_with(
        'unrecognized arguments: other args')

  def testRunBenchmark_UnknownArg(self):
    with self.assertRaises(ParserError):
      parser.ParseArgs(self.mock_config, [
          'run', 'example_benchmark', '--non-existent-option'])
    self._optparse_error.assert_called_with(
        'no such option: --non-existent-option')

  def testRunBenchmark_WithCustomOptionDefaults(self):
    class BenchmarkWithCustomDefaults(benchmark.Benchmark):
      options = {'upload_results': True}

      @classmethod
      def Name(cls):
        return 'custom_benchmark'

    self.benchmarks.append(BenchmarkWithCustomDefaults)
    args = parser.ParseArgs(self.mock_config, [
        'custom_benchmark', '--browser', 'stable'])
    self.assertTrue(args.upload_results)
    self.assertEqual(args.positional_args, ['custom_benchmark'])

  def testRunBenchmark_ExternalOption(self):
    my_parser = argparse.ArgumentParser(add_help=False)
    my_parser.add_argument('--extra-special-option', action='store_true')

    args = parser.ParseArgs(
        self.mock_config,
        ['run', 'example_benchmark', '--extra-special-option'],
        results_arg_parser=my_parser)
    self.assertEqual(args.command, 'run')
    self.assertEqual(args.positional_args, ['example_benchmark'])
    self.assertTrue(args.extra_special_option)

  def testListBenchmarks_NoExternalOptions(self):
    my_parser = argparse.ArgumentParser(add_help=False)
    my_parser.add_argument('--extra-special-option', action='store_true')

    with self.assertRaises(ParserError):
      # Listing benchmarks does not require the external results processor.
      parser.ParseArgs(
          self.mock_config, ['list', '--extra-special-option'],
          results_arg_parser=my_parser)
    self._optparse_error.assert_called_with(
        'no such option: --extra-special-option')

  def testRunBenchmark_WithExternalHelp(self):
    """Test `run --help` message includes both our and external options."""
    my_parser = argparse.ArgumentParser(add_help=False)
    my_parser.add_argument('--extra-special-option', action='store_true')

    with self.assertRaises(ParserExit):
      parser.ParseArgs(
          self.mock_config, ['run', '--help'], results_arg_parser=my_parser)
    self.assertIn('--browser {list,any,', sys.stdout.getvalue())
    self.assertIn('--extra-special-option', sys.stdout.getvalue())

  def testListBenchmarks_WithExternalHelp(self):
    """Test `list --help` message does not include external options."""
    my_parser = argparse.ArgumentParser(add_help=False)
    my_parser.add_argument('--extra-special-option', action='store_true')

    with self.assertRaises(ParserExit):
      parser.ParseArgs(
          self.mock_config, ['list', '--help'], results_arg_parser=my_parser)
    self.assertIn('--browser {list,any,', sys.stdout.getvalue())
    self.assertNotIn('--extra-special-option', sys.stdout.getvalue())
