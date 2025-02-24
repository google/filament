#! /usr/bin/env vpython3
#
# Copyright 2021 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# run_perf_test.py:
#   Runs ANGLE perf tests using some statistical averaging.

import argparse
import contextlib
import glob
import importlib
import io
import json
import logging
import tempfile
import time
import os
import pathlib
import re
import subprocess
import shutil
import sys

SCRIPT_DIR = str(pathlib.Path(__file__).resolve().parent)
PY_UTILS = str(pathlib.Path(SCRIPT_DIR) / 'py_utils')
if PY_UTILS not in sys.path:
    os.stat(PY_UTILS) and sys.path.insert(0, PY_UTILS)
import android_helper
import angle_metrics
import angle_path_util
import angle_test_util

angle_path_util.AddDepsDirToPath('testing/scripts')
import common

angle_path_util.AddDepsDirToPath('third_party/catapult/tracing')
from tracing.value import histogram
from tracing.value import histogram_set
from tracing.value import merge_histograms

DEFAULT_TEST_SUITE = 'angle_perftests'
DEFAULT_LOG = 'info'
DEFAULT_SAMPLES = 10
DEFAULT_TRIALS = 4
DEFAULT_MAX_ERRORS = 3
DEFAULT_TRIAL_TIME = 3

# Test expectations
FAIL = 'FAIL'
PASS = 'PASS'
SKIP = 'SKIP'

EXIT_FAILURE = 1
EXIT_SUCCESS = 0


@contextlib.contextmanager
def temporary_dir(prefix=''):
    path = tempfile.mkdtemp(prefix=prefix)
    try:
        yield path
    finally:
        shutil.rmtree(path)


def _shard_tests(tests, shard_count, shard_index):
    return [tests[index] for index in range(shard_index, len(tests), shard_count)]


def _get_results_from_output(output, result):
    m = re.search(r'Running (\d+) tests', output)
    if m and int(m.group(1)) > 1:
        raise Exception('Found more than one test result in output')

    # Results are reported in the format:
    # name_backend.result: story= value units.
    pattern = r'\.' + result + r':.*= ([0-9.]+)'
    logging.debug('Searching for %s in output' % pattern)
    m = re.findall(pattern, output)
    if not m:
        logging.warning('Did not find the result "%s" in the test output:\n%s' % (result, output))
        return None

    return [float(value) for value in m]


def _truncated_list(data, n):
    """Compute a truncated list, n is truncation size"""
    if len(data) < n * 2:
        raise ValueError('list not large enough to truncate')
    return sorted(data)[n:-n]


def _mean(data):
    """Return the sample arithmetic mean of data."""
    n = len(data)
    if n < 1:
        raise ValueError('mean requires at least one data point')
    return float(sum(data)) / float(n)  # in Python 2 use sum(data)/float(n)


def _sum_of_square_deviations(data, c):
    """Return sum of square deviations of sequence data."""
    ss = sum((float(x) - c)**2 for x in data)
    return ss


def _coefficient_of_variation(data):
    """Calculates the population coefficient of variation."""
    n = len(data)
    if n < 2:
        raise ValueError('variance requires at least two data points')
    c = _mean(data)
    ss = _sum_of_square_deviations(data, c)
    pvar = ss / n  # the population variance
    stddev = (pvar**0.5)  # population standard deviation
    return stddev / c


def _save_extra_output_files(args, results, histograms, metrics):
    isolated_out_dir = os.path.dirname(args.isolated_script_test_output)
    if not os.path.isdir(isolated_out_dir):
        return
    benchmark_path = os.path.join(isolated_out_dir, args.test_suite)
    if not os.path.isdir(benchmark_path):
        os.makedirs(benchmark_path)
    test_output_path = os.path.join(benchmark_path, 'test_results.json')
    results.save_to_json_file(test_output_path)
    perf_output_path = os.path.join(benchmark_path, 'perf_results.json')
    logging.info('Saving perf histograms to %s.' % perf_output_path)
    with open(perf_output_path, 'w') as out_file:
        out_file.write(json.dumps(histograms.AsDicts(), indent=2))

    angle_metrics_path = os.path.join(benchmark_path, 'angle_metrics.json')
    with open(angle_metrics_path, 'w') as f:
        f.write(json.dumps(metrics, indent=2))

    # Calling here to catch errors earlier (fail shard instead of merge script)
    assert angle_metrics.ConvertToSkiaPerf([angle_metrics_path])


