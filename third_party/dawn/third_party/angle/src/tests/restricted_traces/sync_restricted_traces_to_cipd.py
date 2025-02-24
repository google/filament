#!/usr/bin/python3
#
# Copyright 2021 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# sync_restricted_traces_to_cipd.py:
#   Ensures the restricted traces are uploaded to CIPD. Versions are encoded in
#   restricted_traces.json. Requires access to the CIPD path to work.

import argparse
from concurrent import futures
import getpass
import fnmatch
import logging
import json
import os
import platform
import signal
import subprocess
import sys

CIPD_PREFIX = 'angle/traces'
EXPERIMENTAL_CIPD_PREFIX = 'experimental/google.com/%s/angle/traces'
LOG_LEVEL = 'info'
JSON_PATH = 'restricted_traces.json'
SCRIPT_DIR = os.path.dirname(sys.argv[0])
MAX_THREADS = 8
LONG_TIMEOUT = 100000


def cipd(args, suppress_stdout=True):
    logging.debug('running cipd with args: %s', ' '.join(args))
    exe = 'cipd.bat' if platform.system() == 'Windows' else 'cipd'
    if suppress_stdout:
        # Capture stdout, only log if --log=debug after the process terminates
        process = subprocess.run([exe] + args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        if process.stdout:
            logging.debug('cipd stdout:\n%s' % process.stdout.decode())
    else:
        # Stdout is piped to the caller's stdout, visible immediately
        process = subprocess.run([exe] + args)
    return process.returncode


def cipd_name_and_version(trace, trace_version):
    if 'x' in trace_version:
        trace_prefix = EXPERIMENTAL_CIPD_PREFIX % getpass.getuser()
        trace_version = trace_version.strip('x')
    else:
        trace_prefix = CIPD_PREFIX

    trace_name = '%s/%s' % (trace_prefix, trace)

    return trace_name, trace_version


def check_trace_exists(args, trace, trace_version):
    cipd_trace_name, cipd_trace_version = cipd_name_and_version(trace, trace_version)

    # Determine if this version exists
    return cipd(['describe', cipd_trace_name, '-version', 'version:%s' % cipd_trace_version]) == 0


def upload_trace(args, trace, trace_version):
    trace_folder = os.path.join(SCRIPT_DIR, trace)
    cipd_trace_name, cipd_trace_version = cipd_name_and_version(trace, trace_version)
    cipd_args = ['create', '-name', cipd_trace_name]
    cipd_args += ['-in', trace_folder]
    cipd_args += ['-tag', 'version:%s' % cipd_trace_version]
    cipd_args += ['-log-level', args.log.lower()]
    cipd_args += ['-install-mode', 'copy']
    if cipd(cipd_args, suppress_stdout=False) != 0:
        logging.error('%s version %s: cipd create failed', trace, trace_version)
        sys.exit(1)

    logging.info('Uploaded trace to cipd: %s version:%s', cipd_trace_name, cipd_trace_version)


def check_trace_before_upload(trace):
    for root, dirs, files in os.walk(os.path.join(SCRIPT_DIR, trace)):
        if dirs:
            logging.error('Sub-directories detected for trace %s: %s' % (trace, dirs))
            sys.exit(1)
        trace_json = trace + '.json'
        with open(os.path.join(root, trace_json)) as f:
            jtrace = json.load(f)
        additional_files = set([trace_json, trace + '.angledata.gz'])
        extra_files = set(files) - set(jtrace['TraceFiles']) - additional_files
        required_extensions = 'RequiredExtensions' in jtrace
        if extra_files:
            logging.error('Unexpected files, not listed in %s.json [TraceFiles]:\n%s', trace,
                          '\n'.join(extra_files))
        if not required_extensions:
            logging.error(
                '"RequiredExtensions" missing from %s.json. Please run retrace_restricted_traces.py with "get_min_reqs":\n'
                '  ./src/tests/restricted_traces/retrace_restricted_traces.py get_min_reqs out/LinuxDebug --traces "%s"\n',
                trace, trace)
        if extra_files or not required_extensions:
            sys.exit(1)


def main(args):
    logging.basicConfig(level=args.log.upper())

    with open(os.path.join(SCRIPT_DIR, JSON_PATH)) as f:
        traces = json.loads(f.read())

    logging.info('Checking cipd for existing versions (this takes time without --filter)')
    f_exists = {}
    trace_versions = {}
    with futures.ThreadPoolExecutor(max_workers=args.threads) as executor:
        for trace_info in traces['traces']:
            trace, trace_version = trace_info.split(' ')
            trace_versions[trace] = trace_version
            if args.filter and not fnmatch.fnmatch(trace, args.filter):
                logging.debug('Skipping %s because it does not match the test filter.' % trace)
                continue
            assert trace not in f_exists
            f_exists[trace] = executor.submit(check_trace_exists, args, trace, trace_version)

    to_upload = [trace for trace, f in f_exists.items() if not f.result()]
    if not to_upload:
        logging.info('All traces are in sync with cipd')
        return 0

    logging.info('The following traces are out of sync with cipd:')
    for trace in to_upload:
        print(' ', trace, trace_versions[trace])
        check_trace_before_upload(trace)

    if args.upload or input('Upload [y/N]?') == 'y':
        for trace in to_upload:
            upload_trace(args, trace, trace_versions[trace])
    else:
        logging.error('Aborted')
        return 1

    return 0


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-p', '--prefix', help='CIPD Prefix. Default: %s' % CIPD_PREFIX, default=CIPD_PREFIX)
    parser.add_argument(
        '-l', '--log', help='Logging level. Default: %s' % LOG_LEVEL, default=LOG_LEVEL)
    parser.add_argument(
        '-f', '--filter', help='Only sync specified tests. Supports fnmatch expressions.')
    parser.add_argument(
        '-t',
        '--threads',
        help='Maxiumum parallel threads. Default: %s' % MAX_THREADS,
        default=MAX_THREADS)
    parser.add_argument('--upload', action='store_true', help='Upload without asking.')
    args = parser.parse_args()

    logging.basicConfig(level=args.log.upper())

    sys.exit(main(args))
