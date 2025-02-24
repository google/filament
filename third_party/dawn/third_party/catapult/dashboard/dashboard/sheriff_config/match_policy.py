# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Policies to ensure internal or restricted information won't be leaked."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import logging

from dashboard.protobuf import sheriff_pb2
from utils import LRUCacheWithTTL


def IsPrivate(config):
  _, _, subscription = config
  return subscription.visibility == sheriff_pb2.Subscription.INTERNAL_ONLY


def FilterSubscriptionsByPolicy(request, configs):
  # We consider sending information to both private and public subscribers a
  # potential risk of leaking restricted information. So only return private
  # subscribers when matched both public and private.
  privates = [IsPrivate(c) for c in configs]
  if any(privates) and not all(privates):
    logging.warning('Private sheriff overlaps with public: %s, %s',
                    request.path, [(config_set, subscription.name)
                                   for config_set, _, subscription in configs])
    return [c for c in configs if IsPrivate(c)]
  return configs


@LRUCacheWithTTL(ttl_seconds=60, maxsize=128)
def IsGroupMember(auth_client, email, group):
  if not email:
    return False
  request = auth_client.membership(identity=email, group=group)
  response = request.execute()
  is_member = response['is_member']
  return is_member


def FilterSubscriptionsByIdentity(auth_client, request, configs):
  # Only return private subscribers to internal user
  is_internal = IsGroupMember(auth_client, request.identity_email,
                              'chromeperf-access')
  if not is_internal:
    return [c for c in configs if not IsPrivate(c)]
  return configs
