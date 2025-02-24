#!/usr/bin/env python3
# Copyright 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging
import os
import sys
import unittest

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT_DIR)

import roll_downstream_gcs_deps


class CopyPackageTest(unittest.TestCase):

    def testNoDepsToRoll(self):
        with self.assertRaises(Exception):
            roll_downstream_gcs_deps.copy_packages('', '', ['foo'], ['foo'])

        with self.assertRaises(Exception):
            roll_downstream_gcs_deps.copy_packages('deps = {"foo": ""}',
                                                   'deps = {}', ['foo'],
                                                   ['foo'])

        with self.assertRaises(Exception):
            roll_downstream_gcs_deps.copy_packages('deps = {"foo": ""}',
                                                   'deps = {"bar": ""}',
                                                   ['foo'], ['baz'])

    def testNoGCSDeps(self):
        source_deps = '''
deps = {
  "foo": {
    'dep_type': 'unknown',
    'objects': [
      {
        'object_name': 'foo-v2.tar.xz',
        'sha256sum': '1111111111111111111111111111111111111111111111111111111111111111',
        'size_bytes': 101,
        'generation': 901,
      },
    ]
  },
}
'''
        destination_deps = '''
deps = {
  "foo": {
    'dep_type': 'unknown',
    'objects': [
      {
        'object_name': 'foo.tar.xz',
        'sha256sum': '0000000000000000000000000000000000000000000000000000000000000000',
        'size_bytes': 100,
        'generation': 900,
      },
    ]
  },
}
'''
        with self.assertRaises(Exception):
            roll_downstream_gcs_deps.copy_packages(source_deps,
                                                   destination_deps, ['foo'],
                                                   ['foo'])

    def testObjectInLineUpdate(self):
        source_deps = '''
deps = {
  "foo": {
    'dep_type': 'gcs',
    'bucket': 'foo',
    'condition': 'deps source condition',
    'objects': [
      {
        'object_name': 'foo-v2.tar.xz',
        'sha256sum': '1111111111111111111111111111111111111111111111111111111111111111',
        'size_bytes': 101,
        'generation': 901,
        'condition': 'host_os == "linux" and non_git_source and new',
      },
    ]
  },
}
'''
        destination_deps = '''
deps = {
  "other": "preserved",
  "foo": {
    'dep_type': 'gcs',
    'bucket': 'foo',
    'condition': 'deps dest condition',
    'objects': [
      {
        'object_name': 'foo.tar.xz',
        'sha256sum': '0000000000000000000000000000000000000000000000000000000000000000',
        'size_bytes': 100,
        'generation': 900,
        'condition': 'host_os == "linux" and non_git_source',
      },
    ]
  },
  "another": "preserved",
}
'''
        expected_deps = '''
deps = {
  "other": "preserved",
  "foo": {
    'dep_type': 'gcs',
    'bucket': 'foo',
    'condition': 'deps dest condition',
    'objects': [
      {
        'object_name': 'foo-v2.tar.xz',
        'sha256sum': '1111111111111111111111111111111111111111111111111111111111111111',
        'size_bytes': 101,
        'generation': 901,
        'condition': 'host_os == "linux" and non_git_source and new',
      },
    ]
  },
  "another": "preserved",
}
'''
        result = roll_downstream_gcs_deps.copy_packages(source_deps,
                                                        destination_deps,
                                                        ['foo'], ['foo'])
        self.assertEqual(result, expected_deps)

    def testGCSRustPackageNewPlatform(self):
        source_deps = '''
deps = {
  'src/third_party/rust-toolchain': {
    'dep_type': 'gcs',
    'bucket': 'chromium-browser-clang',
    'objects': [
      {
        'object_name': 'Linux_x64/rust-toolchain-595316b4006932405a63862d8fe65f71a6356293-3-llvmorg-20-init-1009-g7088a5ed.tar.xz',
        'sha256sum': '560c02da5300f40441992ef639d83cee96cae3584c3d398704fdb2f02e475bbf',
        'size_bytes': 152024840,
        'generation': 1722663990116408,
        'condition': 'host_os == "linux" and non_git_source',
      },
      {
        'object_name': 'Mac/rust-toolchain-595316b4006932405a63862d8fe65f71a6356293-3-llvmorg-20-init-1009-g7088a5ed.tar.xz',
        'sha256sum': '9f39154b4337438fd170e729ed2ae4c978b22f11708d683c28265bd096df17a5',
        'size_bytes': 144459260,
        'generation': 1722663991651609,
        'condition': 'host_os == "mac" and host_cpu == "x64"',
      },
      {
        'object_name': 'Mac_arm64/rust-toolchain-595316b4006932405a63862d8fe65f71a6356293-3-llvmorg-20-init-1009-g7088a5ed.tar.xz',
        'sha256sum': '4b89cf125ffa39e8fc74f01ec3beeb632fd3069478d8c6cc4fcae506b4917151',
        'size_bytes': 135571272,
        'generation': 1722663993205996,
        'condition': 'host_os == "mac" and host_cpu == "arm64"',
      },
      {
        'object_name': 'Win/rust-toolchain-595316b4006932405a63862d8fe65f71a6356293-3-llvmorg-20-init-1009-g7088a5ed.tar.xz',
        'sha256sum': '3f6a1a87695902062a6575632552b9f2cbbbcda1907fe3232f49b8ea29baecf5',
        'size_bytes': 208844028,
        'generation': 1722663994756449,
        'condition': 'host_os == "win"',
      },
    ],
  },
}
'''
        destination_deps = '''
deps = {
  'third_party/rust-toolchain': {
    'dep_type': 'gcs',
    'bucket': 'chromium-browser-clang',
    'objects': [
      {
        'object_name': 'Linux_x64/rust-toolchain-0000000000000000000000000000000000000000-0.tar.xz',
        'sha256sum': '0000000000000000000000000000000000000000000000000000000000000000',
        'size_bytes': 123,
        'generation': 987,
        'condition': 'other condition',
      },
    ],
  },
}
'''
        expected_deps = '''
deps = {
  'third_party/rust-toolchain': {
    'dep_type': 'gcs',
    'bucket': 'chromium-browser-clang',
    'objects': [
      {
        'object_name': 'Linux_x64/rust-toolchain-595316b4006932405a63862d8fe65f71a6356293-3-llvmorg-20-init-1009-g7088a5ed.tar.xz',
        'sha256sum': '560c02da5300f40441992ef639d83cee96cae3584c3d398704fdb2f02e475bbf',
        'size_bytes': 152024840,
        'generation': 1722663990116408,
        'condition': 'host_os == "linux" and non_git_source',
      },
      {
        'object_name': 'Mac/rust-toolchain-595316b4006932405a63862d8fe65f71a6356293-3-llvmorg-20-init-1009-g7088a5ed.tar.xz',
        'sha256sum': '9f39154b4337438fd170e729ed2ae4c978b22f11708d683c28265bd096df17a5',
        'size_bytes': 144459260,
        'generation': 1722663991651609,
        'condition': 'host_os == "mac" and host_cpu == "x64"',
      },
      {
        'object_name': 'Mac_arm64/rust-toolchain-595316b4006932405a63862d8fe65f71a6356293-3-llvmorg-20-init-1009-g7088a5ed.tar.xz',
        'sha256sum': '4b89cf125ffa39e8fc74f01ec3beeb632fd3069478d8c6cc4fcae506b4917151',
        'size_bytes': 135571272,
        'generation': 1722663993205996,
        'condition': 'host_os == "mac" and host_cpu == "arm64"',
      },
      {
        'object_name': 'Win/rust-toolchain-595316b4006932405a63862d8fe65f71a6356293-3-llvmorg-20-init-1009-g7088a5ed.tar.xz',
        'sha256sum': '3f6a1a87695902062a6575632552b9f2cbbbcda1907fe3232f49b8ea29baecf5',
        'size_bytes': 208844028,
        'generation': 1722663994756449,
        'condition': 'host_os == "win"',
      },
    ],
  },
}
'''
        result = roll_downstream_gcs_deps.copy_packages(
            source_deps, destination_deps, ['src/third_party/rust-toolchain'],
            ['third_party/rust-toolchain'])
        self.assertEqual(result, expected_deps)

        with self.assertRaises(Exception):
            # no destination_package match, so expect a failure.
            roll_downstream_gcs_deps.copy_packages(
                source_deps, destination_deps,
                ['src/third_party/rust-toolchain'],
                ['src/third_party/rust-toolchain'])


if __name__ == '__main__':
    level = logging.DEBUG if '-v' in sys.argv else logging.FATAL
    logging.basicConfig(level=level,
                        format='%(asctime).19s %(levelname)s %(filename)s:'
                        '%(lineno)s %(message)s')
    unittest.main()
