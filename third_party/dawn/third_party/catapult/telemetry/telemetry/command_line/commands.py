# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Defines the commands provided by Telemetry: Run, List."""

from __future__ import print_function
from __future__ import absolute_import
import json
import logging
import sys

from telemetry import benchmark
from telemetry.core import optparse_argparse_migration as oam
from telemetry.internal.browser import browser_finder
from telemetry.internal.browser import browser_options
from telemetry.internal import story_runner
from telemetry.util import matching


def _IsBenchmarkSupported(benchmark_, possible_browser):
  return benchmark_().CanRunOnPlatform(
      possible_browser.platform, possible_browser)


def _GetStoriesWithTags(b):
  """Finds all stories and their tags given a benchmark.

  Args:
    b: a subclass of benchmark.Benchmark
  Returns:
    A list of dicts for the stories, each of the form:
    {
      'name' : story.name
      'tags': list(story.tags)
    }
  """
  # Create a options object which hold default values that are expected
  # by Benchmark.CreateStoriesWithTags(options) method.
  parser = oam.CreateFromOptparseInputs()
  b.AddBenchmarkCommandLineArgs(parser)
  options, _ = parser.parse_args([])

  # Some benchmarks require special options, such as *.cluster_telemetry.
  # Just ignore them for now.
  try:
    story_set = b().CreateStorySet(options)
  # pylint: disable=broad-except
  except Exception as e:
    logging.warning('Unable to get stories for %s due to "%s"', b.Name(), e)
    story_set = []

  stories_info = []
  for s in story_set:
    stories_info.append({
        'name': s.name,
        'tags': sorted(list(s.tags))
    })
  return sorted(stories_info, key=lambda story: story['name'])


def PrintBenchmarkList(
    benchmarks, possible_browser, output_pipe=None,
    json_pipe=None, detailed=False):
  """Print benchmarks that are not filtered in the same order of benchmarks in
  the |benchmarks| list.

  If json_pipe is available, a json file with the following contents will be
  written:
  [
      {
          "name": <string>,
          "description": <string>,
          "enabled": <boolean>,
          "supported": <boolean>,
          "story_tags": [
              <string>,
              ...
          ]
          ...
      },
      ...
  ]

  Note that "enabled" and "supported" carry the same value. "enabled" is
  deprecated since it is misleading since a benchmark could be theoretically
  supported but have all of its stories disabled with expectations.config file.
  "supported" simply checks the benchmark's SUPPORTED_PLATFORMS member.

  Args:
    benchmarks: the list of benchmarks to be printed (in the same order of the
      list).
    possible_browser: the possible_browser instance that's used for checking
      which benchmarks are supported.
    output_pipe: the stream in which benchmarks are printed on.
    json_pipe: if available, also serialize the list into json_pipe.
    detailed: if True, list out stories for each benchmark.
  """
  if output_pipe is None:
    output_pipe = sys.stdout

  if not benchmarks:
    print('No benchmarks found!', file=output_pipe)
    if json_pipe:
      print('[]', file=json_pipe)
    return

  bad_benchmark = next((b for b in benchmarks
                        if not issubclass(b, benchmark.Benchmark)), None)
  assert bad_benchmark is None, (
      '|benchmarks| param contains non benchmark class: %s' % bad_benchmark)

  all_benchmark_info = []
  for b in benchmarks:
    benchmark_info = {'name': b.Name(), 'description': b.Description()}
    benchmark_info['supported'] = (
        not possible_browser or
        _IsBenchmarkSupported(b, possible_browser))
    benchmark_info['enabled'] = benchmark_info['supported']
    benchmark_info['stories'] = _GetStoriesWithTags(b)
    all_benchmark_info.append(benchmark_info)

  # Align the benchmark names to the longest one.
  format_string = '  %%-%ds %%s' % max(len(b['name'])
                                       for b in all_benchmark_info)

  # Sort the benchmarks by benchmark name.
  all_benchmark_info.sort(key=lambda b: b['name'])

  supported = [b for b in all_benchmark_info if b['supported']]
  if supported:
    browser_type_msg = \
      ('for %s ' % possible_browser.browser_type if possible_browser else '')
    print('Available benchmarks %sare:' % browser_type_msg, file=output_pipe)
    for b in supported:
      print(format_string % (b['name'], b['description']), file=output_pipe)
      if detailed:
        for idx, s in enumerate(b['stories']):
          print(f'    Story {idx}: {s}', file=output_pipe)

  not_supported = [b for b in all_benchmark_info if not b['supported']]
  if not_supported:
    print(('\nNot supported benchmarks for %s are (force run with -d):' %
           possible_browser.browser_type),
          file=output_pipe)
    for b in not_supported:
      print(format_string % (b['name'], b['description']), file=output_pipe)

  if not detailed:
    print('Pass --detailed to show all stories and their tags.',
          file=output_pipe)

  print('Pass --browser to list benchmarks for another browser.\n',
        file=output_pipe)

  if json_pipe:
    print(json.dumps(all_benchmark_info, indent=4,
                     sort_keys=True, separators=(',', ': ')),
          end='', file=json_pipe)


