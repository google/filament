# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re

from recipe_engine import recipe_api

class DepsDiffException(Exception):
  pass

class RevisionResolver(object):
  """Resolves the revision based on build properties."""

  def resolve(self, properties):  # pragma: no cover
    raise NotImplementedError()


class RevisionFallbackChain(RevisionResolver):
  """Specify that a given project's sync revision follows the fallback
  chain."""

  def __init__(self, default=None):
    self._default = default

  def resolve(self, properties):
    """Resolves the revision via the revision fallback chain.

    If the given revision was set using the revision_fallback_chain() function,
    this function will follow the chain, looking at relevant build properties
    until it finds one set or reaches the end of the chain and returns the
    default. If the given revision was not set using revision_fallback_chain(),
    this function just returns it as-is.
    """
    return (properties.get('parent_got_revision') or
            properties.get('orig_revision') or
            properties.get('revision') or
            self._default)


def jsonish_to_python(spec, is_top=False):
  """Turns a json spec into a python parsable object.

  This exists because Gclient specs, while resembling json, is actually
  ingested using a python "eval()".  Therefore a bit of plumming is required
  to turn our newly constructed Gclient spec into a gclient-readable spec.
  """
  ret = ''
  if is_top:  # We're the 'top' level, so treat this dict as a suite.
    ret = '\n'.join(
      '%s = %s' % (k, jsonish_to_python(spec[k])) for k in sorted(spec)
    )
  else:
    if isinstance(spec, dict):
      ret += '{'
      ret += ', '.join(
        "%s: %s" % (repr(str(k)), jsonish_to_python(spec[k]))
        for k in sorted(spec)
      )
      ret += '}'
    elif isinstance(spec, list):
      ret += '['
      ret += ', '.join(jsonish_to_python(x) for x in spec)
      ret += ']'
    elif isinstance(spec, str):
      ret = repr(str(spec))
    else:
      ret = repr(spec)
  return ret

