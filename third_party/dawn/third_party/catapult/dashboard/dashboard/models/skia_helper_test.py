# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

import datetime
from unittest import mock

from dashboard.common import testing_common
from dashboard.models import skia_helper

MOCK_MAPPING = [{
    'label': 'label_1',
    'public_host': 'https://a.com',
    'internal_host': 'https://a.corp',
    'masters': ['master_a', 'master_b']
}, {
    'label': 'label_2',
    'public_host': 'https://b.com',
    'internal_host': None,
    'masters': ['master_c']
}]


class SkiaHelper(testing_common.TestCase):
  # pylint: disable=line-too-long
  @mock.patch('dashboard.models.skia_helper.REPOSITORY_HOST_MAPPING',
              MOCK_MAPPING)
  def testGetSkiaUrl_NonExistentMaster(self):
    url = skia_helper.GetSkiaUrl(
        start_time=datetime.datetime(2020, 5, 17),
        end_time=datetime.datetime(2020, 5, 18),
        master='master_d',
        bots=['bot_a', 'bot_b', 'bot_c'],
        benchmarks=['benchmark_a', 'benchmark_b'],
        tests=['test_a', 'test_b'],
        subtests_1=['subtest_1_a', 'subtest_1_b'],
        subtests_2=['subtest_2_a', 'subtest_2_b'],
        internal_only=True,
        num_points=500)
    self.assertEqual(url, None)

  @mock.patch('dashboard.models.skia_helper.REPOSITORY_HOST_MAPPING',
              MOCK_MAPPING)
  def testGetSkiaUrl_MultipleProperties(self):
    url = skia_helper.GetSkiaUrl(
        start_time=datetime.datetime(2020, 5, 17),
        end_time=datetime.datetime(2020, 5, 18),
        master='master_a',
        bots=['bot_a', 'bot_b', 'bot_c'],
        benchmarks=['benchmark_a', 'benchmark_b'],
        tests=['test_a', 'test_b'],
        subtests_1=['subtest_1_a', 'subtest_1_b'],
        subtests_2=['subtest_2_a', 'subtest_2_b'],
        internal_only=True,
        num_points=500)
    expected_url = 'https://a.corp/e/?numCommits=500&begin=1589673600&end=1589760000&queries=stat%3Dvalue%26benchmark%3Dbenchmark_a%26benchmark%3Dbenchmark_b%26bot%3Dbot_a%26bot%3Dbot_b%26bot%3Dbot_c%26test%3Dtest_a%26test%3Dtest_b%26subtest_1%3Dsubtest_1_a%26subtest_1%3Dsubtest_1_b%26subtest_2%3Dsubtest_2_a%26subtest_2%3Dsubtest_2_b'
    self.assertEqual(url, expected_url)

  @mock.patch('dashboard.models.skia_helper.REPOSITORY_HOST_MAPPING',
              MOCK_MAPPING)
  def testGetSkiaUrl_SomeEmptyProperties(self):
    url = skia_helper.GetSkiaUrl(
        start_time=datetime.datetime(2020, 5, 17),
        end_time=datetime.datetime(2020, 5, 18),
        master='master_a',
        bots=['bot_a', 'bot_b', 'bot_c'],
        benchmarks=[],
        tests=['test_a', 'test_b'],
        subtests_1=[],
        subtests_2=['subtest_2_a', 'subtest_2_b'],
        internal_only=True,
        num_points=500)
    expected_url = 'https://a.corp/e/?numCommits=500&begin=1589673600&end=1589760000&queries=stat%3Dvalue%26bot%3Dbot_a%26bot%3Dbot_b%26bot%3Dbot_c%26test%3Dtest_a%26test%3Dtest_b%26subtest_2%3Dsubtest_2_a%26subtest_2%3Dsubtest_2_b'
    self.assertEqual(url, expected_url)

  @mock.patch('dashboard.models.skia_helper.REPOSITORY_HOST_MAPPING',
              MOCK_MAPPING)
  def testGetSkiaUrl_PublicHost(self):
    url = skia_helper.GetSkiaUrl(
        start_time=datetime.datetime(2020, 5, 17),
        end_time=datetime.datetime(2020, 5, 18),
        master='master_a',
        bots=['bot_a', 'bot_b', 'bot_c'],
        benchmarks=['benchmark_a', 'benchmark_b'],
        tests=['test_a', 'test_b'],
        subtests_1=['subtest_1_a', 'subtest_1_b'],
        subtests_2=['subtest_2_a', 'subtest_2_b'],
        internal_only=False,
        num_points=500)
    expected_url = 'https://a.com/e/?numCommits=500&begin=1589673600&end=1589760000&queries=stat%3Dvalue%26benchmark%3Dbenchmark_a%26benchmark%3Dbenchmark_b%26bot%3Dbot_a%26bot%3Dbot_b%26bot%3Dbot_c%26test%3Dtest_a%26test%3Dtest_b%26subtest_1%3Dsubtest_1_a%26subtest_1%3Dsubtest_1_b%26subtest_2%3Dsubtest_2_a%26subtest_2%3Dsubtest_2_b'
    self.assertEqual(url, expected_url)

  @mock.patch('dashboard.models.skia_helper.REPOSITORY_HOST_MAPPING',
              MOCK_MAPPING)
  def testGetSkiaUrl_NoneHost(self):
    url = skia_helper.GetSkiaUrl(
        start_time=datetime.datetime(2020, 5, 17),
        end_time=datetime.datetime(2020, 5, 18),
        master='master_c',
        bots=['bot_a', 'bot_b', 'bot_c'],
        benchmarks=['benchmark_a', 'benchmark_b'],
        tests=['test_a', 'test_b'],
        subtests_1=['subtest_1_a', 'subtest_1_b'],
        subtests_2=['subtest_2_a', 'subtest_2_b'],
        internal_only=True,
        num_points=500)
    self.assertEqual(url, None)

  @mock.patch('dashboard.models.skia_helper.REPOSITORY_HOST_MAPPING',
              MOCK_MAPPING)
  def testGetSkiaUrl_AllEmptyProperties(self):
    url = skia_helper.GetSkiaUrl(
        start_time=datetime.datetime(2020, 5, 17),
        end_time=datetime.datetime(2020, 5, 18),
        master='master_a',
        bots=[],
        benchmarks=[],
        tests=[],
        subtests_1=[],
        subtests_2=[],
        internal_only=True,
        num_points=500)
    expected_url = 'https://a.corp/e/?numCommits=500&begin=1589673600&end=1589760000&queries=stat%3Dvalue'
    self.assertEqual(url, expected_url)

  @mock.patch('dashboard.models.skia_helper.REPOSITORY_HOST_MAPPING',
              MOCK_MAPPING)
  def testGetSkiaUrl_NoneProperties(self):
    url = skia_helper.GetSkiaUrl(
        start_time=datetime.datetime(2020, 5, 17),
        end_time=datetime.datetime(2020, 5, 18),
        master='master_a',
        bots=[],
        benchmarks=None,
        tests=[],
        subtests_1=[],
        subtests_2=None,
        internal_only=True,
        num_points=500)
    expected_url = 'https://a.corp/e/?numCommits=500&begin=1589673600&end=1589760000&queries=stat%3Dvalue'
    self.assertEqual(url, expected_url)

  @mock.patch('dashboard.models.skia_helper.REPOSITORY_HOST_MAPPING',
              MOCK_MAPPING)
  def testGetSkiaUrlForAlertGroup_ValidGroupId(self):
    urls = skia_helper.GetSkiaUrlsForAlertGroup(
        alert_group_id='abc',
        internal_only=True,
        masters=['master_a'],
    )
    expected_url = 'label_1: https://a.corp/_/alertgroup?group_id=abc'
    self.assertEqual(urls[0], expected_url)

  @mock.patch('dashboard.models.skia_helper.REPOSITORY_HOST_MAPPING',
              MOCK_MAPPING)
  def testGetSkiaUrlForAlertGroup_PublicHost(self):
    urls = skia_helper.GetSkiaUrlsForAlertGroup(
        alert_group_id='abc',
        internal_only=False,
        masters=['master_a'],
    )
    expected_url = 'label_1: https://a.com/_/alertgroup?group_id=abc'
    self.assertEqual(urls[0], expected_url)

  @mock.patch('dashboard.models.skia_helper.REPOSITORY_HOST_MAPPING',
              MOCK_MAPPING)
  def testGetSkiaUrlForAlertGroup_EmptyGroupId(self):
    urls = skia_helper.GetSkiaUrlsForAlertGroup(
        alert_group_id='',
        internal_only=True,
        masters=['master_a'],
    )
    expected_url = 'label_1: https://a.corp/_/alertgroup?group_id='
    self.assertEqual(urls[0], expected_url)

  @mock.patch('dashboard.models.skia_helper.REPOSITORY_HOST_MAPPING',
              MOCK_MAPPING)
  def testGetSkiaUrlForAlertGroup_InvalidMaster(self):
    urls = skia_helper.GetSkiaUrlsForAlertGroup(
        alert_group_id='',
        internal_only=True,
        masters=['master_c'],
    )
    self.assertEqual(len(urls), 0)

  @mock.patch('dashboard.models.skia_helper.REPOSITORY_HOST_MAPPING',
              MOCK_MAPPING)
  def testGetSkiaUrlForAlertGroup_MultipleMasters_SameHost(self):
    urls = skia_helper.GetSkiaUrlsForAlertGroup(
        alert_group_id='',
        internal_only=True,
        masters=['master_a', 'master_b'],
    )
    self.assertEqual(len(urls), 1)
    expected_url = 'label_1: https://a.corp/_/alertgroup?group_id='
    self.assertEqual(urls[0], expected_url)

  @mock.patch('dashboard.models.skia_helper.REPOSITORY_HOST_MAPPING',
              MOCK_MAPPING)
  def testGetSkiaUrlForAlertGroup_MultipleMasters_DifferentHost(self):
    urls = skia_helper.GetSkiaUrlsForAlertGroup(
        alert_group_id='',
        internal_only=False,
        masters=['master_a', 'master_c'],
    )
    self.assertEqual(len(urls), 2)
    expected = [
        'label_1: https://a.com/_/alertgroup?group_id=',
        'label_2: https://b.com/_/alertgroup?group_id='
    ]
    self.assertEqual(sorted(urls), expected)

  @mock.patch('dashboard.models.skia_helper.REPOSITORY_HOST_MAPPING',
              MOCK_MAPPING)
  def testGetSkiaUrlForAlertGroup_NonExistentHost(self):
    urls = skia_helper.GetSkiaUrlsForAlertGroup(
        alert_group_id='',
        internal_only=True,
        masters=['master_d'],
    )
    self.assertEqual(len(urls), 0)


if __name__ == '__main__':
  unittest.main()
