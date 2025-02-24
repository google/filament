# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
    'gerrit',
    'recipe_engine/json',
    'recipe_engine/step',
]


def RunSteps(api):
  host = 'https://chromium-review.googlesource.com'
  project = 'v8/v8'

  branch = 'test'
  commit = '67ebf73496383c6777035e374d2d664009e2aa5c'

  data = api.gerrit.create_gerrit_branch(host,
                                         project,
                                         branch,
                                         commit,
                                         allow_existent_branch=True)
  assert data == 'refs/heads/test'

  data = api.gerrit.get_gerrit_branch(host, project, 'main')
  assert data == '67ebf73496383c6777035e374d2d664009e2aa5c'

  data = api.gerrit.create_gerrit_tag(host, project, '1.0', commit)
  assert data == 'refs/tags/1.0'

  tag_body = {
      "revision": "67ebf73496383c6777035e374d2d664009e2aa5c",
  }
  json_data = api.gerrit.call_raw_api(host,
                                      '/projects/%s/tags/1.0' % project,
                                      method='PUT',
                                      body=tag_body,
                                      accept_statuses=[201],
                                      name='raw_create_tag')
  assert json_data['ref'] == 'refs/tags/1.0'

  api.gerrit.move_changes(host, project, 'master', 'main')

  change_info = api.gerrit.update_files(host,
                                        project,
                                        'main',
                                        {'chrome/VERSION': '99.99.99.99'},
                                        'Dummy CL.',
                                        submit=True,
                                        cc_list=['foo@example.com'])
  assert int(change_info['_number']) == 91827, change_info
  assert change_info['status'] == 'MERGED'

  # Query for changes in Chromium's CQ.
  api.gerrit.get_changes(
      host,
      query_params=[
        ('project', 'chromium/src'),
        ('status', 'open'),
        ('label', 'Commit-Queue>0'),
      ],
      start=1,
      limit=1,
  )
  related_changes = api.gerrit.get_related_changes(host,
                                                   change='58478',
                                                   revision='2')
  assert len(related_changes["changes"]) == 1

  # Query which returns no changes is still successful query.
  empty_list = api.gerrit.get_changes(
      host,
      query_params=[
        ('project', 'chromium/src'),
        ('status', 'open'),
        ('label', 'Commit-Queue>2'),
      ],
      name='changes empty query',
  )
  assert len(empty_list) == 0

  api.gerrit.get_change_description(
      host, change=123, patchset=1)

  api.gerrit.set_change_label(host, 123, 'code-review', -1)
  api.gerrit.set_change_label(host, 123, 'commit-queue', 1)

  api.gerrit.add_message(host,
                         123,
                         'This is a non-attention message',
                         automatic_attention_set_update=False)
  api.gerrit.add_message(host, 123, 'This is a comment message')

  api.gerrit.abandon_change(host, 123, 'bad roll')
  api.gerrit.restore_change(host, 123, 'nevermind')

  api.gerrit.get_change_description(
      host,
      change=122,
      patchset=3,
      step_test_data=api.gerrit.test_api.get_empty_changes_response_data)


def GenTests(api):
  yield (api.test('basic', status="INFRA_FAILURE") +
         api.step_data('gerrit create_gerrit_branch (v8/v8 test)',
                       api.gerrit.make_gerrit_create_branch_response_data()) +
         api.step_data('gerrit create_gerrit_tag (v8/v8 1.0)',
                       api.gerrit.make_gerrit_create_tag_response_data()) +
         api.step_data('gerrit raw_create_tag',
                       api.gerrit.make_gerrit_create_tag_response_data()) +
         api.step_data('gerrit create change at (v8/v8 main)',
                       api.gerrit.update_files_response_data()) +
         api.step_data('verify the patchset exists on CL 91827.gerrit changes',
                       api.gerrit.get_empty_changes_response_data()) +
         api.step_data('gerrit submit change 91827',
                       api.gerrit.update_files_response_data(status='MERGED')) +
         api.step_data('gerrit get_gerrit_branch (v8/v8 main)',
                       api.gerrit.make_gerrit_get_branch_response_data()) +
         api.step_data('gerrit move changes',
                       api.gerrit.get_move_change_response_data(branch='main'))
         + api.step_data('gerrit relatedchanges',
                         api.gerrit.get_related_changes_response_data()) +
         api.step_data('gerrit changes empty query',
                       api.gerrit.get_empty_changes_response_data()))
