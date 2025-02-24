# Copyright 2014 Dirk Pranke. All rights reserved.
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

import inspect
import json
import os
import sys
import tempfile
import unittest

from textwrap import dedent as d


from typ import Host, Runner, Stats, TestCase, TestSet, TestInput
from typ import WinMultiprocessing
from typ import runner as runner_module
from typ.fakes import host_fake
from typ.tests.stub_test_func import stub_test_func


def _setup_process(child, context):  # pylint: disable=W0613
    return context


def _teardown_process(child, context):  # pylint: disable=W0613
    return context

def _teardown_throws(child, context):  # pylint: disable=W0613
    raise Exception("exception in teardown")

class MockTestCase(unittest.TestCase):

    def test_pass(self):
        pass


class MockArgs(object):

    def __init__(
        self, test_name_prefix='', skip_globs=None,
        isolate_globs=None,test_filter='', all=False):
        cls = MockTestCase('test_pass').__class__
        self.test_name_prefix = (
            test_name_prefix or '%s.%s.' % (cls.__module__, cls.__name__))
        self.skip = skip_globs or []
        self.isolate = isolate_globs or []
        self.tests = []
        self.all = all
        self.test_filter = test_filter


def _PrefixDoesMatch(runner):
    test_set = TestSet(runner.args.test_name_prefix)
    runner.default_classifier(test_set, MockTestCase('test_pass'))
    return test_set


