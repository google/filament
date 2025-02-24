# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from google.appengine.ext import ndb

from dashboard import email_template
from dashboard.common import utils
from dashboard.models import anomaly


class EmailTemplateTest(unittest.TestCase):

  def testURLEncoding(self):
    actual_output = email_template.GetReportPageLink(
        'ABC/bot-name/abc-perf-test/passed%', '1415919839')

    self.assertEqual(('https://chromeperf.appspot.com/report?masters=ABC&'
                      'bots=bot-name&tests=abc-perf-test%2Fpassed%25'
                      '&checked=passed%25%2Cpassed%25_ref%2Cref&'
                      'rev=1415919839'), actual_output)

    actual_output_no_host = email_template.GetReportPageLink(
        'ABC/bot-name/abc-perf-test/passed%',
        '1415919839',
        add_protocol_and_host=False)

    self.assertEqual(('/report?masters=ABC&bots=bot-name&tests='
                      'abc-perf-test%2Fpassed%25&checked=passed%25%2C'
                      'passed%25_ref%2Cref&rev=1415919839'),
                     actual_output_no_host)

  def testGetReportPageLink(self):
    test_path = 'a/b/c/d'
    link = email_template.GetReportPageLink(
        test_path, rev=None, add_protocol_and_host=True)
    self.assertEqual(
        link,
        'https://chromeperf.appspot.com/report?masters=a&bots=b&tests=c%2Fd&checked=d%2Cd_ref%2Cref'
    )

  def testGetReportPageLinkWithRev(self):
    test_path = 'a/b/c/d'
    link = email_template.GetReportPageLink(
        test_path, rev='1234', add_protocol_and_host=False)
    self.assertEqual(
        link,
        '/report?masters=a&bots=b&tests=c%2Fd&checked=d%2Cd_ref%2Cref&rev=1234')

  def testGetGroupReportPageLink(self):
    a1 = anomaly.Anomaly(id='test_id')
    a2 = anomaly.Anomaly(test=utils.TestKey('a/b/c/d'))

    link1 = email_template.GetGroupReportPageLink(a1)
    link2 = email_template.GetGroupReportPageLink(a2)

    link1_prefix = 'https://chromeperf.appspot.com/group_report?keys='

    self.assertTrue(link1.startswith(link1_prefix))
    self.assertEqual(ndb.Key(urlsafe=link1[len(link1_prefix):]), a1.key)
    self.assertEqual(
        link2,
        'https://chromeperf.appspot.com/report?masters=a&bots=b&tests=c%2Fd&checked=d%2Cd_ref%2Cref'
    )


if __name__ == '__main__':
  unittest.main()
