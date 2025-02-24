# Copyright (c) 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Defines utilities for setting up Git authentication."""

from __future__ import annotations

import enum
import functools
import logging
import os
from typing import TYPE_CHECKING, Callable
import urllib.parse

import gerrit_util
import newauth
import scm

if TYPE_CHECKING:
    # Causes import cycle if imported normally
    import git_cl


class ConfigMode(enum.Enum):
    """Modes to pass to ConfigChanger"""
    NO_AUTH = 1
    NEW_AUTH = 2
    NEW_AUTH_SSO = 3


class ConfigChanger(object):
    """Changes Git auth config as needed for Gerrit."""

    # Can be used to determine whether this version of the config has
    # been applied to a Git repo.
    #
    # Increment this when making changes to the config, so that reliant
    # code can determine whether the config needs to be re-applied.
    VERSION: int = 4

    def __init__(
        self,
        *,
        mode: ConfigMode,
        remote_url: str,
        set_config_func: Callable[..., None] = scm.GIT.SetConfig,
    ):
        """Create a new ConfigChanger.

        Args:
            mode: How to configure auth
            remote_url: Git repository's remote URL, e.g.,
                https://chromium.googlesource.com/chromium/tools/depot_tools.git
            set_config_func: Function used to set configuration.  Used
                for testing.
        """
        self.mode: ConfigMode = mode

        self._remote_url: str = remote_url
        self._set_config_func: Callable[..., None] = set_config_func

    @functools.cached_property
    def _shortname(self) -> str:
        # Example: chromium
        parts: urllib.parse.SplitResult = urllib.parse.urlsplit(
            self._remote_url)
        name: str = parts.netloc.split('.')[0]
        if name.endswith('-review'):
            name = name[:-len('-review')]
        return name

    @functools.cached_property
    def _base_url(self) -> str:
        # Example: https://chromium.googlesource.com/
        # Example: https://chromium-review.googlesource.com/
        parts: urllib.parse.SplitResult = urllib.parse.urlsplit(
            self._remote_url)
        return parts._replace(path='/', query='', fragment='').geturl()

    @classmethod
    def new_from_env(cls, cwd: str, cl: git_cl.Changelist) -> ConfigChanger:
        """Create a ConfigChanger by inferring from env.

        The Gerrit host is inferred from the current repo/branch.
        The user, which is used to determine the mode, is inferred using
        git-config(1) in the given `cwd`.
        """
        # This is determined either from the branch or repo config.
        #
        # Example: chromium-review.googlesource.com
        gerrit_host = cl.GetGerritHost()
        # This depends on what the user set for their remote.
        # There are a couple potential variations for the same host+repo.
        #
        # Example:
        # https://chromium.googlesource.com/chromium/tools/depot_tools.git
        remote_url = cl.GetRemoteUrl()

        if gerrit_host is None or remote_url is None:
            raise Exception(
                'Error Git auth settings inferring from environment:'
                f' {gerrit_host=} {remote_url=}')
        assert gerrit_host is not None
        assert remote_url is not None

        return cls(
            mode=cls._infer_mode(cwd, gerrit_host),
            remote_url=remote_url,
        )

    @classmethod
    def new_for_remote(cls, cwd: str, remote_url: str) -> ConfigChanger:
        """Create a ConfigChanger for the given Gerrit host.

        The user, which is used to determine the mode, is inferred using
        git-config(1) in the given `cwd`.
        """
        c = cls(
            mode=ConfigMode.NEW_AUTH,
            remote_url=remote_url,
        )
        c.mode = cls._infer_mode(cwd, c._shortname + '-review.googlesource.com')
        return c

    @staticmethod
    def _infer_mode(cwd: str, gerrit_host: str) -> ConfigMode:
        """Infer default mode to use."""
        if not newauth.Enabled():
            return ConfigMode.NO_AUTH
        email: str = scm.GIT.GetConfig(cwd, 'user.email') or ''
        if gerrit_util.ShouldUseSSO(gerrit_host, email):
            return ConfigMode.NEW_AUTH_SSO
        if not gerrit_util.GitCredsAuthenticator.gerrit_account_exists(
                gerrit_host):
            return ConfigMode.NO_AUTH
        return ConfigMode.NEW_AUTH

    def apply(self, cwd: str) -> None:
        """Apply config changes to the Git repo directory."""
        self._apply_cred_helper(cwd)
        self._apply_sso(cwd)
        self._apply_gitcookies(cwd)

    def apply_global(self, cwd: str) -> None:
        """Apply config changes to the global (user) Git config.

        This will make the instance's mode (e.g., SSO or not) the global
        default for the Gerrit host, if not overridden by a specific Git repo.
        """
        self._apply_global_cred_helper(cwd)
        self._apply_global_sso(cwd)

    def _apply_cred_helper(self, cwd: str) -> None:
        """Apply config changes relating to credential helper."""
        cred_key: str = f'credential.{self._base_url}.helper'
        if self.mode == ConfigMode.NEW_AUTH:
            self._set_config(cwd, cred_key, '', modify_all=True)
            self._set_config(cwd, cred_key, 'luci', append=True)
        elif self.mode == ConfigMode.NEW_AUTH_SSO:
            self._set_config(cwd, cred_key, None, modify_all=True)
        elif self.mode == ConfigMode.NO_AUTH:
            self._set_config(cwd, cred_key, None, modify_all=True)
        else:
            raise TypeError(f'Invalid mode {self.mode!r}')

    def _apply_sso(self, cwd: str) -> None:
        """Apply config changes relating to SSO."""
        sso_key: str = f'url.sso://{self._shortname}/.insteadOf'
        http_key: str = f'url.{self._remote_url}.insteadOf'
        if self.mode == ConfigMode.NEW_AUTH:
            self._set_config(cwd, 'protocol.sso.allow', None)
            self._set_config(cwd, sso_key, None, modify_all=True)
            # Shadow a potential global SSO rewrite rule.
            self._set_config(cwd, http_key, self._remote_url, modify_all=True)
        elif self.mode == ConfigMode.NEW_AUTH_SSO:
            self._set_config(cwd, 'protocol.sso.allow', 'always')
            self._set_config(cwd, sso_key, self._base_url, modify_all=True)
            self._set_config(cwd, http_key, None, modify_all=True)
        elif self.mode == ConfigMode.NO_AUTH:
            self._set_config(cwd, 'protocol.sso.allow', None)
            self._set_config(cwd, sso_key, None, modify_all=True)
            self._set_config(cwd, http_key, None, modify_all=True)
        else:
            raise TypeError(f'Invalid mode {self.mode!r}')

    def _apply_gitcookies(self, cwd: str) -> None:
        """Apply config changes relating to gitcookies."""
        if self.mode == ConfigMode.NEW_AUTH:
            # Override potential global setting
            self._set_config(cwd, 'http.cookieFile', '', modify_all=True)
        elif self.mode == ConfigMode.NEW_AUTH_SSO:
            # Override potential global setting
            self._set_config(cwd, 'http.cookieFile', '', modify_all=True)
        elif self.mode == ConfigMode.NO_AUTH:
            self._set_config(cwd, 'http.cookieFile', None, modify_all=True)
        else:
            raise TypeError(f'Invalid mode {self.mode!r}')

    def _apply_global_cred_helper(self, cwd: str) -> None:
        """Apply config changes relating to credential helper."""
        cred_key: str = f'credential.{self._base_url}.helper'
        if self.mode == ConfigMode.NEW_AUTH:
            self._set_config(cwd, cred_key, '', scope='global', modify_all=True)
            self._set_config(cwd, cred_key, 'luci', scope='global', append=True)
        elif self.mode == ConfigMode.NEW_AUTH_SSO:
            # Avoid editing the user's config in case they manually
            # configured something.
            pass
        elif self.mode == ConfigMode.NO_AUTH:
            # Avoid editing the user's config in case they manually
            # configured something.
            pass
        else:
            raise TypeError(f'Invalid mode {self.mode!r}')

    def _apply_global_sso(self, cwd: str) -> None:
        """Apply config changes relating to SSO."""
        sso_key: str = f'url.sso://{self._shortname}/.insteadOf'
        if self.mode == ConfigMode.NEW_AUTH:
            # Do not unset protocol.sso.allow because it may be used by
            # other hosts.
            self._set_config(cwd,
                             sso_key,
                             None,
                             scope='global',
                             modify_all=True)
        elif self.mode == ConfigMode.NEW_AUTH_SSO:
            self._set_config(cwd,
                             'protocol.sso.allow',
                             'always',
                             scope='global')
            self._set_config(cwd,
                             sso_key,
                             self._base_url,
                             scope='global',
                             modify_all=True)
        elif self.mode == ConfigMode.NO_AUTH:
            # Avoid editing the user's config in case they manually
            # configured something.
            pass
        else:
            raise TypeError(f'Invalid mode {self.mode!r}')

    def _set_config(self, *args, **kwargs) -> None:
        self._set_config_func(*args, **kwargs)


