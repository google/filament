# Copyright 2014 Google Inc. All rights reserved.
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

import fnmatch
import importlib
import inspect
import json
import os
import pdb
import re
import sys
import unittest
import traceback
from datetime import datetime

from collections import OrderedDict

# This ensures that absolute imports of typ modules will work when
# running typ/runner.py as a script even if typ is not installed.
# We need this entry in addition to the one in __main__.py to ensure
# that typ/runner.py works when invoked via subprocess on windows in
# _spawn_main().
path_to_file = os.path.realpath(__file__)
if path_to_file.endswith('.pyc'):  # pragma: no cover
    path_to_file = path_to_file[:-1]
dir_above_typ = os.path.dirname(os.path.dirname(path_to_file))
dir_cov = os.path.join(os.path.dirname(dir_above_typ), 'coverage')
for path in (dir_above_typ, dir_cov):
    if path not in sys.path:  # pragma: no cover
        sys.path.append(path)

from typ import artifacts
from typ import json_results
from typ import result_sink
from typ.arg_parser import ArgumentParser
from typ.expectations_parser import TestExpectations, Expectation
from typ.host import Host
from typ.pool import make_pool_group
from typ.stats import Stats
from typ.printer import Printer
from typ.test_case import TestCase as TypTestCase
from typ.version import VERSION


Result = json_results.Result
ResultSet = json_results.ResultSet
ResultType = json_results.ResultType
FailureReason = json_results.FailureReason

# Matches the first line of stack entries in formatted Python tracebacks.
# The first capture group is the name of the file, the second is the line.
# The method name is not extracted.
# See: https://github.com/python/cpython/blob/3.10/Lib/traceback.py#L440
_TRACEBACK_FILE_RE = re.compile(r'^  File "[^"]*[/\\](.*)", line ([0-9]+), in ')


def main(argv=None, host=None, win_multiprocessing=None, **defaults):
    host = host or Host()
    runner = Runner(host=host)
    if win_multiprocessing is not None:
        runner.win_multiprocessing = win_multiprocessing
    return runner.main(argv, **defaults)


class TestInput(object):

    def __init__(self, name, msg='', timeout=None, expected=None, iteration=0):
        self.name = name
        self.msg = msg
        self.timeout = timeout
        self.expected = expected
        # Iteration makes more sense as part of the test run, not the test
        # input, but since the pool used to run tests persists across
        # iterations, we need to store the iteration number in something that
        # gets updated each test run, such as TestInput.
        self.iteration = iteration


class TestSet(object):

    def __init__(self, test_name_prefix='', iteration=0):
        self.test_name_prefix = test_name_prefix
        self.parallel_tests = []
        self.isolated_tests = []
        self.tests_to_skip = []
        self.iteration = iteration

    def copy(self):
        test_set = TestSet(self.test_name_prefix)
        test_set.tests_to_skip = self.tests_to_skip[:]
        test_set.isolated_tests = self.isolated_tests[:]
        test_set.parallel_tests = self.parallel_tests[:]
        return test_set

    def _get_test_name(self, test_case):
        _validate_test_starts_with_prefix(
            self.test_name_prefix, test_case.id())
        return test_case.id()[len(self.test_name_prefix):]

    def add_test_to_skip(self, test_case, reason=''):
        self.tests_to_skip.append(
            TestInput(self._get_test_name(
                test_case), reason, iteration=self.iteration))

    def add_test_to_run_isolated(self, test_case):
        self.isolated_tests.append(
            TestInput(self._get_test_name(test_case), iteration=self.iteration))

    def add_test_to_run_in_parallel(self, test_case):
        self.parallel_tests.append(
            TestInput(self._get_test_name(test_case), iteration=self.iteration))


def _validate_test_starts_with_prefix(prefix, test_name):
    assert test_name.startswith(prefix), (
        'The test prefix passed at the command line does not match the prefix '
        'of all the tests generated')


class WinMultiprocessing(object):
    ignore = 'ignore'
    importable = 'importable'
    spawn = 'spawn'

    values = [ignore, importable, spawn]


class _AddTestsError(Exception):
    pass


