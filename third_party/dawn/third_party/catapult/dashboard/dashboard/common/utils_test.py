# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import base64
import json
import unittest

from unittest import mock

from google.appengine.ext import ndb

from dashboard import sheriff_config_client
from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import graph_data


class UtilsTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    testing_common.SetIsInternalUser('internal@chromium.org', True)
    testing_common.SetIsInternalUser('foo@chromium.org', False)
    testing_common.SetIsAdministrator('admin@chromium.org', True)

  def _AssertMatches(self, test_path, pattern):
    """Asserts that a test path matches a pattern with MatchesPattern."""
    test_key = utils.TestKey(test_path)
    self.assertTrue(utils.TestMatchesPattern(test_key, pattern))

  def _AssertDoesntMatch(self, test_path, pattern):
    """Asserts that a test path doesn't match a pattern with MatchesPattern."""
    test_key = utils.TestKey(test_path)
    self.assertFalse(utils.TestMatchesPattern(test_key, pattern))

  def testMatchesPattern_AllWildcards(self):
    self._AssertMatches('ChromiumPerf/cros-one/dromaeo.top25/Total', '*/*/*/*')
    self._AssertDoesntMatch('ChromiumPerf/cros-one/dromaeo.top25/Total',
                            '*/*/*')

  def testMatchesPattern_SomeWildcards(self):
    self._AssertMatches('ChromiumPerf/cros-one/dromaeo.top25/Total',
                        'ChromiumPerf/*/dromaeo.top25/*')
    self._AssertDoesntMatch('ChromiumPerf/cros-one/dromaeo.top25/Total',
                            'ChromiumPerf/*/dromaeo.another_page_set/*')

  def testMatchesPattern_SomePartialWildcards(self):
    self._AssertMatches('ChromiumPerf/cros-one/dromaeo.top25/Total',
                        'ChromiumPerf/cros-*/dromaeo.*/Total')
    self._AssertDoesntMatch('ChromiumPerf/cros-one/dromaeoXtop25/Total',
                            'ChromiumPerf/cros-*/dromaeo.*/Total')
    self._AssertDoesntMatch('ChromiumPerf/cros-one/dromaeo.top25/Total',
                            'OtherMaster/cros-*/dromaeo.*/Total')

  def testMatchesPattern_MorePartialWildcards(self):
    # Note that the wildcard matches zero or more characters.
    self._AssertMatches('ChromiumPerf/cros-one/dromaeo.top25/Total',
                        'Chromium*/cros-one*/*.*/To*al')
    self._AssertDoesntMatch('ChromiumPerf/cros-one/dromaeo.top25/Total',
                            'Chromium*/linux-*/*.*/To*al')

  def testMatchesPattern_RequiresFullMatchAtEnd(self):
    # If there is no wildcard at the beginning or end of the
    # test path part, then a part will only match if it matches
    # right up to the beginning or end.
    self._AssertDoesntMatch('ChromiumPerf/cros-one/dromaeo.top25/Total',
                            'ChromiumPerf/cros-one/dromaeo.top25/*Tot')
    self._AssertDoesntMatch('ChromiumPerf/cros-one/dromaeo.top25/Total',
                            'ChromiumPerf/cros-one/dromaeo.top25/otal*')

  def testMostSpecificMatchingPattern_SpecificVsGeneral(self):
    test_key = utils.TestKey('M/B/S/Total')

    result = utils.MostSpecificMatchingPattern(test_key, [('*/*/*/*', 1),
                                                          ('*/*/*/Total', 2),
                                                          ('*/*/*/Foo', 3)])
    self.assertEqual(2, result)

  def testMostSpecificMatchingPattern_PartialVsGeneral(self):
    test_key = utils.TestKey('M/B/S/Total')

    result = utils.MostSpecificMatchingPattern(test_key, [('*/*/*/*', 1),
                                                          ('*/*/*/To*al', 2),
                                                          ('*/*/*/Foo', 3)])
    self.assertEqual(2, result)

  def testMostSpecificMatchingPattern_2ndLevel(self):
    test_key = utils.TestKey('M/B/S/Total')

    result = utils.MostSpecificMatchingPattern(test_key, [('*/*/*/*', 1),
                                                          ('*/*/S/*', 2),
                                                          ('*/*/*/Foo', 3)])
    self.assertEqual(2, result)

  def testMostSpecificMatchingPattern_TopLevelSpecificOverLowerSpecific(self):
    test_key = utils.TestKey('M/B/S/Total')

    result = utils.MostSpecificMatchingPattern(test_key, [('*/*/S/*', 1),
                                                          ('*/*/*/Total', 2),
                                                          ('*/*/*/Foo', 3)])
    self.assertEqual(2, result)

  def testMostSpecificMatchingPattern_TopLevelPartialOverLowerSpecific(self):
    test_key = utils.TestKey('M/B/S/Total')

    result = utils.MostSpecificMatchingPattern(test_key, [('*/*/S/*', 1),
                                                          ('*/*/*/To*al', 2),
                                                          ('*/*/*/Foo', 3)])
    self.assertEqual(2, result)

  def testMostSpecificMatchingPattern_Duplicate(self):
    test_key = utils.TestKey('Does/Not/Match/Something')

    result = utils.MostSpecificMatchingPattern(test_key,
                                               [('Does/Not/Match/*', 1),
                                                ('Does/Not/Match/*', 2)])
    self.assertEqual(1, result)

  def testParseTelemetryMetricParts_TooShort(self):
    with self.assertRaises(utils.ParseTelemetryMetricFailed):
      utils.ParseTelemetryMetricParts('M/B/S')

  def testParseTelemetryMetricParts_TooLong(self):
    with self.assertRaises(utils.ParseTelemetryMetricFailed):
      utils.ParseTelemetryMetricParts('M/B/S/1/2/3/4')

  def testParseTelemetryMetricParts_1Part(self):
    self.assertEqual(('', 'Measurement', ''),
                     utils.ParseTelemetryMetricParts('M/B/Suite/Measurement'))

  def testParseTelemetryMetricParts_2Part(self):
    self.assertEqual(
        ('', 'Measurement', 'Story'),
        utils.ParseTelemetryMetricParts('M/B/Suite/Measurement/Story'))

  def testParseTelemetryMetricParts_3Part(self):
    self.assertEqual(
        ('TIRLabel', 'Measurement', 'Story'),
        utils.ParseTelemetryMetricParts('M/B/Suite/Measurement/TIRLabel/Story'))

  def _PutEntitiesAllExternal(self):
    """Puts entities (none internal-only) and returns the keys."""
    master = graph_data.Master(id='M').put()
    graph_data.Bot(parent=master, id='b').put()
    keys = [
        graph_data.TestMetadata(id='M/b/a', internal_only=False),
        graph_data.TestMetadata(id='M/b/b', internal_only=False),
        graph_data.TestMetadata(id='M/b/c', internal_only=False),
        graph_data.TestMetadata(id='M/b/d', internal_only=False),
    ]
    for t in keys:
      t.UpdateSheriff()

    keys = [k.put() for k in keys]

    return keys

  def _PutEntitiesHalfInternal(self):
    """Puts entities (half internal-only) and returns the keys."""
    master = graph_data.Master(id='M').put()
    graph_data.Bot(parent=master, id='b').put()
    keys = [
        graph_data.TestMetadata(id='M/b/ax', internal_only=True),
        graph_data.TestMetadata(id='M/b/a', internal_only=False),
        graph_data.TestMetadata(id='M/b/b', internal_only=False),
        graph_data.TestMetadata(id='M/b/bx', internal_only=True),
        graph_data.TestMetadata(id='M/b/c', internal_only=False),
        graph_data.TestMetadata(id='M/b/cx', internal_only=True),
        graph_data.TestMetadata(id='M/b/d', internal_only=False),
        graph_data.TestMetadata(id='M/b/dx', internal_only=True),
    ]

    for t in keys:
      t.UpdateSheriff()

    keys = [k.put() for k in keys]

    return keys

  def testGetMulti_ExternalUser_ReturnsSomeEntities(self):
    keys = self._PutEntitiesHalfInternal()
    self.SetCurrentUser('foo@chromium.org')
    self.assertEqual(len(keys) / 2, len(utils.GetMulti(keys)))

  def testGetMulti_InternalUser_ReturnsAllEntities(self):
    keys = self._PutEntitiesHalfInternal()
    self.SetCurrentUser('internal@chromium.org')
    self.assertEqual(len(keys), len(utils.GetMulti(keys)))

  def testGetMulti_AllExternalEntities_ReturnsAllEntities(self):
    keys = self._PutEntitiesAllExternal()
    self.SetCurrentUser('internal@chromium.org')
    self.assertEqual(len(keys), len(utils.GetMulti(keys)))

  def testTestPath_Test(self):
    key = ndb.Key('Master', 'm', 'Bot', 'b', 'Test', 'suite', 'Test', 'metric')
    self.assertEqual('m/b/suite/metric', utils.TestPath(key))

  def testTestPath_TestMetadata(self):
    key = ndb.Key('TestMetadata', 'm/b/suite/metric')
    self.assertEqual('m/b/suite/metric', utils.TestPath(key))

  def testTestPath_Container(self):
    key = ndb.Key('TestContainer', 'm/b/suite/metric')
    self.assertEqual('m/b/suite/metric', utils.TestPath(key))

  def testTestMetadataKey_None(self):
    key = utils.TestMetadataKey(None)
    self.assertIsNone(key)

  def testTestMetadataKey_Test(self):
    key = utils.TestMetadataKey(
        ndb.Key('Master', 'm', 'Bot', 'b', 'Test', 'suite', 'Test', 'metric'))
    self.assertEqual('TestMetadata', key.kind())
    self.assertEqual('m/b/suite/metric', key.id())
    self.assertEqual(('TestMetadata', 'm/b/suite/metric'), key.flat())

  def testTestMetadataKey_TestMetadata(self):
    original_key = ndb.Key('TestMetadata', 'm/b/suite/metric')
    key = utils.TestMetadataKey(original_key)
    self.assertEqual(original_key, key)

  def testTestMetadataKey_String(self):
    key = utils.TestMetadataKey('m/b/suite/metric/page')
    self.assertEqual('TestMetadata', key.kind())
    self.assertEqual('m/b/suite/metric/page', key.id())
    self.assertEqual(('TestMetadata', 'm/b/suite/metric/page'), key.flat())

  def testOldStyleTestKey_None(self):
    key = utils.OldStyleTestKey(None)
    self.assertIsNone(key)

  def testOldStyleTestKey_Test(self):
    original_key = ndb.Key('Master', 'm', 'Bot', 'b', 'Test', 'suite', 'Test',
                           'metric')
    key = utils.OldStyleTestKey(original_key)
    self.assertEqual(original_key, key)

  def testOldStyleTestKey_TestMetadata(self):
    key = utils.OldStyleTestKey(ndb.Key('TestMetadata', 'm/b/suite/metric'))
    self.assertEqual('Test', key.kind())
    self.assertEqual('metric', key.id())
    self.assertEqual(
        ('Master', 'm', 'Bot', 'b', 'Test', 'suite', 'Test', 'metric'),
        key.flat())

  def testOldStyleTestKey_String(self):
    key = utils.OldStyleTestKey('m/b/suite/metric')
    self.assertEqual('Test', key.kind())
    self.assertEqual('metric', key.id())
    self.assertEqual(
        ('Master', 'm', 'Bot', 'b', 'Test', 'suite', 'Test', 'metric'),
        key.flat())

  def testTestSuiteName_Basic(self):
    key = utils.TestKey('Master/bot/suite-foo/sub/x/y/z')
    self.assertEqual('suite-foo', utils.TestSuiteName(key))

  def testMinimumRange_Empty_ReturnsNone(self):
    self.assertIsNone(utils.MinimumRange([]))

  def testMinimumRange_NotOverlapping_ReturnsNone(self):
    self.assertIsNone(utils.MinimumRange([(5, 10), (15, 20)]))

  def testMinimumRange_OneRange_ReturnsSameRange(self):
    self.assertEqual((5, 10), utils.MinimumRange([(5, 10)]))

  def testMinimumRange_OverlapsForOneNumber_ReturnsRangeWithOneNumber(self):
    self.assertEqual((5, 5), utils.MinimumRange([(2, 5), (5, 10)]))

  def testMinimumRange_MoreThanTwoRanges_ReturnsIntersection(self):
    self.assertEqual((6, 14),
                     utils.MinimumRange([(3, 20), (5, 15), (6, 25), (3, 14)]))

  def testValidate_StringNotInOptionList_Fails(self):
    with self.assertRaises(ValueError):
      utils.Validate(['completed', 'pending', 'failed'], 'running')

  def testValidate_InvalidType_Fails(self):
    with self.assertRaises(ValueError):
      utils.Validate(int, 'a string')

  def testValidate_MissingProperty_Fails(self):
    with self.assertRaises(ValueError):
      utils.Validate(
          {
              'status': str,
              'try_job_id': int,
              'required_property': int
          }, {
              'status': 'completed',
              'try_job_id': 1234
          })

  def testValidate_InvalidTypeInDict_Fails(self):
    with self.assertRaises(ValueError):
      utils.Validate({
          'status': int,
          'try_job_id': int
      }, {
          'status': 'completed',
          'try_job_id': 1234
      })

  def testValidate_StringNotInNestedOptionList_Fails(self):
    with self.assertRaises(ValueError):
      utils.Validate({'values': {
          'nested_values': ['orange', 'banana']
      }}, {'values': {
          'nested_values': 'apple'
      }})

  def testValidate_MissingPropertyInNestedDict_Fails(self):
    with self.assertRaises(ValueError):
      utils.Validate({'values': {
          'nested_values': ['orange', 'banana']
      }}, {'values': {}})

  def testValidate_ExpectedValueIsNone_Passes(self):
    utils.Validate(None, 'running')

  def testValidate_StringInOptionList_Passes(self):
    utils.Validate(str, 'a string')

  def testValidate_HasExpectedProperties_Passes(self):
    utils.Validate({
        'status': str,
        'try_job_id': int
    }, {
        'status': 'completed',
        'try_job_id': 1234
    })

  def testValidate_StringInNestedOptionList_Passes(self):
    utils.Validate({'values': {
        'nested_values': ['orange', 'banana']
    }}, {'values': {
        'nested_values': 'orange'
    }})

  def testValidate_TypeConversion_Passes(self):
    utils.Validate([1], '1')

  def testGetBuildDetailsFromStdioLink_InvalidLink(self):
    base_url, master, bot, number, step = utils.GetBuildDetailsFromStdioLink(
        '[Buildbot stdio](http://notquite/builders/whatever/234)')
    self.assertIsNone(base_url)
    self.assertIsNone(master)
    self.assertIsNone(bot)
    self.assertIsNone(number)
    self.assertIsNone(step)

  def testGetBuildDetailsFromStdioLink(self):
    base_url, master, bot, number, step = utils.GetBuildDetailsFromStdioLink(
        ('[Buildbot stdio](https://build.chromium.org/p/chromium.perf/builders/'
         'Android%20One%20Perf%20%282%29/builds/5365/steps/'
         'blink_style.top_25/logs/stdio)'))
    self.assertEqual('https://build.chromium.org/p/chromium.perf/builders/',
                     base_url)
    self.assertEqual('chromium.perf', master)
    self.assertEqual('Android One Perf (2)', bot)
    self.assertEqual('5365', number)
    self.assertEqual('blink_style.top_25', step)

  def testGetBuildDetailsFromStdioLink_DifferentBaseUrl(self):
    base_url, master, bot, number, step = utils.GetBuildDetailsFromStdioLink(
        ('[Buildbot stdio]('
         'https://uberchromegw.corp.google.com/i/new.master/builders/Builder/'
         'builds/3486/steps/new_test/logs/stdio)'))
    self.assertEqual(
        'https://uberchromegw.corp.google.com/i/new.master/builders/', base_url)
    self.assertEqual('new.master', master)
    self.assertEqual('Builder', bot)
    self.assertEqual('3486', number)
    self.assertEqual('new_test', step)

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch('apiclient.discovery.build')
  def testIsGroupMember_PositiveCase(self, mock_discovery_build):
    mock_request = mock.MagicMock()
    mock_request.execute = mock.MagicMock(return_value={'is_member': True})
    mock_service = mock.MagicMock()
    mock_service.membership = mock.MagicMock(return_value=mock_request)
    mock_discovery_build.return_value = mock_service
    self.assertTrue(utils.IsGroupMember('foo@bar.com', 'group'))
    mock_service.membership.assert_called_once_with(
        identity='foo@bar.com', group='group')

  @mock.patch.object(utils, 'ServiceAccountHttp', mock.MagicMock())
  @mock.patch('logging.error')
  @mock.patch('apiclient.discovery.build')
  def testIsGroupMember_RequestFails_LogsErrorAndReturnsFalse(
      self, mock_discovery_build, mock_logging_error):
    mock_service = mock.MagicMock()
    mock_service.membership = mock.MagicMock(
        return_value={'error': 'Some error'})
    mock_discovery_build.return_value = mock_service
    with self.assertRaises(utils.GroupMemberAuthFailed):
      utils.IsGroupMember('foo@bar.com', 'group')
    self.assertEqual(1, mock_logging_error.call_count)

  def testGetSheriffForAutorollCommit_NotAutoroll_ReturnsNone(self):
    self.assertIsNone(
        utils.GetSheriffForAutorollCommit('user@foo.org',
                                          'TBR=donotreturnme@foo.org'))
    self.assertIsNone(
        utils.GetSheriffForAutorollCommit('not-a-roll@foo.org',
                                          'TBR=donotreturnme@foo.org'))

  def testGetSheriffForAutorollCommit_AutoRoll_ReturnsSheriff(self):
    self.assertEqual(
        'sheriff@foo.org',
        utils.GetSheriffForAutorollCommit(
            'chromium-autoroll@skia-public.iam.gserviceaccount.com',
            'This is a roll.\n\nTBR=sheriff@foo.org,bar@foo.org\n\n'))
    self.assertEqual(
        'sheriff@v8.com',
        utils.GetSheriffForAutorollCommit(
            'v8-ci-autoroll-builder@'
            'chops-service-accounts.iam.gserviceaccount.com',
            'TBR=sheriff@v8.com'))
    # Some alternative spellings for TBR.
    self.assertEqual(
        'sheriff@v8.com',
        utils.GetSheriffForAutorollCommit(
            'v8-ci-autoroll-builder@'
            'chops-service-accounts.iam.gserviceaccount.com',
            'TBR: sheriff@v8.com'))
    self.assertEqual(
        'sheriff@v8.com',
        utils.GetSheriffForAutorollCommit(
            'v8-ci-autoroll-builder@'
            'chops-service-accounts.iam.gserviceaccount.com',
            'Tbr: sheriff@v8.com'))
    self.assertEqual(
        'sheriff@v8.com',
        utils.GetSheriffForAutorollCommit(
            'v8-ci-autoroll-builder@'
            'chops-service-accounts.iam.gserviceaccount.com',
            'TBR= sheriff@v8.com'))

  @mock.patch.object(utils, 'GetEmail',
                     mock.MagicMock(return_value='admin@chromium.org'))
  def testIsAdministrator(self):
    self.assertTrue(utils.IsAdministrator())

  @mock.patch.object(utils, 'GetEmail',
                     mock.MagicMock(return_value='internal@chromium.org'))
  def testIsNotAdministrator(self):
    self.assertFalse(utils.IsAdministrator())

  @mock.patch.object(utils, 'IsGroupMember', mock.MagicMock(return_value=False))
  @mock.patch.object(utils, 'GetEmail',
                     mock.MagicMock(return_value='internal@chromium.org'))
  def testShouldTurnOnUploadCompletionTokenExperiment_NotGroupMember(self):
    self.assertFalse(utils.ShouldTurnOnUploadCompletionTokenExperiment())

  @mock.patch.object(utils, 'IsGroupMember', mock.MagicMock(return_value=True))
  @mock.patch.object(utils, 'GetEmail',
                     mock.MagicMock(return_value='internal@chromium.org'))
  def testShouldTurnOnUploadCompletionTokenExperiment_Positive(self):
    self.assertTrue(utils.ShouldTurnOnUploadCompletionTokenExperiment())

  @mock.patch.object(sheriff_config_client.SheriffConfigClient, '__init__',
                     mock.MagicMock(return_value=None))
  @mock.patch.object(sheriff_config_client.SheriffConfigClient, 'Match',
                     mock.MagicMock(return_value=(['someone'], None)))
  def testIsMonitored_Positive(self):
    sheriff_client = sheriff_config_client.GetSheriffConfigClient()
    self.assertTrue(utils.IsMonitored(sheriff_client, 'test'))

  @mock.patch.object(sheriff_config_client.SheriffConfigClient, '__init__',
                     mock.MagicMock(return_value=None))
  @mock.patch.object(sheriff_config_client.SheriffConfigClient, 'Match',
                     mock.MagicMock(return_value=([], None)))
  def testIsMonitored_Negative(self):
    sheriff_client = sheriff_config_client.GetSheriffConfigClient()
    self.assertFalse(utils.IsMonitored(sheriff_client, 'test'))


  def testConvertBytesBeforeJsonDumps(self):
    none_obj = None
    self.assertEqual(None, utils.ConvertBytesBeforeJsonDumps(none_obj),
                     'Fail converting none object')

    dict_obj = {'a': b'aa', 'b': [b'bb', 'cc']}
    self.assertEqual('{"a": "aa", "b": ["bb", "cc"]}',
                     json.dumps(utils.ConvertBytesBeforeJsonDumps(dict_obj)),
                     'Fail converting dict object')

    list_obj = [{'a': b'aa', 'b': b'bb'}, {'c': b'cc'}]
    self.assertEqual('[{"a": "aa", "b": "bb"}, {"c": "cc"}]',
                     json.dumps(utils.ConvertBytesBeforeJsonDumps(list_obj)),
                     'Fail converting list object')


def _MakeMockFetch(base64_encoded=True, status=200):
  """Returns a mock fetch object that returns a canned response."""

  def _MockFetch(_):
    response_text = json.dumps({'key': 'this is well-formed JSON.'})
    if base64_encoded:
      response_text = base64.b64encode(response_text)
    return testing_common.FakeResponseObject(status, response_text)

  return _MockFetch


if __name__ == '__main__':
  unittest.main()
