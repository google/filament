# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import post_process

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
    'git',
    'recipe_engine/assertions',
    'recipe_engine/path',
    'recipe_engine/properties',
]


def RunSteps(api):
  # Set the checkout_dir because the `git` module implicitly uses this.
  api.path.checkout_dir = api.path.cache_dir / 'builder'

  numbers = api.git.number(
      commitrefs=api.properties.get('commitrefs'),
      test_values=api.properties.get('test_values'),
  )
  expected_numbers = api.properties['expected_numbers']
  api.assertions.assertSequenceEqual(numbers, expected_numbers)


def GenTests(api):
  yield api.test(
      'basic',
      api.properties(expected_numbers=['3000']),
      api.post_check(post_process.StatusSuccess),
      api.post_process(post_process.DropExpectation),
  )

  yield api.test(
      'commitrefs',
      api.properties(
          commitrefs=['rev-1', 'rev-2'],
          expected_numbers=['3000', '3001'],
      ),
      api.post_check(post_process.StatusSuccess),
      api.post_process(post_process.DropExpectation),
  )

  yield api.test(
      'basic-with-test-values',
      api.properties(
          test_values=[42],
          expected_numbers=['42'],
      ),
      api.post_check(post_process.StatusSuccess),
      api.post_process(post_process.DropExpectation),
  )

  yield api.test(
      'commitrefs-with-test-values',
      api.properties(
          test_values=[42, 13],
          commitrefs=['rev-1', 'rev-2'],
          expected_numbers=['42', '13'],
      ),
      api.post_check(post_process.StatusSuccess),
      api.post_process(post_process.DropExpectation),
  )
