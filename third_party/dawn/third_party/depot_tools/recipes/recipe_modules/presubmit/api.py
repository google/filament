# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import recipe_api

from PB.recipe_engine import result as result_pb2
from PB.go.chromium.org.luci.buildbucket.proto import common as common_pb2

# 8 minutes seems like a reasonable upper bound on presubmit timings.
# According to event mon data we have, it seems like anything longer than
# this is a bug, and should just instant fail.
_DEFAULT_TIMEOUT_S = 480


class PresubmitApi(recipe_api.RecipeApi):

  def __init__(self, properties, **kwargs):
    super(PresubmitApi, self).__init__(**kwargs)

    self._runhooks = properties.runhooks
    self._timeout_s = properties.timeout_s or _DEFAULT_TIMEOUT_S

  @property
  def presubmit_support_path(self):
    return self.repo_resource('presubmit_support.py')

  def __call__(self, *args, **kwargs):
    """Returns a presubmit step."""

    name = kwargs.pop('name', 'presubmit')
    with self.m.depot_tools.on_path():
      cmd = ['vpython3', self.presubmit_support_path]
      cmd.extend(args)
      cmd.extend(['--json_output', self.m.json.output()])
      if self.m.resultdb.enabled:
        kwargs['wrapper'] = ('rdb', 'stream', '--')
      step_data = self.m.step(name, cmd, **kwargs)
      output = step_data.json.output or {}
      return output

  @property
  def _relative_root(self):
    if self.m.tryserver.is_tryserver:
      return self.m.gclient.get_gerrit_patch_root().rstrip('/')
    else:
      return self.m.gclient.c.solutions[0].name.rstrip('/')

  def prepare(self, root_solution_revision=None):
    """Sets up a presubmit run.

    This includes:
      - setting up the checkout w/ bot_update
      - locally committing the applied patch
      - running hooks, if requested

    This expects the gclient configuration to already have been set.

    Args:
      root_solution_revision: revision of the root solution

    Returns:
      the StepResult from the bot_update step.
    """
    # Set up the root solution revision by either passing the revision
    # to this function or adding it to the input properties.
    root_solution_revision = (
        root_solution_revision or
        self.m.properties.get('root_solution_revision'))

    # Expect callers to have already set up their gclient configuration.

    bot_update_step = self.m.bot_update.ensure_checkout(
        timeout=3600,
        no_fetch_tags=True,
        root_solution_revision=root_solution_revision)

    abs_root = self.m.context.cwd / self._relative_root
    if self.m.tryserver.is_tryserver:
      with self.m.context(cwd=abs_root):
        # TODO(unowned): Consider either:
        #  - extracting user name & email address from the issue, or
        #  - using a dedicated and clearly nonexistent name/email address
        step_result = self.m.git(
            '-c',
            'user.email=commit-bot@chromium.org',
            '-c',
            'user.name=The Commit Bot',
            '-c',
            'diff.ignoreSubmodules=all',
            'commit',
            '-a',
            '-m',
            'Committed patch',
            name='commit-git-patch',
            raise_on_failure=False,
            stdout=self.m.raw_io.output_text('stdout',
                                             add_output_log='on_failure'),
            infra_step=False,
        )
        if step_result.retcode:
          failure_md_lines = ['Failed to apply patch.']
          if step_result.stdout:
            failure_md_lines += step_result.stdout.splitlines() + ['']
            if 'nothing to commit' in step_result.stdout:
              failure_md_lines.append(
                  'Was an identical diff already submitted elsewhere?')
          raise self.m.step.StepFailure('<br/>'.join(failure_md_lines))

    if self._runhooks:
      with self.m.context(cwd=self.m.path.checkout_dir):
        self.m.gclient.runhooks()

    return bot_update_step

  def execute(self, bot_update_step, skip_owners=False, run_all=False):
    """Runs presubmit and sets summary markdown if applicable.

    Args:
      * bot_update_step: the StepResult from a previously executed bot_update step.
      * skip_owners: a boolean indicating whether Owners checks should be skipped.

    Returns:
      a RawResult object, suitable for being returned from RunSteps.
    """
    abs_root = self.m.context.cwd / self._relative_root
    got_revision_properties = self.m.bot_update.get_project_revision_properties(
        # Replace path.sep with '/', since most recipes are written assuming '/'
        # as the delimiter. This breaks on windows otherwise.
        self._relative_root.replace(self.m.path.sep, '/'),
        self.m.gclient.c)
    upstream = bot_update_step.properties.get(got_revision_properties[0])

    presubmit_args = []
    if self.m.tryserver.is_tryserver:
      presubmit_args = [
          '--issue',
          self.m.tryserver.gerrit_change.change,
          '--patchset',
          self.m.tryserver.gerrit_change.patchset,
          '--gerrit_url',
          'https://%s' % self.m.tryserver.gerrit_change.host,
          '--gerrit_project',
          self.m.tryserver.gerrit_change.project,
          '--gerrit_branch',
          self.m.tryserver.gerrit_change_target_ref,
          '--gerrit_fetch',
      ]

    if run_all:
      presubmit_args.extend([
        '--all', '--no_diffs',
        '--verbose'
        ])

    if self.m.cv.active and self.m.cv.run_mode == self.m.cv.DRY_RUN:
      presubmit_args.append('--dry_run')

    additionalArgs = ['--root', abs_root,'--commit']


    if not run_all:
      additionalArgs.extend([
        '--verbose', '--verbose',
      ])

    additionalArgs.extend([
      '--skip_canned', 'CheckTreeIsOpen',
      '--upstream', upstream,  # '' if not in bot_update mode.
    ])

    presubmit_args.extend(additionalArgs)

    if skip_owners:
      presubmit_args.extend([
        '--skip_canned', 'CheckOwners'
      ])

    raw_result = result_pb2.RawResult()
    step_json = self(
        *presubmit_args,
        timeout=self._timeout_s,
        # ok_ret='any' causes all exceptions to be ignored in this step
        ok_ret='any')
    # Set recipe result values
    if step_json:
      raw_result.summary_markdown = _createSummaryMarkdown(step_json)

    retcode = self.m.step.active_result.retcode
    if retcode == 0:
      raw_result.status = common_pb2.SUCCESS
      return raw_result

    self.m.step.active_result.presentation.status = 'FAILURE'
    if self.m.step.active_result.exc_result.had_timeout:
      raw_result.status = common_pb2.FAILURE
      raw_result.summary_markdown += (
          '\n\nTimeout occurred during presubmit step.')
    elif retcode == 1:
      raw_result.status = common_pb2.FAILURE
      self.m.tryserver.set_test_failure_tryjob_result()
    else:
      raw_result.status = common_pb2.INFRA_FAILURE
      self.m.tryserver.set_invalid_test_results_tryjob_result()
    # Handle unexpected errors not caught by json output
    if raw_result.summary_markdown == '':
      raw_result.status = common_pb2.INFRA_FAILURE
      raw_result.summary_markdown = (
          'Something unexpected occurred'
          ' while running presubmit checks.'
          ' Please [file a bug](https://issues.chromium.org'
          '/issues/new?component=1456211)')
    return raw_result


