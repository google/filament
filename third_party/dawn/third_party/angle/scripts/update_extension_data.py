#!/usr/bin/python3
#
# Copyright 2021 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# update_extension_data.py:
#   Downloads and updates auto-generated extension data.

import argparse
import logging
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile

EXIT_SUCCESS = 0
EXIT_FAILURE = 1

TEST_SUITE = 'angle_end2end_tests'
BUILDERS = [
    'angle/ci/android-arm64-test', 'angle/ci/linux-test', 'angle/ci/win-test',
    'angle/ci/win-x86-test'
]
SWARMING_SERVER = 'chromium-swarm.appspot.com'

d = os.path.dirname
THIS_DIR = d(os.path.abspath(__file__))
ANGLE_ROOT_DIR = d(THIS_DIR)

# Host GPUs
INTEL_UHD630 = '8086:9bc5'
NVIDIA_GTX1660 = '10de:2184'
SWIFTSHADER = 'none'
GPUS = [INTEL_UHD630, NVIDIA_GTX1660, SWIFTSHADER]
GPU_NAME_MAP = {
    INTEL_UHD630: 'intel_630',
    NVIDIA_GTX1660: 'nvidia_1660',
    SWIFTSHADER: 'swiftshader'
}

# OSes
LINUX = 'Linux'
WINDOWS_10 = 'Windows-10'
BOT_OSES = [LINUX, WINDOWS_10]
BOT_OS_NAME_MAP = {LINUX: 'linux', WINDOWS_10: 'win10'}

# Devices
PIXEL_4 = 'flame'
PIXEL_6 = 'oriole'
DEVICES_TYPES = [PIXEL_4, PIXEL_6]
DEVICE_NAME_MAP = {PIXEL_4: 'pixel_4', PIXEL_6: 'pixel_6'}

# Device OSes
ANDROID_11 = 'R'
ANDROID_13 = 'T'
DEVICE_OSES = [ANDROID_11, ANDROID_13]
DEVICE_OS_NAME_MAP = {ANDROID_11: 'android_11', ANDROID_13: 'android_13'}

# Result names
INFO_FILES = [
    'GLinfo_ES3_2_Vulkan.json',
    'GLinfo_ES3_1_Vulkan_SwiftShader.json',
]

LOG_LEVELS = ['WARNING', 'INFO', 'DEBUG']


def run_and_get_json(args):
    logging.debug(' '.join(args))
    output = subprocess.check_output(args)
    return json.loads(output)


def get_bb():
    return 'bb.bat' if os.name == 'nt' else 'bb'


def run_bb_and_get_output(*args):
    bb_args = [get_bb()] + list(args)
    return subprocess.check_output(bb_args, encoding='utf-8')


def run_bb_and_get_json(*args):
    bb_args = [get_bb()] + list(args) + ['-json']
    return run_and_get_json(bb_args)


def get_swarming():
    swarming_bin = 'swarming.exe' if os.name == 'nt' else 'swarming'
    return os.path.join(ANGLE_ROOT_DIR, 'tools', 'luci-go', swarming_bin)


def run_swarming(*args):
    swarming_args = [get_swarming()] + list(args)
    logging.debug(' '.join(swarming_args))
    subprocess.check_call(swarming_args)


def run_swarming_and_get_json(*args):
    swarming_args = [get_swarming()] + list(args)
    return run_and_get_json(swarming_args)


def name_device(gpu, device_type):
    if gpu:
        return GPU_NAME_MAP[gpu]
    else:
        assert device_type
        return DEVICE_NAME_MAP[device_type]


def name_os(bot_os, device_os):
    if bot_os:
        return BOT_OS_NAME_MAP[bot_os]
    else:
        assert device_os
        return DEVICE_OS_NAME_MAP[device_os]


def get_props_string(gpu, bot_os, device_os, device_type):
    d = {'gpu': gpu, 'os': bot_os, 'device os': device_os, 'device': device_type}
    return ', '.join('%s %s' % (k, v) for (k, v) in d.items() if v)