class Runner(object):

    def __init__(self, host=None):
        self.args = None
        self.classifier = None
        self.cov = None
        self.context = None
        self.coverage_source = None
        self.host = host or Host()
        self.loader = unittest.loader.TestLoader()
        self.printer = None
        self.setup_fn = None
        self.stats = None
        self.teardown_fn = None
        self.top_level_dir = None
        self.top_level_dirs = []
        self.win_multiprocessing = WinMultiprocessing.spawn
        self.final_responses = []
        self.has_expectations = False
        self.expectations = None
        self.metadata = {}
        self.path_delimiter = json_results.DEFAULT_TEST_SEPARATOR
        self.artifact_output_dir = None
        self.tag_conflict_checker = None

        # initialize self.args to the defaults.
        parser = ArgumentParser(self.host)
        self.parse_args(parser, [])

    def main(self, argv=None, **defaults):
        parser = ArgumentParser(self.host)
        self.parse_args(parser, argv, **defaults)
        if parser.exit_status is not None:
            return parser.exit_status

        try:
            ret, _, _ = self.run()
            return ret
        except KeyboardInterrupt:
            self.print_("interrupted, exiting", stream=self.host.stderr)
            return 130

    def parse_args(self, parser, argv, **defaults):
        for attrname in defaults:
            if not hasattr(self.args, attrname):
                parser.error("Unknown default argument name '%s'" % attrname,
                             bailout=False)
                return
        parser.set_defaults(**defaults)
        self.args = parser.parse_args(args=argv)
        if parser.exit_status is not None:
            return

    def print_(self, msg='', end='\n', stream=None):
        self.host.print_(msg, end, stream=stream)

    def run(self, test_set=None):

        ret = 0
        h = self.host

        if self.args.version:
            self.print_(VERSION)
            return ret, None, None

        if self.args.write_full_results_to:
            self.artifact_output_dir = os.path.join(
                    os.path.dirname(
                            self.args.write_full_results_to), 'artifacts')

        should_spawn = self._check_win_multiprocessing()
        if should_spawn:
            return self._spawn(test_set)

        ret = self._set_up_runner()
        if ret:
            return ret, None, None

        find_start = h.time()
        if self.cov:  # pragma: no cover
            self.cov.erase()
            self.cov.start()

        full_results = None
        result_set = ResultSet()

        if not test_set:
            ret, test_set = self.find_tests(self.args)
        find_end = h.time()

        if not ret:
            self.stats.total = (len(test_set.parallel_tests) +
                                len(test_set.isolated_tests) +
                                len(test_set.tests_to_skip)) * self.args.repeat
            all_tests = [ti.name for ti in
            _sort_inputs(test_set.parallel_tests +
                         test_set.isolated_tests +
                         test_set.tests_to_skip)]
            self.metadata = {tup[0]:tup[1]
                             for  tup in
                             [md.split('=', 1) for md in self.args.metadata]}
            if self.args.test_name_prefix:
                self.metadata['test_name_prefix'] = self.args.test_name_prefix
            if self.args.tags:
                self.metadata['tags'] = self.args.tags
            if self.args.expectations_files:
                self.metadata['expectations_files'] = [
                        os.path.basename(exp)
                        if not self.args.repository_absolute_path
                        else ('//' + os.path.relpath(
                              exp, self.args.repository_absolute_path).replace(
                                  os.path.sep, '/'))
                        for exp in self.args.expectations_files]
            if self.args.list_only:
                self.print_('\n'.join(all_tests))
            else:
                self.print_('Start running tests: %s' % str(datetime.now()))
                for _ in range(self.args.repeat):
                    current_ret, full_results=self._run_tests(
                        result_set, test_set.copy(), all_tests)
                    ret = ret or current_ret

        if self.cov:  # pragma: no cover
            self.cov.stop()
            self.cov.save()
        test_end = h.time()

        trace = self._trace_from_results(result_set)
        if full_results:
            self._summarize(full_results)
            self._write(self.args.write_full_results_to, full_results)
            upload_ret = self._upload(full_results)
            reporting_end = h.time()
            self._add_trace_event(trace, 'run', find_start, reporting_end)
            self._add_trace_event(trace, 'discovery', find_start, find_end)
            self._add_trace_event(trace, 'testing', find_end, test_end)
            self._add_trace_event(trace, 'reporting', test_end, reporting_end)
            self._write(self.args.write_trace_to, trace)
            cov_ret = self.report_coverage() if self.args.coverage else 0
            # Exit with the code of the first failing step, but do not skip
            # any steps with short-circuiting.
            ret = ret or upload_ret or cov_ret

        return ret, full_results, trace

    def _check_win_multiprocessing(self):
        wmp = self.win_multiprocessing

        ignore, importable, spawn = WinMultiprocessing.values

        if wmp not in WinMultiprocessing.values:
            raise ValueError('illegal value %s for win_multiprocessing' %
                             wmp)

        h = self.host
        if wmp == ignore and h.platform == 'win32':  # pragma: win32
            raise ValueError('Cannot use WinMultiprocessing.ignore for '
                             'win_multiprocessing when actually running '
                             'on Windows.')

        if wmp == ignore or self.args.jobs == 1:
            return False

        if wmp == importable:
            if self._main_is_importable():
                return False
            raise ValueError('The __main__ module (%s) '  # pragma: no cover
                             'may not be importable' %
                             sys.modules['__main__'].__file__)

        assert wmp == spawn
        return True

    def _main_is_importable(self):  # pragma: untested
        path = sys.modules['__main__'].__file__
        if not path:
            return False
        if path.endswith('.pyc'):
            path = path[:-1]
        if not path.endswith('.py'):
            return False
        if path.endswith('__main__.py'):
            # main modules are not directly importable.
            return False

        path = self.host.realpath(path)
        for d in sys.path:
            if path.startswith(self.host.realpath(d)):
                return True
        return False  # pragma: no cover

    def _spawn(self, test_set):
        # TODO: Handle picklable hooks, rather than requiring them to be None.
        assert self.classifier is None
        assert self.context is None
        assert self.setup_fn is None
        assert self.teardown_fn is None
        assert test_set is None
        h = self.host

        if self.args.write_trace_to:  # pragma: untested
            should_delete_trace = False
        else:
            should_delete_trace = True
            fp = h.mktempfile(delete=False)
            fp.close()
            self.args.write_trace_to = fp.name

        if self.args.write_full_results_to:  # pragma: untested
            should_delete_results = False
        else:
            should_delete_results = True
            fp = h.mktempfile(delete=False)
            fp.close()
            self.args.write_full_results_to = fp.name

        argv = ArgumentParser(h).argv_from_args(self.args)
        ret = h.call_inline([h.python_interpreter, path_to_file] + argv)

        trace = self._read_and_delete(self.args.write_trace_to,
                                      should_delete_trace)
        full_results = self._read_and_delete(self.args.write_full_results_to,
                                             should_delete_results)
        return ret, full_results, trace

    def _set_up_runner(self):
        h = self.host
        args = self.args

        self.stats = Stats(args.status_format, h.time, args.jobs)
        self.printer = Printer(
            self.print_, args.overwrite, args.terminal_width)

        if self.args.top_level_dirs and self.args.top_level_dir:
            self.print_(
                'Cannot specify both --top-level-dir and --top-level-dirs',
                stream=h.stderr)
            return 1

        self.top_level_dirs = args.top_level_dirs
        if not self.top_level_dirs and args.top_level_dir:
            self.top_level_dirs = [args.top_level_dir]

        if not self.top_level_dirs:
            for test in [t for t in args.tests if h.exists(t)]:
                if h.isdir(test):
                    top_dir = test
                else:
                    top_dir = h.dirname(test)
                while h.exists(top_dir, '__init__.py'):
                    top_dir = h.dirname(top_dir)
                top_dir = h.realpath(top_dir)
                if not top_dir in self.top_level_dirs:
                    self.top_level_dirs.append(top_dir)
        if not self.top_level_dirs:
            top_dir = h.getcwd()
            while h.exists(top_dir, '__init__.py'):
                top_dir = h.dirname(top_dir)
            top_dir = h.realpath(top_dir)
            self.top_level_dirs.append(top_dir)

        if not self.top_level_dir and self.top_level_dirs:
            self.top_level_dir = self.top_level_dirs[0]

        for path in self.top_level_dirs:
            h.add_to_path(path)

        for path in args.path:
            h.add_to_path(path)

        if args.coverage:  # pragma: no cover
            try:
                import coverage
            except ImportError:
                self.print_('Error: coverage is not installed.')
                return 1

            source = self.args.coverage_source
            if not source:
                source = self.top_level_dirs + self.args.path
            self.coverage_source = source
            self.cov = coverage.Coverage(source=self.coverage_source,
                                         data_suffix=True)
            self.cov.erase()

        if args.expectations_files:
            ret = self.parse_expectations()
            if ret:
                return ret
        elif args.tags:
            self.print_('Error: tags require expectations files.')
            return 1
        return 0

    def parse_expectations(self):
        args = self.args

        if len(args.expectations_files) != 1:
            # TODO(crbug.com/835690): Fix this.
            self.print_(
                'Only a single expectation file is currently supported',
                stream=self.host.stderr)
            return 1
        contents = self.host.read_text_file(args.expectations_files[0])

        expectations = TestExpectations(set(args.tags), args.ignored_tags)
        err, msg = expectations.parse_tagged_list(
            contents, args.expectations_files[0],
            tags_conflict=self.tag_conflict_checker)
        if err:
            self.print_(msg, stream=self.host.stderr)
            return err

        self.has_expectations = True
        self.expectations = expectations

    def find_tests(self, args):
        test_set = TestSet(self.args.test_name_prefix)

        orig_skip = unittest.skip
        orig_skip_if = unittest.skipIf
        if args.all:
            unittest.skip = lambda reason: lambda x: x
            unittest.skipIf = lambda condition, reason: lambda x: x

        try:
            names = self._name_list_from_args(args)
            classifier = self.classifier or self.default_classifier

            for name in names:
                try:
                    self._add_tests_to_set(test_set, args.suffixes,
                                           self.top_level_dirs, classifier,
                                           name)
                except (AttributeError, ImportError, SyntaxError) as e:
                    ex_str = traceback.format_exc()
                    self.print_('Failed to load "%s" in find_tests: %s' %
                                (name, e))
                    self.print_('  %s' %
                                '\n  '.join(ex_str.splitlines()))
                    self.print_(ex_str)
                    return 1, None
                except _AddTestsError as e:
                    self.print_(str(e))
                    return 1, None

            # TODO: Add support for discovering setupProcess/teardownProcess?

            shard_index = args.shard_index
            total_shards = args.total_shards
            assert total_shards >= 1
            assert shard_index >= 0 and shard_index < total_shards, (
                'shard_index (%d) must be >= 0 and < total_shards (%d)' %
                (shard_index, total_shards))
            test_set.parallel_tests = _sort_inputs(
                test_set.parallel_tests)[shard_index::total_shards]
            test_set.isolated_tests = _sort_inputs(
                test_set.isolated_tests)[shard_index::total_shards]
            test_set.tests_to_skip = _sort_inputs(
                test_set.tests_to_skip)[shard_index::total_shards]
            return 0, test_set
        finally:
            unittest.skip = orig_skip
            unittest.skipIf = orig_skip_if

    def _name_list_from_args(self, args):
        if args.tests:
            names = args.tests
        elif args.file_list:
            if args.file_list == '-':
                s = self.host.stdin.read()
            else:
                s = self.host.read_text_file(args.file_list)
            names = [line.strip() for line in s.splitlines()]
        else:
            names = self.top_level_dirs
        return names

    def _add_tests_to_set(self, test_set, suffixes, top_level_dirs, classifier,
                          name):
        h = self.host
        loader = self.loader
        add_tests = _test_adder(test_set, classifier)

        found = set()
        for d in top_level_dirs:
            if h.isfile(name):
                rpath = h.relpath(name, d)
                if rpath.startswith('..'):
                    continue
                if rpath.endswith('.py'):
                    rpath = rpath[:-3]
                module = rpath.replace(h.sep, '.')
                if module not in found:
                    found.add(module)
                    add_tests(loader.loadTestsFromName(module))
            elif h.isdir(name):
                rpath = h.relpath(name, d)
                if rpath.startswith('..'):
                    continue
                for suffix in suffixes:
                    if not name in found:
                        found.add(name + '/' + suffix)
                        add_tests(loader.discover(name, suffix, d))
            else:
                possible_dir = name.replace('.', h.sep)
                if h.isdir(d, possible_dir):
                    for suffix in suffixes:
                        path = h.join(d, possible_dir)
                        if not path in found:
                            found.add(path + '/' + suffix)
                            suite = loader.discover(path, suffix, d)
                            add_tests(suite)
                elif not name in found:
                    found.add(name)
                    add_tests(loader.loadTestsFromName(
                        self.args.test_name_prefix + name))

        # pylint: disable=no-member
        if hasattr(loader, 'errors') and loader.errors:  # pragma: python3
            # In Python3's version of unittest, loader failures get converted
            # into failed test cases, rather than raising exceptions. However,
            # the errors also get recorded so you can err out immediately.
            if isinstance(loader.errors, list):
                raise ImportError('\n'.join(loader.errors))
            raise ImportError(loader.errors)

    def _run_tests(self, result_set, test_set, all_tests):
        h = self.host
        self.last_runs_retry_on_failure_tests = set()

        def get_tests_to_retry(results):
            # If the --retry-only-retry-on-failure-tests command line argument
            # is passed , then a set of test failures with the RetryOnFailure
            # expectation from the last run of tests will be returned. The
            # self.last_runs_retry_on_failure_tests will be set to an empty set
            # for the next run of tests. Otherwise all regressions from the
            # last run will be returned.
            if self.args.retry_only_retry_on_failure_tests:
                ret = self.last_runs_retry_on_failure_tests.copy()
                self.last_runs_retry_on_failure_tests = set()
                return ret
            else:
                return json_results.regressions(results)

        if len(test_set.parallel_tests):
            jobs = min(
                len(test_set.parallel_tests), self.args.jobs)
        else:
            jobs = 1

        child = _Child(self)
        pool_group = make_pool_group(h, jobs, self.args.stable_jobs,
                                     _run_one_test, child, _setup_process,
                                     _teardown_process,
                                     self.args.use_global_pool)
        pool_group.make_global_pool()

        self._run_one_set(self.stats, result_set, test_set, jobs,
                          pool_group)

        tests_to_retry = sorted(get_tests_to_retry(result_set))
        retry_limit = self.args.retry_limit
        try:
            # Start at 1 since we already did iteration 0 above.
            for iteration in range(1, self.args.retry_limit + 1):
                if not tests_to_retry:
                    break
                if retry_limit == self.args.retry_limit:
                    self.flush()
                    self.args.overwrite = False
                    self.printer.should_overwrite = False
                    self.args.verbose = min(self.args.verbose, 1)

                self.print_('')
                self.print_('Retrying failed tests (attempt #%d of %d)...' %
                            (iteration, self.args.retry_limit))
                self.print_('')

                stats = Stats(self.args.status_format, h.time, 1)
                stats.total = len(tests_to_retry)
                test_set = TestSet(self.args.test_name_prefix)
                test_set.isolated_tests = [
                    TestInput(name,
                        iteration=iteration) for name in tests_to_retry]
                tests_to_retry = test_set
                retry_set = ResultSet()
                self._run_one_set(stats, retry_set, tests_to_retry, 1,
                                  pool_group)
                result_set.results.extend(retry_set.results)
                tests_to_retry = get_tests_to_retry(retry_set)
                retry_limit -= 1
            pool_group.close_global_pool()
        finally:
            self.final_responses.extend(pool_group.join_global_pool())

        if retry_limit != self.args.retry_limit:
            self.print_('')

        full_results = json_results.make_full_results(self.metadata,
                                                      int(h.time()),
                                                      all_tests, result_set,
                                                      self.path_delimiter)
        retcode = (json_results.exit_code_from_full_results(full_results)
                   | result_sink.result_sink_retcode_from_result_set(result_set))

        return (retcode, full_results)

    def _run_one_set(self, stats, result_set, test_set, jobs, pool_group):
        self._skip_tests(stats, result_set, test_set.tests_to_skip)
        # Don't bother spinning up any pools if we don't have any use for them.
        have_tests = test_set.parallel_tests or test_set.isolated_tests
        if not have_tests:
            return

        # If we know we are running all tests serially, reduce overhead a bit
        # by only using one pool.
        if jobs == 1:
            pool = pool_group.make_serial_pool()
            try:
                self._run_list(stats, result_set, test_set.parallel_tests,
                               jobs, pool)
                self._run_list(stats, result_set, test_set.isolated_tests,
                               jobs, pool)
                pool_group.close_serial_pool()
            finally:
                self.final_responses.extend(pool_group.join_serial_pool())
            return

        if test_set.parallel_tests:
            pool = pool_group.make_parallel_pool()
            try:
                self._run_list(stats, result_set,
                               test_set.parallel_tests, jobs, pool)
                pool_group.close_parallel_pool()
            finally:
                self.final_responses.extend(pool_group.join_parallel_pool())

        if test_set.isolated_tests:
            pool = pool_group.make_serial_pool()
            try:
                self._run_list(stats, result_set,
                               test_set.isolated_tests, 1, pool)
                pool_group.close_serial_pool()
            finally:
                self.final_responses.extend(pool_group.join_serial_pool())

    def _skip_tests(self, stats, result_set, tests_to_skip):
        for test_input in tests_to_skip:
            last = self.host.time()
            stats.started += 1
            self._print_test_started(stats, test_input)
            now = self.host.time()
            result = Result(test_input.name, actual=ResultType.Skip,
                            started=last, took=(now - last), worker=0,
                            expected=[ResultType.Skip],
                            out=test_input.msg)
            result_set.add(result)
            stats.finished += 1
            self._print_test_finished(stats, result)

    def _run_list(self, stats, result_set, test_inputs, jobs, pool):
        running_jobs = set()

        while test_inputs or running_jobs:
            while test_inputs and (len(running_jobs) < jobs):
                test_input = test_inputs.pop(0)
                stats.started += 1
                pool.send(test_input)
                running_jobs.add(test_input.name)
                self._print_test_started(stats, test_input)

            result, should_retry_on_failure = pool.get()
            if result.is_regression:
                stats.failed += 1
            if (self.args.typ_max_failures is not None
                and stats.failed >= self.args.typ_max_failures):
                print('\nAborting, waiting for processes to close')
                pool.close()
                pool.join()
                raise RuntimeError(
                    'Encountered %d failures with max of %d set, aborting.' % (
                    stats.failed, self.args.typ_max_failures))

            if (self.args.retry_only_retry_on_failure_tests and
                result.actual == ResultType.Failure and
                should_retry_on_failure):
                self.last_runs_retry_on_failure_tests.add(result.name)

            running_jobs.remove(result.name)
            result_set.add(result)
            stats.finished += 1
            self._print_test_finished(stats, result)

    def _print_test_started(self, stats, test_input):
        if self.args.quiet:
            # Print nothing when --quiet was passed.
            return

        # If -vvv was passed, print when the test is queued to be run.
        # We don't actually know when the test picked up to run, because
        # that is handled by the child process (where we can't easily
        # print things). Otherwise, only print when the test is started
        # if we know we can overwrite the line, so that we do not
        # get multiple lines of output as noise (in -vvv, we actually want
        # the noise).
        test_start_msg = stats.format() + test_input.name
        if self.args.verbose > 2:
            self.update(test_start_msg + ' queued', elide=False)
        if self.args.overwrite:
            self.update(test_start_msg, elide=(not self.args.verbose))

    def _print_test_finished(self, stats, result):
        stats.add_time()

        assert result.actual in [ResultType.Failure, ResultType.Skip,
                                 ResultType.Pass]
        if result.actual == ResultType.Failure:
            result_str = ' failed'
        elif result.actual == ResultType.Skip:
            result_str = ' was skipped'
        elif result.actual == ResultType.Pass:
            result_str = ' passed'

        if result.unexpected:
            result_str += ' unexpectedly'
        elif result.actual == ResultType.Failure:
            result_str += ' as expected'

        # Include any associated bugs from relevant expectations if the test:
        #   1. Was skipped
        #   2. Failed as expected
        #   3. Unexpectedly passed
        bug_str = ''
        expectedly_skipped_or_failed = (not result.unexpected and
                                        result.actual != ResultType.Pass)
        unexpectedly_passed = (result.unexpected and
                               result.actual == ResultType.Pass)
        if expectedly_skipped_or_failed or unexpectedly_passed:
            if result.associated_bugs:
                bug_str = f' ({result.associated_bugs})'

        if self.args.timing:
            timing_str = ' %.4fs' % result.took
        else:
            timing_str = ''
        worker_str = ' (worker %d)' % result.worker
        suffix = '%s%s%s%s' % (result_str, bug_str, timing_str, worker_str)
        out = result.out
        err = result.err
        if result.is_regression:
            if out or err:
                suffix += ':\n'
            self.update(stats.format() + result.name + suffix, elide=False)
            for l in out.splitlines():
                self.print_('  %s' % l)
            for l in err.splitlines():
                self.print_('  %s' % l)
        elif not self.args.quiet:
            if self.args.verbose > 1 and (out or err):
                suffix += ':\n'
            self.update(stats.format() + result.name + suffix,
                        elide=(not self.args.verbose))
            if self.args.verbose > 1:
                for l in out.splitlines():
                    self.print_('  %s' % l)
                for l in err.splitlines():
                    self.print_('  %s' % l)
            if self.args.verbose:
                self.flush()

    def update(self, msg, elide):
        self.printer.update(msg, elide)

    def flush(self):
        self.printer.flush()

    def _summarize(self, full_results):
        num_passes = json_results.num_passes(full_results)
        num_failures = json_results.num_failures(full_results)
        num_skips = json_results.num_skips(full_results)
        num_regressions = json_results.num_regressions(full_results)

        if self.args.quiet and num_failures == 0:
            return

        if self.args.timing:
            timing_clause = ' in %.1fs' % (self.host.time() -
                                           self.stats.started_time)
        else:
            timing_clause = ''
        self.update('%d test%s passed%s, %d skipped, %d failure%s.' %
                    (num_passes,
                     '' if num_passes == 1 else 's',
                     timing_clause,
                     num_skips,
                     num_failures,
                     '' if num_failures == 1 else 's'), elide=False)
        self.print_()
        if num_failures or num_regressions:
            regressed_tests = json_results.regressed_tests_names(full_results)
            failed_tests = json_results.failed_tests_names(full_results)
            expected_failed_tests = failed_tests - regressed_tests
            regressed_tests = sorted(list(regressed_tests))
            expected_failed_tests = sorted(list(expected_failed_tests))
            if expected_failed_tests:
                self.update('Tests that failed as expected:\n', elide=False)
                for t in expected_failed_tests:
                    self.print_('  %s' % t)
            if regressed_tests:
                self.update('Tests that regressed (failed unexpectedly)\n', elide=False)
                for t in regressed_tests:
                    self.print_('  %s' % t)

    def _read_and_delete(self, path, delete):
        h = self.host
        obj = None
        if h.exists(path):
            contents = h.read_text_file(path)
            if contents:
                obj = json.loads(contents)
            if delete:
                h.remove(path)
        return obj

    def _write(self, path, obj):
        if path:
            self.host.write_text_file(path, json.dumps(obj, indent=2) + '\n')

    def _upload(self, full_results):
        h = self.host
        if not self.args.test_results_server:
            return 0

        url, content_type, data = json_results.make_upload_request(
            self.args.test_results_server, self.args.builder_name,
            self.args.master_name, self.args.test_type,
            full_results)

        try:
            h.fetch(url, data, {'Content-Type': content_type})
            return 0
        except Exception as e:
            h.print_('Uploading the JSON results raised "%s"' % str(e))
            return 1

    def report_coverage(self):  # pragma: no cover
        self.host.print_()
        import coverage
        cov = coverage.Coverage(data_suffix=True)
        cov.combine()
        percentage = cov.report(show_missing=self.args.coverage_show_missing,
                                omit=self.args.coverage_omit)
        if self.args.coverage_annotate:
            cov.annotate(omit=self.args.coverage_omit)
        # https://coverage.readthedocs.io/en/6.4.2/config.html#report-fail-under
        return 2 if percentage < cov.get_option('report:fail_under') else 0

    def _add_trace_event(self, trace, name, start, end):
        event = {
            'name': name,
            'ts': int((start - self.stats.started_time) * 1000000),
            'dur': int((end - start) * 1000000),
            'ph': 'X',
            'pid': self.host.getpid(),
            'tid': 0,
        }
        trace['traceEvents'].append(event)

    def _trace_from_results(self, result_set):
        trace = OrderedDict()
        trace['traceEvents'] = []
        trace['otherData'] = {}

        if self.metadata:
            trace['otherData'] = self.metadata

        for result in result_set.results:
            started = int((result.started - self.stats.started_time) * 1000000)
            took = int(result.took * 1000000)
            event = OrderedDict()
            event['name'] = result.name
            event['dur'] = took
            event['ts'] = started
            event['ph'] = 'X'  # "Complete" events
            event['pid'] = result.pid
            event['tid'] = result.worker

            args = OrderedDict()
            args['expected'] = sorted(str(r) for r in result.expected)
            args['actual'] = str(result.actual)
            args['out'] = result.out
            args['err'] = result.err
            args['code'] = result.code
            args['unexpected'] = result.unexpected
            args['flaky'] = result.flaky
            args['file'] = result.file_path
            args['line'] = result.line_number
            event['args'] = args

            trace['traceEvents'].append(event)
        return trace

    def expectations_for(self, test_case):
        expectations = self.expectations if self.has_expectations else None
        return _expectations_for(
            test_case, expectations, self.args.test_name_prefix)

    def default_classifier(self, test_set, test):
        if self.matches_filter(test):
            if self.should_skip(test):
                test_set.add_test_to_skip(test, 'skipped by request')
            elif self.should_isolate(test):
                test_set.add_test_to_run_isolated(test)
            else:
                test_set.add_test_to_run_in_parallel(test)

    def matches_filter(self, test_case):
        _validate_test_starts_with_prefix(
            self.args.test_name_prefix, test_case.id())
        test_name = test_case.id()[len(self.args.test_name_prefix):]
        if self.args.test_filter:
            return any(
                fnmatch.fnmatch(test_name, glob)
                for glob in self.args.test_filter.split('::'))
        if self.args.partial_match_filter:
            return any(
                substr in test_name
                for substr in self.args.partial_match_filter)
        return True

    def should_isolate(self, test_case):
        _validate_test_starts_with_prefix(
            self.args.test_name_prefix, test_case.id())
        test_name = test_case.id()[len(self.args.test_name_prefix):]
        return any(fnmatch.fnmatch(test_name, glob)
                   for glob in self.args.isolate)

    def should_skip(self, test_case):
        _validate_test_starts_with_prefix(
            self.args.test_name_prefix, test_case.id())
        if self.args.all:
            return False
        test_name = test_case.id()[len(self.args.test_name_prefix):]
        if self.has_expectations:
            expected_results = self.expectations.expectations_for(test_name).results
        else:
            expected_results = {ResultType.Pass}
        return (
            ResultType.Skip in expected_results or
            any(fnmatch.fnmatch(test_name, glob) for glob in self.args.skip))


