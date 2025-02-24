# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Helper methods for working with histograms and diagnostics."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import re

from tracing.value.diagnostics import reserved_infos

# List of non-TBMv2 chromium.perf Telemetry benchmarks
_LEGACY_BENCHMARKS = [
    'blink_perf.bindings', 'blink_perf.canvas', 'blink_perf.css',
    'blink_perf.dom', 'blink_perf.events', 'blink_perf.image_decoder',
    'blink_perf.layout', 'blink_perf.owp_storage', 'blink_perf.paint',
    'blink_perf.parser', 'blink_perf.shadow_dom', 'blink_perf.svg',
    'cronet_perf_tests', 'dromaeo', 'dummy_benchmark.noisy_benchmark_1',
    'dummy_benchmark.stable_benchmark_1', 'jetstream2', 'jetstream2-minorms',
    'jetstream2.crossbench', 'kraken', 'motionmark1.3.crossbench', 'octane',
    'rasterize_and_record_micro.partial_invalidation',
    'rasterize_and_record_micro.top_25', 'scheduler.tough_scheduling_cases',
    'smoothness.desktop_tough_pinch_zoom_cases',
    'smoothness.gpu_rasterization.polymer',
    'smoothness.gpu_rasterization.top_25_smooth',
    'smoothness.gpu_rasterization.tough_filters_cases',
    'smoothness.gpu_rasterization.tough_path_rendering_cases',
    'smoothness.gpu_rasterization.tough_pinch_zoom_cases',
    'smoothness.gpu_rasterization.tough_scrolling_cases',
    'smoothness.gpu_rasterization_and_decoding.image_decoding_cases',
    'smoothness.image_decoding_cases', 'smoothness.key_desktop_move_cases',
    'smoothness.key_mobile_sites_smooth', 'smoothness.key_silk_cases',
    'smoothness.maps', 'smoothness.pathological_mobile_sites',
    'smoothness.simple_mobile_sites',
    'smoothness.sync_scroll.key_mobile_sites_smooth',
    'smoothness.top_25_smooth', 'smoothness.tough_ad_cases',
    'smoothness.tough_animation_cases', 'smoothness.tough_canvas_cases',
    'smoothness.tough_filters_cases', 'smoothness.tough_image_decode_cases',
    'smoothness.tough_path_rendering_cases',
    'smoothness.tough_pinch_zoom_cases', 'smoothness.tough_scrolling_cases',
    'smoothness.tough_texture_upload_cases', 'smoothness.tough_webgl_ad_cases',
    'smoothness.tough_webgl_cases', 'speedometer', 'speedometer-future',
    'speedometer2', 'speedometer2-future', 'speedometer2-minorms',
    'speedometer2-predictable', 'speedometer3', 'speedometer3-future',
    'speedometer3-minorms', 'speedometer3-predictable',
    'speedometer3.crossbench', 'thread_times.key_hit_test_cases',
    'thread_times.key_idle_power_cases', 'thread_times.key_mobile_sites_smooth',
    'thread_times.key_noop_cases', 'thread_times.key_silk_cases',
    'thread_times.simple_mobile_sites', 'thread_times.tough_compositor_cases',
    'thread_times.tough_scrolling_cases', 'v8.detached_context_age_in_gc'
]
_STATS_BLACKLIST = ['std', 'count', 'max', 'min', 'sum']

SUITE_LEVEL_SPARSE_DIAGNOSTIC_NAMES = {
    reserved_infos.ARCHITECTURES.name,
    reserved_infos.BENCHMARKS.name,
    reserved_infos.BENCHMARK_DESCRIPTIONS.name,
    reserved_infos.BOTS.name,
    reserved_infos.BUG_COMPONENTS.name,
    reserved_infos.DOCUMENTATION_URLS.name,
    reserved_infos.GPUS.name,
    reserved_infos.MASTERS.name,
    reserved_infos.MEMORY_AMOUNTS.name,
    reserved_infos.OS_NAMES.name,
    reserved_infos.OS_VERSIONS.name,
    reserved_infos.OWNERS.name,
    reserved_infos.PRODUCT_VERSIONS.name,
}

HISTOGRAM_LEVEL_SPARSE_DIAGNOSTIC_NAMES = {
    reserved_infos.ALERT_GROUPING.name,
    reserved_infos.DEVICE_IDS.name,
    reserved_infos.STORIES.name,
    reserved_infos.STORYSET_REPEATS.name,
    reserved_infos.STORY_TAGS.name,
}