class GclientApi(recipe_api.RecipeApi):
  # Singleton object to indicate to checkout() that we should run a revert if
  # we detect that we're on the tryserver.
  RevertOnTryserver = object()

  def __init__(self, **kwargs):
    super(GclientApi, self).__init__(**kwargs)
    self.USE_MIRROR = None
    self._spec_alias = None

  def __call__(self, name, cmd, infra_step=True, **kwargs):
    """Wrapper for easy calling of gclient steps."""
    assert isinstance(cmd, (list, tuple))
    prefix = 'gclient '
    if self.spec_alias:
      prefix = ('[spec: %s] ' % self.spec_alias) + prefix

    env_suffixes = {'PATH': [self.repo_resource()]}
    env = {}
    if self.m.buildbucket.build.id != 0:
      env['DEPOT_TOOLS_REPORT_BUILD'] = '%s/%s/%s/%s' % (
          self.m.buildbucket.build.builder.project,
          self.m.buildbucket.build.builder.bucket,
          self.m.buildbucket.build.builder.builder,
          self.m.buildbucket.build.id)
    with self.m.context(env=env, env_suffixes=env_suffixes):
      cmd = ['vpython3', '-u', self.repo_resource('gclient.py')] + cmd
      return self.m.step(prefix + name,
                         cmd,
                         infra_step=infra_step,
                         **kwargs)

  @property
  def use_mirror(self):
    """Indicates if gclient will use mirrors in its configuration."""
    if self.USE_MIRROR is None:
      self.USE_MIRROR = self.m.properties.get('use_mirror', True)
    return self.USE_MIRROR

  @use_mirror.setter
  def use_mirror(self, val):  # pragma: no cover
    self.USE_MIRROR = val

  @property
  def spec_alias(self):
    """Optional name for the current spec for step naming."""
    return self._spec_alias

  @spec_alias.setter
  def spec_alias(self, name):
    self._spec_alias = name

  @spec_alias.deleter
  def spec_alias(self):
    self._spec_alias = None

  def get_config_defaults(self):
    return {
      'USE_MIRROR': self.use_mirror,
      'CACHE_DIR': self.m.path.cache_dir / 'git',
    }

  @staticmethod
  def config_to_pythonish(cfg):
    return jsonish_to_python(cfg.as_jsonish(), True)

  # TODO(machenbach): Remove this method when the old mapping is deprecated.
  @staticmethod
  def got_revision_reverse_mapping(cfg):
    """Returns the merged got_revision_reverse_mapping.

    Returns (dict): A mapping from property name -> project name. It merges the
        values of the deprecated got_revision_mapping and the new
        got_revision_reverse_mapping.
    """
    rev_map = cfg.got_revision_mapping.as_jsonish()
    reverse_rev_map = cfg.got_revision_reverse_mapping.as_jsonish()
    combined_length = len(rev_map) + len(reverse_rev_map)
    reverse_rev_map.update({v: k for k, v in rev_map.items()})

    # Make sure we never have duplicate values in the old map.
    assert combined_length == len(reverse_rev_map)
    return reverse_rev_map

  def resolve_revision(self, revision):
    if hasattr(revision, 'resolve'):
      return revision.resolve(self.m.properties)
    return revision

  def sync(self, cfg, extra_sync_flags=None, **kwargs):
    revisions = []
    self.set_patch_repo_revision(gclient_config=cfg)
    for i, s in enumerate(cfg.solutions):
      if i == 0 and s.revision is None:
        s.revision = RevisionFallbackChain()

      if s.revision is not None and s.revision != '':
        fixed_revision = self.resolve_revision(s.revision)
        if fixed_revision:
          revisions.extend(['--revision', '%s@%s' % (s.name, fixed_revision)])

    for name, revision in sorted(cfg.revisions.items()):
      fixed_revision = self.resolve_revision(revision)
      if fixed_revision:
        revisions.extend(['--revision', '%s@%s' % (name, fixed_revision)])

    test_data_paths = set(
        list(self.got_revision_reverse_mapping(cfg).values()) +
        [s.name for s in cfg.solutions])
    step_test_data = lambda: (
      self.test_api.output_json(test_data_paths))
    try:
      # clean() isn't used because the gclient sync flags passed in checkout()
      # do much the same thing, and they're more correct than doing a separate
      # 'gclient revert' because it makes sure the other args are correct when
      # a repo was deleted and needs to be re-cloned (notably
      # --with_branch_heads), whereas 'revert' uses default args for clone
      # operations.
      #
      # TODO(mmoss): To be like current official builders, this step could
      # just delete the whole <slave_name>/build/ directory and start each
      # build from scratch. That might be the least bad solution, at least
      # until we have a reliable gclient method to produce a pristine working
      # dir for git-based builds (e.g. maybe some combination of 'git
      # reset/clean -fx' and removing the 'out' directory).
      j = '-j2' if self.m.platform.is_win else '-j8'
      args = ['sync', '--verbose', '--nohooks', j, '--reset', '--force',
              '--upstream', '--no-nag-max', '--with_branch_heads',
              '--with_tags']
      args.extend(extra_sync_flags or [])
      if cfg.delete_unversioned_trees:
        args.append('--delete_unversioned_trees')
      self('sync', args + revisions +
                 ['--output-json', self.m.json.output()],
                 step_test_data=step_test_data,
                 **kwargs)
    finally:
      result = self.m.step.active_result
      if result.json.output is not None:
        solutions = result.json.output['solutions']
        for propname, path in sorted(
            self.got_revision_reverse_mapping(cfg).items()):
          # gclient json paths always end with a slash
          sol = solutions.get(path + '/') or solutions.get(path)
          # solution can exist with `revision == null`. We only want to include
          # properties that have set revision.
          if sol and sol['revision']:
            result.presentation.properties[propname] = sol['revision']

    return result

  def inject_parent_got_revision(self, gclient_config=None, override=False):
    """Match gclient config to build revisions obtained from build_properties.

    Args:
      gclient_config (gclient config object) - The config to manipulate. A value
        of None manipulates the module's built-in config (self.c).
      override (bool) - If True, will forcibly set revision and custom_vars
        even if the config already contains values for them.
    """
    cfg = gclient_config or self.c

    for prop, custom_var in cfg.parent_got_revision_mapping.items():
      val = str(self.m.properties.get(prop, ''))
      # TODO(infra): Fix coverage.
      if val:  # pragma: no cover
        # Special case for 'src', inject into solutions[0]
        if custom_var is None:
          # This is not covered because we are deprecating this feature and
          # it is no longer used by the public recipes.
          if cfg.solutions[0].revision is None or override:  # pragma: no cover
            cfg.solutions[0].revision = val
        else:
          if custom_var not in cfg.solutions[0].custom_vars or override:
            cfg.solutions[0].custom_vars[custom_var] = val

  def checkout(self, gclient_config=None, revert=RevertOnTryserver,
               inject_parent_got_revision=True, extra_sync_flags=None,
               **kwargs):
    """Return a step generator function for gclient checkouts."""
    cfg = gclient_config or self.c
    assert cfg.complete()

    if revert is self.RevertOnTryserver:
      revert = self.m.tryserver.is_tryserver

    if inject_parent_got_revision:
      self.inject_parent_got_revision(cfg, override=True)

    self('setup', ['config', '--spec', self.config_to_pythonish(cfg)], **kwargs)

    sync_step = None
    try:
      sync_step = self.sync(cfg, extra_sync_flags=extra_sync_flags, **kwargs)

      cfg_cmds = [
        ('user.name', 'local_bot'),
        ('user.email', 'local_bot@example.com'),
      ]
      for var, val in cfg_cmds:
        name = 'recurse (git config %s)' % var
        self(name, ['recurse', 'git', 'config', var, val], **kwargs)
    finally:
      cwd = self.m.context.cwd or self.m.path.start_dir
      if 'checkout' not in self.m.path:
        self.m.path.checkout_dir = cwd.joinpath(
          *cfg.solutions[0].name.split(self.m.path.sep))

    return sync_step

  def runhooks(self, args=None, name='runhooks', **kwargs):
    args = args or []
    assert isinstance(args, (list, tuple))
    with self.m.context(cwd=(self.m.context.cwd or self.m.path.checkout_dir)):
      return self(name, ['runhooks'] + list(args), infra_step=False, **kwargs)

  def break_locks(self):
    """Remove all index.lock files. If a previous run of git crashed, bot was
    reset, etc... we might end up with leftover index.lock files.
    """
    cmd = ['python3', '-u', self.resource('cleanup.py'), self.m.path.start_dir]
    return self.m.step('cleanup index.lock', cmd)

  def get_gerrit_patch_root(self, gclient_config=None):
    """Returns local path to the repo where gerrit patch will be applied.

    If there is no patch, returns None.
    If patch is specified, but such repo is not found among configured solutions
    or repo_path_map, returns name of the first solution. This is done solely
    for backward compatibility with existing tests.
    Please do not rely on this logic in new code.
    Instead, properly map a repository to a local path using repo_path_map.
    TODO(nodir): remove this. Update all recipe tests to specify a git_repo
    matching the recipe.
    """
    cfg = gclient_config or self.c
    repo_url = self.m.tryserver.gerrit_change_repo_url
    if not repo_url:
      return None
    root =  self.get_repo_path(repo_url, gclient_config=cfg)

    # This is wrong, but that's what a ton of recipe tests expect today
    root = root or cfg.solutions[0].name

    return root

  def _canonicalize_repo_url(self, repo_url):
    """Attempts to make repo_url canonical. Supports Gitiles URL."""
    return self.m.gitiles.canonicalize_repo_url(repo_url)

  def get_repo_path(self, repo_url, gclient_config=None):
    """Returns local path to the repo checkout given its url.

    Consults cfg.repo_path_map and fallbacks to urls in configured solutions.

    Returns None if not found.
    """
    rel_path = self._get_repo_path(repo_url, gclient_config=gclient_config)
    if rel_path:
      return self.m.path.join(*rel_path.split('/'))
    return None

  def _get_repo_path(self, repo_url, gclient_config=None):
    repo_url = self._canonicalize_repo_url(repo_url)
    cfg = gclient_config or self.c
    rel_path, _ = cfg.repo_path_map.get(repo_url, ('', ''))
    if rel_path:
      return rel_path

    # repo_path_map keys may be non-canonical.
    for key, (rel_path, _) in cfg.repo_path_map.items():
      if self._canonicalize_repo_url(key) == repo_url:
        return rel_path

    for s in cfg.solutions:
      if self._canonicalize_repo_url(s.url) == repo_url:
        return s.name

    return None

  def set_patch_repo_revision(self, gclient_config=None):
    """Updates config revision corresponding to patched project.

    Useful for bot_update only, as this is the only consumer of gclient's config
    revision map. This doesn't overwrite the revision if it was already set.
    """
    cfg = gclient_config or self.c
    repo_url = self.m.tryserver.gerrit_change_repo_url
    path, revision = cfg.repo_path_map.get(repo_url, (None, None))
    if path and revision and path not in cfg.revisions:
      cfg.revisions[path] = revision

  def diff_deps(self, cwd):
    with self.m.context(cwd=cwd):
      step_result = self.m.git(
          '-c',
          'core.quotePath=false',
          'checkout',
          'HEAD~',
          '--',
          'DEPS',
          name='checkout the previous DEPS',
          stdout=self.m.raw_io.output()
      )

      try:
        cfg = self.c

        step_result = self(
            'recursively git diff all DEPS',
            ['recurse', 'python3', self.resource('diff_deps.py')],
            stdout=self.m.raw_io.output_text(add_output_log=True),
        )

        paths = []
        # gclient recurse prepends a number and a > to each line
        # Let's take that out
        for line in step_result.stdout.strip().splitlines():
          if 'fatal: bad object' in line:
            msg = "Couldn't checkout previous ref: %s" % line
            step_result.presentation.logs['DepsDiffException'] = msg
            raise self.DepsDiffException(msg)
          elif re.match('\d+>', line):
            paths.append(line[line.index('>') + 1:])


        # Normalize paths
        if self.m.platform.is_win:
          # Looks like "analyze" wants POSIX slashes even on Windows (since git
          # uses that format even on Windows).
          paths = [path.replace('\\', '/') for path in paths]

        if len(paths) > 0:
          return paths
        else:
          msg = 'Unexpected result: autoroll diff found 0 files changed'
          step_result.presentation.logs['DepsDiffException'] = msg
          raise self.DepsDiffException(msg)

      finally:
        self.m.git(
            '-c',
            'core.quotePath=false',
            'checkout',
            'HEAD',
            '--',
            'DEPS',
            name="checkout the original DEPS")

  @property
  def DepsDiffException(self):
    return DepsDiffException

  def roll_deps(self,
                deps_path,
                dep_updates,
                strip_prefix_for_gitlink=None,
                test_data=None):
    """Updates DEPS file to desired revisions, and returns all requried file
    changes.

    Args:
      deps_path - Path to DEPS file that will be modified.
      dep_updates - A map of dependencies to update (key = dependency name,
                    value = revision).
      strip_prefix_for_gitlink - Prefix that will be removed when adding
                                 gitlinks. This is only useful for repositories
                                 that use use_relative_path = True. That's
                                 currently only chromium/src.

    Returns:
      A map of all files that need to be modified (key = file path, value = file
      content) in addition to DEPS file itself.
      Note: that git submodules (gitlinks) are treated as files and content is a
      commit hash.
      Note: we expect DEPS to be in the root of the project.
    """
    deps_content = self.m.file.read_text('Read DEPS file', deps_path, test_data)
    update_gitlink = False
    dep_updates_args = []
    file_changes = {}

    lines = deps_content.split('\n')
    for line in lines:
      if line.startswith('git_dependencies = '):
        if 'DEPS' not in line:
          # Need to update gitlinks
          update_gitlink = True
        break

    for dep, rev in dep_updates.items():
      dep_updates_args.extend(['-r', f'{dep}@{rev}'])
      if update_gitlink:
        # Add gitlink updates to file changes.
        gitlink_path = dep
        if strip_prefix_for_gitlink and \
            gitlink_path.startswith(strip_prefix_for_gitlink):
          # strip src/ from path
          gitlink_path = gitlink_path[len(strip_prefix_for_gitlink):]

        file_changes[gitlink_path] = rev.encode('UTF-8')
    # Apply the updates to the local DEPS files.
    self.m.gclient('setdep',
                   ['setdep', '--deps-file', deps_path] + dep_updates_args)

    updated_deps = self.m.file.read_raw('Read modified DEPS', deps_path)
    file_changes['DEPS'] = updated_deps
    return file_changes