def _test_adder(test_set, classifier):
    def add_tests(obj):
        if isinstance(obj, unittest.suite.TestSuite):
            for el in obj:
                add_tests(el)
        elif (obj.id().startswith('unittest.loader.LoadTestsFailure') or
              obj.id().startswith('unittest.loader.ModuleImportFailure')):
            # Access to protected member pylint: disable=W0212
            module_name = obj._testMethodName
            try:
                method = getattr(obj, obj._testMethodName)
                method()
            except Exception as e:
                if 'LoadTests' in obj.id():
                    raise _AddTestsError('%s.load_tests() failed: %s'
                                         % (module_name, str(e)))
                else:
                    raise _AddTestsError(str(e))
        else:
            assert isinstance(obj, unittest.TestCase)
            classifier(test_set, obj)
    return add_tests


class _Child(object):

    def __init__(self, parent):
        self.host = None
        self.worker_num = None
        self.all = parent.args.all
        self.debugger = parent.args.debugger
        self.coverage = parent.args.coverage and parent.args.jobs > 1
        self.coverage_source = parent.coverage_source
        self.dry_run = parent.args.dry_run
        self.loader = parent.loader
        self.passthrough = parent.args.passthrough
        self.context = parent.context
        self.setup_fn = parent.setup_fn
        self.teardown_fn = parent.teardown_fn
        self.context_after_setup = None
        self.top_level_dir = parent.top_level_dir
        self.top_level_dirs = parent.top_level_dirs
        self.loaded_suites = {}
        self.cov = None
        self.has_expectations = parent.has_expectations
        self.expectations = parent.expectations
        self.test_name_prefix = parent.args.test_name_prefix
        self.artifact_output_dir = parent.artifact_output_dir
        self.result_sink_reporter = None
        self.disable_resultsink = parent.args.disable_resultsink
        self.result_sink_output_file = parent.args.rdb_content_output_file
        self.jobs = parent.args.jobs

    def expectations_for(self, test_case):
        expectations = self.expectations if self.has_expectations else None
        return _expectations_for(test_case, expectations, self.test_name_prefix)


