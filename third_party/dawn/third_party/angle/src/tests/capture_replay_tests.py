#! /usr/bin/env vpython3
#
# Copyright 2020 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
"""
Script testing capture_replay with angle_end2end_tests
"""

# Automation script will:
# 1. Build all tests in angle_end2end with frame capture enabled
# 2. Run each test with frame capture
# 3. Build CaptureReplayTest with cpp trace files
# 4. Run CaptureReplayTest
# 5. Output the number of test successes and failures. A test succeeds if no error occurs during
# its capture and replay, and the GL states at the end of two runs match. Any unexpected failure
# will return non-zero exit code

# Run this script with Python to test capture replay on angle_end2end tests
# python path/to/capture_replay_tests.py
# Command line arguments: run with --help for a full list.

import argparse
import concurrent.futures
import contextlib
import difflib
import distutils.util
import getpass
import glob
import json
import logging
import os
import pathlib
import queue
import random
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import traceback

SCRIPT_DIR = str(pathlib.Path(__file__).resolve().parent)
PY_UTILS = str(pathlib.Path(SCRIPT_DIR) / 'py_utils')
if PY_UTILS not in sys.path:
    os.stat(PY_UTILS) and sys.path.insert(0, PY_UTILS)
import angle_test_util

PIPE_STDOUT = True
DEFAULT_OUT_DIR = "out/CaptureReplayTest"  # relative to angle folder
DEFAULT_FILTER = "*/ES2_Vulkan_SwiftShader"
DEFAULT_TEST_SUITE = "angle_end2end_tests"
REPLAY_SAMPLE_FOLDER = "src/tests/capture_replay_tests"  # relative to angle folder
DEFAULT_BATCH_COUNT = 1  # number of tests batched together for capture
CAPTURE_FRAME_END = 100
TRACE_FILE_SUFFIX = "_context"  # because we only deal with 1 context right now
RESULT_TAG = "*RESULT"
STATUS_MESSAGE_PERIOD = 20  # in seconds
CAPTURE_SUBPROCESS_TIMEOUT = 600  # in seconds
REPLAY_SUBPROCESS_TIMEOUT = 60  # in seconds
DEFAULT_RESULT_FILE = "results.txt"
DEFAULT_LOG_LEVEL = "info"
DEFAULT_MAX_JOBS = 8
REPLAY_BINARY = "capture_replay_tests"
if sys.platform == "win32":
    REPLAY_BINARY += ".exe"
TRACE_FOLDER = "traces"

EXIT_SUCCESS = 0
EXIT_FAILURE = 1
REPLAY_INITIALIZATION_FAILURE = -1
REPLAY_SERIALIZATION_FAILURE = -2

switch_case_without_return_template = """\
        case {case}:
            {namespace}::{call}({params});
            break;
"""

switch_case_with_return_template = """\
        case {case}:
            return {namespace}::{call}({params});
"""

default_case_without_return_template = """\
        default:
            break;"""
default_case_with_return_template = """\
        default:
            return {default_val};"""


def winext(name, ext):
    return ("%s.%s" % (name, ext)) if sys.platform == "win32" else name


GN_PATH = os.path.join('third_party', 'depot_tools', winext('gn', 'bat'))
AUTONINJA_PATH = os.path.join('third_party', 'depot_tools', 'autoninja.py')


def GetGnArgsStr(args, extra_gn_args=[]):
    gn_args = [('angle_with_capture_by_default', 'true'),
               ('angle_enable_vulkan_api_dump_layer', 'false'),
               ('angle_enable_wgpu', 'false')] + extra_gn_args
    if args.use_reclient:
        gn_args.append(('use_remoteexec', 'true'))
    if not args.debug:
        gn_args.append(('is_debug', 'false'))
        gn_args.append(('symbol_level', '1'))
        gn_args.append(('angle_assert_always_on', 'true'))
    if args.asan:
        gn_args.append(('is_asan', 'true'))
    return ' '.join(['%s=%s' % (k, v) for (k, v) in gn_args])


