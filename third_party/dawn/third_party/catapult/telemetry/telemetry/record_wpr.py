# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import argparse
import logging
import shutil
import sys
import tempfile
import six

from telemetry import benchmark
from telemetry import story
from telemetry.internal.browser import browser_options
from telemetry.internal.results import results_options
from telemetry.internal import story_runner
from telemetry.internal.util import binary_manager
from telemetry.page import legacy_page_test
from telemetry.util import matching
from telemetry.util import wpr_modes

from py_utils import discover
import py_utils

DEFAULT_LOG_FORMAT = (
    '(%(levelname)s) %(asctime)s %(module)s.%(funcName)s:%(lineno)d  '
    '%(message)s')


class RecorderPageTest(legacy_page_test.LegacyPageTest):
  def __init__(self, page_test):
    super().__init__()
    self._page_test = page_test
    self._platform = None

  @property
  def platform(self):
    return self._platform

  def CustomizeBrowserOptions(self, options):
    if self._page_test:
      self._page_test.CustomizeBrowserOptions(options)

  def WillStartBrowser(self, platform):
    if self.platform is not None:
      assert platform.GetOSName() == self.platform
    self._platform = platform.GetOSName()  # Record platform name from browser.
    if self._page_test:
      self._page_test.WillStartBrowser(platform)

  def DidStartBrowser(self, browser):
    if self._page_test:
      self._page_test.DidStartBrowser(browser)

  def WillNavigateToPage(self, page, tab):
    """Override to ensure all resources are fetched from network."""
    tab.ClearCache(force=False)
    if self._page_test:
      self._page_test.WillNavigateToPage(page, tab)

  def DidNavigateToPage(self, page, tab):
    if self._page_test:
      self._page_test.DidNavigateToPage(page, tab)
    tab.WaitForDocumentReadyStateToBeComplete()
    py_utils.WaitFor(tab.HasReachedQuiescence, 30)

  def CleanUpAfterPage(self, page, tab):
    if self._page_test:
      self._page_test.CleanUpAfterPage(page, tab)

  def ValidateAndMeasurePage(self, page, tab, results):
    if self._page_test:
      self._page_test.ValidateAndMeasurePage(page, tab, results)


def _GetSubclasses(base_dir, cls):
  """Returns all subclasses of |cls| in |base_dir|.

  Args:
    cls: a class

  Returns:
    dict of {underscored_class_name: benchmark class}
  """
  return discover.DiscoverClasses(base_dir, base_dir, cls,
                                  index_by_class_name=True)


def _MaybeGetInstanceOfClass(target, base_dir, cls):
  if isinstance(target, cls):
    return target
  classes = _GetSubclasses(base_dir, cls)
  return classes[target]() if target in classes else None


def _PrintAllImpl(all_items, item_name, output_stream):
  output_stream.write('Available %s\' names with descriptions:\n' % item_name)
  keys = sorted(all_items.keys())
  key_description = [(k, all_items[k].Description()) for k in keys]
  _PrintPairs(key_description, output_stream)
  output_stream.write('\n')


def _PrintAllBenchmarks(base_dir, output_stream):
  # TODO: reuse the logic of finding supported benchmarks in benchmark_runner.py
  # so this only prints out benchmarks that are supported by the recording
  # platform.
  _PrintAllImpl(_GetSubclasses(base_dir, benchmark.Benchmark), 'benchmarks',
                output_stream)


def _PrintAllStories(base_dir, output_stream):
  # TODO: actually print all stories once record_wpr support general
  # stories recording.
  _PrintAllImpl(_GetSubclasses(base_dir, story.StorySet), 'story sets',
                output_stream)


def _PrintPairs(pairs, output_stream, prefix=''):
  """Prints a list of string pairs with alignment."""
  first_column_length = max(len(a) for a, _ in pairs)
  format_string = '%s%%-%ds  %%s\n' % (prefix, first_column_length)
  for a, b in pairs:
    output_stream.write(format_string % (a, b.strip()))


