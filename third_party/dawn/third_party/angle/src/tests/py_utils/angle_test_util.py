# Copyright 2022 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import contextlib
import datetime
import fnmatch
import json
import importlib
import io
import logging
import os
import signal
import subprocess
import sys
import threading
import time

import android_helper
import angle_path_util

angle_path_util.AddDepsDirToPath('testing/scripts')
import common
if sys.platform.startswith('linux'):
    # vpython3 can handle this on Windows but not python3
    import xvfb


ANGLE_TRACE_TEST_SUITE = 'angle_trace_tests'


def Initialize(suite_name):
    android_helper.Initialize(suite_name)


# Requires .Initialize() to be called first
def IsAndroid():
    return android_helper.IsAndroid()


class LogFormatter(logging.Formatter):

    def __init__(self):
        logging.Formatter.__init__(self, fmt='%(levelname).1s%(asctime)s %(message)s')

    def formatTime(self, record, datefmt=None):
        # Drop date as these scripts are short lived
        return datetime.datetime.fromtimestamp(record.created).strftime('%H:%M:%S.%fZ')


def SetupLogging(level):
    # Reload to reset if it was already setup by a library
    importlib.reload(logging)

    logger = logging.getLogger()
    logger.setLevel(level)

    handler = logging.StreamHandler(sys.stdout)
    handler.setFormatter(LogFormatter())
    logger.addHandler(handler)


def IsWindows():
    return sys.platform == 'cygwin' or sys.platform.startswith('win')


def ExecutablePathInCurrentDir(binary):
    if IsWindows():
        return '.\\%s.exe' % binary
    else:
        return './%s' % binary


def HasGtestShardsAndIndex(env):
    if 'GTEST_TOTAL_SHARDS' in env and int(env['GTEST_TOTAL_SHARDS']) != 1:
        if 'GTEST_SHARD_INDEX' not in env:
            logging.error('Sharding params must be specified together.')
            sys.exit(1)
        return True

    return False


def PopGtestShardsAndIndex(env):
    return int(env.pop('GTEST_TOTAL_SHARDS')), int(env.pop('GTEST_SHARD_INDEX'))


# Adapted from testing/test_env.py: also notifies current process and restores original handlers.
@contextlib.contextmanager
def forward_signals(procs):
    assert all(isinstance(p, subprocess.Popen) for p in procs)

    interrupted_event = threading.Event()

    def _sig_handler(sig, _):
        interrupted_event.set()
        for p in procs:
            if p.poll() is not None:
                continue
            # SIGBREAK is defined only for win32.
            # pylint: disable=no-member
            if sys.platform == 'win32' and sig == signal.SIGBREAK:
                p.send_signal(signal.CTRL_BREAK_EVENT)
            else:
                print("Forwarding signal(%d) to process %d" % (sig, p.pid))
                p.send_signal(sig)
            # pylint: enable=no-member

    if sys.platform == 'win32':
        signals = [signal.SIGBREAK]  # pylint: disable=no-member
    else:
        signals = [signal.SIGINT, signal.SIGTERM]

    original_handlers = {}
    for sig in signals:
        original_handlers[sig] = signal.signal(sig, _sig_handler)

    yield

    for sig, handler in original_handlers.items():
        signal.signal(sig, handler)

    if interrupted_event.is_set():
        raise KeyboardInterrupt()


# From testing/test_env.py, see run_command_with_output below
def _popen(*args, **kwargs):
    assert 'creationflags' not in kwargs
    if sys.platform == 'win32':
        # Necessary for signal handling. See crbug.com/733612#c6.
        kwargs['creationflags'] = subprocess.CREATE_NEW_PROCESS_GROUP
    return subprocess.Popen(*args, **kwargs)


# Forked from testing/test_env.py to add ability to suppress logging with log=False
def run_command_with_output(argv, stdoutfile, env=None, cwd=None, log=True):
    assert stdoutfile
    with io.open(stdoutfile, 'wb') as writer, \
          io.open(stdoutfile, 'rb') as reader:
        process = _popen(argv, env=env, cwd=cwd, stdout=writer, stderr=subprocess.STDOUT)
        with forward_signals([process]):
            while process.poll() is None:
                if log:
                    sys.stdout.write(reader.read().decode('utf-8'))
                # This sleep is needed for signal propagation. See the
                # wait_with_signals() docstring.
                time.sleep(0.1)
            if log:
                sys.stdout.write(reader.read().decode('utf-8'))
            return process.returncode


def RunTestSuite(test_suite,
                 cmd_args,
                 env,
                 show_test_stdout=True,
                 use_xvfb=False):
    if android_helper.IsAndroid():
        result, output, json_results = android_helper.RunTests(
            test_suite, cmd_args, log_output=show_test_stdout)
        return result, output, json_results

    cmd = ExecutablePathInCurrentDir(test_suite) if os.path.exists(
        os.path.basename(test_suite)) else test_suite
    runner_cmd = [cmd] + cmd_args

    logging.debug(' '.join(runner_cmd))
    with contextlib.ExitStack() as stack:
        stdout_path = stack.enter_context(common.temporary_file())

        flag_matches = [a for a in cmd_args if a.startswith('--isolated-script-test-output=')]
        if flag_matches:
            results_path = flag_matches[0].split('=')[1]
        else:
            results_path = stack.enter_context(common.temporary_file())
            runner_cmd += ['--isolated-script-test-output=%s' % results_path]

        if use_xvfb:
            # Default changed to '--use-xorg', which fails per http://crbug.com/40257169#comment34
            # '--use-xvfb' forces the old working behavior
            runner_cmd += ['--use-xvfb']
            xvfb_whd = '3120x3120x24'  # Max screen dimensions from traces, as per:
            # % egrep 'Width|Height' src/tests/restricted_traces/*/*.json | awk '{print $3 $2}' | sort -n
            exit_code = xvfb.run_executable(
                runner_cmd, env, stdoutfile=stdout_path, xvfb_whd=xvfb_whd)
        else:
            exit_code = run_command_with_output(
                runner_cmd, env=env, stdoutfile=stdout_path, log=show_test_stdout)
        with open(stdout_path) as f:
            output = f.read()
        with open(results_path) as f:
            data = f.read()
            json_results = json.loads(data) if data else None  # --list-tests => empty file

    return exit_code, output, json_results


def GetTestsFromOutput(output):
    out_lines = output.split('\n')
    try:
        start = out_lines.index('Tests list:')
        end = out_lines.index('End tests list.')
    except ValueError as e:
        logging.exception(e)
        return None
    return out_lines[start + 1:end]


def FilterTests(tests, test_filter):
    matches = set()
    for single_filter in test_filter.split(':'):
        matches.update(fnmatch.filter(tests, single_filter))
    return sorted(matches)