class XvfbPool(object):

    def __init__(self, worker_count):
        self.queue = queue.Queue()

        self.processes = []
        displays = set()
        tmp = tempfile.TemporaryDirectory()

        logging.info('Starting xvfb and openbox...')
        # Based on the simplest case from testing/xvfb.py, with tweaks to minimize races.
        try:
            for worker in range(worker_count):
                while True:
                    # Pick a set of random displays from a custom range to hopefully avoid
                    # collisions with anything else that might be using xvfb.
                    # Another option would be -displayfd but that has its quirks too.
                    display = random.randint(7700000, 7800000)
                    if display in displays:
                        continue

                    x11_display_file = '/tmp/.X11-unix/X%d' % display

                    if not os.path.exists(x11_display_file):
                        break

                displays.add(display)

                x11_proc = subprocess.Popen([
                    'Xvfb',
                    ':%d' % display, '-screen', '0', '1280x1024x24', '-ac', '-nolisten', 'tcp',
                    '-dpi', '96', '+extension', 'RANDR', '-maxclients', '512'
                ],
                                            stderr=subprocess.STDOUT)
                self.processes.append(x11_proc)

                start_time = time.time()
                while not os.path.exists(x11_display_file):
                    if time.time() - start_time >= 30:
                        raise Exception('X11 failed to start')
                    time.sleep(0.1)

                env = os.environ.copy()
                env['DISPLAY'] = ':%d' % display

                # testing/xvfb.py uses signals instead, which is tricky with multiple displays.
                openbox_ready_file = os.path.join(tmp.name, str(display))
                openbox_proc = subprocess.Popen(
                    ['openbox', '--sm-disable', '--startup',
                     'touch %s' % openbox_ready_file],
                    stderr=subprocess.STDOUT,
                    env=env)
                self.processes.append(openbox_proc)

                start_time = time.time()
                while not os.path.exists(openbox_ready_file):
                    if time.time() - start_time >= 30:
                        raise Exception('Openbox failed to start')
                    time.sleep(0.1)

                self.queue.put(display)

            logging.info('Started a pool of %d xvfb displays: %s', worker_count,
                         ' '.join(str(d) for d in sorted(displays)))
        except Exception:
            self.Teardown()
            raise
        finally:
            tmp.cleanup()

    def GrabDisplay(self):
        return self.queue.get()

    def ReleaseDisplay(self, display):
        self.queue.put(display)

    def Teardown(self):
        logging.info('Stopping xvfb pool')
        for p in reversed(self.processes):
            p.kill()
            p.wait()
        self.processes = []


@contextlib.contextmanager
def MaybeXvfbPool(xvfb, worker_count):
    if xvfb:
        try:
            xvfb_pool = XvfbPool(worker_count)
            yield xvfb_pool
        finally:
            xvfb_pool.Teardown()
    else:
        yield None


@contextlib.contextmanager
def GetDisplayEnv(env, xvfb_pool):
    if not xvfb_pool:
        yield env
        return

    display = xvfb_pool.GrabDisplay()
    display_var = ':%d' % display
    try:
        yield {**env, 'DISPLAY': display_var, 'XVFB_DISPLAY': display_var}
    finally:
        xvfb_pool.ReleaseDisplay(display)


def TestLabel(test_name):
    return test_name.replace(".", "_").replace("/", "_")


def ParseTestNamesFromTestList(output, test_expectation, also_run_skipped_for_capture_tests):
    output_lines = output.splitlines()
    tests = []
    seen_start_of_tests = False
    disabled = 0
    for line in output_lines:
        l = line.strip()
        if l == 'Tests list:':
            seen_start_of_tests = True
        elif l == 'End tests list.':
            break
        elif not seen_start_of_tests:
            pass
        elif not test_expectation.TestIsSkippedForCapture(l) or also_run_skipped_for_capture_tests:
            tests.append(l)
        else:
            disabled += 1

    logging.info('Found %s tests and %d disabled tests.' % (len(tests), disabled))
    return tests


