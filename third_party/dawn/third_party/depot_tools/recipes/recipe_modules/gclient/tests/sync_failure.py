# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import post_process

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
  'recipe_engine/path',

  'gclient',
]

def RunSteps(api):
  src_cfg = api.gclient.make_config(CACHE_DIR=api.path.cache_dir / 'git')
  api.gclient.sync(src_cfg)

def GenTests(api):
  yield api.test(
      'no-json',
      api.override_step_data('gclient sync', retcode=1),
      # Should not fail with uncaught exception
      api.post_process(post_process.SummaryMarkdownRE,
                       r'^(?!Uncaught Exception)'),
      api.post_process(post_process.DropExpectation),
      status="INFRA_FAILURE")