class Results:

    def __init__(self, suffix):
        self._results = {
            'tests': {},
            'interrupted': False,
            'seconds_since_epoch': time.time(),
            'path_delimiter': '.',
            'version': 3,
            'num_failures_by_type': {
                FAIL: 0,
                PASS: 0,
                SKIP: 0,
            },
        }
        self._test_results = {}
        self._suffix = suffix

    def _testname(self, name):
        return name + self._suffix

    def has_failures(self):
        return self._results['num_failures_by_type'][FAIL] > 0

    def has_result(self, test):
        return self._testname(test) in self._test_results

    def result_skip(self, test):
        self._test_results[self._testname(test)] = {'expected': SKIP, 'actual': SKIP}
        self._results['num_failures_by_type'][SKIP] += 1

    def result_pass(self, test):
        self._test_results[self._testname(test)] = {'expected': PASS, 'actual': PASS}
        self._results['num_failures_by_type'][PASS] += 1

    def result_fail(self, test):
        self._test_results[self._testname(test)] = {
            'expected': PASS,
            'actual': FAIL,
            'is_unexpected': True
        }
        self._results['num_failures_by_type'][FAIL] += 1

    def save_to_output_file(self, test_suite, fname):
        self._update_results(test_suite)
        with open(fname, 'w') as out_file:
            out_file.write(json.dumps(self._results, indent=2))

    def save_to_json_file(self, fname):
        logging.info('Saving test results to %s.' % fname)
        with open(fname, 'w') as out_file:
            out_file.write(json.dumps(self._results, indent=2))

    def _update_results(self, test_suite):
        if self._test_results:
            self._results['tests'][test_suite] = self._test_results
            self._test_results = {}


def _read_histogram(histogram_file_path):
    with open(histogram_file_path) as histogram_file:
        histogram = histogram_set.HistogramSet()
        histogram.ImportDicts(json.load(histogram_file))
        return histogram


def _read_metrics(metrics_file_path):
    try:
        with open(metrics_file_path) as f:
            return [json.loads(l) for l in f]
    except FileNotFoundError:
        return []


def _merge_into_one_histogram(test_histogram_set):
    with common.temporary_file() as merge_histogram_path:
        logging.info('Writing merged histograms to %s.' % merge_histogram_path)
        with open(merge_histogram_path, 'w') as merge_histogram_file:
            json.dump(test_histogram_set.AsDicts(), merge_histogram_file)
            merge_histogram_file.close()
        merged_dicts = merge_histograms.MergeHistograms(merge_histogram_path, groupby=['name'])
        merged_histogram = histogram_set.HistogramSet()
        merged_histogram.ImportDicts(merged_dicts)
        return merged_histogram


def _wall_times_stats(wall_times):
    if len(wall_times) > 7:
        truncation_n = len(wall_times) >> 3
        logging.debug('Truncation: Removing the %d highest and lowest times from wall_times.' %
                      truncation_n)
        wall_times = _truncated_list(wall_times, truncation_n)

    if len(wall_times) > 1:
        return ('truncated mean wall_time = %.2f, cov = %.2f%%' %
                (_mean(wall_times), _coefficient_of_variation(wall_times) * 100.0))

    return None


def _run_test_suite(args, cmd_args, env):
    return angle_test_util.RunTestSuite(
        args.test_suite,
        cmd_args,
        env,
        use_xvfb=args.xvfb,
        show_test_stdout=args.show_test_stdout)


def _run_perf(args, common_args, env, steps_per_trial=None):
    run_args = common_args + [
        '--trials',
        str(args.trials_per_sample),
    ]

    if steps_per_trial:
        run_args += ['--steps-per-trial', str(steps_per_trial)]
    else:
        run_args += ['--trial-time', str(args.trial_time)]

    if not args.smoke_test_mode:
        run_args += ['--warmup']  # Render each frame once with glFinish

    if args.perf_counters:
        run_args += ['--perf-counters', args.perf_counters]

    with temporary_dir() as render_output_dir:
        histogram_file_path = os.path.join(render_output_dir, 'histogram')
        run_args += ['--isolated-script-test-perf-output=%s' % histogram_file_path]
        run_args += ['--render-test-output-dir=%s' % render_output_dir]

        exit_code, output, json_results = _run_test_suite(args, run_args, env)
        if exit_code != EXIT_SUCCESS:
            raise RuntimeError('%s failed. Output:\n%s' % (args.test_suite, output))
        if SKIP in json_results['num_failures_by_type']:
            return SKIP, None, None

        # Extract debug files for https://issuetracker.google.com/296921272
        if args.isolated_script_test_output:
            isolated_out_dir = os.path.dirname(args.isolated_script_test_output)
            for path in glob.glob(os.path.join(render_output_dir, '*gzdbg*')):
                shutil.move(path, isolated_out_dir)

        sample_metrics = _read_metrics(os.path.join(render_output_dir, 'angle_metrics'))

        if sample_metrics:
            sample_histogram = _read_histogram(histogram_file_path)
            return PASS, sample_metrics, sample_histogram

    return FAIL, None, None


