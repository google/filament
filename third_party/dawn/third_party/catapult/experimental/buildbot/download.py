#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import print_function
import logging
import multiprocessing
import sys
import time
import traceback

import buildbot
import six


POLL_INTERVAL = 600
BUILD_HISTORY_COUNT = 200
BUILD_RESULTS_COUNT = 50


def FetchLatestBuildResults(builder):
  try:
    builder.FetchRecentBuilds(BUILD_HISTORY_COUNT)
    print('Fetching results for', builder)
    for build in builder.LastBuilds(BUILD_RESULTS_COUNT):
      for step in six.itervalues(build.steps):
        step.results  # pylint: disable=pointless-statement
  except:  # multiprocessing doesn't give useful stack traces, so print it here.
    traceback.print_exc(file=sys.stderr)
    print()
    raise


def main():
  logging.getLogger().setLevel(logging.INFO)
  builders = buildbot.Builders('chromium.perf')

  process_pool = multiprocessing.Pool(4)

  while True:
    print('Refreshing...')
    buildbot.Update('chromium.perf', builders)
    process_pool.map(FetchLatestBuildResults, six.itervalues(builders))
    print('Refreshed!')
    time.sleep(POLL_INTERVAL)


if __name__ == '__main__':
  main()
