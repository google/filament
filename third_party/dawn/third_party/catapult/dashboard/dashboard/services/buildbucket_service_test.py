# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import unittest

from unittest import mock

from dashboard.services import buildbucket_service

_BUILD_PARAMETERS = {
    'builder_name': 'dummy_builder',
    'properties': {
        'bisect_config': {},
        'patch_project': 'patch_project_x'
    }
}


def _mock_uuid(): # pylint: disable=invalid-name
  return 'mock uuid'

class BuildbucketServiceTest(unittest.TestCase):

  def setUp(self):
    patcher = mock.patch('dashboard.services.request.RequestJson')
    self._request_json = patcher.start()
    self.addCleanup(patcher.stop)

    self._request_json.return_value = {'build': {'id': 'build id'}}

  def _AssertCorrectResponse(self, content):
    self.assertEqual(content, {'build': {'id': 'build id'}})


  def _AssertRequestMadeOnce(self, path, *args, **kwargs):
    self._request_json.assert_called_once_with(
        buildbucket_service.API_BASE_URL2 + path, *args, **kwargs)


  def testPut_badBucketName(self):
    self.assertRaises(ValueError, buildbucket_service.Put,
                      'invalid bucket string', [''], _BUILD_PARAMETERS)


  @mock.patch('uuid.uuid4', _mock_uuid)
  def testPut(self):
    mock_hash = '1234567890123456789012345678901234567890'
    gitile_buildset = 'buildset:commit/gitiles/host/project/name/+/' + mock_hash
    patch_buildset = 'buildset:patch/gerrit/host/7654321/8'
    patch_buildset_2 = 'buildset:patch/gerrit/host/8765432/9'
    expected_body = {
        'requestId': 'mock uuid',
        'builder': {
            'project': 'chrome',
            'bucket': 'bucket_name',
            'builder': _BUILD_PARAMETERS['builder_name'],
        },
        'tags': [{
            'key': 'foo',
            'value': 'bar'
        }],
        'properties': _BUILD_PARAMETERS.get('properties', {}),
        'gerritChanges': [{
            "host": 'host',
            "change": '7654321',
            "patchset": '8',
            "project": 'patch_project_x'
        }, {
            "host": 'host',
            "change": '8765432',
            "patchset": '9',
            "project": 'patch_project_x'
        }],
        'gitilesCommit': {
            "host": 'host',
            "project": 'project/name',
            "id": mock_hash,
            "ref": "refs/heads/main"
        }
    }
    tags = [gitile_buildset, patch_buildset, patch_buildset_2, 'foo:bar']
    response = buildbucket_service.Put('luci.chrome.bucket_name', tags,
                                       _BUILD_PARAMETERS)
    self._AssertCorrectResponse(response)
    self._AssertRequestMadeOnce(
        'ScheduleBuild', method='POST', body=expected_body)

  def testGetJobStatus(self):
    response = buildbucket_service.GetJobStatus('job_id')
    self._AssertCorrectResponse(response)
    expected_body = {
        'id': 'job_id',
    }
    self._AssertRequestMadeOnce('GetBuild', method='POST', body=expected_body)


class FakeJob:

  def GetBuildParameters(self):
    return _BUILD_PARAMETERS


if __name__ == '__main__':
  unittest.main()