class _MaxErrorsException(Exception):
    pass


def _skipped_or_glmark2(test, test_status):
    if test_status == SKIP:
        logging.info('Test skipped by suite: %s' % test)
        return True

    # GLMark2Benchmark logs .fps/.score instead of our perf metrics.
    if test.startswith('GLMark2Benchmark.Run/'):
        logging.info('GLMark2Benchmark missing metrics (as expected, skipping): %s' % test)
        return True

    return False


def _sleep_until_temps_below(limit_temp):
    while True:
        max_temp = max(android_helper.GetTemps())
        if max_temp < limit_temp:
            break
        logging.info('Waiting for device temps below %.1f, curently %.1f', limit_temp, max_temp)
        time.sleep(10)


def _maybe_throttle_or_log_temps(custom_throttling_temp):
    is_debug = logging.getLogger().isEnabledFor(logging.DEBUG)

    if angle_test_util.IsAndroid():
        if custom_throttling_temp:
            _sleep_until_temps_below(custom_throttling_temp)
        elif is_debug:
            android_helper.GetTemps()  # calls log.debug
    elif sys.platform == 'linux' and is_debug:
        out = subprocess.check_output('cat /sys/class/hwmon/hwmon*/temp*_input', shell=True)
        logging.debug('hwmon temps: %s',
                      ','.join([str(int(n) // 1000) for n in out.decode().split('\n') if n]))


def _run_tests(tests, args, extra_flags, env):
    if args.split_shard_samples and args.shard_index is not None:
        test_suffix = Results('_shard%d' % args.shard_index)
    else:
        test_suffix = ''

    results = Results(test_suffix)

    histograms = histogram_set.HistogramSet()
    metrics = []
    total_errors = 0
    prepared_traces = set()

    for test_index in range(len(tests)):
        if total_errors >= args.max_errors:
            raise _MaxErrorsException()

        test = tests[test_index]

        if angle_test_util.IsAndroid():
            trace = android_helper.GetTraceFromTestName(test)
            if trace and trace not in prepared_traces:
                android_helper.PrepareRestrictedTraces([trace])
                prepared_traces.add(trace)

        common_args = [
            '--gtest_filter=%s' % test,
            '--verbose',
        ] + extra_flags

        if args.steps_per_trial:
            steps_per_trial = args.steps_per_trial
            trial_limit = 'steps_per_trial=%d' % steps_per_trial
        else:
            steps_per_trial = None
            trial_limit = 'trial_time=%d' % args.trial_time

        logging.info('Test %d/%d: %s (samples=%d trials_per_sample=%d %s)' %
                     (test_index + 1, len(tests), test, args.samples_per_test,
                      args.trials_per_sample, trial_limit))

        wall_times = []
        test_histogram_set = histogram_set.HistogramSet()
        for sample in range(args.samples_per_test):
            try:
                _maybe_throttle_or_log_temps(args.custom_throttling_temp)
                test_status, sample_metrics, sample_histogram = _run_perf(
                    args, common_args, env, steps_per_trial)
            except RuntimeError as e:
                logging.error(e)
                results.result_fail(test)
                total_errors += 1
                break

            if _skipped_or_glmark2(test, test_status):
                results.result_skip(test)
                break

            if not sample_metrics:
                logging.error('Test %s failed to produce a sample output' % test)
                results.result_fail(test)
                break

            sample_wall_times = [
                float(m['value']) for m in sample_metrics if m['metric'] == '.wall_time'
            ]

            logging.info('Test %d/%d Sample %d/%d wall_times: %s' %
                         (test_index + 1, len(tests), sample + 1, args.samples_per_test,
                          str(sample_wall_times)))

            if len(sample_wall_times) != args.trials_per_sample:
                logging.error('Test %s failed to record some wall_times (expected %d, got %d)' %
                              (test, args.trials_per_sample, len(sample_wall_times)))
                results.result_fail(test)
                break

            wall_times += sample_wall_times
            test_histogram_set.Merge(sample_histogram)
            metrics.append(sample_metrics)

        if not results.has_result(test):
            assert len(wall_times) == (args.samples_per_test * args.trials_per_sample)
            stats = _wall_times_stats(wall_times)
            if stats:
                logging.info('Test %d/%d: %s: %s' % (test_index + 1, len(tests), test, stats))
            histograms.Merge(_merge_into_one_histogram(test_histogram_set))
            results.result_pass(test)

    return results, histograms, metrics


def _find_test_suite_directory(test_suite):
    if os.path.exists(angle_test_util.ExecutablePathInCurrentDir(test_suite)):
        return '.'

    if angle_test_util.IsWindows():
        test_suite += '.exe'

    # Find most recent binary in search paths.
    newest_binary = None
    newest_mtime = None

    for path in glob.glob('out/*'):
        binary_path = str(pathlib.Path(SCRIPT_DIR).parent.parent / path / test_suite)
        if os.path.exists(binary_path):
            binary_mtime = os.path.getmtime(binary_path)
            if (newest_binary is None) or (binary_mtime > newest_mtime):
                newest_binary = binary_path
                newest_mtime = binary_mtime

    if newest_binary:
        logging.info('Found %s in %s' % (test_suite, os.path.dirname(newest_binary)))
        return os.path.dirname(newest_binary)
    return None


def _split_shard_samples(tests, samples_per_test, shard_count, shard_index):
    test_samples = [(test, sample) for test in tests for sample in range(samples_per_test)]
    shard_test_samples = _shard_tests(test_samples, shard_count, shard_index)
    return [test for (test, sample) in shard_test_samples]


def _should_lock_gpu_clocks():
    if not angle_test_util.IsWindows():
        return False

    try:
        gpu_info = subprocess.check_output(
            ['nvidia-smi', '--query-gpu=gpu_name', '--format=csv,noheader']).decode()
    except FileNotFoundError:
        # expected in some cases, e.g. non-nvidia bots
        return False

    logging.info('nvidia-smi --query-gpu=gpu_name output: %s' % gpu_info)

    return gpu_info.strip() == 'GeForce GTX 1660'


def _log_nvidia_gpu_temperature():
    t = subprocess.check_output(
        ['nvidia-smi', '--query-gpu=temperature.gpu', '--format=csv,noheader']).decode().strip()
    logging.info('Current GPU temperature: %s ' % t)


@contextlib.contextmanager
def _maybe_lock_gpu_clocks():
    if not _should_lock_gpu_clocks():
        yield
        return

    # Lock to 1410Mhz (`nvidia-smi --query-supported-clocks=gr --format=csv`)
    lgc_out = subprocess.check_output(['nvidia-smi', '--lock-gpu-clocks=1410,1410']).decode()
    logging.info('Lock GPU clocks output: %s' % lgc_out)
    _log_nvidia_gpu_temperature()
    try:
        yield
    finally:
        rgc_out = subprocess.check_output(['nvidia-smi', '--reset-gpu-clocks']).decode()
        logging.info('Reset GPU clocks output: %s' % rgc_out)
        _log_nvidia_gpu_temperature()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--isolated-script-test-output', type=str)
    parser.add_argument('--isolated-script-test-perf-output', type=str)
    parser.add_argument(
        '-f', '--filter', '--isolated-script-test-filter', type=str, help='Test filter.')
    suite_group = parser.add_mutually_exclusive_group()
    suite_group.add_argument(
        '--test-suite', '--suite', help='Test suite to run.', default=DEFAULT_TEST_SUITE)
    suite_group.add_argument(
        '-T',
        '--trace-tests',
        help='Run with the angle_trace_tests test suite.',
        action='store_true')
    parser.add_argument('--xvfb', help='Use xvfb.', action='store_true')
    parser.add_argument(
        '--shard-count',
        help='Number of shards for test splitting. Default is 1.',
        type=int,
        default=1)
    parser.add_argument(
        '--shard-index',
        help='Index of the current shard for test splitting. Default is 0.',
        type=int,
        default=0)
    parser.add_argument(
        '-l', '--log', help='Log output level. Default is %s.' % DEFAULT_LOG, default=DEFAULT_LOG)
    parser.add_argument(
        '-s',
        '--samples-per-test',
        help='Number of samples to run per test. Default is %d.' % DEFAULT_SAMPLES,
        type=int,
        default=DEFAULT_SAMPLES)
    parser.add_argument(
        '-t',
        '--trials-per-sample',
        help='Number of trials to run per sample. Default is %d.' % DEFAULT_TRIALS,
        type=int,
        default=DEFAULT_TRIALS)
    trial_group = parser.add_mutually_exclusive_group()
    trial_group.add_argument(
        '--steps-per-trial', help='Fixed number of steps to run per trial.', type=int)
    trial_group.add_argument(
        '--trial-time',
        help='Number of seconds to run per trial. Default is %d.' % DEFAULT_TRIAL_TIME,
        type=int,
        default=DEFAULT_TRIAL_TIME)
    parser.add_argument(
        '--max-errors',
        help='After this many errors, abort the run. Default is %d.' % DEFAULT_MAX_ERRORS,
        type=int,
        default=DEFAULT_MAX_ERRORS)
    parser.add_argument(
        '--smoke-test-mode', help='Do a quick run to validate correctness.', action='store_true')
    parser.add_argument(
        '--show-test-stdout', help='Prints all test stdout during execution.', action='store_true')
    parser.add_argument(
        '--perf-counters', help='Colon-separated list of extra perf counter metrics.')
    parser.add_argument(
        '-a',
        '--auto-dir',
        help='Run with the most recent test suite found in the build directories.',
        action='store_true')
    parser.add_argument(
        '--split-shard-samples',
        help='Attempt to mitigate variance between machines by splitting samples between shards.',
        action='store_true')
    parser.add_argument(
        '--custom-throttling-temp',
        help='Android: custom thermal throttling with limit set to this temperature (off by default)',
        type=float)

    args, extra_flags = parser.parse_known_args()

    if args.trace_tests:
        args.test_suite = angle_test_util.ANGLE_TRACE_TEST_SUITE

    angle_test_util.SetupLogging(args.log.upper())

    start_time = time.time()

    # Use fast execution for smoke test mode.
    if args.smoke_test_mode:
        args.steps_per_trial = 1
        args.trials_per_sample = 1
        args.samples_per_test = 1

    env = os.environ.copy()

    if angle_test_util.HasGtestShardsAndIndex(env):
        args.shard_count, args.shard_index = angle_test_util.PopGtestShardsAndIndex(env)

    if args.auto_dir:
        test_suite_dir = _find_test_suite_directory(args.test_suite)
        if not test_suite_dir:
            logging.fatal('Could not find test suite: %s' % args.test_suite)
            return EXIT_FAILURE
        else:
            os.chdir(test_suite_dir)

    angle_test_util.Initialize(args.test_suite)

    # Get test list
    exit_code, output, _ = _run_test_suite(args, ['--list-tests', '--verbose'] + extra_flags, env)
    if exit_code != EXIT_SUCCESS:
        logging.fatal('Could not find test list from test output:\n%s' % output)
        sys.exit(EXIT_FAILURE)
    tests = angle_test_util.GetTestsFromOutput(output)

    if args.filter:
        tests = angle_test_util.FilterTests(tests, args.filter)

    # Get tests for this shard (if using sharding args)
    if args.split_shard_samples and args.shard_count >= args.samples_per_test:
        tests = _split_shard_samples(tests, args.samples_per_test, args.shard_count,
                                     args.shard_index)
        assert (len(set(tests)) == len(tests))
        args.samples_per_test = 1
    else:
        tests = _shard_tests(tests, args.shard_count, args.shard_index)

    if not tests:
        logging.error('No tests to run.')
        return EXIT_FAILURE

    logging.info('Running %d test%s' % (len(tests), 's' if len(tests) > 1 else ' '))

    try:
        with _maybe_lock_gpu_clocks():
            results, histograms, metrics = _run_tests(tests, args, extra_flags, env)
    except _MaxErrorsException:
        logging.error('Error count exceeded max errors (%d). Aborting.' % args.max_errors)
        return EXIT_FAILURE

    for test in tests:
        assert results.has_result(test)

    if args.isolated_script_test_output:
        results.save_to_output_file(args.test_suite, args.isolated_script_test_output)

        # Uses special output files to match the merge script.
        _save_extra_output_files(args, results, histograms, metrics)

    if args.isolated_script_test_perf_output:
        with open(args.isolated_script_test_perf_output, 'w') as out_file:
            out_file.write(json.dumps(histograms.AsDicts(), indent=2))

    end_time = time.time()
    logging.info('Elapsed time: %.2lf seconds.' % (end_time - start_time))

    if results.has_failures():
        return EXIT_FAILURE
    return EXIT_SUCCESS


if __name__ == '__main__':
    sys.exit(main())
