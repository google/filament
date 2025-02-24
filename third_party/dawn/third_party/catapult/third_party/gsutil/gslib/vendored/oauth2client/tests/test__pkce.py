# Copyright 2016 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import unittest

import mock

from oauth2client import _pkce


class PKCETests(unittest.TestCase):

    @mock.patch('oauth2client._pkce.os.urandom')
    def test_verifier(self, fake_urandom):
        canned_randomness = (
            b'\x98\x10D7\xf3\xb7\xaa\xfc\xdd\xd3M\xe2'
            b'\xa3,\x06\xa0\xb0\xa9\xb4\x8f\xcb\xd0'
            b'\xf5\x86N2p\x8c]!W\x9a\xed54\x99\x9d'
            b'\x8dv\\\xa7/\x81\xf3J\x98\xc3\x90\xee'
            b'\xb0\x8c\xb7Zc#\x05M0O\x08\xda\t\x1f\x07'
        )
        fake_urandom.return_value = canned_randomness
        expected = (
            b'mBBEN_O3qvzd003ioywGoLCptI_L0PWGTjJwjF0hV5rt'
            b'NTSZnY12XKcvgfNKmMOQ7rCMt1pjIwVNME8I2gkfBw'
        )
        result = _pkce.code_verifier()
        self.assertEqual(result, expected)

    def test_verifier_too_long(self):
        with self.assertRaises(ValueError) as caught:
            _pkce.code_verifier(97)
        self.assertIn("too long", str(caught.exception))

    def test_verifier_too_short(self):
        with self.assertRaises(ValueError) as caught:
            _pkce.code_verifier(30)
        self.assertIn("too short", str(caught.exception))

    def test_challenge(self):
        result = _pkce.code_challenge(b'SOME_VERIFIER')
        expected = b'6xJCQsjTtS3zjUwd8_ZqH0SyviGHnp5PsHXWKOCqDuI'
        self.assertEqual(result, expected)
