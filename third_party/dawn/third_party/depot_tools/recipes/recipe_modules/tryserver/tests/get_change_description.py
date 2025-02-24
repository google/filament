# Copyright 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine.post_process import (DropExpectation, StatusSuccess, MustRun,
                                        DoesNotRun)

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
    'tryserver',
    'recipe_engine/buildbucket',
    'recipe_engine/step',
]


def RunSteps(api):
  api.tryserver.get_change_description()
  api.tryserver.get_change_description()


def GenTests(api):
  yield api.test(
      'basic',
      api.buildbucket.try_build(),
      api.post_process(MustRun, 'gerrit changes'),
      api.post_process(DoesNotRun, 'gerrit changes (2)'),
      api.post_process(StatusSuccess),
      api.post_process(DropExpectation),
  )
