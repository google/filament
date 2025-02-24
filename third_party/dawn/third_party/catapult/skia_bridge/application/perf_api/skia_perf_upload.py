# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json
import logging
import re
import random
import string
import sys
import zlib
from dateutil.parser import parse
from common import utils
from application.perf_api.clients import cloud_storage_client, \
  cloud_tasks_client

from flask import Blueprint, request, make_response

_TASK_QUEUE_NAME = 'skia-upload'
_TASK_QUEUE_LOCATION = 'us-central1'

PUBLIC_BUCKET_NAME = 'chrome-perf-public'
INTERNAL_BUCKET_NAME = 'chrome-perf-non-public'

blueprint = Blueprint('skia_perf_upload', __name__)


@blueprint.route('/', methods=['GET'])
def SkiaPerfUploadGet():
  return 'Ok'


@blueprint.route(
    '/upload_queue', methods=['POST'], endpoint='SkiaPerfUploadQueuePost')
def SkiaPerfUploadQueuePost():
  try:
    task_client = cloud_tasks_client.CloudTasksClient()

    request_data = json.loads(request.data)
    logging.info('Received request: %s' % request_data)
    row_data = request_data['rows']
    is_valid, error = ValidateRows(row_data)
    if not is_valid:
      # Error in the incoming data
      return error, 400

    headers = {'Content-Type': 'application/json'}
    rows = {
        "rows": row_data
    }

    # TODO: We can potentially also chunk the rows into multiple tasks
    # along with compression.
    payload_size_kb = sys.getsizeof(row_data) / 1024
    if payload_size_kb > utils.PAYLOAD_SIZE_LIMIT_KB:
      rows = zlib.compress(json.dumps(rows).encode())
      headers['content-encoding'] = 'gzip'


    host = utils.GetSkiaBridgeUrl()
    url = '%sdata/upload' % host

    task_client.EnqueueTask(
          utils.GetGcloudProject(),
          _TASK_QUEUE_NAME,
          _TASK_QUEUE_LOCATION,
          rows,
          url,
          headers)
    return 'Ok'
  except Exception as e:
    logging.error(e)
    return 'Unexpected error during data upload.', 500


@blueprint.route('/upload', methods=['POST'])
def SkiaPerfUploadPost():
  """ Upload a list of ChromePerf Rows to Skia Perf.
  """

  logging.info('Headers: %s', request.headers)
  logging.info('Payload size = %s', sys.getsizeof(request.data))
  try:
    if request.headers.get('content-encoding') and \
        request.headers['content-encoding'] == 'gzip':
      request_data = json.loads(zlib.decompress(request.data).decode())
    else:
      request_data = json.loads(request.data)

    rows = request_data['rows']
    logging.info('Total Rows: %i', len(rows))
    gcs_client = cloud_storage_client.CloudStorageClient()
    is_valid, error = ValidateRows(rows)
    if not is_valid:
      # Error in the incoming data
      return error, 400

    row_groups = CreateRowGroups(rows)
    logging.info('Total Row groups: %i', len(row_groups))
    for group_key in row_groups:
      row_group = row_groups[group_key]

      test_path = row_group[0]['parent_test']['test_path']
      try:
        UploadRowGroup(gcs_client, row_group)

      except Exception as e:  # pylint: disable=broad-except
        logging.error('Failed to upload Row with Test: %s. Error: %s.',
                      test_path, str(e))
        raise RuntimeError from e
  except Exception as e:  # pylint: disable=broad-except
    logging.error(e)
    return 'Unexpected error during data upload.', 500

  return make_response('')


def ValidateTest(test):
  required_keys = ['test_path', 'master_name', 'bot_name', 'suite_name',
                   'improvement_direction', 'units', 'internal_only']
  for key in required_keys:
    if key not in test:
      return False, '%s key missing in parent_test' % key

  return True, None


def ValidateRow(row):
  required_keys = ['parent_test']
  for key in required_keys:
    if key not in row:
      return False, '%s key missing in row data' % key

  parent_test = row['parent_test']
  return ValidateTest(parent_test)


def ValidateRows(row_group):

  for row in row_group:
    valid, error = ValidateRow(row)
    if not valid:
      return valid, error

  return True, None


def CreateRowGroups(rows):
  row_groups = {}
  for row in rows:
    test_data = row['parent_test']

    if not row.get('a_bot_id'):
      row['a_bot_id'] = ['Unspecified']

    # a_bot_id is a list, so let's get a str representation
    bot_list = ','.join(sorted(row['a_bot_id']))
    row_key = '%s->%s->%s->%s->%s->%s->%s' % (test_data['master_name'],
                                      test_data['bot_name'],
                                      test_data['suite_name'],
                                      test_data['internal_only'],
                                      row['r_commit_pos'],
                                      bot_list,
                                      GetTimeStamp(row['timestamp']))
    if not row_groups.get(row_key):
      row_groups[row_key] = []

    row_groups[row_key].append(row)

  return row_groups


def GetTimeStamp(timestamp_str):
  date_obj = parse(timestamp_str)
  return date_obj.strftime('%Y/%m/%d/%H')

