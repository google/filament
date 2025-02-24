# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import datetime

from flask import Blueprint, request
import logging
import json
import time

from common import cloud_metric, utils
from application.perf_api import datastore_client, auth_helper

blueprint = Blueprint('alert_group', __name__)

ALLOWED_CLIENTS = [
    'ashwinpv@google.com',
    'eduardoyap@google.com',
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

INTERNAL_CLIENTS = [
  'perf-chrome-internal@skia-infra-corp.iam.gserviceaccount.com',
  'perf-widevine-cdm@skia-infra-corp.iam.gserviceaccount.com',
  'perf-widevine-whitebox@skia-infra-corp.iam.gserviceaccount.com',
  'perf-v8-internal@skia-infra-corp.iam.gserviceaccount.com',
  'perf-devtools-frontend@skia-infra-corp.iam.gserviceaccount.com',
  ]

class AnomalyDetail:
  anomaly_id: int
  test_path: str

class AlertGroupDetailsResponse:
  group_id: str
  anomalies: []
  start_commit: int
  end_commit: int

  def ToDict(self):
    return {
      "group_id": self.group_id,
      "start_commit": self.start_commit,
      "end_commit": self.end_commit,
      "anomalies": {
        a.anomaly_id: a.test_path for a in self.anomalies
      }
    }

@blueprint.route('/details', methods=['GET'])
@cloud_metric.APIMetric("skia-bridge", "/alert_group/details")
def AlertGroupDetailsPostHandler():
  try:
    is_authorized, client_email = auth_helper.AuthorizeBearerToken(
      request, ALLOWED_CLIENTS)
    if not is_authorized:
      return 'Unauthorized', 401

    internal:bool = client_email in INTERNAL_CLIENTS
    group_key = request.args.get('key')
    if not group_key:
      return 'Alert group key is required in the request', 400
    logging.info('Received request for alert group details with group id: %s',
                  group_key)

    client = datastore_client.DataStoreClient()
    alert_group = client.GetEntity(datastore_client.EntityType.AlertGroup,
                                    group_key)
    if alert_group:
      anomaly_ids = [a.id_or_name for a in alert_group.get('anomalies')]
      anomalies = client.GetEntities(datastore_client.EntityType.Anomaly,
                                      anomaly_ids)
      logging.info('Retrieved %i anomalies for group id %s', len(anomalies),
                   group_key)
      if not internal:
        public_anomalies = []
        for anomaly in anomalies:
          test_key = anomaly.get('test')
          if test_key:
            parent_test = client.GetEntity(datastore_client.EntityType.TestMetadata,
              test_key.name)
            if parent_test and parent_test.get('internal_only') == False:
              public_anomalies.append(anomaly)

        anomalies = public_anomalies

      response = AlertGroupDetailsResponse()

      if len(anomalies) > 0:
        response.group_id = group_key
        response.anomalies = []
        start_commit = anomalies[0].get('start_revision')
        end_commit = anomalies[0].get('end_revision')
        for anomaly in anomalies :
          # Only return anomaly if it's not a "skia" anomaly.
          # Chromeperf anomalies have source value None
          # while skia ones have 'skia'
          if anomaly.get('source') == None:
            response.anomalies.append(GetAnomalyDetailFromEntity(anomaly))
            if anomaly.get('start_revision') < start_commit:
              start_commit = anomaly.get('start_revision')
            if anomaly.get('end_revision') > end_commit:
              end_commit = anomaly.get('end_revision')

        response.start_commit = start_commit
        response.end_commit = end_commit

        return response.ToDict()
      else:
        logging.info('No anomalies exist for alert group or ' +
                     'user does not have permission to view data.')
        return {}
    else:
      logging.info('No alert group found with key %s', group_key)
      return {}
  except Exception as e:
    logging.exception(e)
    raise e


def GetAnomalyDetailFromEntity(anomaly_entity):
  anomaly_detail = AnomalyDetail()
  anomaly_detail.anomaly_id = anomaly_entity.key.id_or_name
  anomaly_detail.test_path = utils.TestPath(anomaly_entity.get('test'))
  return anomaly_detail
