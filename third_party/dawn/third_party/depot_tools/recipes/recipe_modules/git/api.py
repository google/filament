# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import itertools
import re

from recipe_engine import recipe_api

class GitApi(recipe_api.RecipeApi):
  _GIT_HASH_RE = re.compile('[0-9a-f]{40}', re.IGNORECASE)

  def __call__(self, *args, **kwargs):
    """Returns a git command step."""
    name = kwargs.pop('name', 'git ' + args[0])

    infra_step = kwargs.pop('infra_step', True)
    git_cmd = ['git']

    options = kwargs.pop('git_config_options', {})
    for k, v in sorted(options.items()):
      git_cmd.extend(['-c', '%s=%s' % (k, v)])
    with self.m.context(cwd=(self.m.context.cwd or self.m.path.checkout_dir)):
      return self.m.step(name, git_cmd + list(args), infra_step=infra_step,
                          **kwargs)

  def fetch_tags(self, remote_name=None, **kwargs):
    """Fetches all tags from the remote."""
    kwargs.setdefault('name', 'git fetch tags')
    remote_name = remote_name or 'origin'
    return self('fetch', remote_name, '--tags', **kwargs)

  def cat_file_at_commit(self, file_path, commit_hash, remote_name=None,
                         **kwargs):
    """Outputs the contents of a file at a given revision."""
    self.fetch_tags(remote_name=remote_name, **kwargs)
    kwargs.setdefault('name', 'git cat-file %s:%s' % (commit_hash, file_path))
    return self('cat-file', 'blob', '%s:%s' % (commit_hash, file_path),
                **kwargs)

  def count_objects(self, previous_result=None, raise_on_failure=False, **kwargs):
    """Returns `git count-objects` result as a dict.

    Args:
      * previous_result (dict): the result of previous count_objects call.
        If passed, delta is reported in the log and step text.
      * raise_on_failure (bool): if True, an exception will be raised if the
        operation fails. Defaults to False.

    Returns:
      A dict of count-object values, or None if count-object run failed.
    """
    if previous_result:
      assert isinstance(previous_result, dict)
      assert all(isinstance(v, int) for v in previous_result.values())
      assert 'size' in previous_result
      assert 'size-pack' in previous_result

    step_result = None
    try:
      step_result = self(
          'count-objects', '-v', stdout=self.m.raw_io.output(),
          raise_on_failure=raise_on_failure, **kwargs)

      if not step_result.stdout:
        return None

      result = {}
      for line in step_result.stdout.splitlines():
        line = line.decode('utf-8')
        name, value = line.split(':', 1)
        result[name] = int(value.strip())

      def results_to_text(results):
        return ['  %s: %s' % (k, v) for k, v in sorted(results.items())]

      step_result.presentation.logs['result'] = results_to_text(result)

      if previous_result:
        delta = {
            key: value - previous_result[key]
            for key, value in result.items()
            if key in previous_result}
        step_result.presentation.logs['delta'] = (
            ['before:'] + results_to_text(previous_result) +
            ['', 'after:'] + results_to_text(result) +
            ['', 'delta:'] + results_to_text(delta)
        )

        size_delta = (
            result['size'] + result['size-pack']
            - previous_result['size'] - previous_result['size-pack'])
        # size_delta is in KiB.
        step_result.presentation.step_text = (
            'size delta: %+.2f MiB' % (size_delta / 1024.0))

      return result
    except Exception as ex:
      if step_result:
        step_result.presentation.logs['exception'] = repr(ex)
        step_result.presentation.status = self.m.step.WARNING
      if raise_on_failure:
        raise recipe_api.InfraFailure('count-objects failed: %s' % ex)
      return None

  def checkout(self,
               url,
               ref=None,
               dir_path=None,
               recursive=False,
               submodules=True,
               submodule_update_force=False,
               keep_paths=None,
               step_suffix=None,
               curl_trace_file=None,
               raise_on_failure=True,
               set_got_revision=False,
               remote_name=None,
               display_fetch_size=None,
               file_name=None,
               submodule_update_recursive=True,
               use_git_cache=False,
               progress=True,
               tags=False,
               depth=None):
    """Performs a full git checkout and returns sha1 of checked out revision.

    Args:
      * url (str): url of remote repo to use as upstream
      * ref (str): ref to fetch and check out
      * dir_path (Path): optional directory to clone into
      * recursive (bool): whether to recursively fetch submodules or not
      * submodules (bool): whether to sync and update submodules or not
      * submodule_update_force (bool): whether to update submodules with --force
      * keep_paths (iterable of strings): paths to ignore during git-clean;
          paths are gitignore-style patterns relative to checkout_path.
      * step_suffix (str): suffix to add to a each step name
      * curl_trace_file (Path): if not None, dump GIT_CURL_VERBOSE=1 trace to that
          file. Useful for debugging git issue reproducible only on bots. It has
          a side effect of all stderr output of 'git fetch' going to that file.
      * raise_on_failure (bool): if False, ignore errors during fetch or checkout.
      * set_got_revision (bool): if True, resolves HEAD and sets got_revision
          property.
      * remote_name (str): name of the git remote to use
      * display_fetch_size (bool): if True, run `git count-objects` before and
        after fetch and display delta. Adds two more steps. Defaults to False.
      * file_name (str): optional path to a single file to checkout.
      * submodule_update_recursive (bool): if True, updates submodules
          recursively.
      * use_git_cache (bool): if True, git cache will be used for this checkout.
          WARNING, this is EXPERIMENTAL!!! This wasn't tested with:
           * submodules
           * since origin url is modified
             to a local path, may cause problem with scripts that do
             "git fetch origin" or "git push origin".
           * arbitrary refs such refs/whatever/not-fetched-by-default-to-cache
       progress (bool): whether to show progress for fetch or not
      * tags (bool): Also fetch tags.
      * depth (int): if > 0, limit fetching to the given number of commits from
        the tips of the remote tree.

    Returns: If the checkout was successful, this returns the commit hash of
      the checked-out-repo. Otherwise this returns None.
    """
    retVal = None

    # TODO(robertocn): Break this function and refactor calls to it.
    #     The problem is that there are way too many unrealated use cases for
    #     it, and the function's signature is getting unwieldy and its body
    #     unreadable.
    display_fetch_size = display_fetch_size or False
    if not dir_path:
      dir_path = url.rsplit('/', 1)[-1]
      if dir_path.endswith('.git'):  # ex: https://host/foobar.git
        dir_path = dir_path[:-len('.git')]

      # ex: ssh://host:repo/foobar/.git
      dir_path = dir_path or dir_path.rsplit('/', 1)[-1]

      dir_path = self.m.path.start_dir / dir_path

    if 'checkout' not in self.m.path:
      self.m.path.checkout_dir = dir_path

    git_setup_args = ['--path', dir_path, '--url', url]

    if remote_name:
      git_setup_args += ['--remote', remote_name]
    else:
      remote_name = 'origin'

    step_suffix = '' if step_suffix is  None else ' (%s)' % step_suffix
    self.m.step(
        'git setup%s' % step_suffix,
        ['python3', '-u', self.resource('git_setup.py')] + git_setup_args)

    # Some of the commands below require depot_tools to be in PATH.
    path = self.m.path.pathsep.join([
        str(self.repo_resource()), '%(PATH)s'])

    with self.m.context(cwd=dir_path):
      if use_git_cache:
        with self.m.context(env={'PATH': path}):
          self('cache', 'populate', '-c',
               self.m.path.cache_dir / 'git', url,
               name='populate cache',
               raise_on_failure=raise_on_failure)
          dir_cmd = self(
              'cache', 'exists', '--quiet',
              '--cache-dir', self.m.path.cache_dir / 'git', url,
              raise_on_failure=raise_on_failure,
              stdout=self.m.raw_io.output(),
              step_test_data=lambda:
                  self.m.raw_io.test_api.stream_output('mirror_dir'))
          mirror_dir = dir_cmd.stdout.strip().decode('utf-8')
          self('remote', 'set-url', 'origin', mirror_dir,
               raise_on_failure=raise_on_failure)

      # There are five kinds of refs we can be handed:
      # 0) None. In this case, we default to api.buildbucket.gitiles_commit.ref.
      # 1) A fully qualified branch name, e.g. 'refs/heads/main'.
      #    Chop off 'refs/heads/' and now it matches case (4).
      # 2) A 40-character SHA1 hash.
      # 3) A fully-qualifed arbitrary ref, e.g. 'refs/foo/bar/baz'.
      # 4) A branch name, e.g. 'main'.
      # Note that 'FETCH_HEAD' can be many things (and therefore not a valid
      # checkout target) if many refs are fetched, but we only explicitly fetch
      # one ref here, so this is safe.
      if not ref:                                  # Case 0.
        ref = self.m.buildbucket.gitiles_commit.ref or 'main'

      # If it's a fully-qualified branch name, trim the 'refs/heads/' prefix.
      if ref.startswith('refs/heads/'):            # Case 1.
        ref = ref[len('refs/heads/'):]

      fetch_args = []
      if self._GIT_HASH_RE.match(ref):             # Case 2.
        fetch_remote = remote_name
        fetch_ref = ''
        checkout_ref = ref
      else:                                        # Cases 3 and 4.
        fetch_remote = remote_name
        fetch_ref = ref
        checkout_ref = 'FETCH_HEAD'

      fetch_args = [x for x in (fetch_remote, fetch_ref) if x]
      if recursive:
        fetch_args.append('--recurse-submodules')

      if progress:
        fetch_args.append('--progress')

      fetch_env = {'PATH': path}
      fetch_stderr = None
      if curl_trace_file:
        fetch_env['GIT_CURL_VERBOSE'] = '1'
        fetch_stderr = self.m.raw_io.output(leak_to=curl_trace_file)

      if tags:
        fetch_args.append('--tags')

      if depth:
        assert isinstance(depth, int)
        fetch_args += ['--depth', depth]

      fetch_step_name = 'git fetch%s' % step_suffix
      if display_fetch_size:
        count_objects_before_fetch = self.count_objects(
            name='count-objects before %s' % fetch_step_name,
            step_test_data=lambda: self.m.raw_io.test_api.stream_output(
                self.test_api.count_objects_output(1000)))
      with self.m.context(env=fetch_env):
        self('fetch', *fetch_args,
          name=fetch_step_name,
          stderr=fetch_stderr,
          raise_on_failure=raise_on_failure)
      if display_fetch_size:
        self.count_objects(
            name='count-objects after %s' % fetch_step_name,
            previous_result=count_objects_before_fetch,
            step_test_data=lambda: self.m.raw_io.test_api.stream_output(
                self.test_api.count_objects_output(2000)))

      if file_name:
        self('checkout', '-f', checkout_ref, '--', file_name,
          name='git checkout%s' % step_suffix,
          raise_on_failure=raise_on_failure)

      else:
        self('checkout', '-f', checkout_ref,
          name='git checkout%s' % step_suffix,
          raise_on_failure=raise_on_failure)

      rev_parse_step = self('rev-parse', 'HEAD',
                           name='read revision',
                           stdout=self.m.raw_io.output_text(),
                           raise_on_failure=False,
                           step_test_data=lambda:
                              self.m.raw_io.test_api.stream_output_text('deadbeef'))

      if rev_parse_step.presentation.status == 'SUCCESS':
        sha = rev_parse_step.stdout.strip()
        retVal = sha
        rev_parse_step.presentation.step_text = "<br/>checked out %r<br/>" % sha
        if set_got_revision:
          rev_parse_step.presentation.properties['got_revision'] = sha

      clean_args = list(itertools.chain(
          *[('-e', path) for path in keep_paths or []]))

      self('clean', '-f', '-d', '-x', *clean_args,
        name='git clean%s' % step_suffix,
        raise_on_failure=raise_on_failure)

      if submodules:
        self('submodule', 'sync',
          name='submodule sync%s' % step_suffix,
          raise_on_failure=raise_on_failure)
        submodule_update = ['submodule', 'update', '--init']
        if submodule_update_recursive:
          submodule_update.append('--recursive')
        if submodule_update_force:
          submodule_update.append('--force')
        self(*submodule_update,
          name='submodule update%s' % step_suffix,
          raise_on_failure=raise_on_failure)

    return retVal

  def get_timestamp(self, commit='HEAD', test_data=None, **kwargs):
    """Find and return the timestamp of the given commit."""
    step_test_data = None
    if test_data is not None:
      step_test_data = lambda: self.m.raw_io.test_api.stream_output(test_data)
    return self('show', commit, '--format=%at', '-s',
                stdout=self.m.raw_io.output(),
                step_test_data=step_test_data).stdout.rstrip().decode('utf-8')

  def rebase(self, name_prefix, branch, dir_path, remote_name=None,
             **kwargs):
    """Runs rebase HEAD onto branch

    Args:
      * name_prefix (str): a prefix used for the step names
      * branch (str): a branch name or a hash to rebase onto
      * dir_path (Path): directory to clone into
      * remote_name (str): the remote name to rebase from if not origin
    """
    remote_name = remote_name or 'origin'
    with self.m.context(cwd=dir_path):
      try:
        self('rebase', '%s/main' % remote_name,
             name="%s rebase" % name_prefix, **kwargs)
      except self.m.step.StepFailure:
        self('rebase', '--abort', name='%s rebase abort' % name_prefix,
             **kwargs)
        raise

  def config_get(self, prop_name, **kwargs):
    """Returns git config output.

    Args:
      * prop_name: (str) The name of the config property to query.
      * kwargs: Forwarded to '__call__'.

    Returns: (str) The Git config output, or None if no output was generated.
    """
    kwargs['name'] = kwargs.get('name', 'git config %s' % (prop_name,))
    result = self('config', '--get', prop_name, stdout=self.m.raw_io.output(),
                  **kwargs)

    value = result.stdout
    if value:
      value = value.strip()
      result.presentation.step_text = value.decode('utf-8')
    return value

  def get_remote_url(self, remote_name=None, **kwargs):
    """Returns the remote Git repository URL, or None.

    Args:
      * remote_name: (str) The name of the remote to query, defaults to 'origin'.
      * kwargs: Forwarded to '__call__'.

    Returns: (str) The URL of the remote Git repository, or None.
    """
    remote_name = remote_name or 'origin'
    return self.config_get('remote.%s.url' % (remote_name,), **kwargs)

  def bundle_create(self, bundle_path, rev_list_args=None, **kwargs):
    """Runs 'git bundle create' on a Git repository.

    Args:
      * bundle_path (Path): The path of the output bundle.
      * refs (list): The list of refs to include in the bundle. If None, all
          refs in the Git checkout will be bundled.
      * kwargs: Forwarded to '__call__'.
    """
    if not rev_list_args:
      rev_list_args = ['--all']
    self('bundle', 'create', bundle_path, *rev_list_args, **kwargs)

  def new_branch(self,
                 branch,
                 name=None,
                 upstream=None,
                 upstream_current=False,
                 **kwargs):
    """Runs git new-branch on a Git repository, to be used before git cl
    upload.

    Args:
      * branch (str): new branch name, which must not yet exist.
      * name (str): step name.
      * upstream (str): to origin/main.
      * upstream_current (bool): whether to use '--upstream_current'.
      * kwargs: Forwarded to '__call__'.
    """
    if upstream and upstream_current:
      raise ValueError('Can not define both upstream and upstream_current')
    env = self.m.context.env
    env['PATH'] = self.m.path.pathsep.join([
        str(self.repo_resource()), '%(PATH)s'])
    args = ['new-branch', branch]
    if upstream:
      args.extend(['--upstream', upstream])
    if upstream_current:
      args.append('--upstream_current')
    if not name:
      name = 'git new-branch %s' % branch
    with self.m.context(env=env):
      return self(*args, name=name, **kwargs)

  def number(self, commitrefs=None, test_values=None):
    """Computes the generation number of some commits.

    Args:
      * commitrefs (list[str]): A list of commit references. If none are
        provided, the generation number for HEAD will be retrieved.
      * test_values (list[str]): A list of numbers to use as the return
        value during tests. It is an error if the length of the list
        does not match the number of commitrefs (1 if commitrefs is not
        provided).

    Returns:
    A list of strings containing the generation numbers of the commits.
    If non-empty commitrefs was provided, the order of the returned
    numbers will correspond to the order of the provided commitrefs.
    """

    def step_test_data():
      refs = commitrefs or ['HEAD']
      if test_values:
        assert len(test_values) == len(refs)
      values = test_values or range(3000, 3000 + len(refs))
      output = '\n'.join(str(v) for v in values)
      return self.m.raw_io.test_api.stream_output_text(output)

    args = ['number']
    args.extend(commitrefs or [])
    # Put depot_tools on the path so that git-number can be found
    with self.m.depot_tools.on_path():
      # git-number is only meant for use on bots, so it prints an error message
      # if CHROME_HEADLESS is not set
      with self.m.context(env={'CHROME_HEADLESS': '1'}):
        step_result = self(*args,
                          stdout=self.m.raw_io.output_text(add_output_log=True),
                          step_test_data=step_test_data)
    return [l.strip() for l in step_result.stdout.strip().splitlines()]

  def ls_remote(self, url, ref, name=None, **kwargs):
    """Request the head revision for a given ref using ls-remote. Raise a
    StepFailure if the ref does not exist, or more than one ref was found.

    Args:
      * url (str): url of remote repo to use as upstream.
      * ref (str): ref to query head revision.
      * name (str):  Name of the infra step.

    Returns: A git revision.
    """
    cwd = self.m.context.cwd or self.m.path.start_dir
    name = name or f'Retrieve revision for {ref}'
    cmd = ['ls-remote', url, ref]

    with self.m.context(cwd):
      result = self(*cmd,
                    name=name,
                    stdout=self.m.raw_io.output_text(),
                    **kwargs)
      lines = result.stdout.strip().splitlines()

    if len(lines) > 1:
      raise self.m.step.StepFailure(f'Multiple remote refs found for {ref}')

    if not lines:
      raise self.m.step.StepFailure(f'No remote ref found for {ref}')

    return lines[0].split('\t')[0]
