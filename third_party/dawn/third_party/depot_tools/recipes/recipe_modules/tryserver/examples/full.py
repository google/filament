# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
  'gerrit',
  'recipe_engine/buildbucket',
  'recipe_engine/json',
  'recipe_engine/raw_io',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/step',
  'tryserver',
]

from PB.go.chromium.org.luci.buildbucket.proto.common import GerritChange


def RunSteps(api):
  api.path.checkout_dir = api.path.start_dir
  if api.properties.get('patch_text'):
    api.step('patch_text test', [
        'echo', str(api.tryserver.get_footers(api.properties['patch_text']))])
    api.step('patch_text test', [
        'echo', str(api.tryserver.get_footer(
            'Foo', api.properties['patch_text']))])
    return

  if api.tryserver.gerrit_change:
    assert (api.tryserver.gerrit_change_repo_url ==
            'https://chromium.googlesource.com/chromium/src')
    assert (api.tryserver.gerrit_change_repo_host ==
            'chromium.googlesource.com')
    assert (api.tryserver.gerrit_change_repo_project ==
            'chromium/src')
    assert api.tryserver.gerrit_change_fetch_ref == 'refs/changes/27/91827/1'
    expected_target_ref = api.properties.get(
        'expected_target_ref', 'refs/heads/main')
    assert api.tryserver.gerrit_change_target_ref == expected_target_ref
    assert (api.tryserver.gerrit_change_review_url ==
            'https://chromium-review.googlesource.com/c/91827/1')

  if api.tryserver.is_gerrit_issue:
    api.tryserver.get_footers()
    api.tryserver.get_footer('testfooter')

  if api.tryserver.is_tryserver:
    api.tryserver.set_subproject_tag('v8')

  api.tryserver.set_patch_failure_tryjob_result()
  api.tryserver.set_compile_failure_tryjob_result()
  api.tryserver.set_test_failure_tryjob_result()
  api.tryserver.set_invalid_test_results_tryjob_result()
  api.tryserver.set_test_timeout_tryjob_result()
  api.tryserver.set_test_expired_tryjob_result()

  api.tryserver.normalize_footer_name('Cr-Commit-Position')

  api.tryserver.set_change(
      GerritChange(host='chromium-review.googlesource.com',
                   project='infra/luci/recipes-py',
                   change=1234567,
                   patchset=1))
  assert (api.tryserver.gerrit_change_repo_url ==
          'https://chromium.googlesource.com/infra/luci/recipes-py')
  assert api.tryserver.gerrit_change_fetch_ref == 'refs/changes/67/1234567/1'


def GenTests(api):
  # The 'test_patch_root' property used below is just so that these
  # tests can avoid using the gclient module to calculate the
  # patch root. Normal users would use gclient.get_gerrit_patch_root().
  yield (api.test('with_wrong_patch') +
         api.platform('win', 32) +
         api.properties(test_patch_root=''))

  yield (api.test('with_gerrit_patch') +
         api.buildbucket.try_build(
            'chromium',
            'linux',
            git_repo='https://chromium.googlesource.com/chromium/src',
            change_number=91827,
            patch_set=1))

  yield (api.test('with_gerrit_patch_and_target_ref') +
         api.buildbucket.try_build(
            'chromium',
            'linux',
            git_repo='https://chromium.googlesource.com/chromium/src',
            change_number=91827,
            patch_set=1) +
         api.properties(expected_target_ref='refs/heads/experiment') +
         api.tryserver.gerrit_change_target_ref('refs/heads/experiment'))

  yield (api.test('with_wrong_patch_new') + api.platform('win', 32) +
         api.properties(test_patch_root='sub\\project'))

  yield (api.test('basic_tags') +
         api.properties(
             patch_text='hihihi\nfoo:bar\nbam:baz',
             footer='foo'
         ) +
         api.step_data(
             'parse description',
             api.json.output({'Foo': ['bar']})) +
         api.step_data(
             'parse description (2)',
             api.json.output({'Foo': ['bar']}))
  )
