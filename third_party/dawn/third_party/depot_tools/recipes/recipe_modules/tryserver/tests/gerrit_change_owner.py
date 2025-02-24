# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine.post_process import (DropExpectation, StatusSuccess,
                                        StepCommandContains)

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
    'gerrit',
    'tryserver',
    'recipe_engine/buildbucket',
    'recipe_engine/step',
]


def RunSteps(api):
  api.step(name='print owner',
           cmd=['echo', str(api.tryserver.gerrit_change_owner)])


def GenTests(api):
  yield api.test(
      'basic',
      api.buildbucket.try_build(),
      api.post_process(StepCommandContains, 'print owner',
                       ['echo', "{'name': 'John Doe'}"]),
      api.post_process(StatusSuccess),
      api.post_process(DropExpectation),
  )
