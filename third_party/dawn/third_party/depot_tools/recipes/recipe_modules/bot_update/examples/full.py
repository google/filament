# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import post_process
from recipe_engine import recipe_api

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
  'bot_update',
  'gclient',
  'gerrit',
  'tryserver',
  'recipe_engine/buildbucket',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/runtime',
]

from recipe_engine import engine_types

from RECIPE_MODULES.depot_tools import gclient
from PB.go.chromium.org.luci.buildbucket.proto.build import Build


# TODO: crbug.com/339472834 - Once all downstream uses of presentation and
# json.output have been removed, this test can be updated to not reference them
# and the decorator can be removed
@recipe_api.ignore_warnings('^depot_tools/BOT_UPDATE_CUSTOM_RESULT_ATTRIBUTES$')
def RunSteps(api):
  api.gclient.use_mirror = True
  commit = api.buildbucket.build.input.gitiles_commit

  src_cfg = api.gclient.make_config(CACHE_DIR=api.path.cache_dir / 'git')
  soln = src_cfg.solutions.add()
  soln.name = 'src'
  soln.url = 'https://chromium.googlesource.com/chromium/src.git'
  if api.properties.get('revision_fallback_chain'):
    soln.revision = gclient.api.RevisionFallbackChain()
  else:
    soln.revision = commit.id or commit.ref or None
  api.gclient.c = src_cfg
  api.gclient.c.revisions.update(api.properties.get('revisions', {}))
  if api.properties.get('deprecated_got_revision_mapping'):
    api.gclient.c.got_revision_mapping['src'] = 'got_cr_revision'
  else:
    api.gclient.c.got_revision_reverse_mapping['got_cr_revision'] = 'src'
    api.gclient.c.got_revision_reverse_mapping['got_revision'] = 'src'
    api.gclient.c.got_revision_reverse_mapping['got_v8_revision'] = 'src/v8'
    api.gclient.c.got_revision_reverse_mapping['got_angle_revision'] = (
        'src/third_party/angle')
  api.gclient.c.repo_path_map.update({
      'https://chromium.googlesource.com/angle/angle': (
          'src/third_party/angle', 'HEAD'),
      'https://chromium.googlesource.com/v8/v8': ('src/v8', 'HEAD'),
      'https://webrtc.googlesource.com/src': ('src/third_party/webrtc', 'HEAD'),
  })

  patch = api.properties.get('patch', True)
  clobber = True if api.properties.get('clobber') else False
  with_branch_heads = api.properties.get('with_branch_heads', False)
  with_tags = api.properties.get('with_tags', False)
  refs = api.properties.get('refs', [])
  root_solution_revision = api.properties.get('root_solution_revision')
  suffix = api.properties.get('suffix')
  gerrit_no_reset = True if api.properties.get('gerrit_no_reset') else False
  gerrit_no_rebase_patch_ref = bool(
      api.properties.get('gerrit_no_rebase_patch_ref'))
  patch_refs = api.properties.get('patch_refs')
  add_blamelists = api.properties.get('add_blamelists', False)
  set_output_commit = api.properties.get('set_output_commit', True)

  rev_overrides = {'infra': 'HEAD'}
  bot_update_step = api.bot_update.ensure_checkout(
      patch=patch,
      with_branch_heads=with_branch_heads,
      with_tags=with_tags,
      refs=refs,
      clobber=clobber,
      root_solution_revision=root_solution_revision,
      suffix=suffix,
      gerrit_no_reset=gerrit_no_reset,
      gerrit_no_rebase_patch_ref=gerrit_no_rebase_patch_ref,
      patch_refs=patch_refs,
      add_blamelists=add_blamelists,
      set_output_commit=set_output_commit,
      recipe_revision_overrides=rev_overrides,
  )
  if patch:
    api.bot_update.deapply_patch(bot_update_step)

  if api.properties.get('resolve_chromium_fixed_version'):
    api.bot_update.resolve_fixed_revision(bot_update_step, 'src')

