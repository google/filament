# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Functions for making lists of tests, and an AJAX endpoint to list tests.

This module contains functions for listing:
 - Sub-tests for a given test suite (in a tree structure).
 - Tests which match a given test path pattern.
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json

from google.appengine.ext import ndb

from dashboard.common import layered_cache
from dashboard.common import request_handler
from dashboard.common import utils
from dashboard.models import graph_data


class BadRequestError(Exception):
  pass

# URL endpoint for AJAX requests to list masters, bots, and tests.
from flask import make_response, request


def ListTestsHandlerPost():
  """Outputs a JSON string of the requested list.

  This handler handles 3 different type of requests for historical reasons.
  These could potentially be separated into 3 separate handlers.

  Request parameters:
    type: Type of list to make, one of "suite", "sub_tests" or "pattern".
    suite: Test suite name (applies only if type is "sub_tests").
    bots: Comma-separated bots name (applies only if type is "sub_tests").
    p: Test path pattern (applies only if type is "pattern").
    has_rows: "1" if the requester wants to list only list tests that
        have points (applies only if type is "pattern").
    test_path_dict: A test path dict having the format specified in
        GetTestsForTestPathDict, below (applies only if type is
        "test_path_dict").
    return_selected: "1" if the requester wants to return selected tests,
        otherwise unselected tests will be returned (applies only if type is
        "test_path_dict").

  Outputs:
    A data structure with test names in JSON format, or nothing.
  """
  headers = {'Access-Control-Allow-Origin': '*'}
  list_type = request.values.get('type')

  if list_type == 'sub_tests':
    suite_name = request.values.get('suite')
    bot_names = request.values.get('bots').split(',')
    test_dict = GetSubTests(suite_name, bot_names)
    return make_response(json.dumps(test_dict), headers)

  if list_type == 'pattern':
    pattern = request.values.get('p')
    only_with_rows = request.values.get('has_rows') == '1'
    test_list = GetTestsMatchingPattern(pattern, only_with_rows=only_with_rows)
    return make_response(json.dumps(test_list), headers)

  if list_type == 'test_path_dict':
    test_path_dict = request.values.get('test_path_dict')
    return_selected = request.values.get('return_selected') == '1'
    try:
      test_list = GetTestsForTestPathDict(
          json.loads(test_path_dict), return_selected)
      return make_response(json.dumps(test_list), headers)
    except BadRequestError as e:
      return request_handler.RequestHandlerReportError(str(e), status=400)

  return make_response(
      json.dumps({'error': 'Unknown list_type: %s' % list_type}), 404)


def GetSubTests(suite_name, bot_names):
  """Gets the entire tree of subtests for the suite with the given name.

  Each bot may have different sub-tests available, but there is one combined
  sub-tests dict returned for all the bots specified.

  This method is used by the test-picker select menus to display what tests
  are available; only tests that are not deprecated should be listed.

  Args:
    suite_name: Top level test name.
    bot_names: List of master/bot names in the form "<master>/<platform>".

  Returns:
    A dict mapping test names to dicts to entries which have the keys
    "has_rows" (boolean) and "sub_tests", which is another sub-tests dict.
    This forms a tree structure.
  """
  # For some bots, there may be cached data; First collect and combine this.
  combined = {}
  for bot_name in bot_names:
    master, bot = bot_name.split('/')
    suite_key = ndb.Key('TestMetadata', '%s/%s/%s' % (master, bot, suite_name))

    cache_key = _ListSubTestCacheKey(suite_key)
    cached = layered_cache.Get(cache_key)
    if cached:
      combined = _MergeSubTestsDict(combined, json.loads(cached))
    else:
      sub_test_paths_futures = GetTestDescendantsAsync(
          suite_key, has_rows=True, deprecated=False)
      deprecated_sub_test_path_futures = GetTestDescendantsAsync(
          suite_key, has_rows=True, deprecated=True)

      ndb.Future.wait_all(
          [sub_test_paths_futures, deprecated_sub_test_path_futures])
      sub_test_paths = _MapTestDescendantsToSubTestPaths(
          sub_test_paths_futures.get_result())
      deprecated_sub_test_paths = _MapTestDescendantsToSubTestPaths(
          deprecated_sub_test_path_futures.get_result())

      d1 = _SubTestsDict(sub_test_paths, False)
      d2 = _SubTestsDict(deprecated_sub_test_paths, True)

      sub_tests = _MergeSubTestsDict(d1, d2)

      # Pickle is actually really slow, json.dumps to bypass that.
      layered_cache.Set(cache_key, json.dumps(sub_tests))

      combined = _MergeSubTestsDict(combined, sub_tests)

  return combined