def _limitSize(message_list, char_limit=450):
  """Returns a list of strings within a certain character length.

  Args:
     * message_list (List[str]) - The message to truncate as a list
       of lines (without line endings).
  """
  hint = ('**The complete output can be'
          ' found at the bottom of the presubmit stdout.**')
  char_count = 0
  for index, message in enumerate(message_list):
    char_count += len(message)
    if char_count > char_limit:
      if index == 0:
        # Show at minimum part of the first error message
        first_message = message_list[index].splitlines()
        return ['\n'.join(
          _limitSize(first_message)
          )
        ]
      total_errors = len(message_list)
      # If code is being cropped, the closing code tag will
      # get removed, so add it back before the hint.
      code_tag = '```'
      message_list[index - 1] = '\n'.join((message_list[index - 1], code_tag))
      oversized_msg = ('\n**Error size > %d chars, '
      'there are %d more error(s) (%d total)**') % (
        char_limit, total_errors - index, total_errors
      )
      return message_list[:index] + [oversized_msg, hint]
  return message_list


def _createSummaryMarkdown(step_json):
  """Returns a string with data on errors, warnings, and notifications.

  Extracts the number of errors, warnings and notifications
  from the dictionary(step_json).

  Then it lists all the errors line by line.

  Args:
      * step_json = {
        'errors': [
          {
            'message': string,
            'long_text': string,
            'items: [string],
            'fatal': boolean
          }
        ],
        'notifications': [
          {
            'message': string,
            'long_text': string,
            'items: [string],
            'fatal': boolean
          }
        ],
        'warnings': [
          {
            'message': string,
            'long_text': string,
            'items: [string],
            'fatal': boolean
          }
        ]
      }
  """
  errors = step_json['errors']
  warning_count = len(step_json['warnings'])
  notif_count = len(step_json['notifications'])
  description = (
      f'#### There are {len(errors)} error(s), {warning_count} warning(s), '
      f'and {notif_count} notifications(s).')
  error_messages = []

  for error in errors:
    error_messages.append(
      '**ERROR**\n```\n%s\n%s\n```' % (
      error['message'], error['long_text'])
    )

  error_messages = _limitSize(error_messages)
  # Description is not counted in the total message size.
  # It is inserted afterward to ensure it is the first message seen.
  error_messages.insert(0, description)
  if warning_count or notif_count:
    error_messages.append(
      ('#### To see notifications and warnings,'
      ' look at the stdout of the presubmit step.')
    )
  return '\n\n'.join(error_messages)