def GenTests(api):

  def try_build(**kwargs):
    kwargs.setdefault(
        'git_repo', 'https://chromium.googlesource.com/chromium/src')
    return api.buildbucket.try_build('chromium/src', 'try', 'linux', **kwargs)

  def ci_build(**kwargs):
    kwargs.setdefault(
        'git_repo', 'https://chromium.googlesource.com/chromium/src')
    return (
        api.buildbucket.ci_build('chromium/src', 'ci', 'linux', **kwargs) +
        api.properties(patch=False)
    )


  yield (
      api.test('basic') +
      ci_build()
  )
  yield (
      api.test('input_commit_with_id_without_repo') +
      api.buildbucket.build(Build(
          input={
              'gitiles_commit': {
                  'id': 'a' * 40,
              },
          },
      ))
  )
  yield (api.test('unrecognized_commit_repo', status="INFRA_FAILURE") +
         ci_build(git_repo='https://unrecognized/repo'))

  yield api.test(
      'bot_update_failure',
      ci_build(),
      api.bot_update.fail_checkout(True),
      api.expect_status('INFRA_FAILURE'),
  )

  yield (
      api.test('resolve_chromium_fixed_version') +
      ci_build() +
      api.properties(resolve_chromium_fixed_version=True)
  )
  yield (
      api.test('basic_with_branch_heads') +
      ci_build() +
      api.properties(
          with_branch_heads=True,
          suffix='with branch heads'
      )
  )
  yield (
      api.test('with_tags') +
      api.properties(with_tags=True)
  )
  yield (
      api.test('with_experiments') +
      api.properties(bot_update_experiments=['no_sync','cool_experiment'])
  )
  yield (
      api.test('deprecated_got_revision_mapping') +
      try_build() +
      api.properties(
          deprecated_got_revision_mapping=True,
          set_output_commit=False,
      )
  )
  yield (
      api.test('refs') +
      api.properties(refs=['+refs/change/1/2/333'])
  )
  yield (api.test('tryjob_fail', status="INFRA_FAILURE") + try_build() +
         api.step_data('bot_update', api.json.invalid(None), retcode=1))

  yield api.test(
      'tryjob_fail_patch',
      try_build(),
      api.bot_update.fail_patch(True),
      api.expect_status('FAILURE'),
  )

  yield api.test(
      'tryjob_fail_patch_download',
      try_build(),
      api.bot_update.fail_patch('download'),
      api.expect_status('INFRA_FAILURE'),
  )

  yield api.test(
      'tryjob_fail_missing_bot_update_json',
      try_build(),
      api.override_step_data('bot_update', retcode=1),
      api.post_process(post_process.SummaryMarkdownRE, 'Infra Failure.*'),
      api.post_process(post_process.StatusException),
      api.post_process(post_process.DropExpectation),
      status='INFRA_FAILURE')

  yield (
      api.test('clobber') +
      api.properties(clobber=1)
  )
  yield (
      api.test('reset_root_solution_revision') +
      api.properties(root_solution_revision=api.bot_update.gen_revision('fake-revision'))
  )
  yield (
      api.test('gerrit_no_reset') +
      api.properties(gerrit_no_reset=1)
  )
  yield (
      api.test('gerrit_no_rebase_patch_ref') +
      api.properties(gerrit_no_rebase_patch_ref=True)
  )
  yield (
      api.test('tryjob_v8') +
      try_build(git_repo='https://chromium.googlesource.com/v8/v8') +
       api.properties(revisions={'src/v8': 'abc'})
  )
  yield (
      api.test('tryjob_v8_head_by_default') +
      try_build(git_repo='https://chromium.googlesource.com/v8/v8')
  )
  yield (
      api.test('tryjob_gerrit_angle') +
      try_build(git_repo='https://chromium.googlesource.com/angle/angle')
  )
  yield (
      api.test('tryjob_gerrit_v8_feature_branch') +
      try_build(git_repo='https://chromium.googlesource.com/v8/v8') +
      api.tryserver.gerrit_change_target_ref('refs/heads/experimental/feature')
  )
  yield (
      api.test('tryjob_gerrit_feature_branch') +
      try_build() +
      api.tryserver.gerrit_change_target_ref('refs/heads/experimental/feature')
  )
  yield (
      api.test('tryjob_gerrit_branch_heads') +
      try_build() +
      api.tryserver.gerrit_change_target_ref('refs/branch-heads/67')
  )
  yield (
      api.test('tryjob_gerrit_webrtc') +
      try_build(git_repo='https://webrtc.googlesource.com/src')
  )
  yield (
      api.test('multiple_patch_refs') +
      api.properties(
          patch_refs=[
              ('https://chromium.googlesource.com/chromium/src@'
               'refs/changes/12/34/5'),
              'https://chromium.googlesource.com/v8/v8@refs/changes/124/45/6',
          ],
      )
  )
  yield (
      api.test('origin_main') +
      ci_build(revision='origin/main')
  )

  yield (
      api.test('revision_fallback_chain_set_output_commit') + ci_build() +
      api.properties(
          set_output_commit=True,
          revision_fallback_chain=True,
      ) +
      # Don't set commit position properties so that the set_output_commit code
      # attempts to do comparisons on the revision value
      api.step_data(
          'bot_update (without patch)',
          api.bot_update.output_json(patch_root='src',
                                     first_sln='src',
                                     revision_mapping={'got_revision': 'src'},
                                     commit_positions=False)))

  yield api.test(
      'upload_traces',
      try_build(),
      api.bot_update.fail_patch(True),
      api.expect_status('FAILURE'),
  )

  yield api.test(
      'upload_traces_fail',
      try_build(),
      api.bot_update.fail_patch(True),
      api.step_data(
          'upload git traces.gsutil upload',
          retcode=1,
      ),
      api.expect_status('FAILURE'),
  )

  yield (
      api.test('revision_specifying_ref') +
      ci_build() +
      api.properties(
          revisions={'src': 'refs/branch-heads/4000'},
      )
  )

  yield (
      api.test('add_blamelists') +
      ci_build() +
      api.properties(
          add_blamelists=True,
          revisions={'src/v8': 'HEAD'},
      )
  )

  yield api.test(
      'add_blamelists_bot_update_failure',
      ci_build(),
      api.properties(
          add_blamelists=True,
          revisions={'src/v8': 'HEAD'},
      ),
      api.bot_update.fail_checkout(True),
      api.expect_status('INFRA_FAILURE'),
  )

  yield api.test(
      'no_cp_checkout_a_specific_commit',
      ci_build(revision='a' * 40),
      api.bot_update.commit_positions(False),
  )

  yield api.test(
      'no_cp_checkout_main',
      ci_build(revision=''),
      api.bot_update.commit_positions(False),
  )

  yield api.test(
      'no_cp_checkout_a_branch_head',
      ci_build(revision='', git_ref='refs/branch-heads/x'),
      api.bot_update.commit_positions(False),
  )

  yield api.test(
      'no_cp_checkout_HEAD',
      ci_build(revision='HEAD'),
      api.bot_update.commit_positions(False),
  )

  yield (api.test('stale-process-duration-override') + api.properties(
      **
      {'$depot_tools/bot_update': {
          'stale_process_duration_override': 3000,
      }}) + ci_build())
