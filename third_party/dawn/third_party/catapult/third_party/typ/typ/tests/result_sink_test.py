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

import base64
import hashlib
import json
import os
import unittest

from typ import expectations_parser
from typ import json_results
from typ import result_sink
from typ.fakes import host_fake


DEFAULT_LUCI_CONTEXT = {
    'result_sink': {
        'address': 'address',
        'auth_token': 'auth_token',
    },
}
ARTIFACT_DIR = os.path.join('artifact', 'dir')
FAKE_TEST_PATH = '/src/some/test.py'
FAKE_TEST_LINE = 123
HTML_SUMMARY = ('<p><text-artifact artifact-id="typ_stdout"/></p>'
                '<p><text-artifact artifact-id="typ_stderr"/></p>')
STDOUT_STDERR_ARTIFACTS = {
    'typ_stdout': {
        'contents': base64.b64encode(b'stdout').decode('utf-8')
    },
    'typ_stderr': {
        'contents': base64.b64encode(b'stderr').decode('utf-8')
    }
}


def CreateResult(input_dict):
    """Creates a ResultSet with Results.

    Args:
        input_dict: A dict describing the result to create.

    Returns:
        A Result filled with the information from |input_dict|
    """
    return json_results.Result(name=input_dict['name'],
                               actual=input_dict['actual'],
                               started=True,
                               took=1,
                               worker=None,
                               expected=input_dict.get('expected'),
                               out=input_dict.get('out', 'stdout'),
                               err=input_dict.get('err', 'stderr'),
                               artifacts=input_dict.get('artifacts'),
                               failure_reason=input_dict.get('failure_reason'))


def StubWithRetval(retval):
    def stub(*args, **kwargs):
        stub.args = args
        stub.kwargs = kwargs
        return retval
    stub.args = None
    stub.kwargs = None
    return stub


def GetTestResultFromPostedJson(json_string):
    dict_content = json.loads(json_string)
    return dict_content['testResults'][0]


def CreateExpectedTestResult(
        test_id=None, status=None, expected=None, duration=None,
        summary_html=HTML_SUMMARY, artifacts=None, tags=None, test_metadata=None,
        primary_error_message=None):
    test_id = test_id or 'test_name_prefix.test_name'
    result = {
        'testId': test_id,
        'status': status or json_results.ResultType.Pass,
        'expected': expected if expected is not None else True,
        'duration': duration or '1.000000000s',
        'summaryHtml': summary_html,
        'artifacts': artifacts or STDOUT_STDERR_ARTIFACTS,
        'tags': tags or [
            {'key': 'test_name', 'value': test_id.split('.')[-1]},
            {'key': 'typ_expectation', 'value': json_results.ResultType.Pass},
            {'key': 'raw_typ_expectation', 'value': 'Pass'},
            {'key': 'typ_tag', 'value': 'bar_tag'},
            {'key': 'typ_tag', 'value': 'foo_tag'},],
        'testMetadata': test_metadata or {
            'name': test_id,
            'location': {
                'repo': 'https://chromium.googlesource.com/chromium/src',
                'fileName': '//some/test.py',
                'line': FAKE_TEST_LINE,
            }
        }
    }
    if primary_error_message:
        result['failureReason'] = {
            'primaryErrorMessage': primary_error_message,
        }
    return result


def CreateTestExpectations(expectation_definitions=None):
    expectation_definitions = expectation_definitions or [{'name': 'test_name'}]
    tags = set()
    results = set()
    lines = []
    for ed in expectation_definitions:
        name = ed['name']
        t = ed.get('tags', ['foo_tag', 'bar_tag'])
        r = ed.get('results', ['Pass'])
        str_t = '[ ' + ' '.join(t) + ' ]' if t else ''
        str_r = '[ ' + ' '.join(r) + ' ]'
        lines.append('%s %s %s' % (str_t, name, str_r))
        tags |= set(t)
        results |= set(r)
    data = '# tags: [ %s ]\n# results: [ %s ]\n%s' % (
            ' '.join(sorted(tags)), ' '.join(sorted(results)), '\n'.join(lines))
    # TODO(crbug.com/1148060): Remove the passing in of tags to the constructor
    # once tags are properly updated through parse_tagged_list().
    expectations = expectations_parser.TestExpectations(sorted(tags))
    expectations.parse_tagged_list(data)
    return expectations


