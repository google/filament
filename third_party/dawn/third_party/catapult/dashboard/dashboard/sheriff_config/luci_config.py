# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""API Wrapper for the luci-config service."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from googleapiclient import errors
from google.cloud import datastore
import base64
import collections
import httplib2
import validator
import matcher as matcher_module
import utils

from dashboard.protobuf import sheriff_pb2

# The path which we will look for in projects.
SHERIFF_CONFIG_PATH = 'chromeperf-sheriffs.cfg'


class Error(Exception):
  """All errors associated with the luci_config module."""


class FetchError(Error):

  def __init__(self, error):
    super().__init__('Failed fetching project configs: {}'.format(error))
    self.error = error


class InvalidConfigError(Error):

  def __init__(self, config, fields):
    super().__init__('Config (%r) missing required fields: %r' %
                     (config, fields))
    self.fields = fields
    self.config = config


class InvalidContentError(Error):

  def __init__(self, error, config):
    super().__init__('Config (%r) content decoding error: %s' % (config, error))
    self.config = config
    self.error = error


def FindAllSheriffConfigs(client):
  """Finds all the project configs for chromeperf sheriffs."""
  try:
    configs = client.get_project_configs(path=SHERIFF_CONFIG_PATH).execute()
  except (errors.HttpError, httplib2.HttpLib2Error) as e:
    raise FetchError(e) from e
  return configs


def StoreConfigs(client, configs):
  """Takes configuration dictionaries and persists them in the datastore.

  Args:
    client: The client for accessing the storage API.
    configs: The config objects we'd like to store.

  Raises:
    InvalidConfigError: raised when config has missing required fields.
    InvalidConfigFieldError: raised when the content is not a decodable base64
      encoded JSON document.

  Returns:
    None
  """
  # We group the Subscription instances along a SubscriptionIndex entity. A
  # SubscriptionIndex will always have the latest version of the subscription
  # configurations, for easy lookup, as a list. We will only update the
  # SubscriptionIndex if there were any new revisions in the configuration sets
  # that we've gotten.
  subscription_index_key = client.key('SubscriptionIndex', 'global')

  if not configs:
    # We should clear the existing set of configs, if there are any.
    with client.transaction():
      subscription_index = datastore.Entity(
          key=subscription_index_key, exclude_from_indexes=['config_sets'])
      subscription_index.update({'config_sets': []})
      client.put(subscription_index)
      return

  required_fields = {'config_set', 'content', 'revision', 'url', 'content_hash'}

  def Transform(config):
    missing_fields = required_fields.difference(config)
    if len(missing_fields):
      raise InvalidConfigError(config, missing_fields)

    # We use a combination of the config set, revision, and content_hash to
    # identify this particular configuration.
    key = client.key(
        'Subscription',
        '{config_set}:{revision}'.format(**config),
        parent=subscription_index_key)
    try:
      sheriff_config = validator.Validate(
          base64.standard_b64decode(config['content']))
    except validator.Error as error:
      raise InvalidContentError(error, config) from error

    entity = datastore.Entity(
        key=key, exclude_from_indexes=['sheriff_config', 'url'])
    entity.update({
        'config_set': config['config_set'],
        'revision': config['revision'],
        'content_hash': config['content_hash'],
        'url': config['url'],
        'sheriff_config': sheriff_config.SerializeToString()
    })
    return (key, entity)

  entities = dict(Transform(config) for config in configs)
  with client.transaction():
    # First, lookup the keys for each of the entities to see whether all of them
    # have already been stored. This allows us to avoid performing write
    # operations that end up not doing any updates.
    found_entities = client.get_multi(entities)

    # Next, we only put the entities that are missing.
    for found_entity in found_entities:
      del entities[found_entity.key]

    # From here we'll update an entity in the datastore which lists the grouped
    # keys in the datastore. This will serve as an index on the latest set of
    # configurations.
    subscription_index = datastore.Entity(
        key=subscription_index_key, exclude_from_indexes=['config_sets'])
    subscription_index.update({
        'config_sets': [
            '{config_set}:{revision}'.format(**config) for config in configs
        ]
    })
    client.put_multi(list(entities.values()) + [subscription_index])


