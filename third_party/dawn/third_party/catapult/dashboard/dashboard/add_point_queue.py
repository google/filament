# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""URL endpoint to add new graph data to the datastore."""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json
import logging

from google.appengine.api import datastore_errors
from google.appengine.ext import ndb

from dashboard import add_point
from dashboard import find_anomalies
from dashboard import graph_revisions
from dashboard import units_to_direction
from dashboard import sheriff_config_client
from dashboard.common import datastore_hooks
from dashboard.common import utils
from dashboard.models import anomaly
from dashboard.models import graph_data

from flask import request, make_response


def AddPointQueuePost():
  """Adds a set of points from the post data.

  Request parameters:
    data: JSON encoding of a list of dictionaries. Each dictionary represents
        one point to add. For each dict, one Row entity will be added, and
        any required TestMetadata or Master or Bot entities will be created.
  """
  datastore_hooks.SetPrivilegedRequest()

  data = json.loads(request.values.get('data'))
  _PrewarmGets(data)

  all_put_futures = []
  added_rows = []
  parent_tests = []

  for row_dict in data:
    try:
      new_row, parent_test, put_futures = _AddRow(row_dict)

      added_rows.append(new_row)
      parent_tests.append(parent_test)
      all_put_futures.extend(put_futures)

    except add_point.BadRequestError as e:
      logging.error('Could not add %s, it was invalid.', str(e))
    except datastore_errors.BadRequestError as e:
      logging.info('While trying to store %s', row_dict)
      logging.error('Datastore request failed: %s.', str(e))
      # We should return a response with more information. We kept an
      # empty response here to align with the webapp2 implementation.
      # A possible option:
      #   return request_handler.RequestHandlerReportError(
      #     'Datastore request failed: %s.' % str(e), status=400)
      return make_response('')

  ndb.Future.wait_all(all_put_futures)

  client = sheriff_config_client.GetSheriffConfigClient()
  tests_keys = set()
  for t in parent_tests:
    reason = []
    subscriptions, _ = client.Match(t.test_path, check=True)
    if not subscriptions:
      reason.append('subscriptions')
    if not t.has_rows:
      reason.append('has_rows')
    if IsRefBuild(t.key):
      reason.append('RefBuild')
    if reason:
      # Disable this log since it's killing the quota of Cloud Logging API -
      # write requests per minute
      # logging.info('Skip test: %s reason=%s', t.key, ','.join(reason))
      continue
    logging.info('Process test: %s', t.key)
    tests_keys.add(t.key)

  # Updating of the cached graph revisions should happen after put because
  # it requires the new row to have a timestamp, which happens upon put.
  futures = [
      graph_revisions.AddRowsToCacheAsync(added_rows),
      find_anomalies.ProcessTestsAsync(tests_keys)
  ]
  ndb.Future.wait_all(futures)
  return make_response('')


def _PrewarmGets(data):
  """Prepares the cache so that fetching is faster later.

  The add_point request handler does a LOT of gets, and it's possible for
  each to take seconds.

  However, NDB will does automatic in-context caching:
  https://developers.google.com/appengine/docs/python/ndb/cache#incontext
  This means that doing an async get() at the start will cache the result, so
  that we can prewarm the cache for everything we'll need throughout the
  request at the start.

  Args:
    data: The request json.
  """
  # Prewarm lookups of masters, bots, and tests.
  master_keys = {ndb.Key('Master', r['master']) for r in data}
  bot_keys = {ndb.Key('Master', r['master'], 'Bot', r['bot']) for r in data}
  test_keys = set()
  for row in data:
    start = '%s/%s' % (row['master'], row['bot'])
    test_parts = row['test'].split('/')
    for part in test_parts:
      if not part:
        break
      start += '/%s' % part
      test_keys.add(ndb.Key('TestMetadata', start))

  ndb.get_multi_async(list(master_keys) + list(bot_keys) + list(test_keys))


