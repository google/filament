#!/usr/bin/env vpython3
# Copyright (c) 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for rdb_wrapper.py"""

import contextlib
import json
import logging
import os
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import rdb_wrapper


@contextlib.contextmanager
def lucictx(ctx):
    try:
        orig = os.environ.get('LUCI_CONTEXT')

        if ctx is None:
            os.environ.pop('LUCI_CONTEXT', '')
            yield
        else:
            # windows doesn't allow a file to be opened twice at the same time.
            # therefore, this closes the temp file before yield, so that
            # rdb_wrapper.client() can open the LUCI_CONTEXT file.
            f = tempfile.NamedTemporaryFile(delete=False)
            f.write(json.dumps(ctx).encode('utf-8'))
            f.close()
            os.environ['LUCI_CONTEXT'] = f.name
            yield
            os.unlink(f.name)

    finally:
        if orig is None:
            os.environ.pop('LUCI_CONTEXT', '')
        else:
            os.environ['LUCI_CONTEXT'] = orig


@mock.patch.dict(os.environ, {})
class TestClient(unittest.TestCase):
    def test_without_lucictx(self):
        with lucictx(None):
            with rdb_wrapper.client("prefix") as s:
                self.assertIsNone(s)

        with lucictx({'something else': {'key': 'value'}}):
            with rdb_wrapper.client("prefix") as s:
                self.assertIsNone(s)

    def test_with_lucictx(self):
        with lucictx(
            {'result_sink': {
                'address': '127',
                'auth_token': 'secret'
            }}):
            with rdb_wrapper.client("prefix") as s:
                self.assertIsNotNone(s)
                self.assertEqual(
                    s._url,
                    'http://127/prpc/luci.resultsink.v1.Sink/ReportTestResults',
                )
                self.assertDictEqual(
                    s._session.headers, {
                        'Accept': 'application/json',
                        'Authorization': 'ResultSink secret',
                        'Content-Type': 'application/json',
                    })


class TestResultSink(unittest.TestCase):
    def test_report(self):
        session = mock.MagicMock()
        sink = rdb_wrapper.ResultSink(session, 'http://host', 'test_id_prefix/')
        sink.report("function_foo", rdb_wrapper.STATUS_PASS, 123)
        expected = {
            'testId': 'test_id_prefix/function_foo',
            'status': rdb_wrapper.STATUS_PASS,
            'expected': True,
            'duration': '123.000000000s',
        }
        session.post.assert_called_once_with(
            'http://host',
            json={'testResults': [expected]},
        )

    def test_report_failure_reason(self):
        session = mock.MagicMock()
        sink = rdb_wrapper.ResultSink(session, 'http://host', 'test_id_prefix/')
        sink.report("function_foo", rdb_wrapper.STATUS_PASS, 123, 'Bad CL.')
        expected = {
            'testId': 'test_id_prefix/function_foo',
            'status': rdb_wrapper.STATUS_PASS,
            'expected': True,
            'duration': '123.000000000s',
            'failureReason': {
                'primaryErrorMessage': 'Bad CL.',
            },
        }
        session.post.assert_called_once_with(
            'http://host',
            json={'testResults': [expected]},
        )

    def test_report_failure_reason_truncated(self):
        session = mock.MagicMock()
        sink = rdb_wrapper.ResultSink(session, 'http://host', 'test_id_prefix/')
        sink.report("function_foo", rdb_wrapper.STATUS_PASS, 123, 'X' * 1025)
        trunc_text = rdb_wrapper._FAILURE_REASON_TRUNCATE_TEXT
        limit = rdb_wrapper._FAILURE_REASON_LENGTH_LIMIT
        expected_truncated_error = 'X' * (limit - len(trunc_text)) + trunc_text
        expected = {
            'testId': 'test_id_prefix/function_foo',
            'status': rdb_wrapper.STATUS_PASS,
            'expected': True,
            'duration': '123.000000000s',
            'failureReason': {
                'primaryErrorMessage': expected_truncated_error,
            },
        }
        session.post.assert_called_once_with(
            'http://host',
            json={'testResults': [expected]},
        )


if __name__ == '__main__':
    logging.basicConfig(
        level=logging.DEBUG if '-v' in sys.argv else logging.ERROR)
    unittest.main()