def _setup_process(host, worker_num, child):
    child.host = host
    child.result_sink_reporter = result_sink.ResultSinkReporter(
            host, child.disable_resultsink, child.result_sink_output_file)
    child.worker_num = worker_num
    # pylint: disable=protected-access

    if child.coverage:  # pragma: no cover
        import coverage
        child.cov = coverage.Coverage(source=child.coverage_source,
                                      data_suffix=True)
        child.cov._warn_no_data = False
        child.cov.start()

    if child.setup_fn:
        child.context_after_setup = child.setup_fn(child, child.context)
    else:
        child.context_after_setup = child.context
    return child


def _teardown_process(child):
    res = None
    exc = None
    if child.teardown_fn:
        try:
            res = child.teardown_fn(child, child.context_after_setup)
        except Exception as e:
            exc = e
            pass

    if child.cov:  # pragma: no cover
        child.cov.stop()
        child.cov.save()

    return (child.worker_num, res, exc)


def _run_one_test(child, test_input):
    def _get_expectation_information():
        if child.has_expectations:
            expectation = child.expectations.expectations_for(test_name)
            expected_results = expectation.results
            should_retry_on_failure = expectation.should_retry_on_failure
            associated_bugs = expectation.reason
        else:
            expected_results = {ResultType.Pass}
            should_retry_on_failure = False
            associated_bugs = ''
        return expected_results, should_retry_on_failure, associated_bugs

    h = child.host
    pid = h.getpid()
    test_name = test_input.name

    started = h.time()

    # It is important to capture the output before loading the test
    # to ensure that
    # 1) the loader doesn't logs something we don't captured
    # 2) neither the loader nor the test case grab a reference to the
    #    uncaptured stdout or stderr that later is used when the test is run.
    # This comes up when using the FakeTestLoader and testing typ itself,
    # but could come up when testing non-typ code as well.
    h.capture_output(divert=not child.passthrough)
    (expected_results,
        should_retry_on_failure,
        associated_bugs) = _get_expectation_information()
    ex_str = ''
    try:
        orig_skip = unittest.skip
        orig_skip_if = unittest.skipIf
        if child.all:
            unittest.skip = lambda reason: lambda x: x
            unittest.skipIf = lambda condition, reason: lambda x: x
        elif ResultType.Skip in expected_results:
            h.restore_output()
            return (Result(test_name, ResultType.Skip, started, 0,
                           child.worker_num, expected=expected_results,
                           unexpected=False, pid=pid,
                           associated_bugs=associated_bugs), False)

        test_name_to_load = child.test_name_prefix + test_name
        try:
            # If we have errors around from before, clear them now so we don't
            # attempt to handle them later.
            if hasattr(child.loader, 'errors') and child.loader.errors:
                child.loader.errors.clear()
            suite = child.loader.loadTestsFromName(test_name_to_load)
            # From Python 3.5, AttributeError will not be thrown when calling
            # LoadTestsFromName. Instead, it adds error messages in the loader.
            # As a result, the original handling cannot kick in properly. We
            # now check the error message and throw exception as needed.
            if hasattr(child.loader, 'errors') and child.loader.errors:
                if isinstance(child.loader.errors, list):
                    raise AttributeError('\n'.join(child.loader.errors))
                raise AttributeError(child.loader.errors)
        except Exception as e:
            ex_str = ('loadTestsFromName("%s") failed: %s\n%s\n' %
                      (test_name_to_load, e, traceback.format_exc()))
            try:
                suite = _load_via_load_tests(child, test_name_to_load)
                ex_str += ('\nload_via_load_tests(\"%s\") returned %d tests\n' %
                           (test_name_to_load, len(list(suite))))
            except Exception as e:  # pragma: untested
                suite = []
                ex_str += ('\nload_via_load_tests("%s") failed: %s\n%s\n' %
                           (test_name_to_load, e, traceback.format_exc()))
    finally:
        unittest.skip = orig_skip
        unittest.skipIf = orig_skip_if

    tests = list(suite)
    if len(tests) != 1:
        err = 'Failed to load "%s" in run_one_test' % test_name
        if ex_str:  # pragma: untested
            err += '\n  ' + '\n  '.join(ex_str.splitlines())

        h.restore_output()
        return (Result(test_name, ResultType.Failure, started, took=0,
                       worker=child.worker_num, unexpected=True, code=1,
                       err=err, pid=pid), False)

    art = artifacts.Artifacts(
        child.artifact_output_dir, h, test_input.iteration, test_name)

    test_case = tests[0]
    if isinstance(test_case, TypTestCase):
        test_case.child = child
        test_case.context = child.context_after_setup
        test_case.set_artifacts(art)

    test_result = unittest.TestResult()
    out = ''
    err = ''
    try:
        if child.dry_run:
            pass
        elif child.debugger:  # pragma: no cover
            _run_under_debugger(h, test_case, suite, test_result)
        else:
            suite.run(test_result)
    finally:
        out, err = h.restore_output()
        # Clear the artifact implementation so that later tests don't try to
        # use a stale instance.
        if isinstance(test_case, TypTestCase):
            test_case.set_artifacts(None)

    # We retrieve the expected results again since it's possible that running
    # the test changed something, e.g. restarted the browser with new browser
    # arguments, leading to different tags being generated.
    (expected_results,
        should_retry_on_failure,
        associated_bugs) = _get_expectation_information()

    took = h.time() - started
    additional_tags = None
    test_location = inspect.getsourcefile(test_case.__class__)
    test_method = getattr(test_case, test_case._testMethodName)
    # Test methods are often wrapped by decorators such as @mock. Try to get to
    # the actual test method instead of the wrapper.
    if hasattr(test_method, '__wrapped__'):
        test_method = test_method.__wrapped__
    # Some tests are generated and don't have valid line numbers. Such test
    # methods also have a source location different from module location.
    if inspect.getsourcefile(test_method) == test_location:
        test_line = inspect.getsourcelines(test_method)[1]
    else:
        test_line = None

    # If the test signaled that it should be retried on failure, do so.
    if isinstance(test_case, TypTestCase):
        additional_tags = test_case.additionalTags
        # Handle the case where the test called self.skipTest, e.g. if it
        # determined that the test is not valid on the current configuration.
        if test_result.skipped and test_case.programmaticSkipIsExpected:
            result = Result(test_name, ResultType.Skip, started, took,
                           child.worker_num, expected={ResultType.Skip},
                           unexpected=False, pid=pid,
                           associated_bugs=associated_bugs)
            result.result_sink_retcode =\
                    child.result_sink_reporter.report_individual_test_result(
                        result, child.artifact_output_dir, child.expectations,
                        test_location, test_line, child.test_name_prefix,
                        additional_tags)
            return (result, False)
        should_retry_on_failure = (should_retry_on_failure
                                   or test_case.retryOnFailure)
    result = _result_from_test_result(test_result, test_name, started, took, out,
                                    err, child.worker_num, pid, test_case,
                                    expected_results, child.has_expectations,
                                    art.artifacts, associated_bugs)
    result.result_sink_retcode =\
            child.result_sink_reporter.report_individual_test_result(
                result, child.artifact_output_dir, child.expectations,
                test_location, test_line, child.test_name_prefix,
                additional_tags)
    return (result, should_retry_on_failure)


