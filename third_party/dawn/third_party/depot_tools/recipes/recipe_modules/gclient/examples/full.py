# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
    'gclient',
    'recipe_engine/assertions',
    'recipe_engine/buildbucket',
    'recipe_engine/context',
    'recipe_engine/file',
    'recipe_engine/path',
    'recipe_engine/properties',
    'recipe_engine/step',
]


TEST_CONFIGS = [
    'android',
    'boringssl',
    'build_internal',
    'catapult',
    'celab',
    'crashpad',
    'crossbench',
    'custom_tabs_client',
    'dart',
    'dawn',
    'emscripten_releases',
    'expect_tests',
    'gerrit',
    'gerrit_plugins_binary_size',
    'gerrit_plugins_buildbucket',
    'gerrit_plugins_chromium_behavior',
    'gerrit_plugins_chromium_binary_size',
    'gerrit_plugins_chumpdetector',
    'gerrit_plugins_code_coverage',
    'gerrit_plugins_git_numberer',
    'gerrit_plugins_landingwidget',
    'gerrit_plugins_tricium',
    'gerrit_test_cq_normal',
    'gyp',
    'infra',
    'infra_superproject',
    'infra_internal_superproject',
    'infradata_master_manager',
    'infradata_cloud_run',
    'infradata_config',
    'infradata_rbe',
    'luci_gae',
    'luci_go',
    'luci_py',
    'nacl',
    'devtools',
    'openscreen',
    'pdfium',
    'recipes_py',
    'recipes_py_bare',
    'tint',
    'webports',
    'with_branch_heads',
    'with_tags',
    'ytdevinfra',
]

DEPS_CONTENT = """
git_dependencies = 'SYNC'
"""


def RunSteps(api):
  file_changes = api.gclient.roll_deps(
      'foo', {'foo': '2222222222222222222222222222222222222222'},
      test_data=DEPS_CONTENT)

  api.assertions.assertEqual(file_changes['foo'],
                             b'2222222222222222222222222222222222222222')

  file_changes = api.gclient.roll_deps(
      DEPS_CONTENT, {'src/bar': '2222222222222222222222222222222222222222'},
      strip_prefix_for_gitlink='src/',
      test_data=DEPS_CONTENT)

  # We expect src/ to be stripped from src/bar.
  api.assertions.assertEqual(file_changes['bar'],
                             b'2222222222222222222222222222222222222222')

  for config_name in TEST_CONFIGS:
    api.gclient.make_config(config_name)

  src_cfg = api.gclient.make_config(CACHE_DIR=api.path.cache_dir / 'git')
  soln = src_cfg.solutions.add()
  soln.name = 'src'
  soln.url = 'https://chromium.googlesource.com/chromium/src.git'
  soln.revision = api.buildbucket.gitiles_commit.id
  soln.custom_vars = {'string_var': 'string_val', 'true_var': True}
  src_cfg.parent_got_revision_mapping['parent_got_revision'] = 'got_revision'
  api.gclient.c = src_cfg
  api.gclient.checkout()

  api.gclient.spec_alias = 'Angle'
  bl_cfg = api.gclient.make_config()
  soln = bl_cfg.solutions.add()
  soln.name = 'Angle'
  soln.url = 'https://chromium.googlesource.com/angle/angle.git'
  bl_cfg.revisions['src/third_party/angle'] = 'refs/heads/lkgr'

  bl_cfg.got_revision_mapping['src/blatley'] = 'got_blatley_revision'
  with api.context(cwd=api.path.start_dir.joinpath('src', 'third_party')):
    api.gclient.checkout(
        gclient_config=bl_cfg)

  api.gclient.got_revision_reverse_mapping(bl_cfg)

  api.gclient.break_locks()

  del api.gclient.spec_alias

  api.gclient.runhooks()


def GenTests(api):
  yield api.test('basic')

  yield api.test('revision') + api.buildbucket.ci_build(revision='abc')

  yield api.test('tryserver') + api.buildbucket.try_build()
