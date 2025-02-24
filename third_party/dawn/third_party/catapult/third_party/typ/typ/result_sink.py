# Copyright 2020 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Functionality for interacting with ResultDB's ResultSink.

ResultSink is a process that accepts test results via HTTP requests for
ingesting into ResultDB.

See go/resultdb and go/resultsink for more details.
"""

import base64
import contextlib
import hashlib
import json
import os
import sys

import requests

from typ import host as typ_host
from typ import json_results
from typ import expectations_parser

# Map JSON result types to ResultDB statuses. Currently, this is an identity map
# except for `TIMEOUT`, which maps to ResultDB's more general `ABORT`. See also:
#   https://chromium.googlesource.com/chromium/src/+/HEAD/docs/testing/json_test_results_format.md#test-result-types
#   https://source.chromium.org/chromium/infra/infra/+/master:go/src/go.chromium.org/luci/resultdb/proto/v1/test_result.proto
_JSON_TO_RESULTDB_STATUSES = {
    json_results.ResultType.Pass: 'PASS',
    json_results.ResultType.Failure: 'FAIL',
    json_results.ResultType.Crash: 'CRASH',
    json_results.ResultType.Timeout: 'ABORT',
    json_results.ResultType.Skip: 'SKIP',
}
VALID_STATUSES = set(_JSON_TO_RESULTDB_STATUSES.values())
STDOUT_KEY = 'typ_stdout'
STDERR_KEY = 'typ_stderr'
# From https://source.chromium.org/chromium/infra/infra/+/main:go/src/go.chromium.org/luci/resultdb/pbutil/strpair.go;l=28
MAX_TAG_LENGTH = 256
SHA1_HEX_HASH_LENGTH = 40


class ResultSinkReporter(object):
    def __init__(self, host=None, disable=False, output_file=None):
        """Class for interacting with ResultDB's ResultSink.

        Args:
            host: A typ_host.Host or host_fake.FakeHost instance.
            disable: Whether to explicitly disable ResultSink integration.
            output_file: A string pointing to a filepath to write POST request
                data to instead of actually POSTing to ResultSink. Note: this
                makes no attempt at being thread-safe, as it is only intended
                as a workaround for Skylab, and it is not expected to run tests
                in parallel there.
        """
        self.host = host or typ_host.Host()
        self._sink = None
        self._invocation_level_url = None
        self._url = None
        self._headers = None
        self._session = None
        self._chromium_src_dir = None
        self._output_file = output_file
        self._pending_results = None
        if disable:
            return

        if not output_file:
            luci_context_file = self.host.getenv('LUCI_CONTEXT')
            if not luci_context_file:
                return
            self._sink = json.loads(
                    self.host.read_text_file(luci_context_file)).get(
                            'result_sink')
            if not self._sink:
                return

            self._invocation_level_url = ('http://%s/prpc/luci.resultsink.v1.Sink/ReportInvocationLevelArtifacts'
                         % self._sink['address'])

            self._url = (
                    'http://%s/prpc/luci.resultsink.v1.Sink/ReportTestResults'
                    % self._sink['address'])
            self._headers = {
                    'Content-Type': 'application/json',
                    'Accept': 'application/json',
                    'Authorization': 'ResultSink %s' % self._sink['auth_token']
            }
            self._session = requests.Session()

    @property
    def resultdb_supported(self):
        return self._sink or self._output_file

    def report_invocation_level_artifacts(self, artifacts):
        """Uploads invocation-level artifacts to the ResultSink server.

        This is for artifacts that don't apply to a single test but to the test
        invocation as a whole (eg: system logs).

        Args:
          artifacts: A dict of artifacts to attach to the invocation.

        Returns:
          0 if the result was reported successfully or ResultDB is not
          supported, otherwise 1.
        """
        if not self.resultdb_supported:
            return 0

        req = {'artifacts': artifacts}
        res = self._post(self._invocation_level_url, json.dumps(req))
        return res

    def report_individual_test_result(
            self, result, artifact_output_dir, expectations, test_file_location,
            test_file_line=None, test_name_prefix='', additional_tags=None,
            html_summary=None):
        """Reports a single test result to ResultSink.

        Inputs are typically similar to what is passed to
        json_results.make_full_results(), but for a single test/Result instead
        of multiple tests/a ResultSet.

        Args:
            result: A json_results.Result instance containing the result to
                    report.
            artifact_output_dir: The path to the directory where artifacts test
                    artifacts are saved on disk. If a relative path, will be
                    automatically joined with the cwd. Use '.' instead of '' to
                    point to the cwd.
            expectations: An expectations_parser.TestExpectations instance, or
                    None if one is not available.
            test_file_location: A string containing the path to the file
                    containing the test.
            test_file_line: An int indicating the source file line number
                    containing the test.
            test_name_prefix: A string containing the prefix that will be added
                    to the test name.
            additional_tags: An optional dict of additional tags to report to
                    ResultDB.
            html_summary: Optional human-readable explanation of the result as
                    sanitized HTML. If omitted, the reporter will generate a
                    default summary with links extracted from artifacts.

        Returns:
            0 if the result was reported successfully or ResultDB is not
            supported, otherwise 1.
        """
        if not self.resultdb_supported:
            return 0

        expectation_tags = expectations.tags if expectations else []
        additional_tags = additional_tags or {}

        test_id = test_name_prefix + result.name
        raw_typ_expected_results = (
                expectations.expectations_for(result.name).raw_results
                if expectations
                else [expectations_parser.RESULT_TAGS[t]
                      for t in result.expected])
        result_is_expected = result.actual in result.expected

        # ResultDB has a 256 character limit for arbitrary key/value pairs. The
        # non-arbitrary fields such as test ID have a longer limit, so those
        # should be used if the actual name of the test is needed.
        truncated_name = result.name
        if len(truncated_name) > MAX_TAG_LENGTH:
            m = hashlib.sha1()
            m.update(truncated_name.encode('utf-8'))
            truncated_name = truncated_name[:(MAX_TAG_LENGTH -
                                              SHA1_HEX_HASH_LENGTH)]
            truncated_name += m.hexdigest()

        tag_list = [
            ('test_name', truncated_name),
        ]
        for expectation in result.expected:
            tag_list.append(('typ_expectation', expectation))
        for expectation in raw_typ_expected_results:
            tag_list.append(('raw_typ_expectation', expectation))
        if expectation_tags:
            for tag in expectation_tags:
                tag_list.append(('typ_tag', tag))
        for key, value in additional_tags.items():
            assert isinstance(key, str)
            assert isinstance(value, str)
            tag_list.append((key, value))

        artifacts = {}
        original_artifacts = result.artifacts or {}
        https_artifacts = ''
        assert STDOUT_KEY not in original_artifacts
        assert STDERR_KEY not in original_artifacts
        if original_artifacts:
            assert artifact_output_dir
            if not os.path.isabs(artifact_output_dir):
                artifact_output_dir = self.host.join(
                        self.host.getcwd(), artifact_output_dir)

        for artifact_name, artifact_filepaths in original_artifacts.items():
            # Links can be reported as artifacts and are meant to be clickable.
            # So, pull those out now so they can be added to the HTML summary
            # later. typ's artifact implementation enforces that all links are
            # HTTPS, so we can use that to identify links.
            if (len(artifact_filepaths) == 1
                and artifact_filepaths[0].startswith('https://')):
                https_artifacts += '<a href=%s>%s</a>' % (
                    artifact_filepaths[0], artifact_name)
            # The typ artifact implementation supports multiple artifacts for
            # a single artifact name due to retries, but ResultDB does not.
            elif len(artifact_filepaths) > 1:
                for index, filepath in enumerate(artifact_filepaths):
                    artifacts[artifact_name + '-file%d' % index] = {
                        'filePath': self.host.join(
                                artifact_output_dir, filepath),
                    }
            else:
                artifacts[artifact_name] = {
                    'filePath': self.host.join(
                            artifact_output_dir, artifact_filepaths[0]),
                }

        for artifact_id, contents in [(STDOUT_KEY, result.out),
                                      (STDERR_KEY, result.err)]:
            if contents:
                artifacts[artifact_id] = {
                    'contents': base64.b64encode(
                        contents.encode('utf-8')).decode('utf-8'),
                }

        if not html_summary:
            html_summary = https_artifacts
            for artifact_id in [STDOUT_KEY, STDERR_KEY]:
                if artifact_id in artifacts:
                    html_summary += (
                        '<p><text-artifact artifact-id="%s"/></p>' % artifact_id)

        test_location_in_repo = self._convert_path_to_repo_path(
            os.path.normpath(test_file_location))
        test_metadata = {
            'name': test_id,
            'location': {
                'repo': 'https://chromium.googlesource.com/chromium/src',
                'fileName': test_location_in_repo,
            },
        }

        if test_file_line is not None:
            test_metadata['location'].update({'line': test_file_line})

        status = _JSON_TO_RESULTDB_STATUSES.get(result.actual, result.actual)
        return self._report_result(
                test_id, status, result_is_expected, artifacts, tag_list,
                html_summary, result.took, test_metadata, result.failure_reason)

    @contextlib.contextmanager
    def batch_results(self):
        """Begin buffering test results, which will be uploaded on exit.

        This method allows callers to report multiple test results in one
        request. Batching results can significantly improve the performance of
        `report_individual_test_result()`, which defaults to one result per
        request.

        Usage notes:
          * The reporter is not threadsafe while batching is active.
          * The returned context manager is not reentrant.
          * An exception that reaches the context manager will cancel the
            pending upload, but is otherwise not handled.
        """
        if self._pending_results is not None:
            raise ResultSinkError('`batch_results()` cannot be nested')
        self._pending_results = json_results.ResultSet()
        try:
            yield
            if self._pending_results.results:
                payload = json.dumps({
                    'testResults': self._pending_results.results,
                })
                status = self._post(self._url, payload)
                if status != 0:
                    # There's no easy way to pass the status code to the caller,
                    # so signal failure through an exception instead.
                    raise ResultSinkError(
                        f'failed to upload batch results (status: {status})')
        finally:
            self._pending_results = None

    def _report_result(
            self, test_id, status, expected, artifacts, tag_list, html_summary,
            duration, test_metadata, failure_reason):
        """Reports a single test result to ResultSink.

        Args:
            test_id: A string containing the unique identifier of the test.
            status: A string containing the status of the test. Must be in
                    |VALID_STATUSES|.
            expected: A boolean denoting whether |status| is expected or not.
            artifacts: A dict of artifact names (strings) to dicts, specifying
                    either a filepath or base64-encoded artifact content.
            tag_list: A list of tuples of (str, str), each element being a
                    key/value pair to add as tags to the reported result.
            html_summary: A string containing HTML summarizing the test run.
            duration: How long the test took in seconds.
            test_metadata: A dict containing additional test metadata to upload.
            failure_reason: An optional FailureReason object describing the
                    reason the test failed.

        Returns:
            0 if the result was reported successfully or ResultDB is not
            supported, otherwise 1.
        """
        if not self.resultdb_supported:
            return 0

        # TODO(crbug.com/1104252): Handle testLocation key so that ResultDB can
        # look up the correct component for bug filing.
        test_result = _create_json_test_result(
                test_id, status, expected, artifacts, tag_list, html_summary,
                duration, test_metadata, failure_reason)

        if self._pending_results:
            self._pending_results.add(test_result)
            # Treat the deferred upload as a tentative success.
            return 0
        return self._post(self._url, json.dumps({'testResults': [test_result]}))

    def _post(self, url, content):
        """POST to ResultSink.

        Args:
            url: A string containing the url for the POST request
            content: A string containing the content to send in the body of the
                    POST request.

        Returns:
            0 if the POST succeeded, otherwise 1.
        """
        if self._session:
            res = self._session.post(
                url=url,
                headers=self._headers,
                data=content)
            ret = 0 if res.ok else 1
        elif self._output_file:
            test_results = json.loads(content)['testResults']
            # Append the test results to the jsonl file, where
            # each line represents a test result in json string.
            for t in test_results:
                self.host.append_text_file(self._output_file,
                                           '{}\n'.format(json.dumps(t)))
            ret = 0
        else:
            raise RuntimeError('Called _post without a session or output file')
        return ret

    def _convert_path_to_repo_path(self, filepath):
        """Converts an absolute file path to a repo relative file path.

        Args:
            filepath: A string containing an absolute path to a file.

        Returns:
            A string containing the path to |filepath| in the Chromium
            src repo, leading with //.
        """
        chromium_src_dir = self._get_chromium_src_dir()
        assert chromium_src_dir in filepath
        repo_location = filepath.replace(chromium_src_dir, '//', 1)
        repo_location = repo_location.replace(self.host.sep, '/')
        return repo_location

    def _get_chromium_src_dir(self):
        if not self._chromium_src_dir:
            src_dir = self.host.abspath(
                self.host.join(self.host.dirname(__file__), '..', '..', '..',
                               '..', '..'))
            if not src_dir.endswith(self.host.sep):
                src_dir += self.host.sep
            self._chromium_src_dir = src_dir
        return self._chromium_src_dir


class ResultSinkError(Exception):
    """Base exception for errors when using a result sink reporter."""


def _create_json_test_result(
        test_id, status, expected, artifacts, tag_list, html_summary,
        duration, test_metadata, failure_reason):
    """Formats data to be suitable for sending to ResultSink.

    Args:
        test_id: A string containing the unique identifier of the test.
        status: A string containing the status of the test. Must be in
                |VALID_STATUSES|.
        expected: A boolean denoting whether |status| is expected or not.
        artifacts: A dict of artifact names (strings) to dicts, specifying
                either a filepath or base64-encoded artifact content.
        tag_list: A list of tuples of (str, str), each element being a
                    key/value pair to add as tags to the reported result.
        html_summary: A string containing HTML summarizing the test run.
        duration: How long the test took in seconds.
        test_metadata: A dict containing additional test metadata to upload.
        failure_reason: An optional FailureReason object describing the
                reason the test failed.

    Returns:
        A dict containing the provided data in a format that is ingestable by
        ResultSink.
    """
    if status not in VALID_STATUSES:
        raise ValueError('Status %r is not in the VALID_STATUSES list' % status)

    # This is based off the protobuf in
    # https://source.chromium.org/chromium/infra/infra/+/main:go/src/go.chromium.org/luci/resultdb/sink/proto/v1/test_result.proto
    test_result = {
            'testId': test_id,
            'status': status,
            'expected': expected,
            # If the number is too large or small, python formats the number
            # in scientific notation, but google.protobuf.duration doesn't
            # accept an input formatted in scientific notation.
            #
            # .9fs because nanosecond is the smallest precision that
            # google.protobuf.duration supports.
            'duration': '%.9fs' % duration,
            'summaryHtml': html_summary,
            'artifacts': artifacts,
            'tags': [],
            'testMetadata': test_metadata,
    }
    for (k, v) in tag_list:
        test_result['tags'].append({'key': k, 'value': v})

    # This is based off the protobuf in
    # https://source.chromium.org/chromium/infra/infra/+/main:go/src/go.chromium.org/luci/resultdb/proto/v1/failure_reason.proto
    if failure_reason:
        primary_error_message = _truncate_to_utf8_bytes(
                failure_reason.primary_error_message, 1024)
        test_result['failureReason'] = {
                'primaryErrorMessage': primary_error_message,
        }

    return test_result


def result_sink_retcode_from_result_set(result_set):
    """Determines whether any interactions with ResultSink failed.

    Args:
        result_set: A json_results.ResultSet instance.

    Returns:
        1 if any Result in |result_set| failed to interact properly with
        ResultSink, otherwise 0.
    """
    return int(any(r.result_sink_retcode for r in result_set.results))


def _truncate_to_utf8_bytes(s, length):
    """ Truncates a string to a given number of bytes when encoded as UTF-8.

    Ensures the given string does not take more than length bytes when encoded
    as UTF-8. Adds trailing ellipsis (...) if truncation occurred. A truncated
    string may end up encoding to a length slightly shorter than length
    because only whole Unicode codepoints are dropped.

    Args:
        s: The string to truncate.
        length: the length (in bytes) to truncate to.
    """
    try:
        encoded = s.encode('utf-8')
    # When encode throws UnicodeDecodeError in py2, it usually means the str is
    # already encoded and has non-ascii chars. So skip re-encoding it.
    except UnicodeDecodeError:
        encoded = s
    if len(encoded) > length:
        # Truncate, leaving space for trailing ellipsis (...).
        encoded = encoded[:length - 3]
        # Truncating the string encoded as UTF-8 may have left the final codepoint
        # only partially present. Pass 'ignore' to acknowledge and ensure this is
        # dropped.
        return encoded.decode('utf-8', 'ignore') + "..."
    return s
