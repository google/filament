# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Builds the command line parser, processes args, and runs commands."""

from __future__ import absolute_import
import argparse
import logging
import sys

from py_utils.constants import exit_codes

from telemetry.command_line import commands
from telemetry.command_line import utils
from telemetry.internal.util import binary_manager
from telemetry.internal.util import ps_util


DEFAULT_LOG_FORMAT = (
    '(%(levelname)s) %(asctime)s %(module)s.%(funcName)s:%(lineno)d  '
    '%(message)s')


_COMMANDS = {
    'run': commands.Run,
    'list': commands.List,
}


def _ArgumentParsers(environment, args, results_arg_parser):
  """Build the top level argument parser.

  Currently this only defines two (mostly) empty parsers for 'run' and 'list'
  commands. After the selected command is parsed from the command line,
  remaining unknown args may be passed to the respective legacy opt_parser of
  the chosen command.

  TODO(crbug.com/981349): Other options should be migrated away from optparse
  and registered here instead using the corresponding argparse methods.

  Args:
    environment: A ProjectConfig object with information about the benchmark
      runtime environment.
    results_arg_parser: An optional parser defining extra command line options
      for an external results_processor. These are appended to the options of
      the 'run' command.

  Returns:
    A tuple with:
    - An argparse.ArgumentParser object, the top level arg_parser.
    - A dictionary mapping command names to their respective legacy opt_parsers.
  """
  if results_arg_parser is not None:
    ext_defaults = vars(results_arg_parser.parse_args([]))
    ext_defaults['external_results_processor'] = True
  else:
    ext_defaults = {'external_results_processor': False}

  # Build the legacy parser for each available Telemetry command.
  legacy_parsers = {}
  for name, command in _COMMANDS.items():
    opt_parser = command.CreateParser()
    # The Run.AddCommandLineArgs method can also let benchmarks adjust the
    # default values of options coming from the external results_arg_parser.
    # So here we need first to let the opt_parser know about these options and
    # then, after any adjusments were done, copy the defaults back so we can
    # feed them into the top level parser, as that is the one actually doing
    # the parsing of those args.
    # TODO(crbug.com/985712): Figure out a way to simplify this logic.
    if name == 'run':
      opt_parser.set_defaults(**ext_defaults)
    command.AddCommandLineArgs(opt_parser, args, environment)
    if name == 'run':
      ext_defaults.update((k, opt_parser.defaults[k]) for k in ext_defaults)
    legacy_parsers[name] = opt_parser

  # Build the top level argument parser.
  parser = argparse.ArgumentParser(
      description='Command line tool to run performance benchmarks.',
      epilog='To get help about a command use e.g.: %(prog)s run --help')
  subparsers = parser.add_subparsers(dest='command', title='commands')
  subparsers.required = True

  def add_subparser(name, **kwargs):
    kwargs.update(usage=argparse.SUPPRESS,
                  description=argparse.SUPPRESS,
                  add_help=False)
    subparser = subparsers.add_parser(name, **kwargs)
    subparser.add_argument('-h', '--help', action=utils.MixedHelpAction,
                           legacy_parser=legacy_parsers[name])
    return subparser

  subparser = add_subparser(
      'run', help='run a benchmark (default)',
      parents=[results_arg_parser] if results_arg_parser else [])
  subparser.set_defaults(**ext_defaults)
  subparser.add_argument('--use-local-wpr', action='store_true',
                         help='Builds and runs WPR from Catapult. '
                         'Also enables WPR debug output to STDOUT.')
  subparser.add_argument('--disable-fuzzy-url-matching', action='store_true',
                         help='Requires WPR to exactly match URLs.')
  add_subparser(
      'list', help='list benchmarks or stories')

  return parser, legacy_parsers


def ParseArgs(environment, args=None, results_arg_parser=None):
  """Parse command line arguments.

  Args:
    environment: A ProjectConfig object with information about the benchmark
      runtime environment.
    args: An optional list of arguments to parse. Defaults to obtain the
      arguments from sys.argv.
    results_arg_parser: An optional parser defining extra command line options
      for an external results_processor.

  Returns:
    An options object with the values parsed from the command line.
  """
  if args is None:
    args = sys.argv[1:]
  if len(args) > 0:
    if args[0] == 'help':
      # The old command line allowed "help" as a command. Now just translate
      # to "--help" to teach users about the new interface.
      args[0] = '--help'
    elif args[0] not in ['list', 'run', '-h', '--help']:
      args.insert(0, 'run')  # Default command.

  # TODO(crbug.com/981349): When optparse is gone, this should just call
  # parse_args on the fully formed top level parser. For now we still need
  # to allow unknown args, which are then passed below to the legacy parsers.
  parser, legacy_parsers = _ArgumentParsers(environment, args,
                                            results_arg_parser)
  parsed_args, unknown_args = parser.parse_known_args(args)

  # TODO(crbug.com/981349): Ideally, most of the following should be moved
  # to after argument parsing is completed and before (or at the time) when
  # arguments are processed.

  # The log level is set in browser_options.
  # Clear the log handlers to ensure we can set up logging properly here.
  logging.getLogger().handlers = []
  logging.basicConfig(format=DEFAULT_LOG_FORMAT)

  binary_manager.InitDependencyManager(environment.client_configs)

  command = _COMMANDS[parsed_args.command]
  opt_parser = legacy_parsers[parsed_args.command]

  # Set the default chrome root variable.
  opt_parser.set_defaults(chrome_root=environment.default_chrome_root)

  options, positional_args = opt_parser.parse_args(unknown_args)
  options.positional_args = positional_args
  command.ProcessCommandLineArgs(opt_parser, options, environment)
  options.browser_options.environment = environment

  # Merge back our argparse args with the optparse options.
  for arg in vars(parsed_args):
    setattr(options, arg, getattr(parsed_args, arg))
  return options


def RunCommand(options):
  """Run a selected command from parsed command line args.

  Args:
    options: The return value from ParseArgs.

  Returns:
    The exit_code from the command execution.
  """
  ps_util.EnableListingStrayProcessesUponExitHook()
  return_code = _COMMANDS[options.command]().Run(options)
  if return_code == exit_codes.ALL_TESTS_SKIPPED:
    logging.warning('No stories were run.')
  return return_code


def main(environment):
  """Parse command line arguments and immediately run the selected command."""
  options = ParseArgs(environment)
  return RunCommand(options)
