# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import post_process
from recipe_engine import recipe_api


PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
  'recipe_engine/buildbucket',
  'recipe_engine/path',
  'recipe_engine/properties',

  'gclient',
]


def RunSteps(api):
  src_cfg = api.gclient.make_config(CACHE_DIR=api.path.cache_dir / 'git')
  soln = src_cfg.solutions.add()
  soln.name = 'src'
  soln.url = 'https://chromium.googlesource.com/chromium/src.git'
  src_cfg.repo_path_map.update({
      'https://chromium.googlesource.com/src': ('src', 'HEAD'),
      'https://chromium.googlesource.com/v8/v8': ('src/v8', 'HEAD'),
      # non-canonical URL
      'https://webrtc.googlesource.com/src.git': (
          'src/third_party/webrtc', 'HEAD'),
  })
  assert api.gclient.get_repo_path(
      'https://chromium.googlesource.com/chromium/src.git',
      gclient_config=src_cfg) == 'src'
  assert api.gclient.get_repo_path(
      'https://chromium.googlesource.com/chromium/src',
      gclient_config=src_cfg) == 'src'
  assert api.gclient.get_repo_path(
      'https://chromium.googlesource.com/v8/v8',
      gclient_config=src_cfg) == 'src/v8'
  assert api.gclient.get_repo_path(
      'https://webrtc.googlesource.com/src',
      gclient_config=src_cfg) == 'src/third_party/webrtc'
  assert api.gclient.get_repo_path(
      'https://example.googlesource.com/unrecognized',
      gclient_config=src_cfg) is None

  api.gclient.c = src_cfg
  patch_root = api.gclient.get_gerrit_patch_root(gclient_config=src_cfg)
  assert patch_root == api.properties['expected_patch_root'], patch_root

  api.gclient.set_patch_repo_revision()


def GenTests(api):
  yield (
      api.test('chromium_ci') +
      api.buildbucket.ci_build(
          project='chromium',
          builder='linux',
          git_repo='https://chromium.googlesource.com/src') +
      api.properties(expected_patch_root=None) +
      api.post_process(post_process.DropExpectation)
  )

  yield (
      api.test('chromium_try') +
      api.buildbucket.try_build(
          project='chromium',
          builder='linux',
          git_repo='https://chromium.googlesource.com/src') +
      api.properties(expected_patch_root='src') +
      api.post_process(post_process.DropExpectation)
  )

  yield (
      api.test('v8_try') +
      api.buildbucket.try_build(
          project='chromium',
          builder='linux',
          git_repo='https://chromium.googlesource.com/v8/v8') +
      api.properties(expected_patch_root='src/v8') +
      api.post_process(post_process.DropExpectation)
  )

  yield (
      api.test('webrtc_try') +
      api.buildbucket.try_build(
          project='chromium',
          builder='linux',
          git_repo='https://webrtc.googlesource.com/src') +
      api.properties(expected_patch_root='src/third_party/webrtc')+
      api.post_process(post_process.DropExpectation)
  )
