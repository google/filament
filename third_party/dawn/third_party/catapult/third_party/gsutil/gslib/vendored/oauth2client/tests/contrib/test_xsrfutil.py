# Copyright 2014 Google Inc. All rights reserved.
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

"""Tests for oauth2client.contrib.xsrfutil."""

import base64
import unittest

import mock

from oauth2client import _helpers
from oauth2client.contrib import xsrfutil

# Jan 17 2008, 5:40PM
TEST_KEY = b'test key'
# Jan. 17, 2008 22:40:32.081230 UTC
TEST_TIME = 1200609642081230
TEST_USER_ID_1 = 123832983
TEST_USER_ID_2 = 938297432
TEST_ACTION_ID_1 = b'some_action'
TEST_ACTION_ID_2 = b'some_other_action'
TEST_EXTRA_INFO_1 = b'extra_info_1'
TEST_EXTRA_INFO_2 = b'more_extra_info'


class Test_generate_token(unittest.TestCase):

    def test_bad_positional(self):
        # Need 2 positional arguments.
        with self.assertRaises(TypeError):
            xsrfutil.generate_token(None)
        # At most 2 positional arguments.
        with self.assertRaises(TypeError):
            xsrfutil.generate_token(None, None, None)

    def test_it(self):
        digest = b'foobar'
        digester = mock.Mock()
        digester.digest = mock.Mock(name='digest', return_value=digest)
        with mock.patch('oauth2client.contrib.xsrfutil.hmac') as hmac:
            hmac.new = mock.Mock(name='new', return_value=digester)
            token = xsrfutil.generate_token(TEST_KEY,
                                            TEST_USER_ID_1,
                                            action_id=TEST_ACTION_ID_1,
                                            when=TEST_TIME)
            hmac.new.assert_called_once_with(TEST_KEY)
            digester.digest.assert_called_once_with()

            expected_digest_calls = [
                mock.call.update(_helpers._to_bytes(str(TEST_USER_ID_1))),
                mock.call.update(xsrfutil.DELIMITER),
                mock.call.update(TEST_ACTION_ID_1),
                mock.call.update(xsrfutil.DELIMITER),
                mock.call.update(_helpers._to_bytes(str(TEST_TIME))),
            ]
            self.assertEqual(digester.method_calls, expected_digest_calls)

            expected_token_as_bytes = (digest + xsrfutil.DELIMITER +
                                       _helpers._to_bytes(str(TEST_TIME)))
            expected_token = base64.urlsafe_b64encode(
                expected_token_as_bytes)
            self.assertEqual(token, expected_token)

    def test_with_system_time(self):
        digest = b'foobar'
        curr_time = 1440449755.74
        digester = mock.Mock()
        digester.digest = mock.Mock(name='digest', return_value=digest)
        with mock.patch('oauth2client.contrib.xsrfutil.hmac') as hmac:
            hmac.new = mock.Mock(name='new', return_value=digester)

            with mock.patch('oauth2client.contrib.xsrfutil.time') as time:
                time.time = mock.Mock(name='time', return_value=curr_time)
                # when= is omitted
                token = xsrfutil.generate_token(TEST_KEY,
                                                TEST_USER_ID_1,
                                                action_id=TEST_ACTION_ID_1)

                hmac.new.assert_called_once_with(TEST_KEY)
                time.time.assert_called_once_with()
                digester.digest.assert_called_once_with()

                expected_digest_calls = [
                    mock.call.update(_helpers._to_bytes(str(TEST_USER_ID_1))),
                    mock.call.update(xsrfutil.DELIMITER),
                    mock.call.update(TEST_ACTION_ID_1),
                    mock.call.update(xsrfutil.DELIMITER),
                    mock.call.update(_helpers._to_bytes(str(int(curr_time)))),
                ]
                self.assertEqual(digester.method_calls, expected_digest_calls)

                expected_token_as_bytes = (
                    digest + xsrfutil.DELIMITER +
                    _helpers._to_bytes(str(int(curr_time))))
                expected_token = base64.urlsafe_b64encode(
                    expected_token_as_bytes)
                self.assertEqual(token, expected_token)


