# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import post_process
from recipe_engine.recipe_api import Property

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
  'gsutil',
  'recipe_engine/platform',
  'recipe_engine/properties',
]

PROPERTIES = {
  'boto_configs': Property(default={}, kind=dict),
}


def RunSteps(api, boto_configs):
  with api.gsutil.configure_gsutil(**boto_configs):
    api.gsutil(['cp', 'gs://some/gs/path', '/some/local/path'])


def GenTests(api):
  yield api.test(
      'no_args',
      api.platform('linux', 64),
      api.post_process(post_process.DropExpectation),
  )

  yield api.test(
      'no_env',
      api.properties(boto_configs={'some_config': 'some_val'}),
      api.post_process(post_process.DropExpectation),
  )

  yield api.test(
      'mac',
      api.platform('mac', 64),
      api.post_process(post_process.DropExpectation),
  )

  yield api.test(
      'with_boto_config',
      api.properties(boto_configs={'some_config': 'some_val'}),
      api.properties.environ(
          BOTO_CONFIG='/some/boto/config'
      ),
      api.post_check(lambda check, steps: \
          check(steps['gsutil cp'].env['BOTO_CONFIG'] is None)),
      api.post_check(lambda check, steps: \
          check('/some/boto/config' in steps['gsutil cp'].env['BOTO_PATH'])),
      api.post_process(post_process.DropExpectation),
  )

  yield api.test(
      'with_boto_path',
      api.properties(boto_configs={'some_config': 'some_val'}),
      api.properties.environ(
          BOTO_PATH='/some/boto/path'
      ),
      api.post_check(lambda check, steps: \
          check(steps['gsutil cp'].env['BOTO_CONFIG'] is None)),
      api.post_check(lambda check, steps: \
          check('/some/boto/path' in steps['gsutil cp'].env['BOTO_PATH'])),
      api.post_process(post_process.DropExpectation),
  )
