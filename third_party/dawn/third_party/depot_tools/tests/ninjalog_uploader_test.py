#!/usr/bin/env python3
# Copyright (c) 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import http
import json
import os
import sys
import unittest
import unittest.mock
import urllib.error

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT_DIR)

import ninjalog_uploader


class NinjalogUploaderTest(unittest.TestCase):

    def test_parse_gn_args(self):
        gn_args, explicit_keys = ninjalog_uploader.ParseGNArgs(json.dumps([]))
        self.assertEqual(gn_args, {})
        self.assertEqual(explicit_keys, [])

        # Extract current configs from GN's output json.
        gn_args, explicit_keys = ninjalog_uploader.ParseGNArgs(
            json.dumps([
                {
                    'current': {
                        'value': 'true',
                        'file': '//path/to/args.gn',
                    },
                    'default': {
                        'value': 'false'
                    },
                    'name': 'is_component_build'
                },
                {
                    'default': {
                        'value': '"x64"'
                    },
                    'name': 'host_cpu'
                },
            ]))

        self.assertEqual(gn_args, {
            'is_component_build': 'true',
            'host_cpu': 'x64',
        })
        self.assertEqual(explicit_keys, ['is_component_build'])

        gn_args, explicit_keys = ninjalog_uploader.ParseGNArgs(
            json.dumps([
                {
                    'current': {
                        'value': 'true',
                        'file': '//.gn',
                    },
                    'default': {
                        'value': 'false'
                    },
                    'name': 'is_component_build'
                },
                {
                    'current': {
                        'value': 'false',
                        'file': '//path/to/args.gn',
                    },
                    'default': {
                        'value': 'false'
                    },
                    'name': 'use_remoteexec'
                },
            ]))
        self.assertEqual(gn_args, {
            'is_component_build': 'true',
            'use_remoteexec': 'false'
        })
        self.assertEqual(explicit_keys, ['use_remoteexec'])

        # Do not include sensitive information.
        with unittest.mock.patch('getpass.getuser', return_value='bob'):
            gn_args, explicit_keys = ninjalog_uploader.ParseGNArgs(
                json.dumps([
                    {
                        'current': {
                            'value': 'xyz',
                            'file': '//path/to/args.gn',
                        },
                        'default': {
                            'value': ''
                        },
                        'name': 'google_api_key'
                    },
                    {
                        'current': {
                            'value': '/home/bob/bobo',
                            'file': '//path/to/args.gn',
                        },
                        'default': {
                            'value': ''
                        },
                        'name': 'path_with_homedir'
                    },
                ]))
            self.assertEqual(
                gn_args, {
                    'google_api_key': '<omitted>',
                    'path_with_homedir': '/home/$USER/bobo',
                })

    def test_get_ninjalog(self):
        # No args => default to cwd.
        self.assertEqual(ninjalog_uploader.GetNinjalog(['ninja']),
                         './.ninja_log')

        # Specified by -C case.
        self.assertEqual(
            ninjalog_uploader.GetNinjalog(['ninja', '-C', 'out/Release']),
            'out/Release/.ninja_log')
        self.assertEqual(
            ninjalog_uploader.GetNinjalog(['ninja', '-Cout/Release']),
            'out/Release/.ninja_log')

        # Invalid -C flag case.
        self.assertEqual(ninjalog_uploader.GetNinjalog(['ninja', '-C']),
                         './.ninja_log')

        # Multiple target directories => use the last directory.
        self.assertEqual(
            ninjalog_uploader.GetNinjalog(
                ['ninja', '-C', 'out/Release', '-C', 'out/Debug']),
            'out/Debug/.ninja_log')

    def test_get_build_target_from_command_line(self):
        self.assertEqual(
            ninjalog_uploader.GetBuildTargetFromCommandLine(
                ['python3', 'ninja.py', 'chrome']), ['chrome'])

        self.assertEqual(
            ninjalog_uploader.GetBuildTargetFromCommandLine(
                ['python3', 'ninja.py']), [])

        self.assertEqual(
            ninjalog_uploader.GetBuildTargetFromCommandLine(
                ['python3', 'ninja.py', '-j', '1000', 'chrome']), ['chrome'])

        self.assertEqual(
            ninjalog_uploader.GetBuildTargetFromCommandLine(
                ['python3', 'ninja.py', 'chrome', '-j', '1000']), ['chrome'])

        self.assertEqual(
            ninjalog_uploader.GetBuildTargetFromCommandLine(
                ['python3', 'ninja.py', '-C', 'chrome']), [])

        self.assertEqual(
            ninjalog_uploader.GetBuildTargetFromCommandLine(
                ['python3', 'ninja.py', '-Cout/Release', 'chrome']), ['chrome'])

        self.assertEqual(
            ninjalog_uploader.GetBuildTargetFromCommandLine(
                ['python3', 'ninja.py', '-C', 'out/Release', 'chrome', 'all']),
            ['chrome', 'all'])

    @unittest.skipIf(sys.platform == 'win32', 'posix path test')
    def test_get_build_target_from_command_line_filter_posix(self):
        self.assertEqual(
            ninjalog_uploader.GetBuildTargetFromCommandLine([
                'python3', 'ninja.py', '-C', 'out/Release', 'chrome', 'all',
                '/path/to/foo', '-p'
            ]), ['chrome', 'all'])

    @unittest.skipUnless(sys.platform == 'win32', 'Windows path test')
    def test_get_build_target_from_command_line_filter_win(self):
        self.assertEqual(
            ninjalog_uploader.GetBuildTargetFromCommandLine([
                'python3', 'ninja.py', '-C', 'out/Release', 'chrome', 'all',
                'C:\\path\\to\\foo', '-p'
            ]), ['chrome', 'all'])

    def test_get_j_flag(self):
        self.assertEqual(ninjalog_uploader.GetJflag(['ninja']), None)

        self.assertEqual(ninjalog_uploader.GetJflag(['ninja', '-j', '1000']),
                         1000)

        self.assertEqual(ninjalog_uploader.GetJflag(['ninja', '-j', '1000a']),
                         None)

        self.assertEqual(ninjalog_uploader.GetJflag(['ninja', '-j', 'a']), None)

        self.assertEqual(ninjalog_uploader.GetJflag(['ninja', '-j1000']), 1000)

        self.assertEqual(ninjalog_uploader.GetJflag(['ninja', '-ja']), None)

        self.assertEqual(ninjalog_uploader.GetJflag(['ninja', '-j']), None)

    def test_get_gce_metadata(self):
        with unittest.mock.patch('urllib.request.urlopen') as urlopen_mock:
            urlopen_mock.side_effect = urllib.error.HTTPError(
                'http://test/not-found', http.HTTPStatus.NOT_FOUND, 'not found',
                None, None)
            self.assertEqual(ninjalog_uploader.GetGCEMetadata(), {})

        with unittest.mock.patch(
                'ninjalog_uploader._getGCEInfo') as getGCEInfo_mock:
            getGCEInfo_mock.return_value = {
                "instance": {
                    "machineType":
                    "projects/12345/machineTypes/n2d-standard-128",
                },
                "project": {
                    "projectId": "cloudtop-test"
                }
            }
            self.assertEqual(ninjalog_uploader.GetGCEMetadata(), {
                'gce_machine_type': 'n2d-standard-128',
                'is_cloudtop': True,
            })

        with unittest.mock.patch(
                'ninjalog_uploader._getGCEInfo') as getGCEInfo_mock:
            getGCEInfo_mock.return_value = {
                "instance": {
                    "machineType":
                    "projects/12345/machineTypes/n2d-standard-128",
                },
                "project": {
                    "projectId": "gce-project"
                }
            }
            self.assertEqual(ninjalog_uploader.GetGCEMetadata(), {
                'gce_machine_type': 'n2d-standard-128',
            })

if __name__ == '__main__':
    unittest.main()
