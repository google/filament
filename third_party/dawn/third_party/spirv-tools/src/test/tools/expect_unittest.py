# Copyright (c) 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Tests for the expect module."""

import expect
from spirv_test_framework import TestStatus
import re
import unittest


class TestStdoutMatchADotC(expect.StdoutMatch):
    expected_stdout = re.compile('a.c')


class TestExpect(unittest.TestCase):
    def test_get_object_name(self):
        """Tests get_object_filename()."""
        source_and_object_names = [('a.vert', 'a.vert.spv'),
                                   ('b.frag', 'b.frag.spv'),
                                   ('c.tesc', 'c.tesc.spv'),
                                   ('d.tese', 'd.tese.spv'),
                                   ('e.geom', 'e.geom.spv'),
                                   ('f.comp', 'f.comp.spv'),
                                   ('file', 'file.spv'), ('file.', 'file.spv'),
                                   ('file.uk',
                                    'file.spv'), ('file.vert.',
                                                  'file.vert.spv'),
                                   ('file.vert.bla',
                                    'file.vert.spv')]
        actual_object_names = [
            expect.get_object_filename(f[0]) for f in source_and_object_names
        ]
        expected_object_names = [f[1] for f in source_and_object_names]

        self.assertEqual(actual_object_names, expected_object_names)

    def test_stdout_match_regex_has_match(self):
        test = TestStdoutMatchADotC()
        status = TestStatus(
            test_manager=None,
            returncode=0,
            stdout=b'0abc1',
            stderr=None,
            directory=None,
            inputs=None,
            input_filenames=None)
        self.assertTrue(test.check_stdout_match(status)[0])

    def test_stdout_match_regex_no_match(self):
        test = TestStdoutMatchADotC()
        status = TestStatus(
            test_manager=None,
            returncode=0,
            stdout=b'ab',
            stderr=None,
            directory=None,
            inputs=None,
            input_filenames=None)
        self.assertFalse(test.check_stdout_match(status)[0])

    def test_stdout_match_regex_empty_stdout(self):
        test = TestStdoutMatchADotC()
        status = TestStatus(
            test_manager=None,
            returncode=0,
            stdout=b'',
            stderr=None,
            directory=None,
            inputs=None,
            input_filenames=None)
        self.assertFalse(test.check_stdout_match(status)[0])