def UploadRowGroup(gcs_client, rows):
  """
  Uploads a row group to GCS
  """

  # Currently, we only support rows with a Chromium commit position, as it's
  # the only format our Skia Perf instance can support.
  commit_pos = rows[0].get('r_commit_pos')
  if not commit_pos:
    raise RuntimeError('Row has no Chromium commit position')

  test_data = rows[0]['parent_test']
  timestamp = GetTimeStamp(rows[0]['timestamp'])
  master = test_data['master_name']
  bot = test_data['bot_name']
  benchmark = test_data['suite_name']

  skia_data = _ConvertRowGroupToSkiaPerf(
      rows,
      master,
      bot,
      benchmark,
      commit_pos)

  internal_only = test_data['internal_only']

  random_suffix = ''.join(random.choices(string.ascii_letters, k=10))
  filename = 'ingest/%s/%s/%s/%s/%s-%s.json' % (
      timestamp,
      master,
      bot,
      benchmark,
      '-'.join([str(commit_pos), bot]),
      random_suffix)

  # All data goes to internal bucket (both public and internal_only)
  gcs_client.UploadDataToBucket(filename, json.dumps(skia_data),
                                INTERNAL_BUCKET_NAME, 'application/json')
  if not internal_only:
    # Upload to public bucket only if it's not internal_only
    gcs_client.UploadDataToBucket(filename, json.dumps(skia_data),
                                  PUBLIC_BUCKET_NAME, 'application/json')


  logging.info('Uploaded row to %s', filename)

def _ConvertRowGroupToSkiaPerf(rows, master, bot, benchmark, commit_position):

  skia_data = {
      'version': 1,
      'git_hash': 'CP:%s' % str(commit_position),
      'key': {
          'master': master,
          'bot': bot,
          'benchmark': benchmark,
      },
      'results': [{
          'measurements': {
              'stat': _GetStatsForRow(row)
          },
          'key': _GetMeasurementKey(row)
      } for row in rows],
      'links': _GetLinks(rows[0])
  }

  return skia_data


def _GetStatsForRow(row):
  stats = []
  stats_key_map = [
      ('value', 'value'),
      ('error', 'error'),
      ('d_count', 'count'),
      ('d_max', 'max'),
      ('d_min', 'min'),
      ('d_sum', 'sum'),
      ('d_std', 'std'),
      ('d_avg', 'avg')
  ]

  for key, skia_key in stats_key_map:
    if key in row.keys() and _IsNumber(row[key]):
      stats.append({'value': skia_key, 'measurement': row[key]})

  return stats


def _GetMeasurementKey(row):
  measurement_key = {}

  # Row data specifies 0/1 as directions, but skia expects
  # equivalent string representations
  improvement_directions_map = {0: 'up', 1: 'down'}
  original_imp_dir = row['parent_test']['improvement_direction']
  if not improvement_directions_map.get(original_imp_dir):
    measurement_key['improvement_direction'] = 'unknown'
  else:
    measurement_key['improvement_direction'] = \
      improvement_directions_map[original_imp_dir]


  measurement_key['unit'] = row['parent_test']['units']

  parts = row['parent_test']['test_path'].split('/')

  key_map = [
      'test',
      'subtest_1',
      'subtest_2',
      'subtest_3',
      'subtest_4',
      'subtest_5',
      'subtest_6',
      'subtest_7',
  ]
  if len(parts) >= 4:
    for i in range(3, len(parts)):
      if parts[i]:
        measurement_key[key_map[i-3]] = parts[i]
      else:
        break
  return measurement_key


def _GetLinks(row):
  links = {}

  links['Bot Id'] = ', '.join(sorted(row['a_bot_id']))

  # Annotations
  if row.get('a_benchmark_config'):
    links['Benchmark Config'] = row['a_benchmark_config']
  if row.get('a_build_uri'):
    build_uri = row['a_build_uri']
    m = re.search(r'\[Build Status\]\((.+?)\)', build_uri)
    if m:
      build_uri = m.group(1)

    links['Build Page'] = build_uri
  if row.get('a_tracing_uri'):
    links['Tracing uri'] = row['a_tracing_uri']
  if row.get('a_stdio_uri'):
    links['Test stdio'] = row['a_stdio_uri']
  if row.get('a_os_detail_vers'):
    links['OS Version'] = ','.join(row['a_os_detail_vers'])
  if row.get('a_default_rev'):
    links['Default Revision'] = row['a_default_rev']
  if row.get('a_jobname'):
    links['Swarming Job Name'] = row['a_jobname']

  # Revisions
  if row.get('r_commit_pos') and row['r_commit_pos']:
    links[
        'Chromium Commit Position'] = 'https://crrev.com/%s' % \
                                      row['r_commit_pos']
  if row.get('r_chromium') and row['r_chromium']:
    links['Chromium Git Hash'] = (
        'https://chromium.googlesource.com/chromium/src/+/%s' %
        row['r_chromium'])
  if row.get('r_v8_rev') and row['r_v8_rev']:
    links[
        'V8 Git Hash'] = 'https://chromium.googlesource.com/v8/v8/+/%s' % \
                         row['r_v8_rev']
  if row.get('r_webrtc_git') and row['r_webrtc_git']:
    links[
        'WebRTC Git Hash'] = 'https://webrtc.googlesource.com/src/+/%s' % \
                             row['r_webrtc_git']
  if row.get('r_chrome_version') and row['r_chrome_version']:
    links['Chrome Version'] = (
        'https://chromium.googlesource.com/chromium/src/+/%s' %
        row['r_chrome_version'])

  return links


def _IsNumber(v):
  return isinstance(v, (float, int))