SPARSE_DIAGNOSTIC_NAMES = SUITE_LEVEL_SPARSE_DIAGNOSTIC_NAMES.union(
    HISTOGRAM_LEVEL_SPARSE_DIAGNOSTIC_NAMES)

ADD_HISTOGRAM_RELATED_DIAGNOSTICS = SPARSE_DIAGNOSTIC_NAMES.union({
    reserved_infos.BUILD_URLS.name,
    reserved_infos.CHROMIUM_COMMIT_POSITIONS.name,
    reserved_infos.LOG_URLS.name,
    reserved_infos.POINT_ID.name,
    reserved_infos.SUMMARY_KEYS.name,
    reserved_infos.TRACE_URLS.name,
})


def EscapeName(name):
  """Escapes a trace name so it can be stored in a row.

  Args:
    name: A string representing a name.

  Returns:
    An escaped version of the name.
  """
  return re.sub(r'[\:|=/#&,]', '_', name)


def ComputeTestPath(hist, ignore_grouping_label=False):
  # If a Histogram represents a summary across multiple stories, then its
  # 'stories' diagnostic will contain the names of all of the stories.
  # If a Histogram is not a summary, then its 'stories' diagnostic will contain
  # the singular name of its story.
  is_summary = list(hist.diagnostics.get(reserved_infos.SUMMARY_KEYS.name, []))

  grouping_label = GetGroupingLabelFromHistogram(
      hist) if not ignore_grouping_label else None

  is_ref = hist.diagnostics.get(reserved_infos.IS_REFERENCE_BUILD.name)
  if is_ref and len(is_ref) == 1:
    is_ref = is_ref.GetOnlyElement()

  story_name = hist.diagnostics.get(reserved_infos.STORIES.name)
  if story_name and len(story_name) == 1:
    story_name = story_name.GetOnlyElement()
  else:
    story_name = None

  return ComputeTestPathFromComponents(
      hist.name,
      grouping_label=grouping_label,
      story_name=story_name,
      is_summary=is_summary,
      is_ref=is_ref)


def ComputeTestPathFromComponents(hist_name,
                                  grouping_label=None,
                                  story_name=None,
                                  is_summary=None,
                                  is_ref=False,
                                  needs_escape=True):
  path = hist_name or ''

  if grouping_label and (not is_summary
                         or reserved_infos.STORY_TAGS.name in is_summary):
    path += '/' + grouping_label

  if story_name and not is_summary:
    if needs_escape:
      escaped_story_name = EscapeName(story_name)
      path += '/' + escaped_story_name
    else:
      path += '/' + story_name
    if is_ref:
      path += '_ref'
  elif is_ref:
    path += '/ref'

  return path


def GetGroupingLabelFromHistogram(hist):
  tags = hist.diagnostics.get(reserved_infos.STORY_TAGS.name) or []

  tags_to_use = [t.split(':') for t in tags if ':' in t]

  return '_'.join(v for _, v in sorted(tags_to_use))


def IsLegacyBenchmark(benchmark_name):
  return benchmark_name in _LEGACY_BENCHMARKS


def ShouldFilterStatistic(test_name, benchmark_name, stat_name):
  if test_name == 'benchmark_total_duration':
    return True
  if benchmark_name.startswith(
      'memory') and not benchmark_name.startswith('memory.long_running'):
    if 'memory:' in test_name and stat_name in _STATS_BLACKLIST:
      return True
  if benchmark_name.startswith('memory.long_running'):
    value_name = '%s_%s' % (test_name, stat_name)
    return not _ShouldAddMemoryLongRunningValue(value_name)
  if benchmark_name in ('media.desktop', 'media.mobile'):
    value_name = '%s_%s' % (test_name, stat_name)
    return not _ShouldAddMediaValue(value_name)
  if benchmark_name.startswith('system_health'):
    if stat_name in _STATS_BLACKLIST:
      return True
  return False


def _ShouldAddMediaValue(value_name):
  media_re = re.compile(
      r'(?<!dump)(?<!process)_(std|count|max|min|sum|pct_\d{4}(_\d+)?)$')
  return not media_re.search(value_name)


def _ShouldAddMemoryLongRunningValue(value_name):
  v8_re = re.compile(
      r'renderer_processes:'
      r'(reported_by_chrome:v8|reported_by_os:system_memory:[^:]+$)')
  if 'memory:chrome' in value_name:
    return ('renderer:subsystem:v8' in value_name
            or 'renderer:vmstats:overall' in value_name
            or bool(v8_re.search(value_name)))
  return 'v8' in value_name
