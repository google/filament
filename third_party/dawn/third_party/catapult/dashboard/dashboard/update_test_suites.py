# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Functions for fetching and updating a list of top-level tests."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import collections
import logging

from google.appengine.api import datastore_errors
from google.appengine.ext import ndb

from dashboard.common import datastore_hooks
from dashboard.common import descriptor
from dashboard.common import stored_object
from dashboard.common import namespaced_stored_object
from dashboard.common import utils
from dashboard.models import graph_data

from flask import request, make_response

# TestMetadata suite cache key.
_LIST_SUITES_CACHE_KEY = 'list_tests_get_test_suites'

TEST_SUITES_2_CACHE_KEY = 'test_suites_2'


@ndb.synctasklet
def FetchCachedTestSuites2():
  result = yield FetchCachedTestSuites2Async()
  raise ndb.Return(result)


@ndb.tasklet
def FetchCachedTestSuites2Async():
  results = yield namespaced_stored_object.GetAsync(TEST_SUITES_2_CACHE_KEY)
  raise ndb.Return(results)


def FetchCachedTestSuites():
  """Fetches cached test suite data."""
  cached = namespaced_stored_object.Get(_LIST_SUITES_CACHE_KEY)
  if cached is None:
    # If the cache test suite list is not set, update it before fetching.
    # This is for convenience when testing sending of data to a local instance.
    UpdateTestSuites(datastore_hooks.GetNamespace())
    cached = namespaced_stored_object.Get(_LIST_SUITES_CACHE_KEY)
  return cached


def UpdateTestSuitesPost():
  if request.values.get('internal_only') == 'true':
    logging.info('Going to update internal-only test suites data.')
    # Update internal-only test suites data.
    datastore_hooks.SetPrivilegedRequest()
    UpdateTestSuites(datastore_hooks.INTERNAL)
  else:
    logging.info('Going to update externally-visible test suites data.')
    # Update externally-visible test suites data.
    UpdateTestSuites(datastore_hooks.EXTERNAL)
  return make_response('')


def UpdateTestSuites(permissions_namespace):
  """Updates test suite data for either internal or external users."""
  logging.info('Updating test suite data for: %s', permissions_namespace)
  suite_dict = _CreateTestSuiteDict()
  key = namespaced_stored_object.NamespaceKey(_LIST_SUITES_CACHE_KEY,
                                              permissions_namespace)
  stored_object.Set(key, suite_dict)

  stored_object.Set(
      namespaced_stored_object.NamespaceKey(TEST_SUITES_2_CACHE_KEY,
                                            permissions_namespace),
      _ListTestSuites())


@ndb.tasklet
def _ListTestSuitesAsync(test_suites, partial_tests, parent_test=None):
  # Some test suites are composed of multiple test path components. See
  # Descriptor. When a TestMetadata key doesn't contain enough test path
  # components to compose a full test suite, add its key to partial_tests so
  # that the caller can run another query with parent_test.
  logging.debug("list test suites async parent test: %s\npartial tests: %s",
                parent_test, partial_tests)
  query = graph_data.TestMetadata.query()
  query = query.filter(graph_data.TestMetadata.parent_test == parent_test)
  query = query.filter(graph_data.TestMetadata.deprecated == False)
  # hack to use composite index and capture all descriptions in DataStore
  # description is unfortunately defined as part of the composite index:
  # https://chromium.googlesource.com/catapult.git/+/HEAD/dashboard/index.yaml#317
  query = query.filter(graph_data.TestMetadata.description != "made_up_description")
  keys = yield query.fetch_async(keys_only=True)
  for key in keys:
    test_path = utils.TestPath(key)
    desc = yield descriptor.Descriptor.FromTestPathAsync(test_path)
    if desc.test_suite:
      test_suites.add(desc.test_suite)
    elif partial_tests is not None:
      partial_tests.add(key)
    else:
      logging.error('Unable to parse "%s"', test_path)
  logging.debug("list test suites: %s", test_suites)


@ndb.synctasklet
def _ListTestSuites():
  test_suites = set()
  partial_tests = set()
  yield _ListTestSuitesAsync(test_suites, partial_tests)
  yield [_ListTestSuitesAsync(test_suites, None, key) for key in partial_tests]
  test_suites = sorted(test_suites)
  raise ndb.Return(test_suites)


def _CreateTestSuiteDict():
  """Returns a dictionary with information about top-level tests.

  This method is used to generate the global JavaScript variable TEST_SUITES
  for the report page. This variable is used to initially populate the select
  menus.

  Note that there will be multiple top level TestMetadata entities for each
  suite name, since each suite name appears under multiple bots.

  Returns:
    A dictionary of the form:
      {
          'my_test_suite': {
              'mas': {'ChromiumPerf': {'mac': False, 'linux': False}},
              'dep': True,
              'des': 'A description.'
          },
          ...
      }

    Where 'mas', 'dep', and 'des' are abbreviations for 'masters',
    'deprecated', and 'description', respectively.
  """
  result = collections.defaultdict(lambda: {'mas': {}, 'dep': True})

  for s in _FetchSuites():
    v = result[s.test_name]

    if 'des' not in v and s.description:
      v['des'] = s.description

    # Only depreccate when all tests are deprecated
    v['dep'] &= s.deprecated

    if s.master_name not in v['mas']:
      v['mas'][s.master_name] = {}
    if s.bot_name not in v['mas'][s.master_name]:
      v['mas'][s.master_name][s.bot_name] = s.deprecated

  result.default_factory = None
  return result


def _FetchSuites():
  """Fetches Tests with deprecated and description projections."""
  suite_query = graph_data.TestMetadata.query(
      graph_data.TestMetadata.parent_test == None)
  cursor = None
  more = True
  try:
    while more:
      some_suites, cursor, more = suite_query.fetch_page(
          2000,
          start_cursor=cursor,
          projection=['deprecated', 'description'],
          use_cache=False,
          use_memcache=False)
      for s in some_suites:
        yield s
  except datastore_errors.Timeout:
    logging.error('Timeout fetching test suites.')


def _GetTestSubPath(key):
  """Gets the part of the test path after the suite, for the given test key.

  For example, for a test with the test path 'MyMaster/bot/my_suite/foo/bar',
  this should return 'foo/bar'.

  Args:
    key: The key of the TestMetadata entity.

  Returns:
    Slash-separated test path part after master/bot/suite.
  """
  return '/'.join(p for p in key.string_id().split('/')[3:])
