# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Polls the sheriff_config service. (Copied from dashboard/)"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import json
import logging
import requests
import time

from application import utils


IAM_API_ROOT = 'https://iamcredentials.googleapis.com/v1'
SIGNJWT_ENDPOINT = IAM_API_ROOT + '/projects/-/serviceAccounts/{}:signJwt'
JWT_EXPIRE_LIMIT = 3600

class InternalServerError(Exception):
  """An error indicating that something unexpected happens."""


def GetSheriffConfigClient():
  """Get a cached SheriffConfigClient instance.
  Most code should use this rather than constructing a SheriffConfigClient
  directly.
  """
  if not hasattr(GetSheriffConfigClient, '_client'):
    GetSheriffConfigClient._client = SheriffConfigClient()
  return GetSheriffConfigClient._client


class SheriffConfigClient:
  """Wrapping of sheriff-config HTTP API."""

  _Subscription = None

  def __init__(self):
    """Make the Cloud Endpoints request from this handler."""
    # Defer as many imports as possible until here, to ensure AppEngine
    # workarounds for protobuf import paths are fully installed.
    self._InitAuthHeaders()

  def _InitAuthHeaders(self):
    if utils.IsStagingEnvironment():
      sa_email = 'chromeperf-stage@appspot.gserviceaccount.com'
    else:
      sa_email = 'chromeperf@appspot.gserviceaccount.com'
    audience = 'sheriff-config-dot-chromeperf.appspot.com'
    url = SIGNJWT_ENDPOINT.format(sa_email)
    now = int(time.time())

    payload = {
        'iat': now,
        # expires after 'expiry_length' seconds.
        "exp": now + JWT_EXPIRE_LIMIT,
        # iss must match 'issuer' in the security configuration in your
        # swagger spec (e.g. service account email). It can be any string.
        'iss': sa_email,
        # aud must be either your Endpoints service name, or match the value
        # specified as the 'x-google-audience' in the OpenAPI document.
        'aud':  audience,
        # sub and email should match the service account's email address
        'sub': sa_email,
        'email': sa_email
    }
    body = {
      "delegates": [],
      "payload": json.dumps(payload)
    }
    http = utils.ServiceAccountHttp(timeout=30)
    response, content = http.request(url, method='POST', body=json.dumps(body))

    if response.get('status', None) != '200':
      logging.error('Failed to generated signed jwt. Response: %s', response)
      return

    jwt_token = json.loads(content.decode('utf-8')).get('signedJwt')
    self.auth_header = {
        'Authorization': 'Bearer {}'.format(jwt_token),
        'Accept': 'application/json',
        'Content-Type': 'application/json'
    }

  def Match(self, path, check=False):
    url = 'https://sheriff-config-dot-chromeperf.appspot.com/subscriptions/match'

    response = requests.post(url, headers=self.auth_header, json={'path': path})

    if response.status_code == 401:
      logging.debug('Request unauthorized. Will renew the jwt token and retry.')
      self._InitAuthHeaders()
      response = requests.post(
        url, headers=self.auth_header, json={'path': path})
    if response.status_code == 404:  # If no subscription matched
      return [], None
    if not response.ok:
      err_msg = '%r\n%s' % (response, response.text)
      if check:
        raise InternalServerError(err_msg)
      return None, err_msg

    resp_json = json.loads(response.content.decode('utf-8'))
    subscriptions = resp_json.get('subscriptions', None)
    if subscriptions is None:
      return [], None

    return subscriptions, None
