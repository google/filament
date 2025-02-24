# Copyright 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import post_process

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
    'tryserver',
    'recipe_engine/assertions',
    'recipe_engine/buildbucket',
    'recipe_engine/path',
    'recipe_engine/platform',
    'recipe_engine/properties',
]


def RunSteps(api):
  footers = api.tryserver.get_footers()
  api.assertions.assertCountEqual(footers, api.properties['expected_footers'])


def GenTests(api):
  yield api.test(
      'no-footers',
      api.buildbucket.try_build(
          'chromium',
          'linux',
      ),
      api.properties(expected_footers={}),
      api.tryserver.get_footers({}),
      api.post_check(post_process.StatusSuccess),
      api.post_process(post_process.DropExpectation),
  )

  yield api.test(
      'single-footer',
      api.buildbucket.try_build(
          'chromium',
          'linux',
      ),
      api.properties(expected_footers={'Some-Footer': ['True']}),
      api.tryserver.get_footers({'Some-Footer': ['True']}),
      api.post_check(post_process.StatusSuccess),
      api.post_process(post_process.DropExpectation),
  )

  yield api.test(
      'multiple-footers',
      api.buildbucket.try_build(
          'chromium',
          'linux',
      ),
      api.properties(expected_footers={
          'Some-Footer': ['True'],
          'Another-Footer': ['Foo', 'Bar']
      }),
      api.tryserver.get_footers({
          'Some-Footer': ['True'],
          'Another-Footer': ['Foo', 'Bar']
      }),
      api.post_check(post_process.StatusSuccess),
      api.post_process(post_process.DropExpectation),
  )
