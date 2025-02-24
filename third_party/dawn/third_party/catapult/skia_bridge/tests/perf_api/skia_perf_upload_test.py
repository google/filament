# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import

import json
import os
import sys
import zlib
from pathlib import Path
import unittest

app_path = Path(__file__).parent.parent.parent
if str(app_path) not in sys.path:
  sys.path.insert(0, str(app_path))

from application import app
from common import utils

from unittest import mock


class SkiaUploadTest(unittest.TestCase):
  def setUp(self):
    self.client = app.Create().test_client()
    os.environ['DISABLE_METRICS'] = 'True'
    curr_dir = Path(__file__).parent
    self.json_file = curr_dir.joinpath('sample_row_data.json')

  @mock.patch('application.perf_api.clients.cloud_storage_client'
              '.CloudStorageClient.UploadDataToBucket')
  def testUploadBasic(self, gcs_mock):
    with open(self.json_file) as f:
      row_data = f.read()

    request_obj = json.dumps({"rows": json.loads(row_data)})
    response = self.client.post(
        '/data/upload',
        data=request_obj)
    self.assertEqual(200, response.status_code)
    self.assertEqual(1, gcs_mock.call_count)

  @mock.patch('application.perf_api.clients.cloud_storage_client'
              '.CloudStorageClient.UploadDataToBucket')
  def testUploadBasicCompressed(self, gcs_mock):
    with open(self.json_file) as f:
      row_data = f.read()

    request_obj = json.dumps({"rows": json.loads(row_data)})
    response = self.client.post(
        '/data/upload',
        data=zlib.compress(request_obj.encode()),
        headers={'content-encoding': 'gzip'})
    self.assertEqual(200, response.status_code)
    self.assertEqual(1, gcs_mock.call_count)

  @mock.patch('application.perf_api.clients.cloud_storage_client'
                '.CloudStorageClient.UploadDataToBucket')
  def testUploadMultipleRowGroupsBasic(self, gcs_mock):
    with open(self.json_file) as f:
      row_data = f.read()

    rows = json.loads(row_data)
    rows[0]['parent_test']['master_name'] = 'RandomMaster'
    request_obj = json.dumps({"rows": rows})
    response = self.client.post(
        '/data/upload',
        data=request_obj)
    self.assertEqual(200, response.status_code)
    self.assertEqual(2, gcs_mock.call_count)

  @mock.patch('application.perf_api.clients.cloud_storage_client'
              '.CloudStorageClient.UploadDataToBucket')
  def testUploadInvalidTestMissing(self, gcs_mock):
    with open(self.json_file) as f:
      row_data = f.read()

    rows = json.loads(row_data)
    del rows[0]['parent_test']
    request_obj = json.dumps({"rows": rows})
    response = self.client.post(
        '/data/upload',
        data=request_obj)
    self.assertEqual(400, response.status_code)
    self.assertEqual(0, gcs_mock.call_count)

  @mock.patch('application.perf_api.clients.cloud_storage_client'
                '.CloudStorageClient.UploadDataToBucket')
  def testUploadInvalidTestKeysMissing(self, gcs_mock):
    with open(self.json_file) as f:
      row_data = f.read()

    rows = json.loads(row_data)
    del rows[0]['parent_test']['master_name']
    request_obj = json.dumps({"rows": rows})
    response = self.client.post(
        '/data/upload',
        data=request_obj)
    self.assertEqual(400, response.status_code)
    self.assertEqual(0, gcs_mock.call_count)

  @mock.patch('application.perf_api.clients.cloud_tasks_client'
                '.CloudTasksClient.EnqueueTask')
  def testUploadQueueInvalidTestMissing(self, cloud_tasks_mock):
    with open(self.json_file) as f:
      row_data = f.read()

    rows = json.loads(row_data)
    del rows[0]['parent_test']
    request_obj = json.dumps({"rows": rows})
    response = self.client.post(
        '/data/upload_queue',
        data=request_obj)
    self.assertEqual(400, response.status_code)
    self.assertEqual(0, cloud_tasks_mock.call_count)

  @mock.patch('application.perf_api.clients.cloud_tasks_client'
              '.CloudTasksClient.EnqueueTask')
  def testUploadQueueInvalidTestMissing(self, cloud_tasks_mock):
    with open(self.json_file) as f:
      row_data = f.read()

    rows = json.loads(row_data)
    del rows[0]['parent_test']
    request_obj = json.dumps({"rows": rows})
    response = self.client.post(
        '/data/upload_queue',
        data=request_obj)
    self.assertEqual(400, response.status_code)
    self.assertEqual(0, cloud_tasks_mock.call_count)

  @mock.patch('application.perf_api.clients.cloud_tasks_client'
              '.CloudTasksClient.EnqueueTask')
  def testUploadQueueBasic(self, cloud_tasks_mock):
    with open(self.json_file) as f:
      row_data = f.read()

    request_obj = json.dumps({"rows": json.loads(row_data)})
    response = self.client.post(
        '/data/upload_queue',
        data=request_obj)
    self.assertEqual(200, response.status_code)
    self.assertEqual(1, cloud_tasks_mock.call_count)

  @mock.patch('application.perf_api.clients.cloud_tasks_client'
              '.CloudTasksClient.EnqueueTask')
  def testUploadQueueLargePayload(self, cloud_tasks_mock):
    with open(self.json_file) as f:
      row_data = f.read()

    request_obj = json.dumps({"rows": json.loads(row_data)})

    # Set a low value so that the api adds compression
    utils.PAYLOAD_SIZE_LIMIT_KB = 0.05

    response = self.client.post(
        '/data/upload_queue',
        data=request_obj)
    self.assertEqual(200, response.status_code)
    self.assertEqual(1, cloud_tasks_mock.call_count)
    args, _ = cloud_tasks_mock.call_args
    headers = args[-1]
    self.assertIsNotNone(headers.get('content-encoding'))
    self.assertEqual(headers['content-encoding'], 'gzip')


if __name__ == '__main__':
  unittest.main()
