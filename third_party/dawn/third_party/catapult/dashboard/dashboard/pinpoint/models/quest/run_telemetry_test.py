# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Quest for running a Telemetry benchmark in Swarming."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import re

from dashboard.pinpoint.models.quest import run_performance_test

_DEFAULT_EXTRA_ARGS = [
    '-v', '--upload-results', '--output-format', 'histograms'
]

_STORY_REGEX = re.compile(r'[^a-zA-Z0-9]')

# crbug/1146949
# Please keep this executable-argument mapping synced with perf waterfall:
#  https://chromium.googlesource.com/chromium/src/+/main/tools/perf/core/bot_platforms.py
_WATERFALL_ENABLED_GTEST_NAMES = {
    'base_perftests': [
        '--test-launcher-jobs=1', '--test-launcher-retry-limit=0'
    ],
    'components_perftests': ['--xvfb'],
    'dawn_perf_tests': [
        '--test-launcher-jobs=1', '--test-launcher-retry-limit=0'
    ],
    'gpu_perftests': [],
    'load_library_perf_tests': [],
    'performance_browser_tests': [
        '--full-performance-run',
        '--test-launcher-jobs=1',
        '--test-launcher-retry-limit=0',
        # Allow the full performance runs to take up to 60 seconds (rather
        # than the default of 30 for normal CQ browser test runs).
        '--ui-test-action-timeout=60000',
        '--ui-test-action-max-timeout=60000',
        '--test-launcher-timeout=60000',
        '--gtest_filter=*/TabCapturePerformanceTest.*:'
        '*/CastV2PerformanceTest.*',
    ],
    'sync_performance_tests': [
        '--test-launcher-jobs=1', '--test-launcher-retry-limit=0'
    ],
    'tracing_perftests': [],
    'views_perftests': ['--xvfb']
}

# GTEST_EXECUTABLE_NAME is based on the following link:
# https://source.chromium.org/chromium/chromium/src/+/main:tools/perf/core/bot_platforms.py;l=282
GTEST_EXECUTABLE_NAME = {
    'base_perftests': 'base_perftests',
    'components_perftests': 'components_perftests',
    'dawn_perf_tests': 'dawn_perf_tests',
    'gpu_perftests': 'gpu_perftests',
    'load_library_perf_tests': 'load_library_perf_tests',
    'performance_browser_tests': 'browser_tests',
    'sync_performance_tests': 'sync_performance_tests',
    'tracing_perftests': 'tracing_perftests',
    'views_perftests': 'views_perftests'
}

_CROSSBENCH_NAME = {
    'jetstream2.crossbench': 'jetstream_2.2',
    'motionmark1.3.crossbench': 'motionmark_1.3',
    'speedometer3.crossbench': 'speedometer_3.0',
}


def _StoryToRegex(story_name):
  # Telemetry's --story-filter argument takes in a regex, not a
  # plain string. Stories can have all sorts of special characters
  # in their names (see crbug.com/983993) which would confuse a
  # regex. We thus keep only a small set of "safe chars"
  # and replace all others with match-any-character regex dots.
  return '^%s$' % _STORY_REGEX.sub('.', story_name)


def ChangeDependentArgs(args, change):
  # For results2 to differentiate between runs, we need to add the
  # Telemetry parameter `--results-label <change>` to the runs.
  extra_args = list(args)
  extra_args += ('--results-label', str(change))
  if change.change_args:
    extra_args.extend(change.change_args)
  return extra_args