class Test_validate_token(unittest.TestCase):

    def test_bad_positional(self):
        # Need 3 positional arguments.
        with self.assertRaises(TypeError):
            xsrfutil.validate_token(None, None)
        # At most 3 positional arguments.
        with self.assertRaises(TypeError):
            xsrfutil.validate_token(None, None, None, None)

    def test_no_token(self):
        key = token = user_id = None
        self.assertFalse(xsrfutil.validate_token(key, token, user_id))

    def test_token_not_valid_base64(self):
        key = user_id = None
        token = b'a'  # Bad padding
        self.assertFalse(xsrfutil.validate_token(key, token, user_id))

    def test_token_non_integer(self):
        key = user_id = None
        token = base64.b64encode(b'abc' + xsrfutil.DELIMITER + b'xyz')
        self.assertFalse(xsrfutil.validate_token(key, token, user_id))

    def test_token_too_old_implicit_current_time(self):
        token_time = 123456789
        curr_time = token_time + xsrfutil.DEFAULT_TIMEOUT_SECS + 1

        key = user_id = None
        token = base64.b64encode(_helpers._to_bytes(str(token_time)))
        with mock.patch('oauth2client.contrib.xsrfutil.time') as time:
            time.time = mock.Mock(name='time', return_value=curr_time)
            self.assertFalse(xsrfutil.validate_token(key, token, user_id))
            time.time.assert_called_once_with()

    def test_token_too_old_explicit_current_time(self):
        token_time = 123456789
        curr_time = token_time + xsrfutil.DEFAULT_TIMEOUT_SECS + 1

        key = user_id = None
        token = base64.b64encode(_helpers._to_bytes(str(token_time)))
        self.assertFalse(xsrfutil.validate_token(key, token, user_id,
                                                 current_time=curr_time))

    def test_token_length_differs_from_generated(self):
        token_time = 123456789
        # Make sure it isn't too old.
        curr_time = token_time + xsrfutil.DEFAULT_TIMEOUT_SECS - 1

        key = object()
        user_id = object()
        action_id = object()
        token = base64.b64encode(_helpers._to_bytes(str(token_time)))
        generated_token = b'a'
        # Make sure the token length comparison will fail.
        self.assertNotEqual(len(token), len(generated_token))

        with mock.patch('oauth2client.contrib.xsrfutil.generate_token',
                        return_value=generated_token) as gen_tok:
            self.assertFalse(xsrfutil.validate_token(key, token, user_id,
                                                     current_time=curr_time,
                                                     action_id=action_id))
            gen_tok.assert_called_once_with(key, user_id, action_id=action_id,
                                            when=token_time)

    def test_token_differs_from_generated_but_same_length(self):
        token_time = 123456789
        # Make sure it isn't too old.
        curr_time = token_time + xsrfutil.DEFAULT_TIMEOUT_SECS - 1

        key = object()
        user_id = object()
        action_id = object()
        token = base64.b64encode(_helpers._to_bytes(str(token_time)))
        # It is encoded as b'MTIzNDU2Nzg5', which has length 12.
        generated_token = b'M' * 12
        # Make sure the token length comparison will succeed, but the token
        # comparison will fail.
        self.assertEqual(len(token), len(generated_token))
        self.assertNotEqual(token, generated_token)

        with mock.patch('oauth2client.contrib.xsrfutil.generate_token',
                        return_value=generated_token) as gen_tok:
            self.assertFalse(xsrfutil.validate_token(key, token, user_id,
                                                     current_time=curr_time,
                                                     action_id=action_id))
            gen_tok.assert_called_once_with(key, user_id, action_id=action_id,
                                            when=token_time)

    def test_success(self):
        token_time = 123456789
        # Make sure it isn't too old.
        curr_time = token_time + xsrfutil.DEFAULT_TIMEOUT_SECS - 1

        key = object()
        user_id = object()
        action_id = object()
        token = base64.b64encode(_helpers._to_bytes(str(token_time)))
        with mock.patch('oauth2client.contrib.xsrfutil.generate_token',
                        return_value=token) as gen_tok:
            self.assertTrue(xsrfutil.validate_token(key, token, user_id,
                                                    current_time=curr_time,
                                                    action_id=action_id))
            gen_tok.assert_called_once_with(key, user_id, action_id=action_id,
                                            when=token_time)


class XsrfUtilTests(unittest.TestCase):
    """Test xsrfutil functions."""

    def testGenerateAndValidateToken(self):
        """Test generating and validating a token."""
        token = xsrfutil.generate_token(TEST_KEY,
                                        TEST_USER_ID_1,
                                        action_id=TEST_ACTION_ID_1,
                                        when=TEST_TIME)

        # Check that the token is considered valid when it should be.
        self.assertTrue(xsrfutil.validate_token(TEST_KEY,
                                                token,
                                                TEST_USER_ID_1,
                                                action_id=TEST_ACTION_ID_1,
                                                current_time=TEST_TIME))

        # Should still be valid 15 minutes later.
        later15mins = TEST_TIME + 15 * 60
        self.assertTrue(xsrfutil.validate_token(TEST_KEY,
                                                token,
                                                TEST_USER_ID_1,
                                                action_id=TEST_ACTION_ID_1,
                                                current_time=later15mins))

        # But not if beyond the timeout.
        later2hours = TEST_TIME + 2 * 60 * 60
        self.assertFalse(xsrfutil.validate_token(TEST_KEY,
                                                 token,
                                                 TEST_USER_ID_1,
                                                 action_id=TEST_ACTION_ID_1,
                                                 current_time=later2hours))

        # Or if the key is different.
        self.assertFalse(xsrfutil.validate_token('another key',
                                                 token,
                                                 TEST_USER_ID_1,
                                                 action_id=TEST_ACTION_ID_1,
                                                 current_time=later15mins))

        # Or the user ID....
        self.assertFalse(xsrfutil.validate_token(TEST_KEY,
                                                 token,
                                                 TEST_USER_ID_2,
                                                 action_id=TEST_ACTION_ID_1,
                                                 current_time=later15mins))

        # Or the action ID...
        self.assertFalse(xsrfutil.validate_token(TEST_KEY,
                                                 token,
                                                 TEST_USER_ID_1,
                                                 action_id=TEST_ACTION_ID_2,
                                                 current_time=later15mins))

        # Invalid when truncated
        self.assertFalse(xsrfutil.validate_token(TEST_KEY,
                                                 token[:-1],
                                                 TEST_USER_ID_1,
                                                 action_id=TEST_ACTION_ID_1,
                                                 current_time=later15mins))

        # Invalid with extra garbage
        self.assertFalse(xsrfutil.validate_token(TEST_KEY,
                                                 token + b'x',
                                                 TEST_USER_ID_1,
                                                 action_id=TEST_ACTION_ID_1,
                                                 current_time=later15mins))

        # Invalid with token of None
        self.assertFalse(xsrfutil.validate_token(TEST_KEY,
                                                 None,
                                                 TEST_USER_ID_1,
                                                 action_id=TEST_ACTION_ID_1))