class List():
  """Lists the available benchmarks"""

  @classmethod
  def AddCommandLineArgs(cls, parser, args, environment):
    del args, environment  # Unused.
    parser.add_argument('--json',
                        dest='json_filename',
                        help='Output the list in JSON')
    parser.add_argument('--detailed',
                        action='store_true',
                        help='Print more details in the output.')

  @classmethod
  def CreateParser(cls):
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser('%prog list [benchmark_name] [<options>]')
    return parser

  @classmethod
  def ProcessCommandLineArgs(cls, parser, options, environment):
    if not options.positional_args:
      options.benchmarks = environment.GetBenchmarks()
    elif len(options.positional_args) == 1:
      options.benchmarks = _FuzzyMatchBenchmarkNames(
          options.positional_args[0], environment.GetBenchmarks())
    else:
      parser.error('Must provide at most one benchmark name.')

  def Run(self, options):
    # Set at least log info level for List command.
    # TODO(nedn): remove this once crbug.com/656224 is resolved. The recipe
    # should be change to use verbose logging instead.
    logging.getLogger().setLevel(logging.INFO)
    possible_browser = browser_finder.FindBrowser(options)
    if options.json_filename:
      with open(options.json_filename, 'w') as json_out:
        PrintBenchmarkList(options.benchmarks, possible_browser,
                           json_pipe=json_out)
    else:
      PrintBenchmarkList(options.benchmarks, possible_browser,
                         detailed=options.detailed)
    return 0


class Run():
  """Run one or more benchmarks (default)"""

  @classmethod
  def CreateParser(cls):
    options = browser_options.BrowserFinderOptions()
    parser = options.CreateParser('%prog run benchmark_name [<options>]')
    return parser

  @classmethod
  def AddCommandLineArgs(cls, parser, args, environment):
    story_runner.AddCommandLineArgs(parser)

    # Allow benchmarks to add their own command line options.
    matching_benchmarks = []
    for arg in args:
      matching_benchmark = environment.GetBenchmarkByName(arg)
      if matching_benchmark is not None:
        matching_benchmarks.append(matching_benchmark)

    if matching_benchmarks:
      # TODO(dtu): After move to argparse, add command-line args for all
      # benchmarks to subparser. Using subparsers will avoid duplicate
      # arguments.
      matching_benchmark = matching_benchmarks.pop()
      matching_benchmark.AddCommandLineArgs(parser)
      # The benchmark's options override the defaults!
      matching_benchmark.SetArgumentDefaults(parser)

  @classmethod
  def ProcessCommandLineArgs(cls, parser, options, environment):
    all_benchmarks = environment.GetBenchmarks()
    if not options.positional_args:
      possible_browser = (browser_finder.FindBrowser(options)
                          if options.browser_type else None)
      PrintBenchmarkList(all_benchmarks, possible_browser)
      parser.error('missing required argument: benchmark_name')

    benchmark_name = options.positional_args[0]
    benchmark_class = environment.GetBenchmarkByName(benchmark_name)
    if benchmark_class is None:
      most_likely_matched_benchmarks = matching.GetMostLikelyMatchedObject(
          all_benchmarks, benchmark_name, lambda x: x.Name())
      if most_likely_matched_benchmarks:
        print('Do you mean any of those benchmarks below?', file=sys.stderr)
        PrintBenchmarkList(most_likely_matched_benchmarks, None, sys.stderr)
      parser.error('no such benchmark: %s' % benchmark_name)

    if len(options.positional_args) > 1:
      parser.error(
          'unrecognized arguments: %s' % ' '.join(options.positional_args[1:]))

    assert issubclass(benchmark_class,
                      benchmark.Benchmark), ('Trying to run a non-Benchmark?!')

    story_runner.ProcessCommandLineArgs(parser, options, environment)
    benchmark_class.ProcessCommandLineArgs(parser, options)

    cls._benchmark = benchmark_class

  def Run(self, options):
    b = self._benchmark()
    return min(255, b.Run(options))


def _FuzzyMatchBenchmarkNames(benchmark_name, benchmark_classes):
  def _Matches(input_string, search_string):
    if search_string.startswith(input_string):
      return True
    for part in search_string.split('.'):
      if part.startswith(input_string):
        return True
    return False

  return [
      cls for cls in benchmark_classes if _Matches(benchmark_name, cls.Name())]
