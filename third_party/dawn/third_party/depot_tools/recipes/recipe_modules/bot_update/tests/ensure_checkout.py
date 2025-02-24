# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import post_process

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
  'bot_update',
  'gclient',
  'recipe_engine/json',
]


def RunSteps(api):
  api.gclient.set_config('depot_tools')
  api.bot_update.ensure_checkout()
  api.bot_update.ensure_checkout(no_fetch_tags=True, enforce_fetch=True)
  api.bot_update.ensure_checkout(step_tags={'tag': 'value'})


def GenTests(api):
  yield (
      api.test('basic') +
      api.post_process(post_process.StatusSuccess) +
      api.post_process(post_process.DropExpectation)
  )

  yield (api.test('failure', status="INFRA_FAILURE") + api.override_step_data(
      'bot_update', api.json.output({'did_run': True}), retcode=1) +
         api.post_process(post_process.StatusAnyFailure) +
         api.post_process(post_process.DropExpectation))

  yield (api.test('cancelled', status="INFRA_FAILURE") + api.override_step_data(
      'bot_update', api.json.output({'did_run': True}), cancel=True) +
         api.post_process(post_process.StatusAnyFailure) +
         api.post_process(post_process.DropExpectation))