def _SubTestsDict(paths, deprecated):
  """Constructs a sub-test dict from a list of test paths.

  Args:
    paths: An iterable of test paths for which there are points. Each test
        path is of the form "Master/bot/benchmark/chart/...". Each test path
        corresponds to a TestMetadata entity for which has_rows is set to True.
    deprecated: Whether test are deprecated.

  Returns:
    A recursively nested dict of sub-tests, as returned by GetSubTests.
  """
  merged = {}
  for p in paths:
    test_name = p[0]
    if not test_name in merged:
      merged[test_name] = {'has_rows': False, 'sub_tests': []}
    sub_test_path = p[1:]

    # If this is a top level name, then there are rows
    if not sub_test_path:
      merged[test_name]['has_rows'] = True
    else:
      merged[test_name]['sub_tests'].append(sub_test_path)

  if deprecated:
    for k, v in merged.items():
      merged[k]['deprecated'] = True

  for k, v in merged.items():
    merged[k]['sub_tests'] = _SubTestsDict(v['sub_tests'], deprecated)
  return merged


def _MapTestDescendantsToSubTestPaths(keys):
  """Makes a list of partial test paths for descendants of a test suite.

  Args:
    keys: A list of TestMetadata keys

  Returns:
    A list of test paths for all descendant TestMetadata entities that have
    associated Row entities. These test paths omit the Master/bot/suite part.
  """
  return list(map(_SubTestPath, keys))


def _SubTestPath(test_key):
  """Returns the part of a test path starting from after the test suite."""
  full_test_path = utils.TestPath(test_key)
  parts = full_test_path.split('/')
  assert len(parts) > 3
  return parts[3:]


def _ListSubTestCacheKey(test_key):
  """Returns the sub-tests list cache key for a test suite."""
  parts = utils.TestPath(test_key).split('/')
  master, bot, suite = parts[0:3]
  return graph_data.LIST_TESTS_SUBTEST_CACHE_KEY % (master, bot, suite)


def _MergeSubTestsDict(a, b):
  """Merges two sub-tests dicts together."""
  sub_tests = {}
  a_names, b_names = set(a), set(b)
  for name in a_names & b_names:
    sub_tests[name] = _MergeSubTestsDictEntry(a[name], b[name])
  for name in a_names - b_names:
    sub_tests[name] = a[name]
  for name in b_names - a_names:
    sub_tests[name] = b[name]
  return sub_tests


def _MergeSubTestsDictEntry(a, b):
  """Merges two corresponding sub-tests dict entries together."""
  assert a and b
  deprecated = a.get('deprecated', False) and b.get('deprecated', False)
  entry = {
      'has_rows': a['has_rows'] or b['has_rows'],
      'sub_tests': _MergeSubTestsDict(a['sub_tests'], b['sub_tests'])
  }
  if deprecated:
    entry['deprecated'] = True
  return entry


@ndb.synctasklet
def GetTestsMatchingPattern(pattern,
                            only_with_rows=False,
                            list_entities=False,
                            use_cache=True):
  """Gets the TestMetadata entities or keys which match |pattern|.

  For this function, it's assumed that a test path should only have up to seven
  parts. In theory, tests can be arbitrarily nested, but in practice, tests
  are usually structured as master/bot/suite/graph/trace, and only a few have
  seven parts.

  Args:
    pattern: /-separated string of '*' wildcard and TestMetadata string_ids.
    only_with_rows: If True, only return TestMetadata entities which have data
                    points.
    list_entities: If True, return entities. If false, return keys (faster).

  Returns:
    A list of test paths, or test entities if list_entities is True.
  """
  result = yield GetTestsMatchingPatternAsync(
      pattern,
      only_with_rows=only_with_rows,
      list_entities=list_entities,
      use_cache=use_cache)
  raise ndb.Return(result)


