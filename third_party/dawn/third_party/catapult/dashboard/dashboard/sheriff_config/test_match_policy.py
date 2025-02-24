# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import unittest
import match_policy
import service_client
from dashboard.protobuf import sheriff_config_pb2
from dashboard.protobuf import sheriff_pb2
from tests.utils import HttpMockSequenceWithDiscovery


class MatchPolicyTest(unittest.TestCase):

  def testOverlap(self):
    request = sheriff_config_pb2.MatchRequest()
    configs = [
        ('', '',
         sheriff_pb2.Subscription(
             name='Private',
             visibility=sheriff_pb2.Subscription.INTERNAL_ONLY,
         )),
        ('', '',
         sheriff_pb2.Subscription(
             name='Public',
             visibility=sheriff_pb2.Subscription.PUBLIC,
         )),
    ]
    configs = match_policy.FilterSubscriptionsByPolicy(request, configs)
    self.assertEqual(['Private'], [s.name for _, _, s in configs])

  def testListPrivate(self):
    configs = [
        ('', '',
         sheriff_pb2.Subscription(
             name='Private',
             visibility=sheriff_pb2.Subscription.INTERNAL_ONLY,
         )),
        ('', '',
         sheriff_pb2.Subscription(
             name='Public',
             visibility=sheriff_pb2.Subscription.PUBLIC,
         )),
    ]
    http = HttpMockSequenceWithDiscovery([
        ({
            'status': '200'
        }, '{ "is_member": true }'),
        ({
            'status': '200'
        }, '{ "is_member": false }'),
    ])
    _ = service_client.CreateServiceClient(
        'https://luci-config.appspot.com/_ah/api', 'config', 'v1', http=http)
    auth_client = service_client.CreateServiceClient(
        'https://chrome-infra-auth.appspot.com/_ah/api',
        'auth',
        'v1',
        http=http)
    request = sheriff_config_pb2.ListRequest(identity_email='foo@bar1')
    configs = match_policy.FilterSubscriptionsByIdentity(
        auth_client, request, configs)
    self.assertEqual(['Private', 'Public'], [s.name for _, _, s in configs])

    request = sheriff_config_pb2.ListRequest(identity_email='foo@bar2')
    configs = match_policy.FilterSubscriptionsByIdentity(
        auth_client, request, configs)
    self.assertEqual(['Public'], [s.name for _, _, s in configs])

    request = sheriff_config_pb2.ListRequest()
    configs = match_policy.FilterSubscriptionsByIdentity(
        auth_client, request, configs)
    self.assertEqual(['Public'], [s.name for _, _, s in configs])
