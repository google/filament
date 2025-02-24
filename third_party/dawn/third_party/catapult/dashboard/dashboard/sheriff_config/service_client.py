# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Utilities for service client"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from googleapiclient import discovery
from googleapiclient import errors


class Error(Exception):
  """All errors associated with the service client module."""


class DiscoveryError(Error):

  def __init__(self, error):
    super().__init__('Service discovery failed: {}'.format(error))
    self.error = error


class BadArgumentError(Error):

  def __init__(self, err_str):
    super().__init__('Bad Argument: {}'.format(err_str))


def CreateServiceClient(api_root, api, version, http=None, credentials=None):
  """Factory function for a creating a config client.

  This uses the discovery API to generate a service object corresponding to the
  API we're using from luci-config.

  Args:
    api_root: the URL through which the API will be configured (default:
      API_ROOT)
    api: the service name (default: 'config')
    version: the version of the API (default: 'v1')
    http: a fully configured HTTP client/request
    credentials: a google.auth.Credential, directly supported by the
      googleapiclient library (default: None)
  Returns:
    client: Service client
  Raises:
    BadArgumentError: Both http and credentials are None
    DiscoveryError: Error from discovery
  """
  if not (http or credentials):
    raise BadArgumentError('need http or credentials')

  # ImportError: file_cache is unavailable when using oauth2client >= 4.0.0
  # https://github.com/googleapis/google-api-python-client/issues/299
  # cache_discovery=False
  discovery_url = '%s/discovery/v1/apis/%s/%s/rest' % (api_root, api, version)
  try:
    if credentials:
      client = discovery.build(
          api,
          version,
          discoveryServiceUrl=discovery_url,
          credentials=credentials,
          cache_discovery=False)
    else:
      client = discovery.build(
          api,
          version,
          discoveryServiceUrl=discovery_url,
          http=http,
          cache_discovery=False)
  except (errors.HttpError, errors.UnknownApiNameOrVersion) as e:
    raise DiscoveryError(e) from e
  return client
