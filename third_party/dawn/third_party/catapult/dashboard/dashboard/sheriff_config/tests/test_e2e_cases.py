# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""E2E Tests for the Sheriff Config Service

This test assumes a number of things:

  - We are running against an emulator for the datastore service.
  - We are mocking the luci-config service calls and responses.

We typically run this in the testing Docker container.
"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from tests.utils import HttpMockSequenceWithDiscovery
from google.cloud import datastore
import base64
import service
import time
import unittest
from unittest import mock


class LuciPollingTest(unittest.TestCase):

  def setUp(self):
    with open(
        'tests/sample-configs-get_project_configs.json') as sample_config_file:
      self.sample_config = sample_config_file.read()
    self.app = service.CreateApp({
        'environ': {
            'GOOGLE_CLOUD_PROJECT': 'chromeperf',
            'GAE_SERVICE': 'sheriff-config',
        },
        'datastore_client':
            datastore.Client(project='chromeperf'),
        'http':
            HttpMockSequenceWithDiscovery([({
                'status': '200'
            }, self.sample_config)]),
    })
    self.maxDiff = None

  def testPollAndMatch(self):
    client = self.app.test_client()
    response = client.get(
        '/configs/update', headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)
    response = client.post(
        '/subscriptions/match',
        json={
            'path': 'Master/Bot/Test/Metric/Something',
            'stats': ['PCT_99'],
            'metadata': {
                'units': 'SomeUnit',
                'master': 'Master',
                'bot': 'Bot',
                'benchmark': 'Test',
                'metric_parts': ['Metric', 'Something'],
            }
        },
        headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)
    response_proto = response.get_json()
    self.assertDictEqual(
        response_proto, {
            'subscriptions': [{
                'config_set': 'projects/other_project',
                'revision': '0123456789abcdff',
                'subscription': {
                    'name': 'Expected 1',
                    'monorail_project_id': 'non-chromium',
                    'contact_email': 'expected-1@example.com',
                    'bug_labels': ['Some-Label'],
                    'bug_components': ['Some>Component'],
                    'auto_triage': {
                        'enable': False
                    },
                    'auto_merge': {
                        'enable': False
                    },
                    'auto_bisection': {
                        'enable': False
                    },
                    'rules': {},
                }
            }]
        })

  def testPollAndMatchTriageBisect(self):
    client = self.app.test_client()
    response = client.get(
        '/configs/update', headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)

    def Test(path, triaged, merged, bisected):
      response = client.post(
          '/subscriptions/match',
          json={
              'path': 'Master/Bot/Test/Metric/Something_' + path,
              'stats': ['PCT_99'],
              'metadata': {
                  'units': 'SomeUnit',
                  'master': 'Master',
                  'bot': 'Bot',
                  'benchmark': 'Test',
                  'metric_parts': ['Metric', 'Something'],
              }
          },
          headers={'X-Forwarded-Proto': 'https'})
      self.assertEqual(response.status_code, 200)
      response_proto = response.get_json()
      self.assertDictEqual(
          response_proto, {
              'subscriptions': [{
                  'config_set': 'projects/other_project',
                  'revision': '0123456789abcdff',
                  'subscription': {
                      'name': 'Expected 1',
                      'monorail_project_id': 'non-chromium',
                      'contact_email': 'expected-1@example.com',
                      'bug_labels': ['Some-Label'],
                      'bug_components': ['Some>Component'],
                      'auto_triage': {
                          'enable': triaged
                      },
                      'auto_merge': {
                          'enable': merged
                      },
                      'auto_bisection': {
                          'enable': bisected
                      },
                      'rules': {},
                  }
              }]
          })

    Test('Triage_Merge_Bisect', True, True, True)
    Test('NoTriage_Merge_Bisect', False, False, False)
    Test('Triage_NoMerge_NoBisect', True, False, False)
    Test('Triage_Merge_NoBisect', True, True, False)

  def testPollAndMatchMultiple(self):
    client = self.app.test_client()
    response = client.get(
        '/configs/update', headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)
    response = client.post(
        '/subscriptions/match',
        json={
            'path': 'project/platform/not-important/memory_peak',
            'stats': ['PCT_99'],
            'metadata': {
                'units': 'SomeUnit',
                'master': 'Master',
                'bot': 'Bot',
                'benchmark': 'Test',
                'metric_parts': ['Metric', 'Something'],
            }
        },
        headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)
    response_proto = response.get_json()
    self.assertDictEqual(
        response_proto, {
            'subscriptions': [{
                'config_set': 'projects/project',
                'revision': '0123456789abcdef',
                'subscription': {
                    'name': 'Config 1',
                    'contact_email': 'config-1@example.com',
                    'bug_labels': ['Some-Label'],
                    'bug_components': ['Some>Component'],
                    'auto_triage': {
                        'enable': False
                    },
                    'auto_merge': {
                        'enable': False
                    },
                    'auto_bisection': {
                        'enable': False
                    },
                    'rules': {},
                }
            }, {
                'config_set': 'projects/project',
                'revision': '0123456789abcdef',
                'subscription': {
                    'name': 'Config 2',
                    'contact_email': 'config-2@example.com',
                    'bug_labels': ['Some-Label'],
                    'bug_components': ['Some>Component'],
                    'auto_triage': {
                        'enable': False
                    },
                    'auto_merge': {
                        'enable': False
                    },
                    'auto_bisection': {
                        'enable': False
                    },
                    'rules': {},
                }
            }]
        })

  def testPollAndMatchPostFilter(self):
    client = self.app.test_client()
    response = client.get(
        '/configs/update', headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)
    response = client.post(
        '/subscriptions/match',
        json={
            'path': 'Master/Bot/Test/Metric/Something_PostFilter',
            'stats': ['PCT_99'],
            'metadata': {
                'units': 'SomeUnit',
                'master': 'Master',
                'bot': 'Bot',
                'benchmark': 'Test',
                'metric_parts': ['Metric', 'Something'],
            }
        },
        headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 404)

  def testPollAndMatchWithAnomalyConfig(self):
    client = self.app.test_client()
    response = client.get(
        '/configs/update', headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)
    response = client.post(
        '/subscriptions/match',
        json={
            'path': 'Master/Bot/Test/Metric/WithAnomalyConfig',
            'stats': ['PCT_99'],
            'metadata': {
                'units': 'SomeUnit',
                'master': 'Master',
                'bot': 'Bot',
                'benchmark': 'Test',
                'metric_parts': ['Metric', 'WithAnomalyConfig'],
            }
        },
        headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)
    response_proto = response.get_json()
    self.assertEqual(
        response_proto, {
            'subscriptions': [{
                'config_set': mock.ANY,
                'revision': mock.ANY,
                'subscription': {
                    'name': 'Expected 1',
                    'monorail_project_id': 'non-chromium',
                    'contact_email': 'expected-1@example.com',
                    'bug_labels': ['Some-Label'],
                    'bug_components': ['Some>Component'],
                    'auto_triage': {
                        'enable': False
                    },
                    'auto_merge': {
                        'enable': False
                    },
                    'auto_bisection': {
                        'enable': False
                    },
                    'rules': {},
                    'anomaly_configs': [mock.ANY],
                }
            }]
        })

  def testPollAndMatchNone(self):
    client = self.app.test_client()
    response = client.get(
        '/configs/update', headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)
    response = client.post(
        '/subscriptions/match',
        json={
            'path': 'NoMatch/Nothing/not-important/not-monitored',
            'stats': ['PCT_99'],
            'metadata': {
                'units': 'SomeUnit',
                'master': 'Master',
                'bot': 'Bot',
                'benchmark': 'Test',
                'metric_parts': ['Metric', 'Something'],
            }
        },
        headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 404)

  def testMatchInvalidRequest(self):
    client = self.app.test_client()
    response = client.post(
        '/subscriptions/match',
        json={
            'invalid_key': 'foo',
            'path': 'NoMatch/Nothing/not-important/not-monitored',
            'stats': ['PCT_99'],
            'metadata': {
                'units': 'SomeUnit',
                'master': 'Master',
                'bot': 'Bot',
                'benchmark': 'Test',
                'metric_parts': ['Metric', 'Something'],
            }
        },
        headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 400)

  def testListSubscriptions(self):
    app = service.CreateApp({
        'environ': {
            'GOOGLE_CLOUD_PROJECT': 'chromeperf',
            'GAE_SERVICE': 'sheriff-config',
        },
        'datastore_client':
            datastore.Client(project='chromeperf'),
        'http':
            HttpMockSequenceWithDiscovery([({
                'status': '200'
            }, self.sample_config), ({
                'status': '200'
            }, '{ "is_member": true }'),
                                           ({
                                               'status': '200'
                                           }, '{ "is_member": false }')]),
    })
    client = app.test_client()
    response = client.get(
        '/configs/update', headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)
    response = client.post(
        '/subscriptions/list',
        json={'identity_email': 'any@internal.com'},
        headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)
    self.assertDictEqual(
        response.get_json(), {
            'subscriptions': [{
                'config_set': 'projects/project',
                'revision': '0123456789abcdef',
                'subscription': {
                    'name': 'Config 1',
                    'contact_email': 'config-1@example.com',
                    'bug_labels': ['Some-Label'],
                    'bug_components': ['Some>Component'],
                    'auto_triage': {
                        'enable': False
                    },
                    'auto_merge': {
                        'enable': False
                    },
                    'auto_bisection': {
                        'enable': False
                    },
                    'rules': {},
                }
            }, {
                'config_set': 'projects/project',
                'revision': '0123456789abcdef',
                'subscription': {
                    'name': 'Config 2',
                    'contact_email': 'config-2@example.com',
                    'bug_labels': ['Some-Label'],
                    'bug_components': ['Some>Component'],
                    'auto_triage': {
                        'enable': False
                    },
                    'auto_merge': {
                        'enable': False
                    },
                    'auto_bisection': {
                        'enable': False
                    },
                    'rules': {},
                }
            }, {
                'config_set': 'projects/other_project',
                'revision': '0123456789abcdff',
                'subscription': {
                    'name': 'Expected 1',
                    'monorail_project_id': 'non-chromium',
                    'contact_email': 'expected-1@example.com',
                    'bug_labels': ['Some-Label'],
                    'bug_components': ['Some>Component'],
                    'auto_triage': {
                        'enable': False
                    },
                    'auto_merge': {
                        'enable': False
                    },
                    'auto_bisection': {
                        'enable': False
                    },
                    'rules': {},
                }
            }]
        })
    response = client.post(
        '/subscriptions/list',
        json={'identity_email': 'any@public.com'},
        headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)
    self.assertDictEqual(response.get_json(), {})

  def testPollAndWarmup(self):
    client = self.app.test_client()
    response = client.get(
        '/configs/update', headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)
    response = client.get('/warmup', headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)