class RunTelemetryTest(run_performance_test.RunPerformanceTest):

  @classmethod
  def _ComputeCommand(cls, arguments):
    # We're moving the definition of which command to run here, instead of
    # relying on what's in the isolate because the 'command' feature is
    # deprecated and will be removed soon (EOY 2020).
    # TODO(dberris): Move this out to a configuration elsewhere.
    benchmark = arguments.get('benchmark')
    command = [
        'luci-auth',
        'context',
        '--',
        'vpython3',
        '../../testing/test_env.py',
        '../../testing/scripts/run_performance_tests.py',
    ]
    if benchmark in _WATERFALL_ENABLED_GTEST_NAMES:
      command.append(GTEST_EXECUTABLE_NAME[benchmark])
    elif benchmark in _CROSSBENCH_NAME:
      command.append('../../third_party/crossbench/cb.py')
    else:
      command.append('../../tools/perf/run_benchmark')
    relative_cwd = arguments.get('relative_cwd', 'out/Release')
    return relative_cwd, command

  def Start(self, change, isolate_server, isolate_hash):
    extra_swarming_tags = {'change': str(change)}
    return self._Start(
        change,
        isolate_server,
        isolate_hash,
        ChangeDependentArgs(self._extra_args, change),
        extra_swarming_tags,
        execution_timeout_secs=None)

  @classmethod
  def _CrossbenchExtraTestArgs(cls, benchmark, arguments):
    extra_test_args = []
    extra_test_args.append(f'--benchmark-display-name={benchmark}')
    extra_test_args.append(f'--benchmarks={_CROSSBENCH_NAME[benchmark]}')

    browser = arguments.get('browser')
    if not browser:
      raise TypeError('Missing "browser" argument for crossbench.')
    extra_test_args.append(f'--browser={browser}')

    extra_test_args += super()._ExtraTestArgs(arguments)
    return extra_test_args

  @classmethod
  def _ExtraTestArgs(cls, arguments):
    benchmark = arguments.get('benchmark')
    if not benchmark:
      raise TypeError('Missing "benchmark" argument.')

    if benchmark in _CROSSBENCH_NAME:
      return cls._CrossbenchExtraTestArgs(benchmark, arguments)

    extra_test_args = []

    # If we're running a single test,
    # do so even if it's configured to be ignored in expectations.config.
    if not arguments.get('story_tags'):
      extra_test_args.append('-d')

    if benchmark in _WATERFALL_ENABLED_GTEST_NAMES:
      # crbug/1146949
      # Pass the correct arguments to run gtests on pinpoint.
      # As we don't want to add dependency to chromium, the names of gtests are
      # hard coded here, instead of loading from bot_platforms.py.
      extra_test_args += ('--gtest-benchmark-name', benchmark)
      extra_test_args += ('--non-telemetry', 'true')
      extra_test_args.extend(_WATERFALL_ENABLED_GTEST_NAMES[benchmark])
    else:
      extra_test_args += ('--benchmarks', benchmark)

    story = arguments.get('story')
    if story:
      # TODO(crbug.com/982027): Note that usage of  "--story-filter" may be
      # replaced with --story=<story> (no regex needed). Support for --story
      # flag landed in
      # https://chromium-review.googlesource.com/c/catapult/+/1869800 (Oct 22,
      # 2019) so we cannot turn this on by default until we no longer need to be
      # able to run revisions older than that.
      extra_test_args += ('--story-filter', _StoryToRegex(story))

    story_tags = arguments.get('story_tags')
    if story_tags:
      extra_test_args += ('--story-tag-filter', story_tags)

    extra_test_args += ('--pageset-repeat', '1')

    browser = arguments.get('browser')
    if not browser:
      raise TypeError('Missing "browser" argument.')
    extra_test_args += ('--browser', browser)
    extra_test_args += _DEFAULT_EXTRA_ARGS
    extra_test_args += super(RunTelemetryTest, cls)._ExtraTestArgs(arguments)
    return extra_test_args

  @classmethod
  def _GetSwarmingTags(cls, arguments):
    tags = {}
    benchmark = arguments.get('benchmark')
    if not benchmark:
      raise TypeError('Missing "benchmark" argument.')
    tags['benchmark'] = benchmark
    story_filter = arguments.get('story')
    tag_filter = arguments.get('story_tags')
    tags['hasfilter'] = '1' if story_filter or tag_filter else '0'
    if story_filter:
      tags['storyfilter'] = story_filter
    if tag_filter:
      tags['tagfilter'] = tag_filter
    return tags
