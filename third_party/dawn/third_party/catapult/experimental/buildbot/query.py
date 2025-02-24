#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import json
import logging
import multiprocessing
import sys
import time

import buildbot
import six
from six.moves import map
from six.moves import zip


MASTER_NAME = 'chromium.perf'
BUILDER_NAMES = ('Win 7 Perf (1)', 'Mac 10.9 Perf (1)')
BENCHMARK_NAME = 'smoothness.top_25_smooth'
VALUE_NAME = 'frame_times'

BUILD_COUNT = 100


def QueryBuild(build):
  steps = build.steps
  if not BENCHMARK_NAME in steps:
    return None

  step = steps[BENCHMARK_NAME]
  if step.result != buildbot.SUCCESS:
    return None

  revision_data = []
  trace_results = six.iteritems(step.results['chart_data']['charts'][VALUE_NAME])
  for user_story_name, user_story_data in trace_results:
    revision_data.append({
        'user_story': user_story_name,
        'start_time': step.start_time,
        'end_time': step.end_time,
        'values': user_story_data['values'],
    })
  return {
      'start_time': build.start_time,
      'end_time': build.end_time,
      'user_story_runs': revision_data,
  }


def QueryBuilds(builder):
  return list(map(QueryBuild, builder.LastBuilds(BUILD_COUNT)))


def main():
  logging.getLogger().setLevel(logging.INFO)

  builders = buildbot.Builders(MASTER_NAME)
  process_pool = multiprocessing.Pool(8)

  start_time = time.time()
  data = process_pool.map(QueryBuilds,
                          (builders[name] for name in BUILDER_NAMES))
  data = dict(zip(BUILDER_NAMES, data))
  logging.info('Queried %d builds in %2.2f seconds.',
               BUILD_COUNT, time.time() - start_time)

  start_time = time.time()
  json.dump(data, sys.stdout)
  logging.info('Wrote data in %2.2f seconds.', time.time() - start_time)


if __name__ == '__main__':
  main()