@ndb.tasklet
def GetTestsMatchingPatternAsync(pattern,
                                 only_with_rows=False,
                                 list_entities=False,
                                 use_cache=True):
  property_names = [
      'master_name', 'bot_name', 'suite_name', 'test_part1_name',
      'test_part2_name', 'test_part3_name', 'test_part4_name', 'test_part5_name'
  ]
  pattern_parts = pattern.split('/')
  if len(pattern_parts) > 8:
    raise ndb.Return([])

  # Below, we first build a list of (property_name, value) pairs to filter on.
  query_filters = []
  for index, part in enumerate(pattern_parts):
    if '*' not in part:
      query_filters.append((property_names[index], part))
  for index in range(len(pattern_parts), 7):
    # Tests longer than the desired pattern will have non-empty property names,
    # so they can be filtered out by matching against an empty string.
    # Bug: 'test_part5_name' was added recently, and TestMetadata entities which
    # were created before then do not match it. Since it is the last part, and
    # rarely used, it's okay not to test for it. See
    # https://github.com/catapult-project/catapult/issues/2885
    query_filters.append((property_names[index], ''))

  # Query tests based on the above filters. Pattern parts with * won't be
  # filtered here; the set of tests queried is a superset of the matching tests.
  query = graph_data.TestMetadata.query()
  for f in query_filters:
    query = query.filter(
        # pylint: disable=protected-access
        graph_data.TestMetadata._properties[f[0]] == f[1])
  query = query.order(graph_data.TestMetadata.key)
  if only_with_rows:
    query = query.filter(graph_data.TestMetadata.has_rows == True)
  test_keys = yield query.fetch_async(keys_only=True)

  # Filter to include only tests that match the pattern.
  test_keys = [k for k in test_keys if utils.TestMatchesPattern(k, pattern)]

  if list_entities:
    result = yield ndb.get_multi_async(test_keys, use_cache=use_cache)
    raise ndb.Return(result)
  raise ndb.Return([utils.TestPath(k) for k in test_keys])


def GetTestDescendants(test_key,
                       has_rows=None,
                       deprecated=None,
                       keys_only=True):
  """Returns all the tests which are subtests of the test with the given key.

  Args:
    test_key: The key of the TestMetadata entity to get descendants of.
    has_rows: If set, filter the query for this value of has_rows.
    deprecated: If set, filter the query for this value of deprecated.

  Returns:
    A list of keys of all descendants of the given test.
  """
  return GetTestDescendantsAsync(
      test_key, has_rows=has_rows, deprecated=deprecated,
      keys_only=keys_only).get_result()


def GetTestDescendantsAsync(test_key,
                            has_rows=None,
                            deprecated=None,
                            keys_only=True):
  """Returns all the tests which are subtests of the test with the given key.

  Args:
    test_key: The key of the TestMetadata entity to get descendants of.
    has_rows: If set, filter the query for this value of has_rows.
    deprecated: If set, filter the query for this value of deprecated.

  Returns:
    A future for a list of keys of all descendants of the given test.
  """
  test_parts = utils.TestPath(test_key).split('/')
  query_parts = [
      ('master_name', test_parts[0]),
      ('bot_name', test_parts[1]),
      ('suite_name', test_parts[2]),
  ]
  for index, part in enumerate(test_parts[3:]):
    query_parts.append(('test_part%d_name' % (index + 1), part))
  query = graph_data.TestMetadata.query()
  for part in query_parts:
    query = query.filter(ndb.GenericProperty(part[0]) == part[1])
  if has_rows is not None:
    query = query.filter(graph_data.TestMetadata.has_rows == has_rows)
  if deprecated is not None:
    query = query.filter(graph_data.TestMetadata.deprecated == deprecated)
  futures = query.fetch_async(keys_only=keys_only)
  return futures


