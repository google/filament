# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""SCM-specific utility classes."""

from __future__ import annotations

import abc
import contextlib
import os
import pathlib
import platform
import re
import threading

from collections import defaultdict
from itertools import chain
from typing import Any
from typing import Collection, Iterable, Iterator, Literal, Dict
from typing import Optional, Sequence, Mapping

import gclient_utils
import git_common
import subprocess2

# TODO: Should fix these warnings.
# pylint: disable=line-too-long

# constants used to identify the tree state of a directory.
VERSIONED_NO = 0
VERSIONED_DIR = 1
VERSIONED_SUBMODULE = 2


def determine_scm(root):
    """Similar to upload.py's version but much simpler.

    Returns 'git' or 'diff'.
    """
    if os.path.isdir(os.path.join(root, '.git')):
        return 'git'

    try:
        subprocess2.check_call(['git', 'rev-parse', '--show-cdup'],
                               stdout=subprocess2.DEVNULL,
                               stderr=subprocess2.DEVNULL,
                               cwd=root)
        return 'git'
    except (OSError, subprocess2.CalledProcessError):
        return 'diff'


GitConfigScope = Literal['system', 'global', 'local', 'worktree']
GitScopeOrder: list[GitConfigScope] = ['system', 'global', 'local', 'worktree']
GitFlatConfigData = Mapping[str, Mapping[str, Sequence[str]]]


class GitConfigStateBase(metaclass=abc.ABCMeta):
    """GitConfigStateBase is the abstract base class for implementations of
    CachedGitConfigState.

    This is meant to model the manipulation of some underlying config data.

    In GitConfigStateReal, this is modeled using `git config` commands in
    a specific git repo.

    In GitConfigStateTest, this is modeled using a set of GitConfigScope-indexed
    dictionaries.

    Implementations MUST ensure that all keys returned in load_config are
    already canonicalized, and implementations MUST accept non-canonical keys to
    set_* and unset_* methods.
    """

    @abc.abstractmethod
    def load_config(self) -> GitFlatConfigData:
        """When invoked, this should return the full state of the configuration
        observable.

        The caller must not mutate the returned value.

        Implementations MUST ensure that all keys returned in load_config are
        already canonicalized.
        """

    @abc.abstractmethod
    def set_config(self, key: str, value: str, *, append: bool,
                   scope: GitConfigScope):
        """When invoked, this should set `key` to a singluar `value` in the git
        scope `scope` in this state's underlying data.

        If `append` is True, this should add an additional value to the existing
        `key`, if any.

        Implementations MUST accept non-canonical `key` values.
        """

    @abc.abstractmethod
    def set_config_multi(self, key: str, value: str, *,
                         value_pattern: Optional[str], scope: GitConfigScope):
        """When invoked, this should replace all existing values of `key` with
        `value` in the git scope `scope` in this state's underlying data.

        If `value_pattern` is supplied, only existing values matching this
        pattern will be replaced.

        TODO: Make value_pattern an re.Pattern. This wasn't done at the time of
        this refactor to keep the refactor small.

        Implementations MUST accept non-canonical `key` values.
        """

    @abc.abstractmethod
    def unset_config(self, key: str, *, scope: GitConfigScope,
                     missing_ok: bool):
        """When invoked, remove a singlar value from `key` in this state's underlying data.

        If missing_ok is False and `key` is not present in the given scope, this
        must raise GitConfigUnsetMissingValue with `key` and `scope`.

        If `key` is multi-valued in this scope, this must raise
        GitConfigUnsetMultipleValues with `key` and `scope`.

        Implementations MUST accept non-canonical `key` values.
        """

    @abc.abstractmethod
    def unset_config_multi(self, key: str, *, value_pattern: Optional[str],
                           scope: GitConfigScope, missing_ok: bool):
        """When invoked, remove all values from `key` in this state's underlying data.

        If `value_pattern` is supplied, only values matching this pattern will
        be removed.

        If missing_ok is False and `key` is not present in the given scope, this
        must raise GitConfigUnsetMissingValue with `key` and `scope`.

        TODO: Make value_pattern an re.Pattern. This wasn't done at the time of
        this refactor to keep the refactor small.

        Implementations MUST accept non-canonical `key` values.
        """


class GitConfigUnsetMissingValue(ValueError):

    def __init__(self, key: str, scope: str) -> None:
        super().__init__(
            f'Cannot unset missing key {key!r} in scope {scope!r} with missing_ok=False.'
        )


class GitConfigUnsetMultipleValues(ValueError):

    def __init__(self, key: str, scope: str) -> None:
        super().__init__(
            f'Cannot unset multi-value key {key!r} in scope {scope!r} with modify_all=False.'
        )


class GitConfigUneditableScope(ValueError):

    def __init__(self, scope: str) -> None:
        super().__init__(f'Cannot edit git config in scope {scope!r}.')


class GitConfigUnknownScope(ValueError):

    def __init__(self, scope: str) -> None:
        super().__init__(f'Unknown git config scope {scope!r}.')


