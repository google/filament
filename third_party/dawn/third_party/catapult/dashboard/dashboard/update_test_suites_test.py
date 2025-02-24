# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from flask import Flask
import unittest
import webtest

from google.appengine.ext import ndb

from dashboard import update_test_suites
from dashboard.common import descriptor
from dashboard.common import namespaced_stored_object
from dashboard.common import stored_object
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import graph_data

flask_app = Flask(__name__)


@flask_app.route('/update_test_suites', methods=['GET', 'POST'])
def UpdateTestSuitesPost():
  return update_test_suites.UpdateTestSuitesPost()


class ListTestSuitesTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.testapp = webtest.TestApp(flask_app)
    testing_common.SetIsInternalUser('internal@chromium.org', True)
    self.UnsetCurrentUser()
    stored_object.Set(descriptor.PARTIAL_TEST_SUITES_KEY, [
        'TEST_PARTIAL_TEST_SUITE',
    ])
    stored_object.Set(descriptor.GROUPABLE_TEST_SUITE_PREFIXES_KEY, [
        'TEST_GROUPABLE%',
    ])
    descriptor.Descriptor.ResetMemoizedConfigurationForTesting()

  def testFetchCachedTestSuites_NotEmpty(self):
    # If the cache is set, then whatever's there is returned.
    key = namespaced_stored_object.NamespaceKey(
        update_test_suites._LIST_SUITES_CACHE_KEY)
    stored_object.Set(key, {'foo': 'bar'})
    self.assertEqual({'foo': 'bar'}, update_test_suites.FetchCachedTestSuites())

  def _AddSampleData(self):
    testing_common.AddTests(
        ['Chromium'], ['win7', 'mac'], {
            'dromaeo': {
                'dom': {},
                'jslib': {},
            },
            'scrolling': {
                'commit_time': {
                    'www.yahoo.com': {},
                    'www.cnn.com': {},
                },
                'commit_time_ref': {},
            },
            'really': {
                'nested': {
                    'very': {
                        'deeply': {
                            'subtest': {}
                        }
                    },
                    'very_very': {}
                }
            },
        })

  def testPost_ForcesCacheUpdate(self):
    key = namespaced_stored_object.NamespaceKey(
        update_test_suites._LIST_SUITES_CACHE_KEY)
    stored_object.Set(key, {'foo': 'bar'})
    self.assertEqual({'foo': 'bar'}, update_test_suites.FetchCachedTestSuites())
    self._AddSampleData()
    # Because there is something cached, the cache is
    # not automatically updated when new data is added.
    self.assertEqual({'foo': 'bar'}, update_test_suites.FetchCachedTestSuites())

    stored_object.Set(
        namespaced_stored_object.NamespaceKey(
            update_test_suites.TEST_SUITES_2_CACHE_KEY), ['foo'])
    self.assertEqual(['foo'], update_test_suites.FetchCachedTestSuites2())

    # Making a request to /udate_test_suites forces an update.
    self.testapp.post('/update_test_suites')
    self.assertEqual(
        {
            'dromaeo': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
            'scrolling': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
            'really': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
        }, update_test_suites.FetchCachedTestSuites())

    self.assertEqual(['dromaeo', 'really', 'scrolling'],
                     update_test_suites.FetchCachedTestSuites2())

  def testPost_InternalOnly(self):
    self.SetCurrentUser('internal@chromium.org')
    self._AddSampleData()
    master_key = ndb.Key('Master', 'Chromium')
    graph_data.Bot(
        id='internal_mac', parent=master_key, internal_only=True).put()
    t = graph_data.TestMetadata(
        id='Chromium/internal_mac/internal_test', internal_only=True)
    t.UpdateSheriff()
    t.put()

    self.testapp.post('/update_test_suites?internal_only=true')

    self.assertEqual(
        {
            'dromaeo': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
            'internal_test': {
                'mas': {
                    'Chromium': {
                        'internal_mac': False
                    }
                },
                'dep': False,
            },
            'scrolling': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
            'really': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
        }, update_test_suites.FetchCachedTestSuites())

  def testFetchCachedTestSuites_Empty_UpdatesWhenFetching(self):
    # If the cache is not set at all, then FetchCachedTestSuites
    # just updates the cache before returning the list.
    self._AddSampleData()
    self.assertEqual(
        {
            'dromaeo': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
            'scrolling': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
            'really': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
        }, update_test_suites.FetchCachedTestSuites())

  def testFetchSuites_BasicDescription(self):
    self._AddSampleData()

    for test_path in ['Chromium/win7/scrolling', 'Chromium/mac/scrolling']:
      test = utils.TestKey(test_path).get()
      test.description = 'Description string.'
      test.UpdateSheriff()
      test.put()

    self.assertEqual(
        {
            'dromaeo': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
            'scrolling': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'des': 'Description string.',
                'dep': False,
            },
            'really': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
        }, update_test_suites.FetchCachedTestSuites())

  def testFetchSuites_DifferentMasters(self):
    # If the cache is not set at all, then FetchCachedTestSuites
    # just updates the cache before returning the list.
    self._AddSampleData()
    testing_common.AddTests(['ChromiumFYI'], ['linux'], {
        'sunspider': {
            'Total': {},
        },
    })
    self.assertEqual(
        {
            'dromaeo': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
            'scrolling': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
            'really': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
            'sunspider': {
                'mas': {
                    'ChromiumFYI': {
                        'linux': False
                    }
                },
                'dep': False,
            },
        }, update_test_suites._CreateTestSuiteDict())

  def testFetchSuites_SingleDeprecatedBot(self):
    self._AddSampleData()

    # For another test suite, set it as deprecated on both bots -- it should
    # be marked as deprecated in the response dict.
    for bot in ['win7']:
      test = utils.TestKey('Chromium/%s/really' % bot).get()
      test.deprecated = True
      test.UpdateSheriff()
      test.put()

    self.assertEqual(
        {
            'dromaeo': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
            'scrolling': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
            'really': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': True
                    }
                },
                'dep': False,
            },
        }, update_test_suites._CreateTestSuiteDict())

  def testFetchSuites_AllDeprecatedBots(self):
    self._AddSampleData()

    # For another test suite, set it as deprecated on both bots -- it should
    # be marked as deprecated in the response dict.
    for bot in ['win7', 'mac']:
      test = utils.TestKey('Chromium/%s/really' % bot).get()
      test.deprecated = True
      test.UpdateSheriff()
      test.put()

    self.assertEqual(
        {
            'dromaeo': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
            'scrolling': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
            'really': {
                'mas': {
                    'Chromium': {
                        'mac': True,
                        'win7': True
                    }
                },
                'dep': True,
            },
        }, update_test_suites._CreateTestSuiteDict())

  def testFetchSuites_BasicMonitored(self):
    self._AddSampleData()

    self.assertEqual(
        {
            'dromaeo': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
            'scrolling': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
            'really': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
        }, update_test_suites._CreateTestSuiteDict())

  def testFetchSuites_MultipleMonitored(self):
    self._AddSampleData()
    testing_common.AddTests(['ChromiumFYI'], ['linux'], {
        'dromaeo': {
            'foo': {},
        },
    })

    self.assertEqual(
        {
            'dromaeo': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    },
                    'ChromiumFYI': {
                        'linux': False
                    }
                },
                'dep': False,
            },
            'scrolling': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
            'really': {
                'mas': {
                    'Chromium': {
                        'mac': False,
                        'win7': False
                    }
                },
                'dep': False,
            },
        }, update_test_suites._CreateTestSuiteDict())

  def testFetchSuites(self):
    self._AddSampleData()
    suites = update_test_suites._FetchSuites()
    suite_keys = [s.key for s in suites]
    self.assertEqual(
        list(
            map(utils.TestKey, [
                'Chromium/mac/dromaeo',
                'Chromium/mac/really',
                'Chromium/mac/scrolling',
                'Chromium/win7/dromaeo',
                'Chromium/win7/really',
                'Chromium/win7/scrolling',
            ])), suite_keys)

  def testGetSubTestPath(self):
    key = utils.TestKey('Chromium/mac/my_suite/foo/bar')
    self.assertEqual('foo/bar', update_test_suites._GetTestSubPath(key))

  def testPartialTestSuites(self):
    testing_common.AddTests(['master'], ['bot'], {
        'TEST_PARTIAL_TEST_SUITE': {
            'COMPOSITE': {
                'measurement': {},
            },
        },
    })
    self.testapp.post('/update_test_suites')
    self.assertEqual(['TEST_PARTIAL_TEST_SUITE:COMPOSITE'],
                     update_test_suites.FetchCachedTestSuites2())


if __name__ == '__main__':
  unittest.main()
