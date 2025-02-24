# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
  'presubmit',
  'recipe_engine/json'
]


def RunSteps(api):
  api.presubmit()


def GenTests(api):
  yield (
    api.test('basic') +
    api.step_data('presubmit', api.json.output({}))
  )
