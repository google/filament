# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
from __future__ import absolute_import
import json
import os
import shutil
import tempfile
import unittest
from unittest import mock
import six

from py_utils import cloud_storage  # pylint: disable=import-error

from telemetry.page import page
from telemetry.wpr import archive_info


class MockPage(page.Page):
  def __init__(self, url, name=None, platform_specific=False):
    super().__init__(url, None, name=name)
    self._platform_specific = platform_specific

page1 = MockPage('http://www.page1.com/', 'Page1')
page2 = MockPage('http://www.page2.com/', 'Page2', platform_specific=True)
page3 = MockPage('http://www.page3.com/', 'Page3', platform_specific=True)
pageNew1 = MockPage('http://www.new1.com/', 'New1')
pageNew2 = MockPage('http://www.new2.com/', 'New2', platform_specific=True)
recording1 = 'data_abcdef0001.wprgo'
recording2 = 'data_abcdef0002.wprgo'
recording3 = 'data_abcdef0003.wprgo'
recording4 = 'data_abcdef0004.wprgo'
recording5 = 'data_abcdef0005.wprgo'
_DEFAULT_PLATFORM = archive_info._DEFAULT_PLATFORM

default_archives_info_contents_dict = {
    "platform_specific": True,
    "archives": {
        "Page1": {
            _DEFAULT_PLATFORM: recording1
        },
        "Page2": {
            _DEFAULT_PLATFORM: recording2
        },
        "Page3": {
            _DEFAULT_PLATFORM: recording1,
            "win": recording2,
            "mac": recording3,
            "linux": recording4,
            "android": recording5
        }
    }
}

default_archive_info_contents = json.dumps(default_archives_info_contents_dict)
default_wpr_files = [
    'data_abcdef0001.wprgo', 'data_abcdef0002.wprgo', 'data_abcdef0003.wprgo',
    'data_abcdef0004.wprgo', 'data_abcdef0005.wprgo']
_BASE_ARCHIVE = {
    u'platform_specific': True,
    u'description': (u'Describes the Web Page Replay archives for a'
                     u' story set. Don\'t edit by hand! Use record_wpr for'
                     u' updating.'),
    u'archives': {},
}