def _run_under_debugger(host, test_case, suite,
                        test_result):  # pragma: no cover
    # Access to protected member pylint: disable=W0212
    test_func = getattr(test_case, test_case._testMethodName)
    fname = inspect.getsourcefile(test_func)
    lineno = inspect.getsourcelines(test_func)[1] + 1
    dbg = pdb.Pdb(stdout=host.stdout.stream)
    dbg.set_break(fname, lineno)
    dbg.runcall(suite.run, test_result)


def _result_from_test_result(test_result, test_name, started, took, out, err,
                             worker_num, pid, test_case, expected_results,
                             has_expectations, artifacts, associated_bugs):
    failure_reason = None
    if test_result.failures:
        actual = ResultType.Failure
        code = 1
        err = err + test_result.failures[0][1]
        unexpected = actual not in expected_results
        for i, failure in enumerate(test_result.failures):
            if failure_reason is None:
                failure_reason = _failure_reason_from_traceback(failure[1])
    elif test_result.errors:
        actual = ResultType.Failure
        code = 1
        err = err + test_result.errors[0][1]
        unexpected = actual not in expected_results
        for i, error in enumerate(test_result.errors):
            if failure_reason is None:
                failure_reason = _failure_reason_from_traceback(error[1])
    elif test_result.skipped:
        actual = ResultType.Skip
        err = err + test_result.skipped[0][1]
        code = 0
        if has_expectations:
            unexpected = actual not in expected_results
        else:
            unexpected = False
            expected_results = {ResultType.Skip}
    elif test_result.expectedFailures:
        actual = ResultType.Failure
        code = 1
        err = err + test_result.expectedFailures[0][1]
        unexpected = False
    elif test_result.unexpectedSuccesses:
        actual = ResultType.Pass
        code = 0
        unexpected = True
    else:
        actual = ResultType.Pass
        code = 0
        unexpected = actual not in expected_results

    flaky = False
    test_func = getattr(test_case, test_case._testMethodName)
    test_func = getattr(test_func, 'real_test_func', test_func)
    file_path = inspect.getsourcefile(test_func)
    line_number = inspect.getsourcelines(test_func)[1]
    return Result(test_name, actual, started, took, worker_num,
                  expected_results, unexpected, flaky, code, out, err, pid,
                  file_path, line_number, artifacts, failure_reason,
                  associated_bugs)


