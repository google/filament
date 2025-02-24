# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
  'depot_tools',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/runtime',
  'recipe_engine/step',
]


def RunSteps(api):
  api.step('root', ['ls', api.depot_tools.root])

  api.step(
      'download_from_google_storage',
      ['ls', api.depot_tools.download_from_google_storage_path])

  api.step(
      'upload_to_google_storage',
      ['ls', api.depot_tools.upload_to_google_storage_path])

  api.step('roll_downstream_gcs_deps_path',
           ['ls', api.depot_tools.roll_downstream_gcs_deps_path])

  api.step('cros', ['ls', api.depot_tools.cros_path])

  api.step(
      'gn_py_path', ['ls', api.depot_tools.gn_py_path])

  api.step(
      'gsutil_py_path', ['ls', api.depot_tools.gsutil_py_path])

  api.step(
      'presubmit_support_py_path',
      ['ls', api.depot_tools.presubmit_support_py_path])

  with api.depot_tools.on_path():
    api.step('on_path', ['echo', '$PATH'])


def GenTests(api):
  yield api.test('basic')

  yield api.test('win') + api.platform('win', 32)
