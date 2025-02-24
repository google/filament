# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import post_process
from recipe_engine import recipe_api

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
    'bot_update',
    'gclient',
    'recipe_engine/assertions',
    'recipe_engine/buildbucket',
    'recipe_engine/path',
    'recipe_engine/properties',
]

PROPERTIES = {
    'expected_checkout_dir': recipe_api.Property(),
    'expected_source_root_name': recipe_api.Property(),
    'expected_patch_root_name': recipe_api.Property(default=None),
}


# TODO: crbug.com/339472834 - Once all downstream uses of presentation and
# json.output have been removed, this test can be updated to not reference them
# and the decorator can be removed
@recipe_api.ignore_warnings('^depot_tools/BOT_UPDATE_CUSTOM_RESULT_ATTRIBUTES$')
def RunSteps(api, expected_checkout_dir, expected_source_root_name,
             expected_patch_root_name):
  api.gclient.set_config('depot_tools')
  result = api.bot_update.ensure_checkout()

  api.assertions.assertEqual(result.checkout_dir, expected_checkout_dir)

  api.assertions.assertEqual(result.source_root.name, expected_source_root_name)
  api.assertions.assertEqual(result.source_root.path,
                             expected_checkout_dir / expected_source_root_name)

  if expected_patch_root_name is not None:
    api.assertions.assertEqual(result.patch_root.name, expected_patch_root_name)
    api.assertions.assertEqual(result.patch_root.path,
                               expected_checkout_dir / expected_patch_root_name)
  else:
    api.assertions.assertIsNone(result.patch_root)


def GenTests(api):
  yield api.test(
      'basic',
      api.properties(
          expected_checkout_dir=api.path.start_dir,
          expected_source_root_name='depot_tools',
      ),
      api.expect_status('SUCCESS'),
      api.post_process(post_process.DropExpectation),
  )

  yield api.test(
      'patch',
      api.buildbucket.try_build(),
      api.properties(
          expected_checkout_dir=api.path.start_dir,
          expected_source_root_name='depot_tools',
          expected_patch_root_name='depot_tools',
      ),
      api.expect_status('SUCCESS'),
      api.post_process(post_process.DropExpectation),
  )
