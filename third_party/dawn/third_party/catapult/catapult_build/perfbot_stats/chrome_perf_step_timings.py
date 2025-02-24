#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Script to pull chromium.perf step timings from chrome-infra-stats API.

Currently this pulls the list of steps per builder. For each step, if it is not
a setup step, we get the step stats for the last 20 runs for that builder.

The API documentation for chrome-infra-stats is at:
https://apis-explorer.appspot.com/apis-explorer/?
   base=https://chrome-infra-stats.appspot.com/_ah/api#p/
"""

from __future__ import print_function
import csv
import datetime
import json
import sys
import six.moves.urllib.request
import six.moves.urllib.parse
import six.moves.urllib.error


BUILDER_STEPS_URL = ('https://chrome-infra-stats.appspot.com/_ah/api/stats/v1/'
                     'masters/chromium.perf/%s')


STEP_ACTIVE_URL = ('https://chrome-infra-stats.appspot.com/_ah/api/stats/v1/'
                   'steps/last/chromium.perf/%s/%s/1')


STEP_STATS_URL = ('https://chrome-infra-stats.appspot.com/_ah/api/stats/v1/'
                  'stats/last/chromium.perf/%s/%s/20')


IGNORED_STEPS = [
    'List Perf Tests',
    'Sharded Perf Tests',
    'authorize_adb_devices',
    'bot_update',
    'build__schedule__time__',
    'clean local files',
    'cleanup_temp',
    'device_status_check',
    'extract build',
    'gclient runhooks',
    'get compile targets for scripts',
    'get perf test list',
    'gsutil download_build_product',
    'host_info',
    'install ChromeShell.apk',
    'json.output cache',
    'json.output cache',
    'overall__build__result__',
    'overall__queued__time__',
    'provision_devices',
    'read test spec',
    'rmtree build directory',
    'setup_build',
    'spawn_logcat_monitor',
    'stack_tool_for_tombstones',
    'stack_tool_with_logcat_dump',
    'steps',
    'test_report',
    'unzip_build_product',
    'update_scripts'
]

KNOWN_TESTERS_LIST = [
    'Android Nexus4 Perf',
    'Android Nexus5 Perf',
    'Android Nexus6 Perf',
    'Android Nexus10 Perf',
    'Android Nexus7v2 Perf',
    'Android One Perf',
    'Linux Perf (1)',
    'Linux Perf (2)',
    'Linux Perf (3)',
    'Linux Perf (4)',
    'Linux Perf (5)',
    'Mac 10.8 Perf (1)',
    'Mac 10.8 Perf (2)',
    'Mac 10.8 Perf (3)',
    'Mac 10.8 Perf (4)',
    'Mac 10.8 Perf (5)',
    'Mac 10.9 Perf (1)',
    'Mac 10.9 Perf (2)',
    'Mac 10.9 Perf (3)',
    'Mac 10.9 Perf (4)',
    'Mac 10.9 Perf (5)',
    'Win 7 ATI GPU Perf',
    'Win 7 Intel GPU Perf',
    'Win 7 Low-End Perf (1)',
    'Win 7 Low-End Perf (2)',
    'Win 7 Nvidia GPU Perf',
    'Win 7 Perf (1)',
    'Win 7 Perf (2)',
    'Win 7 Perf (3)',
    'Win 7 Perf (4)',
    'Win 7 Perf (5)',
    'Win 7 x64 Perf (1)',
    'Win 7 x64 Perf (2)',
    'Win 8 Perf (1)',
    'Win 8 Perf (2)',
    'Win XP Perf (1)',
    'Win XP Perf (2)',
    'Win XP Perf (3)',
    'Win XP Perf (4)',
    'Win XP Perf (5)'
]


USAGE = 'Usage: chrome-perf-step-timings.py <outfilename>'


def main():
  if len(sys.argv) != 2:
    print(USAGE)
    sys.exit(0)
  outfilename = sys.argv[1]

  threshold_time = datetime.datetime.now() - datetime.timedelta(days=2)

  col_names = [('builder', 'step', 'run_count', 'stddev', 'mean', 'maximum',
                'median', 'seventyfive', 'ninety', 'ninetynine')]
  with open(outfilename, 'wb') as f:
    writer = csv.writer(f)
    writer.writerows(col_names)

  for builder in KNOWN_TESTERS_LIST:
    step_timings = []
    url = BUILDER_STEPS_URL % six.moves.urllib.parse.quote(builder)
    response = six.moves.urllib.request.urlopen(url)
    results = json.load(response)
    steps = results['steps']
    steps.sort()  # to group tests and their references together.
    for step in steps:
      if step in IGNORED_STEPS:
        continue
      url = STEP_ACTIVE_URL % (
          six.moves.urllib.parse.quote(builder),
          six.moves.urllib.parse.quote(step))
      response = six.moves.urllib.request.urlopen(url)
      results = json.load(response)
      if ('step_records' not in list(results.keys()) or
          len(results['step_records']) == 0):
        continue
      first_record = results['step_records'][0]
      last_step_time = datetime.datetime.strptime(
          first_record['step_start'], "%Y-%m-%dT%H:%M:%S.%f")
      # ignore steps that did not run for more than 2 days
      if last_step_time < threshold_time:
        continue
      url = STEP_STATS_URL % (
          six.moves.urllib.parse.quote(builder),
          six.moves.urllib.parse.quote(step))
      response = six.moves.urllib.request.urlopen(url)
      results = json.load(response)
      step_timings.append(
          [builder, step, results['count'], results['stddev'],
           results['mean'], results['maximum'], results['median'],
           results['seventyfive'], results['ninety'],
           results['ninetynine']])
    with open(outfilename, 'ab') as f:
      writer = csv.writer(f)
      writer.writerows(step_timings)


if __name__ == '__main__':
  main()
