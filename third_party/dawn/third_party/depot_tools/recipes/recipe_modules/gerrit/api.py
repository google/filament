# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import recipe_api
from recipe_engine.recipe_test_api import StepTestData
from typing import Callable, Optional

class GerritApi(recipe_api.RecipeApi):
  """Module for interact with Gerrit endpoints"""

  def __init__(self, *args, **kwargs):
    super(GerritApi, self).__init__(*args, **kwargs)
    self._changes_target_branch_cache = {}

  def __call__(self, name, cmd, infra_step=True, **kwargs):
    """Wrapper for easy calling of gerrit_utils steps."""
    assert isinstance(cmd, (list, tuple))
    prefix = 'gerrit '

    env = self.m.context.env
    env.setdefault('PATH', '%(PATH)s')
    env['PATH'] = self.m.path.pathsep.join([
        env['PATH'], str(self.repo_resource())])

    with self.m.context(env=env):
      return self.m.step(
          prefix + name,
          ['vpython3', self.repo_resource('gerrit_client.py')] + cmd,
          infra_step=infra_step,
          **kwargs)

  def call_raw_api(self,
                   host,
                   path,
                   method=None,
                   body=None,
                   accept_statuses=None,
                   name=None,
                   **kwargs):
    """Call an arbitrary Gerrit API that returns a JSON response.

    Returns:
      The JSON response data.
    """
    args = [
        'rawapi', '--host', host, '--path', path, '--json_file',
        self.m.json.output()
    ]
    if method:
      args.extend(['--method', method])
    if body:
      args.extend(['--body', self.m.json.dumps(body)])
    if accept_statuses:
      args.extend(
          ['--accept_status', ','.join(str(i) for i in accept_statuses)])

    step_name = name or 'call_raw_api (%s)' % path
    step_result = self(step_name, args, **kwargs)
    return step_result.json.output

  def create_gerrit_branch(self, host, project, branch, commit, **kwargs):
    """Creates a new branch from given project and commit

    Returns:
      The ref of the branch created
    """
    args = [
        'branch',
        '--host', host,
        '--project', project,
        '--branch', branch,
        '--commit', commit,
        '--json_file', self.m.json.output()
    ]
    allow_existent_branch = kwargs.pop('allow_existent_branch', False)
    if allow_existent_branch:
      args.append('--allow-existent-branch')
    step_name = 'create_gerrit_branch (%s %s)' % (project, branch)
    step_result = self(step_name, args, **kwargs)
    ref = step_result.json.output.get('ref')
    return ref

  def create_gerrit_tag(self, host, project, tag, commit, **kwargs):
    """Creates a new tag at the given commit.

    Returns:
      The ref of the tag created.
    """
    args = [
        'tag',
        '--host', host,
        '--project', project,
        '--tag', tag,
        '--commit', commit,
        '--json_file', self.m.json.output()
    ]
    step_name = 'create_gerrit_tag (%s %s)' % (project, tag)
    step_result = self(step_name, args, **kwargs)
    ref = step_result.json.output.get('ref')
    return ref

  # TODO(machenbach): Rename to get_revision? And maybe above to
  # create_ref?
  def get_gerrit_branch(self, host, project, branch, **kwargs):
    """Gets a branch from given project and commit

    Returns:
      The revision of the branch
    """
    args = [
        'branchinfo',
        '--host', host,
        '--project', project,
        '--branch', branch,
        '--json_file', self.m.json.output()
    ]
    step_name = 'get_gerrit_branch (%s %s)' % (project, branch)
    step_result = self(step_name, args, **kwargs)
    revision = step_result.json.output.get('revision')
    return revision

  def get_change_description(self,
                             host,
                             change,
                             patchset,
                             timeout=None,
                             step_test_data=None):
    """Gets the description for a given CL and patchset.

    Args:
      host: URL of Gerrit host to query.
      change: The change number.
      patchset: The patchset number.

    Returns:
      The description corresponding to given CL and patchset.
    """
    ri = self.get_revision_info(host, change, patchset, timeout, step_test_data)
    return ri['commit']['message']

  def get_revision_info(self,
                        host,
                        change,
                        patchset,
                        timeout=None,
                        step_test_data=None):
    """
    Returns the info for a given patchset of a given change.

    Args:
      host: Gerrit host to query.
      change: The change number.
      patchset: The patchset number.

    Returns:
      A dict for the target revision as documented here:
          https://gerrit-review.googlesource.com/Documentation/rest-api-changes.html#list-changes
    """
    assert int(change), change
    assert int(patchset), patchset

    step_test_data = step_test_data or (
        lambda: self.test_api.get_one_change_response_data(change_number=change,
                                                           patchset=patchset))

    cls = self.get_changes(host,
                           query_params=[('change', str(change))],
                           o_params=['ALL_REVISIONS', 'ALL_COMMITS'],
                           limit=1,
                           timeout=timeout,
                           step_test_data=step_test_data)
    cl = cls[0] if len(cls) == 1 else {'revisions': {}}
    for ri in cl['revisions'].values():
      # TODO(tandrii): add support for patchset=='current'.
      if str(ri['_number']) == str(patchset):
        return ri

    raise self.m.step.InfraFailure(
        'Error querying for CL description: host:%r change:%r; patchset:%r' % (
            host, change, patchset))

  def get_changes(self, host, query_params, start=None, limit=None,
                  o_params=None, step_test_data=None, **kwargs):
    """Queries changes for the given host.

    Args:
      * host: URL of Gerrit host to query.
      * query_params: Query parameters as list of (key, value) tuples to form a
          query as documented here:
          https://gerrit-review.googlesource.com/Documentation/user-search.html#search-operators
      * start: How many changes to skip (starting with the most recent).
      * limit: Maximum number of results to return.
      * o_params: A list of additional output specifiers, as documented here:
          https://gerrit-review.googlesource.com/Documentation/rest-api-changes.html#list-changes
      * step_test_data: Optional mock test data for the underlying gerrit client.

    Returns:
      A list of change dicts as documented here:
          https://gerrit-review.googlesource.com/Documentation/rest-api-changes.html#list-changes
    """
    args = [
        'changes',
        '--verbose',
        '--host', host,
        '--json_file', self.m.json.output()
    ]
    if start:
      args += ['--start', str(start)]
    if limit:
      args += ['--limit', str(limit)]
    for k, v in query_params:
      args += ['-p', '%s=%s' % (k, v)]
    for v in (o_params or []):
      args += ['-o', v]
    if not step_test_data:
      step_test_data = lambda: self.test_api.get_one_change_response_data()

    return self(
        kwargs.pop('name', 'changes'),
        args,
        step_test_data=step_test_data,
        **kwargs
    ).json.output

  def get_related_changes(self, host, change, revision='current', step_test_data=None):
    """Queries related changes for a given host, change, and revision.

    Args:
      * host: URL of Gerrit host to query.
      * change: The change-id of the change to get related changes for as
          documented here:
          https://gerrit-review.googlesource.com/Documentation/rest-api-changes.html#change-id
      * revision: The revision-id of the revision to get related changes for as
          documented here:
          https://gerrit-review.googlesource.com/Documentation/rest-api-changes.html#revision-id
          This defaults to current, which names the most recent patch set.
      * step_test_data: Optional mock test data for the underlying gerrit client.

    Returns:
      A related changes dictionary as documented here:
          https://gerrit-review.googlesource.com/Documentation/rest-api-changes.html#related-changes-info

    """
    args = [
        'relatedchanges',
        '--host',
        host,
        '--change',
        change,
        '--revision',
        revision,
        '--json_file',
        self.m.json.output(),
    ]
    if not step_test_data:
      step_test_data = lambda: self.test_api.get_related_changes_response_data()

    return self('relatedchanges', args,
                step_test_data=step_test_data).json.output

  def abandon_change(self, host, change, message=None, name=None,
                     step_test_data=None):
    args = [
        'abandon',
        '--host', host,
        '--change', int(change),
        '--json_file', self.m.json.output(),
    ]
    if message:
      args.extend(['--message', message])
    if not step_test_data:
      step_test_data = lambda: self.test_api.get_one_change_response_data(
          status='ABANDONED', _number=str(change))

    return self(
        name or 'abandon',
        args,
        step_test_data=step_test_data,
    ).json.output

  def restore_change(self, host, change, message=None, name=None,
                     step_test_data=None):
    args = [
        'restore',
        '--host', host,
        '--change', change,
        '--json_file', self.m.json.output(),
    ]
    if message:
      args.extend(('--message', message))
    if not step_test_data:
      step_test_data = lambda: self.test_api.get_one_change_response_data(
          status='NEW', _number=str(change))

    return self(
        name or 'restore',
        args,
        step_test_data=step_test_data,
    ).json.output

  def set_change_label(self,
                       host,
                       change,
                       label_name,
                       label_value,
                       name=None,
                       step_test_data=None):
    args = [
        'setlabel', '--host', host, '--change',
        int(change), '--json_file',
        self.m.json.output(), '-l', label_name, label_value
    ]
    return self(
        name or 'setlabel',
        args,
        step_test_data=step_test_data,
    ).json.output

  def add_message(
      self,
      host: str,
      change: int,
      message: str,
      revision: str | int = 'current',
      automatic_attention_set_update: Optional[bool] = None,
      step_name: str = None,
      step_test_data: Callable[[], StepTestData] | None = None) -> None:
    """Add a message to a change at given revision.

    Args:
      * host: URL of Gerrit host of the change.
      * change: The ID of the change to add message to as documented here:
          https://gerrit-review.googlesource.com/Documentation/rest-api-changes.html#change-id
      * message: The content of the message to add to the change.
      * revision: The ID of the revision of change to add message to as
          documented here:
          https://gerrit-review.googlesource.com/Documentation/rest-api-changes.html#revision-id
          This defaults to current, which names the most recent patchset.
      * automatic_attention_set_update: Whether to update the attention set.
      * step_name: Optional step name.
      * step_test_data: Optional mock test data for the underlying gerrit
          client.
    """
    args = [
        'addmessage', '--host', host, '--change',
        int(change), '--revision',
        str(revision), '--message', message, '--json_file',
        self.m.json.output()
    ]
    if automatic_attention_set_update is not None:
      args += [
          '--automatic-attention-set-update' if automatic_attention_set_update
          else '--no-automatic-attention-set-update'
      ]
    if not step_test_data:
      step_test_data = lambda: self.m.json.test_api.output({})
    return self(
        step_name or 'add message',
        args,
        step_test_data=step_test_data,
    ).json.output

  def move_changes(self,
                   host,
                   project,
                   from_branch,
                   to_branch,
                   step_test_data=None):
    args = [
        'movechanges', '--host', host, '-p',
        'project=%s' % project, '-p',
        'branch=%s' % from_branch, '-p', 'status=open', '--destination_branch',
        to_branch, '--json_file',
        self.m.json.output()
    ]

    if not step_test_data:
      step_test_data = lambda: self.test_api.get_one_change_response_data(
          branch=to_branch)

    return self(
        'move changes',
        args,
        step_test_data=step_test_data,
    ).json.output

  def update_files(self,
                   host,
                   project,
                   branch,
                   new_contents_by_file_path,
                   commit_msg,
                   params=frozenset(['status=NEW']),
                   cc_list=frozenset([]),
                   submit=False,
                   submit_later=False,
                   step_test_data_create_change=None,
                   step_test_data_submit_change=None):
    """Update a set of files by creating and submitting a Gerrit CL.

    Args:
      * host: URL of Gerrit host to name.
      * project: Gerrit project name, e.g. chromium/src.
      * branch: The branch to land the change, e.g. main
      * new_contents_by_file_path: Dict of the new contents with file path as
          the key.
      * commit_msg: Description to add to the CL.
      * params: A list of additional ChangeInput specifiers, with format
          'key=value'.
      * cc_list: A list of addresses to notify.
      * submit: Should land this CL instantly.
      * submit_later: If this change has related CLs, we may want to commit
           them in a chain. So only set Bot-Commit+1, making it ready for
           submit together. Ignored if submit is True.
      * step_test_data_create_change: Optional mock test data for the step
           create gerrit change.
      * step_test_data_submit_change: Optional mock test data for the step
          submit gerrit change.

    Returns:
      A ChangeInfo dictionary as documented here:
          https://gerrit-review.googlesource.com/Documentation/rest-api-changes.html#create-change
          Or if the change is submitted, here:
          https://gerrit-review.googlesource.com/Documentation/rest-api-changes.html#submit-change
    """
    assert len(new_contents_by_file_path
               ) > 0, 'The dict of file paths should not be empty.'
    command = [
        'createchange',
        '--host',
        host,
        '--project',
        project,
        '--branch',
        branch,
        '--subject',
        commit_msg,
        '--json_file',
        self.m.json.output(),
    ]
    for p in params:
      command.extend(['-p', p])
    for cc in cc_list:
      command.extend(['--cc', cc])
    step_test_data = step_test_data_create_change or (
        lambda: self.test_api.update_files_response_data())

    step_result = self('create change at (%s %s)' % (project, branch),
                       command,
                       step_test_data=step_test_data)
    change = int(step_result.json.output.get('_number'))
    step_result.presentation.links['change %d' %
                                   change] = '%s/#/q/%d' % (host, change)

    with self.m.step.nest('update contents in CL %d' % change):
      for path, content in new_contents_by_file_path.items():
        _file = self.m.path.mkstemp()
        self.m.file.write_raw('store the new content for %s' % path, _file,
                              content)
        self('edit file %s' % path, [
            'changeedit',
            '--host',
            host,
            '--change',
            change,
            '--path',
            path,
            '--file',
            _file,
        ])

    self('publish edit', [
        'publishchangeedit',
        '--host',
        host,
        '--change',
        change,
    ])

    # Make sure the new patchset is propagated to Gerrit backend.
    with self.m.step.nest('verify the patchset exists on CL %d' % change):
      retries = 0
      max_retries = 2
      while retries <= max_retries:
        try:
          if self.get_revision_info(host, change, 2):
            break
        except self.m.step.InfraFailure:
          if retries == max_retries:  # pragma: no cover
            raise
          retries += 1
          with self.m.step.nest('waiting before retry'):
            self.m.time.sleep((2**retries) * 10)

    if submit or submit_later:
      self('set Bot-Commit+1 for change %d' % change, [
          'setbotcommit',
          '--host',
          host,
          '--change',
          change,
      ])
    if submit:
      submit_cmd = [
          'submitchange',
          '--host',
          host,
          '--change',
          change,
          '--json_file',
          self.m.json.output(),
      ]
      step_test_data = step_test_data_submit_change or (
          lambda: self.test_api.update_files_response_data(status='MERGED'))
      step_result = self('submit change %d' % change,
                         submit_cmd,
                         step_test_data=step_test_data)
    return step_result.json.output
