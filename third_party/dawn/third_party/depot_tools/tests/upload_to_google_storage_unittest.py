#!/usr/bin/env vpython3
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for upload_to_google_storage.py."""

from io import StringIO
import optparse
import os
import posixpath
import queue

import shutil
import sys
import tarfile
import tempfile
import threading
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import upload_to_google_storage
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
        self.ret_codes = queue.Queue()
        self.stdout_queue = queue.Queue()
        self.lorem_ipsum = os.path.join(self.base_path, 'lorem_ipsum.txt')
        self.lorem_ipsum_sha1 = '7871c8e24da15bad8b0be2c36edc9dc77e37727f'

    def tearDown(self):
        shutil.rmtree(self.temp_dir)
        sys.stdin = sys.__stdin__

    def test_upload_single_file(self):
        filenames = [self.lorem_ipsum]
        output_filename = '%s.sha1' % self.lorem_ipsum
        code = upload_to_google_storage.upload_to_google_storage(
            filenames, self.base_url, self.gsutil, True, False, 1, False, 'txt')
        self.assertEqual(self.gsutil.history, [
            ('check_call',
             ('ls', '%s/%s' % (self.base_url, self.lorem_ipsum_sha1))),
            ('check_call',
             ('-h', 'Cache-Control:public, max-age=31536000', 'cp', '-z', 'txt',
              filenames[0], '%s/%s' % (self.base_url, self.lorem_ipsum_sha1)))
        ])
        self.assertTrue(os.path.exists(output_filename))
        self.assertEqual(
            open(output_filename, 'rb').read().decode(),
            '7871c8e24da15bad8b0be2c36edc9dc77e37727f')
        os.remove(output_filename)
        self.assertEqual(code, 0)

    def test_create_archive(self):
        work_dir = os.path.join(self.base_path, 'download_test_data')
        with ChangedWorkingDirectory(work_dir):
            dirname = 'subfolder'
            dirs = [dirname]
            tar_gz_file = '%s.tar.gz' % dirname
            self.assertTrue(
                upload_to_google_storage.validate_archive_dirs(dirs))
            upload_to_google_storage.create_archives(dirs)
            self.assertTrue(os.path.exists(tar_gz_file))
            with tarfile.open(tar_gz_file, 'r:gz') as tar:
                content = map(lambda x: x.name, tar.getmembers())
                self.assertIn(dirname, content)
                self.assertIn(posixpath.join(dirname, 'subfolder_text.txt'),
                              content)
                self.assertIn(
                    posixpath.join(dirname, 'subfolder_text.txt.sha1'), content)

    @unittest.skipIf(sys.platform == 'win32',
                     'os.symlink does not exist on win')
    def test_validate_archive_dirs_fails(self):
        work_dir = os.path.join(self.base_path, 'download_test_data')
        with ChangedWorkingDirectory(work_dir):
            symlink = 'link'
            os.symlink(os.path.join(self.base_path, 'subfolder'), symlink)
        self.assertFalse(
            upload_to_google_storage.validate_archive_dirs([symlink]))
        self.assertFalse(
            upload_to_google_storage.validate_archive_dirs(['foobar']))

    def test_upload_single_file_remote_exists(self):
        filenames = [self.lorem_ipsum]
        output_filename = '%s.sha1' % self.lorem_ipsum
        etag_string = b'ETag: 634d7c1ed3545383837428f031840a1e'
        self.gsutil.add_expected(0, b'', b'')
        self.gsutil.add_expected(0, etag_string, b'')
        code = upload_to_google_storage.upload_to_google_storage(
            filenames, self.base_url, self.gsutil, False, False, 1, False, None)
        self.assertEqual(
            self.gsutil.history,
            [('check_call',
              ('ls', '%s/%s' % (self.base_url, self.lorem_ipsum_sha1))),
             ('check_call',
              ('ls', '-L', '%s/%s' % (self.base_url, self.lorem_ipsum_sha1)))])
        self.assertTrue(os.path.exists(output_filename))
        self.assertEqual(
            open(output_filename, 'rb').read().decode(),
            '7871c8e24da15bad8b0be2c36edc9dc77e37727f')
        os.remove(output_filename)
        self.assertEqual(code, 0)

    def test_upload_worker_errors(self):
        work_queue = queue.Queue()
        work_queue.put((self.lorem_ipsum, self.lorem_ipsum_sha1))
        work_queue.put((None, None))
        self.gsutil.add_expected(1, '', '')  # For the first ls call.
        self.gsutil.add_expected(20, '', 'Expected error message')
        # pylint: disable=protected-access
        upload_to_google_storage._upload_worker(0, work_queue,
                                                self.base_url, self.gsutil,
                                                threading.Lock(), False, False,
                                                self.stdout_queue,
                                                self.ret_codes, None)
        expected_ret_codes = [(
            20,
            'Encountered error on uploading %s to %s/%s\nExpected error message'
            % (self.lorem_ipsum, self.base_url, self.lorem_ipsum_sha1))]
        self.assertEqual(list(self.ret_codes.queue), expected_ret_codes)

    def test_skip_hashing(self):
        filenames = [self.lorem_ipsum]
        output_filename = '%s.sha1' % self.lorem_ipsum
        fake_hash = '6871c8e24da15bad8b0be2c36edc9dc77e37727f'
        with open(output_filename, 'wb') as f:
            f.write(fake_hash.encode())  # Fake hash.
        code = upload_to_google_storage.upload_to_google_storage(
            filenames, self.base_url, self.gsutil, False, False, 1, True, None)
        self.assertEqual(
            self.gsutil.history,
            [('check_call', ('ls', '%s/%s' % (self.base_url, fake_hash))),
             ('check_call', ('ls', '-L', '%s/%s' % (self.base_url, fake_hash))),
             ('check_call',
              ('-h', 'Cache-Control:public, max-age=31536000', 'cp',
               filenames[0], '%s/%s' % (self.base_url, fake_hash)))])
        self.assertEqual(open(output_filename, 'rb').read().decode(), fake_hash)
        os.remove(output_filename)
        self.assertEqual(code, 0)

    def test_get_targets_no_args(self):
        try:
            upload_to_google_storage.get_targets([], self.parser, False)
            self.fail()
        except SystemExit as e:
            self.assertEqual(e.code, 2)

    def test_get_targets_passthrough(self):
        result = upload_to_google_storage.get_targets(['a', 'b', 'c', 'd', 'e'],
                                                      self.parser, False)
        self.assertEqual(result, ['a', 'b', 'c', 'd', 'e'])

    def test_get_targets_multiple_stdin(self):
        inputs = ['a', 'b', 'c', 'd', 'e']
        sys.stdin = StringIO(os.linesep.join(inputs))
        result = upload_to_google_storage.get_targets(['-'], self.parser, False)
        self.assertEqual(result, inputs)

    def test_get_targets_multiple_stdin_null(self):
        inputs = ['a', 'b', 'c', 'd', 'e']
        sys.stdin = StringIO('\0'.join(inputs))
        result = upload_to_google_storage.get_targets(['-'], self.parser, True)
        self.assertEqual(result, inputs)


if __name__ == '__main__':
    unittest.main()
