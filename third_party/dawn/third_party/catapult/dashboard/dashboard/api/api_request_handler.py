# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import functools
import json
import logging
import re
import traceback

from dashboard.api import api_auth
from dashboard.common import utils
from flask import make_response, request

_ALLOWED_ORIGINS = [
    'chromeperf.appspot.com',
    'pinpoint-dot-chromeperf.appspot.com',
    'chromiumdash.appspot.com',
    'chromiumdash-staging.googleplex.com',
]
if utils.IsStagingEnvironment():
  _ALLOWED_ORIGINS = [
      'chromeperf-stage.uc.r.appspot.com',
      'pinpoint-dot-chromeperf-stage.uc.r.appspot.com',
  ]


class BadRequestError(Exception):
  pass


class ForbiddenError(Exception):

  def __init__(self):
    super().__init__('Access denied')


class NotFoundError(Exception):

  def __init__(self):
    super().__init__('Not found')


def SafeOriginRegex(prefix, origin):
  return re.compile(r'^' + prefix + re.escape(origin) + '$')


def RequestHandlerDecoratorFactory(user_checker):

  def RequestHandlerDecorator(request_handler):

    @functools.wraps(request_handler)
    def Wrapper(*args):
      if request.method == 'OPTIONS':
        response = make_response()
        _SetCorsHeadersIfAppropriate(request, response)
        return response

      try:
        user_checker()
      except api_auth.NotLoggedInError as e:
        return _WriteErrorMessage(str(e), 401)
      except api_auth.OAuthError as e:
        return _WriteErrorMessage(str(e), 403)
      except ForbiddenError as e:
        return _WriteErrorMessage(str(e), 403)
      # Allow oauth.Error to manifest as HTTP 500.

      try:
        results = request_handler(*args)
      except NotFoundError as e:
        return _WriteErrorMessage(str(e), 404)
      except (BadRequestError, KeyError, TypeError, ValueError) as e:
        return _WriteErrorMessage(str(e), 400)
      except ForbiddenError as e:
        return _WriteErrorMessage(str(e), 403)

      response = make_response(json.dumps(results))
      _SetCorsHeadersIfAppropriate(request, response)
      return response

    return Wrapper

  return RequestHandlerDecorator


def _SetCorsHeadersIfAppropriate(req, resp):
  resp.headers['Content-Type'] = 'application/json; charset=utf-8'
  set_cors_headers = False
  origin = req.headers.get('Origin', '')
  for allowed in _ALLOWED_ORIGINS:
    dev_pattern = SafeOriginRegex(r'https://[A-Za-z0-9-]+-dot-', allowed)
    prod_pattern = SafeOriginRegex(r'https://', allowed)
    if dev_pattern.match(origin) or prod_pattern.match(origin):
      set_cors_headers = True
  if set_cors_headers:
    resp.headers['Access-Control-Allow-Origin'] = origin
    resp.headers['Access-Control-Allow-Credentials'] = 'true'
    resp.headers['Access-Control-Allow-Methods'] = 'GET,OPTIONS,POST'
    resp.headers[
        'Access-Control-Allow-Headers'] = 'Accept,Authorization,Content-Type'
    resp.headers['Access-Control-Max-Age'] = '3600'


def _WriteErrorMessage(message, status:int):
  # Only log an error message if it's a 5xx error
  if status >= 500:
    logging.error(traceback.format_exc())
  else:
    logging.warning(traceback.format_exc())
  return make_response(json.dumps({'error': message}), status)
