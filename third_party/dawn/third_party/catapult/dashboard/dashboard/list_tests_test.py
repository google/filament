# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from flask import Flask
import json
import unittest
import webtest

from google.appengine.ext import ndb

from dashboard import list_tests
from dashboard.common import datastore_hooks
from dashboard.common import layered_cache
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import graph_data

flask_app = Flask(__name__)


@flask_app.route('/list_tests', methods=['POST'])
def ListTestsHandlerPost():
  return list_tests.ListTestsHandlerPost()


class ListTestsTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    self.testapp = webtest.TestApp(flask_app)
    datastore_hooks.InstallHooks()
    self.UnsetCurrentUser()
    testing_common.SetIsInternalUser('internal@chromium.org', True)
    testing_common.SetIsInternalUser('foo@chromium.org', False)

  def _AddSampleData(self):
    testing_common.AddTests(
        ['Chromium'], ['win7', 'mac'], {
            'dromaeo': {
                'dom': {},
                'jslib': {},
            },
            'scrolling': {
                'commit_time': {
                    'www.alibaba.com': {},
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

  def testPost_GetTestsForTestPath_Selected_Invalid(self):
    self._AddSampleData()
    # Requesting an invalid test path should not throw 500 error, it should
    # silently ignore the requested invalid test path.
    response = self.testapp.post(
        '/list_tests', {
            'type':
                'test_path_dict',
            'test_path_dict':
                json.dumps({
                    'Chromium/win7/scrolling/commit_time': ['commit_time_ref']
                }),
            'return_selected':
                '1'
        })
    expected = {'anyMissing': True, 'tests': []}
    self.assertEqual(expected, json.loads(response.body))

  def testGetSubTests_FetchAndCacheBehavior(self):
    self._AddSampleData()

    # Set the has_rows flag to true on two of the TestMetadata entities.
    for test_path in [
        'Chromium/win7/really/nested/very/deeply/subtest',
        'Chromium/win7/really/nested/very_very'
    ]:
      test = utils.TestKey(test_path).get()
      test.has_rows = True
      test.put()

    # A tree-structured dict of dicts is constructed, and the 'has_rows'
    # flag is set to true for two of these tests. These two tests and
    # their parents are all included in the result.
    response = self.testapp.post(
        '/list_tests', {
            'type': 'sub_tests',
            'suite': 'really',
            'bots': 'Chromium/win7,Chromium/mac'
        })
    self.assertEqual('*', response.headers.get('Access-Control-Allow-Origin'))
    expected = {
        'nested': {
            'has_rows': False,
            'sub_tests': {
                'very': {
                    'has_rows': False,
                    'sub_tests': {
                        'deeply': {
                            'has_rows': False,
                            'sub_tests': {
                                'subtest': {
                                    'has_rows': True,
                                    'sub_tests': {}
                                }
                            }
                        }
                    }
                },
                'very_very': {
                    'has_rows': True,
                    'sub_tests': {}
                }
            }
        }
    }
    # The response should be as expected.
    self.assertEqual(expected, json.loads(response.body))

    # The cache should be set for the win7 bot with the expected response.
    self.assertEqual(
        expected,
        json.loads(
            layered_cache.Get(graph_data.LIST_TESTS_SUBTEST_CACHE_KEY %
                              ('Chromium', 'win7', 'really'))))

    # Change mac subtests in cache. Should be merged with win7.
    mac_subtests = {
        'mactest': {
            'has_rows': False,
            'sub_tests': {
                'graph': {
                    'has_rows': True,
                    'sub_tests': {}
                }
            }
        }
    }
    layered_cache.Set(
        graph_data.LIST_TESTS_SUBTEST_CACHE_KEY % ('Chromium', 'mac', 'really'),
        json.dumps(mac_subtests))
    response = self.testapp.post(
        '/list_tests', {
            'type': 'sub_tests',
            'suite': 'really',
            'bots': 'Chromium/win7,Chromium/mac'
        })
    self.assertEqual('*', response.headers.get('Access-Control-Allow-Origin'))
    expected.update(mac_subtests)
    self.assertEqual(expected, json.loads(response.body))

  def testGetSubTests_ReturnsOnlyNonDeprecatedTests(self):
    # Sub-tests with the same name may be deprecated on only one bot, and not
    # deprecated on another bot; only non-deprecated tests should be returned.
    self._AddSampleData()

    # Set the deprecated flag to True for one test on one platform.
    test = utils.TestKey('Chromium/mac/dromaeo/jslib').get()
    test.deprecated = True
    test.put()

    # Set the has_rows flag to true for all of the test entities.
    for test_path in [
        'Chromium/win7/dromaeo/dom', 'Chromium/win7/dromaeo/jslib',
        'Chromium/mac/dromaeo/dom', 'Chromium/mac/dromaeo/jslib'
    ]:
      test = utils.TestKey(test_path).get()
      test.has_rows = True
      test.put()

    # When a request is made for subtests for the platform wherein that a
    # subtest is deprecated, that subtest will not be listed.
    response = self.testapp.post('/list_tests', {
        'type': 'sub_tests',
        'suite': 'dromaeo',
        'bots': 'Chromium/mac'
    })
    self.assertEqual('*', response.headers.get('Access-Control-Allow-Origin'))
    expected = {
        'dom': {
            'has_rows': True,
            'sub_tests': {}
        },
        'jslib': {
            'has_rows': True,
            'sub_tests': {},
            'deprecated': True
        }
    }
    self.assertEqual(expected, json.loads(response.body))

  def testPost_GetTestsForTestPath_Selected_SomeInternal(self):
    self._AddSampleData()

    # Set has_rows on two subtests but internal_only on only one.
    test = graph_data.TestMetadata.get_by_id(
        'Chromium/win7/scrolling/commit_time/www.cnn.com')
    test.internal_only = True
    test.has_rows = True
    test.put()

    test = graph_data.TestMetadata.get_by_id(
        'Chromium/win7/scrolling/commit_time/www.yahoo.com')
    test.has_rows = True
    test.put()

    request = {
        'type':
            'test_path_dict',
        'test_path_dict':
            json.dumps({
                'Chromium/win7/scrolling/commit_time': [
                    'www.cnn.com', 'www.yahoo.com'
                ]
            }),
        'return_selected':
            '1',
    }

    self.SetCurrentUser('internal@chromium.org')
    response = self.testapp.post('/list_tests', request)

    expected = {
        'anyMissing':
            False,
        'tests': [
            'Chromium/win7/scrolling/commit_time/www.cnn.com',
            'Chromium/win7/scrolling/commit_time/www.yahoo.com',
        ],
    }
    self.assertEqual(expected, json.loads(response.body))

    self.SetCurrentUser('foo@chromium.org')
    response = self.testapp.post('/list_tests', request)

    expected = {
        'anyMissing': True,
        'tests': ['Chromium/win7/scrolling/commit_time/www.yahoo.com',],
    }
    self.assertEqual(expected, json.loads(response.body))

  def testGetSubTests_InternalData_OnlyReturnedForAuthorizedUsers(self):
    # When the user has a an internal account, internal-only data is given.
    self.SetCurrentUser('internal@chromium.org')
    self._AddSampleData()

    # Set internal_only on a bot and top-level test.
    bot = ndb.Key('Master', 'Chromium', 'Bot', 'win7').get()
    bot.internal_only = True
    bot.put()
    test = graph_data.TestMetadata.get_by_id('Chromium/win7/dromaeo')
    test.internal_only = True
    test.put()

    # Set internal_only and has_rows to true on two subtests.
    for name in ['dom', 'jslib']:
      subtest = graph_data.TestMetadata.get_by_id('Chromium/win7/dromaeo/%s' %
                                                  name)
      subtest.internal_only = True
      subtest.has_rows = True
      subtest.put()

    # All of the internal-only tests are returned.
    response = self.testapp.post('/list_tests', {
        'type': 'sub_tests',
        'suite': 'dromaeo',
        'bots': 'Chromium/win7'
    })
    expected = {
        'dom': {
            'has_rows': True,
            'sub_tests': {}
        },
        'jslib': {
            'has_rows': True,
            'sub_tests': {}
        }
    }
    self.assertEqual(expected, json.loads(response.body))

    # After setting the user to another domain, an empty dict is returned.
    self.SetCurrentUser('foo@chromium.org')
    response = self.testapp.post('/list_tests', {
        'type': 'sub_tests',
        'suite': 'dromaeo',
        'bots': 'Chromium/win7'
    })
    self.assertEqual({}, json.loads(response.body))

  def testMergeSubTestsDict(self):
    a = {
        'foo': {
            'has_rows': True,
            'sub_tests': {
                'a': {
                    'has_rows': True,
                    'sub_tests': {}
                }
            },
        },
        'bar': {
            'has_rows': False,
            'sub_tests': {
                'b': {
                    'has_rows': False,
                    'sub_tests': {
                        'c': {
                            'has_rows': False,
                            'deprecated': True,
                            'sub_tests': {},
                        },
                    },
                },
            },
        },
    }
    b = {
        'bar': {
            'has_rows': True,
            'sub_tests': {
                'b': {
                    'has_rows': False,
                    'sub_tests': {
                        'c': {
                            'has_rows': False,
                            'sub_tests': {},
                        },
                    },
                },
            },
        },
        'baz': {
            'has_rows': False,
            'sub_tests': {}
        },
    }
    self.assertEqual(
        {
            'foo': {
                'has_rows': True,
                'sub_tests': {
                    'a': {
                        'has_rows': True,
                        'sub_tests': {}
                    }
                },
            },
            'bar': {
                'has_rows': True,
                'sub_tests': {
                    'b': {
                        'has_rows': False,
                        'sub_tests': {
                            'c': {
                                'has_rows': False,
                                'sub_tests': {}
                            },
                        },
                    },
                },
            },
            'baz': {
                'has_rows': False,
                'sub_tests': {}
            },
        }, list_tests._MergeSubTestsDict(a, b))

  def testSubTestsDict(self):
    paths = [
        ['a', 'b', 'c'],
        ['a', 'b', 'c'],
        ['a', 'b', 'd'],
    ]
    expected = {
        'a': {
            'has_rows': False,
            'sub_tests': {
                'b': {
                    'has_rows': False,
                    'sub_tests': {
                        'c': {
                            'has_rows': True,
                            'sub_tests': {}
                        },
                        'd': {
                            'has_rows': True,
                            'sub_tests': {}
                        },
                    },
                },
            },
        },
    }
    self.assertEqual(expected, list_tests._SubTestsDict(paths, False))

  def testSubTestsDict_Deprecated(self):
    paths = [
        ['a'],
        ['a', 'b'],
    ]
    expected = {
        'a': {
            'has_rows': True,
            'deprecated': True,
            'sub_tests': {
                'b': {
                    'has_rows': True,
                    'deprecated': True,
                    'sub_tests': {},
                },
            },
        },
    }
    self.assertEqual(expected, list_tests._SubTestsDict(paths, True))

  def testSubTestsDict_TopLevel_HasRows_False(self):
    paths = [
        ['a', 'b'],
        ['a', 'c'],
    ]
    expected = {
        'a': {
            'has_rows': False,
            'sub_tests': {
                'b': {
                    'has_rows': True,
                    'sub_tests': {}
                },
                'c': {
                    'has_rows': True,
                    'sub_tests': {}
                },
            },
        },
    }
    self.assertEqual(expected, list_tests._SubTestsDict(paths, False))

  def testSubTestsDict_RepeatedPathIgnored(self):
    paths = [
        ['a', 'b'],
        ['a', 'c'],
        ['a', 'b'],
    ]
    expected = {
        'a': {
            'has_rows': False,
            'sub_tests': {
                'b': {
                    'has_rows': True,
                    'sub_tests': {}
                },
                'c': {
                    'has_rows': True,
                    'sub_tests': {}
                },
            },
        },
    }
    self.assertEqual(expected, list_tests._SubTestsDict(paths, False))

  def testPost_GetTestsMatchingPattern(self):
    """Tests the basic functionality of the GetTestsMatchingPattern function."""
    self._AddSampleData()

    # A pattern can match tests with a particular bot and with a particular
    # number of levels of nesting.
    # The results are lexicographically ordered by test path.
    response = self.testapp.post('/list_tests', {
        'type': 'pattern',
        'p': 'Chromium/mac/*/*/www*'
    })
    expected = [
        'Chromium/mac/scrolling/commit_time/www.alibaba.com',
        'Chromium/mac/scrolling/commit_time/www.cnn.com',
        'Chromium/mac/scrolling/commit_time/www.yahoo.com',
    ]
    self.assertEqual(expected, json.loads(response.body))

    # The same thing is returned if has_rows is set to '0' or another string
    # that is not '1'.
    response = self.testapp.post('/list_tests', {
        'type': 'pattern',
        'has_rows': '0',
        'p': '*/mac/*/*/www*'
    })
    self.assertEqual(expected, json.loads(response.body))
    response = self.testapp.post('/list_tests', {
        'type': 'pattern',
        'has_rows': 'foo',
        'p': '*/mac/*/*/www*'
    })
    self.assertEqual(expected, json.loads(response.body))

  def testPost_GetTestsMatchingPattern_OnlyWithRows(self):
    """Tests GetTestsMatchingPattern with the parameter only_with_rows set."""
    self._AddSampleData()

    # When no TestMetadata entities have has_rows set, filtering with the
    # parameter 'has_rows' set to '1' results in no rows being returned.
    response = self.testapp.post('/list_tests', {
        'type': 'pattern',
        'has_rows': '1',
        'p': '*/mac/dromaeo/*'
    })
    self.assertEqual([], json.loads(response.body))

    # Set the has_rows flag on one of the tests.
    test = utils.TestKey('Chromium/mac/dromaeo/dom').get()
    test.has_rows = True
    test.put()

    # Even though multiple tests could match the pattern, only the test with
    # has_rows set is returned.
    response = self.testapp.post('/list_tests', {
        'type': 'pattern',
        'has_rows': '1',
        'p': '*/mac/dromaeo/*'
    })
    self.assertEqual(['Chromium/mac/dromaeo/dom'], json.loads(response.body))

  def testPost_GetTestsForTestPath_Selected_Core_MonitoredChildWithRows(self):
    yahoo_path = 'Chromium/win7/scrolling/commit_time/www.yahoo.com'

    self._AddSampleData()

    yahoo = graph_data.TestMetadata.get_by_id(yahoo_path)
    yahoo.has_rows = True
    yahoo.put()

    response = self.testapp.post(
        '/list_tests', {
            'type':
                'test_path_dict',
            'test_path_dict':
                json.dumps({'Chromium/win7/scrolling/commit_time': 'core'}),
            'return_selected':
                '1'
        })

    expected = {
        'anyMissing': False,
        'tests': ['Chromium/win7/scrolling/commit_time/www.yahoo.com'],
    }
    self.assertEqual(expected, json.loads(response.body))

  def testPost_GetTestsForTestPath_Selected_Core_MonitoredChildNoRows(self):
    self._AddSampleData()

    response = self.testapp.post(
        '/list_tests', {
            'type':
                'test_path_dict',
            'test_path_dict':
                json.dumps({'Chromium/win7/scrolling/commit_time': 'core'}),
            'return_selected':
                '1'
        })

    expected = {'tests': [], 'anyMissing': False}
    self.assertEqual(expected, json.loads(response.body))

  def testPost_GetTestsForTestPath_Selected_Core_ParentHasRows(self):
    self._AddSampleData()

    core = graph_data.TestMetadata.get_by_id(
        'Chromium/win7/scrolling/commit_time')
    core.has_rows = True
    core.put()

    response = self.testapp.post(
        '/list_tests', {
            'type':
                'test_path_dict',
            'test_path_dict':
                json.dumps({'Chromium/win7/scrolling/commit_time': 'core'}),
            'return_selected':
                '1'
        })

    expected = {
        'anyMissing': False,
        'tests': ['Chromium/win7/scrolling/commit_time'],
    }
    self.assertEqual(expected, json.loads(response.body))

  def testPost_GetTestsForTestPath_Selected_Core_AllHaveRows(self):
    yahoo_path = 'Chromium/win7/scrolling/commit_time/www.yahoo.com'

    self._AddSampleData()

    core = graph_data.TestMetadata.get_by_id(
        'Chromium/win7/scrolling/commit_time')
    core.has_rows = True
    core.put()

    yahoo = graph_data.TestMetadata.get_by_id(yahoo_path)
    yahoo.has_rows = True
    yahoo.put()

    response = self.testapp.post(
        '/list_tests', {
            'type':
                'test_path_dict',
            'test_path_dict':
                json.dumps({'Chromium/win7/scrolling/commit_time': 'core'}),
            'return_selected':
                '1'
        })

    expected = {
        'anyMissing':
            False,
        'tests': [
            'Chromium/win7/scrolling/commit_time',
            'Chromium/win7/scrolling/commit_time/www.yahoo.com',
        ],
    }
    self.assertEqual(expected, json.loads(response.body))

  def testPost_GetTestsForTestPath_Selected_Core_NoRows(self):
    self._AddSampleData()

    response = self.testapp.post(
        '/list_tests', {
            'type':
                'test_path_dict',
            'test_path_dict':
                json.dumps({'Chromium/win7/scrolling/commit_time': 'core'}),
            'return_selected':
                '1'
        })

    expected = {
        'anyMissing': False,
        'tests': [],
    }
    self.assertEqual(expected, json.loads(response.body))

  def testPost_GetTestsForTestPath_Selected_EmptyPreselected(self):
    self._AddSampleData()

    response = self.testapp.post(
        '/list_tests', {
            'type':
                'test_path_dict',
            'test_path_dict':
                json.dumps({'Chromium/win7/scrolling/commit_time': []}),
            'return_selected':
                '1'
        })

    expected = {'anyMissing': False, 'tests': []}
    self.assertEqual(expected, json.loads(response.body))

  def testPost_GetTestsForTestPath_Selected_Preselected(self):
    self._AddSampleData()

    test = utils.TestKey(
        'Chromium/win7/scrolling/commit_time/www.yahoo.com').get()
    test.has_rows = True
    test.put()

    response = self.testapp.post(
        '/list_tests', {
            'type':
                'test_path_dict',
            'test_path_dict':
                json.dumps({
                    'Chromium/win7/scrolling/commit_time':
                        ['commit_time', 'www.yahoo.com']
                }),
            'return_selected':
                '1'
        })

    expected = {
        'anyMissing': True,
        'tests': ['Chromium/win7/scrolling/commit_time/www.yahoo.com'],
    }
    self.assertEqual(expected, json.loads(response.body))

  def testPost_GetTestsForTestPath_Selected_Preselected_Multiple(self):
    self._AddSampleData()

    subtest = graph_data.TestMetadata.get_by_id(
        'Chromium/win7/scrolling/commit_time/www.cnn.com')
    subtest.has_rows = True
    subtest.put()
    subtest = graph_data.TestMetadata.get_by_id(
        'Chromium/mac/scrolling/commit_time/www.cnn.com')
    subtest.has_rows = True
    subtest.put()

    response = self.testapp.post(
        '/list_tests', {
            'type':
                'test_path_dict',
            'test_path_dict':
                json.dumps({
                    'Chromium/win7/scrolling/commit_time': ['www.cnn.com'],
                    'Chromium/mac/scrolling/commit_time': ['www.cnn.com']
                }),
            'return_selected':
                '1'
        })

    expected = {
        'anyMissing':
            False,
        'tests': [
            'Chromium/win7/scrolling/commit_time/www.cnn.com',
            'Chromium/mac/scrolling/commit_time/www.cnn.com'
        ],
    }
    self.assertEqual(expected, json.loads(response.body))

  def testPost_GetTestsForTestPath_Selected_All(self):
    self._AddSampleData()

    subtest = graph_data.TestMetadata.get_by_id(
        'Chromium/win7/scrolling/commit_time/www.cnn.com')
    subtest.has_rows = True
    subtest.put()

    response = self.testapp.post(
        '/list_tests', {
            'type':
                'test_path_dict',
            'test_path_dict':
                json.dumps({'Chromium/win7/scrolling/commit_time': 'all'}),
            'return_selected':
                '1'
        })

    expected = {
        'anyMissing': True,
        'tests': ['Chromium/win7/scrolling/commit_time/www.cnn.com'],
    }
    self.assertEqual(expected, json.loads(response.body))

  def testPost_GetTestsForTestPath_Unselected_Core_NoParent(self):
    self._AddSampleData()

    response = self.testapp.post(
        '/list_tests', {
            'type':
                'test_path_dict',
            'test_path_dict':
                json.dumps({'Chromium/win7/scrolling/commit_time': 'core'}),
            'return_selected':
                '0'
        })

    expected = {'tests': []}
    self.assertEqual(expected, json.loads(response.body))

  def testPost_GetTestsForTestPath_Unselected_Core_Unmonitored(self):
    yahoo_path = 'Chromium/win7/scrolling/commit_time/www.yahoo.com'

    self._AddSampleData()

    cnn = graph_data.TestMetadata.get_by_id(
        'Chromium/win7/scrolling/commit_time/www.cnn.com')
    cnn.has_rows = True
    cnn.put()

    yahoo = graph_data.TestMetadata.get_by_id(yahoo_path)
    yahoo.has_rows = True
    yahoo.put()

    response = self.testapp.post(
        '/list_tests', {
            'type':
                'test_path_dict',
            'test_path_dict':
                json.dumps({'Chromium/win7/scrolling/commit_time': 'core'}),
            'return_selected':
                '0'
        })

    expected = {
        'tests': [
            'Chromium/win7/scrolling/commit_time/www.cnn.com',
            'Chromium/win7/scrolling/commit_time/www.yahoo.com'
        ],
    }
    self.assertEqual(expected, json.loads(response.body))

  def testPost_GetTestsForTestPath_Unselected_EmptyPreselected(self):
    self._AddSampleData()

    response = self.testapp.post(
        '/list_tests', {
            'type':
                'test_path_dict',
            'test_path_dict':
                json.dumps({'Chromium/win7/scrolling/commit_time': []}),
            'return_selected':
                '0'
        })

    expected = {
        'tests': ['Chromium/win7/scrolling/commit_time'],
    }
    self.assertEqual(expected, json.loads(response.body))

  def testPost_GetTestsForTestPath_Unselected_PreselectedWithRows(self):
    self._AddSampleData()

    subtest = graph_data.TestMetadata.get_by_id(
        'Chromium/win7/scrolling/commit_time/www.cnn.com')
    subtest.has_rows = True
    subtest.put()

    response = self.testapp.post(
        '/list_tests', {
            'type':
                'test_path_dict',
            'test_path_dict':
                json.dumps({
                    'Chromium/win7/scrolling/commit_time':
                        ['commit_time', 'www.yahoo.com']
                }),
            'return_selected':
                '0'
        })

    expected = {
        'tests': ['Chromium/win7/scrolling/commit_time/www.cnn.com'],
    }
    self.assertEqual(expected, json.loads(response.body))

  def testPost_GetTestsForTestPath_Unselected_PreselectedWithoutRows(self):
    self._AddSampleData()

    response = self.testapp.post(
        '/list_tests', {
            'type':
                'test_path_dict',
            'test_path_dict':
                json.dumps({
                    'Chromium/win7/scrolling/commit_time':
                        ['commit_time', 'www.yahoo.com']
                }),
            'return_selected':
                '0'
        })

    expected = {'tests': []}
    self.assertEqual(expected, json.loads(response.body))

  def testPost_GetTestsForTestPath_Unselected_All(self):
    self._AddSampleData()

    response = self.testapp.post(
        '/list_tests', {
            'type':
                'test_path_dict',
            'test_path_dict':
                json.dumps({'Chromium/win7/scrolling/commit_time': 'all'}),
            'return_selected':
                '0'
        })

    self.assertEqual({'tests': []}, json.loads(response.body))

  def testGetDescendants(self):
    self._AddSampleData()
    self.assertEqual(
        list_tests.GetTestDescendants(
            ndb.Key('TestMetadata', 'Chromium/mac/dromaeo')), [
                ndb.Key('TestMetadata', 'Chromium/mac/dromaeo'),
                ndb.Key('TestMetadata', 'Chromium/mac/dromaeo/dom'),
                ndb.Key('TestMetadata', 'Chromium/mac/dromaeo/jslib'),
            ])
    self.assertEqual(
        list_tests.GetTestDescendants(
            ndb.Key('TestMetadata', 'Chromium/win7/really/nested')),
        [
            ndb.Key('TestMetadata', 'Chromium/win7/really/nested'),
            ndb.Key('TestMetadata', 'Chromium/win7/really/nested/very'),
            ndb.Key('TestMetadata', 'Chromium/win7/really/nested/very/deeply'),
            ndb.Key('TestMetadata',
                    'Chromium/win7/really/nested/very/deeply/subtest'),
            ndb.Key('TestMetadata', 'Chromium/win7/really/nested/very_very'),
        ])


if __name__ == '__main__':
  unittest.main()