class ResultSinkReporterWithFakeSrc(result_sink.ResultSinkReporter):
    def __init__(self, *args, **kwargs):
        super(ResultSinkReporterWithFakeSrc, self).__init__(*args, **kwargs)
        self._chromium_src_dir = '/src/'


class ResultSinkReporterTest(unittest.TestCase):
    maxDiff = None

    def setUp(self):
        self._host = host_fake.FakeHost()
        self._luci_context_file = '/tmp/luci_context_file.json'

    def setLuciContextWithContent(self, content):
        self._host.env['LUCI_CONTEXT'] = self._luci_context_file
        self._host.files[self._luci_context_file] = json.dumps(content)

    def testNoLuciContext(self):
        if 'LUCI_CONTEXT' in self._host.env:
            del self._host.env['LUCI_CONTEXT']
        rsr = result_sink.ResultSinkReporter(self._host)
        self.assertFalse(rsr.resultdb_supported)

    def testNoLuciContextWithOutputFile(self):
        if 'LUCI_CONTEXT' in self._host.env:
            del self._host.env['LUCI_CONTEXT']
        rsr = result_sink.ResultSinkReporter(self._host, output_file='/path')
        self.assertTrue(rsr.resultdb_supported)

    def testExplicitDisable(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = result_sink.ResultSinkReporter(self._host, True)
        self.assertFalse(rsr.resultdb_supported)

    def testNoSinkKey(self):
        self.setLuciContextWithContent({})
        rsr = result_sink.ResultSinkReporter(self._host)
        self.assertFalse(rsr.resultdb_supported)

    def testValidSinkKey(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = result_sink.ResultSinkReporter(self._host)
        self.assertTrue(rsr.resultdb_supported)

    def testReportInvocationLevelArtifactsEarlyReturnIfNotSupported(self):
        self.setLuciContextWithContent({})
        rsr = result_sink.ResultSinkReporter(self._host)
        rsr._post = lambda: 1/0  # Shouldn't be called.
        self.assertEqual(rsr.report_invocation_level_artifacts({}), 0)

    def testReportInvocationLevelArtifacts(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        rsr._post = StubWithRetval(0)
        artifacts = {"report": {"filePath": "/path/to/report"}}
        retval = rsr.report_invocation_level_artifacts(artifacts)
        self.assertEqual(retval, 0)

    def testReportIndividualTestResultEarlyReturnIfNotSupported(self):
        self.setLuciContextWithContent({})
        rsr = result_sink.ResultSinkReporter(self._host)
        rsr._post = lambda: 1/0  # Shouldn't be called.
        self.assertEqual(
                rsr.report_individual_test_result(None, None, None, None, None,
                                                  None),
                0)

    def testReportIndividualTestResultBasicCase(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        result = CreateResult({
            'name': 'test_name',
            'actual': json_results.ResultType.Timeout,
        })
        rsr._post = StubWithRetval(2)
        retval = rsr.report_individual_test_result(
                result, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
                FAKE_TEST_LINE, 'test_name_prefix.')
        self.assertEqual(retval, 2)
        expected_result = CreateExpectedTestResult(status='ABORT', expected=False)
        self.assertEqual(GetTestResultFromPostedJson(rsr._post.args[1]),
                         expected_result)

    def testReportIndividualTestResultOutputFile(self):
        output_filepath = '/tmp/output.json'
        self.setLuciContextWithContent({})
        rsr = ResultSinkReporterWithFakeSrc(
                self._host, output_file=output_filepath)
        result = CreateResult({
            'name': 'test_name',
            'actual': json_results.ResultType.Timeout,
        })
        retval = rsr.report_individual_test_result(
                result, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
                FAKE_TEST_LINE, 'test_name_prefix.')
        self.assertEqual(retval, 0)
        expected_result = CreateExpectedTestResult(status='ABORT',
                                                   expected=False)
        self.assertIn(output_filepath, self._host.files)
        self.assertEqual(json.loads(self._host.files[output_filepath]),
                         expected_result)

    def testReportIndividualTestResultOutputFileMultiplePosts(self):
        output_filepath = '/tmp/output.json'
        self.setLuciContextWithContent({})
        rsr = ResultSinkReporterWithFakeSrc(
            self._host, output_file=output_filepath)
        result = CreateResult({
            'name': 'test_name',
            'actual': json_results.ResultType.Timeout,
        })
        retval = rsr.report_individual_test_result(
                result, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
                FAKE_TEST_LINE, 'test_name_prefix.')
        self.assertEqual(retval, 0)
        retval = rsr.report_individual_test_result(
                result, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
                FAKE_TEST_LINE, 'test_name_prefix.')
        self.assertEqual(retval, 0)
        expected_result = CreateExpectedTestResult(status='ABORT',
                                                   expected=False)
        self.assertIn(output_filepath, self._host.files)
        got_results = [json.loads(
                x) for x in self._host.files[output_filepath].rstrip(
                '\n').split('\n')]
        self.assertEqual(got_results, [expected_result, expected_result])

    def testBatchResults(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        rsr._post = StubWithRetval(0)
        with rsr.batch_results():
            result1 = CreateResult({
                'name': 'test_name',
                'actual': json_results.ResultType.Timeout,
            })
            status1 = rsr.report_individual_test_result(
                result1, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
                FAKE_TEST_LINE, 'test_name_prefix.')
            self.assertEqual(status1, 0)

            result2 = CreateResult({
                'name': 'test_name',
                'actual': json_results.ResultType.Failure,
            })
            status2 = rsr.report_individual_test_result(
                result2, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
                FAKE_TEST_LINE, 'test_name_prefix.')
            self.assertEqual(status2, 0)

        results = json.loads(rsr._post.args[1])['testResults']
        self.assertEqual(results, [
            CreateExpectedTestResult(status='ABORT', expected=False),
            CreateExpectedTestResult(status='FAIL', expected=False),
        ])

    def testBatchResultsFailure(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        rsr._post = StubWithRetval(2)
        with self.assertRaisesRegex(result_sink.ResultSinkError,
                                    'failed to upload batch results '
                                    '\(status: 2\)'):
            with rsr.batch_results():
                result = CreateResult({
                    'name': 'test_name',
                    'actual': json_results.ResultType.Timeout,
                })
                status = rsr.report_individual_test_result(
                    result, ARTIFACT_DIR, CreateTestExpectations(),
                    FAKE_TEST_PATH, FAKE_TEST_LINE, 'test_name_prefix.')
                self.assertEqual(status, 0)

    def testBatchResultsCancel(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        rsr._post = StubWithRetval(0)
        with self.assertRaises(RuntimeError):
            with rsr.batch_results():
                result = CreateResult({
                    'name': 'test_name',
                    'actual': json_results.ResultType.Timeout,
                })
                status = rsr.report_individual_test_result(
                    result, ARTIFACT_DIR, CreateTestExpectations(),
                    FAKE_TEST_PATH, FAKE_TEST_LINE, 'test_name_prefix.')
                self.assertEqual(status, 0)
                raise RuntimeError('should propagate past `batch_results()`')
        self.assertIsNone(rsr._post.args)

    def testBatchResultsNoResults(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        rsr._post = StubWithRetval(0)
        with rsr.batch_results():
            pass
        self.assertIsNone(rsr._post.args)

    def testBatchResultsCannotNest(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        rsr._post = StubWithRetval(0)
        with rsr.batch_results():
            with self.assertRaisesRegex(result_sink.ResultSinkError,
                                        '`batch_results\(\)` cannot be nested'):
                with rsr.batch_results():
                    pass

    def testReportIndividualTestResultFailureReason(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        result = CreateResult({
            'name': 'test_name',
            'actual': json_results.ResultType.Failure,
            'failure_reason': json_results.FailureReason(
                    'Got "foo", want "bar"')
        })
        rsr._post = StubWithRetval(2)
        retval = rsr.report_individual_test_result(
                result, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
                FAKE_TEST_LINE, 'test_name_prefix.')
        self.assertEqual(retval, 2)
        expected_result = CreateExpectedTestResult(
                status=json_results.ResultType.Failure,
                expected=False,
                primary_error_message='Got "foo", want "bar"')
        self.assertEqual(GetTestResultFromPostedJson(rsr._post.args[1]),
                         expected_result)

    def testReportIndividualTestResultNoTestExpectations(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        result = CreateResult({
            'name': 'test_name',
            'actual': json_results.ResultType.Pass,
        })
        rsr._post = StubWithRetval(2)
        retval = rsr.report_individual_test_result(
                result, ARTIFACT_DIR, None, FAKE_TEST_PATH, FAKE_TEST_LINE,
                'test_name_prefix.')
        self.assertEqual(retval, 2)
        expected_result = CreateExpectedTestResult(tags=[
            {'key': 'test_name', 'value': 'test_name'},
            {'key': 'typ_expectation', 'value': json_results.ResultType.Pass},
            {'key': 'raw_typ_expectation', 'value': 'Pass'},
        ])
        self.assertEqual(GetTestResultFromPostedJson(rsr._post.args[1]),
                         expected_result)

    def testReportIndividualTestResultAdditionalTags(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        result = CreateResult({
            'name': 'test_name',
            'actual': json_results.ResultType.Pass,
        })
        rsr._post = StubWithRetval(2)
        retval = rsr.report_individual_test_result(
                result, ARTIFACT_DIR, None, FAKE_TEST_PATH, FAKE_TEST_LINE,
                'test_name_prefix.', {'additional_tag_key': 'additional_tag_value'})
        self.assertEqual(retval, 2)
        expected_result = CreateExpectedTestResult(tags=[
            {'key': 'test_name', 'value': 'test_name'},
            {'key': 'typ_expectation', 'value': json_results.ResultType.Pass},
            {'key': 'raw_typ_expectation', 'value': 'Pass'},
            {'key': 'additional_tag_key', 'value': 'additional_tag_value'},
        ])
        self.assertEqual(GetTestResultFromPostedJson(rsr._post.args[1]),
                         expected_result)

    def testReportIndividualTestResultAdditionalTagsNoStrings(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        rsr._post = StubWithRetval(0)
        result = CreateResult({
            'name': 'test_name',
            'actual': json_results.ResultType.Pass,
        })
        with self.assertRaises(AssertionError):
            rsr.report_individual_test_result(
                    result, ARTIFACT_DIR, None, FAKE_TEST_PATH,
                    FAKE_TEST_LINE, 'test_name_prefix.', {1: 'str'})
        with self.assertRaises(AssertionError):
            rsr.report_individual_test_result(
                    result, ARTIFACT_DIR, None, FAKE_TEST_PATH,
                    FAKE_TEST_LINE, 'test_name_prefix.', {'str': 1})

    def testReportIndividualTestResultHtmlSummaryUnicode(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        result = CreateResult({
            'name': 'test_name',
            'actual': json_results.ResultType.Pass,
            'out': 'stdout\u00A5',
            'err': 'stderr\u00A5',
        })
        rsr._post = StubWithRetval(0)
        rsr.report_individual_test_result(
                result, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
                FAKE_TEST_LINE, 'test_name_prefix.')

        test_result = GetTestResultFromPostedJson(rsr._post.args[1])
        expected_result = CreateExpectedTestResult(
            artifacts={
                'typ_stdout': {
                    'contents': base64.b64encode('stdout\u00A5'.encode('utf-8')).decode('utf-8')
                },
                'typ_stderr': {
                    'contents': base64.b64encode('stderr\u00A5'.encode('utf-8')).decode('utf-8')
                }
            })
        self.assertEqual(test_result, expected_result)

    def testReportIndividualTestResultConflictingOutputKeys(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        rsr._post = lambda: 1 / 0
        result = CreateResult({
            'name': 'test_name',
            'actual': 'json_results.ResultType.Pass',
            'artifacts': {
                'typ_stdout': [''],
            },
        })
        with self.assertRaises(AssertionError):
            rsr.report_individual_test_result(
                result, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
                FAKE_TEST_LINE, 'test_name_name_prefix')
        result = CreateResult({
            'name': 'test_name',
            'actual': 'json_results.ResultType.Pass',
            'artifacts': {
                'typ_stderr': [''],
            },
        })
        with self.assertRaises(AssertionError):
            rsr.report_individual_test_result(
                result, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
                FAKE_TEST_LINE, 'test_name_name_prefix')

    def testReportIndividualTestResultSingleArtifact(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        rsr._post = StubWithRetval(2)
        results = CreateResult({
            'name': 'test_name',
            'actual': json_results.ResultType.Pass,
            'artifacts': {
                'artifact_name': ['some_artifact'],
            }
        })
        retval = rsr.report_individual_test_result(
                results, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
                FAKE_TEST_LINE, 'test_name_prefix.')
        self.assertEqual(retval, 2)

        test_result = GetTestResultFromPostedJson(rsr._post.args[1])
        expected_artifacts = {
            'artifact_name': {
                'filePath': self._host.join(self._host.getcwd(),
                                            ARTIFACT_DIR,
                                            'some_artifact'),
            },
        }
        expected_artifacts.update(STDOUT_STDERR_ARTIFACTS)
        expected_result = CreateExpectedTestResult(
                artifacts=expected_artifacts)
        self.assertEqual(test_result, expected_result)

    def testReportIndividualTestResultMultipleArtifacts(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        rsr._post = StubWithRetval(2)
        results = CreateResult({
            'name': 'test_name',
            'actual': json_results.ResultType.Pass,
            'artifacts': {
                'artifact_name': ['some_artifact', 'another_artifact'],
            },
        })
        retval = rsr.report_individual_test_result(
                results, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
                FAKE_TEST_LINE, 'test_name_prefix.')
        self.assertEqual(retval, 2)

        test_result = GetTestResultFromPostedJson(rsr._post.args[1])
        expected_artifacts = {
            'artifact_name-file0': {
                'filePath': self._host.join(self._host.getcwd(),
                                            ARTIFACT_DIR,
                                            'some_artifact'),
            },
            'artifact_name-file1': {
                'filePath': self._host.join(self._host.getcwd(),
                                            ARTIFACT_DIR,
                                            'another_artifact'),
            },
        }
        expected_artifacts.update(STDOUT_STDERR_ARTIFACTS)
        expected_result = CreateExpectedTestResult(
                artifacts=expected_artifacts)
        self.assertEqual(test_result, expected_result)

    def testReportIndividualTestResultHttpsArtifact(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        rsr._post = StubWithRetval(2)
        results = CreateResult({
            'name': 'test_name',
            'actual': json_results.ResultType.Pass,
            'artifacts': {
                'artifact_name': ['https://somelink.com'],
            }
        })
        retval = rsr.report_individual_test_result(
                results, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
                FAKE_TEST_LINE, 'test_name_prefix.')
        self.assertEqual(retval, 2)

        test_result = GetTestResultFromPostedJson(rsr._post.args[1])
        expected_artifacts = {}
        expected_artifacts.update(STDOUT_STDERR_ARTIFACTS)
        expected_html_summary = (
            '<a href=https://somelink.com>artifact_name</a>'
            + HTML_SUMMARY)
        expected_result = CreateExpectedTestResult(
                artifacts=expected_artifacts,
                summary_html=expected_html_summary)
        self.assertEqual(test_result, expected_result)

    def testResultIndividualTestResultNoStdout(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        rsr._post = StubWithRetval(2)
        results = CreateResult({
            'name': 'test_name',
            'actual': json_results.ResultType.Pass,
            'artifacts': {},
            'out': '',
        })
        retval = rsr.report_individual_test_result(
                results, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
                FAKE_TEST_LINE, 'test_name_prefix.')
        self.assertEqual(retval, 2)

        test_result = GetTestResultFromPostedJson(rsr._post.args[1])
        expected_result = CreateExpectedTestResult(
            summary_html='<p><text-artifact artifact-id="typ_stderr"/></p>')
        expected_result['artifacts'] = {
            'typ_stderr': STDOUT_STDERR_ARTIFACTS['typ_stderr'],
        }
        self.assertEqual(test_result, expected_result)

    def testReportIndividualTestResultNoStdoutOrStderr(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        rsr._post = StubWithRetval(2)
        results = CreateResult({
            'name': 'test_name',
            'actual': json_results.ResultType.Pass,
            'artifacts': {},
            'out': '',
            'err': '',
        })
        retval = rsr.report_individual_test_result(
                results, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
                FAKE_TEST_LINE, 'test_name_prefix.')
        self.assertEqual(retval, 2)

        test_result = GetTestResultFromPostedJson(rsr._post.args[1])
        expected_result = CreateExpectedTestResult(summary_html='')
        expected_result['artifacts'] = {}
        self.assertEqual(test_result, expected_result)

    def testReportIndividualTestResultCustomSummary(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        rsr._post = StubWithRetval(2)
        results = CreateResult({
            'name': 'test_name',
            'actual': json_results.ResultType.Pass,
            'artifacts': {
                'artifact_name': ['https://somelink.com'],
            }
        })
        html_summary = '<h3>Overrides the default summary</h3>'
        retval = rsr.report_individual_test_result(
                results, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
                FAKE_TEST_LINE, 'test_name_prefix.',
                html_summary=html_summary)
        self.assertEqual(retval, 2)

        test_result = GetTestResultFromPostedJson(rsr._post.args[1])
        expected_artifacts = {}
        expected_artifacts.update(STDOUT_STDERR_ARTIFACTS)
        expected_result = CreateExpectedTestResult(
                artifacts=expected_artifacts,
                summary_html=html_summary)
        self.assertEqual(test_result, expected_result)

    def testReportIndividualTestResultLongTestName(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        test_name = 'a' * (result_sink.MAX_TAG_LENGTH + 1)
        result = CreateResult({
            'name': test_name,
            'actual': json_results.ResultType.Pass,
        })
        rsr._post = StubWithRetval(0)
        _ = rsr.report_individual_test_result(
            result, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
            FAKE_TEST_LINE, 'test_name_prefix.')
        expected_results = CreateExpectedTestResult(
            test_id=('test_name_prefix.' + test_name))
        index = -1
        for i, tag_dict in enumerate(expected_results['tags']):
            if tag_dict['key'] == 'test_name':
                index = i
                break
        self.assertGreater(index, -1)
        m = hashlib.sha1()
        m.update(test_name.encode('utf-8'))
        expected_name = ('a' * 216) + m.hexdigest()
        expected_results['tags'][index] = {
            'key': 'test_name',
            'value': expected_name
        }
        self.assertEqual(GetTestResultFromPostedJson(rsr._post.args[1]),
                         expected_results)

    def testReportIndividualTestResultTestNameAtLengthLimit(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        test_name = 'a' * result_sink.MAX_TAG_LENGTH
        result = CreateResult({
            'name': test_name,
            'actual': json_results.ResultType.Pass,
        })
        rsr._post = StubWithRetval(0)
        _ = rsr.report_individual_test_result(
            result, ARTIFACT_DIR, CreateTestExpectations(), FAKE_TEST_PATH,
            FAKE_TEST_LINE, 'test_name_prefix.')
        expected_results = CreateExpectedTestResult(
            test_id=('test_name_prefix.' + test_name))
        self.assertEqual(GetTestResultFromPostedJson(rsr._post.args[1]),
                         expected_results)

    def testReportResultEarlyReturnIfNotSupported(self):
        self.setLuciContextWithContent({})
        rsr = result_sink.ResultSinkReporter(self._host)
        # We need to keep a reference to this and restore it later when we're
        # done testing, otherwise subsequent tests can end up failing due to
        # trying to use the monkey patched function.
        original_function = result_sink._create_json_test_result
        result_sink._create_json_test_result = lambda: 1/0
        try:
            self.assertEqual(rsr._report_result(
                    'test_id', json_results.ResultType.Pass, True, {}, {},
                    '<pre>summary</pre>', 1, {}, None), 0, {})
        finally:
            result_sink._create_json_test_result = original_function

    def testCreateJsonTestResultInvalidStatus(self):
        with self.assertRaises(ValueError):
            result_sink._create_json_test_result(
                'test_id', 'InvalidStatus', False, {}, {}, '', 1, {}, None)

    def testCreateJsonTestResultBasic(self):
        retval = result_sink._create_json_test_result(
            'test_id', json_results.ResultType.Failure, True,
            {'artifact': {'filePath': 'somepath'}},
            [('tag_key', 'tag_value')], '<pre>summary</pre>', 1,
            {'name': 'test_name', 'location': {'repo': 'a repo'}},
            json_results.FailureReason('got "foo", want "bar"'))
        self.assertEqual(retval, {
            'testId': 'test_id',
            'status': json_results.ResultType.Failure,
            'expected': True,
            'duration': '1.000000000s',
            'summaryHtml': '<pre>summary</pre>',
            'artifacts': {
                'artifact': {
                    'filePath': 'somepath',
                },
            },
            'tags': [
                {
                    'key': 'tag_key',
                    'value': 'tag_value',
                },
            ],
            'testMetadata': {
                'name': 'test_name',
                'location': {
                    'repo': 'a repo',
                },
            },
            'failureReason': {
                'primaryErrorMessage': 'got "foo", want "bar"',
            },
        })

    def testCreateJsonWithVerySmallDuration(self):
        retval = result_sink._create_json_test_result(
            'test_id', json_results.ResultType.Pass, True,
            {'artifact': {'filePath': 'somepath'}},
            [('tag_key', 'tag_value')], '<pre>summary</pre>', 1e-10, {}, None)
        self.assertEqual(retval['duration'], '0.000000000s')

    def testCreateJsonFormatsWithVeryLongDuration(self):
        retval = result_sink._create_json_test_result(
            'test_id', json_results.ResultType.Pass, True,
            {'artifact': {'filePath': 'somepath'}},
            [('tag_key', 'tag_value')], '<pre>summary</pre>', 1e+16, {}, None)
        self.assertEqual(retval['duration'], '10000000000000000.000000000s')

    def testTruncateBasicCase(self):
        input = 'a' * 1050
        actual = result_sink._truncate_to_utf8_bytes(input, 1024)
        self.assertEqual(actual, ('a' * 1021) + '...')

    def testTruncateUTF8(self):
        # Swedish "Place of interest symbol", which encodes as 3 bytes.
        poi = b'\xE2\x8C\x98'.decode('utf-8')
        input = poi * 350
        actual = result_sink._truncate_to_utf8_bytes(input, 1024)
        # Output should be slightly less than 1024 as only whole
        # UTF8 code points should have been removed.
        self.assertEqual(len(actual.encode('utf-8')), 1023)
        self.assertEqual(actual, (poi * 340) + '...')

    def testConvertPathToRepoPath(self):
        self.setLuciContextWithContent(DEFAULT_LUCI_CONTEXT)
        rsr = ResultSinkReporterWithFakeSrc(self._host)
        p = '/src/some/file.txt'
        repo_path = rsr._convert_path_to_repo_path(p)
        self.assertEqual(repo_path, '//some/file.txt')

        rsr._chromium_src_dir = 'c:\\src\\'
        self._host.sep = '\\'
        p = 'c:\\src\\some\\file.txt'
        repo_path = rsr._convert_path_to_repo_path(p)
        self.assertEqual(repo_path, '//some/file.txt')
