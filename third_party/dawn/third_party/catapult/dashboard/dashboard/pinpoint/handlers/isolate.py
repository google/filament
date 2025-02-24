# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Service for tracking isolates and looking them up by builder and commit.

An isolate is a way to describe the dependencies of a specific build.

More about isolates:
https://github.com/luci/luci-py/blob/master/appengine/isolate/doc/client/Design.md
"""
from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json

from dashboard.api import api_request_handler
from dashboard.api import api_auth
from dashboard.common import utils
from dashboard.pinpoint.models import change as change_module
from dashboard.pinpoint.models import isolate

from flask import make_response, request

import logging


def _CheckUser():
  if request.remote_addr and request.remote_addr in utils.GetIpAllowlist():
    return
  if utils.IsDevAppserver():
    return
  api_auth.Authorize()
  if not utils.IsInternalUser():
    raise api_request_handler.ForbiddenError()


def _ValidateParameters(parameters, validators):
  """Ensure the right parameters are present and valid.
  Args:
    parameters: parameters in dictionary
    validators: Iterable of (name, converter) tuples where name is the
                parameter name and converter is a function used to validate
                and convert that parameter into its internal representation.
  Returns:
    A list of parsed parameter values.

  Raises:
    TypeError: The wrong parameters are present.
    ValueError: The parameters have invalid values.
  """
  given_parameters = set(parameters.keys())
  expected_parameters = set(validators.keys())

  if given_parameters != expected_parameters:
    logging.info('Unexpected request parameters. Given: %s. Expected: %s',
                 given_parameters, expected_parameters)
    if given_parameters - expected_parameters:
      raise TypeError('Unknown parameter: %s' % given_parameters -
                      expected_parameters)
    if expected_parameters - given_parameters:
      raise TypeError('Missing parameter: %s' % expected_parameters -
                      given_parameters)

  parameter_values = {}

  for parameter_name, parameter_value in parameters.items():
    converted_value = validators[parameter_name](parameter_value)
    if not converted_value:
      raise ValueError('Empty parameter: %s' % parameter_name)
    parameter_values[parameter_name] = converted_value

  return parameter_values


@api_request_handler.RequestHandlerDecoratorFactory(_CheckUser)
def IsolateHandler():
  if request.method == 'POST':
    validators = {
        'builder_name': str,
        'change': (lambda x: change_module.Change.FromDict(json.loads(x))),
        'isolate_server': str,
        'isolate_map': json.loads
    }
    validated_parameters = _ValidateParameters(request.form, validators)

    # Put information into the datastore.
    isolate_infos = [
        (validated_parameters['builder_name'], validated_parameters['change'],
         target, validated_parameters['isolate_server'], isolate_hash)
        for target, isolate_hash in validated_parameters['isolate_map'].items()
    ]

    isolate.Put(isolate_infos)

    return isolate_infos

  if request.method == 'GET':
    validators = {
        'builder_name': str,
        'change': (lambda x: change_module.Change.FromDict(json.loads(x))),
        'target': str
    }
    try:
      validated_parameters = _ValidateParameters(request.form, validators)
    except (KeyError, TypeError, ValueError) as e:
      return make_response(json.dumps({'error': str(e)}), 400)

    try:
      isolate_server, isolate_hash = isolate.Get(
          validated_parameters['builder_name'], validated_parameters['change'],
          validated_parameters['target'])
    except KeyError as e:
      return make_response(json.dumps({'error': str(e)}), 404)

    return {'isolate_server': isolate_server, 'isolate_hash': isolate_hash}
  return {}


def IsolateCleanupHandler():
  isolate.DeleteExpiredIsolates()
  return make_response('', 200)
