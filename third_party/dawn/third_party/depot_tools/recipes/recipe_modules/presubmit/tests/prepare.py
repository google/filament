# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import post_process
from recipe_engine import recipe_api


PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
    'gclient',
    'presubmit',
    'recipe_engine/buildbucket',
    'recipe_engine/context',
    'recipe_engine/path',
    'recipe_engine/properties',
    'recipe_engine/raw_io',
    'recipe_engine/runtime',
]


PROPERTIES = {
  'patch_project': recipe_api.Property(None),
  'patch_repository_url': recipe_api.Property(None),
}


def RunSteps(api, patch_project, patch_repository_url):
  api.gclient.set_config('infra')
  with api.context(cwd=api.path.cache_dir / 'builder'):
    bot_update_step = api.presubmit.prepare()


def GenTests(api):
  yield (api.test('basic') + api.runtime(is_experimental=False) +
         api.buildbucket.try_build(project='infra') +
         api.post_process(post_process.StatusSuccess) +
         api.post_process(post_process.DropExpectation))

  yield (api.test('runhooks') + api.runtime(is_experimental=False) +
         api.buildbucket.try_build(project='infra') +
         api.presubmit(runhooks=True) +
         api.post_process(post_process.MustRun, 'gclient runhooks') +
         api.post_process(post_process.StatusSuccess) +
         api.post_process(post_process.DropExpectation))

  yield api.test(
      'failed_commit',
      api.runtime(is_experimental=False),
      api.buildbucket.try_build(project='infra'),
      api.step_data(
          'commit-git-patch',
          retcode=1,
          stdout=api.raw_io.output_text(
              'nothing to commit, working tree clean'),
      ),
      api.expect_status('FAILURE'),
      api.post_check(
          post_process.SummaryMarkdownRE,
          'Was an identical diff already submitted elsewhere?',
      ),
      api.post_process(post_process.DropExpectation),
  )
