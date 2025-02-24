# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Provides a web interface for dumping graph data as JSON.

This is meant to be used with /load_from_prod in order to easily grab
data for a graph to a local server for testing.
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import base64
from flask import request, make_response
import json

from google.appengine.ext import ndb
from google.appengine.ext.ndb import model

from dashboard.common import request_handler
from dashboard.common import utils
from dashboard.models import anomaly
from dashboard.models import graph_data

_DEFAULT_MAX_POINTS = 500
# This is about the limit we want to return since we fetch many associated
# entities for each anomaly.
_DEFAULT_MAX_ANOMALIES = 30


def DumpGraphJsonHandlerGet():
  if request.values.get('sheriff'):
    sheriff_name = request.values.get('sheriff')
    num_points = int(request.values.get('num_points', _DEFAULT_MAX_POINTS))
    num_anomalies = int(
        request.values.get('num_alerts', _DEFAULT_MAX_ANOMALIES))

    protobuf_json = _DumpAnomalyDataForSheriff(sheriff_name, num_points,
                                               num_anomalies)

    return make_response(protobuf_json)
  if request.values.get('test_path'):
    test_path = request.values.get('test_path')
    num_points = int(request.values.get('num_points', _DEFAULT_MAX_POINTS))
    end_rev = request.values.get('end_rev')

    protobuf_json = _DumpTestData(test_path, num_points, end_rev)

    return make_response(protobuf_json)
  return request_handler.RequestHandlerReportError('No parameters specified.')


def _DumpTestData(test_path, num_points, end_rev):
  """Dumps data for the requested test.

  Request parameters:
    test_path: A single full test path, including master/bot.
    num_points: Max number of Row entities (optional).
    end_rev: Ending revision number, inclusive (optional).

  Outputs:
    JSON array of encoded protobuf messages, which encode all of
    the datastore entities relating to one test (including Master, Bot,
    TestMetadata, Row, Anomaly and Sheriff entities).
  """
  test_key = utils.TestKey(test_path)
  if not test_key or test_key.kind() != 'TestMetadata':
    # Bad test_path passed in.
    return json.dumps([])

  # List of datastore entities that will be dumped.
  entities = []

  entities.extend(_GetTestAncestors([test_key]))

  # Get the Row entities.
  q = graph_data.Row.query()
  q = q.filter(graph_data.Row.parent_test == utils.OldStyleTestKey(test_key))
  if end_rev:
    q = q.filter(graph_data.Row.revision <= int(end_rev))
  q = q.order(-graph_data.Row.revision)  # pylint: disable=invalid-unary-operand-type
  entities += q.fetch(limit=num_points)

  # Get the Anomaly and Sheriff entities.
  alerts, _, _ = anomaly.Anomaly.QueryAsync(test=test_key).get_result()
  subscriptions = [s for a in alerts for s in a.subscriptions]
  entities += alerts
  entities += subscriptions

  # Convert the entities to protobuf message strings and output as JSON.
  protobuf_strings = list(map(EntityToBinaryProtobuf, entities))
  return json.dumps(protobuf_strings)


def _DumpAnomalyDataForSheriff(sheriff_name, num_points, num_anomalies):
  """Dumps Anomaly data for all sheriffs.

  Request parameters:
    sheriff: Sheriff name.
    num_points: Max number of Row entities (optional).
    num_alerts: Max number of Anomaly entities (optional).

  Outputs:
    JSON array of encoded protobuf messages, which encode all of
    the datastore entities relating to one test (including Master, Bot,
    TestMetadata, Row, Anomaly and Sheriff entities).
  """
  anomalies, _, _ = anomaly.Anomaly.QueryAsync(
      subscriptions=[sheriff_name], limit=num_anomalies).get_result()
  test_keys = [a.GetTestMetadataKey() for a in anomalies]

  # List of datastore entities that will be dumped.
  entities = []

  entities.extend(_GetTestAncestors(test_keys))

  # Get the Row entities.
  entities.extend(_FetchRowsAsync(test_keys, num_points))

  # Add the Anomaly and Sheriff entities.
  entities += anomalies
  subscriptions = [s for a in anomalies for s in a.subscriptions]
  entities += subscriptions

  # Convert the entities to protobuf message strings and output as JSON.
  protobuf_strings = list(map(EntityToBinaryProtobuf, entities))
  return json.dumps(protobuf_strings)


def _GetTestAncestors(test_keys):
  """Gets the TestMetadata, Bot, and Master entities preceding in path."""
  entities = []
  added_parents = set()
  for test_key in test_keys:
    if test_key.kind() != 'TestMetadata':
      continue
    parts = utils.TestPath(test_key).split('/')
    for index, _, in enumerate(parts):
      test_path = '/'.join(parts[:index + 1])
      if test_path in added_parents:
        continue
      added_parents.add(test_path)
      if index == 0:
        entities.append(ndb.Key('Master', parts[0]).get())
      elif index == 1:
        entities.append(ndb.Key('Master', parts[0], 'Bot', parts[1]).get())
      else:
        entities.append(ndb.Key('TestMetadata', test_path).get())
  return [e for e in entities if e is not None]


def _FetchRowsAsync(test_keys, num_points):
  """Fetches recent Row asynchronously across all 'test_keys'."""
  rows = []
  futures = []
  for test_key in test_keys:
    q = graph_data.Row.query()
    q = q.filter(graph_data.Row.parent_test == utils.OldStyleTestKey(test_key))
    q = q.order(-graph_data.Row.revision)  # pylint: disable=invalid-unary-operand-type
    futures.append(q.fetch_async(limit=num_points))
  ndb.Future.wait_all(futures)
  for future in futures:
    rows.extend(future.get_result())
  return rows


def EntityToBinaryProtobuf(entity):
  """Converts an ndb entity to a protobuf message in binary format."""
  # Encode in binary representation of the protocol buffer.
  message = ndb.ModelAdapter().entity_to_pb(entity).Encode()
  # Base64 encode the data to text format for json.dumps.
  return base64.b64encode(message)


def BinaryProtobufToEntity(pb_str):
  """Converts a protobuf message in binary format to an ndb entity.

  Args:
    pb_str: Binary encoded protocol buffer which is encoded as text.

  Returns:
    A ndb Entity.
  """
  message = model.entity_pb.EntityProto(base64.b64decode(pb_str))
  return ndb.ModelAdapter().pb_to_entity(message)
