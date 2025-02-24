# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from flask import Flask
import unittest

from google.appengine.ext import ndb

from dashboard.common import datastore_hooks
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import graph_data


class FakeRequest:

  def __init__(self):
    self.registry = {}


class DatastoreHooksTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    testing_common.SetIsInternalUser('internal@chromium.org', True)
    testing_common.SetIsInternalUser('foo@chromium.org', False)
    self._AddDataToDatastore()
    datastore_hooks.InstallHooks()
    self.PatchDatastoreHooksRequest()

  def tearDown(self):
    super().tearDown()
    self.UnsetCurrentUser()

  def _AddDataToDatastore(self):
    """Puts a set of entities; some internal-only, some not."""
    # Need to be privileged to add TestMetadata and Row objects to the datastore
    # because there is a get() for the parent_test in the pre_put_hook. This
    # should work correctly in production because Rows and TestMetadata should
    # only be added by /add_point, which is privileged.
    self.SetCurrentUser('internal@chromium.org')
    testing_common.AddTests(
        ['ChromiumPerf'], ['Win7External', 'FooInternal'], {
            'TestInternal': {
                'SubTestInternal': {}
            },
            'TestExternal': {
                'SubTestExternal': {}
            },
        })
    internal_key = ['Master', 'ChromiumPerf', 'Bot', 'FooInternal']
    internal_bot = ndb.Key(*internal_key).get()
    internal_bot.internal_only = True
    internal_bot.put()
    internal_test = ndb.Key('TestMetadata',
                            'ChromiumPerf/Win7External/TestInternal').get()
    internal_test.internal_only = True
    internal_test.put()
    internal_test = ndb.Key('TestMetadata',
                            'ChromiumPerf/FooInternal/TestInternal').get()
    internal_test.internal_only = True
    internal_test.put()
    internal_sub_test = ndb.Key(
        'TestMetadata',
        'ChromiumPerf/Win7External/TestInternal/SubTestInternal').get()
    internal_sub_test.internal_only = True
    internal_sub_test.put()
    internal_sub_test = ndb.Key(
        'TestMetadata',
        'ChromiumPerf/FooInternal/TestInternal/SubTestInternal').get()
    internal_sub_test.internal_only = True
    internal_sub_test.put()

    internal_key = internal_sub_test.key
    external_key = ndb.Key(
        'TestMetadata',
        'ChromiumPerf/Win7External/TestExternal/SubTestExternal')

    internal_test_container_key = utils.GetTestContainerKey(internal_key)
    external_test_container_key = utils.GetTestContainerKey(external_key)
    for i in range(0, 100, 10):
      graph_data.Row(
          parent=internal_test_container_key,
          id=i,
          value=float(i * 2),
          internal_only=True).put()
      graph_data.Row(
          parent=external_test_container_key, id=i, value=float(i * 2)).put()
    self.UnsetCurrentUser()

  def _CheckQueryResults(self, include_internal):
    """Asserts that the expected entities are fetched.

    The expected entities are the ones added in |_AddDataToDatastore|.

    Args:
      include_internal: Whether or not internal-only entities are included
          in the set of expected entities to be fetched.
    """
    bots = graph_data.Bot.query().fetch()
    if include_internal:
      self.assertEqual(2, len(bots))
      self.assertEqual('FooInternal', bots[0].key.string_id())
      self.assertEqual('Win7External', bots[1].key.string_id())
    else:
      self.assertEqual(1, len(bots))
      self.assertEqual('Win7External', bots[0].key.string_id())

    tests = graph_data.TestMetadata.query().fetch()
    if include_internal:
      self.assertEqual(8, len(tests))
      self.assertEqual('ChromiumPerf/FooInternal/TestExternal',
                       tests[0].key.string_id())
      self.assertEqual('ChromiumPerf/FooInternal/TestExternal/SubTestExternal',
                       tests[1].key.string_id())
      self.assertEqual('ChromiumPerf/FooInternal/TestInternal',
                       tests[2].key.string_id())
      self.assertEqual('ChromiumPerf/FooInternal/TestInternal/SubTestInternal',
                       tests[3].key.string_id())
      self.assertEqual('ChromiumPerf/Win7External/TestExternal',
                       tests[4].key.string_id())
      self.assertEqual('ChromiumPerf/Win7External/TestExternal/SubTestExternal',
                       tests[5].key.string_id())
      self.assertEqual('ChromiumPerf/Win7External/TestInternal',
                       tests[6].key.string_id())
      self.assertEqual('ChromiumPerf/Win7External/TestInternal/SubTestInternal',
                       tests[7].key.string_id())
    else:
      self.assertEqual(4, len(tests))
      self.assertEqual('ChromiumPerf/FooInternal/TestExternal',
                       tests[0].key.string_id())
      self.assertEqual('ChromiumPerf/FooInternal/TestExternal/SubTestExternal',
                       tests[1].key.string_id())
      self.assertEqual('ChromiumPerf/Win7External/TestExternal',
                       tests[2].key.string_id())
      self.assertEqual('ChromiumPerf/Win7External/TestExternal/SubTestExternal',
                       tests[3].key.string_id())

    tests = graph_data.TestMetadata.query(
        graph_data.TestMetadata.master_name == 'ChromiumPerf',
        graph_data.TestMetadata.bot_name == 'FooInternal').fetch()
    if include_internal:
      self.assertEqual(4, len(tests))
    else:
      self.assertEqual(2, len(tests))

  def testQuery_NoUser_InternalOnlyNotFetched(self):
    self.UnsetCurrentUser()
    self._CheckQueryResults(include_internal=False)

  def testQuery_ExternalUser_InternalOnlyNotFetched(self):
    self.SetCurrentUser('foo@chromium.org')
    self._CheckQueryResults(include_internal=False)

  def testQuery_InternalUser_InternalOnlyFetched(self):
    self.SetCurrentUser('internal@chromium.org')
    self._CheckQueryResults(True)

  def testQuery_PrivilegedRequest_InternalOnlyFetched(self):
    app = Flask(__name__)
    with app.test_request_context('dummy/path', 'GET'):
      self.UnsetCurrentUser()
      datastore_hooks.SetPrivilegedRequest()
      self._CheckQueryResults(True)

  def testQuery_SinglePrivilegedRequest_InternalOnlyFetched(self):
    app = Flask(__name__)
    with app.test_request_context('dummy/path', 'GET'):
      self.UnsetCurrentUser()
      datastore_hooks.SetSinglePrivilegedRequest()
      # Not using _CheckQueryResults because this only affects a single query.
      # First query has internal results.
      bots = graph_data.Bot.query().fetch()
      self.assertEqual(2, len(bots))

      # Second query does not.
      bots = graph_data.Bot.query().fetch()
      self.assertEqual(1, len(bots))

  def _CheckGet(self, include_internal):
    m = ndb.Key('Master', 'ChromiumPerf').get()
    self.assertEqual(m.key.string_id(), 'ChromiumPerf')
    external_bot = ndb.Key('Master', 'ChromiumPerf', 'Bot',
                           'Win7External').get()
    self.assertEqual(external_bot.key.string_id(), 'Win7External')
    external_bot_2 = graph_data.Bot.get_by_id('Win7External', parent=m.key)
    self.assertEqual(external_bot_2.key.string_id(), 'Win7External')
    external_test = ndb.Key(
        'TestMetadata',
        'ChromiumPerf/Win7External/TestExternal/SubTestExternal').get()
    self.assertEqual('ChromiumPerf/Win7External/TestExternal/SubTestExternal',
                     external_test.key.string_id())
    if include_internal:
      internal_bot = ndb.Key('Master', 'ChromiumPerf', 'Bot',
                             'FooInternal').get()
      self.assertEqual(internal_bot.key.string_id(), 'FooInternal')
      internal_bot_2 = graph_data.Bot.get_by_id('FooInternal', parent=m.key)
      self.assertEqual(internal_bot_2.key.string_id(), 'FooInternal')
    else:
      k = ndb.Key('Master', 'ChromiumPerf', 'Bot', 'FooInternal')
      self.assertRaises(AssertionError, k.get)
      self.assertRaises(
          AssertionError, graph_data.Bot.get_by_id, 'FooInternal', parent=m.key)

  def testGet_NoUser(self):
    self.UnsetCurrentUser()
    self._CheckGet(include_internal=False)

  def testGet_ExternalUser(self):
    self.SetCurrentUser('foo@chromium.org')
    self._CheckGet(include_internal=False)

  def testGet_InternalUser(self):
    self.SetCurrentUser('internal@chromium.org')
    self._CheckGet(include_internal=True)

  def testGet_AdminUser(self):
    self.SetCurrentUser('foo@chromium.org', is_admin=True)
    self._CheckGet(include_internal=True)

  def testGet_PrivilegedRequest(self):
    app = Flask(__name__)
    with app.test_request_context('dummy/path', 'GET'):
      self.UnsetCurrentUser()
      datastore_hooks.SetPrivilegedRequest()
      self._CheckGet(include_internal=True)


if __name__ == '__main__':
  unittest.main()