def _failure_reason_from_traceback(traceback):
    """Attempts to extract a failure reason from formatted Traceback data.

    The formatted traceback data handled by this method is that populated on
    unittest.TestResult objects in the errors and/or failures attribute(s).

    We reverse this formatting process to obtain the underlying failure
    exception message or assertion failure, excluding stacktrace and other
    data.

    When reading this method, it is useful to read python unittest sources
    at the same time, as this reverses some of the formatting defined there.
    https://github.com/python/cpython/blob/3.10/Lib/unittest/result.py#L119
    https://github.com/python/cpython/blob/3.10/Lib/unittest/result.py#L173
    https://github.com/python/cpython/blob/3.10/Lib/traceback.py#L652

    This method may not succeed in extracting a failure reason. In this case,
    it returns None.
    """
    lines = traceback.splitlines()

    # Start line index of the interesting region (the line(s) that has
    # the assertion failure or exception emssage).
    start_index = 0

    # End index of the interesting region.
    end_index = len(lines)

    # The file name and line that raised the exception or assertion failure.
    # Formatted as "filename.py(123)".
    context_file_line = None

    in_traceback = False
    for i, line in enumerate(lines):
        # Tracebacks precede the interesting message. It is possible
        # for there to be multiple tracebacks blocks in case of chained
        # exceptions. E.g. "While handling a XYZError, the following
        # exception was also raised:". The interesting message is
        # after all such chained stacks.
        if line == 'Traceback (most recent call last):':
            in_traceback = True
            start_index = i + 1
            context_file_line = None
        elif line.startswith('  ') and in_traceback:
            # Continuation of traceback.
            start_index = i + 1

            # Keep track of the last file in the traceback.
            file_match = _TRACEBACK_FILE_RE.match(line)
            if file_match:
                context_file_line = '{}({})'.format(
                    file_match.group(1),
                    file_match.group(2))
        else:
            in_traceback = False

        # The "Stdout:" or "Stderr:" blocks (if present) are after the
        # interesting failure message.
        if line == 'Stdout:' or line == 'Stderr:':
            if i < end_index:
                end_index = i

    interesting_lines = lines[start_index:end_index]
    if len(interesting_lines) > 0 and context_file_line is not None:
        # Let the failure reason be look like:
        # "my_unittest.py(123): AssertionError: unexpectedly None".
        #
        # We include the file and line of the original exception
        # in failure reason, as basic assertion failures
        # (true != false, None is not None, etc.) can be too generic
        # to be clustered in a useful way without this.
        message = '{}: {}'.format(context_file_line,
            '\n'.join(interesting_lines).strip())
        return FailureReason(message)
    return None