def AutoConfigure(cwd: str, cl: git_cl.Changelist) -> None:
    """Configure Git authentication automatically.

    This tracks when the config that has already been applied and skips
    doing anything if so.

    This may modify the global Git config and the local repo config as
    needed.
    """
    latestVer: int = ConfigChanger.VERSION
    v: int = 0
    try:
        v = int(
            scm.GIT.GetConfig(cwd, 'depot-tools.gitauthautoconfigured') or '0')
    except ValueError:
        v = 0
    if v < latestVer:
        logging.debug(
            'Automatically configuring Git repo authentication'
            ' (current version: %r, latest: %r)', v, latestVer)
        Configure(cwd, cl)
        scm.GIT.SetConfig(cwd, 'depot-tools.gitAuthAutoConfigured',
                          str(latestVer))


def Configure(cwd: str, cl: git_cl.Changelist) -> None:
    """Configure Git authentication.

    This may modify the global Git config and the local repo config as
    needed.
    """
    logging.debug('Configuring Git authentication...')

    logging.debug('Configuring global Git authentication...')

    # We want the user's global config.
    # We can probably assume the root directory doesn't have any local
    # Git configuration.
    c = ConfigChanger.new_from_env('/', cl)
    c.apply_global(os.path.expanduser('~'))

    c2 = ConfigChanger.new_from_env(cwd, cl)
    if c2.mode == c.mode:
        logging.debug(
            'Local user wants same mode %s as global;'
            ' clearing local repo auth config', c2.mode)
        c2.mode = ConfigMode.NO_AUTH
        c2.apply(cwd)
        return
    logging.debug('Local user wants mode %s while global user wants mode %s',
                  c2.mode, c.mode)
    logging.debug('Configuring current Git repo authentication...')
    c2.apply(cwd)


def ConfigureGlobal(cwd: str, remote_url: str) -> None:
    """Configure global/user Git authentication."""
    logging.debug('Configuring global Git authentication for %s', remote_url)
    ConfigChanger.new_for_remote(cwd, remote_url).apply_global(cwd)


def ClearRepoConfig(cwd: str, cl: git_cl.Changelist) -> None:
    """Clear the current Git repo authentication."""
    logging.debug('Clearing current Git repo authentication...')
    c = ConfigChanger.new_from_env(cwd, cl)
    c.mode = ConfigMode.NO_AUTH
    c.apply(cwd)
