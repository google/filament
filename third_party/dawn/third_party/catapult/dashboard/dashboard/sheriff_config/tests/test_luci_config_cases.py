# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Tests for the luci_config client."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import unittest
from googleapiclient.http import HttpMockSequence
import luci_config
import service_client
from tests.utils import HttpMockSequenceWithDiscovery


class LuciConfigTest(unittest.TestCase):

  # TODO(dberris): Implement more tests as we gather more use-cases.
  def testFindingAllSheriffConfigs(self):
    with open('tests/configs-projects-empty.json') as configs_get_projects_file:
      configs_get_projects_response = configs_get_projects_file.read()
    http = HttpMockSequenceWithDiscovery([
        ({
            'status': '200'
        }, configs_get_projects_response),
    ])
    service = service_client.CreateServiceClient(
        'https://luci-config.appspot.com/_ah/api', 'config', 'v1', http=http)
    _ = service_client.CreateServiceClient(
        'https://chrome-infra-auth.appspot.com/_ah/api',
        'auth',
        'v1',
        http=http)
    self.assertEqual({}, luci_config.FindAllSheriffConfigs(service))

  def testFailingCreateConfigClient(self):
    with self.assertRaisesRegex(service_client.DiscoveryError,
                                'Service discovery failed:'):
      http = HttpMockSequence([({'status': '403'}, 'Forbidden')])
      _ = service_client.CreateServiceClient(
          'https://luci-config.appspot.com/_ah/api', 'config', 'v1', http=http)
