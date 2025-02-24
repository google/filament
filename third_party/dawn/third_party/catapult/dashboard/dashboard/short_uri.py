# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Provides an endpoint for handling storing and retrieving page states."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import hashlib
import json
import six

from google.appengine.ext import ndb

from dashboard import list_tests
from dashboard.common import descriptor
from dashboard.common import request_handler
from dashboard.common import datastore_hooks
from dashboard.models import page_state

from flask import make_response, request


def ShortUriHandlerGet():
  """Handles getting page states."""
  state_id = request.args.get('sid')

  if not state_id:
    return request_handler.RequestHandlerReportError(
        'Missing required parameters.', status=400)

  state = ndb.Key(page_state.PageState, state_id).get()

  if not state:
    return request_handler.RequestHandlerReportError('Invalid sid.', status=400)

  if request.args.get('v2', None) is None:
    return make_response(state.value)

  if state.value_v2 is None:
    state.value_v2 = _Upgrade(state.value)
    # If the user is not signed in, then they won't be able to see
    # internal_only TestMetadata, so value_v2 will be incomplete.
    # If the user is signed in, then value_v2 is complete, so it's safe to
    # store it.
    if datastore_hooks.IsUnalteredQueryPermitted():
      state.put()
  return make_response(state.value_v2)


def ShortUriHandlerPost():
  """Handles saving page states and getting state id."""

  state = request.values.get('page_state')

  if not state:
    return request_handler.RequestHandlerReportError(
        'Missing required parameters.', status=400)

  state_id = GetOrCreatePageState(state)

  return make_response(json.dumps({'sid': state_id}))


def GetOrCreatePageState(state):
  state = state.encode('utf-8')
  state_id = GenerateHash(state)
  if not ndb.Key(page_state.PageState, state_id).get():
    page_state.PageState(id=state_id, value=state).put()
  return state_id


def GenerateHash(state_string):
  """Generates a hash for a state string."""
  return hashlib.sha256(six.ensure_binary(state_string)).hexdigest()


def _UpgradeChart(chart):
  groups = []
  if isinstance(chart, list):
    groups = chart
  elif isinstance(chart, dict):
    groups = chart['seriesGroups']

  suites = set()
  measurements = set()
  bots = set()
  cases = set()

  for prefix, suffixes in groups:
    if suffixes == ['all']:
      paths = list_tests.GetTestsMatchingPattern(
          prefix + '/*', only_with_rows=True)
    else:
      paths = []
      for suffix in suffixes:
        if suffix == prefix.split('/')[-1]:
          paths.append(prefix)
        else:
          paths.append(prefix + '/' + suffix)

    for path in paths:
      desc = descriptor.Descriptor.FromTestPathSync(path)
      suites.add(desc.test_suite)
      bots.add(desc.bot)
      measurements.add(desc.measurement)
      if desc.test_case:
        cases.add(desc.test_case)

  return {
      'parameters': {
          'testSuites': list(suites),
          'measurements': list(measurements),
          'bots': list(bots),
          'testCases': list(cases),
      },
  }


def _Upgrade(statejson):
  try:
    state = json.loads(statejson)
  except ValueError:
    return statejson
  if 'charts' not in state:
    return statejson
  state = {
      'showingReportSection': False,
      'chartSections': [_UpgradeChart(chart) for chart in state['charts']],
  }
  statejson = json.dumps(state)
  return six.ensure_binary(statejson)