class LuciContentChangesTest(unittest.TestCase):

  def setUp(self):
    with open(
        'tests/sample-configs-get_project_configs.json') as sample_config_file:
      self.sample_config = sample_config_file.read()
    self.maxDiff = None

  def AssertProjectConfigSet1Holds(self, client, expected_code):
    response = client.post(
        '/subscriptions/match',
        json={
            'path': 'Master/Bot/Test/Metric/Something',
            'stats': ['PCT_99'],
            'metadata': {
                'units': 'SomeUnit',
                'master': 'Master',
                'bot': 'Bot',
                'benchmark': 'Test',
                'metric_parts': ['Metric', 'Something'],
            }
        },
        headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, expected_code)
    if expected_code != 200:
      return
    response_proto = response.get_json()
    self.assertDictEqual(
        response_proto, {
            'subscriptions': [{
                'config_set': 'projects/other_project',
                'revision': '0123456789abcdff',
                'subscription': {
                    'name': 'Expected 1',
                    'monorail_project_id': 'non-chromium',
                    'contact_email': 'expected-1@example.com',
                    'bug_labels': ['Some-Label'],
                    'bug_components': ['Some>Component'],
                    'auto_triage': {
                        'enable': False
                    },
                    'auto_merge': {
                        'enable': False
                    },
                    'auto_bisection': {
                        'enable': False
                    },
                    'rules': {},
                }
            }]
        })

  def AssertProjectConfigSet2Holds(self, client, expected_code):
    response = client.post(
        '/subscriptions/match',
        json={
            'path': 'project/platform/not-important/memory_peak',
            'stats': ['PCT_99'],
            'metadata': {
                'units': 'SomeUnit',
                'master': 'Master',
                'bot': 'Bot',
                'benchmark': 'Test',
                'metric_parts': ['Metric', 'Something'],
            }
        },
        headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, expected_code)
    if expected_code != 200:
      return
    response_proto = response.get_json()
    self.assertDictEqual(
        response_proto, {
            'subscriptions': [{
                'config_set': 'projects/project',
                'revision': '0123456789abcdef',
                'subscription': {
                    'name': 'Config 1',
                    'contact_email': 'config-1@example.com',
                    'bug_labels': ['Some-Label'],
                    'bug_components': ['Some>Component'],
                    'auto_triage': {
                        'enable': False
                    },
                    'auto_merge': {
                        'enable': False
                    },
                    'auto_bisection': {
                        'enable': False
                    },
                    'rules': {},
                }
            }, {
                'config_set': 'projects/project',
                'revision': '0123456789abcdef',
                'subscription': {
                    'name': 'Config 2',
                    'contact_email': 'config-2@example.com',
                    'bug_labels': ['Some-Label'],
                    'bug_components': ['Some>Component'],
                    'auto_triage': {
                        'enable': False
                    },
                    'auto_merge': {
                        'enable': False
                    },
                    'auto_bisection': {
                        'enable': False
                    },
                    'rules': {},
                }
            }]
        })

  def testPollAndEmptyConfigs(self):
    app = service.CreateApp({
        'environ': {
            'GOOGLE_CLOUD_PROJECT': 'chromeperf',
            'GAE_SERVICE': 'sheriff-config',
        },
        'datastore_client':
            datastore.Client(project='chromeperf'),
        'http':
            HttpMockSequenceWithDiscovery([({
                'status': '200'
            }, '{}')])
    })
    client = app.test_client()
    response = client.get(
        '/configs/update', headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)

  def testPollConfigAddsAndRemoves(self):
    with open('tests/sample-configs-get_project_configs_reduced.json'
             ) as sample_config_file:
      sample_config_reduced = sample_config_file.read()
    app = service.CreateApp({
        'environ': {
            'GOOGLE_CLOUD_PROJECT': 'chromeperf',
            'GAE_SERVICE': 'sheriff-config',
        },
        'datastore_client':
            datastore.Client(project='chromeperf'),
        'http':
            HttpMockSequenceWithDiscovery([({
                'status': '200'
            }, self.sample_config), ({
                'status': '200'
            }, sample_config_reduced)]),
    })

    # Step 1: Get one configuration with two config sets.
    client = app.test_client()
    response = client.get(
        '/configs/update', headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)

    self.AssertProjectConfigSet1Holds(client, 200)
    self.AssertProjectConfigSet2Holds(client, 200)

    # Step 2: Get another configuration, this time with just one config set.
    response = client.get(
        '/configs/update', headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)

    # Update doesn't take effect because of caching
    self.AssertProjectConfigSet1Holds(client, 200)
    self.AssertProjectConfigSet2Holds(client, 200)

    # mocking utils.Time to invalid caching
    with mock.patch('utils.Time') as mock_time:
      mock_time.method.return_value = (time.time() + 60)
      self.AssertProjectConfigSet1Holds(client, 404)
      self.AssertProjectConfigSet2Holds(client, 200)

  def testInvalidContentPulled(self):
    subscription = """
subscriptions: {
  name: "Missing Email"
  bug_labels: ["Some-Label"]
  bug_components: ["Some>Component"]
  patterns: [{glob: "project/**"}]
}"""
    invalid_content = """
{
  "configs": [
    {
      "url": "https://example.com/p/s/+/0123456789abcde/chromeperf-sheriff.cfg",
      "content": "%s",
      "content_hash": "v1:somehash",
      "config_set": "projects/project",
      "revision": "0123456789abcdef"
    }
  ]
}""" % (base64.standard_b64encode(bytearray(subscription, 'utf-8')).decode(),)
    app = service.CreateApp({
        'environ': {
            'GOOGLE_CLOUD_PROJECT': 'chromeperf',
            'GAE_SERVICE': 'sheriff-config',
        },
        'datastore_client':
            datastore.Client(project='chromeperf'),
        'http':
            HttpMockSequenceWithDiscovery([({
                'status': '200'
            }, invalid_content), ({
                'status': '200'
            }, self.sample_config), ({
                'status': '200'
            }, invalid_content)]),
    })
    client = app.test_client()

    # Step 1: Get an invalid config.
    response = client.get(
        '/configs/update', headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 500)

    # Step 2: Get a config that's valid.
    response = client.get(
        '/configs/update', headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 200)

    self.AssertProjectConfigSet1Holds(client, 200)
    self.AssertProjectConfigSet2Holds(client, 200)

    # Step 3: Get a config that's invalid, but ensure that the valid config
    # holds.
    response = client.get(
        '/configs/update', headers={'X-Forwarded-Proto': 'https'})
    self.assertEqual(response.status_code, 500)
    self.AssertProjectConfigSet1Holds(client, 200)
    self.AssertProjectConfigSet2Holds(client, 200)