class RunnerTests(TestCase):

    def _PrefixDoesNotMatch(self, runner):
        test_set = TestSet(runner.args.test_name_prefix)
        with self.assertRaises(AssertionError) as context:
            runner.default_classifier(test_set, MockTestCase('test_pass'))
        self.assertIn(
            'The test prefix passed at the command line does not match the prefix '
            'of all the tests generated', str(context.exception))

    def test_test_filter_arg(self):
        runner = Runner()
        runner.args = MockArgs(test_filter='test_pass')
        test_set = _PrefixDoesMatch(runner)
        self.assertEqual(len(test_set.parallel_tests), 1)

    def test_test_filter_arg_causes_assertion(self):
        runner = Runner()
        runner.args = MockArgs(test_name_prefix='DontMatch')
        self._PrefixDoesNotMatch(runner)

    def test_skip_arg(self):
        runner = Runner()
        runner.args = MockArgs(skip_globs=['test_pas*'], test_filter='test_pass')
        test_set = _PrefixDoesMatch(runner)
        self.assertEqual(len(test_set.tests_to_skip), 1)

    def test_skip_arg_causes_assertion(self):
        runner = Runner()
        runner.args = MockArgs(
            test_name_prefix='DontMatch', skip_globs=['test_pas*'])
        self._PrefixDoesNotMatch(runner)

    def test_isolate_arg(self):
        runner = Runner()
        runner.args = MockArgs(isolate_globs=['test_pas*'], test_filter='test_pass')
        test_set = _PrefixDoesMatch(runner)
        self.assertEqual(len(test_set.isolated_tests), 1)

    def test_isolate_arg_causes_assertion(self):
        runner = Runner()
        runner.args = MockArgs(
            test_name_prefix='DontMatch', isolate_globs=['test_pas*'],
            test_filter='test_pass')
        self._PrefixDoesNotMatch(runner)

    def test_context(self):
        r = Runner()
        r.args.tests = ['typ.tests.runner_test.ContextTests']
        r.context = {'foo': 'bar'}
        r.setup_fn = _setup_process
        r.teardown_fn = _teardown_process
        r.win_multiprocessing = WinMultiprocessing.importable
        ret, _, _ = r.run()
        self.assertEqual(ret, 0)

    @unittest.skipIf(sys.version_info.major == 3, 'fails under python3')
    def test_exception_in_teardown(self):
        r = Runner()
        r.args.tests = ['typ.tests.runner_test.ContextTests']
        r.context = {'foo': 'bar'}
        r.setup_fn = _setup_process
        r.teardown_fn = _teardown_throws
        r.win_multiprocessing = WinMultiprocessing.importable
        ret, _, _ = r.run()
        self.assertEqual(ret, 0)
        self.assertEqual(r.final_responses[0][2].message,
                         'exception in teardown')

    def test_bad_default(self):
        r = Runner()
        ret = r.main([], foo='bar')
        self.assertEqual(ret, 2)

    def test_good_default(self):
        r = Runner()
        ret = r.main([], tests=['typ.tests.runner_test.ContextTests'])
        self.assertEqual(ret, 0)

    def test_max_failures_fail_if_equal(self):
        r = Runner()
        r.args.tests = ['typ.tests.runner_test.FailureTests']
        r.args.jobs = 1
        r.args.typ_max_failures = 1
        r.context = True
        with self.assertRaises(RuntimeError):
            r.run()

    def test_max_failures_pass_if_under(self):
        r = Runner()
        r.args.tests = [
            'typ.tests.runner_test.ContextTests',
            'typ.tests.runner_test.FailureTests'
        ]
        r.args.jobs = 1
        r.args.typ_max_failures = 2
        r.context = False
        r.run()

    def test_max_failures_ignored_if_unset(self):
        r = Runner()
        r.args.tests = ['typ.tests.runner_test.FailureTests']
        r.args.jobs = 1
        r.args.typ_max_failures = None
        r.context = True
        r.run()

    def test_skip_test(self):
        r = Runner()
        r.args.tests = ['typ.tests.runner_test.SkipTests']
        r.args.jobs = 1
        ret, full_results, _ = r.run()
        self.assertEqual(ret, 0)
        self.assertEqual(
            full_results['num_failures_by_type'],
            {'FAIL': 0, 'TIMEOUT': 0, 'CRASH': 0, 'PASS': 0, 'SKIP': 1})
        result = full_results['tests']['typ']['tests']['runner_test']['SkipTests']['test_skip']
        self.assertEqual(result['expected'], 'SKIP')
        self.assertEqual(result['actual'], 'SKIP')

    def test_use_real_test_func_attribute(self):
      fd, trace_filepath = tempfile.mkstemp(prefix='trace', suffix='.json')
      os.close(fd)
      test_name = ('typ.tests.runner_test.RealTestFuncTests'
                   '.test_use_real_test_func_attribute')
      try:
        r = Runner()
        r.args.tests = [test_name]
        r.args.write_trace_to = trace_filepath
        r.args.jobs = 1
        r.args.typ_max_failures = None
        r.context = True
        r.run()

        with open(trace_filepath, 'r') as f:
          trace = json.load(f)
        test_trace_event = [event for event in trace['traceEvents']
                            if event['name'] == test_name][0]

        actual_test_source_filepath = test_trace_event['args']['file']
        actual_test_source_line = test_trace_event['args']['line']
        expected_test_source_filepath = inspect.getsourcefile(stub_test_func)
        expected_test_source_line = inspect.getsourcelines(stub_test_func)[1]

        self.assertEqual(actual_test_source_filepath,
                         expected_test_source_filepath)
        self.assertEqual(actual_test_source_line,
                         expected_test_source_line)
      finally:
        os.remove(trace_filepath)


