# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import json
from unittest import mock

from dashboard.common import utils
from dashboard.pinpoint import test
from dashboard.pinpoint.handlers import jobs
from dashboard.pinpoint.models import job as job_module
from dashboard.pinpoint.models import results2 as results2_module

_SERVICE_ACCOUNT_EMAIL = 'some-service-account@example.com'


@mock.patch('dashboard.services.swarming.GetAliveBotsByDimensions',
            mock.MagicMock(return_value=["a"]))
@mock.patch('dashboard.common.cloud_metric._PublishTSCloudMetric',
            mock.MagicMock())
class JobsTest(test.TestCase):

  @mock.patch.object(utils,
                     'ServiceAccountEmail', lambda: _SERVICE_ACCOUNT_EMAIL)
  @mock.patch.object(utils, 'IsStagingEnvironment', lambda: False)
  @mock.patch.object(results2_module, 'GetCachedResults2', return_value="")
  def testGet_NoUser(self, _):
    job = job_module.Job.New((), ())

    data = json.loads(self.testapp.get('/api/jobs?o=STATE').body)

    self.assertEqual(1, data['count'])
    self.assertEqual(1, len(data['jobs']))
    self.assertEqual(job.AsDict([job_module.OPTION_STATE]), data['jobs'][0])
    self.assertFalse(data['prev'])
    self.assertFalse(data['next'])
    self.assertTrue(data['next_cursor'])
    self.assertFalse(data['prev_cursor'])


  @mock.patch.object(utils,
                     'ServiceAccountEmail', lambda: _SERVICE_ACCOUNT_EMAIL)
  @mock.patch.object(utils, 'IsStagingEnvironment', lambda: False)
  @mock.patch.object(results2_module, 'GetCachedResults2', return_value="")
  def testGet_WithUserFilter(self, _):
    job_module.Job.New((), ())
    job_module.Job.New((), (), user='lovely.user@example.com')
    job = job_module.Job.New((), (), user='lovely.user@example.com')

    data = json.loads(
        self.testapp.get(
            '/api/jobs?o=STATE&filter=user=lovely.user@example.com').body)

    # We now always explicitly get all the jobs, unless we explicitly filter
    # for a user's own jobs.
    self.assertEqual(2, data['count'])
    self.assertEqual(2, len(data['jobs']))

    self.assertFalse(data['prev'])
    self.assertFalse(data['next'])
    self.assertTrue(data['next_cursor'])
    self.assertFalse(data['prev_cursor'])

    sorted_data = sorted(data['jobs'], key=lambda d: d['job_id'])
    self.assertEqual(job.AsDict([job_module.OPTION_STATE]), sorted_data[-1])


  @mock.patch.object(utils,
                     'ServiceAccountEmail', lambda: _SERVICE_ACCOUNT_EMAIL)
  @mock.patch.object(utils, 'IsStagingEnvironment', lambda: False)
  @mock.patch.object(jobs.utils, 'GetEmail',
                     mock.MagicMock(return_value=_SERVICE_ACCOUNT_EMAIL))
  @mock.patch.object(results2_module, 'GetCachedResults2', return_value="")
  def testGet_WithServiceAccountUser(self, _):
    job_module.Job.New((), ())
    job_module.Job.New((), (), user=_SERVICE_ACCOUNT_EMAIL)
    job = job_module.Job.New((), (), user=_SERVICE_ACCOUNT_EMAIL)

    data = json.loads(
        self.testapp.get('/api/jobs?o=STATE&filter=user=%s' %
                         (_SERVICE_ACCOUNT_EMAIL,)).body)

    self.assertEqual(2, data['count'])
    self.assertEqual(2, len(data['jobs']))
    self.assertEqual(data['jobs'][0]['user'], 'chromeperf (automation)')
    self.assertEqual(data['jobs'][1]['user'], 'chromeperf (automation)')

    self.assertFalse(data['prev'])
    self.assertFalse(data['next'])
    self.assertTrue(data['next_cursor'])
    self.assertFalse(data['prev_cursor'])

    sorted_data = sorted(data['jobs'], key=lambda d: d['job_id'])
    expected_job_dict = job.AsDict([job_module.OPTION_STATE])
    expected_job_dict['user'] = 'chromeperf (automation)'
    self.assertEqual(expected_job_dict, sorted_data[-1])

  @mock.patch.object(utils,
                     'ServiceAccountEmail', lambda: utils.LEGACY_SERVICE_ACCOUNT
                    )
  @mock.patch.object(utils, 'IsStagingEnvironment', lambda: False)
  @mock.patch.object(jobs.utils, 'GetEmail',
                     mock.MagicMock(return_value=utils.LEGACY_SERVICE_ACCOUNT))
  @mock.patch.object(results2_module, 'GetCachedResults2', return_value="")
  def testGet_WithServiceAccountUser_Legacy(self, _):
    job_module.Job.New((), ())
    job_module.Job.New((), (), user=utils.LEGACY_SERVICE_ACCOUNT)
    job = job_module.Job.New((), (), user=utils.LEGACY_SERVICE_ACCOUNT)

    data = json.loads(
        self.testapp.get('/api/jobs?o=STATE&filter=user=%s' %
                         (utils.LEGACY_SERVICE_ACCOUNT,)).body)

    self.assertEqual(2, data['count'])
    self.assertEqual(2, len(data['jobs']))
    self.assertEqual(data['jobs'][0]['user'], 'chromeperf (automation)')
    self.assertEqual(data['jobs'][1]['user'], 'chromeperf (automation)')

    self.assertFalse(data['prev'])
    self.assertFalse(data['next'])
    self.assertTrue(data['next_cursor'])
    self.assertFalse(data['prev_cursor'])

    sorted_data = sorted(data['jobs'], key=lambda d: d['job_id'])
    expected_job_dict = job.AsDict([job_module.OPTION_STATE])
    expected_job_dict['user'] = 'chromeperf (automation)'
    self.assertEqual(expected_job_dict, sorted_data[-1])

  @mock.patch.object(utils,
                     'ServiceAccountEmail', lambda: _SERVICE_ACCOUNT_EMAIL)
  @mock.patch.object(utils, 'IsStagingEnvironment', lambda: True)
  @mock.patch.object(jobs.utils, 'GetEmail',
                     mock.MagicMock(return_value=_SERVICE_ACCOUNT_EMAIL))
  @mock.patch.object(results2_module, 'GetCachedResults2', return_value="")
  def testGet_WithServiceAccountUserInStaging(self, _):
    job_module.Job.New((), (), user=_SERVICE_ACCOUNT_EMAIL)

    data = json.loads(self.testapp.get('/api/jobs?o=STATE').body)
    self.assertEqual(1, data['count'])
    self.assertEqual(1, len(data['jobs']))
    self.assertEqual(data['jobs'][0]['user'], _SERVICE_ACCOUNT_EMAIL)


  @mock.patch.object(utils,
                     'ServiceAccountEmail', lambda: _SERVICE_ACCOUNT_EMAIL)
  @mock.patch.object(utils, 'IsStagingEnvironment', lambda: False)
  @mock.patch.object(jobs.utils, 'GetEmail',
                     mock.MagicMock(return_value=_SERVICE_ACCOUNT_EMAIL))
  @mock.patch.object(results2_module, 'GetCachedResults2', return_value="")
  def testGet_WithUserAndConfig(self, _):
    expected_bot = 'some-bot'
    some_user = 'some-user@example.com'
    expected_automation_email = 'chromeperf (automation)'
    job_module.Job.New(
        (),
        (),
        user=some_user,
        arguments={'configuration': expected_bot},
    )
    job_module.Job.New((), (), user=_SERVICE_ACCOUNT_EMAIL)
    job = job_module.Job.New(
        (),
        (),
        user=_SERVICE_ACCOUNT_EMAIL,
        arguments={'configuration': expected_bot},
    )

    data = json.loads(
        self.testapp.get(
            '/api/jobs?o=STATE&filter=user={}%20AND%20configuration={}'.format(
                _SERVICE_ACCOUNT_EMAIL, expected_bot)).body)
    self.assertEqual(1, data['count'])
    self.assertEqual(1, len(data['jobs']))

    self.assertFalse(data['prev'])
    self.assertFalse(data['next'])
    self.assertTrue(data['next_cursor'])
    self.assertFalse(data['prev_cursor'])

    got_job = data['jobs'][0]
    self.assertEqual(got_job['user'], expected_automation_email)
    self.assertEqual(got_job['configuration'], expected_bot)

    sorted_data = sorted(data['jobs'], key=lambda d: d['job_id'])
    expected_job_dict = job.AsDict([job_module.OPTION_STATE])
    expected_job_dict['user'] = expected_automation_email
    self.assertEqual(expected_job_dict, sorted_data[-1])

    # Now we'll expect two jobs which have the same configuration.
    data = json.loads(
        self.testapp.get('/api/jobs?o=STATE&filter=configuration={}'.format(
            expected_bot)).body)
    self.assertEqual(2, data['count'])
    self.assertEqual(2, len(data['jobs']))

    self.assertFalse(data['prev'])
    self.assertFalse(data['next'])
    self.assertTrue(data['next_cursor'])
    self.assertFalse(data['prev_cursor'])

    sorted_data = list(sorted(data['jobs'], key=lambda d: d['job_id']))
    self.assertEqual([{k: d[k] for k in {
        'user',
        'configuration',
    }} for d in sorted_data], [{
        'configuration': expected_bot,
        'user': some_user,
    }, {
        'configuration': expected_bot,
        'user': expected_automation_email,
    }])


  @mock.patch.object(utils,
                     'ServiceAccountEmail', lambda: _SERVICE_ACCOUNT_EMAIL)
  @mock.patch.object(utils, 'IsStagingEnvironment', lambda: False)
  @mock.patch.object(jobs.utils, 'GetEmail',
                     mock.MagicMock(return_value=_SERVICE_ACCOUNT_EMAIL))
  @mock.patch.object(results2_module, 'GetCachedResults2', return_value="")
  def testGet_WithUserConfigAndComparisonMode(self, _):
    expected_bot = 'some-bot'
    some_user = 'some-user@example.com'
    expected_automation_email = 'chromeperf (automation)'
    job_module.Job.New(
        (),
        (),
        user=some_user,
        arguments={
            'configuration': expected_bot,
        },
        comparison_mode='performance',
    )
    job_module.Job.New((), (), user=_SERVICE_ACCOUNT_EMAIL)
    job = job_module.Job.New(
        (),
        (),
        user=_SERVICE_ACCOUNT_EMAIL,
        arguments={
            'configuration': expected_bot,
        },
        comparison_mode='try',
    )
    data = json.loads(
        self.testapp.get('/api/jobs?o=STATE&filter=comparison_mode=try').body)
    self.assertEqual(1, data['count'])
    self.assertEqual(1, len(data['jobs']))

    self.assertFalse(data['prev'])
    self.assertFalse(data['next'])
    self.assertTrue(data['next_cursor'])
    self.assertFalse(data['prev_cursor'])

    got_job = data['jobs'][0]
    self.assertEqual(got_job['user'], expected_automation_email)
    self.assertEqual(got_job['configuration'], expected_bot)
    self.assertEqual(got_job['comparison_mode'], 'try')

    sorted_data = sorted(data['jobs'], key=lambda d: d['job_id'])
    expected_job_dict = job.AsDict([job_module.OPTION_STATE])
    expected_job_dict['user'] = expected_automation_email
    self.assertEqual(expected_job_dict, sorted_data[-1])


  @mock.patch.object(utils,
                     'ServiceAccountEmail', lambda: _SERVICE_ACCOUNT_EMAIL)
  @mock.patch.object(utils, 'IsStagingEnvironment', lambda: False)
  @mock.patch.object(jobs.utils, 'GetEmail',
                     mock.MagicMock(return_value=_SERVICE_ACCOUNT_EMAIL))
  @mock.patch.object(results2_module, 'GetCachedResults2', return_value="")
  def testGet_WithBatchId(self, _):
    expected_bot = 'some-bot'
    some_user = 'some-user@example.com'
    job = job_module.Job.New(
        (),
        (),
        user=some_user,
        arguments={
            'configuration': expected_bot,
        },
        comparison_mode='try',
        batch_id='some-id',
    )
    job_module.Job.New((), (), user=_SERVICE_ACCOUNT_EMAIL)
    job_module.Job.New(
        (),
        (),
        user=_SERVICE_ACCOUNT_EMAIL,
        arguments={
            'configuration': expected_bot,
        },
        comparison_mode='try',
    )
    data = json.loads(
        self.testapp.get('/api/jobs?o=STATE&filter=batch_id=some-id').body)
    self.assertEqual(1, data['count'])
    self.assertEqual(1, len(data['jobs']))

    self.assertFalse(data['prev'])
    self.assertFalse(data['next'])
    self.assertTrue(data['next_cursor'])
    self.assertFalse(data['prev_cursor'])

    got_job = data['jobs'][0]
    self.assertEqual(got_job['user'], some_user)
    self.assertEqual(got_job['configuration'], expected_bot)
    self.assertEqual(got_job['comparison_mode'], 'try')
    self.assertEqual(got_job['batch_id'], 'some-id')

    sorted_data = sorted(data['jobs'], key=lambda d: d['job_id'])
    self.maxDiff = None
    expected_job_dict = job.AsDict([job_module.OPTION_STATE])
    expected_job_dict['user'] = some_user
    self.assertEqual(expected_job_dict, sorted_data[-1])


  @mock.patch.object(utils,
                     'ServiceAccountEmail', lambda: _SERVICE_ACCOUNT_EMAIL)
  @mock.patch.object(utils, 'IsStagingEnvironment', lambda: False)
  @mock.patch.object(jobs.utils, 'GetEmail',
                     mock.MagicMock(return_value=_SERVICE_ACCOUNT_EMAIL))
  @mock.patch.object(results2_module, 'GetCachedResults2', return_value="")
  @mock.patch.object(jobs, '_MAX_JOBS_TO_FETCH', 2)
  def testGet_MultiplePages(self, _):

    for i in range(5):
      job_module.Job.New((), (), user=str(i))

    data = json.loads(self.testapp.get('/api/jobs?o=STATE').body)
    self.assertEqual(5, data['count'])
    self.assertEqual(2, len(data['jobs']))
    self.assertTrue(data['next'])
    self.assertFalse(data['prev'])

    next_cursor = data['next_cursor']
    prev_cursor = data['prev_cursor']
    self.assertTrue(next_cursor)
    self.assertFalse(prev_cursor)

    jobs_ = data['jobs']
    self.assertEqual(jobs_[0]['user'], '4')
    self.assertEqual(jobs_[1]['user'], '3')

    data = json.loads(
        self.testapp.get('/api/jobs?o=STATE&next_cursor=%s' % next_cursor).body)

    self.assertEqual(5, data['count'])
    self.assertEqual(2, len(data['jobs']))

    self.assertTrue(data['next'])
    self.assertTrue(data['prev'])
    next_cursor = data['next_cursor']
    prev_cursor = data['prev_cursor']
    self.assertTrue(next_cursor)
    self.assertTrue(prev_cursor)

    jobs_ = data['jobs']
    self.assertEqual(jobs_[0]['user'], '2')
    self.assertEqual(jobs_[1]['user'], '1')

    data = json.loads(
        self.testapp.get('/api/jobs?o=STATE&next_cursor=%s' % next_cursor).body)

    self.assertEqual(5, data['count'])
    self.assertEqual(1, len(data['jobs']))

    self.assertFalse(data['next'])
    self.assertTrue(data['prev'])
    next_cursor = data['next_cursor']
    prev_cursor = data['prev_cursor']
    self.assertTrue(next_cursor)
    self.assertTrue(prev_cursor)

    jobs_ = data['jobs']
    self.assertEqual(jobs_[0]['user'], '0')

    data = json.loads(
        self.testapp.get('/api/jobs?o=STATE&prev_cursor=%s' % prev_cursor).body)

    self.assertEqual(5, data['count'])
    self.assertEqual(2, len(data['jobs']))

    self.assertTrue(data['next'])
    self.assertTrue(data['prev'])
    next_cursor = data['next_cursor']
    prev_cursor = data['prev_cursor']
    self.assertTrue(next_cursor)
    self.assertTrue(prev_cursor)

    jobs_ = data['jobs']
    self.assertEqual(jobs_[0]['user'], '2')
    self.assertEqual(jobs_[1]['user'], '1')

  @mock.patch.object(utils,
                     'ServiceAccountEmail', lambda: _SERVICE_ACCOUNT_EMAIL)
  @mock.patch.object(utils, 'IsStagingEnvironment', lambda: False)
  @mock.patch.object(jobs.utils, 'GetEmail',
                     mock.MagicMock(return_value=_SERVICE_ACCOUNT_EMAIL))
  @mock.patch.object(results2_module, 'GetCachedResults2', return_value="")
  @mock.patch.object(jobs, '_MAX_JOBS_TO_FETCH', 2)
  @mock.patch.object(jobs, '_DEFAULT_FILTERED_JOBS', 2)
  def testGet_MultiplePagesWithFilter(self, _):

    for i in range(20):
      job_module.Job.New((), (), user=str(i % 4), name=str(i // 4))

    data = json.loads(self.testapp.get('/api/jobs?filter=user=2').body)
    self.assertEqual(5, data['count'])
    self.assertEqual(2, len(data['jobs']))
    self.assertTrue(data['next'])
    self.assertFalse(data['prev'])

    next_cursor = data['next_cursor']
    prev_cursor = data['prev_cursor']
    self.assertTrue(next_cursor)
    self.assertFalse(prev_cursor)

    jobs_ = data['jobs']
    self.assertEqual(jobs_[0]['name'], '4')
    self.assertEqual(jobs_[1]['name'], '3')

    data = json.loads(
        self.testapp.get('/api/jobs?next_cursor=%s&filter=user=2' %
                         next_cursor).body)

    self.assertEqual(5, data['count'])
    self.assertEqual(2, len(data['jobs']))

    self.assertTrue(data['next'])
    self.assertTrue(data['prev'])
    next_cursor = data['next_cursor']
    prev_cursor = data['prev_cursor']
    self.assertTrue(next_cursor)
    self.assertTrue(prev_cursor)

    jobs_ = data['jobs']
    self.assertEqual(jobs_[0]['name'], '2')
    self.assertEqual(jobs_[1]['name'], '1')

    data = json.loads(
        self.testapp.get('/api/jobs?next_cursor=%s&filter=user=2' %
                         next_cursor).body)

    self.assertEqual(5, data['count'])
    self.assertEqual(1, len(data['jobs']))

    self.assertFalse(data['next'])
    self.assertTrue(data['prev'])
    next_cursor = data['next_cursor']
    prev_cursor = data['prev_cursor']
    self.assertTrue(next_cursor)
    self.assertTrue(prev_cursor)

    jobs_ = data['jobs']
    self.assertEqual(jobs_[0]['name'], '0')

    data = json.loads(
        self.testapp.get('/api/jobs?prev_cursor=%s&filter=user=2' %
                         prev_cursor).body)

    self.assertEqual(5, data['count'])
    self.assertEqual(2, len(data['jobs']))

    self.assertTrue(data['next'])
    self.assertTrue(data['prev'])
    next_cursor = data['next_cursor']
    prev_cursor = data['prev_cursor']
    self.assertTrue(next_cursor)
    self.assertTrue(prev_cursor)

    jobs_ = data['jobs']
    self.assertEqual(jobs_[0]['name'], '2')
    self.assertEqual(jobs_[1]['name'], '1')
