# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Common utils for issue tracker client."""

import functools
import json
import logging
import os

from flask import request as flask_request, make_response
import httplib2

import google.auth
import google_auth_httplib2

TOKEN_INFO_ENDPOINT = 'https://oauth2.googleapis.com/tokeninfo'
EMAIL_SCOPE = 'https://www.googleapis.com/auth/userinfo.email'
PROD_SERVICE_ACCOUNT = 'chromeperf@appspot.gserviceaccount.com'
STAGING_SERVICE_ACCOUNT = 'chromeperf-stage@appspot.gserviceaccount.com'
# pylint: disable=line-too-long
PINPOINT_SKIA_SERVICE_ACCOUNT = 'pinpoint-worker@skia-infra-public.iam.gserviceaccount.com'

_STAGING_APP_ID = 'chromeperf-stage'


def IsStagingEnvironment():
  return os.environ.get('GOOGLE_CLOUD_PROJECT') == _STAGING_APP_ID


def ServiceAccount():
  if IsStagingEnvironment():
    return STAGING_SERVICE_ACCOUNT
  return PROD_SERVICE_ACCOUNT


def AllowList():
  if IsStagingEnvironment():
    return {STAGING_SERVICE_ACCOUNT}
  return {PROD_SERVICE_ACCOUNT}


def ServiceAccountHttp(scope=EMAIL_SCOPE, timeout=None):
  """Returns the Credentials of the service account if available."""

  assert scope, "ServiceAccountHttp scope must not be None."
  credentials = _GetAppDefaultCredentials(scope)
  http = google_auth_httplib2.AuthorizedHttp(credentials)
  if timeout:
    http.timeout = timeout
  return http


def _GetAppDefaultCredentials(scope=None):
  try:
    credentials, _ = google.auth.default()
    if scope and credentials.requires_scopes:
      credentials = credentials.with_scopes([scope])
    return credentials
  except google.auth.exceptions.DefaultCredentialsError as e:
    logging.error('Error when getting the application default credentials: %s',
                  str(e))
    return None


def AuthorizeBearerToken(request):
  """
    Verifies that the request is authorized by checking the email info from
    the bearer token.
  """

  # Check that the request includes the `Authorization` header.
  if "Authorization" not in request.headers:
    logging.warning('[Auth] Missing "Authorization" header in the request.')
    return False

  access_token = request.headers["Authorization"].split("Bearer ")[1]
  token_info_url = '%s?bearer_token=%s' % (TOKEN_INFO_ENDPOINT, access_token)
  response, content = httplib2.Http(timeout=30).request(token_info_url)

  if response.status != 200:
    logging.warning(
      '[Auth] Invalid response status from token info endpoint: %s. Url: %s',
      response, token_info_url)
    return False

  content_dict = json.loads(content)
  email = content_dict.get('email')
  if email and content_dict.get('email_verified') and email in AllowList():
    return True

  logging.warning(
    '[Auth] No valid email is found in request token. Email: %s. Validation response: %s',
    email, content_dict)
  return False


def BearerTokenAuthorizer(wrapped_handler):
  @functools.wraps(wrapped_handler)
  def Wrapper(*args, **kwargs):
    if not IsStagingEnvironment() and not AuthorizeBearerToken(flask_request):
      return make_response('Failed to validate the incoming request.', 403)
    return wrapped_handler(*args, **kwargs)

  return Wrapper