class FailureReasonExtractionTests(TestCase):
    def test_basecase(self):
        input = """Traceback (most recent call last):
  File "C:\somepath\my_test.py", line 45, in testSomething
    self.assertGreater(samples[0], 0, 'Sample from %s was not > 0' % name)
AssertionError: 0 not greater than 0 : Sample from rasterize_time was not > 0
"""
        fr = runner_module._failure_reason_from_traceback(input)
        self.assertIsNotNone(fr)
        self.assertEqual(fr.primary_error_message,
            'my_test.py(45): AssertionError: 0 not greater than 0 : Sample from rasterize_time was not > 0')

    def test_not_extractable(self):
        input = """Traceback (most recent call last):"""
        fr = runner_module._failure_reason_from_traceback(input)
        self.assertIsNone(fr)

    def test_trailing_stderr(self):
        input = """Traceback (most recent call last):
  File "C:\somepath\my_test.py", line 45, in testSomething
    self.assertSomething(...)
AssertionError: Wanted "foo", got "bar"
Stderr:
This output should be ignored."""
        fr = runner_module._failure_reason_from_traceback(input)
        self.assertIsNotNone(fr)
        self.assertEqual(fr.primary_error_message,
            'my_test.py(45): AssertionError: Wanted "foo", got "bar"')

    def test_trailing_stdout(self):
        input = """Traceback (most recent call last):
  File "C:\somepath\my_test.py", line 45, in testSomething
    self.assertSomething(...)
AssertionError: Wanted "foo", got "bar"
Stdout:
This output should be ignored.
"""
        fr = runner_module._failure_reason_from_traceback(input)
        self.assertIsNotNone(fr)
        self.assertEqual(fr.primary_error_message,
            'my_test.py(45): AssertionError: Wanted "foo", got "bar"')

    def test_chained_traceback(self):
        input = """
Traceback (most recent call last):
  File "/b/s/w/ir/third_party/catapult/telemetry/telemetry/internal/browser/browser.py", line 145, in Close
    if self._browser_backend.IsBrowserRunning():
  File "/b/s/w/ir/third_party/catapult/telemetry/telemetry/core/cros_interface.py", line 481, in ListProcesses
    assert stderr == '', stderr
AssertionError: this message should not be extracted.


During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/b/s/w/ir/third_party/catapult/common/py_utils/py_utils/exc_util.py", line 79, in Wrapper
    func(*args, **kwargs)
  File "/b/s/w/ir/third_party/catapult/telemetry/telemetry/core/cros_interface.py", line 481, in ListProcesses
    assert stderr == '', stderr
AssertionError: ssh: connect to host 127.0.0.1 port 9222: Connection timed out
"""
        fr = runner_module._failure_reason_from_traceback(input)
        self.assertIsNotNone(fr)
        self.assertEqual(fr.primary_error_message,
            'cros_interface.py(481): AssertionError: ssh: connect to host 127.0.0.1 port 9222: Connection timed out')


class TestSetTests(TestCase):
    # This class exists to test the failures that can come up if you
    # create your own test sets and bypass find_tests(); failures that
    # would normally be caught there can occur later during test execution.

    def test_missing_name(self):
        test_set = TestSet(MockArgs())
        test_set.parallel_tests = [TestInput('nonexistent test')]
        r = Runner()
        r.args.jobs = 1
        ret, _, _ = r.run(test_set)
        self.assertEqual(ret, 1)

    def test_failing_load_test(self):
        h = Host()
        orig_wd = h.getcwd()
        tmpdir = None
        try:
            tmpdir = h.mkdtemp()
            h.chdir(tmpdir)
            h.write_text_file('load_test.py', d("""\
                import unittest
                def load_tests(_, _2, _3):
                    assert False
                """))
            test_set = TestSet(MockArgs())
            test_set.parallel_tests = [TestInput('load_test.BaseTest.test_x')]
            r = Runner()
            r.args.jobs = 1
            ret, _, trace = r.run(test_set)
            self.assertEqual(ret, 1)
            self.assertIn('BaseTest',
                          trace['traceEvents'][0]['args']['err'])
        finally:
            h.chdir(orig_wd)
            if tmpdir:
                h.rmtree(tmpdir)