class GroupedResult():
    Passed = "Pass"
    Failed = "Fail"
    TimedOut = "Timeout"
    CompileFailed = "CompileFailed"
    CaptureFailed = "CaptureFailed"
    ReplayFailed = "ReplayFailed"
    Skipped = "Skipped"
    FailedToTrace = "FailedToTrace"

    ResultTypes = [
        Passed, Failed, TimedOut, CompileFailed, CaptureFailed, ReplayFailed, Skipped,
        FailedToTrace
    ]

    def __init__(self, resultcode, message, output, tests):
        self.resultcode = resultcode
        self.message = message
        self.output = output
        self.tests = []
        for test in tests:
            self.tests.append(test)


def CaptureProducedRequiredFiles(all_trace_files, test_name):
    label = TestLabel(test_name)

    test_files = [f for f in all_trace_files if f.startswith(label)]

    frame_files_count = 0
    context_header_count = 0
    context_source_count = 0
    source_json_count = 0
    context_id = 0
    for f in test_files:
        # TODO: Consolidate. http://anglebug.com/42266223
        if "_001.cpp" in f or "_001.c" in f:
            frame_files_count += 1
        elif f.endswith(".json"):
            source_json_count += 1
        elif f.endswith(".h"):
            context_header_count += 1
            if TRACE_FILE_SUFFIX in f:
                context = f.split(TRACE_FILE_SUFFIX)[1][:-2]
                context_id = int(context)
        # TODO: Consolidate. http://anglebug.com/42266223
        elif f.endswith(".cpp") or f.endswith(".c"):
            context_source_count += 1
    got_all_files = (
        frame_files_count >= 1 and context_header_count >= 1 and context_source_count >= 1 and
        source_json_count == 1)
    return got_all_files


def GetCaptureEnv(args, trace_folder_path):
    env = {
        'ANGLE_CAPTURE_SERIALIZE_STATE': '1',
        'ANGLE_FEATURE_OVERRIDES_ENABLED': 'forceRobustResourceInit:forceInitShaderVariables',
        'ANGLE_FEATURE_OVERRIDES_DISABLED': 'supportsHostImageCopy',
        'ANGLE_CAPTURE_ENABLED': '1',
        'ANGLE_CAPTURE_OUT_DIR': trace_folder_path,
    }

    if args.mec > 0:
        env['ANGLE_CAPTURE_FRAME_START'] = '{}'.format(args.mec)
        env['ANGLE_CAPTURE_FRAME_END'] = '{}'.format(args.mec + 1)
    else:
        env['ANGLE_CAPTURE_FRAME_END'] = '{}'.format(CAPTURE_FRAME_END)

    if args.expose_nonconformant_features:
        env['ANGLE_FEATURE_OVERRIDES_ENABLED'] += ':exposeNonConformantExtensionsAndVersions'

    return env


def PrintContextDiff(replay_build_dir, test_name):
    frame = 1
    found = False
    while True:
        capture_file = "{}/{}_ContextCaptured{}.json".format(replay_build_dir, test_name, frame)
        replay_file = "{}/{}_ContextReplayed{}.json".format(replay_build_dir, test_name, frame)
        if os.path.exists(capture_file) and os.path.exists(replay_file):
            found = True
            captured_context = open(capture_file, "r").readlines()
            replayed_context = open(replay_file, "r").readlines()
            for line in difflib.unified_diff(
                    captured_context, replayed_context, fromfile=capture_file, tofile=replay_file):
                print(line, end="")
        else:
            if frame > CAPTURE_FRAME_END:
                break
        frame = frame + 1
    if not found:
        logging.error('Could not find serialization diff files for %s' % test_name)


def UnlinkContextStateJsonFilesIfPresent(replay_build_dir):
    for f in glob.glob(os.path.join(replay_build_dir, '*_ContextCaptured*.json')):
        os.unlink(f)
    for f in glob.glob(os.path.join(replay_build_dir, '*_ContextReplayed*.json')):
        os.unlink(f)


