# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import logging
import os
import six

from google.appengine.ext import deferred
from google.appengine.ext import ndb

from dashboard import list_tests
from dashboard import update_test_suites
from dashboard.common import datastore_hooks
from dashboard.common import namespaced_stored_object
from dashboard.common import stored_object
from dashboard.common import utils
from dashboard.models import graph_data
from dashboard.models import histogram
from tracing.value.diagnostics import reserved_infos
from tracing.value.diagnostics import generic_set

from flask import request, make_response


def CacheKey(master, test_suite):
  return 'test_suite_descriptor_%s_%s' % (master, test_suite)


def FetchCachedTestSuiteDescriptor(master, test_suite):
  masters = [master]
  if not master:
    masters = _GetMastersForSuite(test_suite)

  futures = [
      namespaced_stored_object.GetAsync(CacheKey(m, test_suite))
      for m in masters
  ]

  ndb.Future.wait_all(futures)

  desc = {'measurements': [], 'bots': [], 'cases': [], 'caseTags': {}}

  for f in futures:
    cur_desc = f.get_result()
    if cur_desc is not None:
      desc['measurements'].extend(cur_desc['measurements'])
      desc['bots'].extend(cur_desc['bots'])
      desc['cases'].extend(cur_desc['cases'])
      for tag, case_tags in six.iteritems(cur_desc['caseTags']):
        desc['caseTags'].setdefault(tag, []).extend(case_tags)

  desc['measurements'] = list(sorted(set(desc['measurements'])))
  desc['bots'] = list(sorted(set(desc['bots'])))
  desc['cases'] = list(sorted(set(desc['cases'])))
  for tag in desc['caseTags']:
    desc['caseTags'][tag] = list(sorted(set(desc['caseTags'][tag])))

  return desc


def UpdateTestSuiteDescriptorsPost():
  namespace = datastore_hooks.EXTERNAL
  if request.values.get('internal_only') == 'true':
    namespace = datastore_hooks.INTERNAL
  UpdateTestSuiteDescriptors(namespace)
  return make_response('')


def UpdateTestSuiteDescriptors(namespace):
  key = namespaced_stored_object.NamespaceKey(
      update_test_suites.TEST_SUITES_2_CACHE_KEY, namespace)
  for test_suite in stored_object.Get(key):
    ScheduleUpdateDescriptor(test_suite, namespace)


def ScheduleUpdateDescriptor(test_suite, namespace):
  deferred.defer(_UpdateDescriptorByMaster, test_suite, namespace)


@ndb.tasklet
def _QueryCaseTags(test_path, case):
  data_by_name = yield histogram.SparseDiagnostic.GetMostRecentDataByNamesAsync(
      utils.TestKey(test_path), [reserved_infos.STORY_TAGS.name])
  data = data_by_name.get(reserved_infos.STORY_TAGS.name)
  tags = list(generic_set.GenericSet.FromDict(data)) if data else []
  raise ndb.Return((case, tags))


def _CollectCaseTags(futures, case_tags):
  ndb.Future.wait_all(futures)
  for future in futures:
    case, tags = future.get_result()
    for tag in tags:
      case_tags.setdefault(tag, []).append(case)


TESTS_TO_FETCH = 5000


def _GetMastersForSuite(suite):
  masters = list_tests.GetTestsMatchingPattern('*/*/%s' % suite)
  masters = list({m.split('/')[0] for m in masters})
  return masters


def _UpdateDescriptorByMaster(test_suite, namespace):
  masters = _GetMastersForSuite(test_suite)
  for m in masters:
    deferred.defer(_UpdateDescriptor, m, test_suite, namespace)


def _UpdateDescriptor(master,
                      test_suite,
                      namespace,
                      start_cursor=None,
                      measurements=(),
                      bots=(),
                      cases=(),
                      case_tags=None):
  logging.info('%s %s %s %d %d %d', master, test_suite, namespace,
               len(measurements), len(bots), len(cases))
  # This function always runs in the taskqueue as an anonymous user.
  if namespace == datastore_hooks.INTERNAL:
    try:
      datastore_hooks.SetPrivilegedRequest()
    except RuntimeError:
      # _UpdateDescriptor is called from deferred queue, and the value of
      # PATH_INFO, '/_ah/queue/deferred', will qualify the request as
      # privileged. We should be safe to skip the SetPrivilegedRequest here.
      path_info = os.environ.get('PATH_INFO', None)
      if path_info:
        logging.info(
            'No flask context found. Privileged request check will rely on PATH_INFO: %s',
            path_info)
      else:
        logging.error(
            'Failed to set privileged request for internal descriptor update.')
        return

  measurements = set(measurements)
  bots = set(bots)
  cases = set(cases)
  case_tags = case_tags or {}
  tags_futures = []

  query = graph_data.TestMetadata.query()
  query = query.filter(graph_data.TestMetadata.master_name == master)
  query = query.filter(graph_data.TestMetadata.suite_name == test_suite)
  query = query.filter(graph_data.TestMetadata.deprecated == False)
  query = query.filter(graph_data.TestMetadata.has_rows == True)

  tests, next_cursor, more = query.fetch_page(
      TESTS_TO_FETCH,
      start_cursor=start_cursor,
      use_cache=False,
      use_memcache=False)

  for test in tests:
    bots.add(test.bot_name)

    try:
      _, measurement, story = utils.ParseTelemetryMetricParts(test.test_path)
    except utils.ParseTelemetryMetricFailed as e:
      # Log the error and process the rest of the test suite.
      logging.error('Parsing error encounted: %s', e)
      continue

    if test.unescaped_story_name:
      story = test.unescaped_story_name

    if measurement:
      measurements.add(measurement)

    if story and story not in cases:
      cases.add(story)
      tags_futures.append(_QueryCaseTags(test.test_path, story))

  _CollectCaseTags(tags_futures, case_tags)

  logging.info('%d keys, %d measurements, %d bots, %d cases, %d tags',
               len(tests), len(measurements), len(bots), len(cases),
               len(case_tags))

  if more:
    logging.info('continuing')
    deferred.defer(_UpdateDescriptor, master, test_suite, namespace,
                   next_cursor, measurements, bots, cases, case_tags)
    return

  desc = {
      'measurements': list(sorted(measurements)),
      'bots': list(sorted(bots)),
      'cases': list(sorted(cases)),
      'caseTags': {
          tag: sorted(cases) for tag, cases in list(case_tags.items())
      }
  }

  key = namespaced_stored_object.NamespaceKey(
      CacheKey(master, test_suite), namespace)
  stored_object.Set(key, desc)
