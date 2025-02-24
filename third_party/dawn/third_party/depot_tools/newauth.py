# Copyright (c) 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Defines common conditions for the new auth stack migration."""

from __future__ import annotations

import os

import scm


def Enabled() -> bool:
    """Returns True if new auth stack is enabled."""
    return scm.GIT.GetConfig(os.getcwd(),
                             'depot-tools.usenewauthstack') in ('yes', 'on',
                                                                'true', '1')


def ExplicitlyDisabled() -> bool:
    """Returns True if new auth stack is explicitly disabled."""
    return scm.GIT.GetConfig(os.getcwd(),
                             'depot-tools.usenewauthstack') in ('no', 'off',
                                                                'false', '0')


def SkipSSO() -> bool:
    """Returns True if skip SSO is set."""
    return scm.GIT.GetConfig(os.getcwd(),
                             'depot-tools.newauthskipsso') in ('yes', 'on',
                                                               'true', '1')
