# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import post_process

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
    'gerrit',
    'recipe_engine/buildbucket',
    'recipe_engine/properties',
    'recipe_engine/step',
    'tryserver',
]


def RunSteps(api):
  api.step(name=api.tryserver.gerrit_change_target_ref or 'None!', cmd=None)


def GenTests(api):
  yield api.test(
      'works',
      api.buildbucket.try_build(
          'chromium',
          'linux',
          git_repo='https://chromium.googlesource.com/chromium/src',
          change_number=91827,
          patch_set=1),
      api.tryserver.gerrit_change_target_ref('refs/branch-heads/custom'),
      api.post_check(
          lambda check, steps: check('refs/branch-heads/custom' in steps)),
      api.post_process(post_process.DropExpectation))
