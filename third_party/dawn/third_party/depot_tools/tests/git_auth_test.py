#!/usr/bin/env vpython3
# coding=utf-8
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for git_cl.py."""

import logging
import os
import sys
import unittest
from typing import Iterable

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import git_auth
import scm
import scm_mock


class TestConfigChanger(unittest.TestCase):

    maxDiff = None

    def setUp(self):
        self._global_state_view: Iterable[tuple[str,
                                                list[str]]] = scm_mock.GIT(self)

    @property
    def global_state(self):
        return dict(self._global_state_view)

    def test_apply_new_auth(self):
        git_auth.ConfigChanger(
            mode=git_auth.ConfigMode.NEW_AUTH,
            remote_url=
            'https://chromium.googlesource.com/chromium/tools/depot_tools.git',
        ).apply('/some/fake/dir')
        want = {
            '/some/fake/dir': {
                'credential.https://chromium.googlesource.com/.helper':
                ['', 'luci'],
                'http.cookiefile': [''],
                'url.https://chromium.googlesource.com/chromium/tools/depot_tools.git.insteadof':
                [
                    'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
                ],
            },
        }
        self.assertEqual(scm.GIT._dump_config_state(), want)

    def test_apply_new_auth_sso(self):
        git_auth.ConfigChanger(
            mode=git_auth.ConfigMode.NEW_AUTH_SSO,
            remote_url=
            'https://chromium.googlesource.com/chromium/tools/depot_tools.git',
        ).apply('/some/fake/dir')
        want = {
            '/some/fake/dir': {
                'protocol.sso.allow': ['always'],
                'url.sso://chromium/.insteadof':
                ['https://chromium.googlesource.com/'],
                'http.cookiefile': [''],
            },
        }
        self.assertEqual(scm.GIT._dump_config_state(), want)

    def test_apply_no_auth(self):
        git_auth.ConfigChanger(
            mode=git_auth.ConfigMode.NO_AUTH,
            remote_url=
            'https://chromium.googlesource.com/chromium/tools/depot_tools.git',
        ).apply('/some/fake/dir')
        want = {
            '/some/fake/dir': {},
        }
        self.assertEqual(scm.GIT._dump_config_state(), want)

    def test_apply_chain_sso_new(self):
        git_auth.ConfigChanger(
            mode=git_auth.ConfigMode.NEW_AUTH_SSO,
            remote_url=
            'https://chromium.googlesource.com/chromium/tools/depot_tools.git',
        ).apply('/some/fake/dir')
        git_auth.ConfigChanger(
            mode=git_auth.ConfigMode.NEW_AUTH,
            remote_url=
            'https://chromium.googlesource.com/chromium/tools/depot_tools.git',
        ).apply('/some/fake/dir')
        want = {
            '/some/fake/dir': {
                'credential.https://chromium.googlesource.com/.helper':
                ['', 'luci'],
                'http.cookiefile': [''],
                'url.https://chromium.googlesource.com/chromium/tools/depot_tools.git.insteadof':
                [
                    'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
                ],
            },
        }
        self.assertEqual(scm.GIT._dump_config_state(), want)

    def test_apply_chain_new_sso(self):
        git_auth.ConfigChanger(
            mode=git_auth.ConfigMode.NEW_AUTH,
            remote_url=
            'https://chromium.googlesource.com/chromium/tools/depot_tools.git',
        ).apply('/some/fake/dir')
        git_auth.ConfigChanger(
            mode=git_auth.ConfigMode.NEW_AUTH_SSO,
            remote_url=
            'https://chromium.googlesource.com/chromium/tools/depot_tools.git',
        ).apply('/some/fake/dir')
        want = {
            '/some/fake/dir': {
                'protocol.sso.allow': ['always'],
                'url.sso://chromium/.insteadof':
                ['https://chromium.googlesource.com/'],
                'http.cookiefile': [''],
            },
        }
        self.assertEqual(scm.GIT._dump_config_state(), want)

    def test_apply_chain_new_no(self):
        git_auth.ConfigChanger(
            mode=git_auth.ConfigMode.NEW_AUTH,
            remote_url=
            'https://chromium.googlesource.com/chromium/tools/depot_tools.git',
        ).apply('/some/fake/dir')
        git_auth.ConfigChanger(
            mode=git_auth.ConfigMode.NO_AUTH,
            remote_url=
            'https://chromium.googlesource.com/chromium/tools/depot_tools.git',
        ).apply('/some/fake/dir')
        want = {
            '/some/fake/dir': {},
        }
        self.assertEqual(scm.GIT._dump_config_state(), want)

    def test_apply_chain_sso_no(self):
        git_auth.ConfigChanger(
            mode=git_auth.ConfigMode.NEW_AUTH_SSO,
            remote_url=
            'https://chromium.googlesource.com/chromium/tools/depot_tools.git',
        ).apply('/some/fake/dir')
        git_auth.ConfigChanger(
            mode=git_auth.ConfigMode.NO_AUTH,
            remote_url=
            'https://chromium.googlesource.com/chromium/tools/depot_tools.git',
        ).apply('/some/fake/dir')
        want = {
            '/some/fake/dir': {},
        }
        self.assertEqual(scm.GIT._dump_config_state(), want)

    def test_apply_global_new_auth(self):
        git_auth.ConfigChanger(
            mode=git_auth.ConfigMode.NEW_AUTH,
            remote_url=
            'https://chromium.googlesource.com/chromium/tools/depot_tools.git',
        ).apply_global('/some/fake/dir')
        want = {
            'credential.https://chromium.googlesource.com/.helper':
            ['', 'luci'],
        }
        self.assertEqual(self.global_state, want)

    def test_apply_global_new_auth_sso(self):
        git_auth.ConfigChanger(
            mode=git_auth.ConfigMode.NEW_AUTH_SSO,
            remote_url=
            'https://chromium.googlesource.com/chromium/tools/depot_tools.git',
        ).apply_global('/some/fake/dir')
        want = {
            'protocol.sso.allow': ['always'],
            'url.sso://chromium/.insteadof':
            ['https://chromium.googlesource.com/'],
        }
        self.assertEqual(self.global_state, want)

    def test_apply_global_chain_sso_new(self):
        git_auth.ConfigChanger(
            mode=git_auth.ConfigMode.NEW_AUTH_SSO,
            remote_url=
            'https://chromium.googlesource.com/chromium/tools/depot_tools.git',
        ).apply_global('/some/fake/dir')
        git_auth.ConfigChanger(
            mode=git_auth.ConfigMode.NEW_AUTH,
            remote_url=
            'https://chromium.googlesource.com/chromium/tools/depot_tools.git',
        ).apply_global('/some/fake/dir')
        want = {
            'protocol.sso.allow': ['always'],
            'credential.https://chromium.googlesource.com/.helper':
            ['', 'luci'],
        }
        self.assertEqual(self.global_state, want)


if __name__ == '__main__':
    logging.basicConfig(
        level=logging.DEBUG if '-v' in sys.argv else logging.ERROR)
    unittest.main()