class GitConfigInvalidKey(ValueError):

    def __init__(self, key: str) -> None:
        super().__init__(
            f'Invalid git config key {key!r}: does not contain a section.')


def canonicalize_git_config_key(key: str) -> str:
    """Returns the canonicalized form of `key` for git config.

    Git config internally canonicalizes keys (i.e. for
    'section.subsection.variable', both 'section' and 'variable' will be
    lowercased, but 'subsection' will not).

    This also normalizes keys in the form 'section.variable' (both 'section' and
    'variable' will be lowercased).
    """
    sections = key.split('.')
    if len(sections) >= 3:
        return '.'.join(
            chain((sections[0].lower(), ), sections[1:-1],
                  (sections[-1].lower(), )))
    if len(sections) == 2:
        return '.'.join((sections[0].lower(), sections[1].lower()))
    raise GitConfigInvalidKey(key)


class CachedGitConfigState(object):
    """This represents the observable git configuration state for a given
    repository (whose top-level path is `root`).

    This maintains an in-memory cache of the entire, flattened, observable
    configuration state according to the GitConfigStateBase implementation.

    All SetConfig operations which actually change the underlying data will
    clear the internal cache. All read operations will either use the internal
    cache, or repopulate it from the GitConfigStateBase implementation
    on-demand.

    This design assumes no other processes are mutating git config state, which
    is typically true for git_cl and other short-lived programs in depot_tools
    which use scm.py.
    """

    def __init__(self, impl: GitConfigStateBase):
        """Initializes a git config cache against the given underlying
        GitConfigStateBase (either GitConfigStateReal or GitConfigStateTest).
        """
        self._impl: GitConfigStateBase = impl

        # Actual cached configuration from the point of view of this root.
        self._config: Optional[GitFlatConfigData] = None

    def _maybe_load_config(self) -> GitFlatConfigData:
        if self._config is None:
            # NOTE: Implementations of self._impl must already ensure that all
            # keys are canonicalized.
            self._config = self._impl.load_config()
        return self._config

    def clear_cache(self):
        self._config = None

    def GetConfig(self,
                  key: str,
                  default: Optional[str] = None,
                  scope: Optional[str] = None) -> Optional[str]:
        """Lazily loads all configration observable for this CachedGitConfigState,
        then returns the last value for `key` as a string.

        If `key` is missing, returns default.
        """

        key = canonicalize_git_config_key(key)
        if not scope:
            scope = "default"

        scoped_config = self._maybe_load_config()
        if not scoped_config:
            return default

        scoped_config = scoped_config.get(scope, None)
        if not scoped_config:
            return default

        values = scoped_config.get(key, None)

        if not values:
            return default
        return values[-1]

    def GetConfigBool(self, key: str) -> bool:
        """Returns the booleanized value of `key`.

        This follows `git config` semantics (i.e. it normalizes the string value
        of the config value to "true" - all other string values return False).
        """
        return self.GetConfig(key) == 'true'

    def GetConfigList(self, key: str) -> list[str]:
        """Returns all values of `key` as a list of strings."""
        key = canonicalize_git_config_key(key)
        return list(self._maybe_load_config().get('default', {}).get(key, ()))

    def YieldConfigRegexp(self,
                          pattern: Optional[str] = None
                          ) -> Iterable[tuple[str, str]]:
        """Yields (key, value) pairs for any config keys matching `pattern`.

        This use re.match, so `pattern` needs to be for the entire config key.

        If `pattern` is None, this returns all config items.

        Note that `pattern` is always matched against the canonicalized key
        value (i.e. for 'section.[subsection.]variable', both 'section' and
        'variable' will be lowercased, but 'subsection', if present, will not).
        """
        if pattern is None:
            pred = lambda _: True
        else:
            pred = re.compile(pattern).match
        for key, values in sorted(self._maybe_load_config().get('default',
                                                                {}).items()):
            if pred(key):
                for value in values:
                    yield key, value

    def SetConfig(self,
                  key,
                  value=None,
                  *,
                  append: bool = False,
                  missing_ok: bool = True,
                  modify_all: bool = False,
                  scope: GitConfigScope = 'local',
                  value_pattern: Optional[str] = None):
        """Sets or unsets one or more config values.

        Args:
            cwd: path to set `git config` for.
            key: The specific config key to affect.
            value: The value to set. If this is None, `key` will be unset.
            append: If True and `value` is not None, this will append
                the value instead of replacing an existing one. Must not be
                specified with value_pattern.
            missing_ok: If `value` is None (i.e. this is an unset operation),
                ignore retcode=5 from `git config` (meaning that the value is
                not present). If `value` is not None, then this option has no
                effect. If this is false and the key is missing, this will raise
                GitConfigUnsetMissingValue.
            modify_all: If True, this will change a set operation to
                `--replace-all`, and will change an unset operation to
                `--unset-all`. Must not be specified with value_pattern.
            scope: By default this is the `local` scope, but could be `global`
                or `worktree`, depending on which config scope you want to affect.
                Note that the `system` scope cannot be modified.
            value_pattern: For use with `modify_all=True`, allows
                further filtering of the set or unset operation based on
                the currently configured value. Ignored for
                `modify_all=False`.
        """
        if scope not in GitScopeOrder:
            raise GitConfigUnknownScope(scope)
        if scope == 'system':
            raise GitConfigUneditableScope(scope)

        if value is None:
            if modify_all:
                self._impl.unset_config_multi(key,
                                              value_pattern=value_pattern,
                                              scope=scope,
                                              missing_ok=missing_ok)
            else:
                self._impl.unset_config(key, scope=scope, missing_ok=missing_ok)
        else:
            if value_pattern:
                if not modify_all:
                    raise ValueError(
                        'SetConfig with (value_pattern) and (not modify_all) is invalid.'
                    )
                if append:
                    raise ValueError(
                        'SetConfig with (value_pattern) and (append) is invalid.'
                    )

                self._impl.set_config_multi(key,
                                            value,
                                            value_pattern=value_pattern,
                                            scope=scope)
            else:
                if modify_all:
                    self._impl.set_config_multi(key,
                                                value,
                                                value_pattern=None,
                                                scope=scope)

                self._impl.set_config(key, value, append=append, scope=scope)

        # Once the underlying storage has set the value, we clear our cache so
        # the next getter will reload it.
        self.clear_cache()