def _load_via_load_tests(child, test_name):
    # If we couldn't import a test directly, the test may be only loadable
    # via unittest's load_tests protocol. See if we can find a load_tests
    # entry point that will work for this test.
    loader = child.loader
    comps = test_name.split('.')
    new_suite = unittest.TestSuite()

    while comps:
        name = '.'.join(comps)
        module = None
        suite = None
        if name not in child.loaded_suites:
            try:
                module = importlib.import_module(name)
            except ImportError:
                pass
            if module:
                suite = loader.loadTestsFromModule(module)
            child.loaded_suites[name] = suite
        suite = child.loaded_suites[name]
        if suite:
            for test_case in suite:
                assert isinstance(test_case, unittest.TestCase)
                if test_case.id() == test_name:  # pragma: untested
                    new_suite.addTest(test_case)
                    break
        comps.pop()
    return new_suite


def _sort_inputs(inps):
    return sorted(inps, key=lambda inp: inp.name)


def _expectations_for(test_case, expectations, test_name_prefix):
    test_name = test_case.id()[len(test_name_prefix):]
    if expectations:
        return expectations.expectations_for(test_name)
    else:
        return Expectation(test=test_name)


if __name__ == '__main__':  # pragma: no cover
    sys.modules['__main__'].__file__ = path_to_file
    sys.exit(main(win_multiprocessing=WinMultiprocessing.importable))
