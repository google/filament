# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import json
import os
import sys

# This is the list of tests to run. It is a dictionary with the following
# fields:
#
# name (required): The name of the step, to show on the buildbot status page.
# path (required): The path to the executable which runs the tests.
# additional_args (optional): An array of optional arguments.
# uses_sandbox_env (optional): True if CHROME_DEVEL_SANDBOX must be in
#   environment.
# disabled (optional): List of platforms the test is disabled on. May contain
#   'win', 'mac', 'linux', or 'android'.
# outputs_presentation_json (optional): If True, pass in --presentation-json
#   argument to the test executable to allow it to update the buildbot status
#   page. More details here:
# github.com/luci/recipes-py/blob/master/recipe_modules/generator_script/api.py
_DASHBOARD_TESTS = [
    {
        'name': 'Dashboard Python Tests',
        'path': 'dashboard/bin/run_py_tests',
        'additional_args': ['--no-install-hooks'],
        'disabled': ['android', 'win', 'mac'],
    },
]

_PERF_ISSUE_SERVICE_TESTS = [
    {
        'name': 'Perf Issue Service Python Tests',
        'path': 'perf_issue_service/tests/bin/run_py_tests',
        'disabled': ['android', 'win', 'mac'],
    }
]

_CATAPULT_TESTS = [
    {
        'name': 'Build Python Tests',
        'path': 'catapult_build/bin/run_py_tests',
        'disabled': ['android'],
    },
    {
        'name': 'Common Tests',
        'path': 'common/bin/run_tests',
    },
    {
        'name': 'Dependency Manager Tests',
        'path': 'dependency_manager/bin/run_tests',
    },
    {
        'name': 'Devil Device Tests',
        'path': 'devil/bin/run_py_devicetests',
        'disabled': ['win', 'mac', 'linux']
    },
    {
        'name': 'Devil Python Tests',
        'path': 'devil/bin/run_py3_tests',
        'disabled': ['mac', 'win'],
    },
    {
        'name': 'Native Heap Symbolizer Tests',
        'path': 'tracing/bin/run_symbolizer_tests',
        'disabled': ['android'],
    },
    {
        'name': 'Py-vulcanize Tests',
        'path': 'common/py_vulcanize/bin/run_py_tests',
        'additional_args': ['--no-install-hooks'],
        'disabled': ['android'],
    },
    {
        'name': 'Systrace Tests',
        'path': 'systrace/bin/run_tests',
    },
    {
        'name': 'Snap-it Tests',
        'path': 'telemetry/bin/run_snap_it_unittest',
        'additional_args': ['--browser=reference',],
        'uses_sandbox_env': True,
        'disabled': ['android'],
        'python_versions': [3],
    },
    {
        'name': 'Telemetry Tests with Stable Browser (Desktop)',
        'path': 'catapult_build/fetch_telemetry_deps_and_run_tests',
        'additional_args': [
            '--browser=reference',
            '--start-xvfb',
            '-v',
        ],
        'uses_sandbox_env': True,
        'disabled': ['android'],
        'python_versions': [3],
    },
    {
        'name': 'Telemetry Tests with Stable Browser (Android)',
        'path': 'catapult_build/fetch_telemetry_deps_and_run_tests',
        'additional_args': [
            '--browser=reference',
            '--device=android',
            '--jobs=1',
            '-v',
        ],
        'uses_sandbox_env': True,
        'disabled': ['win', 'mac', 'linux'],
        'python_versions': [3],
    },
    {
        'name': 'Telemetry Integration Tests with Stable Browser',
        'path': 'telemetry/bin/run_browser_tests',
        'additional_args': [
            'BrowserTest',
            '--browser=reference',
            '-v',
        ],
        'uses_sandbox_env': True,
        'disabled': ['android', 'linux'],  # TODO(nedn): enable this on linux
        'python_versions': [3],
    },
    {
        'name': 'Tracing Dev Server Tests',
        'path': 'tracing/bin/run_dev_server_tests',
        'additional_args': [
            '--no-install-hooks',
            '--no-use-local-chrome',
            '--channel=stable',
            '--timeout-sec=900',
        ],
        'outputs_presentation_json': True,
        'disabled': ['android'],
    },
    {
        'name': 'Tracing Dev Server Tests Canary',
        'path': 'tracing/bin/run_dev_server_tests',
        'additional_args': [
            '--no-install-hooks',
            '--no-use-local-chrome',
            '--channel=canary',
            '--timeout-sec=900',
        ],
        'outputs_presentation_json': True,
        'disabled': ['android'],
    },
    {
        'name': 'Tracing D8 Tests',
        'path': 'tracing/bin/run_vinn_tests',
        'disabled': ['android'],
    },
    {
        'name': 'Tracing Python Tests',
        'path': 'tracing/bin/run_py_tests',
        'additional_args': ['--no-install-hooks'],
        'disabled': ['android'],
    },
    {
        'name': 'Typ unittest',
        'path': 'third_party/typ/run',
        'additional_args': ['tests'],
        'disabled': ['android', 'win'
                    ],  # TODO(crbug.com/851498): enable typ unittests on Win
    },
    {
        'name': 'Vinn Tests',
        'path': 'third_party/vinn/bin/run_tests',
        'disabled': ['android'],
    },
    {
        'name': 'NetLog Viewer Dev Server Tests',
        'path': 'netlog_viewer/bin/run_dev_server_tests',
        'additional_args': [
            '--no-install-hooks',
            '--no-use-local-chrome',
        ],
        'disabled': ['android', 'win', 'mac', 'linux'],
    },
]

_STALE_FILE_TYPES = ['.pyc', '.pseudo_lock']


