# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from unittest import mock

from dashboard.models import graph_data
from dashboard.services import skia_bridge_service

def _CreateRows(count:int):
  rows = []
  if count > 0:
    for i in range(count):
      row = graph_data.Row()
      row.value = i
      row.r_commit_pos = 1234
      rows.append(row)

  return rows

class SkiaBridgeServiceTest(unittest.TestCase):

  @mock.patch('dashboard.services.skia_bridge_service.SkiaServiceClient'
              '.SendSkiaBridgeRequest')
  def testSendRow(self, request_mock):
    request_mock.return_value = {'status': '200'}, 'Ok'
    rows = _CreateRows(5)
    parent_test = graph_data.TestMetadata(id='Chromeperf/skia/test')
    skia_client = skia_bridge_service.SkiaServiceClient()
    skia_client.AddRowsForUpload(rows, parent_test)
    skia_client.SendRowsToSkiaBridge()
    self.assertEqual(1, request_mock.call_count)

  def testInvalidParentTest(self):
    rows = _CreateRows(1)
    skia_client = skia_bridge_service.SkiaServiceClient()
    self.assertRaises(ValueError,
                      skia_client.AddRowsForUpload, rows, None)

  @mock.patch('dashboard.services.skia_bridge_service.SkiaServiceClient'
              '.SendSkiaBridgeRequest')
  def testRowWithoutCommitPos(self, request_mock):
    rows = _CreateRows(1)
    delattr(rows[0], 'r_commit_pos')
    parent_test = graph_data.TestMetadata(id='Chromeperf/skia/test')
    skia_client = skia_bridge_service.SkiaServiceClient()
    skia_client.AddRowsForUpload(rows, parent_test)
    skia_client.SendRowsToSkiaBridge()
    self.assertEqual(0, request_mock.call_count)

  def testInvalidRowsTest(self):
    rows = _CreateRows(0)
    parent_test = graph_data.TestMetadata(id='Chromeperf/skia/test')
    skia_client = skia_bridge_service.SkiaServiceClient()
    self.assertRaises(ValueError,
                      skia_client.AddRowsForUpload, rows, parent_test)