class Matcher:

  def __init__(self, subscription):
    self._match_subscription = matcher_module.CompileRules(
        subscription.rules, ignore_broken=True)

    if subscription.auto_triage.enable:
      self._match_auto_triage = lambda s: True
    else:
      self._match_auto_triage = matcher_module.CompileRules(
          subscription.auto_triage.rules, ignore_broken=True)

    if subscription.auto_merge.enable:
      self._match_auto_merge = lambda s: True
    else:
      self._match_auto_merge = matcher_module.CompileRules(
          subscription.auto_merge.rules, ignore_broken=True)

    if subscription.auto_bisection.enable:
      self._match_auto_bisection = lambda s: True
    else:
      self._match_auto_bisection = matcher_module.CompileRules(
          subscription.auto_bisection.rules, ignore_broken=True)

    def Reduced(config):
      copied_config = sheriff_pb2.AnomalyConfig()
      copied_config.CopyFrom(config)
      copied_config.rules.Clear()
      return copied_config

    self._anomaly_config_matchers = [(
        matcher_module.CompileRules(
            config.rules,
            ignore_broken=True,
        ),
        Reduced(config),
    ) for config in subscription.anomaly_configs]

  def MatchSubscription(self, test):
    return self._match_subscription(test)

  def MatchAutoTriage(self, test):
    return self._match_auto_triage(test)

  def MatchAutoMerge(self, test):
    return self._match_auto_merge(test)

  def MatchAutoBisection(self, test):
    return self._match_auto_bisection(test)

  def GetAnomalyConfig(self, test):
    for config_matcher, config in self._anomaly_config_matchers:
      if config_matcher(test):
        return config
    return None


def GetMatcher(revision, subscription):
  if not hasattr(GetMatcher, 'cache'):
    GetMatcher.cache = dict()
  cached = GetMatcher.cache.get(subscription.name)
  if cached and cached.revision == revision:
    return cached.matcher

  cached_tuple = collections.namedtuple('CachedMatcher',
                                        ['revision', 'matcher'])
  matcher = Matcher(subscription)
  GetMatcher.cache[subscription.name] = cached_tuple(revision, matcher)
  return matcher


@utils.LRUCacheWithTTL(ttl_seconds=60, maxsize=2)
def ListAllConfigs(client):
  """Yield tuples of (config_set, revision, subscription)."""
  with client.transaction(read_only=True):
    # First look up the global index.
    subscription_index_key = client.key('SubscriptionIndex', 'global')
    subscription_index = client.get(subscription_index_key)
    if subscription_index is None:
      return None

    # Then for each instance in the 'config_sets', create a key based on the
    # subscription_index_key as a parent, and look those up in one go.
    config_sets = client.get_multi([
        client.key('Subscription', key, parent=subscription_index_key)
        for key in subscription_index['config_sets']
    ])

  # From there we can then go through each of the subscriptions in the retrieved
  # configs
  subscriptions = []
  for config in config_sets:
    sheriff_config = sheriff_pb2.SheriffConfig()
    sheriff_config.ParseFromString(config['sheriff_config'])
    for subscription in sheriff_config.subscriptions:
      subscriptions.append((
          config['config_set'],
          config['revision'],
          subscription,
      ))
  return subscriptions


def FindMatchingConfigs(client, request):
  """Yield tuples of (config_set, revision, subscription)."""
  for config_set, revision, subscription in ListAllConfigs(client):
    matcher = GetMatcher(revision, subscription)
    if matcher.MatchSubscription(request.path):
      subscription.auto_triage.enable = matcher.MatchAutoTriage(request.path)
      subscription.auto_merge.enable = (
          subscription.auto_triage.enable
          and matcher.MatchAutoMerge(request.path))
      subscription.auto_bisection.enable = (
          subscription.auto_triage.enable
          and matcher.MatchAutoBisection(request.path))
      yield (config_set, revision, subscription)


def CopyNormalizedSubscription(src, dst):
  dst.CopyFrom(src)
  # We shouldn't use rules outside the sheriff-config in any case.
  # Maybe allow being explicitily requsted for debug usage later.
  dst.rules.Clear()

  # We also should not provide any of the anomaly configs.
  del dst.anomaly_configs[:]

  auto_triage_enable = dst.auto_triage.enable
  dst.auto_triage.Clear()
  dst.auto_triage.enable = auto_triage_enable

  auto_merge_enable = dst.auto_merge.enable
  dst.auto_merge.Clear()
  dst.auto_merge.enable = auto_merge_enable

  auto_bisection_enable = dst.auto_bisection.enable
  dst.auto_bisection.Clear()
  dst.auto_bisection.enable = auto_bisection_enable