class WprRecorder():

  def __init__(self, environment, target, args=None, parse_flags=True):
    self._base_dir = environment.top_level_dir
    self._output_dir = tempfile.mkdtemp()
    try:
      self._options = self._CreateOptions()
      self._benchmark = _MaybeGetInstanceOfClass(target, self._base_dir,
                                                 benchmark.Benchmark)
      if parse_flags:
        self._parser = self._options.CreateParser(usage='See %prog --help')
        self._AddCommandLineArgs()
        self._ParseArgs(args)
        self._ProcessCommandLineArgs(environment)
        self._SetExtraBrowserOptions()
      page_test = None
      if self._benchmark is not None:
        test = self._benchmark.CreatePageTest(self.options)
        # Object only needed for legacy pages; newer benchmarks don't need this.
        if isinstance(test, legacy_page_test.LegacyPageTest):
          page_test = test

      self._record_page_test = RecorderPageTest(page_test)
      options = self._options
      self._page_set_base_dir = (
          self._options.page_set_base_dir if
          (hasattr('options', 'page_set_base_dir') and
           options.page_set_base_dir)
          else self._base_dir)
      self._story_set = self._GetStorySet(target)
    except:
      self._CleanUp()
      raise

  def __enter__(self):
    return self

  def __exit__(self, *args):
    self._CleanUp()

  @property
  def options(self):
    return self._options

  @property
  def story_set(self):
    return self._story_set

  def _CreateOptions(self):
    options = browser_options.BrowserFinderOptions()
    options.browser_options.wpr_mode = wpr_modes.WPR_RECORD
    options.intermediate_dir = self._output_dir
    return options

  def _CleanUp(self):
    shutil.rmtree(self._output_dir)

  def CreateResults(self):
    if self._benchmark is not None:
      benchmark_name = self._benchmark.Name()
      benchmark_description = self._benchmark.Description()
    else:
      benchmark_name = 'record_wpr'
      benchmark_description = None

    return results_options.CreateResults(
        self._options,
        benchmark_name=benchmark_name,
        benchmark_description=benchmark_description,
        report_progress=True)

  def _AddCommandLineArgs(self):
    self._parser.add_argument('--page-set-base-dir')
    story_runner.AddCommandLineArgs(self._parser)
    if self._benchmark is not None:
      self._benchmark.AddCommandLineArgs(self._parser)
      self._benchmark.SetArgumentDefaults(self._parser)
    self._parser.add_argument('--upload', action='store_true')
    self._parser.add_argument('--use-local-wpr',
                              action='store_true',
                              help=('Builds and runs WPR from Catapult. Also '
                                    'enables WPR debug output to STDOUT.'))
    self._SetArgumentDefaults()

  def _SetArgumentDefaults(self):
    self._parser.set_defaults(output_formats=['none'])

  def _ParseArgs(self, args=None):
    args_to_parse = sys.argv[1:] if args is None else args
    self._parser.parse_args(args_to_parse)

  def _ProcessCommandLineArgs(self, environment):
    story_runner.ProcessCommandLineArgs(self._parser, self._options,
                                        environment)

    if self._options.use_live_sites:
      self._parser.error("Can't --use-live-sites while recording")

    if self._benchmark is not None:
      self._benchmark.ProcessCommandLineArgs(self._parser, self._options)

  def _SetExtraBrowserOptions(self):
    if self._benchmark is not None:
      self._benchmark.SetExtraBrowserOptions(self._options)

  def _GetStorySet(self, target):
    if self._benchmark is not None:
      return self._benchmark.CreateStorySet(self._options)
    story_set = _MaybeGetInstanceOfClass(target, self._page_set_base_dir,
                                         story.StorySet)
    if story_set is None:
      sys.stderr.write('Target %s is neither benchmark nor story set.\n'
                       % target)
      if not self._HintMostLikelyBenchmarksStories(target):
        sys.stderr.write(
            'Found no similar benchmark or story. Please use '
            '--list-benchmarks or --list-stories to list candidates.\n')
        self._parser.print_usage()
      sys.exit(1)
    return story_set

  def _HintMostLikelyBenchmarksStories(self, target):
    def _Impl(all_items, category_name):
      candidates = matching.GetMostLikelyMatchedObject(
          six.iteritems(all_items), target, name_func=lambda kv: kv[1].Name())
      if candidates:
        sys.stderr.write('\nDo you mean any of those %s below?\n' %
                         category_name)
        _PrintPairs([(k, v.Description()) for k, v in candidates], sys.stderr)
        return True
      return False

    has_benchmark_hint = _Impl(
        _GetSubclasses(self._base_dir, benchmark.Benchmark), 'benchmarks')
    has_story_hint = _Impl(
        _GetSubclasses(self._base_dir, story.StorySet), 'stories')
    return has_benchmark_hint or has_story_hint

  def Record(self, results):
    assert self._story_set.wpr_archive_info, (
        'Pageset archive_data_file path must be specified.')

    # Always record the benchmark one time only.
    self._options.pageset_repeat = 1
    self._story_set.wpr_archive_info.AddNewTemporaryRecording()
    self._record_page_test.CustomizeBrowserOptions(self._options)
    story_runner.RunStorySet(
        self._record_page_test,
        self._story_set,
        self._options,
        results)

  def HandleResults(self, results, upload_to_cloud_storage):
    if results.had_failures or results.had_skips:
      logging.warning('Some pages failed and/or were skipped. The recording '
                      'has not been updated for these pages.')
    results.Finalize()
    self._story_set.wpr_archive_info.AddRecordedStories(
        [run.story for run in results.IterStoryRuns() if run.ok],
        upload_to_cloud_storage,
        target_platform=self._record_page_test.platform)


