#!/usr/bin/python3
#
# Copyright 2023 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# angle_deqp_bundle.py:
#   Makes a zip bundle allowing to run deqp tests, similarly to mb.py but
#    - trims most of the dependencies
#    - includes run_tests.sh (see below)
#    - lib.unstripped only included if --include-unstripped-libs
#    - does not depend on vpython
#    - just adds files to the zip instead of "isolate remap" with a temp dir
#
#  Example usage:
#    % scripts/angle_deqp_bundle.py out/Android angle_deqp_gles31_rotate90_tests angle_deqp_bundle.zip
#
#    (transfer the zip elsewhere)
#    % unzip angle_deqp_bundle.zip -d angle_deqp_bundle
#    % angle_deqp_bundle/run_tests.sh --list-tests
#    % angle_deqp_bundle/run_tests.sh --gtest_filter='dEQP-GLES31.functional.primitive_bounding_box.triangles.*'

import argparse
import json
import os
import subprocess
import sys
import zipfile

RUN_TESTS_TEMPLATE = r'''#!/bin/bash
cd "$(dirname "$0")"
python3 src/tests/angle_android_test_runner.py gtest --suite={suite} --output-directory={gn_dir} "$@"
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('gn_dir', help='path to GN. (e.g. out/Android)')
    parser.add_argument('suite', help='dEQP suite (e.g. angle_deqp_gles31_rotate90_tests)')
    parser.add_argument('output_zip_file', help='output zip file')
    parser.add_argument(
        '--include-unstripped-libs', action='store_true', help='include lib.unstripped')
    args, _ = parser.parse_known_args()

    gn_dir = os.path.join(os.path.normpath(args.gn_dir), '')
    suite = args.suite
    assert os.path.sep == '/' and gn_dir.endswith('/')
    assert gn_dir[0] not in ('.', '/')  # expecting relative to angle root

    subprocess.check_call([
        'python3', 'tools/mb/mb.py', 'isolate', gn_dir, suite, '-i',
        'infra/specs/gn_isolate_map.pyl'
    ])

    with open(os.path.join(args.gn_dir, '%s.isolate' % suite)) as f:
        isolate_file_paths = json.load(f)['variables']['files']

    # Currently not in deps, add manually
    isolate_file_paths.append('../../src/tests/angle_android_test_runner.py')

    skipped_prefixes = [
        'build/',
        'third_party/catapult/',
        'third_party/colorama/',
        'third_party/jdk/',
        'third_party/jinja2/',
        'third_party/logdog/',
        'third_party/r8/',
        'third_party/requests/',
        os.path.join(gn_dir, 'lib.java/'),
        os.path.join(gn_dir, 'obj/'),
    ]

    if not args.include_unstripped_libs:
        skipped_prefixes.append(os.path.join(gn_dir, 'lib.unstripped/'))

    with zipfile.ZipFile(args.output_zip_file, 'w', zipfile.ZIP_DEFLATED, allowZip64=True) as fzip:
        for fn in isolate_file_paths:
            path = os.path.normpath(os.path.join(gn_dir, fn))
            if any(path.startswith(p) for p in skipped_prefixes):
                continue

            fzip.write(path)

        # Create a script directly inside the zip file
        info = zipfile.ZipInfo('run_tests.sh')
        info.external_attr = 0o755 << 16  # unnecessarily obscure way to chmod 755...
        fzip.writestr(info, RUN_TESTS_TEMPLATE.format(gn_dir=gn_dir, suite=suite))

    return 0


if __name__ == '__main__':
    sys.exit(main())
