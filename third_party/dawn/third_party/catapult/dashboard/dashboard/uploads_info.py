# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""URL endpoint for getting information about histogram upload proccess."""
from __future__ import absolute_import

import logging
import uuid

from google.appengine.ext import ndb

from dashboard.api import api_request_handler
from dashboard.api import api_auth
from dashboard.common import histogram_helpers
from dashboard.common import utils
from dashboard.models import histogram
from dashboard.models import upload_completion_token
from tracing.value import histogram as histogram_module
from tracing.value.diagnostics import diagnostic as diagnostic_module
from tracing.value.diagnostics import generic_set
from tracing.value.diagnostics import diagnostic_ref

from flask import request

def _IsValidUuid(val):
  try:
    uuid.UUID(str(val))
    return True
  except ValueError:
    return False

# pylint: disable=abstract-method
def _CheckUser():
  if request.remote_addr and request.remote_addr in utils.GetIpAllowlist():
    return
  if utils.IsDevAppserver():
    return
  api_auth.Authorize()
  if not utils.IsInternalUser():
    raise api_request_handler.ForbiddenError()

@ndb.tasklet
def _GenerateMeasurementInfo(measurement, get_dimensions_info=False):
  info = {
      'name': measurement.test_path,
      'state': upload_completion_token.StateToString(measurement.state),
      'monitored': measurement.monitored,
      'lastUpdated': str(measurement.update_time),
  }
  if measurement.error_message is not None:
    info['error_message'] = measurement.error_message
  if get_dimensions_info and measurement.histogram is not None:
    histogram_entity = yield measurement.histogram.get_async()
    attached_histogram = histogram_module.Histogram.FromDict(
        histogram_entity.data)
    info['dimensions'] = []
    for name, diagnostic in attached_histogram.diagnostics.items():
      if name not in histogram_helpers.ADD_HISTOGRAM_RELATED_DIAGNOSTICS:
        continue

      if isinstance(diagnostic, diagnostic_ref.DiagnosticRef):
        original_diagnostic = histogram.SparseDiagnostic.get_by_id(
            diagnostic.guid)
        diagnostic = diagnostic_module.Diagnostic.FromDict(
            original_diagnostic.data)

      # We don't have other diagnostics in
      # histogram_helpers.ADD_HISTOGRAM_RELATED_DIAGNOSTICS if they apper
      # in the future, dimensions format should be changed.
      assert isinstance(diagnostic, generic_set.GenericSet)

      info['dimensions'].append({'name': name, 'value': list(diagnostic)})
  raise ndb.Return(info)

def _GenerateResponse(token,
                      get_measurement_info=False,
                      get_dimensions_info=False):
  measurements = token.GetMeasurements() if get_measurement_info else []
  result = {
      'token': token.key.id(),
      'file': token.temporary_staging_file_path,
      'created': str(token.creation_time),
      'lastUpdated': str(token.update_time),
      'state': upload_completion_token.StateToString(token.state),
  }
  if token.error_message is not None:
    result['error_message'] = token.error_message
  if measurements:
    result['measurements'] = []

  measurement_info_futures = []
  for measurement in measurements:
    measurement_info_futures.append(
        _GenerateMeasurementInfo(measurement, get_dimensions_info))
  ndb.Future.wait_all(measurement_info_futures)
  for future in measurement_info_futures:
    result['measurements'].append(future.get_result())
  return result


@api_request_handler.RequestHandlerDecoratorFactory(_CheckUser)
def UploadsInfoGet(token_id):
  """Returns json, that describes state of the token.

  Can be called by get request to /uploads/<token_id>. Measurements info can
  be requested with GET parameter ?additional_info=measurements. If dimensions
  info is also required use ?additional_info=measurements,dimensions.

  Response is json of the form:
  {
    "token": "...",
    "file": "...",
    "created": "...",
    "lastUpdated": "...",
    "state": "PENDING|PROCESSING|FAILED|COMPLETED",
    "error_message": "...",
    "measurements": [
      {
        "name": "...",
        "state": "PROCESSING|FAILED|COMPLETED",
        "monitored": True|False,
        "lastUpdated": "...",
        "error_message": "...",
        "dimensions": [
          {
            "name": "...",
            "values": ["...", ...]
          },
          ...
        ]
      },
      ...
    ]
  }
  Description of the fields:
    - token: Token id from the request.
    - file: Temporary staging file path, where /add_histogram request data is
      stored during the PENDING stage. For more information look at
      /add_histogram api.
    - created: Date and time of creation.
    - lastUpdated: Date and time of last update.
    - state: Aggregated state of the token and all associated measurements.
    - error_message: If historgam upload failed on token level (during
      /add_histogram) will contain addition information about failure.
      Absent otherwise.
    - measurements: List of json objects, that describes measurements
      associated with the token. If there are no such measurements, the field
      will be absent. This field may be absent if full information is not
      requested.
      - name: The path  of the measurement. It is a fully-qualified path in
        the Dashboard.
      - state: State of the measurement.
      - error_message: If state is FAILED, contains addition information
        about failure. Absent otherwise.
      - monitored: A boolean indicating whether the path is monitored by a
        sheriff configuration.
      - lastUpdated: Date and time of last update.
      - dimensions: List of relevant for /add_histogram api diagnostics,
        associated to the histogram, that is represented by the measurement.
        This field will be present in response only after the histogram has
        been added to Datastore. This field may be absent if full information
        is not requested.
        - name: Name of the diagnostic.
        - values: List of values, stored in the GenericSet diagnostic.

  Meaning of some common error codes:
    - 400: Invalid format of the token id.
    - 403: The user is not authorized to check on the status of an upload.
    - 404: Token could not be found. It is either expired or was never
      created.
  """

  if not _IsValidUuid(token_id):
    logging.error('Upload completion token id is not valid. Token id: %s',
                  token_id)
    raise ValueError
  token = upload_completion_token.Token.get_by_id(token_id)
  if token is None:
    logging.error('Upload completion token not found. Token id: %s', token_id)
    raise api_request_handler.NotFoundError

  additional_info = request.values.get('additional_info')
  if additional_info:
    get_measurement_info = 'measurements' in additional_info
    get_dimensions_info = 'dimensions' in additional_info
    return _GenerateResponse(token, get_measurement_info, get_dimensions_info)
  return _GenerateResponse(token)