class GitConfigStateReal(GitConfigStateBase):
    """GitConfigStateReal implements CachedGitConfigState by actually interacting with
    the git configuration files on disk via GIT.Capture.
    """

    _GLOBAL_LOCK = threading.Lock()

    def __init__(self, root: pathlib.Path):
        super().__init__()
        self.root = root

    def load_config(self) -> GitFlatConfigData:
        # NOTE: `git config --list` already canonicalizes keys.
        try:
            gitConfigCmd = ['config', '--list', '-z', '--show-scope']
            if git_common.get_git_version() <= (2, 25):
                gitConfigCmd = ['config', '--list', '-z']

            rawConfig = GIT.Capture(gitConfigCmd,
                                    cwd=self.root,
                                    strip_out=False)
        except subprocess2.CalledProcessError:
            return {}

        assert isinstance(rawConfig, str)
        cfg: Dict[str, Dict[str,
                            list[str]]] = defaultdict(lambda: defaultdict(list))

        entries = rawConfig.split('\x00')[:-1]

        def process_entry(entry: str, scope: str) -> None:
            parts = entry.split('\n', 1)
            key, value = parts if len(parts) == 2 else (parts[0], '')
            key, value = key.strip(), value.strip()
            cfg[scope][key].append(value)
            if scope != "default":
                cfg["default"][key].append(value)

        i = 0
        while i < len(entries):
            if entries[i] in ['local', 'global', 'system']:
                scope = entries[i]
                i += 1
                if i < len(entries):
                    process_entry(entries[i], scope)
            else:
                process_entry(entries[i], "default")
            i += 1

        return cfg

    def set_config(self, key: str, value: str, *, append: bool,
                   scope: GitConfigScope):
        # NOTE: `git config` already canonicalizes key.
        args = ['config', f'--{scope}', key, value]
        if append:
            args.append('--add')
        with self._scope_lock(scope):
            GIT.Capture(args, cwd=self.root)

    def set_config_multi(self, key: str, value: str, *,
                         value_pattern: Optional[str], scope: GitConfigScope):
        # NOTE: `git config` already canonicalizes key.
        args = ['config', f'--{scope}', '--replace-all', key, value]
        if value_pattern is not None:
            args.append(value_pattern)
        with self._scope_lock(scope):
            GIT.Capture(args, cwd=self.root)

    def unset_config(self, key: str, *, scope: GitConfigScope,
                     missing_ok: bool):
        # NOTE: `git config` already canonicalizes key.
        accepted_retcodes = (0, 5) if missing_ok else (0, )
        try:
            with self._scope_lock(scope):
                GIT.Capture(['config', f'--{scope}', '--unset', key],
                            cwd=self.root,
                            accepted_retcodes=accepted_retcodes)
        except subprocess2.CalledProcessError as cpe:
            if cpe.returncode == 5:
                if b'multiple values' in cpe.stderr:
                    raise GitConfigUnsetMultipleValues(key, scope)
                raise GitConfigUnsetMissingValue(key, scope)
            raise

    def unset_config_multi(self, key: str, *, value_pattern: Optional[str],
                           scope: GitConfigScope, missing_ok: bool):
        # NOTE: `git config` already canonicalizes key.
        accepted_retcodes = (0, 5) if missing_ok else (0, )
        args = ['config', f'--{scope}', '--unset-all', key]
        if value_pattern is not None:
            args.append(value_pattern)
        try:
            with self._scope_lock(scope):
                GIT.Capture(args,
                            cwd=self.root,
                            accepted_retcodes=accepted_retcodes)
        except subprocess2.CalledProcessError as cpe:
            if cpe.returncode == 5:
                raise GitConfigUnsetMissingValue(key, scope)
            raise

    def _scope_lock(
            self,
            scope: GitConfigScope) -> contextlib.AbstractContextManager[Any]:
        if scope == 'global':
            return self._GLOBAL_LOCK
        # TODO(ayatane): We should lock per local repo scope as well
        # from a correctness perspective.
        return contextlib.nullcontext()


