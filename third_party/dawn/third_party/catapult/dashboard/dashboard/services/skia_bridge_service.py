# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from datetime import datetime
from dashboard.common import utils
from dashboard.models import graph_data

import logging
import json
import random
import ssl
import time


SKIA_UPLOAD_URL = 'https://skia-bridge-dot-chromeperf.appspot.com/data/upload_queue'
if utils.IsStagingEnvironment():
  SKIA_UPLOAD_URL = 'https://skia-bridge-dot-chromeperf-stage.uc.r.appspot.com/data/upload_queue'

_RETRY_STATUS_CODES = ['502', '503']
_MAX_RETRY_COUNT = 10


class SkiaServiceClient:

  def __init__(self):
    self._row_data_cache = []

  def SendRowsToSkiaBridge(self):
    if len(self._row_data_cache) > 0:
      request_data = {'rows': self._row_data_cache}
      logging.info('Sending %i rows to skia bridge', len(self._row_data_cache))
      response, _ = self.SendSkiaBridgeRequest(
          'POST',
          request_data,
          {
              'Content-Type': 'application/json'
          })
      if response['status'] != '200':
        logging.error('Error sending data to skia-bridge')
      self._row_data_cache.clear()

  def SendSkiaBridgeRequest(self, http_method, body, headers):
    http = utils.ServiceAccountHttp()
    is_complete = False
    retry_count = 0
    while not is_complete:
      should_retry = False
      try:
        response, content = http.request(
            SKIA_UPLOAD_URL,
            method=http_method,
            body=json.dumps(body),
            headers=headers)
        should_retry = response['status'] in _RETRY_STATUS_CODES
      except ssl.SSLEOFError as e:
        # This error is intermittent and likely related to GAE proxies.
        # Use the existing retry mechanism to retry in this scenario
        should_retry = True
        response = None
        logging.info('Encountered retryable error %s', str(e))

      if should_retry:
        retry_count += 1
        if retry_count < _MAX_RETRY_COUNT:
          # Use exponential retry
          max_sleep_seconds = 2**retry_count
          sleep_seconds = random.random() * max_sleep_seconds
          if response:
            logging.info('Received status code %s.', response['status'])
          logging.info('Sleeping for %i seconds before retry', sleep_seconds)
          time.sleep(sleep_seconds)
        else:
          logging.error('Exhausted max %i retry attempts', _MAX_RETRY_COUNT)
          is_complete = True
      else:
        is_complete = True

    return response, content


  def AddRowsForUpload(self, rows, parent_test):
    if rows is None or len(rows) == 0:
      raise ValueError('Rows cannot be empty')

    # Take only the rows that have the r_commit_pos attribute set
    filtered_rows = []
    for row in rows:
      if hasattr(row, 'r_commit_pos'):
        filtered_rows.append(row)

    if len(filtered_rows) == 0:
      logging.info('No rows found with r_commit_pos attribute')
      return

    if parent_test is None:
      raise ValueError('Parent test cannot be None')

    row_data = [self._ConvertRowForSkiaUpload(row, parent_test)
                for row in filtered_rows]
    self._row_data_cache.extend(row_data)

  def _ConvertRowForSkiaUpload(self, row: graph_data.Row,
      parent_test: graph_data.TestMetadata):
    exact_row_properties = [
        'r_commit_pos',
        'timestamp',
        'd_count',
        'd_max',
        'd_min',
        'd_sum',
        'd_std',
        'd_avg',
        'value',
        'error',
        'a_benchmark_config',
        'a_build_uri',
        'a_tracing_uri',
        'a_stdio_uri',
        'a_bot_id',
        'a_os_detail_vers',
        'a_default_rev',
        'a_jobname',
        'r_chromium',
        'r_v8_rev',
        'r_webrtc_git',
        'r_chrome_version',
    ]

    def CopyItemAttrToDict(obj, properties, attr_is_property=False):
      prop_dict = {}
      for prop in properties:
        if hasattr(obj, prop):
          attr_value = getattr(obj, prop)
          if isinstance(attr_value, datetime):
            val = str(attr_value()) if attr_is_property else str(attr_value)
          else:
            val = attr_value() if attr_is_property else attr_value

          prop_dict[prop] = val
      return prop_dict

    row_item = CopyItemAttrToDict(row, exact_row_properties)
    test_item = {}
    test_item['test_path'] = utils.TestPath(parent_test.key)
    test_item['master_name'] = parent_test.master_name
    test_item['bot_name'] = parent_test.bot_name
    test_item['suite_name'] = parent_test.suite_name
    test_item['internal_only'] = parent_test.internal_only
    test_item['improvement_direction'] = parent_test.improvement_direction
    test_item['units'] = parent_test.units
    test_item['test_part1_name'] = parent_test.test_part1_name
    test_item['test_part2_name'] = parent_test.test_part2_name
    test_item['test_part3_name'] = parent_test.test_part3_name
    test_item['test_part4_name'] = parent_test.test_part4_name
    test_item['test_part5_name'] = parent_test.test_part5_name

    row_item['parent_test'] = test_item

    return row_item
