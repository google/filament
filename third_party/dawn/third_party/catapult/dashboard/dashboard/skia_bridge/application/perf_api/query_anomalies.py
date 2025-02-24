# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import datetime

from flask import Blueprint, request, jsonify
import logging
import json

from dashboard.models import anomaly
from dashboard.common import datastore_hooks
from dashboard.common import utils

blueprint = Blueprint('query_anomalies', __name__)


def Serialize(value):
  if isinstance(value, datetime.datetime):
    return str(value)

  return value.__dict__


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
  t_statistic:float

  def __init__(
      self,
      **kwargs):
    self.__dict__.update(kwargs)

  def ToJson(self):
    return json.dumps(self, default=Serialize)


@blueprint.route('/find', methods=['POST'])
def QueryAnomaliesPostHandler():
  try:
    logging.info('Received query request with data %s', request.data)
    datastore_hooks.SetPrivilegedRequest()
    data = json.loads(request.data)
    test_keys = [utils.TestKey(test_path) for test_path in data['tests']]
    anomalies, _, _ = anomaly.Anomaly.QueryAsync(
        test_keys=test_keys,
        max_start_revision=data['max_revision'],
        min_end_revision=data['min_revision']).get_result()

    logging.info('%i anomalies found for the request.', len(anomalies))
    response = {}
    for found_anomaly in anomalies:
      anomaly_data = GetAnomalyData(found_anomaly)
      if not response.get(anomaly_data.test_path):
        response[anomaly_data.test_path] = []

      response[anomaly_data.test_path].append(anomaly_data.ToJson())

    return jsonify(response)
  except Exception as e:
    logging.exception(e)
    raise


def GetAnomalyData(anomaly_obj: anomaly.Anomaly):
  return AnomalyData(
      test_path=utils.TestPath(anomaly_obj.test),
      start_revision=anomaly_obj.start_revision,
      end_revision=anomaly_obj.end_revision,
      timestamp=anomaly_obj.timestamp,
      id=anomaly_obj.key.id(),
      bug_id=anomaly_obj.bug_id,
      is_improvement=anomaly_obj.is_improvement,
      recovered=anomaly_obj.recovered,
      state=anomaly_obj.state,
      statistic=anomaly_obj.statistic,
      units=anomaly_obj.units,
      degrees_of_freedom=anomaly_obj.degrees_of_freedom,
      median_before_anomaly=anomaly_obj.median_before_anomaly,
      median_after_anomaly=anomaly_obj.median_after_anomaly,
      p_value=anomaly_obj.p_value,
      segment_size_after=anomaly_obj.segment_size_after,
      segment_size_before=anomaly_obj.segment_size_before,
      std_dev_before_anomaly=anomaly_obj.std_dev_before_anomaly,
      t_statistic=anomaly_obj.t_statistic,
  )