class GitConfigStateTest(GitConfigStateBase):
    """A fake implementation of GitConfigStateBase for testing.

    To properly initialize this, see tests/scm_mock.py.
    """

    def __init__(self,
                 global_state_lock: threading.Lock,
                 global_state: dict[str, list[str]],
                 *,
                 system_state: Optional[GitFlatConfigData] = None):
        """Initializes a new (local, worktree) config state, with a reference to
        a single global `global` state and an optional immutable `system` state.

        All keys in global_state and system_state MUST already be canonicalized
        with canonicalize_key().

        The caller must supply a single shared Lock, plus a mutable reference to
        the global-state dictionary.

        This implementation will hold global_state_lock during all read/write
        operations on the 'global' scope.
        """
        self.system_state: GitFlatConfigData = system_state or {}

        self.global_state_lock = global_state_lock
        self.global_state = global_state

        self.worktree_state: dict[str, list[str]] = {}
        self.local_state: dict[str, list[str]] = {}

        super().__init__()

    @contextlib.contextmanager
    def _editable_scope(
            self, scope: GitConfigScope) -> Iterator[dict[str, list[str]]]:
        if scope == 'system':
            # This is also checked in CachedGitConfigState.SetConfig, but double
            # check here.
            raise GitConfigUneditableScope(scope)

        if scope == 'global':
            with self.global_state_lock:
                yield self.global_state
        elif scope == 'local':
            yield self.local_state
        elif scope == 'worktree':
            yield self.worktree_state
        else:
            # This is also checked in CachedGitConfigState.SetConfig, but double
            # check here.
            raise GitConfigUnknownScope(scope)

    def load_config(self) -> GitFlatConfigData:
        cfg: Mapping[str, Mapping[str, list[str]]] = defaultdict(
            lambda: defaultdict(list))

        for key, values in self.system_state.items():
            cfg['system'][key].extend(values)
            cfg['default'][key].extend(values)
        for ordered_scope in GitScopeOrder:
            if ordered_scope == 'system':
                continue
            with self._editable_scope(ordered_scope) as scope_cfg:
                for key, values in scope_cfg.items():
                    cfg[ordered_scope][key].extend(values)
                    cfg['default'][key].extend(values)
        return cfg

    def set_config(self, key: str, value: str, *, append: bool,
                   scope: GitConfigScope):
        key = canonicalize_git_config_key(key)
        with self._editable_scope(scope) as cfg:
            cur = cfg.get(key)
            if cur is None:
                cfg[key] = [value]
                return
            if append:
                cfg[key] = cur + [value]
                return
            if len(cur) == 1:
                cfg[key] = [value]
                return
            raise ValueError(f'GitConfigStateTest: Cannot set key {key} '
                             f'- current value {cur!r} is multiple.')

    def set_config_multi(self, key: str, value: str, *,
                         value_pattern: Optional[str], scope: GitConfigScope):
        key = canonicalize_git_config_key(key)
        with self._editable_scope(scope) as cfg:
            cur = cfg.get(key)
            if value_pattern is None or cur is None:
                cfg[key] = [value]
                return

            # We want to insert `value` in place of the first pattern match - if
            # multiple values match, they will all be removed.
            pat = re.compile(value_pattern)
            newval = []
            added = False
            for val in cur:
                if pat.match(val):
                    if not added:
                        newval.append(value)
                        added = True
                else:
                    newval.append(val)
            if not added:
                newval.append(value)
            cfg[key] = newval

    def unset_config(self, key: str, *, scope: GitConfigScope,
                     missing_ok: bool):
        key = canonicalize_git_config_key(key)
        with self._editable_scope(scope) as cfg:
            cur = cfg.get(key)
            if cur is None:
                if missing_ok:
                    return
                raise GitConfigUnsetMissingValue(key, scope)
            if len(cur) == 1:
                del cfg[key]
                return
            raise GitConfigUnsetMultipleValues(key, scope)

    def unset_config_multi(self, key: str, *, value_pattern: Optional[str],
                           scope: GitConfigScope, missing_ok: bool):
        key = canonicalize_git_config_key(key)
        with self._editable_scope(scope) as cfg:
            cur = cfg.get(key)
            if cur is None:
                if not missing_ok:
                    raise GitConfigUnsetMissingValue(key, scope)
                return

            if value_pattern is None:
                del cfg[key]
                return

            if cur is None:
                del cfg[key]
                return

            pat = re.compile(value_pattern)
            cfg[key] = [v for v in cur if not pat.match(v)]


