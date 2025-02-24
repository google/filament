# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import os
import sys

_CATAPULT_PATH = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..'))

# Directories in catapult/third_party required by dashboard.
THIRD_PARTY_LIBRARIES = [
    'cloudstorage',
    'depot_tools',
    'flot',
    'jquery',
    'polymer',
    'polymer-svg-template',
    'redux/redux.min.js',
]

# Files and directories in catapult/dashboard.
DASHBOARD_FILES = [
    'api.yaml',
    'app.yaml',
    'cron.yaml',
    'dashboard',
    'dispatch.yaml',
    'index.yaml',
    'pinpoint.yaml',
    'queue.yaml',
    'requirements.txt',
    'scripts.yaml',
    'upload-processing.yaml',
    'upload.yaml',
]

TRACING_PATHS = [
    'tracing/tracing',
    'tracing/tracing_build',
    'tracing/third_party/gl-matrix/dist/gl-matrix-min.js',
    'tracing/third_party/mannwhitneyu',
]


def PathsForDeployment():
  """Returns a list of paths to things required for deployment.

  This includes both Python libraries that are required, and also
  other files, such as config files.

  This list is used when building a temporary deployment directory;
  each of the items in this list will have a corresponding file or
  directory with the same basename in the deployment directory.
  """
  paths = []
  paths.extend(_CatapultThirdPartyLibraryPaths())
  for name in DASHBOARD_FILES:
    paths.append(os.path.join(_CATAPULT_PATH, 'dashboard', name))
  paths.append(os.path.join(_CATAPULT_PATH, 'tracing', 'tracing_project.py'))
  paths.append(os.path.join(_CATAPULT_PATH, 'common', 'py_utils', 'py_utils'))
  # Required by py_utils
  paths.append(os.path.join(_CATAPULT_PATH, 'devil', 'devil'))
  paths.extend(_TracingPaths())
  return paths


def PathsForTesting():
  """Returns a list of Python library paths required for dashboard tests."""
  return _AllSdkThirdPartyLibraryPaths() + _CatapultThirdPartyLibraryPaths() + [
      os.path.join(_CATAPULT_PATH, 'dashboard'),
      os.path.join(_CATAPULT_PATH, 'tracing'),
      os.path.join(_CATAPULT_PATH, 'common', 'py_utils', 'py_utils'),

      # Required by py_utils
      os.path.join(_CATAPULT_PATH, 'devil', 'devil'),

      # Isolate the sheriff_config package, since it's deployed independently.
      os.path.join(_CATAPULT_PATH, 'dashboard', 'dashboard', 'sheriff_config'),
  ]


def _AllSdkThirdPartyLibraryPaths():
  """Returns a list of all third party library paths from the SDK.

  The AppEngine documentation directs us to add App Engine libraries from the
  SDK to our Python path for local unit tests.
    https://cloud.google.com/appengine/docs/python/tools/localunittesting
  """
  paths = []
  for sdk_bin_path in os.environ['PATH'].split(os.pathsep):
    if 'google-cloud-sdk' not in sdk_bin_path:
      continue

    if not os.path.isdir(sdk_bin_path):
      sdk_bin_path = os.path.dirname(sdk_bin_path)

    appengine_path = os.path.join(sdk_bin_path, 'platform', 'google_appengine')
    paths.append(appengine_path)
    sys.path.insert(0, appengine_path)

  try:
    # pylint: disable=import-outside-toplevel
    import google.appengine  # pylint: disable=unused-import
  except ImportError:
    # TODO: Put the Cloud SDK in the path with the binary dependency manager.
    # https://github.com/catapult-project/catapult/issues/2135
    print('This script requires the Google Cloud SDK to be in PYTHONPATH.')
    print('See https://chromium.googlesource.com/catapult/'
          '+/HEAD/dashboard/README.md')
    sys.exit(1)
  return paths


def _CatapultThirdPartyLibraryPaths():
  """Returns a list of required third-party libraries in catapult."""
  paths = []
  paths.append(
      os.path.join(_CATAPULT_PATH, 'common', 'node_runner', 'node_runner',
                   'node_modules', '@chopsui'))
  for library in THIRD_PARTY_LIBRARIES:
    paths.append(os.path.join(_CATAPULT_PATH, 'third_party', library))
  return paths


def _TracingPaths():
  """Returns a list of paths that may be imported from tracing."""
  # TODO(sullivan): This should either pull from tracing_project or be generated
  # via gypi. See https://github.com/catapult-project/catapult/issues/3048.
  paths = []
  for path in TRACING_PATHS:
    paths.append(os.path.join(_CATAPULT_PATH, os.path.normpath(path)))
  return paths
