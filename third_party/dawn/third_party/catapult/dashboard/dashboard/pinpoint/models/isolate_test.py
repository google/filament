# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime

from unittest import mock

from google.appengine.ext import ndb

from dashboard.pinpoint.models.change import change_test
from dashboard.pinpoint.models import isolate
from dashboard.pinpoint import test


class IsolateTest(test.TestCase):

  def testPutAndGet(self):
    isolate_infos = (
        ('Mac Builder Perf', change_test.Change(1), 'target_name',
         'https://isolate.server', '7c7e90be'),
        ('Mac Builder Perf', change_test.Change(2), 'target_name',
         'https://isolate.server', '38e2f262'),
    )
    isolate.Put(isolate_infos)

    isolate_server, isolate_hash = isolate.Get('Mac Builder Perf',
                                               change_test.Change(1),
                                               'target_name')
    self.assertEqual(isolate_server, 'https://isolate.server')
    self.assertEqual(isolate_hash, '7c7e90be')

  def testUnknownIsolate(self):
    with self.assertRaises(KeyError):
      isolate.Get('Wrong Builder', change_test.Change(1), 'target_name')

  @mock.patch.object(isolate, 'datetime')
  def testExpiredIsolate(self, mock_datetime):
    isolate_infos = (('Mac Builder Perf', change_test.Change(1), 'target_name',
                      'https://isolate.server', '7c7e90be'),)
    isolate.Put(isolate_infos)

    # Teleport to the future after the isolate is expired.
    mock_datetime.datetime.utcnow.return_value = (
        datetime.datetime.utcnow() + isolate.ISOLATE_EXPIRY_DURATION +
        datetime.timedelta(days=1))
    mock_datetime.timedelta = datetime.timedelta

    with self.assertRaises(KeyError):
      isolate.Get('Mac Builder Perf', change_test.Change(1), 'target_name')

  def testDeleteExpiredIsolate(self):
    isolate_infos = (
        ('Mac Builder Perf', change_test.Change(0), 'target_name',
         'https://isolate.server', '123'),
        ('Mac Builder Perf', change_test.Change(1), 'target_name',
         'https://isolate.server', '456'),
    )
    isolate.Put(isolate_infos)

    cur = ndb.Key(
        'Isolate',
        isolate._Key('Mac Builder Perf', change_test.Change(0),
                     'target_name')).get()
    cur.created = datetime.datetime.utcnow() - datetime.timedelta(hours=1)
    cur.put()

    cur = ndb.Key(
        'Isolate',
        isolate._Key(isolate_infos[1][0], isolate_infos[1][1],
                     isolate_infos[1][2])).get()
    cur.created = datetime.datetime.utcnow() - (
        isolate.ISOLATE_EXPIRY_DURATION + datetime.timedelta(hours=1))
    cur.put()

    isolate.DeleteExpiredIsolates()

    q = isolate.Isolate.query()
    isolates = q.fetch()

    self.assertEqual(1, len(isolates))
    self.assertEqual('123', isolates[0].isolate_hash)