class GIT(object):
    current_version = None
    rev_parse_cache = {}

    # Maps cwd -> {config key, [config values]}
    # This cache speeds up all `git config ...` operations by only running a
    # single subcommand, which can greatly accelerate things like
    # git-map-branches.
    _CONFIG_CACHE: Dict[pathlib.Path, Optional[CachedGitConfigState]] = {}
    _CONFIG_CACHE_LOCK = threading.Lock()

    @classmethod
    def drop_config_cache(cls):
        """Completely purges all cached git config data.

        This should always be safe to call (it will be lazily repopulated), but
        really is only meant to be called from tests.
        """
        with cls._CONFIG_CACHE_LOCK:
            cls._CONFIG_CACHE = {}

    @staticmethod
    def _new_config_state(root: pathlib.Path) -> GitConfigStateBase:
        """_new_config_state is mocked in tests/scm_mock to return
        a GitConfigStateTest."""
        return GitConfigStateReal(root)

    @classmethod
    def _get_config_state(cls, cwd: str) -> CachedGitConfigState:
        key = pathlib.Path(cwd).absolute()
        with cls._CONFIG_CACHE_LOCK:
            cur = GIT._CONFIG_CACHE.get(key, None)
            if cur is not None:
                return cur
            ret = CachedGitConfigState(cls._new_config_state(key))
            cls._CONFIG_CACHE[key] = ret
            return ret

    @classmethod
    def _dump_config_state(cls) -> Dict[str, GitFlatConfigData]:
        """Dump internal config state.

        Used for testing.  This will NOT work properly in non-test
        contexts as it relies on internal caches.
        """
        with cls._CONFIG_CACHE_LOCK:
            state = {}
            for key, val in cls._CONFIG_CACHE.items():
                if val is not None:
                    state[str(key)] = val._maybe_load_config().get(
                        'default', {})
        return state

    @staticmethod
    def ApplyEnvVars(kwargs):
        env = kwargs.pop('env', None) or os.environ.copy()
        # Don't prompt for passwords; just fail quickly and noisily.
        # By default, git will use an interactive terminal prompt when a
        # username/ password is needed.  That shouldn't happen in the chromium
        # workflow, and if it does, then gclient may hide the prompt in the
        # midst of a flood of terminal spew.  The only indication that something
        # has gone wrong will be when gclient hangs unresponsively.  Instead, we
        # disable the password prompt and simply allow git to fail noisily.  The
        # error message produced by git will be copied to gclient's output.
        env.setdefault('GIT_ASKPASS', 'true')
        env.setdefault('SSH_ASKPASS', 'true')
        # 'cat' is a magical git string that disables pagers on all platforms.
        env.setdefault('GIT_PAGER', 'cat')
        return env

    @staticmethod
    def Capture(args, cwd=None, strip_out=True, **kwargs) -> str | bytes:
        kwargs.setdefault('env', GIT.ApplyEnvVars(kwargs))
        kwargs.setdefault('cwd', cwd)
        kwargs.setdefault('autostrip', strip_out)
        return git_common.run(*args, **kwargs)

    @staticmethod
    def CaptureStatus(
            cwd: str,
            upstream_branch: str,
            end_commit: Optional[str] = None,
            ignore_submodules: bool = True) -> Sequence[tuple[str, str]]:
        """Returns git status.

        Returns an array of (status, file) tuples."""
        if end_commit is None:
            end_commit = ''
        if upstream_branch is None:
            upstream_branch = GIT.GetUpstreamBranch(cwd)
            if upstream_branch is None:
                raise gclient_utils.Error('Cannot determine upstream branch')

        command = [
            '-c', 'core.quotePath=false', 'diff', '--name-status',
            '--no-renames'
        ]
        if ignore_submodules:
            command.append('--ignore-submodules=all')
        command.extend(['-r', '%s...%s' % (upstream_branch, end_commit)])

        status = GIT.Capture(command, cwd)
        assert isinstance(status, str)
        results = []
        if status:
            for statusline in status.splitlines():
                # 3-way merges can cause the status can be 'MMM' instead of 'M'.
                # This can happen when the user has 2 local branches and he
                # diffs between these 2 branches instead diffing to upstream.
                m = re.match(r'^(\w)+\t(.+)$', statusline)
                if not m:
                    raise gclient_utils.Error(
                        'status currently unsupported: %s' % statusline)
                # Only grab the first letter.
                results.append(('%s      ' % m.group(1)[0], m.group(2)))
        return results

    @staticmethod
    def GetConfig(cwd: str,
                  key: str,
                  default: Optional[str] = None,
                  scope: Optional[str] = None) -> Optional[str]:
        """Lazily loads all configration observable for this CachedGitConfigState,
        then returns the last value for `key` as a string.

        If `key` is missing, returns default.
        """
        return GIT._get_config_state(cwd).GetConfig(key, default, scope)

    @staticmethod
    def GetConfigBool(cwd: str, key: str) -> bool:
        """Returns the booleanized value of `key`.

        This follows `git config` semantics (i.e. it normalizes the string value
        of the config value to "true" - all other string values return False).
        """
        return GIT._get_config_state(cwd).GetConfigBool(key)

    @staticmethod
    def GetConfigList(cwd: str, key: str) -> list[str]:
        """Returns all values of `key` as a list of strings."""
        return GIT._get_config_state(cwd).GetConfigList(key)

    @staticmethod
    def YieldConfigRegexp(
            cwd: str,
            pattern: Optional[str] = None) -> Iterable[tuple[str, str]]:
        """Yields (key, value) pairs for any config keys matching `pattern`.

        This use re.match, so `pattern` needs to be for the entire config key.

        If pattern is None, this returns all config items.
        """
        yield from GIT._get_config_state(cwd).YieldConfigRegexp(pattern)

    @staticmethod
    def GetBranchConfig(cwd: str,
                        branch: str,
                        key: str,
                        default: Optional[str] = None) -> Optional[str]:
        assert branch, 'A branch must be given'
        key = 'branch.%s.%s' % (branch, key)
        return GIT.GetConfig(cwd, key, default)

    @staticmethod
    def SetConfig(cwd: str,
                  key: str,
                  value: Optional[str] = None,
                  *,
                  append: bool = False,
                  missing_ok: bool = True,
                  modify_all: bool = False,
                  scope: GitConfigScope = 'local',
                  value_pattern: Optional[str] = None):
        """Sets or unsets one or more config values.

        Args:
            cwd: path to set `git config` for.
            key: The specific config key to affect.
            value: The value to set. If this is None, `key` will be unset.
            append: If True and `value` is not None, this will append
                the value instead of replacing an existing one. Must not be
                specified with value_pattern.
            missing_ok: If `value` is None (i.e. this is an unset operation),
                ignore retcode=5 from `git config` (meaning that the value is
                not present). If `value` is not None, then this option has no
                effect. If this is false and the key is missing, this will raise
                GitConfigUnsetMissingValue.
            modify_all: If True, this will change a set operation to
                `--replace-all`, and will change an unset operation to
                `--unset-all`. Must not be specified with value_pattern.
            scope: By default this is the `local` scope, but could be `global`
                or `worktree`, depending on which config scope you want to affect.
                Note that the `system` scope cannot be modified.
            value_pattern: For use with `modify_all=True`, allows
                further filtering of the set or unset operation based on
                the currently configured value. Ignored for
                `modify_all=False`.
        """
        GIT._get_config_state(cwd).SetConfig(key,
                                             value,
                                             append=append,
                                             missing_ok=missing_ok,
                                             modify_all=modify_all,
                                             scope=scope,
                                             value_pattern=value_pattern)

    @staticmethod
    def SetBranchConfig(cwd, branch, key, value=None):
        assert branch, 'A branch must be given'
        key = 'branch.%s.%s' % (branch, key)
        GIT.SetConfig(cwd, key, value)

    @staticmethod
    def ShortBranchName(branch):
        """Converts a name like 'refs/heads/foo' to just 'foo'."""
        return branch.replace('refs/heads/', '')

    @staticmethod
    def GetBranchRef(cwd):
        """Returns the full branch reference, e.g. 'refs/heads/main'."""
        try:
            return GIT.Capture(['symbolic-ref', 'HEAD'], cwd=cwd)
        except subprocess2.CalledProcessError:
            return None

    @staticmethod
    def GetRemoteHeadRef(cwd, url, remote):
        """Returns the full default remote branch reference, e.g.
        'refs/remotes/origin/main'."""
        if os.path.exists(cwd):
            ref = 'refs/remotes/%s/HEAD' % remote
            try:
                # Try using local git copy first
                ref = GIT.Capture(['symbolic-ref', ref], cwd=cwd)
                assert isinstance(ref, str)
                if not ref.endswith('master'):
                    return ref
            except subprocess2.CalledProcessError:
                pass

            try:
                # Check if there are changes in the default branch for this
                # particular repository.
                GIT.Capture(['remote', 'set-head', '-a', remote], cwd=cwd)
                return GIT.Capture(['symbolic-ref', ref], cwd=cwd)
            except subprocess2.CalledProcessError:
                pass

        try:
            # Fetch information from git server
            resp = GIT.Capture(['ls-remote', '--symref', url, 'HEAD'])
            assert isinstance(resp, str)
            regex = r'^ref: (.*)\tHEAD$'
            for line in resp.split('\n'):
                m = re.match(regex, line)
                if m:
                    refpair = GIT.RefToRemoteRef(m.group(1), remote)
                    assert isinstance(refpair, tuple)
                    return ''.join(refpair)
        except subprocess2.CalledProcessError:
            pass
        # Return default branch
        return 'refs/remotes/%s/main' % remote

    @staticmethod
    def GetBranch(cwd):
        """Returns the short branch name, e.g. 'main'."""
        branchref = GIT.GetBranchRef(cwd)
        if branchref:
            return GIT.ShortBranchName(branchref)
        return None

    @staticmethod
    def GetRemoteBranches(cwd):
        return GIT.Capture(['branch', '-r'], cwd=cwd).split()

    @staticmethod
    def FetchUpstreamTuple(
            cwd: str,
            branch: Optional[str] = None
    ) -> tuple[Optional[str], Optional[str]]:
        """Returns a tuple containing remote and remote ref,
        e.g. 'origin', 'refs/heads/main'
        """
        try:
            branch = branch or GIT.GetBranch(cwd)
        except subprocess2.CalledProcessError:
            pass
        if branch:
            upstream_branch = GIT.GetBranchConfig(cwd, branch, 'merge')
            if upstream_branch:
                remote = GIT.GetBranchConfig(cwd, branch, 'remote', '.')
                return remote, upstream_branch

        upstream_branch = GIT.GetConfig(cwd, 'rietveld.upstream-branch')
        if upstream_branch:
            remote = GIT.GetConfig(cwd, 'rietveld.upstream-remote', '.')
            return remote, upstream_branch

        # Else, try to guess the origin remote.
        remote_branches = GIT.GetRemoteBranches(cwd)
        if 'origin/main' in remote_branches:
            # Fall back on origin/main if it exits.
            return 'origin', 'refs/heads/main'

        if 'origin/master' in remote_branches:
            # Fall back on origin/master if it exits.
            return 'origin', 'refs/heads/master'

        return None, None

    @staticmethod
    def RefToRemoteRef(ref, remote) -> Optional[tuple[str, str]]:
        """Convert a checkout ref to the equivalent remote ref.

        Returns:
            A tuple of the remote ref's (common prefix, unique suffix), or None if it
            doesn't appear to refer to a remote ref (e.g. it's a commit hash).
        """
        # TODO(mmoss): This is just a brute-force mapping based of the expected
        # git config. It's a bit better than the even more brute-force
        # replace('heads', ...), but could still be smarter (like maybe actually
        # using values gleaned from the git config).
        m = re.match('^(refs/(remotes/)?)?branch-heads/', ref or '')
        if m:
            return ('refs/remotes/branch-heads/', ref.replace(m.group(0), ''))

        m = re.match('^((refs/)?remotes/)?%s/|(refs/)?heads/' % remote, ref
                     or '')
        if m:
            return ('refs/remotes/%s/' % remote, ref.replace(m.group(0), ''))

        return None

    @staticmethod
    def RemoteRefToRef(ref, remote):
        assert remote, 'A remote must be given'
        if not ref or not ref.startswith('refs/'):
            return None
        if not ref.startswith('refs/remotes/'):
            return ref
        if ref.startswith('refs/remotes/branch-heads/'):
            return 'refs' + ref[len('refs/remotes'):]
        if ref.startswith('refs/remotes/%s/' % remote):
            return 'refs/heads' + ref[len('refs/remotes/%s' % remote):]
        return None

    @staticmethod
    def GetUpstreamBranch(cwd) -> Optional[str]:
        """Gets the current branch's upstream branch."""
        remote, upstream_branch = GIT.FetchUpstreamTuple(cwd)
        if remote != '.' and upstream_branch:
            remote_ref = GIT.RefToRemoteRef(upstream_branch, remote)
            if remote_ref:
                upstream_branch = ''.join(remote_ref)
        return upstream_branch

    @staticmethod
    def IsAncestor(maybe_ancestor: str,
                   ref: str,
                   cwd: Optional[str] = None) -> bool:
        """Verifies if |maybe_ancestor| is an ancestor of |ref|."""
        try:
            GIT.Capture(['merge-base', '--is-ancestor', maybe_ancestor, ref],
                        cwd=cwd)
            return True
        except subprocess2.CalledProcessError:
            return False

    @staticmethod
    def GetOldContents(cwd, filename, branch=None):
        if not branch:
            branch = GIT.GetUpstreamBranch(cwd)
        if platform.system() == 'Windows':
            # git show <sha>:<path> wants a posix path.
            filename = filename.replace('\\', '/')
        command = ['show', '%s:%s' % (branch, filename)]
        try:
            return GIT.Capture(command, cwd=cwd, strip_out=False)
        except subprocess2.CalledProcessError:
            return ''

    @staticmethod
    def GenerateDiff(cwd: str,
                     branch: Optional[str] = None,
                     branch_head: str = 'HEAD',
                     full_move: bool = False,
                     files: Optional[Iterable[str]] = None) -> str:
        """Diffs against the upstream branch or optionally another branch.

        full_move means that move or copy operations should completely recreate the
        files, usually in the prospect to apply the patch for a try job."""
        if not branch:
            branch = GIT.GetUpstreamBranch(cwd)
            assert isinstance(branch, str)
        command = [
            '-c',
            'core.quotePath=false',
            'diff',
            '-p',
            '--no-color',
            '--no-prefix',
            '--no-ext-diff',
            branch + "..." + branch_head,
        ]
        if full_move:
            command.append('--no-renames')
        else:
            command.append('-C')
        # TODO(maruel): --binary support.
        if files:
            command.append('--')
            command.extend(files)
        output = GIT.Capture(command, cwd=cwd, strip_out=False)
        assert isinstance(output, str)
        diff = output.splitlines(True)
        for i in range(len(diff)):
            # In the case of added files, replace /dev/null with the path to the
            # file being added.
            if diff[i].startswith('--- /dev/null'):
                diff[i] = '--- %s' % diff[i + 1][4:]
        return ''.join(diff)

    @staticmethod
    def GetAllFiles(cwd):
        """Returns the list of all files under revision control."""
        command = ['-c', 'core.quotePath=false', 'ls-files', '--', '.']
        return GIT.Capture(command, cwd=cwd).splitlines(False)

    @staticmethod
    def GetSubmoduleCommits(cwd: str,
                            submodules: list[str]) -> Mapping[str, str]:
        """Returns a mapping of staged or committed new commits for submodules."""
        if not submodules:
            return {}
        result = subprocess2.check_output(['git', 'ls-files', '-s', '--'] +
                                          submodules,
                                          cwd=cwd).decode('utf-8')
        commit_hashes = {}
        for r in result.splitlines():
            # ['<mode>', '<commit_hash>', '<stage_number>', '<path>'].
            record = r.strip().split(maxsplit=3)  # path can contain spaces.
            assert record[0] == '160000', 'file is not a gitlink: %s' % record
            commit_hashes[record[3]] = record[1]
        return commit_hashes

    @staticmethod
    def GetCheckoutRoot(cwd) -> str:
        """Returns the top level directory of a git checkout as an absolute path.
        """
        root = GIT.Capture(['rev-parse', '--show-cdup'], cwd=cwd)
        assert isinstance(root, str)
        return os.path.abspath(os.path.join(cwd, root))

    @staticmethod
    def IsInsideWorkTree(cwd):
        try:
            return GIT.Capture(['rev-parse', '--is-inside-work-tree'], cwd=cwd)
        except (OSError, subprocess2.CalledProcessError):
            return False

    @staticmethod
    def IsVersioned(cwd: str, relative_dir: str) -> int:
        """Checks whether the given |relative_dir| is part of cwd's repo."""
        output = GIT.Capture(['ls-tree', 'HEAD', '--', relative_dir], cwd=cwd)
        assert isinstance(output, str)
        if not output:
            return VERSIONED_NO
        if output.startswith('160000'):
            return VERSIONED_SUBMODULE
        return VERSIONED_DIR

    @staticmethod
    def ListSubmodules(repo_root: str) -> Collection[str]:
        """Returns the list of submodule paths for the given repo.

        Path separators will be adjusted for the current OS.
        """
        if not os.path.exists(os.path.join(repo_root, '.gitmodules')):
            return []
        try:
            config_output = GIT.Capture(
                ['config', '--file', '.gitmodules', '--get-regexp', 'path'],
                cwd=repo_root)
        except subprocess2.CalledProcessError:
            # Git exits with 1 if no config matches are found.
            return []
        assert isinstance(config_output, str)
        return [
            line.split()[-1].replace('/', os.path.sep)
            for line in config_output.splitlines()
        ]

    @staticmethod
    def CleanupDir(cwd, relative_dir):
        """Cleans up untracked file inside |relative_dir|."""
        return bool(GIT.Capture(['clean', '-df', relative_dir], cwd=cwd))

    @staticmethod
    def ResolveCommit(cwd, rev):
        cache_key = None
        # We do this instead of rev-parse --verify rev^{commit}, since on
        # Windows git can be either an executable or batch script, each of which
        # requires escaping the caret (^) a different way.
        if gclient_utils.IsFullGitSha(rev):
            # Only cache full SHAs
            cache_key = hash(cwd + rev)
            if val := GIT.rev_parse_cache.get(cache_key):
                return val

            # git-rev parse --verify FULL_GIT_SHA always succeeds, even if we
            # don't have FULL_GIT_SHA locally. Removing the last character
            # forces git to check if FULL_GIT_SHA refers to an object in the
            # local database.
            rev = rev[:-1]
        res = GIT.Capture(['rev-parse', '--quiet', '--verify', rev], cwd=cwd)
        if cache_key:
            # We don't expect concurrent execution, so we don't lock anything.
            GIT.rev_parse_cache[cache_key] = res

        return res

    @staticmethod
    def IsValidRevision(cwd, rev, sha_only=False):
        """Verifies the revision is a proper git revision.

        sha_only: Fail unless rev is a sha hash.
        """
        try:
            sha = GIT.ResolveCommit(cwd, rev)
        except subprocess2.CalledProcessError:
            return None

        if sha_only:
            return sha == rev.lower()
        return True


class DIFF(object):

    @staticmethod
    def GetAllFiles(cwd):
        """Return all files under the repo at cwd.

        If .gitmodules exists in cwd, use it to determine which folders are
        submodules and don't recurse into them. Submodule paths are returned.
        """
        # `git config --file` works outside of a git workspace.
        submodules = GIT.ListSubmodules(cwd)
        if not submodules:
            return [
                str(p.relative_to(cwd)) for p in pathlib.Path(cwd).rglob("*")
                if p.is_file()
            ]

        full_path_submodules = {os.path.join(cwd, s) for s in submodules}

        def should_recurse(dirpath, dirname):
            full_path = os.path.join(dirpath, dirname)
            return full_path not in full_path_submodules

        paths = list(full_path_submodules)
        for dirpath, dirnames, filenames in os.walk(cwd):
            paths.extend([os.path.join(dirpath, f) for f in filenames])
            dirnames[:] = [d for d in dirnames if should_recurse(dirpath, d)]

        return [os.path.relpath(p, cwd) for p in paths]


# vim: sts=4:ts=4:sw=4:tw=80:et:
