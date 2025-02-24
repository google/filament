# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import datetime
from dateutil import parser
from flask import Blueprint, request, make_response
import logging
import json
import uuid

from common import cloud_metric, utils
from application.perf_api import datastore_client, auth_helper


blueprint = Blueprint('anomalies', __name__)


ALLOWED_CLIENTS = [
    # Chrome (public) skia instance service account
    'perf-chrome-public@skia-infra-public.iam.gserviceaccount.com',
    # Chrome (internal) skia instance service account
    'perf-chrome-internal@skia-infra-corp.iam.gserviceaccount.com',
    # WebRTC (public) skia instance service account
    'perf-webrtc-public@skia-infra-public.iam.gserviceaccount.com',
    # Widevine CDM (internal) skia instance service account
    'perf-widevine-cdm@skia-infra-corp.iam.gserviceaccount.com',
    # Widevine Whitebox (internal) skia instance service account
    'perf-widevine-whitebox@skia-infra-corp.iam.gserviceaccount.com',
    # V8 (internal) skia instance service account
    'perf-v8-internal@skia-infra-corp.iam.gserviceaccount.com',
    # Devtools-Frontend skia instance service account
    'perf-devtools-frontend@skia-infra-corp.iam.gserviceaccount.com',
]

DATASTORE_TEST_BATCH_SIZE = 25

def Serialize(value):
  if isinstance(value, datetime.datetime):
    return str(value)

  return value.__dict__

class AnomalyUpdateFailedException(Exception):
  """Raised when the update of anomalies fail."""
  pass

class AnomalyData:
  test_path:str
  start_revision:int
  end_revision:int
  id:int
  timestamp:datetime.datetime
  bug_id:int
  is_improvement:bool
  recovered:bool
  state:str
  statistic:str
  units:str
  degrees_of_freedom:float
  median_before_anomaly:float
  median_after_anomaly:float
  p_value:float
  segment_size_after:int
  segment_size_before:int
  std_dev_before_anomaly:float

  def __init__(
      self,
      **kwargs):
    self.__dict__.update(kwargs)

class AnomalyResponse:
  def __init__(self):
    self.anomalies = {}

  def AddAnomaly(self, test_name: str, anomaly_data:AnomalyData):
    if not self.anomalies.get(test_name):
      self.anomalies[test_name] = []

    self.anomalies[test_name].append(anomaly_data.__dict__)

  def ToDict(self):
    return {
      "anomalies": {
          test_name: self.anomalies[test_name] for test_name in self.anomalies
      }
    }

@blueprint.route('/find', methods=['POST'])
@cloud_metric.APIMetric("skia-bridge", "/anomalies/find")
def QueryAnomaliesPostHandler():
  try:
    logging.info('Received query request with data %s', request.data)
    is_authorized, _ = auth_helper.AuthorizeBearerToken(
      request, ALLOWED_CLIENTS)
    if not is_authorized:
      return 'Unauthorized', 401
    try:
      data = json.loads(request.data)
    except json.decoder.JSONDecodeError:
      return 'Malformed Json', 400

    client = datastore_client.DataStoreClient()
    if data.get('revision', None):
      anomalies = client.QueryAnomaliesAroundRevision(int(data['revision']))
    else:
      is_valid, error = ValidateRequest(
        data,
        ['tests', 'min_revision', 'max_revision'])
      if not is_valid:
        return error, 400

      batched_tests = list(CreateTestBatches(data['tests']))
      logging.info('Created %i batches for DataStore query', len(batched_tests))
      anomalies = []
      for batch in batched_tests:
        batch_anomalies = client.QueryAnomalies(
          batch, data['min_revision'], data['max_revision'])
        if batch_anomalies and len(batch_anomalies) > 0:
          anomalies.extend(batch_anomalies)

    logging.info('%i anomalies returned from DataStore', len(anomalies))
    response = AnomalyResponse()
    for found_anomaly in anomalies:
      anomaly_data = GetAnomalyData(found_anomaly)
      response.AddAnomaly(anomaly_data.test_path, anomaly_data)

    return make_response(response.ToDict())
  except Exception as e:
    logging.exception(e)
    raise


@blueprint.route('/get', methods=['GET'], endpoint='GetAnomalyHandler')
@cloud_metric.APIMetric("skia-bridge", "/anomalies/get")
def GetAnomalyHandler():
  try:
    is_authorized, _ = auth_helper.AuthorizeBearerToken(request,
                                                        ALLOWED_CLIENTS)
    if not is_authorized:
      return 'Unauthorized', 401
    anomaly_key = request.args.get('key')
    if not anomaly_key:
      return 'Anomaly key is required in the request', 400

    logging.info('Received request to get anomaly details with key: %s',
                 anomaly_key)
    client = datastore_client.DataStoreClient()
    try:
      anomaly_entity = client.GetEntityFromUrlSafeKey(anomaly_key)
    except Exception as e:
      logging.info('Error reading anomaly with key %s from datastore: %s',
                   anomaly_key, str(e))
      return 'Invalid key', 400

    response = AnomalyResponse()
    if anomaly_entity:
      anomaly_data = GetAnomalyData(anomaly_entity)
      response.AddAnomaly(anomaly_data.test_path, anomaly_data)

    return make_response(response.ToDict())
  except Exception as e:
    logging.exception(e)
    raise


@blueprint.route('/find_time',
                 methods=['POST'],
                 endpoint='QueryAnomaliesByTimePostHandler')
@cloud_metric.APIMetric("skia-bridge", "/anomalies/find_time")
def QueryAnomaliesByTimePostHandler():
  try:
    logging.info('Received query request with data %s', request.data)
    is_authorized, _ = auth_helper.AuthorizeBearerToken(
      request, ALLOWED_CLIENTS)
    if not is_authorized:
      return 'Unauthorized', 401
    try:
      data = json.loads(request.data)
    except json.decoder.JSONDecodeError:
      return 'Malformed Json', 400

    client = datastore_client.DataStoreClient()
    is_valid, error = ValidateRequest(
        data,
        ['tests', 'start_time', 'end_time'])
    if not is_valid:
      return error, 400

    start_time = parser.parse(data['start_time'])
    end_time = parser.parse(data['end_time'])
    if end_time < start_time:
      return 'end_time needs to be after start_time', 400

    batched_tests = list(CreateTestBatches(data['tests']))
    logging.info('Created %i batches for DataStore query', len(batched_tests))
    anomalies = []
    for batch in batched_tests:
      batch_anomalies = client.QueryAnomaliesTimestamp(
        batch, start_time, end_time)
      if batch_anomalies and len(batch_anomalies) > 0:
        anomalies.extend(batch_anomalies)

    logging.info('%i anomalies returned from DataStore', len(anomalies))
    response = AnomalyResponse()
    for found_anomaly in anomalies:
      anomaly_data = GetAnomalyData(found_anomaly)
      response.AddAnomaly(anomaly_data.test_path, anomaly_data)

    return make_response(response.ToDict())
  except Exception as e:
    logging.exception(e)
    raise


@blueprint.route('/add', methods=['POST'], endpoint='AddAnomalyPostHandler')
@cloud_metric.APIMetric("skia-bridge", "/anomalies/add")
def AddAnomalyPostHandler():
  try:
    logging.info('Received query request with data %s', request.data)
    is_authorized, _ = auth_helper.AuthorizeBearerToken(
      request, ALLOWED_CLIENTS)
    if not is_authorized:
      return 'Unauthorized', 401
    try:
      data = json.loads(request.data)
    except json.decoder.JSONDecodeError:
      return 'Malformed Json', 400

    required_keys = ['start_revision', 'end_revision', 'project_id',
                     'test_path', 'is_improvement', 'bot_name',
                     'internal_only']
    # TODO: Make the below keys required once the changes are rolled
    # out in to skia perf
    optional_keys = ['median_before_anomaly', 'median_after_anomaly']
    is_valid, error = ValidateRequest(data, required_keys)
    if not is_valid:
      return error, 400

    client = datastore_client.DataStoreClient()
    test_path = data['test_path']
    test_metadata = client.GetEntity(datastore_client.EntityType.TestMetadata,
                                     test_path)
    if test_metadata:
      # Create the anomaly entity with the required data
      required_keys.remove('test_path')
      anomaly_data = {key : data[key] for key in required_keys}
      anomaly_data.update(GetTestFieldsFromPath(test_path))
      anomaly_data['timestamp'] = datetime.datetime.utcnow()
      anomaly_data['source'] = 'skia'

      for optional_key in optional_keys:
        if data.get(optional_key, None):
          anomaly_data[optional_key] = data[optional_key]

      _ExtendRevisions(anomaly_data)
      # Lets create the anomaly entity and save it in datastore.
      # The save is required for the entity to have a complete key.
      # If we do the save in the transaction below, the key is not available
      # until the transaction is committed (i.e outside the with block) and
      # the key is required to be added to the ungrouped group
      anomaly = client.CreateEntity(datastore_client.EntityType.Anomaly,
                                    str(uuid.uuid4()),
                                    anomaly_data,
                                    save=True)

      anomaly['test'] = test_metadata.key

      # The following operations need to happen in a transaction
      # 1. Read the ungrouped group from datastore.
      # 2. Add the current anomaly key into the ungrouped group.
      # 3. Specify the ungrouped group in the anomaly.
      # 4. Write this update into both the group and the anomaly entities
      #
      # We also need optimistic locking on the alert group since multiple
      # requests can be updating it. We can use the 'updated' field in the
      # alert group to check if another request updated it before we do the
      # put calls.
      # Attempt to retry the failed transaction up to 5 times
      success: bool = True
      alert_group = None
      for i in range(0,5):
        try:
          alert_group = _GetOrCreateUngroupedGroup(client)
          def UpdateAnomalyInDatastore():
            group_anomalies = alert_group.get('anomalies', [])
            group_anomalies.append(anomaly.key)
            alert_group['anomalies'] = group_anomalies

            anomaly['groups'] = [alert_group]
            latest_alert_group = client.GetEntity(
              datastore_client.EntityType.AlertGroup,
              alert_group.key.id_or_name)
            if alert_group.get('updated') == latest_alert_group.get('updated'):
              alert_group['updated'] = datetime.datetime.utcnow()
              client.PutEntities([anomaly, alert_group])
              return True, alert_group
            else:
              raise AnomalyUpdateFailedException

          success, alert_group = client.RunTransaction(UpdateAnomalyInDatastore)
        except AnomalyUpdateFailedException:
          logging.warn(
            '(%i/%i) Transaction failed while committing the anomaly update.',
            i, 5)

        if success:
          break

      if success:
        return {
          'anomaly_id': anomaly.key.id_or_name,
          'alert_group_id': alert_group.key.id_or_name
        }, 200
      else:
        return {
          'anomaly_id': '0',
          'alert_group_id': '0'
        }, 409 # Conflict
    else:
      logging.warn('Test Metadata does not exist for path %s', test_path)
      return {
        'anomaly_id': '0',
        'alert_group_id': '0'
      }, 404 # Not Found
  except Exception as e:
    logging.exception(e)
    raise


def CreateTestBatches(testList):
  for i in range(0, len(testList), DATASTORE_TEST_BATCH_SIZE):
    yield testList[i:i + DATASTORE_TEST_BATCH_SIZE]

def GetAnomalyData(anomaly_obj):
  bug_id = anomaly_obj.get('bug_id')

  if bug_id is None:
    bug_id = '-1'

  return AnomalyData(
      test_path=utils.TestPath(anomaly_obj.get('test')),
      start_revision=anomaly_obj.get('start_revision'),
      end_revision=anomaly_obj.get('end_revision'),
      timestamp=anomaly_obj.get('timestamp'),
      id=anomaly_obj.id,
      bug_id=int(bug_id),
      is_improvement=anomaly_obj.get('is_improvement'),
      recovered=anomaly_obj.get('recovered'),
      state=anomaly_obj.get('state'),
      statistic=anomaly_obj.get('statistic'),
      units=anomaly_obj.get('units'),
      degrees_of_freedom=anomaly_obj.get('degrees_of_freedom'),
      median_before_anomaly=anomaly_obj.get('median_before_anomaly'),
      median_after_anomaly=anomaly_obj.get('median_after_anomaly'),
      p_value=anomaly_obj.get('p_value'),
      segment_size_after=anomaly_obj.get('segment_size_after'),
      segment_size_before=anomaly_obj.get('segment_size_before'),
      std_dev_before_anomaly=anomaly_obj.get('std_dev_before_anomaly'),
  )

def ValidateRequest(request_data, required_keys):
  missing_keys = []
  for key in required_keys:
    value = request_data.get(key, None)
    # Not using "if not value" since value can be boolean False
    if value == None:
      missing_keys.append(key)

  error = None
  result = True
  if len(missing_keys) > 0:
    result = False
    error = 'Required parameters %s missing from the request.' % missing_keys

  return result, error

def GetTestFieldsFromPath(test_path: str):
  # The test path is in the form master/bot/benchmark/test/...
  test_fields = {}
  test_parts = test_path.split('/')
  if len(test_parts) < 4:
    raise ValueError("Test path needs at least 4 parts")

  test_keys = ['master_name', 'bot_name', 'benchmark_name']
  for i in range(len(test_keys)):
    test_fields[test_keys[i]] = test_parts[i]
  return test_fields

def _ExtendRevisions(anomaly_data):
  start_revision = int(anomaly_data['start_revision']) - 5
  end_revision = int(anomaly_data['end_revision']) + 5

  anomaly_data['start_revision'] = start_revision
  anomaly_data['end_revision'] = end_revision

def _GetOrCreateUngroupedGroup(client: datastore_client.DataStoreClient):
  skia_ungrouped_name = 'Ungrouped_Skia'
  ungrouped_type = 2 # 2 is the type for "ungrouped" groups
  alert_groups = client.QueryAlertGroups(skia_ungrouped_name, ungrouped_type)
  if not alert_groups:
    ungrouped_data = {
      'project_id': anomaly_data['project_id'],
      'group_type': ungrouped_type,
      'active': True,
      'anomalies': [anomaly.key],
      'name': skia_ungrouped_name,
      'created': datetime.datetime.utcnow(),
      'updated': datetime.datetime.utcnow()
    }
    alert_group = client.CreateEntity(
      datastore_client.EntityType.AlertGroup,
      str(uuid.uuid4()),
      ungrouped_data,
      save=True)
  else:
    alert_group = alert_groups[0]

  return alert_group