class WprArchiveInfoTest(unittest.TestCase):
  def setUp(self):
    self.tmp_dir = tempfile.mkdtemp()
    # Set file for the metadata.
    self.story_set_archive_info_file = os.path.join(
        self.tmp_dir, 'story_name.json')
    self.maxDiff = None

  def tearDown(self):
    shutil.rmtree(self.tmp_dir)

  def createArchiveInfo(
      self, archive_data=default_archive_info_contents,
      cloud_storage_bucket=cloud_storage.PUBLIC_BUCKET, wpr_files=None):

    # Cannot set lists as a default parameter, so doing it this way.
    if wpr_files is None:
      wpr_files = default_wpr_files

    with open(self.story_set_archive_info_file, 'w') as f:
      f.write(archive_data)

    assert isinstance(wpr_files, list)
    for wpr_file in wpr_files:
      assert isinstance(wpr_file, six.string_types)
      with open(os.path.join(self.tmp_dir, wpr_file), 'w') as f:
        f.write(archive_data)
    return archive_info.WprArchiveInfo.FromFile(
        self.story_set_archive_info_file, cloud_storage_bucket)

  def testInitNotPlatformSpecific(self):
    with open(self.story_set_archive_info_file, 'w') as f:
      f.write('{}')
    with self.assertRaises(AssertionError):
      self.createArchiveInfo(archive_data='{}')


  @mock.patch('telemetry.wpr.archive_info.cloud_storage.GetIfChanged')
  def testDownloadArchivesIfNeededAllOrOneNeeded(self, get_mock):
    test_archive_info = self.createArchiveInfo()

    test_archive_info.DownloadArchivesIfNeeded()
    expected_recordings = [
        recording1, recording1, recording2, recording2, recording3, recording4,
        recording5
    ]
    expected_calls = [
        mock.call(os.path.join(self.tmp_dir, r), cloud_storage.PUBLIC_BUCKET)
        for r in expected_recordings
    ]
    get_mock.assert_has_calls(expected_calls, any_order=True)

  @mock.patch('telemetry.wpr.archive_info.cloud_storage.GetIfChanged')
  def testDownloadArchivesIfNeededWithStoryNames(self, get_mock):
    test_archive_info = self.createArchiveInfo()
    story_names = ['Page1', 'Page2', 'Not_used']

    test_archive_info.DownloadArchivesIfNeeded(story_names=story_names)
    expected_calls = [
        mock.call(os.path.join(self.tmp_dir, r), cloud_storage.PUBLIC_BUCKET)
        for r in (recording1, recording2)
    ]
    get_mock.assert_has_calls(expected_calls, any_order=True)
    self.assertEqual(get_mock.call_count, 2)

  @mock.patch('telemetry.wpr.archive_info.cloud_storage.GetIfChanged')
  def testDownloadArchivesIfNeededNonDefault(self, get_mock):
    data = {
        'platform_specific': True,
        'archives': {
            'http://www.page3.com/': {
                _DEFAULT_PLATFORM: 'data_abcdef0001.wprgo',
                'win': 'data_abcdef0002.wprgo',
                'linux': 'data_abcdef0004.wprgo',
                'mac': 'data_abcdef0003.wprgo',
                'android': 'data_abcdef0005.wprgo'},
            'Page1': {_DEFAULT_PLATFORM: 'data_abcdef0003.wprgo'},
            'Page2': {_DEFAULT_PLATFORM: 'data_abcdef0002.wprgo'}
        }
    }
    test_archive_info = self.createArchiveInfo(
        archive_data=json.dumps(data, separators=(',', ': ')))

    test_archive_info.DownloadArchivesIfNeeded(target_platforms=['linux'])
    expected_calls = [
        mock.call(os.path.join(self.tmp_dir, r), cloud_storage.PUBLIC_BUCKET)
        for r in (recording1, recording2, recording3, recording4)
    ]
    get_mock.assert_has_calls(expected_calls, any_order=True)

  @mock.patch('telemetry.wpr.archive_info.cloud_storage.GetIfChanged')
  def testDownloadArchivesIfNeededNoBucket(self, get_mock):
    test_archive_info = self.createArchiveInfo(cloud_storage_bucket=None)

    test_archive_info.DownloadArchivesIfNeeded()
    self.assertEqual(get_mock.call_count, 0)

  def testWprFilePathForStoryDefault(self):
    test_archive_info = self.createArchiveInfo()
    self.assertEqual(
        test_archive_info.WprFilePathForStory(page1),
        os.path.join(self.tmp_dir, recording1))
    self.assertEqual(
        test_archive_info.WprFilePathForStory(page2),
        os.path.join(self.tmp_dir, recording2))
    self.assertEqual(
        test_archive_info.WprFilePathForStory(page3),
        os.path.join(self.tmp_dir, recording1))

  def testWprFilePathForStoryMac(self):
    test_archive_info = self.createArchiveInfo()
    self.assertEqual(test_archive_info.WprFilePathForStory(page1, 'mac'),
                     os.path.join(self.tmp_dir, recording1))
    self.assertEqual(test_archive_info.WprFilePathForStory(page2, 'mac'),
                     os.path.join(self.tmp_dir, recording2))
    self.assertEqual(test_archive_info.WprFilePathForStory(page3, 'mac'),
                     os.path.join(self.tmp_dir, recording3))

  def testWprFilePathForStoryWin(self):
    test_archive_info = self.createArchiveInfo()
    self.assertEqual(test_archive_info.WprFilePathForStory(page1, 'win'),
                     os.path.join(self.tmp_dir, recording1))
    self.assertEqual(test_archive_info.WprFilePathForStory(page2, 'win'),
                     os.path.join(self.tmp_dir, recording2))
    self.assertEqual(test_archive_info.WprFilePathForStory(page3, 'win'),
                     os.path.join(self.tmp_dir, recording2))

  def testWprFilePathForStoryAndroid(self):
    test_archive_info = self.createArchiveInfo()
    self.assertEqual(test_archive_info.WprFilePathForStory(page1, 'android'),
                     os.path.join(self.tmp_dir, recording1))
    self.assertEqual(test_archive_info.WprFilePathForStory(page2, 'android'),
                     os.path.join(self.tmp_dir, recording2))
    self.assertEqual(test_archive_info.WprFilePathForStory(page3, 'android'),
                     os.path.join(self.tmp_dir, recording5))

  def testWprFilePathForStoryLinux(self):
    test_archive_info = self.createArchiveInfo()
    self.assertEqual(test_archive_info.WprFilePathForStory(page1, 'linux'),
                     os.path.join(self.tmp_dir, recording1))
    self.assertEqual(test_archive_info.WprFilePathForStory(page2, 'linux'),
                     os.path.join(self.tmp_dir, recording2))
    self.assertEqual(test_archive_info.WprFilePathForStory(page3, 'linux'),
                     os.path.join(self.tmp_dir, recording4))

  def testWprFilePathForStoryBadStory(self):
    test_archive_info = self.createArchiveInfo()
    self.assertIsNone(test_archive_info.WprFilePathForStory(pageNew1))


  def testAddRecordedStoriesNoStories(self):
    test_archive_info = self.createArchiveInfo()
    old_data = test_archive_info._data.copy()
    test_archive_info.AddNewTemporaryRecording()
    test_archive_info.AddRecordedStories(None)
    self.assertDictEqual(old_data, test_archive_info._data)

  def assertWprFileDoesNotExist(self, file_name):
    sha_file = file_name + '.sha1'
    self.assertFalse(os.path.isfile(os.path.join(self.tmp_dir, sha_file)))
    self.assertFalse(os.path.isfile(os.path.join(self.tmp_dir, file_name)))

  def assertWprFileDoesExist(self, file_name):
    sha_file = file_name + '.sha1'
    self.assertTrue(os.path.isfile(os.path.join(self.tmp_dir, sha_file)))
    self.assertTrue(os.path.isfile(os.path.join(self.tmp_dir, file_name)))

  @mock.patch(
      'telemetry.wpr.archive_info.cloud_storage.CalculateHash',
      return_value='abcdef0006deadcode')
  def testAddRecordedStoriesDefault(self, hash_mock):
    test_archive_info = self.createArchiveInfo()
    self.assertWprFileDoesNotExist('story_name_abcdef0006.wprgo')

    new_temp_recording = os.path.join(self.tmp_dir, 'recording.wprgo')

    with open(new_temp_recording, 'w') as f:
      f.write('wpr data')

    test_archive_info.AddNewTemporaryRecording(new_temp_recording)
    test_archive_info.AddRecordedStories([page2, page3])

    with open(self.story_set_archive_info_file, 'r') as f:
      archive_file_contents = json.load(f)

    expected_archive_contents = _BASE_ARCHIVE.copy()
    expected_archive_contents['archives'] = {
        page1.name: {
            _DEFAULT_PLATFORM: recording1
        },
        page2.name: {
            _DEFAULT_PLATFORM: 'story_name_abcdef0006.wprgo'
        },
        page3.name: {
            _DEFAULT_PLATFORM: 'story_name_abcdef0006.wprgo',
            'linux': recording4,
            'mac': recording3,
            'win': recording2,
            'android': recording5
        }
    }

    self.assertDictEqual(expected_archive_contents, archive_file_contents)
    # Ensure the saved JSON does not contain trailing spaces.
    with open(self.story_set_archive_info_file) as f:
      for line in f:
        self.assertFalse(line.rstrip('\n').endswith(' '))
    self.assertWprFileDoesExist('story_name_abcdef0006.wprgo')
    self.assertEqual(hash_mock.call_count, 1)
    hash_mock.assert_called_with(new_temp_recording)

  @mock.patch(
      'telemetry.wpr.archive_info.cloud_storage.CalculateHash',
      return_value='abcdef0006deadcode')
  def testAddRecordedStoriesNotDefault(self, hash_mock):
    test_archive_info = self.createArchiveInfo()
    self.assertWprFileDoesNotExist('story_name_abcdef0006.wprgo')
    new_temp_recording = os.path.join(self.tmp_dir, 'recording.wprgo')

    with open(new_temp_recording, 'w') as f:
      f.write('wpr data')
    test_archive_info.AddNewTemporaryRecording(new_temp_recording)
    test_archive_info.AddRecordedStories([page2, page3],
                                         target_platform='android')

    with open(self.story_set_archive_info_file, 'r') as f:
      archive_file_contents = json.load(f)

    expected_archive_contents = _BASE_ARCHIVE.copy()
    expected_archive_contents['archives'] = {
        page1.name: {
            _DEFAULT_PLATFORM: recording1
        },
        page2.name: {
            _DEFAULT_PLATFORM: recording2,
            'android': 'story_name_abcdef0006.wprgo'
        },
        page3.name: {
            _DEFAULT_PLATFORM: recording1,
            'linux': recording4,
            'mac': recording3,
            'win': recording2,
            'android': 'story_name_abcdef0006.wprgo'
        },
    }

    self.assertDictEqual(expected_archive_contents, archive_file_contents)
    # Ensure the saved JSON does not contain trailing spaces.
    with open(self.story_set_archive_info_file) as f:
      for line in f:
        self.assertFalse(line.rstrip('\n').endswith(' '))
    self.assertWprFileDoesExist('story_name_abcdef0006.wprgo')
    self.assertEqual(hash_mock.call_count, 1)
    hash_mock.assert_called_with(new_temp_recording)

  def testAddRecordedStoriesNewPage(self):
    test_archive_info = self.createArchiveInfo()
    self.assertWprFileDoesNotExist('story_name_abcdef0006.wprgo')
    self.assertWprFileDoesNotExist('story_name_abcdef0007.wprgo')
    new_temp_recording = os.path.join(self.tmp_dir, 'recording.wprgo')

    with mock.patch('telemetry.wpr.archive_info.cloud_storage.CalculateHash',
                    return_value='abcdef0006deadcode') as hash_mock:
      with open(new_temp_recording, 'w') as f:
        f.write('wpr data')
      test_archive_info.AddNewTemporaryRecording(new_temp_recording)
      self.assertEqual(hash_mock.call_count, 0)
      test_archive_info.AddRecordedStories([pageNew1])
      hash_mock.assert_any_call(new_temp_recording)
      self.assertEqual(hash_mock.call_count, 1)

    with mock.patch('telemetry.wpr.archive_info.cloud_storage.CalculateHash',
                    return_value='abcdef0007deadcode') as hash_mock:
      with open(new_temp_recording, 'w') as f:
        f.write('wpr data2')
      test_archive_info.AddNewTemporaryRecording(new_temp_recording)
      self.assertEqual(hash_mock.call_count, 0)
      test_archive_info.AddRecordedStories([pageNew2],
                                           target_platform='android')
      hash_mock.assert_any_call(new_temp_recording)
      self.assertEqual(hash_mock.call_count, 1)

    with open(self.story_set_archive_info_file, 'r') as f:
      archive_file_contents = json.load(f)

    expected_archive_contents = _BASE_ARCHIVE.copy()
    expected_archive_contents['archives'] = {
        page1.name: {
            _DEFAULT_PLATFORM: recording1
        },
        page2.name: {
            _DEFAULT_PLATFORM: recording2,
        },
        page3.name: {
            _DEFAULT_PLATFORM: recording1,
            'linux': recording4,
            'mac': recording3,
            'win': recording2,
            'android': recording5
        },
        pageNew1.name: {
            _DEFAULT_PLATFORM: 'story_name_abcdef0006.wprgo'
        },
        pageNew2.name: {
            _DEFAULT_PLATFORM: 'story_name_abcdef0007.wprgo',
            'android': 'story_name_abcdef0007.wprgo'
        }
    }

    self.assertDictEqual(expected_archive_contents, archive_file_contents)
    # Ensure the saved JSON does not contain trailing spaces.
    with open(self.story_set_archive_info_file) as f:
      for line in f:
        self.assertFalse(line.rstrip('\n').endswith(' '))
    self.assertWprFileDoesExist('story_name_abcdef0006.wprgo')
    self.assertWprFileDoesExist('story_name_abcdef0007.wprgo')

  def testRemoveStory(self):
    data = {
        'platform_specific': True,
        'archives': {
            'http://www.page3.com/': {
                _DEFAULT_PLATFORM: 'data_abcdef0001.wprgo',
                'win': 'data_abcdef0002.wprgo',
                'linux': 'data_abcdef0004.wprgo',
                'mac': 'data_abcdef0003.wprgo',
                'android': 'data_abcdef0005.wprgo'},
            'Page1': {_DEFAULT_PLATFORM: 'data_abcdef0003.wprgo'},
            'Page2': {_DEFAULT_PLATFORM: 'data_abcdef0002.wprgo'}
        }
    }

    expected_data = _BASE_ARCHIVE.copy()
    expected_data["archives"] = {
        'Page1': {_DEFAULT_PLATFORM: 'data_abcdef0003.wprgo'},
        'Page2': {_DEFAULT_PLATFORM: 'data_abcdef0002.wprgo'}
    }

    test_archive_info = self.createArchiveInfo(
        archive_data=json.dumps(data, separators=(',', ': ')))
    test_archive_info.RemoveStory('http://www.page3.com/')

    with open(self.story_set_archive_info_file, 'r') as f:
      archive_file_contents = json.load(f)
      self.assertDictEqual(expected_data, archive_file_contents)
