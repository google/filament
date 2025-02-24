# Copyright 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import post_process as post

DEPS = [
    'recipe_engine/json',
    'recipe_engine/properties',
    'recipe_engine/raw_io',
    'recipe_engine/step',
    'git',
]

REPO_URL = 'https://chromium.googlesource.com/v8/v8'


def RunSteps(api):
  url = api.properties.get('url', REPO_URL)
  ref = api.properties.get('ref', 'main')

  result = api.git.ls_remote(url, ref)
  api.step.empty('revision', step_text=result)


def GenTests(api):

  def mock_ls_remote(ref, revision_refs, retcode=None):
    lines = [f"{revision}\t{ref}" for revision, ref in revision_refs] + ['']

    return api.step_data(
        f'Retrieve revision for {ref}',
        api.raw_io.stream_output_text(
            '\n'.join(lines),
            retcode=retcode or 0,
            stream='stdout',
        ),
    )

  def test(name, cmd, *checks, **kwargs):
    ref = cmd[-1]

    return api.test(
        name,
        api.post_process(
            post.StepCommandEquals,
            f'Retrieve revision for {ref}',
            cmd,
        ),
        *checks,
        api.post_process(post.DropExpectation),
        **kwargs,
    )

  yield test(
      'basic',
      ['git', 'ls-remote', REPO_URL, 'main'],
      mock_ls_remote('main', [('badc0ffee0ded', 'refs/heads/main')]),
      api.post_process(
          post.StepTextEquals,
          'revision',
          'badc0ffee0ded',
      ),
  )

  yield test(
      'multiple-refs',
      ['git', 'ls-remote', REPO_URL, '13.3.19'],
      api.properties(ref='13.3.19'),
      mock_ls_remote('13.3.19', [
          ('badc0ffee0ded', 'refs/heads/13.3.19'),
          ('c001c001c001d', 'refs/tags/13.3.19'),
      ]),
      api.post_process(post.StatusFailure),
      api.post_process(post.SummaryMarkdown,
                       "Multiple remote refs found for 13.3.19"),
      status='FAILURE',
  )

  yield test(
      'no-refs',
      ['git', 'ls-remote', REPO_URL, '13.3'],
      api.properties(ref='13.3'),
      mock_ls_remote('13.3', []),
      api.post_process(post.StatusFailure),
      api.post_process(post.SummaryMarkdown, "No remote ref found for 13.3"),
      status='FAILURE',
  )
