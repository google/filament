# Copyright 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import post_process

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
    'tryserver',
    'recipe_engine/assertions',
    'recipe_engine/path',
    'recipe_engine/platform',
    'recipe_engine/properties',
]


def RunSteps(api):
  files = api.tryserver.get_files_affected_by_patch(
      api.properties['patch_root'],
      report_files_via_property=api.properties.get('report_files_via_property'))
  api.assertions.assertCountEqual(files, api.properties['expected_files'])


def GenTests(api):
  def no_properties_set(check, steps, step_name):
    check(not steps[step_name].output_properties)

  yield api.test(
      'basic',
      api.properties(
          patch_root='',
          expected_files=['foo.cc'],
      ),
      api.post_check(no_properties_set, 'git diff to analyze patch'),
      api.post_check(post_process.StatusSuccess),
      api.post_process(post_process.DropExpectation),
  )

  yield api.test(
      'patch_root',
      api.properties(
          patch_root='test/patch/root',
          expected_files=['test/patch/root/foo.cc'],
      ),
      api.post_check(post_process.StatusSuccess),
      api.post_process(post_process.DropExpectation),
  )

  yield api.test(
      'test-data',
      api.tryserver.get_files_affected_by_patch(['foo/bar.cc', 'baz/shaz.cc']),
      api.properties(
          patch_root='test/patch/root',
          expected_files=[
              'test/patch/root/foo/bar.cc',
              'test/patch/root/baz/shaz.cc',
          ],
      ),
      api.post_check(post_process.StatusSuccess),
      api.post_process(post_process.DropExpectation),
  )

  yield api.test(
      'report-files-via-property',
      api.tryserver.get_files_affected_by_patch(['foo/bar.cc', 'baz/shaz.cc']),
      api.properties(
          patch_root='test/patch/root',
          report_files_via_property='affected-files',
          expected_files=[
              'test/patch/root/foo/bar.cc',
              'test/patch/root/baz/shaz.cc',
          ],
      ),
      api.post_check(post_process.StatusSuccess),
      api.post_process(post_process.DropExpectation),
  )

  yield api.test(
      'windows',
      api.tryserver.get_files_affected_by_patch(['foo/bar.cc', 'baz/shaz.cc']),
      api.platform('win', 32),
      api.properties(
          patch_root='test\\patch\\root',
          report_files_via_property='affected-files',
          expected_files=[
              'test/patch/root/foo/bar.cc',
              'test/patch/root/baz/shaz.cc',
          ],
      ),
      api.post_check(post_process.StatusSuccess),
      api.post_process(post_process.DropExpectation),
  )

  yield api.test(
      'file-with-spaces',
      api.tryserver.get_files_affected_by_patch(['file with spaces.txt']),
      api.properties(
          patch_root='',
          expected_files=['file with spaces.txt'],
      ),
      api.post_check(post_process.StatusSuccess),
      api.post_process(post_process.DropExpectation),
  )