class TestWinMultiprocessing(TestCase):
    def make_host(self):
        return Host()

    def call(self, argv, platform=None, win_multiprocessing=None, **kwargs):
        h = self.make_host()
        orig_wd = h.getcwd()
        tmpdir = None
        try:
            tmpdir = h.mkdtemp()
            h.chdir(tmpdir)
            h.capture_output()
            if platform is not None:
                h.platform = platform
            r = Runner(h)
            if win_multiprocessing is not None:
                r.win_multiprocessing = win_multiprocessing
            ret = r.main(argv, **kwargs)
        finally:
            out, err = h.restore_output()
            h.chdir(orig_wd)
            if tmpdir:
                h.rmtree(tmpdir)

        # Ignore the new logging added for timing.
        if out.startswith('Start running tests'):
            out = '\n'.join(out.split('\n')[1:])

        return ret, out, err

    def test_bad_value(self):
        self.assertRaises(ValueError, self.call, [], win_multiprocessing='foo')

    def test_ignore(self):
        h = self.make_host()
        if h.platform == 'win32':  # pragma: win32
            self.assertRaises(ValueError, self.call, [],
                              win_multiprocessing=WinMultiprocessing.ignore)
        else:
            result = self.call([],
                               win_multiprocessing=WinMultiprocessing.ignore)
            ret, out, err = result
            self.assertEqual(ret, 0)
            self.assertEqual(out, '0 tests passed, 0 skipped, 0 failures.\n')
            self.assertEqual(err, '')

    def test_real_unimportable_main(self):
        h = self.make_host()
        tmpdir = None
        orig_wd = h.getcwd()
        out = err = None
        out_str = err_str = ''
        try:
            tmpdir = h.mkdtemp()
            h.chdir(tmpdir)
            out = tempfile.NamedTemporaryFile(delete=False)
            err = tempfile.NamedTemporaryFile(delete=False)
            path_above_typ = h.realpath(h.dirname(__file__), '..', '..')
            env = h.env.copy()
            if 'PYTHONPATH' in env:  # pragma: untested
                env['PYTHONPATH'] = '%s%s%s' % (env['PYTHONPATH'],
                                                h.pathsep,
                                                path_above_typ)
            else:  # pragma: untested.
                env['PYTHONPATH'] = path_above_typ

            h.write_text_file('test', d("""
                import sys
                import typ
                importable = typ.WinMultiprocessing.importable
                sys.exit(typ.main(win_multiprocessing=importable))
                """))
            h.stdout = out
            h.stderr = err
            ret = h.call_inline([h.python_interpreter, h.join(tmpdir, 'test')],
                                env=env)
        finally:
            h.chdir(orig_wd)
            if tmpdir:
                h.rmtree(tmpdir)
            if out:
                out.close()
                out = open(out.name)
                out_str = out.read()
                out.close()
                h.remove(out.name)
            if err:
                err.close()
                err = open(err.name)
                err_str = err.read()
                err.close()
                h.remove(err.name)

        self.assertEqual(ret, 1)
        self.assertEqual(out_str, '')
        self.assertIn('ValueError: The __main__ module ',
                      err_str)

    def test_single_job(self):
        ret, out, err = self.call(['-j', '1'], platform='win32')
        self.assertEqual(ret, 0)
        self.assertEqual('0 tests passed, 0 skipped, 0 failures.\n', out )
        self.assertEqual(err, '')

    def test_spawn(self):
        ret, out, err = self.call([])
        self.assertEqual(ret, 0)
        self.assertEqual('0 tests passed, 0 skipped, 0 failures.\n', out)
        self.assertEqual(err, '')


class ContextTests(TestCase):
    def test_context(self):
        # This test is mostly intended to be called by
        # RunnerTests.test_context, above. It is not interesting on its own.
        if self.context:
            self.assertEquals(self.context['foo'], 'bar')


class FailureTests(TestCase):
    def test_failure(self):
        # Intended to be called from tests above.
        if self.context:
            self.fail()


class SkipTests(TestCase):
    def test_skip(self):
        self.programmaticSkipIsExpected = True
        self.skipTest('Skipped test')


def test_func(self):
    # This test is mostly intended to be called by
    # RunnerTests.test_use_real_test_func_attribute, above.
    # It is not interesting on its own.
    pass


class RealTestFuncTests(TestCase):
    pass


setattr(test_func, 'real_test_func', stub_test_func)
setattr(RealTestFuncTests, 'test_use_real_test_func_attribute', test_func)