def main(args=None):
  """Send list of test to run to recipes generator_script.

  See documentation at:
  github.com/luci/recipes-py/blob/master/recipe_modules/generator_script/api.py
  """
  parser = argparse.ArgumentParser(description='Run catapult tests.')
  parser.add_argument('--api-path-checkout', help='Path to catapult checkout')
  parser.add_argument(
      '--app-engine-sdk-pythonpath',
      help='PYTHONPATH to include app engine SDK path')
  parser.add_argument('--platform', help='Platform name (linux, mac, or win)')
  parser.add_argument('--platform_arch', help='Platform arch (intel or arm)')
  parser.add_argument('--output-json', help='Output for buildbot status page')
  parser.add_argument(
      '--run_android_tests', default=True, help='Run Android tests')
  parser.add_argument(
      '--dashboard_only',
      default=False,
      help='Run only the Dashboard and Pinpoint tests',
      action='store_true')
  parser.add_argument(
      '--perf_issue_service_only',
      default=False,
      help='Run only the Perf Issue Service tests',
      action='store_true')
  args = parser.parse_args(args)

  dashboard_protos_folder = os.path.join(args.api_path_checkout, 'dashboard',
                                       'dashboard', 'protobuf')
  dashboard_proto_files = [
      os.path.join(dashboard_protos_folder, p)
      for p in ['sheriff.proto', 'sheriff_config.proto']
  ]

  dashboard_protos_path = os.path.join(args.api_path_checkout, 'dashboard')

  sheriff_proto_output_path = os.path.join(args.api_path_checkout, 'dashboard',
                                           'dashboard', 'sheriff_config')
  dashboard_proto_output_path = os.path.join(args.api_path_checkout,
                                             'dashboard')

  tracing_protos_path = os.path.join(args.api_path_checkout, 'tracing',
                                     'tracing', 'proto')
  tracing_proto_output_path = tracing_protos_path
  tracing_proto_files = [os.path.join(tracing_protos_path, 'histogram.proto')]

  protoc_path = 'protoc'

  steps = [
      {
          # Always remove stale files first. Not listed as a test above
          # because it is a step and not a test, and must be first.
          'name':
              'Remove Stale files',
          'cmd': [
              'python3',
              os.path.join(args.api_path_checkout, 'catapult_build',
                           'remove_stale_files.py'),
              args.api_path_checkout,
              ','.join(_STALE_FILE_TYPES),
          ]
      },
      # Since we might not have access to 'make', let's run the protobuf
      # compiler directly. We want to run the proto compiler to generate the
      # right data in the right places.
      {
          'name':
              'Generate Sheriff Config protocol buffers',
          'cmd': [
              protoc_path,
              '--proto_path',
              dashboard_protos_path,
              '--python_out',
              sheriff_proto_output_path,
          ] + dashboard_proto_files,
      },
      {
          'name':
              'Generate Dashboard protocol buffers',
          'cmd': [
              protoc_path,
              '--proto_path',
              dashboard_protos_path,
              '--python_out',
              dashboard_proto_output_path,
          ] + dashboard_proto_files,
      },
      {
          'name':
              'Generate Tracing protocol buffers',
          'cmd': [
              protoc_path,
              '--proto_path',
              tracing_protos_path,
              '--python_out',
              tracing_proto_output_path,
          ] + tracing_proto_files,
      },
  ]
  if args.platform == 'android' and args.run_android_tests:
    # On Android, we need to prepare the devices a bit before using them in
    # tests. These steps are not listed as tests above because they aren't
    # tests and because they must precede all tests.
    steps.extend([
        {
            'name':
                'Android: Recover Devices',
            'cmd': [
                'vpython3',
                os.path.join(args.api_path_checkout, 'devil', 'devil',
                             'android', 'tools', 'device_recovery.py')
            ],
        },
        {
            'name':
                'Android: Provision Devices',
            'cmd': [
                'vpython3',
                os.path.join(args.api_path_checkout, 'devil', 'devil',
                             'android', 'tools', 'provision_devices.py')
            ],
        },
        {
            'name':
                'Android: Device Status',
            'cmd': [
                'vpython3',
                os.path.join(args.api_path_checkout, 'devil', 'devil',
                             'android', 'tools', 'device_status.py')
            ],
        },
    ])

  tests = None
  if args.dashboard_only:
    tests = _DASHBOARD_TESTS
  elif args.perf_issue_service_only:
    tests = _PERF_ISSUE_SERVICE_TESTS
  else:
    tests = _CATAPULT_TESTS

  for test in tests:
    if args.platform == 'android' and not args.run_android_tests:
      # Remove all the steps for the Android configuration if we're asked to not
      # run the Android tests.
      steps = []
      break

    if args.platform in test.get('disabled', []):
      continue

    test_path = test['path']

    step = {'name': test['name'], 'env': {}}

    vpython_executable = "vpython3"

    if sys.platform == 'win32':
      vpython_executable += '.bat'

    # Always add the appengine SDK path.
    step['env']['PYTHONPATH'] = args.app_engine_sdk_pythonpath

    step['cmd'] = [
        vpython_executable,
        os.path.join(args.api_path_checkout, test_path)
    ]
    if step['name'] == 'Systrace Tests':
      step['cmd'] += ['--device=' + args.platform]
    if test.get('additional_args'):
      step['cmd'] += test['additional_args']
    if test.get('uses_sandbox_env'):
      step['env']['CHROME_DEVEL_SANDBOX'] = '/opt/chromium/chrome_sandbox'
    if test.get('outputs_presentation_json'):
      step['outputs_presentation_json'] = True
    step['always_run'] = True
    steps.append(step)

  with open(args.output_json, 'w') as outfile:
    json.dump(steps, outfile)


if __name__ == '__main__':
  main(sys.argv[1:])
