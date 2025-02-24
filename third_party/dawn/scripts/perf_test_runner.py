#!/usr/bin/env python3
#
# Copyright 2019 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Based on Angle's perf_test_runner.py

import glob
import subprocess
import sys
import os
import re

# Assume running the dawn_perf_tests build in Chromium checkout
# Dawn locates at /path/to/Chromium/src/third_party/dawn/
# Chromium build usually locates at /path/to/Chromium/src/out/Release/
# You might want to change the base_path if you want to run dawn_perf_tests build from a Dawn standalone build.
base_path = os.path.abspath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../../'))

# Look for a [Rr]elease build.
perftests_paths = glob.glob('out/*elease*')
metric = 'wall_time'
max_experiments = 10

binary_name = 'dawn_perf_tests'
if sys.platform == 'win32':
    binary_name += '.exe'

scores = []


def mean(data):
    """Return the sample arithmetic mean of data."""
    n = len(data)
    if n < 1:
        raise ValueError('mean requires at least one data point')
    return float(sum(data)) / float(n)  # in Python 2 use sum(data)/float(n)


def sum_of_square_deviations(data, c):
    """Return sum of square deviations of sequence data."""
    ss = sum((float(x) - c)**2 for x in data)
    return ss


def coefficient_of_variation(data):
    """Calculates the population coefficient of variation."""
    n = len(data)
    if n < 2:
        raise ValueError('variance requires at least two data points')
    c = mean(data)
    ss = sum_of_square_deviations(data, c)
    pvar = ss / n  # the population variance
    stddev = (pvar**0.5)  # population standard deviation
    return stddev / c


def truncated_list(data, n):
    """Compute a truncated list, n is truncation size"""
    if len(data) < n * 2:
        raise ValueError('list not large enough to truncate')
    return sorted(data)[n:-n]


def truncated_mean(data, n):
    """Compute a truncated mean, n is truncation size"""
    return mean(truncated_list(data, n))


def truncated_cov(data, n):
    """Compute a truncated coefficient of variation, n is truncation size"""
    return coefficient_of_variation(truncated_list(data, n))


# Find most recent binary
newest_binary = None
newest_mtime = None

for path in perftests_paths:
    binary_path = os.path.join(base_path, path, binary_name)
    if os.path.exists(binary_path):
        binary_mtime = os.path.getmtime(binary_path)
        if (newest_binary is None) or (binary_mtime > newest_mtime):
            newest_binary = binary_path
            newest_mtime = binary_mtime

perftests_path = newest_binary

if perftests_path == None or not os.path.exists(perftests_path):
    print('Cannot find Release %s!' % binary_name)
    sys.exit(1)

if len(sys.argv) >= 2:
    test_name = sys.argv[1]

print('Using test executable: ' + perftests_path)
print('Test name: ' + test_name)


def get_results(metric, extra_args=[]):
    process = subprocess.Popen(
        [perftests_path, '--gtest_filter=' + test_name] + extra_args,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)
    output, err = process.communicate()

    output_string = output.decode('utf-8')

    m = re.search(r"Running (\d+) tests", output_string)
    if m and int(m.group(1)) > 1:
        print("Found more than one test result in output:")
        print(output_string)
        sys.exit(3)

    pattern = metric + r'.*= ([0-9.]+)'
    m = re.findall(pattern, output_string)
    if not m:
        print("Did not find the metric '%s' in the test output:" % metric)
        print(output_string)
        sys.exit(1)

    return [float(value) for value in m]


# Calibrate the number of steps
steps = get_results("steps", ["--calibration"])[0]
print("running with %d steps." % steps)

# Loop 'max_experiments' times, running the tests.
for experiment in range(max_experiments):
    experiment_scores = get_results(metric, ["--override-steps", str(steps)])

    for score in experiment_scores:
        sys.stdout.write("%s: %.2f" % (metric, score))
        scores.append(score)

        if (len(scores) > 1):
            sys.stdout.write(", mean: %.2f" % mean(scores))
            sys.stdout.write(", variation: %.2f%%" %
                             (coefficient_of_variation(scores) * 100.0))

        if (len(scores) > 7):
            truncation_n = len(scores) >> 3
            sys.stdout.write(", truncated mean: %.2f" %
                             truncated_mean(scores, truncation_n))
            sys.stdout.write(", variation: %.2f%%" %
                             (truncated_cov(scores, truncation_n) * 100.0))

        print("")
