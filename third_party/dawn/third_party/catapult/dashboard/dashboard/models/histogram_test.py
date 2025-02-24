# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json
import sys

from dashboard.common import testing_common
from dashboard.common import utils
from dashboard.models import histogram
from tracing.value.diagnostics import generic_set
from tracing.value.diagnostics import reserved_infos


class SparseDiagnosticTest(testing_common.TestCase):
  """Test case for functions in SparseDiagnostic."""

  def setUp(self):
    super().setUp()
    self.SetCurrentUser('foo@bar.com', is_admin=True)

  def _AddMockData(self, test_key):
    data_samples = {
        'owners': [
            {
                'type': 'GenericSet',
                'guid': '1',
                'values': ['1']
            },
            {
                'type': 'GenericSet',
                'guid': '1',
                'values': ['2']
            },
            {
                'type': 'GenericSet',
                'guid': '1',
                'values': ['3']
            },
        ],
        'bugs': [
            {
                'type': 'GenericSet',
                'guid': '1',
                'values': ['a']
            },
            {
                'type': 'GenericSet',
                'guid': '1',
                'values': ['b']
            },
            {
                'type': 'GenericSet',
                'guid': '1',
                'values': ['c']
            },
        ]
    }
    for k, diagnostic_samples in data_samples.items():
      for i, sample in enumerate(diagnostic_samples):
        start_revision = i * 10
        end_revision = (i + 1) * 10 - 1
        if i == len(diagnostic_samples) - 1:
          end_revision = sys.maxsize

        e = histogram.SparseDiagnostic(
            data=sample,
            test=test_key,
            start_revision=start_revision,
            end_revision=end_revision,
            name=k,
            internal_only=False)
        e.put()

  def testFixupDiagnostics_Middle_FixesRange(self):
    test_key = utils.TestKey('Chromium/win7/foo')

    self._AddMockData(test_key)

    data = {'type': 'GenericSet', 'guid': '1', 'values': ['10']}

    e = histogram.SparseDiagnostic(
        data=data,
        test=test_key,
        start_revision=5,
        end_revision=sys.maxsize,
        name='owners',
        internal_only=False)
    e.put()

    histogram.SparseDiagnostic.FixDiagnostics(test_key).get_result()

    expected = {
        'owners': [(0, 4), (5, 9), (10, 19), (20, sys.maxsize)],
        'bugs': [(0, 9), (10, 19), (20, sys.maxsize)],
    }
    diags = histogram.SparseDiagnostic.query().fetch()
    for d in diags:
      self.assertIn((d.start_revision, d.end_revision), expected[d.name])
      expected[d.name].remove((d.start_revision, d.end_revision))
    self.assertEqual(0, len(expected['owners']))
    self.assertEqual(0, len(expected['bugs']))

  def testFixupDiagnostics_End_FixesRange(self):
    test_key = utils.TestKey('Chromium/win7/foo')

    self._AddMockData(test_key)

    data = {'type': 'GenericSet', 'guid': '1', 'values': ['10']}

    e = histogram.SparseDiagnostic(
        data=data,
        test=test_key,
        start_revision=100,
        end_revision=sys.maxsize,
        name='owners',
        internal_only=False)
    e.put()

    histogram.SparseDiagnostic.FixDiagnostics(test_key).get_result()

    expected = {
        'owners': [(0, 9), (10, 19), (20, 99), (100, sys.maxsize)],
        'bugs': [(0, 9), (10, 19), (20, sys.maxsize)],
    }
    diags = histogram.SparseDiagnostic.query().fetch()
    for d in diags:
      self.assertIn((d.start_revision, d.end_revision), expected[d.name])
      expected[d.name].remove((d.start_revision, d.end_revision))
    self.assertEqual(0, len(expected['owners']))
    self.assertEqual(0, len(expected['bugs']))

  def testFixupDiagnostics_DifferentTestPath_NoChange(self):
    test_key1 = utils.TestKey('Chromium/win7/1')
    test_key2 = utils.TestKey('Chromium/win7/2')

    self._AddMockData(test_key1)
    self._AddMockData(test_key2)

    data = {'type': 'GenericSet', 'guid': '1', 'values': ['10']}

    e = histogram.SparseDiagnostic(
        data=data,
        test=test_key1,
        start_revision=5,
        end_revision=sys.maxsize,
        name='owners',
        internal_only=False)
    e.put()

    histogram.SparseDiagnostic.FixDiagnostics(test_key2).get_result()

    expected = {
        'owners': [(0, 9), (10, 19), (20, sys.maxsize)],
        'bugs': [(0, 9), (10, 19), (20, sys.maxsize)],
    }
    diags = histogram.SparseDiagnostic.query(
        histogram.SparseDiagnostic.test == test_key2).fetch()
    for d in diags:
      self.assertIn((d.start_revision, d.end_revision), expected[d.name])
      expected[d.name].remove((d.start_revision, d.end_revision))
    self.assertEqual(0, len(expected['owners']))
    self.assertEqual(0, len(expected['bugs']))

  def testFixupDiagnostics_NotUnique_NoChange(self):
    test_key = utils.TestKey('Chromium/win7/foo')

    self._AddMockData(test_key)

    data = {'type': 'GenericSet', 'guid': '1', 'values': ['1']}

    e = histogram.SparseDiagnostic(
        data=data,
        test=test_key,
        start_revision=5,
        end_revision=sys.maxsize,
        name='owners',
        internal_only=False)
    e.put()

    histogram.SparseDiagnostic.FixDiagnostics(test_key).get_result()

    expected = {
        'owners': [(0, 9), (10, 19), (20, sys.maxsize)],
        'bugs': [(0, 9), (10, 19), (20, sys.maxsize)],
    }
    diags = histogram.SparseDiagnostic.query(
        histogram.SparseDiagnostic.test == test_key).fetch()
    for d in diags:
      self.assertIn((d.start_revision, d.end_revision), expected[d.name])
      expected[d.name].remove((d.start_revision, d.end_revision))
    self.assertEqual(0, len(expected['owners']))
    self.assertEqual(0, len(expected['bugs']))

  def testGetMostRecentDataByNames_ReturnAllData(self):
    data_samples = [{
        'type': 'GenericSet',
        'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb826',
        'values': ['alice@chromium.org']
    }, {
        'type': 'GenericSet',
        'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb827',
        'values': ['abc']
    }]

    test_key = utils.TestKey('Chromium/win7/foo')
    entity = histogram.SparseDiagnostic(
        data=data_samples[0],
        test=test_key,
        start_revision=1,
        end_revision=sys.maxsize,
        id=data_samples[0]['guid'],
        name=reserved_infos.OWNERS.name)
    entity.put()

    entity = histogram.SparseDiagnostic(
        data=data_samples[1],
        test=test_key,
        start_revision=1,
        end_revision=sys.maxsize,
        id=data_samples[1]['guid'],
        name=reserved_infos.BUG_COMPONENTS.name)
    entity.put()

    lookup_result = histogram.SparseDiagnostic.GetMostRecentDataByNamesSync(
        test_key,
        {reserved_infos.OWNERS.name, reserved_infos.BUG_COMPONENTS.name})

    self.assertEqual(
        lookup_result.get(reserved_infos.OWNERS.name).get('values'),
        ['alice@chromium.org'])
    self.assertEqual(
        lookup_result.get(reserved_infos.BUG_COMPONENTS.name).get('values'),
        ['abc'])

  def testGetMostRecentDataByNames_ReturnsNoneIfNoneFound(self):
    data_sample = {
        'type': 'GenericSet',
        'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb826',
        'values': ['alice@chromium.org']
    }

    test_key = utils.TestKey('Chromium/win7/foo')
    entity = histogram.SparseDiagnostic(
        data=data_sample,
        test=test_key,
        start_revision=1,
        end_revision=sys.maxsize,
        id=data_sample['guid'],
        name=reserved_infos.OWNERS.name)
    entity.put()

    lookup_result = histogram.SparseDiagnostic.GetMostRecentDataByNamesSync(
        test_key,
        {reserved_infos.OWNERS.name, reserved_infos.BUG_COMPONENTS.name})

    self.assertEqual(
        lookup_result.get(reserved_infos.OWNERS.name).get('values'),
        ['alice@chromium.org'])
    self.assertIsNone(lookup_result.get(reserved_infos.BUG_COMPONENTS.name))

  def testGetMostRecentDataByNames_ReturnsNoneIfNoName(self):
    data_sample = {'guid': 'abc', 'osName': 'linux', 'type': 'DeviceInfo'}

    test_key = utils.TestKey('Chromium/win7/foo')
    entity = histogram.SparseDiagnostic(
        data=json.dumps(data_sample),
        test=test_key,
        start_revision=1,
        end_revision=sys.maxsize,
        id=data_sample['guid'])
    entity.put()

    lookup_result = histogram.SparseDiagnostic.GetMostRecentDataByNamesSync(
        test_key,
        {reserved_infos.OWNERS.name, reserved_infos.BUG_COMPONENTS.name})

    self.assertIsNone(lookup_result.get(reserved_infos.OWNERS.name))
    self.assertIsNone(lookup_result.get(reserved_infos.BUG_COMPONENTS.name))

  def testGetMostRecentDataByNames_ToleratesDuplicateName(self):
    data_samples = [{
        'type': 'GenericSet',
        'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb826',
        'values': ['alice@chromium.org']
    }, {
        'type': 'GenericSet',
        'guid': 'eb212e80-db58-4cbd-b331-c2245ecbb827',
        'values': ['bob@chromium.org']
    }]

    test_key = utils.TestKey('Chromium/win7/foo')
    entity = histogram.SparseDiagnostic(
        data=data_samples[0],
        test=test_key,
        start_revision=1,
        end_revision=sys.maxsize,
        id=data_samples[0]['guid'],
        name=reserved_infos.OWNERS.name)
    entity.put()

    entity = histogram.SparseDiagnostic(
        data=data_samples[1],
        test=test_key,
        start_revision=2,
        end_revision=sys.maxsize,
        id=data_samples[1]['guid'],
        name=reserved_infos.OWNERS.name)
    entity.put()

    # TODO(crbug.com/877809): assertRaises
    lookup_result = histogram.SparseDiagnostic.GetMostRecentDataByNamesSync(
        test_key, {reserved_infos.OWNERS.name})
    self.assertEqual(
        lookup_result.get(reserved_infos.OWNERS.name).get('values'),
        data_samples[1]['values'])

  def _CreateGenericDiagnostic(self,
                               name,
                               values,
                               test_key,
                               start_revision,
                               end_revision=sys.maxsize):
    d = generic_set.GenericSet([values])
    e = histogram.SparseDiagnostic(
        id=d.guid,
        data=d.AsDict(),
        name=name,
        test=test_key,
        start_revision=start_revision,
        end_revision=end_revision)
    return e

  def _AddGenericDiagnostic(self,
                            name,
                            values,
                            test_key,
                            start_revision,
                            end_revision=sys.maxsize):
    e = self._CreateGenericDiagnostic(name, values, test_key, start_revision,
                                      end_revision)
    ek = e.put()
    suite_key = utils.TestKey('/'.join(test_key.id().split('/')[:3]))
    histogram.HistogramRevisionRecord.GetOrCreate(suite_key,
                                                  start_revision).put()
    histogram.SparseDiagnostic.FixDiagnostics(test_key).get_result()
    return ek

  def _CheckExpectations(self, diagnostic, guid_mapping, expected):
    q = histogram.SparseDiagnostic.query()
    q = q.order(histogram.SparseDiagnostic.end_revision)
    sparse = q.fetch()

    # Check that the mapping is correct, in that if there is one it should point
    # to a diagnostic with a valid range and same data.
    sparse_by_guid = dict((s.key.id(), s) for s in sparse)

    if guid_mapping:
      mapped_diagnostic = guid_mapping[diagnostic.key.id()]
      existing_diagnostic = sparse_by_guid[mapped_diagnostic['guid']]

      self.assertFalse(existing_diagnostic.IsDifferent(diagnostic))

      # We check that the start position is within the range, but not the end
      # since the end is set to sys.maxint and gets capped during insertion.
      self.assertTrue(
          existing_diagnostic.start_revision <= diagnostic.start_revision)
      self.assertTrue(
          existing_diagnostic.end_revision >= diagnostic.start_revision)

    for d in sparse:
      if not isinstance(d.data, dict):
        continue
      self.assertIn((d.start_revision, d.end_revision, d.data['values']),
                    expected)
      expected.remove((d.start_revision, d.end_revision, d.data['values']))

    self.assertFalse(expected)

  def testFindOrInsertDiagnostics_Latest_Same(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 1)
    e = self._CreateGenericDiagnostic('foo', 'm1', test_key, 10)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e], test_key, e.start_revision, e.start_revision).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, sys.maxsize, [u'm1']),
    ])

  def testFindOrInsertDiagnostics_Latest_Different(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 1)
    e = self._CreateGenericDiagnostic('foo', 'm2', test_key, 10)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e], test_key, e.start_revision, e.start_revision).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, 9, [u'm1']),
        (10, sys.maxsize, [u'm2']),
    ])

  def testFindOrInsertDiagnostics_Latest_Invalid(self):
    test_key = utils.TestKey('M/B/S')

    invalid = self._AddGenericDiagnostic('foo', 'm1', test_key, 1).get()
    invalid.data = '{'
    invalid.put()
    e = self._CreateGenericDiagnostic('foo', 'm2', test_key, 10)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e], test_key, e.start_revision, e.start_revision).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (10, sys.maxsize, [u'm2']),
    ])

  def testFindOrInsertDiagnostics_Latest_New(self):
    test_key = utils.TestKey('M/B/S')

    e = self._CreateGenericDiagnostic('foo', 'm1', test_key, 10)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e], test_key, e.start_revision, e.start_revision).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (10, sys.maxsize, [u'm1']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_Same(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 1)
    self._AddGenericDiagnostic('foo', 'm1', test_key, 10)
    e = self._CreateGenericDiagnostic('foo', 'm1', test_key, 5)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            10,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, sys.maxsize, [u'm1']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_Before_Same(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 5)
    self._AddGenericDiagnostic('foo', 'm1', test_key, 10)
    e = self._CreateGenericDiagnostic('foo', 'm1', test_key, 1)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            10,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, sys.maxsize, [u'm1']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_Before_Diff(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 5)
    self._AddGenericDiagnostic('foo', 'm1', test_key, 10)
    e = self._CreateGenericDiagnostic('foo', 'm2', test_key, 1)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            10,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, 4, [u'm2']),
        (5, sys.maxsize, [u'm1']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_Splits_CurSame_NextDiff(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 1)
    self._AddGenericDiagnostic('foo', 'm1', test_key, 10)
    e = self._CreateGenericDiagnostic('foo', 'm2', test_key, 5)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            10,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, 4, [u'm1']),
        (5, 9, [u'm2']),
        (10, sys.maxsize, [u'm1']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_Splits_CurDiff_NextNone(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 1)
    self._AddGenericDiagnostic('foo', 'm1', test_key, 10)
    e = self._CreateGenericDiagnostic('foo', 'm2', test_key, 12)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            10,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, 11, [u'm1']),
        (12, sys.maxsize, [u'm2']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_Splits_CurDiff_NextNone_Rev(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 1)
    self._AddGenericDiagnostic('foo', 'm1', test_key, 10)
    e = self._CreateGenericDiagnostic('foo', 'm2', test_key, 8)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            10,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, 7, [u'm1']),
        (8, 9, [u'm2']),
        (10, sys.maxsize, [u'm1']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_Splits_CurDiff_HasRevs(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 1)
    self._AddGenericDiagnostic('foo', 'm1', test_key, 8)
    self._AddGenericDiagnostic('foo', 'm2', test_key, 10)
    e = self._CreateGenericDiagnostic('foo', 'm2', test_key, 5)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            10,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, 4, [u'm1']),
        (5, 7, [u'm2']),
        (8, 9, [u'm1']),
        (10, sys.maxsize, [u'm2']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_Splits_CurDiff_NextDiff(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 1)
    self._AddGenericDiagnostic('foo', 'm3', test_key, 10)
    e = self._CreateGenericDiagnostic('foo', 'm2', test_key, 5)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            10,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, 4, [u'm1']),
        (5, 9, [u'm2']),
        (10, sys.maxsize, [u'm3']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_Splits_CurDiff_NextSame(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 1)
    self._AddGenericDiagnostic('foo', 'm3', test_key, 10)
    e = self._CreateGenericDiagnostic('foo', 'm3', test_key, 5)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            10,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, 4, [u'm1']),
        (5, sys.maxsize, [u'm3']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_Clobber_NoNext_NoRevs(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 1)
    self._AddGenericDiagnostic('foo', 'm3', test_key, 10)
    e = self._CreateGenericDiagnostic('foo', 'm2', test_key, 10)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            10,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, 9, [u'm1']),
        (10, sys.maxsize, [u'm2']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_Clobber_NoNext_Revs(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 1)
    self._AddGenericDiagnostic('foo', 'm3', test_key, 10)
    self._AddGenericDiagnostic('foo', 'm3', test_key, 15)
    e = self._CreateGenericDiagnostic('foo', 'm2', test_key, 10)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            15,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, 9, [u'm1']),
        (10, 14, [u'm2']),
        (15, sys.maxsize, [u'm3']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_Clobber_Next_Revs(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 1)
    self._AddGenericDiagnostic('foo', 'm3', test_key, 10)
    self._AddGenericDiagnostic('foo', 'm3', test_key, 13)
    self._AddGenericDiagnostic('foo', 'm4', test_key, 15)
    e = self._CreateGenericDiagnostic('foo', 'm2', test_key, 10)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            15,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, 9, [u'm1']),
        (10, 12, [u'm2']),
        (13, 14, [u'm3']),
        (15, sys.maxsize, [u'm4']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_Clobber_Next_Revs_Same(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 1)
    self._AddGenericDiagnostic('foo', 'm3', test_key, 10)
    self._AddGenericDiagnostic('foo', 'm2', test_key, 15)
    e = self._CreateGenericDiagnostic('foo', 'm2', test_key, 10)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            15,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, 9, [u'm1']),
        (10, sys.maxsize, [u'm2']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_Clobber_Next_NoRevs_Diff(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 1)
    self._AddGenericDiagnostic('foo', 'm2', test_key, 10)
    self._AddGenericDiagnostic('foo', 'm3', test_key, 15)
    e = self._CreateGenericDiagnostic('foo', 'm4', test_key, 10)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            15,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, 9, [u'm1']),
        (10, 14, [u'm4']),
        (15, sys.maxsize, [u'm3']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_Clobber_Next_NoRevs_Same(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 1)
    self._AddGenericDiagnostic('foo', 'm2', test_key, 10)
    self._AddGenericDiagnostic('foo', 'm3', test_key, 15)
    e = self._CreateGenericDiagnostic('foo', 'm3', test_key, 10)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            15,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, 9, [u'm1']),
        (10, sys.maxsize, [u'm3']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_Clobber_Next_Prev_Same(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 1)
    self._AddGenericDiagnostic('foo', 'm2', test_key, 10)
    self._AddGenericDiagnostic('foo', 'm1', test_key, 15)
    e = self._CreateGenericDiagnostic('foo', 'm1', test_key, 10)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            15,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, sys.maxsize, [u'm1']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_Clobber_NextDiff_PrevSame(self):
    test_key = utils.TestKey('M/B/S')

    self._AddGenericDiagnostic('foo', 'm1', test_key, 1)
    self._AddGenericDiagnostic('foo', 'm2', test_key, 10)
    self._AddGenericDiagnostic('foo', 'm3', test_key, 15)
    e = self._CreateGenericDiagnostic('foo', 'm1', test_key, 10)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            15,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, 14, [u'm1']),
        (15, sys.maxsize, [u'm3']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_LastInvalid(self):
    test_key = utils.TestKey('M/B/S')

    # end_revision = sys.maxsize - 1 to ensure that FindOrInsertDiagnostics
    # always crashes without fix.
    self._AddGenericDiagnostic('foo', 'm1', test_key, 1, sys.maxsize - 1)
    invalid = self._AddGenericDiagnostic('foo', 'm2', test_key, 5).get()
    invalid.data = '{'
    invalid.put()

    e = self._CreateGenericDiagnostic('foo', 'm3', test_key, 3)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            5,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, 2, [u'm1']),
        (3, 4, [u'm3']),
        (5, sys.maxsize, [u'm1']),
    ])

  def testFindOrInsertDiagnostics_OutOfOrder_AllInvalid(self):
    test_key = utils.TestKey('M/B/S')

    invalid = self._AddGenericDiagnostic('foo', 'm1', test_key, 5).get()
    invalid.data = '{'
    invalid.put()

    e = self._CreateGenericDiagnostic('foo', 'm2', test_key, 1)

    guid_mapping = (
        histogram.SparseDiagnostic.FindOrInsertDiagnostics(
            [e],
            test_key,
            e.start_revision,
            5,
        ).get_result())

    self._CheckExpectations(e, guid_mapping, [
        (1, sys.maxsize, [u'm2']),
    ])