def _AddRow(row_dict):
  """Adds a Row entity to the datastore.

  There are three main things that are needed in order to make a new entity;
  the ID, the parent key, and all of the properties. Making these three
  things, and validating the related input fields, are delegated to
  sub-functions.

  Args:
    row_dict: A dictionary obtained from the JSON that was received.

  Returns:
    A triple: The new row, the parent test, and a list of entity put futures.

  Raises:
    add_point.BadRequestError: The input dict was invalid.
    RuntimeError: The required parent entities couldn't be created.
  """
  parent_test = _GetParentTest(row_dict)
  test_container_key = utils.GetTestContainerKey(parent_test.key)

  columns = add_point.GetAndValidateRowProperties(row_dict)

  row_id = add_point.GetAndValidateRowId(row_dict)

  # Update the last-added revision record for this test.
  master, bot, test = row_dict['master'], row_dict['bot'], row_dict['test']
  test_path = '%s/%s/%s' % (master, bot, test)
  last_added_revision_entity = graph_data.LastAddedRevision(
      id=test_path, revision=row_id)
  entity_put_futures = []
  entity_put_futures.append(last_added_revision_entity.put_async())

  # If the row ID isn't the revision, that means that the data is Chrome OS
  # data, and we want the default revision to be Chrome version.
  if row_id != row_dict.get('revision'):
    columns['a_default_rev'] = 'r_chrome_version'

  # Create the entity and add it asynchronously.
  new_row = graph_data.Row(id=row_id, parent=test_container_key, **columns)
  entity_put_futures.append(new_row.put_async())
  entity_put_futures.append(new_row.UpdateParentAsync())

  return new_row, parent_test, entity_put_futures


def _GetParentTest(row_dict):
  """Gets the parent test for a Row based on an input dictionary.

  Args:
    row_dict: A dictionary from the data parameter.

  Returns:
    A TestMetadata entity.

  Raises:
    RuntimeError: Something went wrong when trying to get the parent test.
  """
  master_name = row_dict.get('master')
  bot_name = row_dict.get('bot')
  test_name = row_dict.get('test').strip('/')
  units = row_dict.get('units')
  higher_is_better = row_dict.get('higher_is_better')
  improvement_direction = _ImprovementDirection(higher_is_better)
  internal_only = graph_data.Bot.GetInternalOnlySync(master_name, bot_name)
  benchmark_description = row_dict.get('benchmark_description')
  unescaped_story_name = row_dict.get('unescaped_story_name')

  parent_test = GetOrCreateAncestors(
      master_name,
      bot_name,
      test_name,
      internal_only=internal_only,
      benchmark_description=benchmark_description,
      units=units,
      improvement_direction=improvement_direction,
      unescaped_story_name=unescaped_story_name)

  return parent_test


def _ImprovementDirection(higher_is_better):
  """Returns an improvement direction (constant from alerts_data) or None."""
  if higher_is_better is None:
    return None
  return anomaly.UP if higher_is_better else anomaly.DOWN


def GetOrCreateAncestors(master_name,
                         bot_name,
                         test_name,
                         internal_only=True,
                         benchmark_description='',
                         units=None,
                         improvement_direction=None,
                         unescaped_story_name=None):
  """Gets or creates all parent Master, Bot, TestMetadata entities for a Row."""

  master_entity = _GetOrCreateMaster(master_name)
  _GetOrCreateBot(bot_name, master_entity.key, internal_only)

  # Add all ancestor tests to the datastore in order.
  ancestor_test_parts = test_name.split('/')

  test_path = '%s/%s' % (master_name, bot_name)
  suite = None
  for index, ancestor_test_name in enumerate(ancestor_test_parts):
    # Certain properties should only be updated if the TestMetadata is for a
    # leaf test.
    is_leaf_test = (index == len(ancestor_test_parts) - 1)
    test_properties = {
        'units': units if is_leaf_test else None,
        'internal_only': internal_only,
    }
    if is_leaf_test and improvement_direction is not None:
      test_properties['improvement_direction'] = improvement_direction
    if is_leaf_test and unescaped_story_name is not None:
      test_properties['unescaped_story_name'] = unescaped_story_name
    ancestor_test = _GetOrCreateTest(ancestor_test_name, test_path,
                                     test_properties)
    if index == 0:
      suite = ancestor_test
    test_path = ancestor_test.test_path
  if benchmark_description and suite.description != benchmark_description:
    suite.description = benchmark_description
  return ancestor_test


def _GetOrCreateMaster(name):
  """Gets or creates a new Master."""
  existing = graph_data.Master.get_by_id(name)
  if existing:
    return existing
  new_entity = graph_data.Master(id=name)
  new_entity.put()
  return new_entity


def _GetOrCreateBot(name, parent_key, internal_only):
  """Gets or creates a new Bot under the given Master."""
  existing = graph_data.Bot.get_by_id(name, parent=parent_key)
  if existing:
    if existing.internal_only != internal_only:
      existing.internal_only = internal_only
      existing.put()
    return existing
  logging.info('Adding bot %s/%s', parent_key.id(), name)
  new_entity = graph_data.Bot(
      id=name, parent=parent_key, internal_only=internal_only)
  new_entity.put()
  return new_entity


def _GetOrCreateTest(name, parent_test_path, properties):
  """Either gets an entity if it already exists, or creates one.

  If the entity already exists but the properties are different than the ones
  specified, then the properties will be updated first. This implies that a
  new point is being added for an existing TestMetadata, so if the TestMetadata
  has been previously marked as deprecated then it can be updated and marked as
  non-deprecated.

  If the entity doesn't yet exist, a new one will be created with the given
  properties.

  Args:
    name: The string ID of the Test to get or create.
    parent_test_path: The test_path of the parent entity.
    properties: A dictionary of properties that should be set.

  Returns:
    An entity (which has already been put).

  Raises:
    datastore_errors.BadRequestError: Something went wrong getting the entity.
  """
  test_path = '%s/%s' % (parent_test_path, name)
  existing = graph_data.TestMetadata.get_by_id(test_path)

  if not existing:
    # Add improvement direction if this is a new test.
    if 'units' in properties and 'improvement_direction' not in properties:
      units = properties['units']
      direction = units_to_direction.GetImprovementDirection(units)
      properties['improvement_direction'] = direction
    elif 'units' not in properties or properties['units'] is None:
      properties['improvement_direction'] = anomaly.UNKNOWN
    new_entity = graph_data.TestMetadata(id=test_path, **properties)
    new_entity.UpdateSheriff()
    new_entity.put()
    # TODO(sullivan): Consider putting back Test entity in a scoped down
    # form so we can check if it exists here.
    return new_entity

  # Flag indicating whether we want to re-put the entity before returning.
  properties_changed = False

  if existing.deprecated:
    existing.deprecated = False
    properties_changed = True

  # Special case to update improvement direction from units for TestMetadata
  # entities when units are being updated. If an improvement direction is
  # explicitly provided in the properties, then we can skip this check since it
  # will get overwritten below. Additionally, by skipping we avoid
  # touching the entity and setting off an expensive put() operation.
  if properties.get('improvement_direction') is None:
    units = properties.get('units')
    if units:
      direction = units_to_direction.GetImprovementDirection(units)
      if direction != existing.improvement_direction:
        properties['improvement_direction'] = direction

  # Go through the list of general properties and update if necessary.
  for prop, value in list(properties.items()):
    if (hasattr(existing, prop) and value is not None
        and getattr(existing, prop) != value):
      setattr(existing, prop, value)
      properties_changed = True

  if properties_changed:
    existing.UpdateSheriff()
    existing.put()
  return existing


def IsRefBuild(test_key):
  """Checks whether a TestMetadata is for a reference build test run."""
  test_parts = test_key.id().split('/')
  return test_parts[-1] == 'ref' or test_parts[-1].endswith('_ref')
