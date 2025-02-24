# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import datetime
import unittest

from google.appengine.ext import ndb

from dashboard.common import descriptor
from dashboard.common import report_query
from dashboard.common import stored_object
from dashboard.common import testing_common
from dashboard.models import graph_data
from dashboard.models import report_template


@report_template.Static(
    internal_only=False,
    template_id=584630894,
    name='Test:External',
    modified=datetime.datetime.now())
def _External(revisions):
  template = {
      'rows': [],
      'statistics': ['avg'],
      'url': 'http://exter.nal',
  }
  return report_query.ReportQuery(template, revisions)


class ReportTemplateTest(testing_common.TestCase):

  def setUp(self):
    super().setUp()
    stored_object.Set(descriptor.PARTIAL_TEST_SUITES_KEY, [])
    stored_object.Set(descriptor.COMPOSITE_TEST_SUITES_KEY, [])
    stored_object.Set(descriptor.GROUPABLE_TEST_SUITE_PREFIXES_KEY, [])
    descriptor.Descriptor.ResetMemoizedConfigurationForTesting()

    report_template.ReportTemplate(
        id='ex-id',
        name='external',
        owners=[testing_common.EXTERNAL_USER.email()],
        template={
            'rows': [],
            'statistics': ['avg']
        }).put()
    report_template.ReportTemplate(
        internal_only=True,
        name='internal',
        id='in-id',
        owners=[testing_common.INTERNAL_USER.email()],
        template={
            'rows': [],
            'statistics': ['avg']
        }).put()

  def testInternal_PutTemplate(self):
    self.SetCurrentUser(testing_common.INTERNAL_USER.email())

    with self.assertRaises(ValueError):
      report_template.PutTemplate(None, 'Test:External',
                                  [testing_common.INTERNAL_USER.email()],
                                  {'rows': []})

    with self.assertRaises(ValueError):
      report_template.PutTemplate('in-id', 'Test:External',
                                  [testing_common.INTERNAL_USER.email()],
                                  {'rows': []})

    with self.assertRaises(ValueError):
      report_template.PutTemplate('invalid', 'bad',
                                  [testing_common.INTERNAL_USER.email()],
                                  {'rows': []})

    with self.assertRaises(ValueError):
      report_template.PutTemplate('ex-id', 'bad',
                                  [testing_common.INTERNAL_USER.email()],
                                  {'rows': []})
    self.assertEqual('external', ndb.Key('ReportTemplate', 'ex-id').get().name)

    with self.assertRaises(ValueError):
      report_template.PutTemplate(584630894, 'bad',
                                  [testing_common.INTERNAL_USER.email()],
                                  {'rows': []})

    report_template.PutTemplate('in-id', 'foo',
                                [testing_common.INTERNAL_USER.email()],
                                {'rows': []})
    self.assertEqual('foo', ndb.Key('ReportTemplate', 'in-id').get().name)

  def testPutTemplate_InternalOnly(self):
    self.SetCurrentUser(testing_common.INTERNAL_USER.email())
    test = graph_data.TestMetadata(
        has_rows=True,
        internal_only=True,
        id='master/internalbot/internalsuite/measure',
        units='units')
    test.put()
    report_template.PutTemplate(
        None, 'foo', [testing_common.INTERNAL_USER.email()], {
            'rows': [{
                'testSuites': ['internalsuite'],
                'bots': ['master:internalbot'],
                'measurement': 'measure',
                'testCases': [],
            },],
        })
    template = report_template.ReportTemplate.query(
        report_template.ReportTemplate.name == 'foo').get()
    self.assertTrue(template.internal_only)

  def testPutTemplate_External(self):
    self.SetCurrentUser(testing_common.EXTERNAL_USER.email())
    report_template.PutTemplate(
        None, 'foo', [testing_common.EXTERNAL_USER.email()], {
            'rows': [{
                'testSuites': ['externalsuite'],
                'bots': ['master:externalbot'],
                'measurement': 'measure',
                'testCases': [],
            },],
        })
    template = report_template.ReportTemplate.query(
        report_template.ReportTemplate.name == 'foo').get()
    self.assertFalse(template.internal_only)

  def testAnonymous_PutTemplate(self):
    self.SetCurrentUser('')
    with self.assertRaises(ValueError):
      report_template.PutTemplate('ex-id', 'foo',
                                  [testing_common.EXTERNAL_USER.email()], {})
    self.assertEqual('external', ndb.Key('ReportTemplate', 'ex-id').get().name)

  def testInternal_GetReport(self):
    self.SetCurrentUser(testing_common.INTERNAL_USER.email())
    report = report_template.GetReport('in-id', [10, 20])
    self.assertTrue(report['internal'])
    self.assertEqual(0, len(report['report']['rows']))
    self.assertEqual('internal', report['name'])
    self.assertTrue(report['editable'])
    self.assertEqual([testing_common.INTERNAL_USER.email()], report['owners'])

  def testAnonymous_GetReport(self):
    self.SetCurrentUser('')
    self.assertEqual(None, report_template.GetReport('in-id', [10, 20]))
    report = report_template.GetReport('ex-id', [10, 20])
    self.assertFalse(report['internal'])
    self.assertEqual(0, len(report['report']['rows']))
    self.assertEqual('external', report['name'])
    self.assertFalse(report['editable'])
    self.assertEqual([testing_common.EXTERNAL_USER.email()], report['owners'])

  def testGetReport_Static(self):
    self.SetCurrentUser(testing_common.EXTERNAL_USER.email())
    report = report_template.GetReport(584630894, ['latest'])
    self.assertFalse(report['internal'])
    self.assertEqual('http://exter.nal', report['report']['url'])
    self.assertEqual('Test:External', report['name'])
    self.assertFalse(report['editable'])
    self.assertNotIn('owners', report)


if __name__ == '__main__':
  unittest.main()