def collect_task_and_update_json(task_id, found_dims):
    gpu = found_dims.get('gpu', None)
    bot_os = found_dims.get('os', None)
    device_os = found_dims.get('device_os', None)
    device_type = found_dims.get('device_type', None)
    logging.info('Found task with ID: %s, %s' %
                 (task_id, get_props_string(gpu, bot_os, device_os, device_type)))
    target_file_name = '%s_%s.json' % (name_device(gpu, device_type), name_os(bot_os, device_os))
    target_file = os.path.join(THIS_DIR, 'extension_data', target_file_name)
    with tempfile.TemporaryDirectory() as tempdirname:
        run_swarming('collect', '-S', SWARMING_SERVER, '-output-dir=%s' % tempdirname, task_id)
        task_dir = os.path.join(tempdirname, task_id)
        found = False
        for fname in os.listdir(task_dir):
            if fname in INFO_FILES:
                if found:
                    logging.warning('Multiple candidates found for %s' % target_file_name)
                    return
                else:
                    logging.info('%s -> %s' % (fname, target_file))
                    found = True
                    source_file = os.path.join(task_dir, fname)
                    shutil.copy(source_file, target_file)


def get_intersect_or_none(list_a, list_b):
    i = [v for v in list_a if v in list_b]
    assert not i or len(i) == 1
    return i[0] if i else None


def main():
    parser = argparse.ArgumentParser(description='Pulls extension support data from ANGLE CI.')
    parser.add_argument(
        '-v', '--verbose', help='Print additional debugging into.', action='count', default=0)
    args = parser.parse_args()

    if args.verbose >= len(LOG_LEVELS):
        args.verbose = len(LOG_LEVELS) - 1
    logging.basicConfig(level=LOG_LEVELS[args.verbose])

    name_expr = re.compile(r'^' + TEST_SUITE + r' on (.*) on (.*)$')

    for builder in BUILDERS:

        # Step 1: Find the build ID.
        # We list two builds using 'bb ls' and take the second, to ensure the build is finished.
        ls_output = run_bb_and_get_output('ls', builder, '-n', '2', '-id')
        build_id = ls_output.splitlines()[1]
        logging.info('%s: build id %s' % (builder, build_id))

        # Step 2: Get the test suite swarm hashes.
        # 'bb get' returns build properties, including cloud storage identifiers for this test suite.
        get_json = run_bb_and_get_json('get', build_id, '-p')
        test_suite_hash = get_json['output']['properties']['swarm_hashes'][TEST_SUITE]
        logging.info('Found swarm hash: %s' % test_suite_hash)

        # Step 3: Find all tasks using the swarm hashes.
        # 'swarming tasks' can find instances of the test suite that ran on specific systems.
        task_json = run_swarming_and_get_json('tasks', '-tag', 'data:%s' % test_suite_hash, '-S',
                                              SWARMING_SERVER)

        # Step 4: Download the extension data for each configuration we're monitoring.
        # 'swarming collect' downloads test artifacts to a temporary directory.
        dim_map = {
            'gpu': GPUS,
            'os': BOT_OSES,
            'device_os': DEVICE_OSES,
            'device_type': DEVICES_TYPES,
        }

        for task in task_json:
            found_dims = {}
            for bot_dim in task['bot_dimensions']:
                key, value = bot_dim['key'], bot_dim['value']
                if key in dim_map:
                    logging.debug('%s=%s' % (key, value))
                    mapped_values = dim_map[key]
                    found_dim = get_intersect_or_none(mapped_values, value)
                    if found_dim:
                        found_dims[key] = found_dim
            found_gpu_or_device = ('gpu' in found_dims or 'device_type' in found_dims)
            found_os = ('os' in found_dims or 'device_os' in found_dims)
            if found_gpu_or_device and found_os:
                collect_task_and_update_json(task['task_id'], found_dims)

    return EXIT_SUCCESS


if __name__ == '__main__':
    sys.exit(main())
