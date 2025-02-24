# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import

from typing import List, Optional

import datetime
import logging
import six
import urllib.parse as encoder

from dashboard.models import graph_data

REPOSITORY_HOST_MAPPING = [{
    'label':
        'Chromium',
    'public_host':
        'https://perf.luci.app',
    'internal_host':
        'https://chrome-perf.corp.goog',
    'masters': [
        'ChromeFYIInternal',
        'ChromiumAndroid',
        'ChromiumChrome',
        'ChromiumChromiumos',
        'ChromiumClang',
        'ChromiumFuchsia',
        'ChromiumGPUFYI',
        'ChromiumPerf',
        'ChromiumPerfFyi',
        'ChromiumPerfPGO',
        'TryServerChromiumFuchsia',
        'TryserverChromiumChromiumOS',
        'ChromiumFuchsiaFyi',
        'TryserverChromiumAndroid',
        'ChromiumAndroidFyi',
        'ChromiumFYI',
        'ChromiumPerfFyi.all',
        'ChromiumGPU',
    ]
}, {
    'label': 'WebRTC',
    'public_host': 'https://webrtc-perf.luci.app',
    'internal_host': None,
    'masters': ['WebRTCPerf']
}, {
    'label': 'Widevine CDM',
    'public_host': None,
    'internal_host': 'https://widevine-cdm-perf.corp.goog',
    'masters': ['WidevineCdmPerf']
}, {
    'label': 'Widevine Whitebox',
    'public_host': None,
    'internal_host': 'https://widevine-whitebox-perf.corp.goog',
    'masters': ['WidevineWhiteboxPerf_master']
}, {
    'label': 'V8',
    'public_host': None,
    'internal_host': 'https://v8-perf.corp.goog',
    'masters': ['internal.client.v8', 'client.v8']
}, {
    'label': 'Devtools-Frontend',
    'public_host': None,
    'internal_host': 'https://devtools-frontend-perf.corp.goog',
    'masters': ['client.devtools-frontend.integration']
}]

QUERY_TEST_LIMIT = 5


def GetSkiaUrl(start_time: datetime.datetime,
               end_time: datetime.datetime,
               master: str,
               bots: Optional[List[str]] = None,
               benchmarks: Optional[List[str]] = None,
               tests: Optional[List[str]] = None,
               subtests_1: Optional[List[str]] = None,
               subtests_2: Optional[List[str]] = None,
               internal_only: bool = True,
               num_points: int = 500):
  repo_map = _GetRepoMapForMaster(master)
  if not repo_map:
    return None

  host = repo_map['internal_host'] if internal_only else repo_map[
    'public_host']
  if not host:
    logging.warning(
        'Skia instance does not exist for master %s and internal_only=%s',
        master, internal_only)
    return None

  def _FormatList(lst):
    # Converts None into empty lists.
    # Remove duplicates while preserving order, as opposed to using set.

    if lst == None:
      return []

    return list(dict.fromkeys(lst))

  benchmark_query_str = ''.join(
      '&benchmark=%s' % benchmark for benchmark in _FormatList(benchmarks))
  bot_query_str = ''.join('&bot=%s' % bot for bot in _FormatList(bots))
  test_query_str = ''.join('&test=%s' % test for test in _FormatList(tests))
  subtest_1_query_str = ''.join(
      '&subtest_1=%s' % subtest for subtest in _FormatList(subtests_1))
  subtest_2_query_str = ''.join(
      '&subtest_2=%s' % subtest for subtest in _FormatList(subtests_2))

  query_str = encoder.quote('stat=value%s%s%s%s%s' %
                            (benchmark_query_str, bot_query_str, test_query_str,
                             subtest_1_query_str, subtest_2_query_str))
  return _GenerateUrl(host, query_str, start_time, end_time, num_points)


def GetSkiaUrlsForAlertGroup(alert_group_id: str,
                            internal_only: bool,
                            masters):
  urls = set()
  for master in masters:
    repo_map = _GetRepoMapForMaster(master)
    if repo_map:
      label = repo_map['label']
      host = repo_map['internal_host'] if internal_only else repo_map[
          'public_host']
      if host:
        urls.add('%s: %s/_/alertgroup?group_id=%s' %
                 (label, host, alert_group_id))

  return list(urls)


def GetSkiaUrlForAnomaly(anomaly: graph_data.anomaly.Anomaly) -> str:
  repo_map = _GetRepoMapForMaster(anomaly.master_name)
  if repo_map:
    host = repo_map['internal_host'] if anomaly.internal_only else repo_map[
          'public_host']
    return '%s/_/anomaly?key=%s' % (host, six.ensure_str(anomaly.key.urlsafe()))

  return ''


def _GetRepoMapForMaster(master: str):
  for repo_map in REPOSITORY_HOST_MAPPING:
    if master in repo_map['masters']:
      return repo_map
  return None


def _GenerateUrl(host: str, query_str: str, begin_date: datetime.datetime,
                 end_date: datetime.datetime, num_commits: int):
  request_params_str = 'numCommits=%i' % num_commits
  if begin_date:
    begin = _GetTimeInt(begin_date)
    request_params_str += '&begin=%i' % begin
  if end_date:
    end = _GetTimeInt(end_date)
    request_params_str += '&end=%i' % end
  request_params_str += '&queries=%s' % query_str
  return '%s/e/?%s' % (host, request_params_str)


def _GetTimeInt(date: datetime.datetime):
  epoch = datetime.datetime.utcfromtimestamp(0)
  int_timestamp = (date - epoch) // datetime.timedelta(seconds=1)
  return int_timestamp
