#!/usr/bin/env vpython3
# Copyright (c) 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for upload_to_google_storage_first_class.py."""

from io import StringIO
import optparse
import os
import posixpath

import shutil
import sys
import tarfile
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import upload_to_google_storage_first_class
from download_from_google_storage_unittest import GsutilMock
from download_from_google_storage_unittest import ChangedWorkingDirectory

# ../third_party/gsutil/gsutil
GSUTIL_DEFAULT_PATH = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'third_party',
    'gsutil', 'gsutil')
TEST_DIR = os.path.dirname(os.path.abspath(__file__))


class UploadTests(unittest.TestCase):

    def setUp(self):
        self.gsutil = GsutilMock(GSUTIL_DEFAULT_PATH, None)
        self.temp_dir = tempfile.mkdtemp(prefix='gstools_test')
        self.base_path = os.path.join(self.temp_dir, 'gstools')
        shutil.copytree(os.path.join(TEST_DIR, 'gstools'), self.base_path)
        self.base_url = 'gs://sometesturl'
        self.parser = optparse.OptionParser()
        self.lorem_ipsum = os.path.join(self.base_path, 'lorem_ipsum.txt')

    def tearDown(self):
        shutil.rmtree(self.temp_dir)
        sys.stdin = sys.__stdin__

    def test_upload_single_file_missing_generation(self):
        file = self.lorem_ipsum
        object_name = 'gs_object_name/version123'

        self.gsutil.add_expected(0, "", "")  # ls call
        self.gsutil.add_expected(0, "", 'weird output')  # cp call

        actual = upload_to_google_storage_first_class.upload_to_google_storage(
            file=file,
            base_url=self.base_url,
            object_name=object_name,
            gsutil=self.gsutil,
            force=True,
            gzip=False,
            dry_run=False)

        self.assertEqual(
            self.gsutil.history,
            [('check_call', ('ls', '%s/%s' % (self.base_url, object_name))),
             ('check_call',
              ('-h', 'Cache-Control:public, max-age=31536000', 'cp', '-v', file,
               '%s/%s' % (self.base_url, object_name)))])
        self.assertEqual(
            actual, upload_to_google_storage_first_class.MISSING_GENERATION_MSG)

    def test_upload_single_file(self):
        file = self.lorem_ipsum
        object_name = 'gs_object_name/version123'

        self.gsutil.add_expected(0, "", "")  # ls call
        expected_generation = '1712070862651948'
        expected_cp_status = (
            f'Copying file://{file} [Content-Type=text/plain].\n'
            f'Created: {self.base_url}/{object_name}#{expected_generation}\n'
            'Operation completed over 1 objects/8.0 B.')
        self.gsutil.add_expected(0, "", expected_cp_status)  # cp call

        actual = upload_to_google_storage_first_class.upload_to_google_storage(
            file=file,
            base_url=self.base_url,
            object_name=object_name,
            gsutil=self.gsutil,
            force=True,
            gzip=False,
            dry_run=False)

        self.assertEqual(
            self.gsutil.history,
            [('check_call', ('ls', '%s/%s' % (self.base_url, object_name))),
             ('check_call',
              ('-h', 'Cache-Control:public, max-age=31536000', 'cp', '-v', file,
               '%s/%s' % (self.base_url, object_name)))])
        self.assertEqual(actual, expected_generation)

    def test_upload_single_file_remote_exists(self):
        file = self.lorem_ipsum
        object_name = 'gs_object_name/version123'
        etag_string = 'ETag: 634d7c1ed3545383837428f031840a1e'
        self.gsutil.add_expected(0, b'', b'')
        self.gsutil.add_expected(0, etag_string, b'')

        with self.assertRaises(Exception):
            upload_to_google_storage_first_class.upload_to_google_storage(
                file=file,
                base_url=self.base_url,
                object_name=object_name,
                gsutil=self.gsutil,
                force=False,
                gzip=False,
                dry_run=False)

        self.assertEqual(self.gsutil.history,
                         [('check_call',
                           ('ls', '%s/%s' % (self.base_url, object_name))),
                          ('check_call', ('ls', '-L', '%s/%s' %
                                          (self.base_url, object_name)))])

    def test_create_archive(self):
        work_dir = os.path.join(self.base_path, 'download_test_data')
        with ChangedWorkingDirectory(work_dir):
            dirname = 'subfolder'
            dirs = [dirname]
            self.assertTrue(
                upload_to_google_storage_first_class.validate_archive_dirs(
                    dirs))
            tar_filename = upload_to_google_storage_first_class.create_archive(
                dirs)
            with tarfile.open(tar_filename, 'r:gz') as tar:
                content = map(lambda x: x.name, tar.getmembers())
                self.assertIn(dirname, content)

    def test_create_archive_multiple_dirs(self):
        work_dir = os.path.join(self.base_path, 'download_test_data')
        with ChangedWorkingDirectory(work_dir):
            dirs = ['subfolder', 'subfolder2']
            self.assertTrue(
                upload_to_google_storage_first_class.validate_archive_dirs(
                    dirs))
            tar_filename = upload_to_google_storage_first_class.create_archive(
                dirs)
            with tarfile.open(tar_filename, 'r:gz') as tar:
                content = map(lambda x: x.name, tar.getmembers())
                for dirname in dirs:
                    self.assertIn(dirname, content)

    @unittest.skipIf(sys.platform == 'win32',
                     'os.symlink does not exist on win')
    def test_validate_archive_dirs_fails(self):
        work_dir = os.path.join(self.base_path, 'download_test_data')
        with ChangedWorkingDirectory(work_dir):
            symlink = 'link'
            os.symlink(os.path.join(self.base_path, 'subfolder'), symlink)
        self.assertFalse(
            upload_to_google_storage_first_class.validate_archive_dirs(
                [symlink]))
        self.assertFalse(
            upload_to_google_storage_first_class.validate_archive_dirs(
                ['foobar']))

    def test_dry_run(self):
        file = self.lorem_ipsum
        object_name = 'gs_object_name/version123'

        upload_to_google_storage_first_class.upload_to_google_storage(
            file=file,
            base_url=self.base_url,
            object_name=object_name,
            gsutil=self.gsutil,
            force=False,
            gzip=False,
            dry_run=True)

        self.assertEqual(self.gsutil.history,
                         [('check_call',
                           ('ls', '%s/%s' % (self.base_url, object_name))),
                          ('check_call', ('ls', '-L', '%s/%s' %
                                          (self.base_url, object_name)))])

    def test_get_targets_no_args(self):
        try:
            upload_to_google_storage_first_class.get_targets([], self.parser,
                                                             False)
            self.fail()
        except SystemExit as e:
            self.assertEqual(e.code, 2)

    def test_get_targets_passthrough(self):
        result = upload_to_google_storage_first_class.get_targets(
            ['a', 'b', 'c', 'd', 'e'], self.parser, False)
        self.assertEqual(result, ['a', 'b', 'c', 'd', 'e'])

    def test_get_targets_multiple_stdin(self):
        inputs = ['a', 'b', 'c', 'd', 'e']
        sys.stdin = StringIO(os.linesep.join(inputs))
        result = upload_to_google_storage_first_class.get_targets(['-'],
                                                                  self.parser,
                                                                  False)
        self.assertEqual(result, inputs)

    def test_get_targets_multiple_stdin_null(self):
        inputs = ['a', 'b', 'c', 'd', 'e']
        sys.stdin = StringIO('\0'.join(inputs))
        result = upload_to_google_storage_first_class.get_targets(['-'],
                                                                  self.parser,
                                                                  True)
        self.assertEqual(result, inputs)


if __name__ == '__main__':
    unittest.main()
