# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import post_process

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
    'gerrit',
    'recipe_engine/buildbucket',
    'recipe_engine/properties',
    'tryserver',
]


def RunSteps(api):
  api.tryserver.gerrit_change_fetch_ref


def GenTests(api):
  yield (api.test('timeout', status="INFRA_FAILURE") +
         api.buildbucket.try_build(
             'chromium',
             'linux',
             git_repo='https://chromium.googlesource.com/chromium/src',
             change_number=91827,
             patch_set=1) +
         api.tryserver.gerrit_change_target_ref('refs/heads/main') +
         api.override_step_data('gerrit fetch current CL info',
                                times_out_after=1200) +
         api.post_process(post_process.StatusException) +
         api.post_process(post_process.DropExpectation))