def Main(environment, **log_config_kwargs):
  # the log level is set in browser_options
  log_config_kwargs.pop('level', None)
  log_config_kwargs.setdefault('format', DEFAULT_LOG_FORMAT)
  logging.basicConfig(**log_config_kwargs)

  parser = argparse.ArgumentParser(
      usage='Record a benchmark or a story (page set).')
  parser.add_argument(
      'benchmark_or_story_set', # Note, positional.
      help=('benchmark or story set name. This argument might be optional. '
            'If both benchmark name '
            'and story name are specified, this takes precedence as the '
            'target of the recording.'),
      nargs='?')
  parser.add_argument('--story', help='story (page set) name')
  parser.add_argument('--list-stories', dest='list_stories',
                      action='store_true', help='list all story names.')
  parser.add_argument('--list-benchmarks', dest='list_benchmarks',
                      action='store_true', help='list all benchmark names.')
  parser.add_argument('--upload', action='store_true',
                      help='upload to cloud storage.')

  args, extra_args = parser.parse_known_args()

  if args.list_benchmarks or args.list_stories:
    if args.list_benchmarks:
      _PrintAllBenchmarks(environment.top_level_dir, sys.stderr)
    if args.list_stories:
      _PrintAllStories(environment.top_level_dir, sys.stderr)
    return 0

  target = args.benchmark_or_story_set or args.story

  if not target:
    sys.stderr.write('Please specify target (benchmark or story). Please refer '
                     'usage below\n\n')
    parser.print_help()
    return 0

  binary_manager.InitDependencyManager(environment.client_configs)

  # TODO(crbug.com/1111556): update WprRecorder so that it handles the
  # difference between recording a benchmark vs recording a story better based
  # on the distinction between args.benchmark & args.story
  with WprRecorder(environment, target, extra_args) as wpr_recorder:
    results = wpr_recorder.CreateResults()
    wpr_recorder.Record(results)
    wpr_recorder.HandleResults(results, args.upload)
    return min(255, results.num_failed)
