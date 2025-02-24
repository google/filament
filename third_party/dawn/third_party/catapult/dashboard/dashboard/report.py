# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Provides the web interface for reporting a graph of traces."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json
import six

from google.appengine.ext import ndb

from dashboard import chart_handler
from dashboard import list_tests
from dashboard.common import cloud_metric
from dashboard.common import request_handler
from dashboard import short_uri
from dashboard import update_test_suites
from dashboard.models import page_state

from flask import make_response, redirect, request


def ReportHandlerGet():
  """Renders the static UI for selecting graphs."""
  query_string = _GetQueryStringForOldUri()
  if query_string:
    return redirect('/report?' + query_string)
  return request_handler.RequestHandlerRenderStaticHtml('report.html')


@cloud_metric.APIMetric("chromeperf", "/report")
def ReportHandlerPost():
  """Gets dynamic data for selecting graphs"""
  values = {}
  chart_handler.GetDynamicVariables(values)
  return make_response(
      json.dumps({
          'is_internal_user': values['is_internal_user'],
          'login_url': values['login_url'],
          'revision_info': values['revision_info'],
          'xsrf_token': six.ensure_str(values['xsrf_token']),
          'test_suites': update_test_suites.FetchCachedTestSuites(),
      }))


def _GetQueryStringForOldUri():
  """Gets a new query string if old URI parameters are present.

  SID is a hash string generated from a page state dictionary which is
  created here from old URI request parameters.

  Returns:
    A query string if request parameters are from old URI, otherwise None.
  """
  masters = request.values.get('masters')
  bots = request.values.get('bots')
  tests = request.values.get('tests')
  checked = request.values.get('checked')

  if not (masters and bots and tests):
    return None

  # Page state is a list of chart state.  Chart state is
  # a list of pair of test path and selected series which is used
  # to generate a chart on /report page.
  state = _CreatePageState(masters, bots, tests, checked)

  # Replace default separators to remove whitespace.
  state_json = json.dumps(state, separators=(',', ':'))
  state_id = short_uri.GenerateHash(state_json)

  # Save page state.
  if not ndb.Key(page_state.PageState, state_id).get():
    page_state.PageState(id=state_id, value=six.ensure_binary(state_json)).put()

  query_string = 'sid=' + state_id
  if request.values.get('start_rev'):
    query_string += '&start_rev=' + request.values.get('start_rev')
  if request.values.get('end_rev'):
    query_string += '&end_rev=' + request.values.get('end_rev')
  if request.values.get('rev'):
    query_string += '&rev=' + request.values.get('rev')
  return query_string


def _CreatePageState(masters, bots, tests, checked):
  """Creates a page state dictionary for old URI parameters.

  Based on original /report page, each combination of masters, bots, and
  tests is a chart; therefor we create a list of chart states for those
  combinations.

  Args:
    masters: A string with comma separated list of masters.
    bots: A string with comma separated list of bots.
    tests: A string with comma separated list of tests.
    checked: A string with comma separated list of checked series.

  Returns:
    Page state dictionary.
  """
  selected_series = []
  if checked:
    if checked == 'all':
      selected_series = ['all']
    else:
      selected_series = checked.split(',')

  masters = masters.split(',')
  bots = bots.split(',')
  tests = tests.split(',')
  test_paths = []
  for master in masters:
    for bot in bots:
      for test in tests:
        test_parts = test.split('/')
        if len(test_parts) == 1:
          first_test_parts = _GetFirstTest(test, master + '/' + bot)
          if first_test_parts:
            test += '/' + '/'.join(first_test_parts)
            if not selected_series:
              selected_series.append(first_test_parts[-1])
        test_paths.append(master + '/' + bot + '/' + test)

  chart_states = []
  for path in test_paths:
    chart_states.append([[path, selected_series]])

  return {'charts': chart_states}


def _GetFirstTest(test_suite, bot_path):
  """Gets the first test.

  Args:
    test_suite: Test suite name.
    bot_path: Master and bot name separated by a slash.

  Returns:
    A list of test path parts of the first test that has rows, otherwise
    returns None.
  """
  sub_test_tree = list_tests.GetSubTests(test_suite, [bot_path])
  test_parts = []
  while sub_test_tree:
    first_test = sorted(sub_test_tree.keys())[0]
    test_parts.append(first_test)
    if sub_test_tree[first_test]['has_rows']:
      return test_parts
    sub_test_tree = sub_test_tree[first_test]['sub_tests']
  return None
