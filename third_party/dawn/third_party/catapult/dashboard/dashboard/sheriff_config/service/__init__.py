# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Support python3
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import base64
import logging
import time
import os

from flask import Flask, request, jsonify
from flask_talisman import Talisman
from google.cloud import datastore
from google.protobuf import json_format
import google.auth

from dashboard.protobuf import sheriff_config_pb2
import luci_config
import match_policy
import service_client
import validator


class Error(Exception):
  """Base class for service-related set-up errors."""


class MissingEnvironmentVars(Error):

  def __init__(self, env_vars):
    self.env_vars = env_vars
    super().__init__()

  def __str__(self):
    return 'Missing environment variables: %r' % (self.env_vars)


def CreateApp(test_config=None):
  """Factory for Flask App configuration.

  This is the factory function for setting up the service. This function
  contains all the HTTP routing information and handlers that are registered for
  the Flask application.

  Args:
    test_config: a dict of overrides for the app configuration.

  Raises:
    MissingEnvironmentVars: raised we find that we don't have the AppEngine
    environment variables available when creating the application.

  Returns:
    A Flask application configured with the appropriate URL handlers.
  """
  app = Flask(__name__, instance_relative_config=True)
  _ = Talisman(app)

  environ = os.environ if test_config is None else test_config.get(
      'environ', {})

  # We can set up a preconfigured HTTP instance from the test_config, otherwise
  # we'll use the default auth for the production environment.
  client_config = {}
  if test_config:
    client_config['http'] = test_config.get('http')
  else:
    client_config['credentials'], _ = google.auth.default()

  # In the python37 environment, we need to synthesize the URL from the
  # various parts in the environment variable, because we do not have access
  # to the appengine APIs in the python37 standard environment.
  domain_parts = {
      'app_id': environ.get('GOOGLE_CLOUD_PROJECT', ''),
      'service': environ.get('GAE_SERVICE', ''),
  }
  empty_keys = [k for k, v in domain_parts.items() if len(v) == 0]
  if len(empty_keys):
    raise MissingEnvironmentVars(empty_keys)
  domain = '{parts[service]}-dot-{parts[app_id]}.appspot.com'.format(
      parts=domain_parts)

  # We create an instance of the luci-config and Auth client, which we'll use
  # in all requests handled in this application.
  config_client = service_client.CreateServiceClient(
      'https://luci-config.appspot.com/_ah/api', 'config', 'v1',
      **client_config)
  auth_client = service_client.CreateServiceClient(
      'https://chrome-infra-auth.appspot.com/_ah/api', 'auth', 'v1',
      **client_config)

  # First we check whether the test_config already has a predefined
  # datastore_client.
  if test_config:
    datastore_client = test_config.get('datastore_client')
  else:
    datastore_client = datastore.Client()

  @app.route('/service-metadata')
  def ServiceMetadata():  # pylint: disable=unused-variable
    return jsonify({
        'version': '1.0',
        'validation': {
            'patterns': [{
                'config_set': 'regex:projects/.+',
                'path': 'regex:chromeperf-sheriffs.cfg'
            }],
            'url': 'https://%s/configs/validate' % (domain)
        }
    })

  @app.route('/warmup')
  def Warmup():  # pylint: disable=unused-variable
    """Caching configs and compiled patterns for warmup."""
    configs = luci_config.ListAllConfigs(
        datastore_client, _cache_timestamp=time.time() + 10)
    for _, revision, subscription in configs:
      luci_config.GetMatcher(revision, subscription)
    return jsonify({})

  @app.route('/configs/validate', methods=['POST'])
  def Validate():  # pylint: disable=unused-variable
    validation_request = request.get_json()
    if validation_request is None:
      return u'Invalid request.', 400
    for member in ('config_set', 'path', 'content'):
      if member not in validation_request:
        return u'Missing \'%s\' member in request.' % (member), 400
    try:
      _ = validator.Validate(
          base64.standard_b64decode(validation_request['content']))
    except validator.Error as error:
      return jsonify({
          'messages': [{
              'path': validation_request['path'],
              'severity': 'ERROR',
              'text': '%s' % (error)
          }]
      })
    return jsonify({})

  @app.route('/configs/update')
  def UpdateConfigs():  # pylint: disable=unused-variable
    """Poll the luci-config service."""
    try:
      configs = luci_config.FindAllSheriffConfigs(config_client)
      luci_config.StoreConfigs(datastore_client, configs.get('configs', []))
      return jsonify({})
    except (luci_config.InvalidConfigError,
            luci_config.InvalidContentError) as error:
      logging.warning('loading configs from luci-config failed: %s', error)
      return jsonify({}), 500

  @app.route('/subscriptions/match', methods=['POST'])
  def MatchSubscriptions():  # pylint: disable=unused-variable
    """Match the subscriptions given the request.

    This is an API handler, which requires that we have an authenticated user
    making the request. We'll require that the user be a service account
    (from the main dashboard service).

    TODO(fancl): Allow access from an authenticated user. We'll enforce
    that we're only serving "public" Subscriptions to non-privileged users.
    """

    try:
      match_request = json_format.Parse(request.get_data(),
                                        sheriff_config_pb2.MatchRequest())
    except json_format.ParseError as error:
      return jsonify(
          {'messages': [{
              'severity': 'ERROR',
              'text': '%s' % (error)
          }]}), 400
    match_response = sheriff_config_pb2.MatchResponse()
    configs = list(
        luci_config.FindMatchingConfigs(datastore_client, match_request))
    configs = match_policy.FilterSubscriptionsByPolicy(match_request, configs)
    for config_set, revision, subscription in configs:
      subscription_metadata = match_response.subscriptions.add()
      subscription_metadata.config_set = config_set
      subscription_metadata.revision = revision
      luci_config.CopyNormalizedSubscription(
          subscription,
          subscription_metadata.subscription,
      )

      # Then we find one anomaly config that matches.
      matcher = luci_config.GetMatcher(revision, subscription)
      anomaly_config = matcher.GetAnomalyConfig(match_request.path)
      if anomaly_config:
        subscription_metadata.subscription.anomaly_configs.append(
            anomaly_config)
    if not match_response.subscriptions:
      return jsonify({}), 404
    return (json_format.MessageToJson(
        match_response, preserving_proto_field_name=True), 200, {
            'Content-Type': 'application/json'
        })

  @app.route('/subscriptions/list', methods=['POST'])
  def ListSubscriptions():  # pylint: disable=unused-variable
    """List all visible subscriptions based on identity.

    This is an API handler, which requires that we have an authenticated user
    making the request. We'll require that the user be a service account
    (from the main dashboard service).

    """

    try:
      list_request = json_format.Parse(request.get_data(),
                                       sheriff_config_pb2.ListRequest())
    except json_format.ParseError as error:
      return jsonify(
          {'messages': [{
              'severity': 'ERROR',
              'text': '%s' % (error)
          }]}), 400
    list_response = sheriff_config_pb2.ListResponse()
    configs = list(luci_config.ListAllConfigs(datastore_client))
    configs = match_policy.FilterSubscriptionsByIdentity(
        auth_client, list_request, configs)
    for config_set, revision, subscription in configs:
      subscription_metadata = list_response.subscriptions.add()
      subscription_metadata.config_set = config_set
      subscription_metadata.revision = revision
      luci_config.CopyNormalizedSubscription(subscription,
                                             subscription_metadata.subscription)
    return (json_format.MessageToJson(
        list_response, preserving_proto_field_name=True), 200, {
            'Content-Type': 'application/json'
        })

  return app