class TestExpectation():
    # tests that must not be run as list
    skipped_for_capture_tests = {}
    skipped_for_capture_tests_re = {}

    # test expectations for tests that do not pass
    non_pass_results = {}

    # COMPILE_FAIL
    compile_fail_tests = {}
    compile_fail_tests_re = {}

    flaky_tests = []

    non_pass_re = {}

    result_map = {
        "FAIL": GroupedResult.Failed,
        "TIMEOUT": GroupedResult.TimedOut,
        "COMPILE_FAIL": GroupedResult.CompileFailed,
        "NOT_RUN": GroupedResult.Skipped,
        "SKIP_FOR_CAPTURE": GroupedResult.Skipped,
        "PASS": GroupedResult.Passed,
    }

    def __init__(self, args):
        expected_results_filename = "capture_replay_expectations.txt"
        expected_results_path = os.path.join(REPLAY_SAMPLE_FOLDER, expected_results_filename)
        self._asan = args.asan
        with open(expected_results_path, "rt") as f:
            for line in f:
                l = line.strip()
                if l != "" and not l.startswith("#"):
                    self.ReadOneExpectation(l, args.debug)

    def _CheckTagsWithConfig(self, tags, config_tags):
        for tag in tags:
            if tag not in config_tags:
                return False
        return True

    def ReadOneExpectation(self, line, is_debug):
        (testpattern, result) = line.split('=')
        (test_info_string, test_name_string) = testpattern.split(':')
        test_name = test_name_string.strip()
        test_info = test_info_string.strip().split()
        result_stripped = result.strip()

        tags = []
        if len(test_info) > 1:
            tags = test_info[1:]

        config_tags = [GetPlatformForSkip()]
        if self._asan:
            config_tags += ['ASAN']
        if is_debug:
            config_tags += ['DEBUG']

        if self._CheckTagsWithConfig(tags, config_tags):
            test_name_regex = re.compile('^' + test_name.replace('*', '.*') + '$')
            if result_stripped == 'COMPILE_FAIL':
                self.compile_fail_tests[test_name] = self.result_map[result_stripped]
                self.compile_fail_tests_re[test_name] = test_name_regex
            if result_stripped == 'SKIP_FOR_CAPTURE' or result_stripped == 'TIMEOUT':
                self.skipped_for_capture_tests[test_name] = self.result_map[result_stripped]
                self.skipped_for_capture_tests_re[test_name] = test_name_regex
            elif result_stripped == 'FLAKY':
                self.flaky_tests.append(test_name_regex)
            else:
                self.non_pass_results[test_name] = self.result_map[result_stripped]
                self.non_pass_re[test_name] = test_name_regex

    def TestIsSkippedForCapture(self, test_name):
        return any(p.match(test_name) for p in self.skipped_for_capture_tests_re.values())

    def TestIsCompileFail(self, test_name):
        return any(p.match(test_name) for p in self.compile_fail_tests_re.values())

    def Filter(self, test_list, run_all_tests):
        result = {}
        for t in test_list:
            for key in self.non_pass_results.keys():
                if self.non_pass_re[key].match(t) is not None:
                    result[t] = self.non_pass_results[key]
            for key in self.compile_fail_tests.keys():
                if self.compile_fail_tests_re[key].match(t) is not None:
                    result[t] = self.compile_fail_tests[key]
            if run_all_tests:
                for [key, r] in self.skipped_for_capture_tests.items():
                    if self.skipped_for_capture_tests_re[key].match(t) is not None:
                        result[t] = r
        return result

    def IsFlaky(self, test_name):
        for flaky in self.flaky_tests:
            if flaky.match(test_name) is not None:
                return True
        return False


def GetPlatformForSkip():
    # yapf: disable
    # we want each pair on one line
    platform_map = { 'win32' : 'WIN',
                     'linux' : 'LINUX' }
    # yapf: enable
    return platform_map.get(sys.platform, 'UNKNOWN')


