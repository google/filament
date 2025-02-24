# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import

import datetime
import logging
import os
import time
import uuid
from google.cloud import monitoring_v3

METRIC_TYPE_PREFIX = "custom.googleapis.com/"
RESOURCE_TYPE = "generic_task"
LOCATION = "us-central1"
NAMESPACE = "Prod"
DEFAULT_TASK_ID = "task_id"
API_METRIC_TYPE = "api/metrics"
API_NAME = "api_name"
REQUEST_STATUS = "request_status"
UUID = "uuid"

client_create_time:datetime = None
metrics_client = None


def GetClient():
  curr_time = datetime.datetime.utcnow()
  global client_create_time, metrics_client

  if client_create_time is None:
    client_create_time = curr_time

  diff = curr_time - client_create_time

  # Update the metrics service client
  if diff.total_seconds() > 60 or metrics_client is None:
    logging.info('Creating a new client')
    metrics_client = monitoring_v3.MetricServiceClient()

  return metrics_client


def _PublishTSCloudMetric(
    service_name,
    metric_type,
    label_dict,
    metric_value=1):
  client = GetClient()
  project_id = os.environ.get('GOOGLE_CLOUD_PROJECT')
  project_name = f"projects/{project_id}"

  series = monitoring_v3.TimeSeries()

  series.metric.type = METRIC_TYPE_PREFIX + metric_type

  series.resource.type = RESOURCE_TYPE

  # The identifier of the GCP project associated with this resource,
  # such as "my-project".
  series.resource.labels["project_id"] = project_id

  # The GCP region in which data about the resource is stored
  series.resource.labels["location"] = LOCATION

  # A namespace identifier, such as a cluster name: Dev, Staging or Prod
  series.resource.labels["namespace"] = NAMESPACE

  # An identifier for a grouping of related tasks, such as the name of
  # a microservice or distributed batch job
  series.resource.labels["job"] = service_name

  # A unique identifier for the task within the namespace and job,
  # set default value for this manditory field
  series.resource.labels["task_id"] = DEFAULT_TASK_ID

  # debug infor for crbug/1422306
  for key in label_dict:
    try:
      series.metric.labels[key] = label_dict[key]
    except TypeError as e:
      series.metric.labels[key] = str(label_dict[key])
      logging.warning('Invalid value found in label_dict: %s. (%s)', label_dict,
                      str(e))

  now = time.time()
  seconds = int(now)
  nanos = int((now - seconds) * 10 ** 9)
  interval = monitoring_v3.TimeInterval(
      {"end_time": {
          "seconds": seconds,
          "nanos": nanos
      }})
  try:
    point = monitoring_v3.Point({
        "interval": interval,
        "value": {
            "double_value": metric_value
        }
    })
  except TypeError as e:
    point = monitoring_v3.Point({
        "interval": interval,
        "value": {
            "double_value": int(metric_value)
        }
    })
    logging.warning('Invalid value found in metric_value: %s. (%s)',
                    metric_value, str(e))

  series.points = [point]

  try:
    client.create_time_series(name=project_name, time_series=[series])
  except Exception as e:  # pylint: disable=broad-except
    # Swallow the error from Cloud Monitoring API, the failure from
    # Cloud Monitoring API should not break our code logic.
    logging.warning('Publish data to Cloud Monitoring failed. Error: %s', e)


class APIMetricLogger:

  def __init__(self, service_name, api_name):
    """ This metric logger can be used by the with statement:
    https://peps.python.org/pep-0343/
    """
    self._service_name = service_name
    self._api_name = api_name
    self._start = None
    self.seconds = 0
    self.is_disabled = os.environ.get('DISABLE_METRICS')

  def _Now(self):
    return time.time()

  def __enter__(self):
    if self.is_disabled:
      return
    self._start = self._Now()

  def __exit__(self, exception_type, exception_value, execution_traceback):
    if self.is_disabled:
      return True
    if exception_type is None:
      # with statement BLOCK runs succeed
      self.seconds = self._Now() - self._start
      logging.info('%s:%s=%f', self._service_name, self._api_name, self.seconds)
      label_dict = {API_NAME: self._api_name, REQUEST_STATUS: "completed",
                    UUID: str(uuid.uuid4())}
      _PublishTSCloudMetric(self._service_name, API_METRIC_TYPE, label_dict,
                            self.seconds)
      return True

    # with statement BLOCK throws exception
    label_dict = {API_NAME: self._api_name, REQUEST_STATUS: "failed",
                  UUID: str(uuid.uuid4())}
    _PublishTSCloudMetric(self._service_name, API_METRIC_TYPE, label_dict)
    # throw out the original exception
    return False


def APIMetric(service_name, api_name):
  def Decorator(wrapped):
    def Wrapper(*a, **kw):
      with APIMetricLogger(service_name, api_name):
        return wrapped(*a, **kw)

    return Wrapper

  return Decorator
