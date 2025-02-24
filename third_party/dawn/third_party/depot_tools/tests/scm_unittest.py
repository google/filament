#!/usr/bin/env vpython3
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for scm.py."""

from __future__ import annotations

import logging
import os
import sys
import tempfile
import threading
from collections import defaultdict
import unittest
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from testing_support import fake_repos

import scm
import subprocess
import subprocess2


def callError(code=1, cmd='', cwd='', stdout=b'', stderr=b''):
    return subprocess2.CalledProcessError(code, cmd, cwd, stdout, stderr)


class GitWrapperTestCase(unittest.TestCase):
    def setUp(self):
        super(GitWrapperTestCase, self).setUp()
        self.root_dir = '/foo/bar'

    def testRefToRemoteRef(self):
        remote = 'origin'
        refs = {
            'refs/branch-heads/1234': ('refs/remotes/branch-heads/', '1234'),
            # local refs for upstream branch
            'refs/remotes/%s/foobar' % remote:
            ('refs/remotes/%s/' % remote, 'foobar'),
            '%s/foobar' % remote: ('refs/remotes/%s/' % remote, 'foobar'),
            # upstream ref for branch
            'refs/heads/foobar': ('refs/remotes/%s/' % remote, 'foobar'),
            # could be either local or upstream ref, assumed to refer to
            # upstream, but probably don't want to encourage refs like this.
            'heads/foobar': ('refs/remotes/%s/' % remote, 'foobar'),
            # underspecified, probably intended to refer to a local branch
            'foobar':
            None,
            # tags and other refs
            'refs/tags/TAG':
            None,
            'refs/changes/34/1234':
            None,
        }
        for k, v in refs.items():
            r = scm.GIT.RefToRemoteRef(k, remote)
            self.assertEqual(r, v, msg='%s -> %s, expected %s' % (k, r, v))

    def testRemoteRefToRef(self):
        remote = 'origin'
        refs = {
            'refs/remotes/branch-heads/1234': 'refs/branch-heads/1234',
            # local refs for upstream branch
            'refs/remotes/origin/foobar': 'refs/heads/foobar',
            # tags and other refs
            'refs/tags/TAG': 'refs/tags/TAG',
            'refs/changes/34/1234': 'refs/changes/34/1234',
            # different remote
            'refs/remotes/other-remote/foobar': None,
            # underspecified, probably intended to refer to a local branch
            'heads/foobar': None,
            'origin/foobar': None,
            'foobar': None,
            None: None,
        }
        for k, v in refs.items():
            r = scm.GIT.RemoteRefToRef(k, remote)
            self.assertEqual(r, v, msg='%s -> %s, expected %s' % (k, r, v))

    @mock.patch('scm.GIT.Capture')
    @mock.patch('os.path.exists', lambda _: True)
    def testGetRemoteHeadRefLocal(self, mockCapture):
        mockCapture.side_effect = ['refs/remotes/origin/main']
        self.assertEqual(
            'refs/remotes/origin/main',
            scm.GIT.GetRemoteHeadRef('foo', 'proto://url', 'origin'))
        self.assertEqual(mockCapture.call_count, 1)

    @mock.patch('scm.GIT.Capture')
    @mock.patch('os.path.exists', lambda _: True)
    def testGetRemoteHeadRefLocalUpdateHead(self, mockCapture):
        mockCapture.side_effect = [
            'refs/remotes/origin/master',  # first symbolic-ref call
            'foo',  # set-head call
            'refs/remotes/origin/main',  # second symbolic-ref call
        ]
        self.assertEqual(
            'refs/remotes/origin/main',
            scm.GIT.GetRemoteHeadRef('foo', 'proto://url', 'origin'))
        self.assertEqual(mockCapture.call_count, 3)

    @mock.patch('scm.GIT.Capture')
    @mock.patch('os.path.exists', lambda _: True)
    def testGetRemoteHeadRefRemote(self, mockCapture):
        mockCapture.side_effect = [
            subprocess2.CalledProcessError(1, '', '', '', ''),
            subprocess2.CalledProcessError(1, '', '', '', ''),
            'ref: refs/heads/main\tHEAD\n' +
            '0000000000000000000000000000000000000000\tHEAD',
        ]
        self.assertEqual(
            'refs/remotes/origin/main',
            scm.GIT.GetRemoteHeadRef('foo', 'proto://url', 'origin'))
        self.assertEqual(mockCapture.call_count, 3)

    @mock.patch('scm.GIT.Capture')
    def testIsVersioned(self, mockCapture):
        mockCapture.return_value = (
            '160000 blob 423dc77d2182cb2687c53598a1dcef62ea2804ae   dir')
        actual_state = scm.GIT.IsVersioned('cwd', 'dir')
        self.assertEqual(actual_state, scm.VERSIONED_SUBMODULE)

        mockCapture.return_value = ''
        actual_state = scm.GIT.IsVersioned('cwd', 'dir')
        self.assertEqual(actual_state, scm.VERSIONED_NO)

        mockCapture.return_value = (
            '040000 tree ef016abffb316e47a02af447bc51342dcef6f3ca    dir')
        actual_state = scm.GIT.IsVersioned('cwd', 'dir')
        self.assertEqual(actual_state, scm.VERSIONED_DIR)

    @mock.patch('os.path.exists', return_value=True)
    @mock.patch('scm.GIT.Capture')
    def testListSubmodules(self, mockCapture, *_mock):
        mockCapture.return_value = (
            'submodule.submodulename.path foo/path/script'
            '\nsubmodule.submodule2name.path foo/path/script2')
        actual_list = scm.GIT.ListSubmodules('root')
        if sys.platform.startswith('win'):
            self.assertEqual(actual_list,
                             ['foo\\path\\script', 'foo\\path\\script2'])
        else:
            self.assertEqual(actual_list,
                             ['foo/path/script', 'foo/path/script2'])

    def testListSubmodules_missing(self):
        self.assertEqual(scm.GIT.ListSubmodules('root'), [])

    @mock.patch('os.path.exists', return_value=True)
    @mock.patch('scm.GIT.Capture')
    def testListSubmodules_empty(self, mockCapture, *_mock):
        mockCapture.side_effect = [
            subprocess2.CalledProcessError(1, '', '', '', ''),
        ]
        self.assertEqual(scm.GIT.ListSubmodules('root'), [])



class RealGitTest(fake_repos.FakeReposTestBase):
    def setUp(self):
        super(RealGitTest, self).setUp()
        self.enabled = self.FAKE_REPOS.set_up_git()
        if self.enabled:
            self.cwd = scm.os.path.join(self.FAKE_REPOS.git_base, 'repo_1')
        else:
            self.skipTest('git fake repos not available')

    def testResolveCommit(self):
        with self.assertRaises(Exception):
            scm.GIT.ResolveCommit(self.cwd, 'zebra')
        with self.assertRaises(Exception):
            scm.GIT.ResolveCommit(self.cwd, 'r123456')
        first_rev = self.githash('repo_1', 1)
        self.assertEqual(first_rev, scm.GIT.ResolveCommit(self.cwd, first_rev))
        self.assertEqual(self.githash('repo_1', 2),
                         scm.GIT.ResolveCommit(self.cwd, 'HEAD'))

    def testIsValidRevision(self):
        # Sha1's are [0-9a-z]{32}, so starting with a 'z' or 'r' should always
        # fail.
        self.assertFalse(scm.GIT.IsValidRevision(cwd=self.cwd, rev='zebra'))
        self.assertFalse(scm.GIT.IsValidRevision(cwd=self.cwd, rev='r123456'))
        # Valid cases
        first_rev = self.githash('repo_1', 1)
        self.assertTrue(scm.GIT.IsValidRevision(cwd=self.cwd, rev=first_rev))
        self.assertTrue(scm.GIT.IsValidRevision(cwd=self.cwd, rev='HEAD'))

    def testIsAncestor(self):
        self.assertTrue(
            scm.GIT.IsAncestor(self.githash('repo_1', 1),
                               self.githash('repo_1', 2),
                               cwd=self.cwd))
        self.assertFalse(
            scm.GIT.IsAncestor(self.githash('repo_1', 2),
                               self.githash('repo_1', 1),
                               cwd=self.cwd))
        self.assertFalse(scm.GIT.IsAncestor(self.githash('repo_1', 1), 'zebra'))

    def testGetAllFiles(self):
        self.assertEqual(['DEPS', 'foo bar', 'origin'],
                         scm.GIT.GetAllFiles(self.cwd))

    def testScopedConfig(self):
        scm.GIT.SetConfig(self.cwd,
                          "diff.test-key",
                          value="some value",
                          scope="global")
        self.assertEqual(scm.GIT.GetConfig(self.cwd, "diff.test-key", None),
                         "some value")
        self.assertEqual(
            scm.GIT.GetConfig(self.cwd, "diff.test-key", None, scope="local"),
            None)

        scm.GIT.SetConfig(self.cwd,
                          "diff.test-key1",
                          value="some value",
                          scope="local")
        self.assertEqual(scm.GIT.GetConfig(self.cwd, "diff.test-key1", None),
                         "some value")
        self.assertEqual(
            scm.GIT.GetConfig(self.cwd, "diff.test-key1", None, scope="local"),
            "some value")

    def testGetSetConfig(self):
        key = 'scm.test-key'

        self.assertIsNone(scm.GIT.GetConfig(self.cwd, key))
        self.assertEqual('default-value',
                         scm.GIT.GetConfig(self.cwd, key, 'default-value'))

        scm.GIT.SetConfig(self.cwd, key, 'set-value')
        self.assertEqual('set-value', scm.GIT.GetConfig(self.cwd, key))
        self.assertEqual('set-value',
                         scm.GIT.GetConfig(self.cwd, key, 'default-value'))

        scm.GIT.SetConfig(self.cwd, key, '')
        self.assertEqual('', scm.GIT.GetConfig(self.cwd, key))
        self.assertEqual('', scm.GIT.GetConfig(self.cwd, key, 'default-value'))

        # Clear the cache because we externally manipulate the git config with
        # the subprocess call.
        scm.GIT.drop_config_cache()
        subprocess.run(['git', 'config', key, 'line 1\nline 2\nline 3'],
                       cwd=self.cwd)
        self.assertEqual('line 1\nline 2\nline 3',
                         scm.GIT.GetConfig(self.cwd, key))
        self.assertEqual('line 1\nline 2\nline 3',
                         scm.GIT.GetConfig(self.cwd, key, 'default-value'))

        scm.GIT.SetConfig(self.cwd, key)
        self.assertIsNone(scm.GIT.GetConfig(self.cwd, key))
        self.assertEqual('default-value',
                         scm.GIT.GetConfig(self.cwd, key, 'default-value'))

        # Test missing_ok
        key = 'scm.missing-key'
        with self.assertRaises(scm.GitConfigUnsetMissingValue):
            scm.GIT.SetConfig(self.cwd, key, None, missing_ok=False)
        with self.assertRaises(scm.GitConfigUnsetMissingValue):
            scm.GIT.SetConfig(self.cwd,
                              key,
                              None,
                              modify_all=True,
                              missing_ok=False)
        with self.assertRaises(scm.GitConfigUnsetMissingValue):
            scm.GIT.SetConfig(self.cwd,
                              key,
                              None,
                              value_pattern='some_value',
                              missing_ok=False)

        scm.GIT.SetConfig(self.cwd, key, None)
        scm.GIT.SetConfig(self.cwd, key, None, modify_all=True)
        scm.GIT.SetConfig(self.cwd, key, None, value_pattern='some_value')

    def testGetSetConfigBool(self):
        key = 'scm.test-key'
        self.assertFalse(scm.GIT.GetConfigBool(self.cwd, key))

        scm.GIT.SetConfig(self.cwd, key, 'true')
        self.assertTrue(scm.GIT.GetConfigBool(self.cwd, key))

        scm.GIT.SetConfig(self.cwd, key)
        self.assertFalse(scm.GIT.GetConfigBool(self.cwd, key))

    def testGetSetConfigList(self):
        key = 'scm.test-key'
        self.assertListEqual([], scm.GIT.GetConfigList(self.cwd, key))

        scm.GIT.SetConfig(self.cwd, key, 'foo')
        scm.GIT.Capture(['config', '--add', key, 'bar'], cwd=self.cwd)
        self.assertListEqual(['foo', 'bar'],
                             scm.GIT.GetConfigList(self.cwd, key))

        scm.GIT.SetConfig(self.cwd, key, modify_all=True, value_pattern='^f')
        self.assertListEqual(['bar'], scm.GIT.GetConfigList(self.cwd, key))

        scm.GIT.SetConfig(self.cwd, key)
        self.assertListEqual([], scm.GIT.GetConfigList(self.cwd, key))

    def testYieldConfigRegexp(self):
        key1 = 'scm.aaa'
        key2 = 'scm.aaab'

        config = scm.GIT.YieldConfigRegexp(self.cwd, key1)
        with self.assertRaises(StopIteration):
            next(config)

        scm.GIT.SetConfig(self.cwd, key1, 'foo')
        scm.GIT.SetConfig(self.cwd, key2, 'bar')
        scm.GIT.Capture(['config', '--add', key2, 'baz'], cwd=self.cwd)

        config = scm.GIT.YieldConfigRegexp(self.cwd, '^scm\\.aaa')
        self.assertEqual((key1, 'foo'), next(config))
        self.assertEqual((key2, 'bar'), next(config))
        self.assertEqual((key2, 'baz'), next(config))
        with self.assertRaises(StopIteration):
            next(config)

    def testGetSetBranchConfig(self):
        branch = scm.GIT.GetBranch(self.cwd)
        key = 'scm.test-key'

        self.assertIsNone(scm.GIT.GetBranchConfig(self.cwd, branch, key))
        self.assertEqual(
            'default-value',
            scm.GIT.GetBranchConfig(self.cwd, branch, key, 'default-value'))

        scm.GIT.SetBranchConfig(self.cwd, branch, key, 'set-value')
        self.assertEqual('set-value',
                         scm.GIT.GetBranchConfig(self.cwd, branch, key))
        self.assertEqual(
            'set-value',
            scm.GIT.GetBranchConfig(self.cwd, branch, key, 'default-value'))
        self.assertEqual(
            'set-value',
            scm.GIT.GetConfig(self.cwd, 'branch.%s.%s' % (branch, key)))

        scm.GIT.SetBranchConfig(self.cwd, branch, key)
        self.assertIsNone(scm.GIT.GetBranchConfig(self.cwd, branch, key))

    def testFetchUpstreamTuple_NoUpstreamFound(self):
        self.assertEqual((None, None), scm.GIT.FetchUpstreamTuple(self.cwd))

    @mock.patch('scm.GIT.GetRemoteBranches', return_value=['origin/main'])
    def testFetchUpstreamTuple_GuessOriginMaster(self, _mockGetRemoteBranches):
        self.assertEqual(('origin', 'refs/heads/main'),
                         scm.GIT.FetchUpstreamTuple(self.cwd))

    @mock.patch('scm.GIT.GetRemoteBranches',
                return_value=['origin/master', 'origin/main'])
    def testFetchUpstreamTuple_GuessOriginMain(self, _mockGetRemoteBranches):
        self.assertEqual(('origin', 'refs/heads/main'),
                         scm.GIT.FetchUpstreamTuple(self.cwd))

    def testFetchUpstreamTuple_RietveldUpstreamConfig(self):
        scm.GIT.SetConfig(self.cwd, 'rietveld.upstream-branch',
                          'rietveld-upstream')
        scm.GIT.SetConfig(self.cwd, 'rietveld.upstream-remote',
                          'rietveld-remote')
        self.assertEqual(('rietveld-remote', 'rietveld-upstream'),
                         scm.GIT.FetchUpstreamTuple(self.cwd))
        scm.GIT.SetConfig(self.cwd, 'rietveld.upstream-branch')
        scm.GIT.SetConfig(self.cwd, 'rietveld.upstream-remote')

    @mock.patch('scm.GIT.GetBranch', side_effect=callError())
    def testFetchUpstreamTuple_NotOnBranch(self, _mockGetBranch):
        scm.GIT.SetConfig(self.cwd, 'rietveld.upstream-branch',
                          'rietveld-upstream')
        scm.GIT.SetConfig(self.cwd, 'rietveld.upstream-remote',
                          'rietveld-remote')
        self.assertEqual(('rietveld-remote', 'rietveld-upstream'),
                         scm.GIT.FetchUpstreamTuple(self.cwd))
        scm.GIT.SetConfig(self.cwd, 'rietveld.upstream-branch')
        scm.GIT.SetConfig(self.cwd, 'rietveld.upstream-remote')

    def testFetchUpstreamTuple_BranchConfig(self):
        branch = scm.GIT.GetBranch(self.cwd)
        scm.GIT.SetBranchConfig(self.cwd, branch, 'merge', 'branch-merge')
        scm.GIT.SetBranchConfig(self.cwd, branch, 'remote', 'branch-remote')
        self.assertEqual(('branch-remote', 'branch-merge'),
                         scm.GIT.FetchUpstreamTuple(self.cwd))
        scm.GIT.SetBranchConfig(self.cwd, branch, 'merge')
        scm.GIT.SetBranchConfig(self.cwd, branch, 'remote')

    def testFetchUpstreamTuple_AnotherBranchConfig(self):
        branch = 'scm-test-branch'
        scm.GIT.SetBranchConfig(self.cwd, branch, 'merge', 'other-merge')
        scm.GIT.SetBranchConfig(self.cwd, branch, 'remote', 'other-remote')
        self.assertEqual(('other-remote', 'other-merge'),
                         scm.GIT.FetchUpstreamTuple(self.cwd, branch))
        scm.GIT.SetBranchConfig(self.cwd, branch, 'merge')
        scm.GIT.SetBranchConfig(self.cwd, branch, 'remote')

    def testGetBranchRef(self):
        self.assertEqual('refs/heads/main', scm.GIT.GetBranchRef(self.cwd))
        HEAD = scm.GIT.Capture(['rev-parse', 'HEAD'], cwd=self.cwd)
        scm.GIT.Capture(['checkout', HEAD], cwd=self.cwd)
        self.assertIsNone(scm.GIT.GetBranchRef(self.cwd))
        scm.GIT.Capture(['checkout', 'main'], cwd=self.cwd)

    def testGetBranch(self):
        self.assertEqual('main', scm.GIT.GetBranch(self.cwd))
        HEAD = scm.GIT.Capture(['rev-parse', 'HEAD'], cwd=self.cwd)
        scm.GIT.Capture(['checkout', HEAD], cwd=self.cwd)
        self.assertIsNone(scm.GIT.GetBranchRef(self.cwd))
        scm.GIT.Capture(['checkout', 'main'], cwd=self.cwd)


class DiffTestCase(unittest.TestCase):

    def setUp(self):
        self.root = tempfile.mkdtemp()

        os.makedirs(os.path.join(self.root, "foo", "dir"))
        with open(os.path.join(self.root, "foo", "file.txt"), "w") as f:
            f.write("foo\n")
        with open(os.path.join(self.root, "foo", "dir", "file.txt"), "w") as f:
            f.write("foo dir\n")

        os.makedirs(os.path.join(self.root, "baz_repo"))
        with open(os.path.join(self.root, "baz_repo", "file.txt"), "w") as f:
            f.write("baz\n")

    @mock.patch('scm.GIT.ListSubmodules')
    def testGetAllFiles_ReturnsAllFilesIfNoSubmodules(self, mockListSubmodules):
        mockListSubmodules.return_value = []
        files = scm.DIFF.GetAllFiles(self.root)

        if sys.platform.startswith('win'):
            self.assertCountEqual(
                files,
                ["foo\\file.txt", "foo\\dir\\file.txt", "baz_repo\\file.txt"])
        else:
            self.assertCountEqual(
                files,
                ["foo/file.txt", "foo/dir/file.txt", "baz_repo/file.txt"])

    @mock.patch('scm.GIT.ListSubmodules')
    def testGetAllFiles_IgnoresFilesInSubmodules(self, mockListSubmodules):
        mockListSubmodules.return_value = ['baz_repo']
        files = scm.DIFF.GetAllFiles(self.root)

        if sys.platform.startswith('win'):
            self.assertCountEqual(
                files, ["foo\\file.txt", "foo\\dir\\file.txt", "baz_repo"])
        else:
            self.assertCountEqual(
                files, ["foo/file.txt", "foo/dir/file.txt", "baz_repo"])


class GitConfigStateTestTest(unittest.TestCase):

    @staticmethod
    def _make(*,
              global_state: dict[str, list[str]] | None = None,
              system_state: dict[str, list[str]] | None = None):
        """_make constructs a GitConfigStateTest with an internal Lock.

        If global_state is None, an empty dictionary will be constructed and
        returned, otherwise the caller's provided global_state is returned,
        unmodified.

        Returns (GitConfigStateTest, global_state) - access to global_state must
        be manually synchronized with access to GitConfigStateTest, or at least
        with GitConfigStateTest.global_state_lock.
        """
        global_state = global_state or {}
        m = scm.GitConfigStateTest(threading.Lock(),
                                   global_state,
                                   system_state=system_state)
        return m, global_state

    def test_construction_empty(self):
        m, gs = self._make()
        self.assertDictEqual(gs, {})
        self.assertDictEqual(m.load_config(), {})

        gs['section.key'] = ['override']
        self.assertDictEqual(
            m.load_config(), {
                "global": {
                    'section.key': ['override']
                },
                "default": {
                    'section.key': ['override']
                },
            })

    def defaultdict_to_dict(self, d):
        if isinstance(d, defaultdict):
            return {k: self.defaultdict_to_dict(v) for k, v in d.items()}
        return d

    def test_construction_global(self):

        m, gs = self._make(global_state={
            'section.key': ['global'],
        })
        self.assertDictEqual(self.defaultdict_to_dict(gs),
                             {'section.key': ['global']})
        self.assertDictEqual(
            self.defaultdict_to_dict(m.load_config()), {
                "global": {
                    'section.key': ['global']
                },
                "default": {
                    'section.key': ['global']
                },
            })

        gs['section.key'] = ['override']
        self.assertDictEqual(
            self.defaultdict_to_dict(m.load_config()), {
                "global": {
                    'section.key': ['override']
                },
                "default": {
                    'section.key': ['override']
                },
            })

    @mock.patch('git_common.get_git_version')
    @mock.patch('scm.GIT.Capture')
    def test_load_config_git_version_2_old(self, mock_capture,
                                           mock_get_git_version):
        mock_get_git_version.return_value = (2, 25)
        mock_capture.return_value = ""

        config_state = scm.GitConfigStateReal("/fake/path")
        config_state.load_config()

        mock_capture.assert_called_once_with(['config', '--list', '-z'],
                                             cwd=config_state.root,
                                             strip_out=False)

    @mock.patch('git_common.get_git_version')
    @mock.patch('scm.GIT.Capture')
    def test_load_config_git_version_new(self, mock_capture,
                                         mock_get_git_version):
        mock_get_git_version.return_value = (2, 26)
        mock_capture.return_value = ""

        config_state = scm.GitConfigStateReal("/fake/path")
        config_state.load_config()

        mock_capture.assert_called_once_with(
            ['config', '--list', '-z', '--show-scope'],
            cwd=config_state.root,
            strip_out=False)

    def test_construction_system(self):
        m, gs = self._make(
            global_state={'section.key': ['global']},
            system_state={'section.key': ['system']},
        )
        self.assertDictEqual(self.defaultdict_to_dict(gs),
                             {'section.key': ['global']})
        self.assertDictEqual(
            self.defaultdict_to_dict(m.load_config()), {
                'default': {
                    'section.key': ['system', 'global']
                },
                "global": {
                    'section.key': ['global']
                },
                "system": {
                    'section.key': ['system']
                }
            })

        gs['section.key'] = ['override']
        self.assertDictEqual(
            self.defaultdict_to_dict(m.load_config()), {
                "global": {
                    'section.key': ['override']
                },
                "system": {
                    'section.key': ['system']
                },
                'default': {
                    'section.key': ['system', 'override']
                }
            })

    def test_set_config_system(self):
        m, _ = self._make()

        with self.assertRaises(scm.GitConfigUneditableScope):
            m.set_config('section.key',
                         'new_global',
                         append=False,
                         scope='system')

    def test_set_config_unknown(self):
        m, _ = self._make()

        with self.assertRaises(scm.GitConfigUnknownScope):
            m.set_config('section.key',
                         'new_global',
                         append=False,
                         scope='meepmorp')

    def test_set_config_global_append_empty(self):
        m, gs = self._make()
        self.assertDictEqual(gs, {})
        self.assertDictEqual(m.load_config(), {})

        m.set_config('section.key', 'new_global', append=True, scope='global')
        self.assertDictEqual(
            m.load_config(), {
                "default": {
                    'section.key': ['new_global']
                },
                "global": {
                    'section.key': ['new_global']
                }
            })

    def test_set_config_global(self):
        m, gs = self._make()
        self.assertDictEqual(gs, {})
        self.assertDictEqual(m.load_config(), {})

        m.set_config('section.key', 'new_global', append=False, scope='global')
        self.assertDictEqual(
            self.defaultdict_to_dict(m.load_config()), {
                "global": {
                    'section.key': ['new_global']
                },
                "default": {
                    'section.key': ['new_global']
                }
            })

        m.set_config('section.key', 'new_global2', append=True, scope='global')
        self.assertDictEqual(
            self.defaultdict_to_dict(m.load_config()), {
                "global": {
                    'section.key': ['new_global', 'new_global2']
                },
                "default": {
                    'section.key': ['new_global', 'new_global2']
                },
            })

        self.assertDictEqual(self.defaultdict_to_dict(gs),
                             {'section.key': ['new_global', 'new_global2']})

    def test_set_config_multi_global(self):
        m, gs = self._make(global_state={'section.key': ['1', '2']})

        m.set_config_multi('section.key',
                           'new_global',
                           value_pattern=None,
                           scope='global')
        self.assertDictEqual(
            self.defaultdict_to_dict(m.load_config()), {
                "default": {
                    'section.key': ['new_global']
                },
                "global": {
                    'section.key': ['new_global']
                }
            })

        self.assertDictEqual(gs, {'section.key': ['new_global']})

        m.set_config_multi('othersection.key',
                           'newval',
                           value_pattern=None,
                           scope='global')
        self.assertDictEqual(
            m.load_config(), {
                "global": {
                    'section.key': ['new_global'],
                    'othersection.key': ['newval'],
                },
                "default": {
                    'section.key': ['new_global'],
                    'othersection.key': ['newval'],
                }
            })

        self.assertDictEqual(gs, {
            'section.key': ['new_global'],
            'othersection.key': ['newval'],
        })

    def test_set_config_multi_global_pattern(self):
        m, _ = self._make(global_state={
            'section.key': ['1', '1', '2', '2', '2', '3'],
        })

        m.set_config_multi('section.key',
                           'new_global',
                           value_pattern='2',
                           scope='global')
        self.assertDictEqual(
            m.load_config(), {
                "global": {
                    'section.key': ['1', '1', 'new_global', '3']
                },
                "default": {
                    'section.key': ['1', '1', 'new_global', '3']
                }
            })

        m.set_config_multi('section.key',
                           'additional',
                           value_pattern='narp',
                           scope='global')
        self.assertDictEqual(
            m.load_config(), {
                "default": {
                    'section.key': ['1', '1', 'new_global', '3', 'additional']
                },
                "global": {
                    'section.key': ['1', '1', 'new_global', '3', 'additional']
                }
            })

    def test_unset_config_global(self):
        m, _ = self._make(global_state={
            'section.key': ['someval'],
        })

        m.unset_config('section.key', scope='global', missing_ok=False)
        self.assertDictEqual(m.load_config(), {})

        with self.assertRaises(scm.GitConfigUnsetMissingValue):
            m.unset_config('section.key', scope='global', missing_ok=False)

        self.assertDictEqual(m.load_config(), {})

        m.unset_config('section.key', scope='global', missing_ok=True)
        self.assertDictEqual(m.load_config(), {})

    def test_unset_config_global_extra(self):
        m, _ = self._make(global_state={
            'section.key': ['someval'],
            'extra': ['another'],
        })

        m.unset_config('section.key', scope='global', missing_ok=False)
        self.assertDictEqual(m.load_config(), {
            "global": {
                'extra': ['another']
            },
            "default": {
                'extra': ['another']
            }
        })

        with self.assertRaises(scm.GitConfigUnsetMissingValue):
            m.unset_config('section.key', scope='global', missing_ok=False)

        self.assertDictEqual(m.load_config(), {
            "global": {
                'extra': ['another']
            },
            "default": {
                'extra': ['another']
            }
        })

        m.unset_config('section.key', scope='global', missing_ok=True)
        self.assertDictEqual(m.load_config(), {
            "global": {
                'extra': ['another']
            },
            "default": {
                'extra': ['another']
            }
        })

    def test_unset_config_global_multi(self):
        m, _ = self._make(global_state={
            'section.key': ['1', '2'],
        })

        with self.assertRaises(scm.GitConfigUnsetMultipleValues):
            m.unset_config('section.key', scope='global', missing_ok=True)

    def test_unset_config_multi_global(self):
        m, _ = self._make(global_state={
            'section.key': ['1', '2'],
        })

        m.unset_config_multi('section.key',
                             value_pattern=None,
                             scope='global',
                             missing_ok=False)
        self.assertDictEqual(m.load_config(), {})

        with self.assertRaises(scm.GitConfigUnsetMissingValue):
            m.unset_config_multi('section.key',
                                 value_pattern=None,
                                 scope='global',
                                 missing_ok=False)

    def test_unset_config_multi_global_pattern(self):
        m, _ = self._make(global_state={
            'section.key': ['1', '2', '3', '1', '2'],
        })

        m.unset_config_multi('section.key',
                             value_pattern='2',
                             scope='global',
                             missing_ok=False)
        self.assertDictEqual(
            m.load_config(), {
                'global': {
                    'section.key': ['1', '3', '1'],
                },
                'default': {
                    'section.key': ['1', '3', '1'],
                }
            })


class CanonicalizeGitConfigKeyTest(unittest.TestCase):

    def setUp(self) -> None:
        self.ck = scm.canonicalize_git_config_key
        return super().setUp()

    def test_many(self):
        self.assertEqual(self.ck("URL.https://SoMeThInG.example.com.INSTEADOF"),
                         "url.https://SoMeThInG.example.com.insteadof")

    def test_three(self):
        self.assertEqual(self.ck("A.B.C"), "a.B.c")
        self.assertEqual(self.ck("a.B.C"), "a.B.c")
        self.assertEqual(self.ck("a.b.C"), "a.b.c")

    def test_two(self):
        self.assertEqual(self.ck("A.B"), "a.b")
        self.assertEqual(self.ck("a.B"), "a.b")
        self.assertEqual(self.ck("a.b"), "a.b")

    def test_one(self):
        with self.assertRaises(scm.GitConfigInvalidKey):
            self.ck("KEY")

    def test_zero(self):
        with self.assertRaises(scm.GitConfigInvalidKey):
            self.ck("")


class CachedGitConfigStateTest(unittest.TestCase):

    @staticmethod
    def _make():
        return scm.CachedGitConfigState(
            scm.GitConfigStateTest(threading.Lock(), {}))

    def test_empty(self):
        gcs = self._make()

        self.assertListEqual(list(gcs.YieldConfigRegexp()), [])

    def test_set_single(self):
        gcs = self._make()

        gcs.SetConfig('SECTION.VARIABLE', 'value')
        self.assertListEqual(list(gcs.YieldConfigRegexp()), [
            ('section.variable', 'value'),
        ])

    def test_set_append(self):
        gcs = self._make()

        gcs.SetConfig('SECTION.VARIABLE', 'value')
        gcs.SetConfig('SeCtIoN.vArIaBLe', 'value2', append=True)
        self.assertListEqual(list(gcs.YieldConfigRegexp()), [
            ('section.variable', 'value'),
            ('section.variable', 'value2'),
        ])

    def test_set_global(self):
        gcs = self._make()

        gcs.SetConfig('SECTION.VARIABLE', 'value')
        gcs.SetConfig('SeCtIoN.vArIaBLe', 'value2', append=True)

        gcs.SetConfig('SeCtIoN.vArIaBLe', 'gvalue', scope='global')
        self.assertListEqual(list(gcs.YieldConfigRegexp()), [
            ('section.variable', 'gvalue'),
            ('section.variable', 'value'),
            ('section.variable', 'value2'),
        ])

    def test_unset_multi_global(self):
        gcs = self._make()

        gcs.SetConfig('SECTION.VARIABLE', 'value')
        gcs.SetConfig('SeCtIoN.vArIaBLe', 'value2', append=True)
        gcs.SetConfig('SeCtIoN.vArIaBLe', 'gvalue', scope='global')
        self.assertListEqual(list(gcs.YieldConfigRegexp()), [
            ('section.variable', 'gvalue'),
            ('section.variable', 'value'),
            ('section.variable', 'value2'),
        ])

        gcs.SetConfig('section.variable', None, modify_all=True)
        self.assertListEqual(list(gcs.YieldConfigRegexp()), [
            ('section.variable', 'gvalue'),
        ])

    def test_errors(self):
        gcs = self._make()

        with self.assertRaises(scm.GitConfigInvalidKey):
            gcs.SetConfig('key', 'value')

        with self.assertRaises(scm.GitConfigUnknownScope):
            gcs.SetConfig('section.variable', 'value', scope='dude')

        with self.assertRaises(scm.GitConfigUneditableScope):
            gcs.SetConfig('section.variable', 'value', scope='system')

        with self.assertRaisesRegex(ValueError,
                                    'value_pattern.*modify_all.*invalid'):
            gcs.SetConfig('section.variable',
                          'value',
                          value_pattern='hi',
                          modify_all=False)

        with self.assertRaisesRegex(ValueError,
                                    'value_pattern.*append.*invalid'):
            gcs.SetConfig('section.variable',
                          'value',
                          value_pattern='hi',
                          modify_all=True,
                          append=True)

    def test_set_pattern(self):
        gcs = self._make()

        gcs.SetConfig('section.variable', 'value', append=True)
        gcs.SetConfig('section.variable', 'value_bleem', append=True)
        gcs.SetConfig('section.variable', 'value_bleem', append=True)
        gcs.SetConfig('section.variable', 'value_bleem', append=True)
        gcs.SetConfig('section.variable', 'value_bleem', append=True)
        gcs.SetConfig('section.variable', 'value', append=True)

        self.assertListEqual(list(gcs.YieldConfigRegexp()), [
            ('section.variable', 'value'),
            ('section.variable', 'value_bleem'),
            ('section.variable', 'value_bleem'),
            ('section.variable', 'value_bleem'),
            ('section.variable', 'value_bleem'),
            ('section.variable', 'value'),
        ])

        gcs.SetConfig('section.variable',
                      'poof',
                      value_pattern='.*_bleem',
                      modify_all=True)

        self.assertListEqual(list(gcs.YieldConfigRegexp()), [
            ('section.variable', 'value'),
            ('section.variable', 'poof'),
            ('section.variable', 'value'),
        ])

    def test_set_all(self):
        gcs = self._make()

        gcs.SetConfig('section.variable', 'value', append=True)
        gcs.SetConfig('section.variable', 'value_bleem', append=True)
        gcs.SetConfig('section.variable', 'value_bleem', append=True)
        gcs.SetConfig('section.variable', 'value_bleem', append=True)
        gcs.SetConfig('section.variable', 'value_bleem', append=True)
        gcs.SetConfig('section.variable', 'value', append=True)

        self.assertListEqual(list(gcs.YieldConfigRegexp()), [
            ('section.variable', 'value'),
            ('section.variable', 'value_bleem'),
            ('section.variable', 'value_bleem'),
            ('section.variable', 'value_bleem'),
            ('section.variable', 'value_bleem'),
            ('section.variable', 'value'),
        ])

        gcs.SetConfig('section.variable', 'poof', modify_all=True)

        self.assertListEqual(list(gcs.YieldConfigRegexp()), [
            ('section.variable', 'poof'),
        ])

    def test_get_config(self):
        gcs = self._make()

        gcs.SetConfig('section.variable', 'value', append=True)
        gcs.SetConfig('section.variable', 'value_bleem', append=True)

        self.assertEqual(gcs.GetConfig('section.varIABLE'), 'value_bleem')
        self.assertEqual(gcs.GetConfigBool('section.varIABLE'), False)

        self.assertEqual(gcs.GetConfig('section.noexist'), None)
        self.assertEqual(gcs.GetConfig('section.noexist', 'dflt'), 'dflt')

        gcs.SetConfig('section.variable', 'true', append=True)
        self.assertEqual(gcs.GetConfigBool('section.varIABLE'), True)

        self.assertListEqual(list(gcs.YieldConfigRegexp()), [
            ('section.variable', 'value'),
            ('section.variable', 'value_bleem'),
            ('section.variable', 'true'),
        ])

        self.assertListEqual(gcs.GetConfigList('seCTIon.vARIable'), [
            'value',
            'value_bleem',
            'true',
        ])


if __name__ == '__main__':
    if '-v' in sys.argv:
        logging.basicConfig(level=logging.DEBUG)
    unittest.main()

# vim: ts=4:sw=4:tw=80:et:
