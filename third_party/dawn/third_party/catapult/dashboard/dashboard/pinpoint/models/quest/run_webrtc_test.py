# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Quest for running WebRTC perf tests in Swarming."""

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import os
from dashboard.pinpoint.models.quest import run_test


def _StoryToGtestFilter(story_name):
  """Returns a gtest_filter to run only tests generating data for |story_name|.

  WebRTC perf tests story names and gtest names are different and use different
  formats (e.g. snake case vs camel case). This function uses some heuristics
  to create a gtest filter that will allow PinPoint to run only the tests it
  requires for the bisection (instead of running all the tests in the binary).

  Here are the rules that compose this heuristic function:
  - The gtest name is the story name with each word starting with a uppercase.
  - If the story name is too long, it is sometimes truncated in the gtest
    name but after at least 50 characters.
  - The story in the gtest name can be followed by any set of character. This
    can lead to running too much tests but is useful for TEST_F tests.
  - Some tests simply don't fit in these rules (e.g. RampUpTest).

  If the filter generated is wrong, PinPoint will not execute any tests, and
  the bisection will fail to find a culprit. In that case, this function or the
  test name needs to be updated.
  """
  if story_name.endswith('_alice'):
    story_name = story_name[:-len('_alice')]
  elif story_name.endswith('_alice-video'):
    story_name = story_name[:-len('_alice-video')]
  elif story_name.endswith('_bob'):
    story_name = story_name[:-len('_bob')]

  if story_name in ['first_rampup', 'rampdown', 'second_rampup']:
    return 'RampUpTest.*'
  if story_name.startswith('real - estimated'):
    return '*.Real_Estimated_*'
  if story_name.startswith('bwe_after_'):
    return '*.Bwe_After_*'

  if len(story_name) > 50:
    story_name = story_name[:50]

  return '*.%s*' % '_'.join(
      w[:1].upper() + w[1:] for w in story_name.split('_'))


class RunWebRtcTest(run_test.RunTest):

  @classmethod
  def _ComputeCommand(cls, arguments):
    if 'target' not in arguments:
      raise ValueError('Missing "target" in arguments.')

    # This is the command used to run webrtc_perf_tests.
    if 'android' in arguments.get('configuration'):
      default_command = [
          'vpython3',
          '../../build/android/test_wrapper/logdog_wrapper.py',
          '--target',
          arguments.get('target'),
          '--logdog-bin-cmd',
          '../../bin/logdog_butler',
          '--logcat-output-file',
          '${ISOLATED_OUTDIR}/logcats',
      ]
    else:
      default_command = [
          'vpython3',
          '../../tools_webrtc/flags_compatibility.py',
          '../../testing/test_env.py',
          os.path.join('.', arguments.get('target')),
          '--test_artifacts_dir=${ISOLATED_OUTDIR}',
      ]
    command = arguments.get('command', default_command)

    # The tests are run in the builder out directory.
    builder_cwd = _SanitizeFileName(arguments.get('builder'))
    relative_cwd = arguments.get('relative_cwd', 'out/' + builder_cwd)
    return relative_cwd, command

  def Start(self, change, isolate_server, isolate_hash):
    return self._Start(
        change,
        isolate_server,
        isolate_hash,
        self._extra_args,
        swarming_tags={},
        execution_timeout_secs=10800)

  @classmethod
  def _ExtraTestArgs(cls, arguments):
    results_filename = '${ISOLATED_OUTDIR}/webrtc_perf_tests/perf_results.json'
    extra_test_args = [
        '--nologs',
        '--isolated-script-test-perf-output=%s' % results_filename,
    ]
    # Gtests are filtered based on the story name.
    story = arguments.get('story')
    if story:
      extra_test_args.append('--gtest_filter=%s' % _StoryToGtestFilter(story))

    extra_test_args += super(RunWebRtcTest, cls)._ExtraTestArgs(arguments)
    return extra_test_args


def _SanitizeFileName(name):
  safe_with_spaces = ''.join(c if c.isalnum() else ' ' for c in name)
  return '_'.join(safe_with_spaces.split())
