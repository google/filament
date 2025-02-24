# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import six.moves.cPickle as cPickle
import datetime
from unittest import mock
import unittest

from google.appengine.ext import ndb
from google.appengine.runtime import apiproxy_errors

from dashboard.common import layered_cache
from dashboard.common import stored_object
from dashboard.common import testing_common


class LayeredCacheTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.UnsetCurrentUser()
    testing_common.SetIsInternalUser('internal@chromium.org', True)
    testing_common.SetIsInternalUser('foo@chromium.org', False)

  def testSetAndGet(self):
    self.SetCurrentUser('internal@chromium.org')
    layered_cache.Set('str', 'Hello, World!')
    layered_cache.Set('dict', {'hello': [1, 2, 3]})
    self.assertEqual(
        'Hello, World!',
        cPickle.loads(
            ndb.Key('CachedPickledString', 'internal_only__str').get().value))
    self.assertIsNone(
        ndb.Key('CachedPickledString', 'externally_visible__str').get())
    self.assertEqual('Hello, World!', layered_cache.Get('str'))
    self.SetCurrentUser('foo@chromium.org')
    self.assertIsNone(layered_cache.Get('str'))
    self.SetCurrentUser('internal@chromium.org')
    self.assertEqual({'hello': [1, 2, 3]},
                     cPickle.loads(
                         ndb.Key('CachedPickledString',
                                 'internal_only__dict').get().value))
    self.assertIsNone(
        ndb.Key('CachedPickledString', 'externally_visible__dict').get())
    self.assertEqual({'hello': [1, 2, 3]}, layered_cache.Get('dict'))
    self.SetCurrentUser('foo@chromium.org')
    self.assertIsNone(layered_cache.Get('dict'))

  def testGetAndSet_External(self):
    layered_cache.SetExternal('str', 'Hello, World!')
    layered_cache.SetExternal('dict', {'hello': [1, 2, 3]})
    self.assertEqual(
        'Hello, World!',
        cPickle.loads(
            ndb.Key('CachedPickledString',
                    'externally_visible__str').get().value))
    self.assertEqual(None,
                     ndb.Key('CachedPickledString', 'internal_only__str').get())
    self.assertEqual('Hello, World!', layered_cache.GetExternal('str'))
    self.assertEqual({'hello': [1, 2, 3]},
                     cPickle.loads(
                         ndb.Key('CachedPickledString',
                                 'externally_visible__dict').get().value))
    self.assertEqual(
        None,
        ndb.Key('CachedPickledString', 'internal_only__dict').get())
    self.assertEqual({'hello': [1, 2, 3]}, layered_cache.GetExternal('dict'))

  def testDelete(self):
    self.SetCurrentUser('internal@chromium.org')
    layered_cache.Set('hello', 'secret')
    self.SetCurrentUser('foo@chromium.org')
    layered_cache.Set('hello', 'not secret')
    layered_cache.Delete('hello')
    self.SetCurrentUser('internal@chromium.org')
    self.assertIsNone(layered_cache.Get('hello'))
    self.SetCurrentUser('foo@chromium.org')
    self.assertIsNone(layered_cache.Get('hello'))

  def testExpireTime(self):
    self.SetCurrentUser('internal@chromium.org')
    layered_cache.Set('str1', 'Hello, World!', days_to_keep=10)
    key_internal = ndb.Key('CachedPickledString', 'internal_only__str1')
    key_external = ndb.Key('CachedPickledString', 'externally_visible__str1')
    self.assertEqual('Hello, World!', cPickle.loads(key_internal.get().value))
    self.assertIsNone(key_external.get())
    self.assertEqual('Hello, World!', layered_cache.Get('str1'))

    # The expire date should be 10 days after the current date.
    actual_date = key_internal.get().expire_time
    expected_date = datetime.datetime.now() + datetime.timedelta(days=10)
    self.assertEqual(actual_date.date(), expected_date.date())

    # When current user is external, the external version is returned by Get.
    self.SetCurrentUser('foo@chromium.org')
    self.assertIsNone(layered_cache.Get('str1'))

  def testDeleteAllExpiredEntities(self):
    self.SetCurrentUser('internal@chromium.org')
    layered_cache.Set('expired_str1', 'apple', days_to_keep=-10)
    layered_cache.Set('expired_str2', 'bat', days_to_keep=-1)
    layered_cache.Set('expired_str3', 'cat', days_to_keep=10)
    layered_cache.Set('expired_str4', 'dog', days_to_keep=0)
    layered_cache.Set('expired_str5', 'egg')
    self.assertEqual('apple', layered_cache.Get('expired_str1'))
    self.assertEqual('bat', layered_cache.Get('expired_str2'))
    self.assertEqual('cat', layered_cache.Get('expired_str3'))
    self.assertEqual('dog', layered_cache.Get('expired_str4'))
    self.assertEqual('egg', layered_cache.Get('expired_str5'))
    layered_cache.DeleteAllExpiredEntities()
    self.assertIsNone(layered_cache.Get('expired_str1'))
    self.assertIsNone(layered_cache.Get('expired_str2'))
    self.assertEqual('cat', layered_cache.Get('expired_str3'))
    self.assertEqual('dog', layered_cache.Get('expired_str4'))
    self.assertEqual('egg', layered_cache.Get('expired_str5'))

  @mock.patch.object(layered_cache.CachedPickledString, 'put')
  def testSet_TooBig(self, mock_cached_pickled_string):
    e = apiproxy_errors.RequestTooLargeError('too big!')
    mock_cached_pickled_string.side_effect = e
    layered_cache.Set('foo', 'bar')
    self.assertEqual('bar', stored_object.Get('foo'))

  def testGet_StoredObject(self):
    stored_object.Set('foo', 'bar')
    self.assertEqual('bar', layered_cache.Get('foo'))


if __name__ == '__main__':
  unittest.main()
