#!/usr/bin/env vpython3
# Copyright (c) 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import contextlib
import json
import os
import requests

# Constants describing TestStatus for ResultDB
STATUS_PASS = 'PASS'
STATUS_FAIL = 'FAIL'
STATUS_CRASH = 'CRASH'
STATUS_ABORT = 'ABORT'
STATUS_SKIP = 'SKIP'

# ResultDB limits failure reasons to 1024 characters.
_FAILURE_REASON_LENGTH_LIMIT = 1024

# Message to use at the end of a truncated failure reason.
_FAILURE_REASON_TRUNCATE_TEXT = '\n...\nFailure reason was truncated.'


class ResultSink(object):
    def __init__(self, session, url, prefix):
        self._session = session
        self._url = url
        self._prefix = prefix

    def report(self, function_name, status, elapsed_time, failure_reason=None):
        """Reports the result and elapsed time of a presubmit function call.

        Args:
            function_name (str): The name of the presubmit function
            status: the status to report the function call with
            elapsed_time: the time taken to invoke the presubmit function
            failure_reason (str or None): if set, the failure reason
        """
        tr = {
            'testId': self._prefix + function_name,
            'status': status,
            'expected': status == STATUS_PASS,
            'duration': '{:.9f}s'.format(elapsed_time)
        }
        if failure_reason:
            if len(failure_reason) > _FAILURE_REASON_LENGTH_LIMIT:
                failure_reason = failure_reason[:-len(
                    _FAILURE_REASON_TRUNCATE_TEXT) - 1]
                failure_reason += _FAILURE_REASON_TRUNCATE_TEXT
            tr['failureReason'] = {'primaryErrorMessage': failure_reason}
        self._session.post(self._url, json={'testResults': [tr]})


@contextlib.contextmanager
def client(prefix):
    """Returns a client for ResultSink.

    This is a context manager that returns a client for ResultSink,
    if LUCI_CONTEXT with a section of result_sink is present. When the context
    is closed, all the connetions to the SinkServer are closed.

    Args:
        prefix: A prefix to be added to the test ID of reported function names.
        The format for this is
            presubmit:gerrit_host/folder/to/repo:path/to/file/
        for example,
            presubmit:chromium-review.googlesource.com/chromium/src/:services/viz/  # pylint: disable=line-too-long
    Returns:
        An instance of ResultSink() if the luci context is present. None,
        otherwise.
    """
    luci_ctx = os.environ.get('LUCI_CONTEXT')
    if not luci_ctx:
        yield None
        return

    sink_ctx = None
    with open(luci_ctx) as f:
        sink_ctx = json.load(f).get('result_sink')
        if not sink_ctx:
            yield None
            return

    url = 'http://{0}/prpc/luci.resultsink.v1.Sink/ReportTestResults'.format(
        sink_ctx['address'])
    with requests.Session() as s:
        s.headers = {
            'Content-Type': 'application/json',
            'Accept': 'application/json',
            'Authorization': 'ResultSink {0}'.format(sink_ctx['auth_token'])
        }
        yield ResultSink(s, url, prefix)