def RunInParallel(f, lst, max_workers, stop_event):
    with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as executor:
        future_to_arg = {executor.submit(f, arg): arg for arg in lst}
        try:
            for future in concurrent.futures.as_completed(future_to_arg):
                yield future, future_to_arg[future]
        except KeyboardInterrupt:
            stop_event.set()
            raise


def RunProcess(cmd, env, xvfb_pool, stop_event, timeout):
    stdout = [None]

    def _Reader(process):
        stdout[0] = process.stdout.read().decode()

    with GetDisplayEnv(env, xvfb_pool) as run_env:
        process = subprocess.Popen(
            cmd, env=run_env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        t = threading.Thread(target=_Reader, args=(process,))
        t.start()
        time_start = time.time()

        while True:
            time.sleep(0.1)
            if process.poll() is not None:
                t.join()
                return process.returncode, stdout[0]
            if timeout is not None and time.time() - time_start > timeout:
                process.kill()
                t.join()
                return subprocess.TimeoutExpired, stdout[0]
            if stop_event.is_set():
                process.kill()
                t.join()
                return None, stdout[0]


def ReturnCodeWithNote(rc):
    s = 'rc=%s' % rc
    if sys.platform.startswith('linux'):
        if rc == -9:
            # OOM killer sends SIGKILL to the process, return code is -signal
            s += ' SIGKILL possibly due to OOM'
    return s


def RunCaptureInParallel(args, trace_folder_path, test_names, worker_count, xvfb_pool):
    n = args.batch_count
    test_batches = [test_names[i:i + n] for i in range(0, len(test_names), n)]

    extra_env = GetCaptureEnv(args, trace_folder_path)
    env = {**os.environ.copy(), **extra_env}
    test_exe_path = os.path.join(args.out_dir, 'Capture', args.test_suite)

    stop_event = threading.Event()

    def _RunCapture(tests):
        filt = ':'.join(tests)

        results_file = tempfile.mktemp()
        cmd = [
            test_exe_path,
            '--gtest_filter=%s' % filt,
            '--angle-per-test-capture-label',
            '--results-file=' + results_file,
        ]

        # Add --use-config to avoid registering all test configurations
        configs = set([t.split('/')[-1] for t in filt.split(':')])
        if len(configs) == 1:
            config, = configs
            if '*' not in config:
                cmd.append('--use-config=%s' % config)

        test_results = None
        try:
            rc, stdout = RunProcess(cmd, env, xvfb_pool, stop_event, CAPTURE_SUBPROCESS_TIMEOUT)
            if rc == 0:
                with open(results_file) as f:
                    test_results = json.load(f)
        finally:
            try:
                os.unlink(results_file)
            except Exception:
                pass

        return rc, test_results, stdout

    skipped_by_suite = set()
    capture_failed = False
    for (future, tests) in RunInParallel(_RunCapture, test_batches, worker_count, stop_event):
        rc, test_results, stdout = future.result()

        if rc == subprocess.TimeoutExpired:
            logging.error('Capture failed - timed out after %ss\nTests: %s\nPartial stdout:\n%s',
                          CAPTURE_SUBPROCESS_TIMEOUT, ':'.join(tests), stdout)
            capture_failed = True
            continue

        if rc != 0:
            logging.error('Capture failed (%s)\nTests: %s\nStdout:\n%s', ReturnCodeWithNote(rc),
                          ':'.join(tests), stdout)
            capture_failed = True
            continue

        if args.show_capture_stdout:
            logging.info('Capture test stdout:\n%s', stdout)

        for test_name, res in test_results['tests'].items():
            if res['actual'] == 'SKIP':
                skipped_by_suite.add(test_name)

    return not capture_failed, skipped_by_suite


def RunReplayTestsInParallel(args, replay_build_dir, replay_tests, expected_results,
                             labels_to_tests, worker_count, xvfb_pool):
    extra_env = {}
    if args.expose_nonconformant_features:
        extra_env['ANGLE_FEATURE_OVERRIDES_ENABLED'] = 'exposeNonConformantExtensionsAndVersions'
    env = {**os.environ.copy(), **extra_env}

    stop_event = threading.Event()

    def _RunReplay(test):
        replay_exe_path = os.path.join(replay_build_dir, REPLAY_BINARY)
        cmd = [replay_exe_path, TestLabel(test)]
        return RunProcess(cmd, env, xvfb_pool, stop_event, REPLAY_SUBPROCESS_TIMEOUT)

    replay_failed = False
    for (future, test) in RunInParallel(_RunReplay, replay_tests, worker_count, stop_event):
        expected_to_pass = expected_results[test] == GroupedResult.Passed
        rc, stdout = future.result()
        if rc == subprocess.TimeoutExpired:
            if expected_to_pass:
                logging.error('Replay failed - timed out after %ss\nTest: %s\nPartial stdout:\n%s',
                              REPLAY_SUBPROCESS_TIMEOUT, test, stdout)
                replay_failed = True
            else:
                logging.info('Ignoring replay timeout due to expectation: %s [expected %s]', test,
                             expected_results[test])
            continue

        if rc != 0:
            if expected_to_pass:
                logging.error('Replay failed (%s)\nTest: %s\nStdout:\n%s', ReturnCodeWithNote(rc),
                              test, stdout)
                replay_failed = True
            else:
                logging.info('Ignoring replay failure due to expectation: %s [expected %s]', test,
                             expected_results[test])
            continue

        if args.show_replay_stdout:
            logging.info('Replay test stdout:\n%s', stdout)

        output_lines = stdout.splitlines()
        for output_line in output_lines:
            words = output_line.split(" ")
            if len(words) == 3 and words[0] == RESULT_TAG:
                test_name = labels_to_tests[words[1]]
                result = int(words[2])

                if result == 0:
                    pass
                elif result == REPLAY_INITIALIZATION_FAILURE:
                    if expected_to_pass:
                        replay_failed = True
                        logging.error('Replay failed. Initialization failure: %s' % test_name)
                    else:
                        logging.info(
                            'Ignoring replay failure due to expectation: %s [expected %s]', test,
                            expected_results[test])
                elif result == REPLAY_SERIALIZATION_FAILURE:
                    if expected_to_pass:
                        replay_failed = True
                        logging.error('Replay failed. Context comparison failed: %s' % test_name)
                        PrintContextDiff(replay_build_dir, words[1])
                    else:
                        logging.info(
                            'Ignoring replay context diff due to expectation: %s [expected %s]',
                            test, expected_results[test])
                else:
                    replay_failed = True
                    logging.error('Replay failed. Unknown result code: %s -> %d' %
                                  (test_name, result))

    return not replay_failed


def CleanupAfterReplay(replay_build_dir, test_labels):
    # Remove files that have test labels in the file name, .e.g:
    # ClearTest_ClearIsClamped_ES2_Vulkan_SwiftShader.dll.pdb
    for build_file in os.listdir(replay_build_dir):
        if any(label in build_file for label in test_labels):
            os.unlink(os.path.join(replay_build_dir, build_file))


def main(args):
    angle_test_util.SetupLogging(args.log.upper())

    # Set cwd to ANGLE root
    os.chdir(os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..")))

    if getpass.getuser() == 'chrome-bot':
        # bots need different re-client auth settings than developers b/319246651
        os.environ["RBE_use_gce_credentials"] = "true"
        os.environ["RBE_use_application_default_credentials"] = "false"
        os.environ["RBE_automatic_auth"] = "false"
        os.environ["RBE_experimental_credentials_helper"] = ""
        os.environ["RBE_experimental_credentials_helper_args"] = ""

    trace_dir = "%s%d" % (TRACE_FOLDER, 0)
    trace_folder_path = os.path.join(REPLAY_SAMPLE_FOLDER, trace_dir)
    if os.path.exists(trace_folder_path):
        shutil.rmtree(trace_folder_path)
    os.makedirs(trace_folder_path)

    capture_build_dir = os.path.join(args.out_dir, 'Capture')
    replay_build_dir = os.path.join(args.out_dir, 'Replay%d' % 0)

    logging.info('Building capture tests')

    subprocess.check_call([GN_PATH, 'gen', '--args=%s' % GetGnArgsStr(args), capture_build_dir])
    subprocess.check_call(
        [sys.executable, AUTONINJA_PATH, '-C', capture_build_dir, args.test_suite])

    with MaybeXvfbPool(args.xvfb, 1) as xvfb_pool:
        logging.info('Getting test list')
        test_path = os.path.join(capture_build_dir, args.test_suite)
        with GetDisplayEnv(os.environ, xvfb_pool) as env:
            test_list = subprocess.check_output(
                [test_path, "--list-tests",
                 "--gtest_filter=%s" % args.filter], env=env, text=True)

    test_expectation = TestExpectation(args)
    test_names = ParseTestNamesFromTestList(test_list, test_expectation,
                                            args.also_run_skipped_for_capture_tests)
    test_expectation_for_list = test_expectation.Filter(test_names,
                                                        args.also_run_skipped_for_capture_tests)

    test_names = [
        t for t in test_names if (not test_expectation.TestIsCompileFail(t) and
                                  not test_expectation.TestIsSkippedForCapture(t))
    ]

    if not test_names:
        logging.error('No capture tests to run. Is everything skipped?')
        return EXIT_FAILURE

    worker_count = min(args.max_jobs, os.cpu_count(), 1 + len(test_names) // 10)

    logging.info('Running %d capture tests, worker_count=%d batch_count=%d', len(test_names),
                 worker_count, args.batch_count)

    with MaybeXvfbPool(args.xvfb, worker_count) as xvfb_pool:
        success, skipped_by_suite = RunCaptureInParallel(args, trace_folder_path, test_names,
                                                         worker_count, xvfb_pool)
        if not success:
            logging.error('Capture tests failed, see "Capture failed" errors above')
            return EXIT_FAILURE

        logging.info('RunCaptureInParallel finished')

        labels_to_tests = {TestLabel(t): t for t in test_names}

        all_trace_files = [f.name for f in os.scandir(trace_folder_path) if f.is_file()]

        replay_tests = []
        failed = False
        for test_name in test_names:
            if test_name not in skipped_by_suite:
                if CaptureProducedRequiredFiles(all_trace_files, test_name):
                    replay_tests.append(test_name)
                else:
                    logging.error('Capture failed: test missing replay files: %s', test_name)
                    failed = True

        if failed:
            logging.error('Capture tests failed, see "Capture failed" errors above')
            return EXIT_FAILURE

        logging.info('CaptureProducedRequiredFiles finished')

        composite_file_id = 1
        names_path = os.path.join(trace_folder_path, 'test_names_%d.json' % composite_file_id)
        with open(names_path, 'w') as f:
            f.write(json.dumps({'traces': [TestLabel(t) for t in replay_tests]}))

        replay_build_dir = os.path.join(args.out_dir, 'Replay%d' % 0)
        UnlinkContextStateJsonFilesIfPresent(replay_build_dir)

        logging.info('Building replay tests')

        extra_gn_args = [('angle_build_capture_replay_tests', 'true'),
                         ('angle_capture_replay_test_trace_dir', '"%s"' % trace_dir),
                         ('angle_capture_replay_composite_file_id', str(composite_file_id))]
        subprocess.check_call(
            [GN_PATH, 'gen',
             '--args=%s' % GetGnArgsStr(args, extra_gn_args), replay_build_dir])
        subprocess.check_call(
            [sys.executable, AUTONINJA_PATH, '-C', replay_build_dir, REPLAY_BINARY])

        if not replay_tests:
            logging.error('No replay tests to run. Is everything skipped?')
            return EXIT_FAILURE
        logging.info('Running %d replay tests', len(replay_tests))

        expected_results = {}
        for test in replay_tests:
            expected_result = test_expectation_for_list.get(test, GroupedResult.Passed)
            if test_expectation.IsFlaky(test):
                expected_result = 'Flaky'
            expected_results[test] = expected_result

        if not RunReplayTestsInParallel(args, replay_build_dir, replay_tests, expected_results,
                                        labels_to_tests, worker_count, xvfb_pool):
            logging.error('Replay tests failed, see "Replay failed" errors above')
            return EXIT_FAILURE

        logging.info('Replay tests finished successfully')

    if not args.keep_temp_files:
        CleanupAfterReplay(replay_build_dir, list(labels_to_tests.keys()))
        shutil.rmtree(trace_folder_path)

    return EXIT_SUCCESS


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--out-dir',
        default=DEFAULT_OUT_DIR,
        help='Where to build ANGLE for capture and replay. Relative to the ANGLE folder. Default is "%s".'
        % DEFAULT_OUT_DIR)
    parser.add_argument(
        '-f',
        '--filter',
        '--gtest_filter',
        default=DEFAULT_FILTER,
        help='Same as GoogleTest\'s filter argument. Default is "%s".' % DEFAULT_FILTER)
    parser.add_argument(
        '--test-suite',
        default=DEFAULT_TEST_SUITE,
        help='Test suite binary to execute. Default is "%s".' % DEFAULT_TEST_SUITE)
    parser.add_argument(
        '--batch-count',
        default=DEFAULT_BATCH_COUNT,
        type=int,
        help='Number of tests in a (capture) batch. Default is %d.' % DEFAULT_BATCH_COUNT)
    parser.add_argument(
        '--keep-temp-files',
        action='store_true',
        help='Whether to keep the temp files and folders. Off by default')
    parser.add_argument(
        '--use-reclient',
        default=False,
        action='store_true',
        help='Set use_remoteexec=true in args.gn.')
    parser.add_argument(
        '--result-file',
        default=DEFAULT_RESULT_FILE,
        help='Name of the result file in the capture_replay_tests folder. Default is "%s".' %
        DEFAULT_RESULT_FILE)
    parser.add_argument('-v', '--verbose', action='store_true', help='Shows full test output.')
    parser.add_argument(
        '-l',
        '--log',
        default=DEFAULT_LOG_LEVEL,
        help='Controls the logging level. Default is "%s".' % DEFAULT_LOG_LEVEL)
    parser.add_argument(
        '-j',
        '--max-jobs',
        default=DEFAULT_MAX_JOBS,
        type=int,
        help='Maximum number of test processes. Default is %d.' % DEFAULT_MAX_JOBS)
    parser.add_argument(
        '-M',
        '--mec',
        default=0,
        type=int,
        help='Enable mid execution capture starting at specified frame, (default: 0 = normal capture)'
    )
    parser.add_argument(
        '-a',
        '--also-run-skipped-for-capture-tests',
        action='store_true',
        help='Also run tests that are disabled in the expectations by SKIP_FOR_CAPTURE')
    parser.add_argument('--xvfb', action='store_true', help='Run with xvfb.')
    parser.add_argument('--asan', action='store_true', help='Build with ASAN.')
    parser.add_argument(
        '-E',
        '--expose-nonconformant-features',
        action='store_true',
        help='Expose non-conformant features to advertise GLES 3.2')
    parser.add_argument(
        '--show-capture-stdout', action='store_true', help='Print test stdout during capture.')
    parser.add_argument(
        '--show-replay-stdout', action='store_true', help='Print test stdout during replay.')
    parser.add_argument('--debug', action='store_true', help='Debug builds (default is Release).')
    args = parser.parse_args()
    if args.debug and (args.out_dir == DEFAULT_OUT_DIR):
        args.out_dir = args.out_dir + "Debug"

    if sys.platform == "win32":
        args.test_suite += ".exe"

    sys.exit(main(args))