def GetTestsForTestPathDict(test_path_dict, return_selected):
  """Outputs a list of selected/unselected tests for a given test path dict.

  When figuring out what chart data to query for, we use a test path dict,
  which maps full test paths to either:

    * an exact list of subtests
    * 'all', which specifies that all subtests that have rows should be added,
      or
    * 'core', which specifies that only those subtests that are 'core' as
      determined by the algorithm (which will be implemented in a forthcoming
      CL) should be added.

  This method resolves the dict into a list of full test paths which are to
  be selected or unselected, so that this list can be passed to /graph_json to
  acquire the timeseries data.

  An example dict could be:

  {
    "ChromiumPerf/win-zenbook/sunspider/Total": ["Total", "ref"]
  }

  which specifies that the test itself, as well as its corresponding ref test
  ChromiumPerf/win-zenbook/sunspider/Total/ref, should be selected.

  If the |return_selected| argument is true, we should expect this
  list:

  [
    "ChromiumPerf/win-zenbook/sunspider/Total",
    "ChromiumPerf/win-zenbook/sunspider/Total/ref"
  ]

  but if |return_selected| is false, then we should expect the other subtests
  of 'ChromiumPerf/win-zenbook/sunspider/Total' to be returned.

  Args:
    test_path_dict: A dict having the structure described above.
    return_selected: true if selected tests should be returned, false if
        unselected tests should be returned.

  Outputs:
    A list of selected/unselected test paths.
  """
  # TODO(eakuefner): Caching once people are using this
  if not isinstance(test_path_dict, dict):
    raise BadRequestError('test_path_dict must be a dict')

  if return_selected:
    return _GetSelectedTestPathsForDict(test_path_dict)

  return _GetUnselectedTestPathsForDict(test_path_dict)


def _GetSelectedTestPathsForDict(test_path_dict):
  paths = []
  test_key_futures = []
  any_missing = False
  for path, selection in test_path_dict.items():
    if selection == 'core':
      try:
        paths.extend(_GetCoreTestPathsForTest(path, True))
      except AssertionError:
        any_missing = True
    elif selection == 'all':
      test_key_futures.append(utils.TestKey(path).get_async())
      try:
        paths.extend(
            GetTestsMatchingPattern('%s/*' % path, only_with_rows=True))
      except AssertionError:
        any_missing = True
    elif isinstance(selection, list):
      parent_test_name = path.split('/')[-1]
      for part in selection:
        if part == parent_test_name:
          # When the element in the selected list is the same as the last part
          # of the path, it's meant to mean just the path.
          # TODO(eakuefner): Disambiguate this by making it explicit.
          part_path = path
        else:
          part_path = '%s/%s' % (path, part)
        test_key_futures.append(utils.TestKey(part_path).get_async())
    else:
      raise BadRequestError("selected must be 'all', 'core', or a list of "
                            "subtests")

  # Can't use Future.wait_all(): if any one test_key is internal-only, then
  # wait_all() throws AssertionError.
  for test_key_future in test_key_futures:
    try:
      test_key = test_key_future.get_result()
    except AssertionError:
      # This is an internal-only timeseries. Don't leak that fact by returning
      # 500. Pretend that it doesn't exist.
      any_missing = True
      continue

    if test_key is None or not test_key.has_rows:
      any_missing = True
    else:
      paths.append(test_key.test_path)

  return {'anyMissing': any_missing, 'tests': paths}


def _GetUnselectedTestPathsForDict(test_path_dict):
  paths = []
  for path, selection in test_path_dict.items():
    if selection == 'core':
      paths.extend(_GetCoreTestPathsForTest(path, False))
    elif selection == 'all':
      return {'tests': []}
    elif isinstance(selection, list):
      # The parent is represented in the selection by its last component, so if
      # we don't see it, we know we need to include it in unselected.
      parent_test_name = path.split('/')[-1]
      if not parent_test_name in selection:
        paths.append(path)
      # We want to find all the complementary children, which are
      # those that do not appear in the selection, with rows.
      children = GetTestsMatchingPattern('%s/*' % path, only_with_rows=True)
      for child in children:
        item = child.split('/')[-1]
        if item not in selection:
          paths.append(child)
  return {'tests': paths}


def _GetCoreTestPathsForTest(path, return_selected):
  if len(path.split('/')) < 4:
    raise BadRequestError(
        'path must be a full subtest path in order to select core tests.')

  paths = []
  parent_test = utils.TestKey(path).get()

  # The parent test is always considered core as long as it has rows.
  if return_selected and parent_test and parent_test.has_rows:
    paths.append(path)
  for subtest in GetTestsMatchingPattern(
      '%s/*' % path, only_with_rows=True, list_entities=True):
    # All subtests that are monitored are core.
    if return_selected:
      paths.append(utils.TestPath(subtest.key))
    elif not return_selected:
      paths.append(utils.TestPath(subtest.key))

  return paths
