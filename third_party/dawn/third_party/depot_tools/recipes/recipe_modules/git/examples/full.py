# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
  'recipe_engine/buildbucket',
  'recipe_engine/context',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/raw_io',
  'recipe_engine/step',

  'git',
]


def RunSteps(api):
  url = 'https://chromium.googlesource.com/chromium/src.git'

  # git.checkout can optionally dump GIT_CURL_VERBOSE traces to a log file,
  # useful for debugging git access issues that are reproducible only on bots.
  curl_trace_file = None
  if api.properties.get('use_curl_trace'):
    curl_trace_file = api.path.start_dir / 'curl_trace.log'

  submodule_update_force = api.properties.get('submodule_update_force', False)
  submodule_update_recursive = api.properties.get('submodule_update_recursive',
          True)

  # You can use api.git.checkout to perform all the steps of a safe checkout.
  revision = (api.buildbucket.gitiles_commit.ref or
              api.buildbucket.gitiles_commit.id)
  retVal = api.git.checkout(
      url,
      ref=revision,
      recursive=True,
      submodule_update_force=submodule_update_force,
      set_got_revision=api.properties.get('set_got_revision'),
      curl_trace_file=curl_trace_file,
      remote_name=api.properties.get('remote_name'),
      display_fetch_size=api.properties.get('display_fetch_size'),
      file_name=api.properties.get('checkout_file_name'),
      submodule_update_recursive=submodule_update_recursive,
      use_git_cache=api.properties.get('use_git_cache'),
      tags=api.properties.get('tags'),
      depth=api.properties.get('depth'))

  assert retVal == "deadbeef", (
    "expected retVal to be %r but was %r" % ("deadbeef", retVal))

  # count_objects shows number and size of objects in .git dir.
  api.git.count_objects(
      name='count-objects',
      raise_on_failure=api.properties.get('count_objects_can_fail_build', False),
      git_config_options={'foo': 'bar'})

  # Get the remote URL.
  api.git.get_remote_url(
      step_test_data=lambda: api.raw_io.test_api.stream_output('foo'))

  api.git.get_timestamp(test_data='foo')

  # You can use api.git.fetch_tags to fetch all tags from the remote
  api.git.fetch_tags(api.properties.get('remote_name'))

  # If you need to run more arbitrary git commands, you can use api.git itself,
  # which behaves like api.step(), but automatically sets the name of the step.
  with api.context(cwd=api.path.checkout_dir):
    api.git('status')

  api.git('status', name='git status can_fail_build', raise_on_failure=True)

  api.git('status', name='git status cannot_fail_build', raise_on_failure=False)

  # You should run git new-branch before you upload something with git cl.
  api.git.new_branch('refactor')  # Upstream is origin/main by default.

  if api.properties.get('set_both_upstream_and_upstream_current'):
    api.git.new_branch('failed_new_branch', upstream='will_fail', upstream_current=True) #pylint: disable = line-too-long
  # And use upstream kwarg to set up different upstream for tracking.
  api.git.new_branch('feature', upstream='refactor')
  # A new branching tracking the current branch, which is 'feature'.
  api.git.new_branch('track_current', upstream_current=True)
  # You can use api.git.rebase to rebase the current branch onto another one
  api.git.rebase(name_prefix='my repo', branch='origin/main',
                 dir_path=api.path.checkout_dir,
                 remote_name=api.properties.get('remote_name'))

  if api.properties.get('cat_file', None):
    step_result = api.git.cat_file_at_commit(api.properties['cat_file'],
                                             revision,
                                             stdout=api.raw_io.output())
    if 'TestOutput' in step_result.stdout.decode('utf-8'):
      pass  # Success!

  # Bundle the repository.
  api.git.bundle_create(
        api.path.start_dir / 'all.bundle')


def GenTests(api):
  yield api.test('basic')
  yield api.test('basic_tags') + api.properties(tags=True)
  yield api.test('basic_ref') + api.buildbucket.ci_build(git_ref='refs/foo/bar')
  yield api.test('basic_branch') + api.buildbucket.ci_build(
      git_ref='refs/heads/testing')
  yield api.test('basic_hash') + api.buildbucket.ci_build(
      revision='abcdef0123456789abcdef0123456789abcdef01', git_ref=None)
  yield api.test('basic_file_name') + api.properties(checkout_file_name='DEPS')
  yield api.test('basic_submodule_update_force') + api.properties(
      submodule_update_force=True)

  yield api.test('platform_win') + api.platform.name('win')

  yield (
      api.test('curl_trace_file') +
      api.properties(use_curl_trace=True) +
      api.buildbucket.ci_build(git_ref='refs/foo/bar')
  )

  yield (api.test('can_fail_build', status="INFRA_FAILURE") +
         api.step_data('git status can_fail_build', retcode=1))

  yield (
    api.test('cannot_fail_build') +
    api.step_data('git status cannot_fail_build', retcode=1)
  )

  yield (
    api.test('set_got_revision') +
    api.properties(set_got_revision=True)
  )

  yield (api.test('rebase_failed', status="INFRA_FAILURE") +
         api.step_data('my repo rebase', retcode=1))

  yield api.test('remote_not_origin') + api.properties(remote_name='not_origin')

  yield (
      api.test('count-objects_delta') +
      api.properties(display_fetch_size=True))

  yield (
      api.test('count-objects_failed') +
      api.step_data('count-objects', retcode=1))

  yield (
      api.test('count-objects_with_bad_output') +
      api.step_data(
          'count-objects',
          stdout=api.raw_io.output(api.git.count_objects_output('xxx'))))

  yield (api.test('count-objects_with_bad_output_fails_build',
                  status="INFRA_FAILURE") +
         api.step_data('count-objects',
                       stdout=api.raw_io.output(
                           api.git.count_objects_output('xxx'))) +
         api.properties(count_objects_can_fail_build=True))
  yield (
      api.test('cat-file_test') +
      api.step_data('git cat-file abcdef12345:TestFile',
                    stdout=api.raw_io.output('TestOutput')) +
      api.buildbucket.ci_build(revision='abcdef12345', git_ref=None) +
      api.properties(cat_file='TestFile'))

  yield (
      api.test('git-cache-checkout') +
      api.properties(use_git_cache=True))

  yield (api.test('new_branch_failed', status="INFRA_FAILURE") +
         api.properties(set_both_upstream_and_upstream_current=True) +
         api.expect_exception('ValueError'))

  yield (api.test('git-checkout-with-depth') + api.properties(depth=1))
