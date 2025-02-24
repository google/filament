# Copyright (c) 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import annotations

import os
import sys
import threading
from typing import Iterable

from unittest import mock
import unittest

# This is to be able to import scm from the root of the repo.
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import scm


def GIT(
    test: unittest.TestCase,
    *,
    branchref: str | None = None,
    system_config: dict[str, list[str]] | None = None
) -> Iterable[tuple[str, list[str]]]:
    """Installs fakes/mocks for scm.GIT so that:

      * GetBranch will just return a fake branchname starting with the value of
        branchref.
      * git_new_branch.create_new_branch will be mocked to update the value
        returned by GetBranch.

    If provided, `system_config` allows you to set the 'system' scoped
    git-config which will be visible as the immutable base configuration layer
    for all git config scopes.

    NOTE: The dependency on git_new_branch.create_new_branch seems pretty
    circular - this functionality should probably move to scm.GIT?
    """
    _branchref = [branchref or 'refs/heads/main']

    global_lock = threading.Lock()
    global_state = {}

    def _newBranch(branchref):
        _branchref[0] = branchref

    patches: list[mock._patch] = [
        mock.patch('scm.GIT._new_config_state',
                   side_effect=lambda _: scm.GitConfigStateTest(
                       global_lock, global_state, system_state=system_config)),
        mock.patch('scm.GIT.GetBranchRef', side_effect=lambda _: _branchref[0]),
        mock.patch('git_new_branch.create_new_branch', side_effect=_newBranch)
    ]

    for p in patches:
        p.start()
        test.addCleanup(p.stop)

    test.addCleanup(scm.GIT.drop_config_cache)

    return global_state.items()
