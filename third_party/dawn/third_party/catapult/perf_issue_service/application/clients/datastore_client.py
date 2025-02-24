# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import datetime
import logging
import uuid

from google.cloud import datastore

from application import utils


class AlertGroupStatus:
  unknown = 0
  untriaged = 1
  triaged = 2
  bisected = 3
  closed = 4


class AlertGroupType:
  test_suite = 0
  ungrouped = 2
  test_suite_skia = 3


class DataStoreClient():
  gae_project_name = 'chromeperf'
  if utils.IsStagingEnvironment():
    gae_project_name = 'chromeperf-stage'
  _client = datastore.Client(project=gae_project_name)


  def _Key(self, kind, id):
    return self._client.key(kind, id)


  def Transaction(self):
    return self._client.transaction()


  def QueryAlertGroup(self, active=True, extra_filters=[], limit=None):
    ''' Query the AlertGroup kind.

    Args:
      active: whether the entity is active
      extra_filters: filters given in a list of triples: (field, op, value)
      limit: the limit of returned entities.

    Return:
      a list of alert group entities.
    '''
    filters = [('active', '=', active)]
    filters += extra_filters

    query = self._client.query(kind='AlertGroup', filters=filters)

    return query.fetch(limit=limit)


  def AlertGroupKey(self, group_id):
    ''' Generate a datastore Key for an alert group

    Args:
      group_id: the id of the group

    Returns:
      the datastore.client.key() object.
    '''
    return self._Key('AlertGroup', group_id)


  def NewAlertGroup(self, benchmark_name, group_type, master_name=None, subscription_name=None,
                    project_id=None, start_rev=None, end_rev=None):
    '''Create a new alert group object. (not saved in datastore)

    Args:
      benchmark_name: the name of the benchmark
      group_type: 0 for test_suites, 2 for reserved (ungrouped)
          (The legacy value 1, logical, is no longer in use)
      master_name: the first part of the test key, will be saved as 'domain'.
      subscription_name: the subscription name from sheriff config.
      project_id: the monorail project name
      start_rev: the start revision of the alert group.
      end_ref: the end revision of the alert group.

    Returns:
      A new alert group entity.
    '''
    if group_type == AlertGroupType.ungrouped:
      new_group = datastore.Entity(self._client.key('AlertGroup'))
    elif group_type == AlertGroupType.test_suite or \
         group_type == AlertGroupType.test_suite_skia:
      new_id = str(uuid.uuid4())
      new_group = datastore.Entity(self._client.key('AlertGroup', new_id))
    else:
      logging.warning('[PerfIssueService] Unsupported group type: %s', group_type)
      return None

    new_group.update(
        {
           'name': benchmark_name,
           'status' : AlertGroupStatus.untriaged,
           'group_type': group_type,
           'active': True
        }
    )

    if master_name:
      new_group['domain'] = master_name
    if subscription_name:
      new_group['subscription_name'] = subscription_name
    if project_id:
      new_group['project_id'] = project_id
    if start_rev and end_rev:
      new_revision = datastore.Entity()
      new_revision.update(
        {
          'repository': 'chromium',
          'start': start_rev,
          'end': end_rev
        }
      )
      new_group['revision'] = new_revision

    return new_group


  def SaveAlertGroup(self, group):
    ''' Save the alert group in datastore.

    The previous ndb implementation has 'created' and 'updated' properties
    by setting the auto_now_add flag. This flag will have the property updated
    on create.

    Args:
      group: the alert group entity.

    Returns:
      None
    '''
    time_stamp = datetime.datetime.now(tz=datetime.timezone.utc)
    if not group.get('created'):
      group['created'] = time_stamp
    group['updated'] = time_stamp

    return self._client.put(group)


  def SaveAnomaly(self, anomaly):
    ''' Save the anomaly entity in datastore

    Args:
      anomaly: the anomaly entity.

    Returns:
      None
    '''
    return self._client.put(anomaly)


  def GetEntityByKey(self, key):
    '''Load the datastore entity by the key

    Args:
      key: the entity key.

    Returns:
      the datastore entity.
    '''
    return self._client.get(key)


  def GetMultiEntitiesByKeys(self, keys):
    ''' Load multiple entities using a list of keys

    Args:
      keys: the list of the entity keys.

    Returns:
      a list of entities
    '''
    return self._client.get_multi(keys)


  def GetEntityId(self, entity, key_type=None):
    '''Load the id the entity

    The id of the entity can be an auto-assigned integer, or a custom string
    value. We have been using both and thus need an extra way to load only
    the string values in some cases.

    Args:
      entity: the entity object
      key_type: None by default. Can be 'id' or 'name'.

    Returns:
      The integer value if key_type is 'id', the string value if key_type is
      'name'. When key_type is None, return the integer value if exist, or the
      string value.
    '''
    if key_type == 'id':
        return entity.key.id
    elif key_type == 'name':
        return entity.key.name
    return entity.key.id_or_name


