#!/usr/bin/env vpython3
# coding=utf-8
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for git_cl.py."""

import codecs
import datetime
import json
import logging
import multiprocessing
import optparse
import os
import io
import shutil
import sys
import tempfile
import unittest

from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import scm_mock

import metrics
import metrics_utils
# We have to disable monitoring before importing git_cl.
metrics_utils.COLLECT_METRICS = False

import clang_format
import contextlib
import gclient_utils
import gerrit_util
import git_cl
import git_common
import git_footers
import git_new_branch
import owners_client
import scm
import subprocess2

# TODO: Should fix these warnings.
# pylint: disable=line-too-long

def callError(code=1, cmd='', cwd='', stdout=b'', stderr=b''):
    return subprocess2.CalledProcessError(code, cmd, cwd, stdout, stderr)


CERR1 = callError(1)


def getAccountDetailsMock(host, account_id='self'):
    if account_id == 'self':
        return {
            '_account_id': 123456,
            'avatars': [],
            'email': 'getAccountDetailsMock@example.com',
            'name': 'GetAccountDetails(self)',
            'status': 'OOO',
        }
    return None


class TemporaryFileMock(object):
    def __init__(self):
        self.suffix = 0

    @contextlib.contextmanager
    def __call__(self):
        self.suffix += 1
        yield '/tmp/fake-temp' + str(self.suffix)


class ChangelistMock(object):
    # A class variable so we can access it when we don't have access to the
    # instance that's being set.
    desc = ''

    def __init__(self, gerrit_change=None, use_python3=False, **kwargs):
        self._gerrit_change = gerrit_change
        self._use_python3 = use_python3

    def GetIssue(self):
        return 1

    def FetchDescription(self):
        return ChangelistMock.desc

    def UpdateDescription(self, desc, force=False):
        ChangelistMock.desc = desc

    def GetGerritChange(self, patchset=None, **kwargs):
        del patchset
        return self._gerrit_change

    def GetRemoteBranch(self):
        return ('origin', 'refs/remotes/origin/main')


class WatchlistsMock(object):
    def __init__(self, _):
        pass

    @staticmethod
    def GetWatchersForPaths(_):
        return ['joe@example.com']


class CodereviewSettingsFileMock(object):
    def __init__(self):
        pass

    # pylint: disable=no-self-use
    def read(self):
        return ('CODE_REVIEW_SERVER: gerrit.chromium.org\n' +
                'GERRIT_HOST: True\n')


class AuthenticatorMock(object):
    def __init__(self, *_args):
        pass

    def has_cached_credentials(self):
        return True

    def authorize(self, http):
        return http


def CookiesAuthenticatorMockFactory(hosts_with_creds=None, same_auth=False):
    """Use to mock Gerrit/Git credentials from ~/.gitcookies.

    Usage:
        >>> self.mock(git_cl.gerrit_util, "CookiesAuthenticator",
                    CookiesAuthenticatorMockFactory({'host': ('user', 'pass')})

    OR
        >>> self.mock(git_cl.gerrit_util, "CookiesAuthenticator",
                    CookiesAuthenticatorMockFactory(
                        same_auth=('user', 'pass'))
    """
    class CookiesAuthenticatorMock(git_cl.gerrit_util.CookiesAuthenticator):
        def __init__(self):  # pylint: disable=super-init-not-called
            # Intentionally not calling super() because it reads actual cookie
            # files.
            pass

        @classmethod
        def get_gitcookies_path(cls):
            return os.path.join('~', '.gitcookies')

        def _get_auth_for_host(self, host):
            if same_auth:
                return same_auth
            return (hosts_with_creds or {}).get(host)

    return CookiesAuthenticatorMock


class MockChangelistWithBranchAndIssue():
    def __init__(self, branch, issue):
        self.branch = branch
        self.issue = issue

    def GetBranch(self):
        return self.branch

    def GetIssue(self):
        return self.issue


class SystemExitMock(Exception):
    pass


class ParserErrorMock(Exception):
    pass


class TestGitClBasic(unittest.TestCase):
    def setUp(self):
        mock.patch('sys.exit', side_effect=SystemExitMock).start()
        mock.patch('sys.stdout', io.StringIO()).start()
        mock.patch('sys.stderr', io.StringIO()).start()
        self.addCleanup(mock.patch.stopall)

    def test_die_with_error(self):
        with self.assertRaises(SystemExitMock):
            git_cl.DieWithError('foo', git_cl.ChangeDescription('lorem ipsum'))
        self.assertEqual(sys.stderr.getvalue(), 'foo\n')
        self.assertTrue('saving CL description' in sys.stdout.getvalue())
        self.assertTrue('Content of CL description' in sys.stdout.getvalue())
        self.assertTrue('lorem ipsum' in sys.stdout.getvalue())
        sys.exit.assert_called_once_with(1)

    def test_die_with_error_no_desc(self):
        with self.assertRaises(SystemExitMock):
            git_cl.DieWithError('foo')
        self.assertEqual(sys.stderr.getvalue(), 'foo\n')
        self.assertEqual(sys.stdout.getvalue(), '')
        sys.exit.assert_called_once_with(1)

    def test_fetch_description(self):
        cl = git_cl.Changelist(issue=1, codereview_host='host')
        cl.description = 'x'
        self.assertEqual(cl.FetchDescription(), 'x')

    @mock.patch('git_cl.Changelist.EnsureAuthenticated')
    @mock.patch('git_cl.Changelist.GetStatus', lambda cl: cl.status)
    def test_get_cl_statuses(self, *_mocks):
        statuses = [
            'closed', 'commit', 'dry-run', 'lgtm', 'reply', 'unsent', 'waiting'
        ]
        changes = []
        for status in statuses:
            cl = git_cl.Changelist()
            cl.status = status
            changes.append(cl)

        actual = set(git_cl.get_cl_statuses(changes, True))
        self.assertEqual(set(zip(changes, statuses)), actual)

    def test_upload_to_non_default_branch_no_retry(self):
        m = mock.patch('git_cl.Changelist._CMDUploadChange',
                       side_effect=[git_cl.GitPushError(), None]).start()
        mock.patch('git_cl.Changelist.GetRemoteBranch',
                   return_value=('foo', 'bar')).start()
        mock.patch('git_cl.Changelist.GetGerritProject',
                   return_value='foo').start()
        mock.patch('git_cl.gerrit_util.GetProjectHead',
                   return_value='refs/heads/main').start()

        cl = git_cl.Changelist()
        options = optparse.Values()
        options.target_branch = 'refs/heads/bar'
        with self.assertRaises(SystemExitMock):
            cl.CMDUploadChange(options, [], 'foo',
                               git_cl.ChangeDescription('bar'))

        # ensure upload is called once
        self.assertEqual(len(m.mock_calls), 1)
        sys.exit.assert_called_once_with(1)
        # option not set as retry didn't happen
        self.assertFalse(hasattr(options, 'force'))
        self.assertFalse(hasattr(options, 'edit_description'))

    def test_upload_to_meta_config_branch_no_retry(self):
        m = mock.patch('git_cl.Changelist._CMDUploadChange',
                       side_effect=[git_cl.GitPushError(), None]).start()
        mock.patch('git_cl.Changelist.GetRemoteBranch',
                   return_value=('foo', 'bar')).start()
        mock.patch('git_cl.Changelist.GetGerritProject',
                   return_value='foo').start()
        mock.patch('git_cl.gerrit_util.GetProjectHead',
                   return_value='refs/heads/main').start()

        cl = git_cl.Changelist()
        options = optparse.Values()
        options.target_branch = 'refs/meta/config'
        with self.assertRaises(SystemExitMock):
            cl.CMDUploadChange(options, [], 'foo',
                               git_cl.ChangeDescription('bar'))

        # ensure upload is called once
        self.assertEqual(len(m.mock_calls), 1)
        sys.exit.assert_called_once_with(1)
        # option not set as retry didn't happen
        self.assertFalse(hasattr(options, 'force'))
        self.assertFalse(hasattr(options, 'edit_description'))

    def test_upload_to_old_default_still_active(self):
        m = mock.patch('git_cl.Changelist._CMDUploadChange',
                       side_effect=[git_cl.GitPushError(), None]).start()
        mock.patch('git_cl.Changelist.GetRemoteBranch',
                   return_value=('foo', git_cl.DEFAULT_OLD_BRANCH)).start()
        mock.patch('git_cl.Changelist.GetGerritProject',
                   return_value='foo').start()
        mock.patch('git_cl.gerrit_util.GetProjectHead',
                   return_value='refs/heads/main').start()

        cl = git_cl.Changelist()
        options = optparse.Values()
        options.target_branch = 'refs/heads/main'
        with self.assertRaises(SystemExitMock):
            cl.CMDUploadChange(options, [], 'foo',
                               git_cl.ChangeDescription('bar'))

        # ensure upload is called once
        self.assertEqual(len(m.mock_calls), 1)
        sys.exit.assert_called_once_with(1)
        # option not set as retry didn't happen
        self.assertFalse(hasattr(options, 'force'))
        self.assertFalse(hasattr(options, 'edit_description'))

    def test_upload_with_message_file_no_editor(self):
        m = mock.patch('git_cl.ChangeDescription.prompt',
                       return_value=None).start()
        mock.patch('git_cl.Changelist.GetRemoteBranch',
                   return_value=('foo', git_cl.DEFAULT_NEW_BRANCH)).start()
        mock.patch('git_cl.GetTargetRef',
                   return_value='refs/heads/main').start()
        mock.patch('git_cl.Changelist._GerritCommitMsgHookCheck',
                   lambda offer_removal: None).start()
        mock.patch('git_cl.Changelist.GetIssue', return_value=None).start()
        mock.patch('git_cl.Changelist.GetBranch',
                   side_effect=SystemExitMock).start()
        mock.patch('git_cl.GenerateGerritChangeId', return_value=None).start()
        mock.patch('git_cl.RunGit').start()

        cl = git_cl.Changelist()
        options = optparse.Values()
        options.target_branch = 'refs/heads/main'
        options.squash = True
        options.edit_description = False
        options.force = False
        options.preserve_tryjobs = False
        options.message_file = "message.txt"
        options.commit_description = None

        with self.assertRaises(SystemExitMock):
            cl.CMDUploadChange(options, [], 'foo',
                               git_cl.ChangeDescription('bar'))
        self.assertEqual(len(m.mock_calls), 0)

        options.message_file = None
        with self.assertRaises(SystemExitMock):
            cl.CMDUploadChange(options, [], 'foo',
                               git_cl.ChangeDescription('bar'))
        self.assertEqual(len(m.mock_calls), 1)

    def test_get_cl_statuses_no_changes(self):
        self.assertEqual([], list(git_cl.get_cl_statuses([], True)))

    @mock.patch('git_cl.Changelist.EnsureAuthenticated')
    @mock.patch('multiprocessing.pool.ThreadPool')
    def test_get_cl_statuses_timeout(self, *_mocks):
        changes = [git_cl.Changelist() for _ in range(2)]
        pool = multiprocessing.pool.ThreadPool()
        it = pool.imap_unordered.return_value.__iter__ = mock.Mock()
        it.return_value.next.side_effect = [
            (changes[0], 'lgtm'),
            multiprocessing.TimeoutError,
        ]

        actual = list(git_cl.get_cl_statuses(changes, True))
        self.assertEqual([(changes[0], 'lgtm'), (changes[1], 'error')], actual)

    @mock.patch('git_cl.Changelist.GetIssueURL')
    def test_get_cl_statuses_not_finegrained(self, _mock):
        changes = [git_cl.Changelist() for _ in range(2)]
        urls = ['some-url', None]
        git_cl.Changelist.GetIssueURL.side_effect = urls

        actual = set(git_cl.get_cl_statuses(changes, False))
        self.assertEqual(set([(changes[0], 'waiting'), (changes[1], 'error')]),
                         actual)

    def test_get_issue_url(self):
        cl = git_cl.Changelist(issue=123)
        cl._gerrit_server = 'https://example.com'
        self.assertEqual(cl.GetIssueURL(), 'https://example.com/123')
        self.assertEqual(cl.GetIssueURL(short=True), 'https://example.com/123')

        cl = git_cl.Changelist(issue=123)
        cl._gerrit_server = 'https://chromium-review.googlesource.com'
        self.assertEqual(cl.GetIssueURL(),
                         'https://chromium-review.googlesource.com/123')
        self.assertEqual(cl.GetIssueURL(short=True), 'https://crrev.com/c/123')

    def test_set_preserve_tryjobs(self):
        d = git_cl.ChangeDescription('Simple.')
        d.set_preserve_tryjobs()
        self.assertEqual(d.description.splitlines(), [
            'Simple.',
            '',
            'Cq-Do-Not-Cancel-Tryjobs: true',
        ])
        before = d.description
        d.set_preserve_tryjobs()
        self.assertEqual(before, d.description)

        d = git_cl.ChangeDescription('\n'.join([
            'One is enough',
            '',
            'Cq-Do-Not-Cancel-Tryjobs: dups not encouraged, but don\'t hurt',
            'Change-Id: Ideadbeef',
        ]))
        d.set_preserve_tryjobs()
        self.assertEqual(d.description.splitlines(), [
            'One is enough',
            '',
            'Cq-Do-Not-Cancel-Tryjobs: dups not encouraged, but don\'t hurt',
            'Change-Id: Ideadbeef',
            'Cq-Do-Not-Cancel-Tryjobs: true',
        ])

    def test_get_bug_line_values(self):
        f = lambda p, bugs: list(git_cl._get_bug_line_values(p, bugs))
        self.assertEqual(f('', ''), [])
        self.assertEqual(f('', '123,v8:456'), ['123', 'v8:456'])
        # Prefix that ends with colon.
        self.assertEqual(f('v8:', '456'), ['v8:456'])
        self.assertEqual(f('v8:', 'chromium:123,456'),
                         ['v8:456', 'chromium:123'])
        # Prefix that ends without colon.
        self.assertEqual(f('v8', '456'), ['v8:456'])
        self.assertEqual(f('v8', 'chromium:123,456'),
                         ['v8:456', 'chromium:123'])
        # Not nice, but not worth carying.
        self.assertEqual(f('v8:', 'chromium:123,456,v8:123'),
                         ['v8:456', 'chromium:123', 'v8:123'])
        self.assertEqual(f('v8', 'chromium:123,456,v8:123'),
                         ['v8:456', 'chromium:123', 'v8:123'])

    @mock.patch('gerrit_util.GetAccountDetails')
    def test_valid_accounts(self, mockGetAccountDetails):
        mock_per_account = {
            'u1': None,  # 404, doesn't exist.
            'u2': {
                '_account_id': 123124,
                'avatars': [],
                'email': 'u2@example.com',
                'name': 'User Number 2',
                'status': 'OOO',
            },
            'u3': git_cl.gerrit_util.GerritError(500,
                                                 'retries didn\'t help :('),
        }

        def GetAccountDetailsMock(_, account):
            # Poor-man's mock library's side_effect.
            v = mock_per_account.pop(account)
            if isinstance(v, Exception):
                raise v
            return v

        mockGetAccountDetails.side_effect = GetAccountDetailsMock
        actual = git_cl.gerrit_util.ValidAccounts('host', ['u1', 'u2', 'u3'],
                                                  max_threads=1)
        self.assertEqual(
            actual, {
                'u2': {
                    '_account_id': 123124,
                    'avatars': [],
                    'email': 'u2@example.com',
                    'name': 'User Number 2',
                    'status': 'OOO',
                },
            })


class TestParseIssueURL(unittest.TestCase):
    def _test(self, arg, issue=None, patchset=None, hostname=None, fail=False):
        parsed = git_cl.ParseIssueNumberArgument(arg)
        self.assertIsNotNone(parsed)
        if fail:
            self.assertFalse(parsed.valid)
            return
        self.assertTrue(parsed.valid)
        self.assertEqual(parsed.issue, issue)
        self.assertEqual(parsed.patchset, patchset)
        self.assertEqual(parsed.hostname, hostname)

    def test_basic(self):
        self._test('123', 123)
        self._test('', fail=True)
        self._test('abc', fail=True)
        self._test('123/1', fail=True)
        self._test('123a', fail=True)
        self._test('ssh://chrome-review.source.com/#/c/123/4/', fail=True)
        self._test('ssh://chrome-review.source.com/c/123/1/', fail=True)

    def test_gerrit_url(self):
        self._test('https://codereview.source.com/123', 123, None,
                   'codereview.source.com')
        self._test('http://chrome-review.source.com/c/123', 123, None,
                   'chrome-review.source.com')
        self._test('https://chrome-review.source.com/c/123/', 123, None,
                   'chrome-review.source.com')
        self._test('https://chrome-review.source.com/c/123/4', 123, 4,
                   'chrome-review.source.com')
        self._test('https://chrome-review.source.com/#/c/123/4', 123, 4,
                   'chrome-review.source.com')
        self._test('https://chrome-review.source.com/c/123/4', 123, 4,
                   'chrome-review.source.com')
        self._test('https://chrome-review.source.com/123', 123, None,
                   'chrome-review.source.com')
        self._test('https://chrome-review.source.com/123/4', 123, 4,
                   'chrome-review.source.com')

        self._test('https://chrome-review.source.com/bad/123/4', fail=True)
        self._test('https://chrome-review.source.com/c/123/1/whatisthis',
                   fail=True)
        self._test('https://chrome-review.source.com/c/abc/', fail=True)

    def test_short_urls(self):
        self._test('https://crrev.com/c/2151934', 2151934, None,
                   'chromium-review.googlesource.com')

    def test_missing_scheme(self):
        self._test('codereview.source.com/123', 123, None,
                   'codereview.source.com')
        self._test('crrev.com/c/2151934', 2151934, None,
                   'chromium-review.googlesource.com')


class GitCookiesCheckerTest(unittest.TestCase):
    def setUp(self):
        super(GitCookiesCheckerTest, self).setUp()
        self.c = git_cl._GitCookiesChecker()
        self.c._all_hosts = []
        mock.patch('sys.stdout', io.StringIO()).start()
        self.addCleanup(mock.patch.stopall)

    def mock_hosts_creds(self, subhost_identity_pairs):
        def ensure_googlesource(h):
            if not h.endswith(git_cl._GOOGLESOURCE):
                assert not h.endswith('.')
                return h + '.' + git_cl._GOOGLESOURCE
            return h

        self.c._all_hosts = [(ensure_googlesource(h), i, '.gitcookies')
                             for h, i in subhost_identity_pairs]

    def test_identity_parsing(self):
        self.assertEqual(self.c._parse_identity('ldap.google.com'),
                         ('ldap', 'google.com'))
        self.assertEqual(self.c._parse_identity('git-ldap.example.com'),
                         ('ldap', 'example.com'))
        # Specical case because we know there are no subdomains in chromium.org.
        self.assertEqual(self.c._parse_identity('git-note.period.chromium.org'),
                         ('note.period', 'chromium.org'))
        # Pathological: ".period." can be either username OR domain, more likely
        # domain.
        self.assertEqual(self.c._parse_identity('git-note.period.example.com'),
                         ('note', 'period.example.com'))

    def test_analysis_nothing(self):
        self.c._all_hosts = []
        self.assertFalse(self.c.has_generic_host())
        self.assertEqual(set(), self.c.get_conflicting_hosts())
        self.assertEqual(set(), self.c.get_duplicated_hosts())
        self.assertEqual(set(), self.c.get_partially_configured_hosts())

    def test_analysis(self):
        self.mock_hosts_creds([
            ('.googlesource.com', 'git-example.chromium.org'),
            ('chromium', 'git-example.google.com'),
            ('chromium-review', 'git-example.google.com'),
            ('chrome-internal', 'git-example.chromium.org'),
            ('chrome-internal-review', 'git-example.chromium.org'),
            ('conflict', 'git-example.google.com'),
            ('conflict-review', 'git-example.chromium.org'),
            ('dup', 'git-example.google.com'),
            ('dup', 'git-example.google.com'),
            ('dup-review', 'git-example.google.com'),
            ('partial', 'git-example.google.com'),
            ('gpartial-review', 'git-example.google.com'),
        ])
        self.assertTrue(self.c.has_generic_host())
        self.assertEqual(set(['conflict.googlesource.com']),
                         self.c.get_conflicting_hosts())
        self.assertEqual(set(['dup.googlesource.com']),
                         self.c.get_duplicated_hosts())
        self.assertEqual(
            set([
                'partial.googlesource.com', 'gpartial-review.googlesource.com'
            ]), self.c.get_partially_configured_hosts())

    def test_report_no_problems(self):
        self.test_analysis_nothing()
        self.assertFalse(self.c.find_and_report_problems())
        self.assertEqual(sys.stdout.getvalue(), '')

    @mock.patch('git_cl.gerrit_util.CookiesAuthenticator.get_gitcookies_path',
                return_value=os.path.join('~', '.gitcookies'))
    def test_report(self, *_mocks):
        self.test_analysis()
        self.assertTrue(self.c.find_and_report_problems())
        with open(
                os.path.join(os.path.dirname(__file__),
                             'git_cl_creds_check_report.txt')) as f:
            expected = f.read() % {
                'sep': os.sep,
            }

        def by_line(text):
            return [l.rstrip() for l in text.rstrip().splitlines()]

        self.maxDiff = 10000  # pylint: disable=attribute-defined-outside-init
        self.assertEqual(by_line(sys.stdout.getvalue().strip()),
                         by_line(expected))


class TestGitCl(unittest.TestCase):
    def setUp(self):
        super(TestGitCl, self).setUp()
        self.calls = []
        self._calls_done = []

        oldEnv = dict(os.environ)
        def _resetEnv():
            os.environ = oldEnv
        self.addCleanup(_resetEnv)

        self.failed = False
        mock.patch('sys.stdout', io.StringIO()).start()
        mock.patch('git_cl.time_time',
                   lambda: self._mocked_call('time.time')).start()
        mock.patch('git_cl.metrics.collector.add_repeated',
                   lambda *a: self._mocked_call('add_repeated', *a)).start()
        mock.patch('subprocess2.call', self._mocked_call).start()
        mock.patch('subprocess2.check_call', self._mocked_call).start()
        mock.patch('subprocess2.check_output', self._mocked_call).start()
        mock.patch('subprocess2.communicate', lambda *a, **_k:
                   ([self._mocked_call(*a), ''], 0)).start()
        mock.patch('git_cl.gclient_utils.CheckCallAndFilter',
                   self._mocked_call).start()
        mock.patch('git_common.is_dirty_git_tree', lambda x: False).start()
        mock.patch('git_cl.FindCodereviewSettingsFile', return_value='').start()
        mock.patch(
            'git_cl.SaveDescriptionBackup',
            lambda _: self._mocked_call('SaveDescriptionBackup')).start()
        mock.patch('git_cl.write_json',
                   lambda *a: self._mocked_call('write_json', *a)).start()
        mock.patch('git_cl.Changelist.RunHook',
                   return_value={
                       'more_cc': ['test-more-cc@chromium.org']
                   }).start()
        mock.patch('git_cl.watchlists.Watchlists', WatchlistsMock).start()
        mock.patch('git_cl.auth.Authenticator', AuthenticatorMock).start()
        mock.patch('gerrit_util.GetChangeDetail').start()
        mock.patch(
            'git_cl.gerrit_util.GetChangeComments',
            lambda *a: self._mocked_call('GetChangeComments', *a)).start()
        mock.patch(
            'git_cl.gerrit_util.GetChangeRobotComments',
            lambda *a: self._mocked_call('GetChangeRobotComments', *a)).start()
        mock.patch('git_cl.gerrit_util.AddReviewers',
                   lambda *a: self._mocked_call('AddReviewers', *a)).start()
        mock.patch('git_cl.gerrit_util.SetReview',
                   lambda h, i, msg=None, labels=None, notify=None, ready=None:
                   (self._mocked_call('SetReview', h, i, msg, labels, notify,
                                      ready))).start()
        mock.patch('git_cl.gerrit_util.LuciContextAuthenticator.is_applicable',
                   return_value=False).start()
        mock.patch('git_cl.gerrit_util.GceAuthenticator.is_applicable',
                   return_value=False).start()
        mock.patch('git_cl.gerrit_util.ValidAccounts',
                   lambda *a: self._mocked_call('ValidAccounts', *a)).start()
        mock.patch('sys.exit', side_effect=SystemExitMock).start()
        mock.patch('git_cl.Settings.GetRoot', return_value='').start()
        scm_mock.GIT(self)
        mock.patch('scm.GIT.ResolveCommit', return_value='hash').start()
        mock.patch('scm.GIT.IsValidRevision', return_value=True).start()
        mock.patch('scm.GIT.FetchUpstreamTuple',
                   return_value=('origin', 'refs/heads/main')).start()
        mock.patch('scm.GIT.CaptureStatus',
                   return_value=[('M', 'foo.txt')]).start()
        # It's important to reset settings to not have inter-tests interference.
        git_cl.settings = git_cl.Settings()
        self.addCleanup(mock.patch.stopall)
        gerrit_util._Authenticator._resolved = None

    def tearDown(self):
        try:
            if not self.failed:
                self.assertEqual([], self.calls)
        except AssertionError:
            calls = ''.join('  %s\n' % str(call) for call in self.calls[:5])
            if len(self.calls) > 5:
                calls += ' ...\n'
            self.fail(
                '\n'
                'There are un-consumed calls after this test has finished:\n' +
                calls)
        finally:
            super(TestGitCl, self).tearDown()

    def _mocked_call(self, *args, **_kwargs):
        self.assertTrue(
            self.calls, '@%d  Expected: <Missing>   Actual: %r' %
            (len(self._calls_done), args))
        top = self.calls.pop(0)
        expected_args, result = top

        # Also logs otherwise it could get caught in a try/finally and be hard
        # to diagnose.
        if expected_args != args:
            N = 5
            prior_calls = '\n  '.join(
                '@%d: %r' % (len(self._calls_done) - N + i, c[0])
                for i, c in enumerate(self._calls_done[-N:]))
            following_calls = '\n  '.join('@%d: %r' %
                                          (len(self._calls_done) + i + 1, c[0])
                                          for i, c in enumerate(self.calls[:N]))
            extended_msg = ('A few prior calls:\n  %s\n\n'
                            'This (expected):\n  @%d: %r\n'
                            'This (actual):\n  @%d: %r\n\n'
                            'A few following expected calls:\n  %s' %
                            (prior_calls, len(self._calls_done), expected_args,
                             len(self._calls_done), args, following_calls))

            self.failed = True
            self.fail(
                '@%d\n'
                '  Expected: %r\n'
                '  Actual:   %r\n'
                '\n'
                '%s' %
                (len(self._calls_done), expected_args, args, extended_msg))

        self._calls_done.append(top)
        if isinstance(result, Exception):
            raise result
        # stdout from git commands is supposed to be a bytestream. Convert it
        # here instead of converting all test output in this file to bytes.
        if args[0][0] == 'git' and not isinstance(result, bytes):
            result = result.encode('utf-8')
        return result

    @mock.patch('sys.stdin', io.StringIO('blah\nye\n'))
    @mock.patch('sys.stdout', io.StringIO())
    def test_ask_for_explicit_yes_true(self):
        self.assertTrue(git_cl.ask_for_explicit_yes('prompt'))
        self.assertEqual('prompt [Yes/No]: Please, type yes or no: ',
                         sys.stdout.getvalue())

    def test_LoadCodereviewSettingsFromFile_gerrit(self):
        codereview_file = io.StringIO('GERRIT_HOST: true')
        self.calls = [
        ]
        self.assertIsNone(
            git_cl.LoadCodereviewSettingsFromFile(codereview_file))

    @classmethod
    def _gerrit_base_calls(cls,
                           issue=None,
                           fetched_description=None,
                           fetched_status=None,
                           other_cl_owner=None,
                           custom_cl_base=None,
                           short_hostname='chromium',
                           change_id=None,
                           default_branch='main',
                           reset_issue=False):
        calls = [
            (
                (['os.path.isfile', '.gitmodules'], ),
                'True',
            ),
        ]
        if custom_cl_base:
            ancestor_revision = custom_cl_base
        else:
            # Determine ancestor_revision to be merge base.
            ancestor_revision = 'origin/' + default_branch

        if issue:
            # TODO: if tests don't provide a `change_id` the default used here
            # will cause the TRACES_README_FORMAT mock (which uses the test
            # provided `change_id` to fail.
            gerrit_util.GetChangeDetail.return_value = {
                'owner': {
                    'email': (other_cl_owner or 'owner@example.com')
                },
                'change_id': (change_id or '123456789'),
                'current_revision': 'sha1_of_current_revision',
                'revisions': {
                    'sha1_of_current_revision': {
                        'commit': {
                            'message': fetched_description
                        },
                    }
                },
                'status': fetched_status or 'NEW',
            }

            if fetched_status == 'ABANDONED':
                return calls
            if fetched_status == 'MERGED':
                calls.append(((
                    'ask_for_data',
                    'Change https://chromium-review.googlesource.com/%s has been '
                    'submitted, new uploads are not allowed. Would you like to start '
                    'a new change (Y/n)?' % issue),
                              'y' if reset_issue else 'n'))
                if not reset_issue:
                    return calls
                # Part of SetIssue call.
                calls.append(((['git', 'log', '-1', '--format=%B'], ), ''))
            if other_cl_owner:
                calls += [
                    (('ask_for_data',
                      'Press Enter to upload, or Ctrl+C to abort'), ''),
                ]

        calls += [
            ((['git', 'rev-list', '--count'] +
              ([f'{custom_cl_base}..HEAD']
               if custom_cl_base else [f'{ancestor_revision}..HEAD']), ), '3'),
        ]

        calls += [
            ((['git', 'diff', '--no-ext-diff', '--stat', '-l100000', '-C50'] +
              ([custom_cl_base]
               if custom_cl_base else [ancestor_revision, 'HEAD']), ), '+dat'),
        ]
        return calls

    def _gerrit_upload_calls(self,
                             description,
                             reviewers,
                             squash,
                             squash_mode='default',
                             title=None,
                             notify=False,
                             post_amend_description=None,
                             issue=None,
                             cc=None,
                             custom_cl_base=None,
                             short_hostname='chromium',
                             labels=None,
                             change_id=None,
                             final_description=None,
                             gitcookies_exists=True,
                             force=False,
                             edit_description=None,
                             default_branch='main',
                             ref_to_push='abcdef0123456789',
                             external_parent=None,
                             push_opts=None):
        if post_amend_description is None:
            post_amend_description = description
        cc = cc or []

        calls = []

        if squash_mode in ('override_squash', 'override_nosquash'):
            scm.GIT.SetConfig(
                '', 'gerrit.override-squash-uploads',
                'true' if squash_mode == 'override_squash' else 'false')

        if not git_footers.get_footer_change_id(description) and not squash:
            calls += [
                (('DownloadGerritHook', False), ''),
            ]
        if squash:
            if not issue and not force:
                calls += [
                    ((['RunEditor'], ), description),
                ]
            # user wants to edit description
            if edit_description:
                calls += [
                    ((['RunEditor'], ), edit_description),
                ]

            if external_parent:
                parent = external_parent
            else:
                if custom_cl_base is None:
                    parent = 'origin/' + default_branch
                    git_common.get_or_create_merge_base.return_value = parent
                else:
                    calls += [
                        (([
                            'git', 'merge-base', '--is-ancestor',
                            custom_cl_base,
                            'refs/remotes/origin/' + default_branch
                        ], ), callError(1)),  # Means not ancenstor.
                        (('ask_for_data',
                          'Do you take responsibility for cleaning up potential mess '
                          'resulting from proceeding with upload? Press Enter to upload, '
                          'or Ctrl+C to abort'), ''),
                    ]
                    parent = custom_cl_base

            calls += [
                (
                    (['git', 'rev-parse',
                      'HEAD:'], ),  # `HEAD:` means HEAD's tree hash.
                    '0123456789abcdef'),
                ((['FileWrite', '/tmp/fake-temp1', description], ), None),
                (([
                    'git', 'commit-tree', '0123456789abcdef', '-p', parent,
                    '-F', '/tmp/fake-temp1'
                ], ), ref_to_push),
            ]
        else:
            ref_to_push = 'HEAD'
            parent = 'origin/refs/heads/' + default_branch

        calls += [
            (('SaveDescriptionBackup', ), None),
            ((['git', 'rev-list',
               parent + '..' + ref_to_push], ), '1hashPerLine\n'),
        ]

        metrics_arguments = []
        ref_suffix_list = []
        if notify:
            ref_suffix_list.append('ready,notify=ALL')
            metrics_arguments += ['ready', 'notify=ALL']
        elif not issue and squash:
            ref_suffix_list.append('wip')
            metrics_arguments.append('wip')

        # If issue is given, then description is fetched from Gerrit instead.
        if not title and squash_mode != "override_nosquash":
            if issue is None:
                if squash:
                    title = 'Initial upload'
            else:
                calls += [
                    ((['git', 'show', '-s', '--format=%s', 'HEAD',
                       '--'], ), ''),
                    (('ask_for_data', 'Title for patchset []: '), 'User input'),
                ]
                title = 'User input'
        if title:
            ref_suffix_list.append('m=' +
                                   gerrit_util.PercentEncodeForGitRef(title))
            metrics_arguments.append('m')

        for k, v in sorted((labels or {}).items()):
            ref_suffix_list.append('l=%s+%d' % (k, v))
            metrics_arguments.append('l=%s+%d' % (k, v))

        if short_hostname == 'chromium':
            # All reviewers and ccs get into ref_suffix.
            for r in sorted(reviewers):
                ref_suffix_list.append('r=%s' % r)
                metrics_arguments.append('r')
            if issue is None:
                cc += ['test-more-cc@chromium.org', 'joe@example.com']
            for c in sorted(cc):
                ref_suffix_list.append('cc=%s' % c)
                metrics_arguments.append('cc')
            reviewers, cc = [], []
        else:
            # TODO(crbug/877717): remove this case.
            calls += [(('ValidAccounts',
                        '%s-review.googlesource.com' % short_hostname,
                        sorted(reviewers) +
                        ['joe@example.com', 'test-more-cc@chromium.org'] + cc),
                       {
                           e: {
                               'email': e
                           }
                           for e in (reviewers + ['joe@example.com'] + cc)
                       })]
            for r in sorted(reviewers):
                if r != 'bad-account-or-email':
                    ref_suffix_list.append('r=%s' % r)
                    metrics_arguments.append('r')
                    reviewers.remove(r)
            if issue is None:
                cc += ['joe@example.com']
            for c in sorted(cc):
                ref_suffix_list.append('cc=%s' % c)
                metrics_arguments.append('cc')
                if c in cc:
                    cc.remove(c)

        ref_suffix = ''
        if ref_suffix_list:
            ref_suffix = '%' + ','.join(ref_suffix_list)
        calls += [
            (
                ('time.time', ),
                1000,
            ),
            (
                ([
                    'git', 'push',
                    'https://%s.googlesource.com/my/repo' % short_hostname,
                    ref_to_push + ':refs/for/refs/heads/' + default_branch +
                    ref_suffix
                ] + (push_opts if push_opts else []), ),
                (('remote:\n'
                  'remote: Processing changes: (\)\n'
                  'remote: Processing changes: (|)\n'
                  'remote: Processing changes: (/)\n'
                  'remote: Processing changes: (-)\n'
                  'remote: Processing changes: new: 1 (/)\n'
                  'remote: Processing changes: new: 1, done\n'
                  'remote:\n'
                  'remote: New Changes:\n'
                  'remote:   '
                  'https://%s-review.googlesource.com/#/c/my/repo/+/123456'
                  ' XXX\n'
                  'remote:\n'
                  'To https://%s.googlesource.com/my/repo\n'
                  ' * [new branch]      hhhh -> refs/for/refs/heads/%s\n') %
                 (short_hostname, short_hostname, default_branch)),
            ),
            (
                ('time.time', ),
                2000,
            ),
            (
                ('add_repeated', 'sub_commands', {
                    'execution_time': 1000,
                    'command': 'git push',
                    'exit_code': 0,
                    'arguments': sorted(metrics_arguments),
                }),
                None,
            ),
        ]

        final_description = final_description or post_amend_description.strip()

        trace_name = os.path.join('TRACES_DIR', '20170316T200041.000000')

        # Trace-related calls
        calls += [
            # Write a description with context for the current trace.
            (
                ([
                    'FileWrite', trace_name + '-README',
                    '%(date)s\n'
                    '%(short_hostname)s-review.googlesource.com\n'
                    '%(change_id)s\n'
                    '%(title)s\n'
                    '%(description)s\n'
                    '1000\n'
                    '0\n'
                    '%(trace_name)s' % {
                        'date': '2017-03-16T20:00:41.000000',
                        'short_hostname': short_hostname,
                        'change_id': change_id,
                        'description': final_description,
                        'title': title or '<untitled>',
                        'trace_name': trace_name,
                    }
                ], ),
                None,
            ),
            # Read traces and shorten git hashes.
            (
                (['os.path.isfile',
                  os.path.join('TEMP_DIR', 'trace-packet')], ),
                True,
            ),
            (
                (['FileRead',
                  os.path.join('TEMP_DIR', 'trace-packet')], ),
                ('git-hash: 0123456789012345678901234567890123456789\n'
                 'git-hash: abcdeabcdeabcdeabcdeabcdeabcdeabcdeabcde\n'),
            ),
            (
                ([
                    'FileWrite',
                    os.path.join('TEMP_DIR', 'trace-packet'),
                    'git-hash: 012345\n'
                    'git-hash: abcdea\n'
                ], ),
                None,
            ),
            # Make zip file for the git traces.
            (
                (['make_archive', trace_name + '-traces', 'zip', 'TEMP_DIR'], ),
                None,
            ),
            # Collect git config and gitcookies.
            #
            # We accept ANY for the git-config file because it's just reflecting
            # our mocked git config in scm.GIT anyway.
            (
                ([
                    'FileWrite',
                    os.path.join('TEMP_DIR', 'git-config'), mock.ANY,
                ], ),
                None,
            ),
            (
                (['os.path.isfile',
                  os.path.join('~', '.gitcookies')], ),
                gitcookies_exists,
            ),
        ]
        if gitcookies_exists:
            calls += [
                (
                    (['FileRead', os.path.join('~', '.gitcookies')], ),
                    'gitcookies 1/SECRET',
                ),
                (
                    ([
                        'FileWrite',
                        os.path.join(
                            'TEMP_DIR',
                            'CookiesAuthenticatorMock.debug_summary_state'),
                        'gitcookies REDACTED'
                    ], ),
                    None,
                ),
            ]
        else:
            calls += [
                (
                    ([
                        'FileWrite',
                        os.path.join(
                            'TEMP_DIR',
                            'CookiesAuthenticatorMock.debug_summary_state'), ''
                    ], ),
                    None,
                ),
            ]
        calls += [
            # Make zip file for the git config and gitcookies.
            (
                (['make_archive', trace_name + '-git-info', 'zip',
                  'TEMP_DIR'], ),
                None,
            ),
        ]

        # TODO(crbug/877717): this should never be used.
        if squash and short_hostname != 'chromium':
            calls += [
                (('AddReviewers', 'chromium-review.googlesource.com',
                  'my%2Frepo~123456', sorted(reviewers),
                  cc + ['test-more-cc@chromium.org'], notify), ''),
            ]
        return calls

    def _run_gerrit_upload_test(self,
                                upload_args,
                                description,
                                reviewers=None,
                                squash=True,
                                squash_mode=None,
                                title=None,
                                notify=False,
                                post_amend_description=None,
                                issue=None,
                                patchset=None,
                                cc=None,
                                fetched_status=None,
                                other_cl_owner=None,
                                custom_cl_base=None,
                                short_hostname='chromium',
                                labels=None,
                                change_id=None,
                                final_description=None,
                                gitcookies_exists=True,
                                force=False,
                                log_description=None,
                                edit_description=None,
                                fetched_description=None,
                                default_branch='main',
                                ref_to_push='abcdef0123456789',
                                external_parent=None,
                                push_opts=None,
                                reset_issue=False):
        """Generic gerrit upload test framework."""
        if squash_mode is None:
            if '--no-squash' in upload_args:
                squash_mode = 'nosquash'
            elif '--squash' in upload_args:
                squash_mode = 'squash'
            else:
                squash_mode = 'default'

        reviewers = reviewers or []
        cc = cc or []
        mock.patch(
            'git_cl.gerrit_util.CookiesAuthenticator',
            CookiesAuthenticatorMockFactory(same_auth=('git-owner.example.com',
                                                       'pass'))).start()
        mock.patch('git_cl.Changelist._GerritCommitMsgHookCheck',
                   lambda offer_removal: None).start()
        mock.patch('git_cl.Changelist.GetMostRecentPatchset',
                   lambda _, update: patchset).start()
        mock.patch('git_cl.gclient_utils.RunEditor',
                   lambda *_, **__: self._mocked_call(['RunEditor'])).start()
        mock.patch('git_cl.DownloadGerritHook', lambda force: self._mocked_call(
            'DownloadGerritHook', force)).start()
        mock.patch('git_cl.gclient_utils.FileRead',
                   lambda path: self._mocked_call(['FileRead', path])).start()
        mock.patch(
            'git_cl.gclient_utils.FileWrite', lambda path, contents: self.
            _mocked_call(['FileWrite', path, contents])).start()
        mock.patch(
            'git_cl.datetime_now',
            lambda: datetime.datetime(2017, 3, 16, 20, 0, 41, 0)).start()
        mock.patch('git_cl.tempfile.mkdtemp', lambda: 'TEMP_DIR').start()
        mock.patch('git_cl.TRACES_DIR', 'TRACES_DIR').start()
        mock.patch(
            'git_cl.TRACES_README_FORMAT', '%(now)s\n'
            '%(gerrit_host)s\n'
            '%(change_id)s\n'
            '%(title)s\n'
            '%(description)s\n'
            '%(execution_time)s\n'
            '%(exit_code)s\n'
            '%(trace_name)s').start()
        mock.patch(
            'git_cl.shutil.make_archive', lambda *args: self._mocked_call(
                ['make_archive'] + list(args))).start()
        mock.patch(
            'os.path.isfile',
            lambda path: self._mocked_call(['os.path.isfile', path])).start()
        mock.patch('git_cl._create_description_from_log',
                   return_value=log_description or description).start()
        mock.patch('git_cl.Changelist._AddChangeIdToCommitMessage',
                   return_value=post_amend_description or description).start()
        mock.patch('git_cl.GenerateGerritChangeId',
                   return_value=change_id).start()
        mock.patch('git_common.get_or_create_merge_base',
                   return_value='origin/' + default_branch).start()
        mock.patch('gerrit_util.GetAccountDetails',
                    getAccountDetailsMock).start()
        mock.patch(
            'gclient_utils.AskForData',
            lambda prompt: self._mocked_call('ask_for_data', prompt)).start()

        scm.GIT.SetConfig('', 'gerrit.host', 'true')
        scm.GIT.SetConfig('', 'branch.main.gerritissue',
                          (str(issue) if issue else None))
        scm.GIT.SetConfig('', 'remote.origin.url',
                          f'https://{short_hostname}.googlesource.com/my/repo')
        scm.GIT.SetConfig('', 'user.email', 'owner@example.com')

        if squash_mode == "override_nosquash":
            if issue:
                mock.patch('gerrit_util.GetChange',
                           return_value={
                               '_number': issue
                           }).start()
            else:
                mock.patch('gerrit_util.GetChange', return_value={}).start()

        self.calls = self._gerrit_base_calls(
            issue=issue,
            fetched_description=fetched_description or description,
            fetched_status=fetched_status,
            other_cl_owner=other_cl_owner,
            custom_cl_base=custom_cl_base,
            short_hostname=short_hostname,
            change_id=change_id,
            default_branch=default_branch,
            reset_issue=reset_issue)

        if fetched_status == 'ABANDONED' or (fetched_status == 'MERGED'
                                             and not reset_issue):
            pass  # readability
        else:
            if fetched_status == 'MERGED' and reset_issue:
                fetched_status = 'NEW'
                issue = None
            mock.patch('gclient_utils.temporary_file',
                       TemporaryFileMock()).start()
            mock.patch('os.remove', return_value=True).start()
            self.calls += self._gerrit_upload_calls(
                description,
                reviewers,
                squash,
                squash_mode=squash_mode,
                title=title,
                notify=notify,
                post_amend_description=post_amend_description,
                issue=issue,
                cc=cc,
                custom_cl_base=custom_cl_base,
                short_hostname=short_hostname,
                labels=labels,
                change_id=change_id,
                final_description=final_description,
                gitcookies_exists=gitcookies_exists,
                force=force,
                edit_description=edit_description,
                default_branch=default_branch,
                ref_to_push=ref_to_push,
                external_parent=external_parent,
                push_opts=push_opts)
        # Uncomment when debugging.
        # print('\n'.join(map(lambda x: '%2i: %s' % x, enumerate(self.calls))))
        git_cl.main(['upload'] + upload_args)
        if squash:
            self.assertIssueAndPatchset(patchset=str((patchset or 0) + 1))
            self.assertEqual(
                ref_to_push,
                scm.GIT.GetBranchConfig('', 'main',
                                        git_cl.GERRIT_SQUASH_HASH_CONFIG_KEY))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_gerrit_upload_traces_no_gitcookies(self):
        self._run_gerrit_upload_test(
            ['--no-squash'],
            'desc ✔\n\nBUG=\n', [],
            squash=False,
            post_amend_description='desc ✔\n\nBUG=\n\nChange-Id: Ixxx',
            change_id='Ixxx',
            gitcookies_exists=False)

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_gerrit_upload_without_change_id_nosquash(self):
        self._run_gerrit_upload_test(
            ['--no-squash'],
            'desc ✔\n\nBUG=\n', [],
            squash=False,
            post_amend_description='desc ✔\n\nBUG=\n\nChange-Id: Ixxx',
            change_id='Ixxx')

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_gerrit_upload_without_change_id_override_nosquash(self):
        self._run_gerrit_upload_test(
            [],
            'desc ✔\n\nBUG=\n', [],
            squash=False,
            squash_mode='override_nosquash',
            post_amend_description='desc ✔\n\nBUG=\n\nChange-Id: Ixxx',
            change_id='Ixxx')

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_gerrit_no_reviewer(self):
        self._run_gerrit_upload_test(
            [],
            'desc ✔\n\nBUG=\n\nChange-Id: I123456789\n', [],
            squash=False,
            squash_mode='override_nosquash',
            change_id='I123456789')

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_gerrit_push_opts(self):
        self._run_gerrit_upload_test(
            ['-o', 'wip'],
            'desc ✔\n\nBUG=\n\nChange-Id: I123456789\n', [],
            squash=False,
            squash_mode='override_nosquash',
            change_id='I123456789',
            push_opts=['-o', 'wip'])

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_gerrit_no_reviewer_non_chromium_host(self):
        # TODO(crbug/877717): remove this test case.
        self._run_gerrit_upload_test(
            [],
            'desc ✔\n\nBUG=\n\nChange-Id: I123456789\n', [],
            squash=False,
            squash_mode='override_nosquash',
            short_hostname='other',
            change_id='I123456789')

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_gerrit_patchset_title_special_chars_nosquash(self):
        self._run_gerrit_upload_test(
            ['-f', '-t', 'We\'ll escape ^_ ^ special chars...@{u}'],
            'desc ✔\n\nBUG=\n\nChange-Id: I123456789',
            squash=False,
            squash_mode='override_nosquash',
            change_id='I123456789',
            title='We\'ll escape ^_ ^ special chars...@{u}')

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_gerrit_reviewers_cmd_line(self):
        self._run_gerrit_upload_test(
            ['-r', 'foo@example.com', '--send-mail'],
            'desc ✔\n\nBUG=\n\nChange-Id: I123456789',
            reviewers=['foo@example.com'],
            squash=False,
            squash_mode='override_nosquash',
            notify=True,
            change_id='I123456789',
            final_description=(
                'desc ✔\n\nBUG=\nR=foo@example.com\n\nChange-Id: I123456789'))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_gerrit_reviewers_cmd_line_send_email(self):
        self._run_gerrit_upload_test(
            ['-r', 'foo@example.com', '--send-email'],
            'desc ✔\n\nBUG=\n\nChange-Id: I123456789',
            reviewers=['foo@example.com'],
            squash=False,
            squash_mode='override_nosquash',
            notify=True,
            change_id='I123456789',
            final_description=(
                'desc ✔\n\nBUG=\nR=foo@example.com\n\nChange-Id: I123456789'))

    @mock.patch('git_cl.Changelist.GetGerritHost',
                return_value='chromium-review.googlesource.com')
    @mock.patch('git_cl.Changelist.GetRemoteBranch',
                return_value=('origin', 'refs/remotes/origin/main'))
    @mock.patch('git_cl.Changelist.PostUploadUpdates')
    @mock.patch('git_cl.Changelist._RunGitPushWithTraces')
    @mock.patch('git_cl._UploadAllPrecheck')
    @mock.patch('git_cl.Changelist.PrepareCherryPickSquashedCommit')
    def test_upload_all_squashed_cherry_pick(self, mockCherryPickCommit,
                                             mockUploadAllPrecheck,
                                             mockRunGitPush,
                                             mockPostUploadUpdates, *_mocks):
        # Set up
        cls = [
            git_cl.Changelist(branchref='refs/heads/current-branch',
                              issue='12345'),
            git_cl.Changelist(branchref='refs/heads/upstream-branch')
        ]
        mockUploadAllPrecheck.return_value = (cls, True)

        upstream_gerrit_commit = 'upstream-commit'
        scm.GIT.SetConfig('', 'branch.upstream-branch.gerritsquashhash',
                          upstream_gerrit_commit)

        reviewers = []
        ccs = []
        commit_to_push = 'commit-to-push'
        new_last_upload = 'new-last-upload'
        change_desc = git_cl.ChangeDescription(
            'stonks/nChange-Id:ec15e81197380')
        prev_patchset = 2
        new_upload = git_cl._NewUpload(reviewers, ccs, commit_to_push,
                                       new_last_upload, upstream_gerrit_commit,
                                       change_desc, prev_patchset)
        mockCherryPickCommit.return_value = new_upload

        options = optparse.Values()
        options.send_mail = options.private = False
        options.squash = True
        options.title = None
        options.message = 'honk stonk'
        options.topic = 'circus'
        options.enable_auto_submit = False
        options.set_bot_commit = False
        options.cq_dry_run = False
        options.use_commit_queue = False
        options.hashtags = ['cow']
        options.target_branch = None
        options.push_options = ['uploadvalidator~skip']
        orig_args = []

        mockRunGitPush.return_value = (
            'remote:   https://chromium-review.'
            'googlesource.com/c/chromium/circus/clown/+/1234 stonks')

        # Call
        git_cl.UploadAllSquashed(options, orig_args)

        # Asserts
        mockCherryPickCommit.assert_called_once_with(options,
                                                     upstream_gerrit_commit)
        expected_refspec = ('commit-to-push:refs/for/refs/heads/main%'
                            'm=honk_stonk,topic=circus,hashtag=cow')
        expected_refspec_opts = ['m=honk_stonk', 'topic=circus', 'hashtag=cow']
        mockRunGitPush.assert_called_once_with(expected_refspec,
                                               expected_refspec_opts, mock.ANY,
                                               options.push_options)
        mockPostUploadUpdates.assert_called_once_with(options, new_upload,
                                                      '1234')

    @mock.patch('git_cl.Changelist.GetGerritHost',
                return_value='chromium-review.googlesource.com')
    @mock.patch('git_cl.Changelist.GetRemoteBranch',
                return_value=('origin', 'refs/remotes/origin/main'))
    @mock.patch(
        'git_cl.Changelist.GetCommonAncestorWithUpstream',
        side_effect=['current-upstream-ancestor', 'next-upstream-ancestor'])
    @mock.patch('git_cl.Changelist.PostUploadUpdates')
    @mock.patch('git_cl.Changelist._RunGitPushWithTraces')
    @mock.patch('git_cl._UploadAllPrecheck')
    @mock.patch('git_cl.Changelist.PrepareSquashedCommit')
    def test_upload_all_squashed(self, mockSquashedCommit,
                                 mockUploadAllPrecheck, mockRunGitPush,
                                 mockPostUploadUpdates, *_mocks):
        # Set up
        cls = [
            git_cl.Changelist(branchref='refs/heads/current-branch',
                              issue='12345'),
            git_cl.Changelist(branchref='refs/heads/upstream-branch')
        ]
        mockUploadAllPrecheck.return_value = (cls, False)

        reviewers = []
        ccs = []

        current_commit_to_push = 'commit-to-push'
        current_new_last_upload = 'new-last-upload'
        change_desc = git_cl.ChangeDescription(
            'stonks/nChange-Id:ec15e81197380')
        prev_patchset = 2
        new_upload_current = git_cl._NewUpload(reviewers, ccs,
                                               current_commit_to_push,
                                               current_new_last_upload,
                                               'next-upstream-ancestor',
                                               change_desc, prev_patchset)

        upstream_desc = git_cl.ChangeDescription('kwak')
        upstream_parent = 'origin-commit'
        upstream_new_last_upload = 'upstrea-last-upload'
        upstream_commit_to_push = 'upstream_push_commit'
        new_upload_upstream = git_cl._NewUpload(reviewers, ccs,
                                                upstream_commit_to_push,
                                                upstream_new_last_upload,
                                                upstream_parent, upstream_desc,
                                                prev_patchset)
        mockSquashedCommit.side_effect = [
            new_upload_upstream, new_upload_current
        ]

        options = optparse.Values()
        options.send_mail = options.private = False
        options.squash = True
        options.title = None
        options.message = 'honk stonk'
        options.topic = 'circus'
        options.enable_auto_submit = False
        options.set_bot_commit = False
        options.cq_dry_run = False
        options.use_commit_queue = False
        options.hashtags = ['cow']
        options.target_branch = None
        options.push_options = ['uploadvalidator~skip']
        orig_args = []

        mockRunGitPush.return_value = (
            'remote:   https://chromium-review.'
            'googlesource.com/c/chromium/circus/clown/+/1233 kwak'
            '\n'
            'remote:   https://chromium-review.'
            'googlesource.com/c/chromium/circus/clown/+/1234 stonks')

        # Call
        git_cl.UploadAllSquashed(options, orig_args)

        # Asserts
        self.maxDiff = None
        self.assertEqual(mockSquashedCommit.mock_calls, [
            mock.call(options,
                      'current-upstream-ancestor',
                      'current-upstream-ancestor',
                      end_commit='next-upstream-ancestor'),
            mock.call(options,
                      upstream_commit_to_push,
                      'next-upstream-ancestor',
                      end_commit=None)
        ])

        expected_refspec = ('commit-to-push:refs/for/refs/heads/main%'
                            'topic=circus,hashtag=cow')
        expected_refspec_opts = ['topic=circus', 'hashtag=cow']
        mockRunGitPush.assert_called_once_with(expected_refspec,
                                               expected_refspec_opts, mock.ANY,
                                               options.push_options)

        self.assertEqual(mockPostUploadUpdates.mock_calls, [
            mock.call(options, new_upload_upstream, '1233'),
            mock.call(options, new_upload_current, '1234')
        ])

    @mock.patch('git_cl.Changelist.GetGerritHost',
                return_value='chromium-review.googlesource.com')
    @mock.patch('git_cl.Changelist.GetRemoteBranch',
                return_value=('origin', 'refs/remotes/origin/main'))
    @mock.patch('git_cl.Changelist.GetCommonAncestorWithUpstream',
                return_value='current-upstream-ancestor')
    @mock.patch('git_cl.Changelist._UpdateWithExternalChanges')
    @mock.patch('git_cl.Changelist.PostUploadUpdates')
    @mock.patch('git_cl.Changelist._RunGitPushWithTraces')
    @mock.patch('git_cl._UploadAllPrecheck')
    @mock.patch('git_cl.Changelist.PrepareSquashedCommit')
    def test_upload_all_squashed_external_changes(self, mockSquashedCommit,
                                                  mockUploadAllPrecheck,
                                                  mockRunGitPush,
                                                  mockPostUploadUpdates,
                                                  mockExternalChanges, *_mocks):
        options = optparse.Values()
        options.send_mail = options.private = False
        options.squash = True
        options.title = None
        options.topic = 'circus'
        options.message = 'honk stonk'
        options.enable_auto_submit = False
        options.set_bot_commit = False
        options.cq_dry_run = False
        options.use_commit_queue = False
        options.hashtags = ['cow']
        options.target_branch = None
        options.push_options = ['uploadvalidator~skip']
        orig_args = []

        cls = [
            git_cl.Changelist(branchref='refs/heads/current-branch',
                              issue='12345')
        ]
        mockUploadAllPrecheck.return_value = (cls, False)
        reviewers = []
        ccs = []

        # Test case: user wants to pull in external changes.
        mockExternalChanges.reset_mock()
        mockExternalChanges.return_value = None

        current_commit_to_push = 'commit-to-push'
        current_new_last_upload = 'new-last-upload'
        change_desc = git_cl.ChangeDescription(
            'stonks/nChange-Id:ec15e81197380')
        prev_patchset = 2
        new_upload_current = git_cl._NewUpload(reviewers, ccs,
                                               current_commit_to_push,
                                               current_new_last_upload,
                                               'next-upstream-ancestor',
                                               change_desc, prev_patchset)
        mockSquashedCommit.return_value = new_upload_current

        mockRunGitPush.return_value = (
            'remote:   https://chromium-review.'
            'googlesource.com/c/chromium/circus/clown/+/1233 kwak')

        # Test case: user wants to pull in external changes.
        mockExternalChanges.reset_mock()
        mockExternalChanges.return_value = 'external-commit'

        # Call
        git_cl.UploadAllSquashed(options, orig_args)

        # Asserts
        self.assertEqual(mockSquashedCommit.mock_calls, [
            mock.call(
                options, 'external-commit', 'external-commit', end_commit=None)
        ])

        expected_refspec = ('commit-to-push:refs/for/refs/heads/main%'
                            'm=honk_stonk,topic=circus,hashtag=cow')
        expected_refspec_opts = ['m=honk_stonk', 'topic=circus', 'hashtag=cow']
        mockRunGitPush.assert_called_once_with(expected_refspec,
                                               expected_refspec_opts, mock.ANY,
                                               options.push_options)

        self.assertEqual(mockPostUploadUpdates.mock_calls,
                         [mock.call(options, new_upload_current, '1233')])

        # Test case: user does not want external changes or there are none.
        mockSquashedCommit.reset_mock()
        mockExternalChanges.return_value = None

        # Call
        git_cl.UploadAllSquashed(options, orig_args)

        # Asserts
        self.assertEqual(mockSquashedCommit.mock_calls, [
            mock.call(options,
                      'current-upstream-ancestor',
                      'current-upstream-ancestor',
                      end_commit=None)
        ])

    @mock.patch('git_cl.Changelist._GerritCommitMsgHookCheck',
                lambda offer_removal: None)
    @mock.patch('git_cl.RunGit')
    @mock.patch('git_cl.RunGitSilent')
    @mock.patch('git_cl.Changelist._GitGetBranchConfigValue')
    @mock.patch('git_cl.Changelist.FetchUpstreamTuple')
    @mock.patch('git_cl.Changelist.GetCommonAncestorWithUpstream')
    @mock.patch('scm.GIT.GetBranchRef')
    @mock.patch('git_cl.Changelist.GetRemoteBranch')
    @mock.patch('scm.GIT.IsAncestor')
    @mock.patch('gclient_utils.AskForData')
    def test_upload_all_precheck_long_chain(
            self, mockAskForData, mockIsAncestor, mockGetRemoteBranch,
            mockGetBranchRef, mockGetCommonAncestorWithUpstream,
            mockFetchUpstreamTuple, mockGitGetBranchConfigValue,
            mockRunGitSilent, mockRunGit, *_mocks):

        mockGetRemoteBranch.return_value = ('origin',
                                            'refs/remotes/origin/main')
        branches = [
            'current', 'upstream3', 'blank3', 'blank2', 'upstream2', 'blank1',
            'upstream1', 'origin/main'
        ]
        mockGetBranchRef.side_effect = (
            ['refs/heads/current'] +  # detached HEAD check
            ['refs/heads/%s' % b for b in branches])
        mockGetCommonAncestorWithUpstream.side_effect = [
            'commit3.5',
            'commit3.5',
            'commit3.5',
            'commit2.5',
            'commit1.5',
            'commit1.5',
            'commit0.5',
        ]
        mockFetchUpstreamTuple.side_effect = [('.', 'refs/heads/upstream3'),
                                              ('.', 'refs/heads/blank3'),
                                              ('.', 'refs/heads/blank2'),
                                              ('.', 'refs/heads/upstream2'),
                                              ('.', 'refs/heads/blank1'),
                                              ('.', 'refs/heads/upstream1'),
                                              ('origin',
                                               'refs/heads/origin/main')]

        # end commits
        mockRunGit.side_effect = [
            'commit4', 'commit3.5', 'commit3.5', 'commit2', 'commit1.5',
            'commit1', 'commit0.5'
        ]
        mockRunGitSilent.side_effect = ['80', '81', '0', '0', '82', '0', '83']

        # Get gerrit squash hash. We only check this for branches that have a
        # diff. Set to None to trigger `must_upload_upstream`.
        mockGitGetBranchConfigValue.return_value = None

        options = optparse.Values()
        options.force = False
        options.cherry_pick_stacked = False
        orig_args = ['--preserve-tryjobs', '--chicken']

        # Case 2: upstream3 has never been uploaded.
        # (so no LAST_UPLOAD_HASH_CONIFG_KEY)

        # Case 4: upstream2's last_upload is behind upstream3's base_commit
        key = f'branch.upstream2.{git_cl.LAST_UPLOAD_HASH_CONFIG_KEY}'
        scm.GIT.SetConfig('', key, 'commit2.3')
        mockIsAncestor.side_effect = [True]

        # Case 3: upstream1's last_upload matches upstream2's base_commit
        key = f'branch.upstream1.{git_cl.LAST_UPLOAD_HASH_CONFIG_KEY}'
        scm.GIT.SetConfig('', key, 'commit1.5')

        cls, cherry_pick = git_cl._UploadAllPrecheck(options, orig_args)
        self.assertFalse(cherry_pick)
        mockAskForData.assert_called_once_with(
            "\noptions ['--preserve-tryjobs', '--chicken'] will be used for all "
            "uploads.\nAt least one parent branch in `current, upstream3, "
            "upstream2` has never been uploaded and must be uploaded before/with "
            "`upstream3`.\nPress Enter to confirm, or Ctrl+C to abort")
        self.assertEqual(len(cls), 3)

    @mock.patch('git_cl.Changelist._GerritCommitMsgHookCheck',
                lambda offer_removal: None)
    @mock.patch('git_cl.RunGit')
    @mock.patch('git_cl.RunGitSilent')
    @mock.patch('git_cl.Changelist._GitGetBranchConfigValue')
    @mock.patch('git_cl.Changelist.FetchUpstreamTuple')
    @mock.patch('git_cl.Changelist.GetCommonAncestorWithUpstream')
    @mock.patch('scm.GIT.GetBranchRef')
    @mock.patch('git_cl.Changelist.GetRemoteBranch')
    @mock.patch('scm.GIT.IsAncestor')
    @mock.patch('gclient_utils.AskForData')
    def test_upload_all_precheck_options_must_upload(
            self, mockAskForData, mockIsAncestor, mockGetRemoteBranch,
            mockGetBranchRef, mockGetCommonAncestorWithUpstream,
            mockFetchUpstreamTuple, mockGitGetBranchConfigValue,
            mockRunGitSilent, mockRunGit, *_mocks):

        mockGetRemoteBranch.return_value = ('origin',
                                            'refs/remotes/origin/main')
        branches = ['current', 'upstream3', 'main']
        mockGetBranchRef.side_effect = (
            ['refs/heads/current'] +  # detached HEAD check
            ['refs/heads/%s' % b for b in branches])

        mockGetCommonAncestorWithUpstream.side_effect = [
            'commit3.5', 'commit0.5'
        ]
        mockFetchUpstreamTuple.side_effect = [('.', 'refs/heads/upstream3'),
                                              ('origin', 'refs/heads/main')]
        mockIsAncestor.return_value = True

        # end commits
        mockRunGit.return_value = 'any-commit'
        mockRunGitSilent.return_value = '42'

        # Get gerrit squash hash. We only check this for branches that have a
        # diff.
        mockGitGetBranchConfigValue.return_value = None

        # Test case: User wants to cherry pick, but all branches must be
        # uploaded.
        options = optparse.Values()
        options.force = True
        options.cherry_pick_stacked = True
        orig_args = []
        with self.assertRaises(SystemExitMock):
            git_cl._UploadAllPrecheck(options, orig_args)

        # Test case: User does not require cherry picking
        options.cherry_pick_stacked = False
        # reset side_effects
        mockGetBranchRef.side_effect = (
            ['refs/heads/current'] +  # detached HEAD check
            ['refs/heads/%s' % b for b in branches])
        mockGetCommonAncestorWithUpstream.side_effect = [
            'commit3.5', 'commit0.5'
        ]
        mockFetchUpstreamTuple.side_effect = [('.', 'refs/heads/upstream3'),
                                              ('origin', 'refs/heads/main')]

        cls, cherry_pick = git_cl._UploadAllPrecheck(options, orig_args)
        self.assertFalse(cherry_pick)
        self.assertEqual(len(cls), 2)
        mockAskForData.assert_not_called()

        # Test case: User does not require cherry picking and not in force mode.
        options.force = False
        # reset side_effects
        mockGetBranchRef.side_effect = (
            ['refs/heads/current'] +  # detached HEAD check
            ['refs/heads/%s' % b for b in branches])
        mockGetCommonAncestorWithUpstream.side_effect = [
            'commit3.5', 'commit0.5'
        ]
        mockFetchUpstreamTuple.side_effect = [('.', 'refs/heads/upstream3'),
                                              ('origin', 'refs/heads/main')]

        cls, cherry_pick = git_cl._UploadAllPrecheck(options, orig_args)
        self.assertFalse(cherry_pick)
        self.assertEqual(len(cls), 2)
        mockAskForData.assert_called_once()

    @mock.patch('git_cl.Changelist._GerritCommitMsgHookCheck',
                lambda offer_removal: None)
    @mock.patch('git_cl.RunGit')
    @mock.patch('git_cl.RunGitSilent')
    @mock.patch('git_cl.Changelist._GitGetBranchConfigValue')
    @mock.patch('git_cl.Changelist.FetchUpstreamTuple')
    @mock.patch('git_cl.Changelist.GetCommonAncestorWithUpstream')
    @mock.patch('scm.GIT.GetBranchRef')
    @mock.patch('scm.GIT.IsAncestor')
    @mock.patch('gclient_utils.AskForData')
    def test_upload_all_precheck_must_rebase(
            self, mockAskForData, mockIsAncestor, mockGetBranchRef,
            mockGetCommonAncestorWithUpstream, mockFetchUpstreamTuple,
            mockGitGetBranchConfigValue, mockRunGitSilent, mockRunGit, *_mocks):
        branches = ['current', 'upstream3']
        mockGetBranchRef.side_effect = ['refs/heads/%s' % b for b in branches]
        mockGetCommonAncestorWithUpstream.return_value = 'commit3.5'

        mockFetchUpstreamTuple.return_value = ('.', 'refs/heads/upstream3')

        # end commits
        mockRunGit.return_value = 'commit4'
        mockRunGitSilent.return_value = '42'

        # Get gerrit squash hash. We only check this for branches that have a
        # diff. Set to None to trigger `must_upload_upstream`.
        mockGitGetBranchConfigValue.return_value = None

        # Case 5: current's base_commit is behind upstream3's last_upload.
        key = f'branch.upstream3.{git_cl.LAST_UPLOAD_HASH_CONFIG_KEY}'
        scm.GIT.SetConfig('', key, 'commit3.7')
        mockIsAncestor.side_effect = [False, True]
        with self.assertRaises(SystemExitMock):
            options = optparse.Values()
            options.force = False
            options.cherry_pick_stacked = False
            git_cl._UploadAllPrecheck(options, [])

    @mock.patch('git_cl.Changelist._GerritCommitMsgHookCheck',
                lambda offer_removal: None)
    @mock.patch('git_cl.RunGit')
    @mock.patch('git_cl.RunGitSilent')
    @mock.patch('git_cl.Changelist._GitGetBranchConfigValue')
    @mock.patch('git_cl.Changelist.FetchUpstreamTuple')
    @mock.patch('git_cl.Changelist.GetCommonAncestorWithUpstream')
    @mock.patch('scm.GIT.GetBranchRef')
    @mock.patch('git_cl.Changelist.GetRemoteBranch')
    @mock.patch('scm.GIT.IsAncestor')
    @mock.patch('gclient_utils.AskForData')
    def test_upload_all_precheck_hit_main(
            self, mockAskForData, mockIsAncestor, mockGetRemoteBranch,
            mockGetBranchRef, mockGetCommonAncestorWithUpstream,
            mockFetchUpstreamTuple, mockGitGetBranchConfigValue,
            mockRunGitSilent, mockRunGit, *_mocks):

        options = optparse.Values()
        options.force = False
        options.cherry_pick_stacked = False
        orig_args = ['--preserve-tryjobs', '--chicken']

        mockGetRemoteBranch.return_value = ('origin',
                                            'refs/remotes/origin/main')
        branches = ['current', 'upstream3', 'main']
        mockGetBranchRef.side_effect = (
            ['refs/heads/current'] +  # detached HEAD check
            ['refs/heads/%s' % b for b in branches])

        mockGetCommonAncestorWithUpstream.side_effect = [
            'commit3.5', 'commit0.5'
        ]
        mockFetchUpstreamTuple.side_effect = [('.', 'refs/heads/upstream3'),
                                              ('origin', 'refs/heads/main')]
        mockIsAncestor.return_value = True

        # Give upstream3 a last upload hash
        key = f'branch.upstream3.{git_cl.LAST_UPLOAD_HASH_CONFIG_KEY}'
        scm.GIT.SetConfig('', key, 'commit3.4')

        # end commits
        mockRunGit.return_value = 'commit4'
        mockRunGitSilent.return_value = '42'

        # Get gerrit squash hash. We only check this for branches that have a
        # diff.
        mockGitGetBranchConfigValue.return_value = 'just needs to exist'

        # Test case: user cherry picks with options
        options.cherry_pick_stacked = True
        # Reset side_effects
        mockGetBranchRef.side_effect = (
            ['refs/heads/current'] +  # detached HEAD check
            ['refs/heads/%s' % b for b in branches])
        mockGetCommonAncestorWithUpstream.side_effect = [
            'commit3.5', 'commit0.5'
        ]
        mockFetchUpstreamTuple.side_effect = [('.', 'refs/heads/upstream3'),
                                              ('origin', 'refs/heads/main')]
        cls, cherry_pick = git_cl._UploadAllPrecheck(options, orig_args)
        self.assertTrue(cherry_pick)
        self.assertEqual(len(cls), 2)
        mockAskForData.assert_not_called()

        # Test case: user uses force, no cherry-pick.
        options.cherry_pick_stacked = False
        options.force = True
        # Reset side_effects
        mockGetBranchRef.side_effect = (
            ['refs/heads/current'] +  # detached HEAD check
            ['refs/heads/%s' % b for b in branches])
        mockGetCommonAncestorWithUpstream.side_effect = [
            'commit3.5', 'commit0.5'
        ]
        mockFetchUpstreamTuple.side_effect = [('.', 'refs/heads/upstream3'),
                                              ('origin', 'refs/heads/main')]
        cls, cherry_pick = git_cl._UploadAllPrecheck(options, orig_args)
        self.assertFalse(cherry_pick)
        self.assertEqual(len(cls), 2)
        mockAskForData.assert_not_called()

        # Test case: user wants to cherry pick after being asked.
        mockAskForData.return_value = 'n'
        options.cherry_pick_stacked = False
        options.force = False
        # Reset side_effects
        mockGetBranchRef.side_effect = (
            ['refs/heads/current'] +  # detached HEAD check
            ['refs/heads/%s' % b for b in branches])
        mockGetCommonAncestorWithUpstream.side_effect = [
            'commit3.5', 'commit0.5'
        ]
        mockFetchUpstreamTuple.side_effect = [('.', 'refs/heads/upstream3'),
                                              ('origin', 'refs/heads/main')]
        cls, cherry_pick = git_cl._UploadAllPrecheck(options, orig_args)
        self.assertTrue(cherry_pick)
        self.assertEqual(len(cls), 2)
        mockAskForData.assert_called_once_with(
            "\noptions ['--preserve-tryjobs', '--chicken'] will be used for all "
            "uploads.\n"
            "Press enter to update branches current, upstream3.\n"
            "Or type `n` to upload only `current` cherry-picked on upstream3's "
            "last upload:")

    @mock.patch('git_cl.Changelist._GerritCommitMsgHookCheck',
                lambda offer_removal: None)
    @mock.patch('git_cl.RunGit')
    @mock.patch('git_cl.RunGitSilent')
    @mock.patch('git_cl.Changelist._GitGetBranchConfigValue')
    @mock.patch('git_cl.Changelist.FetchUpstreamTuple')
    @mock.patch('git_cl.Changelist.GetCommonAncestorWithUpstream')
    @mock.patch('scm.GIT.GetBranchRef')
    @mock.patch('git_cl.Changelist.GetRemoteBranch')
    @mock.patch('scm.GIT.IsAncestor')
    @mock.patch('gclient_utils.AskForData')
    def test_upload_all_precheck_one_change(
            self, mockAskForData, mockIsAncestor, mockGetRemoteBranch,
            mockGetBranchRef, mockGetCommonAncestorWithUpstream,
            mockFetchUpstreamTuple, mockGitGetBranchConfigValue,
            mockRunGitSilent, mockRunGit, *_mocks):

        options = optparse.Values()
        options.force = False
        options.cherry_pick_stacked = False
        orig_args = ['--preserve-tryjobs', '--chicken']

        mockGetRemoteBranch.return_value = ('origin',
                                            'refs/remotes/origin/main')
        mockGetBranchRef.side_effect = [
            'refs/heads/current',  # detached HEAD check
            'refs/heads/current',  # call within while loop
            'refs/heads/main',
            'refs/heads/main'
        ]
        mockGetCommonAncestorWithUpstream.return_value = 'commit3.5'
        mockFetchUpstreamTuple.return_value = ('', 'refs/heads/main')
        mockIsAncestor.return_value = True

        # end commits
        mockRunGit.return_value = 'commit4'
        mockRunGitSilent.return_value = '42'

        # Get gerrit squash hash. We only check this for branches that have a
        # diff. Set to None to trigger `must_upload_upstream`.
        mockGitGetBranchConfigValue.return_value = 'does not matter'

        # Case 1: We hit the main branch
        cls, cherry_pick = git_cl._UploadAllPrecheck(options, orig_args)
        self.assertFalse(cherry_pick)
        self.assertEqual(len(cls), 1)

        mockAskForData.assert_not_called()

        # No diff for current change
        mockRunGitSilent.return_value = '0'
        with self.assertRaises(SystemExitMock):
            git_cl._UploadAllPrecheck(options, orig_args)

    @mock.patch('scm.GIT.GetBranchRef', return_value=None)
    def test_upload_all_precheck_detached_HEAD(self, mockGetBranchRef):

        with self.assertRaises(SystemExitMock):
            git_cl._UploadAllPrecheck(optparse.Values(), [])

    @mock.patch('git_cl.RunGit')
    @mock.patch('git_cl.CMDupload')
    @mock.patch('sys.stdin', io.StringIO('\n'))
    @mock.patch('sys.stdout', io.StringIO())
    def test_upload_branch_deps(self, *_mocks):
        def mock_run_git(*args, **_kwargs):
            if args[0] == [
                    'for-each-ref',
                    '--format=%(refname:short) %(upstream:short)', 'refs/heads'
            ]:
                # Create a local branch dependency tree that looks like this:
                # test1 -> test2 -> test3   -> test4 -> test5
                #                -> test3.1
                # test6 -> test0
                branch_deps = [
                    'test2 test1',  # test1 -> test2
                    'test3 test2',  # test2 -> test3
                    'test3.1 test2',  # test2 -> test3.1
                    'test4 test3',  # test3 -> test4
                    'test5 test4',  # test4 -> test5
                    'test6 test0',  # test0 -> test6
                    'test7',  # test7
                ]
                return '\n'.join(branch_deps)

        git_cl.RunGit.side_effect = mock_run_git
        git_cl.CMDupload.return_value = 0

        class MockChangelist():
            def __init__(self):
                pass

            def GetBranch(self):
                return 'test1'

            def GetIssue(self):
                return '123'

            def GetPatchset(self):
                return '1001'

            def IsGerrit(self):
                return False

        ret = git_cl.upload_branch_deps(MockChangelist(), [])
        # CMDupload should have been called 5 times because of 5 dependent
        # branches.
        self.assertEqual(5, len(git_cl.CMDupload.mock_calls))
        self.assertEqual(0, ret)

    def test_gerrit_change_id(self):
        self.calls = [
            ((['git', 'write-tree'], ), 'hashtree'),
            ((['git', 'rev-parse', 'HEAD~0'], ), 'branch-parent'),
            ((['git', 'var',
               'GIT_AUTHOR_IDENT'], ), 'A B <a@b.org> 1456848326 +0100'),
            ((['git', 'var',
               'GIT_COMMITTER_IDENT'], ), 'C D <c@d.org> 1456858326 +0100'),
            ((['git', 'hash-object', '-t', 'commit',
               '--stdin'], ), 'hashchange'),
        ]
        change_id = git_cl.GenerateGerritChangeId('line1\nline2\n')
        self.assertEqual(change_id, 'Ihashchange')

    @mock.patch('gerrit_util.IsCodeOwnersEnabledOnHost')
    @mock.patch('git_cl.Settings.GetBugPrefix')
    @mock.patch('git_cl.Changelist.FetchDescription')
    @mock.patch('git_cl.Changelist.GetBranch')
    @mock.patch('git_cl.Changelist.GetCommonAncestorWithUpstream')
    @mock.patch('git_cl.Changelist.GetGerritHost')
    @mock.patch('git_cl.Changelist.GetGerritProject')
    @mock.patch('git_cl.Changelist.GetRemoteBranch')
    @mock.patch('owners_client.OwnersClient.BatchListOwners')
    def getDescriptionForUploadTest(self,
                                    mockBatchListOwners=None,
                                    mockGetRemoteBranch=None,
                                    mockGetGerritProject=None,
                                    mockGetGerritHost=None,
                                    mockGetCommonAncestorWithUpstream=None,
                                    mockGetBranch=None,
                                    mockFetchDescription=None,
                                    mockGetBugPrefix=None,
                                    mockIsCodeOwnersEnabledOnHost=None,
                                    initial_description='desc',
                                    commit_description=None,
                                    bug=None,
                                    fixed=None,
                                    branch='branch',
                                    reviewers=None,
                                    add_owners_to=None,
                                    expected_description='desc'):
        reviewers = reviewers or []
        owners_by_path = {
            'a': ['a@example.com'],
            'b': ['b@example.com'],
            'c': ['c@example.com'],
        }
        mockIsCodeOwnersEnabledOnHost.return_value = True
        mockGetBranch.return_value = branch
        mockGetBugPrefix.return_value = 'prefix'
        mockGetCommonAncestorWithUpstream.return_value = 'upstream'
        mockGetRemoteBranch.return_value = ('origin',
                                            'refs/remotes/origin/main')
        mockFetchDescription.return_value = 'desc'
        mockBatchListOwners.side_effect = lambda ps: {
            p: owners_by_path.get(p)
            for p in ps
        }

        cl = git_cl.Changelist(issue=1234)
        actual = cl._GetDescriptionForUpload(options=mock.Mock(
            bug=bug,
            fixed=fixed,
            reviewers=reviewers,
            add_owners_to=add_owners_to,
            message=initial_description,
            commit_description=commit_description),
                                             git_diff_args=None,
                                             files=list(owners_by_path))
        self.assertEqual(expected_description, actual.description)

    def testGetDescriptionForUpload(self):
        self.getDescriptionForUploadTest()

    def testGetDescriptionForUpload_Bug(self):
        self.getDescriptionForUploadTest(bug='1234',
                                         expected_description='\n'.join([
                                             'desc',
                                             '',
                                             'Bug: prefix:1234',
                                         ]))

    def testGetDescriptionForUpload_Fixed(self):
        self.getDescriptionForUploadTest(fixed='1234',
                                         expected_description='\n'.join([
                                             'desc',
                                             '',
                                             'Fixed: prefix:1234',
                                         ]))

    @mock.patch('git_cl.Changelist.GetIssue')
    def testGetDescriptionForUpload_BugFromBranch(self, mockGetIssue):
        mockGetIssue.return_value = None
        self.getDescriptionForUploadTest(branch='bug-1234',
                                         expected_description='\n'.join([
                                             'desc',
                                             '',
                                             'Bug: prefix:1234',
                                         ]))

    @mock.patch('git_cl.Changelist.GetIssue')
    def testGetDescriptionForUpload_FixedFromBranch(self, mockGetIssue):
        mockGetIssue.return_value = None
        self.getDescriptionForUploadTest(branch='fix-1234',
                                         expected_description='\n'.join([
                                             'desc',
                                             '',
                                             'Fixed: prefix:1234',
                                         ]))

    def testGetDescriptionForUpload_SkipBugFromBranchIfAlreadyUploaded(self):
        self.getDescriptionForUploadTest(
            branch='bug-1234',
            expected_description='desc',
        )

    def testGetDescriptionForUpload_AddOwnersToR(self):
        self.getDescriptionForUploadTest(
            reviewers=['a@example.com'],
            add_owners_to='R',
            expected_description='\n'.join([
                'desc',
                '',
                'R=a@example.com, b@example.com, c@example.com',
            ]))

    def testGetDescriptionForUpload_AddOwnersToNoOwnersNeeded(self):
        self.getDescriptionForUploadTest(
            reviewers=['a@example.com', 'c@example.com'],
            expected_description='\n'.join([
                'desc',
                '',
                'R=a@example.com, c@example.com',
            ]))

    def testGetDescriptionForUpload_Reviewers(self):
        self.getDescriptionForUploadTest(
            reviewers=['a@example.com', 'b@example.com'],
            expected_description='\n'.join([
                'desc',
                '',
                'R=a@example.com, b@example.com',
            ]))

    def testGetDescriptionForUpload_NewDesc(self):
        self.getDescriptionForUploadTest(
            commit_description='this is a new desc',
            expected_description='this is a new desc')

    @mock.patch('sys.stdin', io.StringIO('this is a new desc'))
    def testGetDescriptionForUpload_NewDescFromStdin(self):
        self.getDescriptionForUploadTest(
            commit_description='-', expected_description='this is a new desc')

    def test_description_append_footer(self):
        for init_desc, footer_line, expected_desc in [
                # Use unique desc first lines for easy test failure
                # identification.
            ('foo', 'R=one', 'foo\n\nR=one'),
            ('foo\n\nR=one', 'BUG=', 'foo\n\nR=one\nBUG='),
            ('foo\n\nR=one', 'Change-Id: Ixx',
             'foo\n\nR=one\n\nChange-Id: Ixx'),
            ('foo\n\nChange-Id: Ixx', 'R=one',
             'foo\n\nR=one\n\nChange-Id: Ixx'),
            ('foo\n\nR=one\n\nChange-Id: Ixx', 'Foo-Bar: baz',
             'foo\n\nR=one\n\nChange-Id: Ixx\nFoo-Bar: baz'),
            ('foo\n\nChange-Id: Ixx', 'Foo-Bak: baz',
             'foo\n\nChange-Id: Ixx\nFoo-Bak: baz'),
            ('foo', 'Change-Id: Ixx', 'foo\n\nChange-Id: Ixx'),
        ]:
            desc = git_cl.ChangeDescription(init_desc)
            desc.append_footer(footer_line)
            self.assertEqual(desc.description, expected_desc)

    def test_update_reviewers(self):
        data = [
            ('foo', [], 'foo'),
            ('foo\nR=xx', [], 'foo\nR=xx'),
            ('foo', ['a@c'], 'foo\n\nR=a@c'),
            ('foo\nR=xx', ['a@c'], 'foo\n\nR=a@c, xx'),
            ('foo\nBUG=', ['a@c'], 'foo\nBUG=\nR=a@c'),
            ('foo\nR=xx\nR=bar', ['a@c'], 'foo\n\nR=a@c, bar, xx'),
            ('foo', ['a@c', 'b@c'], 'foo\n\nR=a@c, b@c'),
            ('foo\nBar\n\nR=\nBUG=', ['c@c'], 'foo\nBar\n\nR=c@c\nBUG='),
            ('foo\nBar\n\nR=\nBUG=\nR=', ['c@c'], 'foo\nBar\n\nR=c@c\nBUG='),
            # Same as the line before, but full of whitespaces.
            (
                'foo\nBar\n\n R = \n BUG = \n R = ',
                ['c@c'],
                'foo\nBar\n\nR=c@c\n BUG =',
            ),
            # Whitespaces aren't interpreted as new lines.
            ('foo BUG=allo R=joe ', ['c@c'], 'foo BUG=allo R=joe\n\nR=c@c'),
        ]
        expected = [i[-1] for i in data]
        actual = []
        for orig, reviewers, _expected in data:
            obj = git_cl.ChangeDescription(orig)
            obj.update_reviewers(reviewers)
            actual.append(obj.description)
        self.assertEqual(expected, actual)

    def test_get_hash_tags(self):
        cases = [
            ('', []),
            ('a', []),
            ('[a]', ['a']),
            ('[aa]', ['aa']),
            ('[a ]', ['a']),
            ('[a- ]', ['a']),
            ('[a- b]', ['a-b']),
            ('[a--b]', ['a-b']),
            ('[a', []),
            ('[a]x', ['a']),
            ('[aa]x', ['aa']),
            ('[a b]', ['a-b']),
            ('[a  b]', ['a-b']),
            ('[a__b]', ['a-b']),
            ('[a] x', ['a']),
            ('[a][b]', ['a', 'b']),
            ('[a] [b]', ['a', 'b']),
            ('[a][b]x', ['a', 'b']),
            ('[a][b] x', ['a', 'b']),
            ('[a]\n[b]', ['a']),
            ('[a\nb]', []),
            ('[a][', ['a']),
            ('Revert "[a] feature"', ['a']),
            ('Reland "[a] feature"', ['a']),
            ('Revert: [a] feature', ['a']),
            ('Reland: [a] feature', ['a']),
            ('Revert "Reland: [a] feature"', ['a']),
            ('Foo: feature', ['foo']),
            ('Foo Bar: feature', ['foo-bar']),
            ('Change Foo::Bar', []),
            ('Foo: Change Foo::Bar', ['foo']),
            ('Revert "Foo bar: feature"', ['foo-bar']),
            ('Reland "Foo bar: feature"', ['foo-bar']),
        ]
        for desc, expected in cases:
            change_desc = git_cl.ChangeDescription(desc)
            actual = change_desc.get_hash_tags()
            self.assertEqual(
                actual, expected,
                'GetHashTags(%r) == %r, expected %r' % (desc, actual, expected))

        self.assertEqual(None, git_cl.GetTargetRef('origin', None, 'main'))
        self.assertEqual(
            None, git_cl.GetTargetRef(None, 'refs/remotes/origin/main', 'main'))

        # Check default target refs for branches.
        self.assertEqual(
            'refs/heads/main',
            git_cl.GetTargetRef('origin', 'refs/remotes/origin/main', None))
        self.assertEqual(
            'refs/heads/main',
            git_cl.GetTargetRef('origin', 'refs/remotes/origin/lkgr', None))
        self.assertEqual(
            'refs/heads/main',
            git_cl.GetTargetRef('origin', 'refs/remotes/origin/lkcr', None))
        self.assertEqual(
            'refs/branch-heads/123',
            git_cl.GetTargetRef('origin', 'refs/remotes/branch-heads/123',
                                None))
        self.assertEqual(
            'refs/diff/test',
            git_cl.GetTargetRef('origin', 'refs/remotes/origin/refs/diff/test',
                                None))
        self.assertEqual(
            'refs/heads/chrome/m42',
            git_cl.GetTargetRef('origin', 'refs/remotes/origin/chrome/m42',
                                None))

        # Check target refs for user-specified target branch.
        for branch in ('branch-heads/123', 'remotes/branch-heads/123',
                       'refs/remotes/branch-heads/123'):
            self.assertEqual(
                'refs/branch-heads/123',
                git_cl.GetTargetRef('origin', 'refs/remotes/origin/main',
                                    branch))
        for branch in ('origin/main', 'remotes/origin/main',
                       'refs/remotes/origin/main'):
            self.assertEqual(
                'refs/heads/main',
                git_cl.GetTargetRef('origin', 'refs/remotes/branch-heads/123',
                                    branch))
        for branch in ('main', 'heads/main', 'refs/heads/main'):
            self.assertEqual(
                'refs/heads/main',
                git_cl.GetTargetRef('origin', 'refs/remotes/branch-heads/123',
                                    branch))

    @mock.patch('git_common.is_dirty_git_tree', return_value=True)
    def test_patch_when_dirty(self, *_mocks):
        # Patch when local tree is dirty.
        self.assertNotEqual(git_cl.main(['patch', '123456']), 0)

    def assertIssueAndPatchset(self,
                               branch='main',
                               issue='123456',
                               patchset='7',
                               git_short_host='chromium'):
        self.assertEqual(issue,
                         scm.GIT.GetBranchConfig('', branch, 'gerritissue'))
        self.assertEqual(patchset,
                         scm.GIT.GetBranchConfig('', branch, 'gerritpatchset'))
        self.assertEqual('https://%s-review.googlesource.com' % git_short_host,
                         scm.GIT.GetBranchConfig('', branch, 'gerritserver'))

    def _patch_common(self, git_short_host='chromium'):
        mock.patch('scm.GIT.ResolveCommit', return_value='deadbeef').start()
        scm.GIT.SetConfig('', 'remote.origin.url',
                          f'https://{git_short_host}.googlesource.com/my/repo')
        gerrit_util.GetChangeDetail.return_value = {
            'current_revision': '7777777777',
            'revisions': {
                '1111111111': {
                    '_number': 1,
                    'fetch': {
                        'http': {
                            'url': 'https://%s.googlesource.com/my/repo' %
                            git_short_host,
                            'ref': 'refs/changes/56/123456/1',
                        }
                    },
                },
                '7777777777': {
                    '_number': 7,
                    'fetch': {
                        'http': {
                            'url': 'https://%s.googlesource.com/my/repo' %
                            git_short_host,
                            'ref': 'refs/changes/56/123456/7',
                        }
                    },
                },
            },
        }

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_patch_gerrit_default(self):
        self._patch_common()
        self.calls += [
            (([
                'git', 'fetch', 'https://chromium.googlesource.com/my/repo',
                'refs/changes/56/123456/7'
            ], ), ''),
            ((['git', 'cherry-pick', 'FETCH_HEAD'], ), ''),
        ]
        self.assertEqual(git_cl.main(['patch', '123456']), 0)
        self.assertIssueAndPatchset()

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_patch_gerrit_new_branch(self):
        self._patch_common()
        self.calls += [
            (([
                'git', 'fetch', 'https://chromium.googlesource.com/my/repo',
                'refs/changes/56/123456/7'
            ], ), ''),
            ((['git', 'cherry-pick', 'FETCH_HEAD'], ), ''),
        ]
        self.assertEqual(git_cl.main(['patch', '-b', 'feature', '123456']), 0)
        self.assertIssueAndPatchset(branch='feature')

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_patch_gerrit_force(self):
        self._patch_common('host')
        self.calls += [
            (([
                'git', 'fetch', 'https://host.googlesource.com/my/repo',
                'refs/changes/56/123456/7'
            ], ), ''),
            ((['git', 'reset', '--hard', 'FETCH_HEAD'], ), ''),
        ]
        self.assertEqual(git_cl.main(['patch', '123456', '--force']), 0)
        self.assertIssueAndPatchset(git_short_host='host')

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_patch_gerrit_guess_by_url(self):
        self._patch_common('else')
        self.calls += [
            (([
                'git', 'fetch', 'https://else.googlesource.com/my/repo',
                'refs/changes/56/123456/1'
            ], ), ''),
            ((['git', 'cherry-pick', 'FETCH_HEAD'], ), ''),
        ]
        self.assertEqual(
            git_cl.main(
                ['patch', 'https://else-review.googlesource.com/#/c/123456/1']),
            0)
        self.assertIssueAndPatchset(patchset='1', git_short_host='else')

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_patch_gerrit_guess_by_url_with_repo(self):
        self._patch_common('else')
        self.calls += [
            (([
                'git', 'fetch', 'https://else.googlesource.com/my/repo',
                'refs/changes/56/123456/1'
            ], ), ''),
            ((['git', 'cherry-pick', 'FETCH_HEAD'], ), ''),
        ]
        self.assertEqual(
            git_cl.main([
                'patch',
                'https://else-review.googlesource.com/c/my/repo/+/123456/1'
            ]), 0)
        self.assertIssueAndPatchset(patchset='1', git_short_host='else')

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    @mock.patch('sys.stderr', io.StringIO())
    def test_patch_gerrit_conflict(self):
        self._patch_common()
        self.calls += [
            (([
                'git', 'fetch', 'https://chromium.googlesource.com/my/repo',
                'refs/changes/56/123456/7'
            ], ), ''),
            ((['git', 'cherry-pick', 'FETCH_HEAD'], ), CERR1),
        ]
        with self.assertRaises(SystemExitMock):
            git_cl.main(['patch', '123456'])
        self.assertEqual('Command "git cherry-pick FETCH_HEAD" failed.\n\n',
                         sys.stderr.getvalue())

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    @mock.patch('gerrit_util.GetChangeDetail',
                side_effect=gerrit_util.GerritError(404, ''))
    @mock.patch('sys.stderr', io.StringIO())
    def test_patch_gerrit_not_exists(self, *_mocks):
        scm.GIT.SetConfig('', 'remote.origin.url',
                          'https://chromium.googlesource.com/my/repo')
        with self.assertRaises(SystemExitMock):
            self.assertEqual(1, git_cl.main(['patch', '123456']))
        self.assertEqual(
            'change 123456 at https://chromium-review.googlesource.com does not '
            'exist or you have no access to it\n', sys.stderr.getvalue())

    def _checkout_config(self):
        scm.GIT.SetConfig('', 'branch.ger-branch.gerritissue', '123456')
        scm.GIT.SetConfig('', 'branch.gbranch654.gerritissue', '654321')

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_checkout_gerrit(self):
        """Tests git cl checkout <issue>."""
        self._checkout_config()
        self.calls += [((['git', 'checkout', 'ger-branch'], ), '')]
        self.assertEqual(0, git_cl.main(['checkout', '123456']))

    def test_checkout_not_found(self):
        """Tests git cl checkout <issue>."""
        self._checkout_config()
        self.assertEqual(1, git_cl.main(['checkout', '99999']))

    def test_checkout_no_branch_issues(self):
        """Tests git cl checkout <issue>."""
        self.assertEqual(1, git_cl.main(['checkout', '99999']))

    def _test_gerrit_ensure_authenticated_common(self, auth):
        mock.patch(
            'gclient_utils.AskForData',
            lambda prompt: self._mocked_call('ask_for_data', prompt)).start()
        mock.patch(
            'git_cl.gerrit_util.CookiesAuthenticator',
            CookiesAuthenticatorMockFactory(hosts_with_creds=auth)).start()
        scm.GIT.SetConfig('', 'remote.origin.url',
                          'https://chromium.googlesource.com/my/repo')
        cl = git_cl.Changelist()
        cl.branch = 'main'
        cl.branchref = 'refs/heads/main'
        return cl

    @mock.patch('sys.stderr', io.StringIO())
    def test_gerrit_ensure_authenticated_missing(self):
        cl = self._test_gerrit_ensure_authenticated_common(auth={
            'chromium.googlesource.com': ('git-is.ok', 'but gerrit is missing'),
        })
        with self.assertRaises(SystemExitMock):
            cl.EnsureAuthenticated(force=False)
        self.assertEqual(
            'Credentials for the following hosts are required:\n'
            '  chromium-review.googlesource.com\n'
            'These are read from ~%(sep)s.gitcookies\n'
            'You can (re)generate your credentials by visiting '
            'https://chromium.googlesource.com/new-password\n' % {
                'sep': os.sep,
            }, sys.stderr.getvalue())

    def test_gerrit_ensure_authenticated_conflict(self):
        cl = self._test_gerrit_ensure_authenticated_common(
            auth={
                'chromium.googlesource.com': ('git-one.example.com', 'secret1'),
                'chromium-review.googlesource.com': ('git-other.example.com',
                                                     'secret2'),
            })
        self.calls.append((('ask_for_data', 'If you know what you are doing '
                            'press Enter to continue, or Ctrl+C to abort'), ''))
        self.assertIsNone(cl.EnsureAuthenticated(force=False))

    def test_gerrit_ensure_authenticated_ok(self):
        cl = self._test_gerrit_ensure_authenticated_common(
            auth={
                'chromium.googlesource.com': ('git-same.example.com', 'secret'),
                'chromium-review.googlesource.com': ('git-same.example.com',
                                                     'secret'),
            })
        self.assertIsNone(cl.EnsureAuthenticated(force=False))

    def test_gerrit_ensure_authenticated_skipped(self):
        scm.GIT.SetConfig('', 'gerrit.skip-ensure-authenticated', 'true')
        cl = self._test_gerrit_ensure_authenticated_common(auth={})
        self.assertIsNone(cl.EnsureAuthenticated(force=False))

    def test_gerrit_ensure_authenticated_sso(self):
        scm.GIT.SetConfig('', 'remote.origin.url', 'sso://repo')

        mock.patch(
            'git_cl.gerrit_util.CookiesAuthenticator',
            CookiesAuthenticatorMockFactory(hosts_with_creds={})).start()

        cl = git_cl.Changelist()
        cl.branch = 'main'
        cl.branchref = 'refs/heads/main'
        cl.lookedup_issue = True
        self.assertIsNone(cl.EnsureAuthenticated(force=False))

    def test_gerrit_ensure_authenticated_bearer_token(self):
        cl = self._test_gerrit_ensure_authenticated_common(
            auth={
                'chromium.googlesource.com': ('', 'secret'),
                'chromium-review.googlesource.com': ('', 'secret'),
            })
        self.assertIsNone(cl.EnsureAuthenticated(force=False))
        conn = gerrit_util.HttpConn(
            req_uri='???',
            req_method='GET',
            req_host='chromium.googlesource.com',
            req_headers={},
            req_body=None,
        )
        gerrit_util.CookiesAuthenticator().authenticate(conn)
        self.assertTrue('Bearer' in conn.req_headers['Authorization'])

    def test_gerrit_ensure_authenticated_non_https_sso(self):
        scm.GIT.SetConfig('', 'remote.origin.url', 'custom-scheme://repo')
        self.calls = [
            (('logging.warning',
              'Ignoring branch %(branch)s with non-https remote '
              '%(remote)s', {
                  'branch': 'main',
                  'remote': 'custom-scheme://repo'
              }), None),
        ]
        mock.patch(
            'git_cl.gerrit_util.CookiesAuthenticator',
            CookiesAuthenticatorMockFactory(hosts_with_creds={})).start()
        mock.patch('logging.warning',
                   lambda *a: self._mocked_call('logging.warning', *a)).start()
        cl = git_cl.Changelist()
        cl.branch = 'main'
        cl.branchref = 'refs/heads/main'
        cl.lookedup_issue = True
        self.assertIsNone(cl.EnsureAuthenticated(force=False))

    def test_gerrit_ensure_authenticated_non_url(self):
        scm.GIT.SetConfig('', 'remote.origin.url',
                          'git@somehost.example:foo/bar.git')
        self.calls = [
            (('logging.error',
              'Remote "%(remote)s" for branch "%(branch)s" points to "%(url)s", '
              'but it doesn\'t exist.', {
                  'remote': 'origin',
                  'branch': 'main',
                  'url': 'git@somehost.example:foo/bar.git'
              }), None),
        ]
        mock.patch(
            'git_cl.gerrit_util.CookiesAuthenticator',
            CookiesAuthenticatorMockFactory(hosts_with_creds={})).start()
        mock.patch('logging.error',
                   lambda *a: self._mocked_call('logging.error', *a)).start()
        cl = git_cl.Changelist()
        cl.branch = 'main'
        cl.branchref = 'refs/heads/main'
        cl.lookedup_issue = True
        self.assertIsNone(cl.EnsureAuthenticated(force=False))

    def _cmd_set_commit_gerrit_common(self, vote, notify=None):
        scm.GIT.SetConfig('', 'branch.main.gerritissue', '123')
        scm.GIT.SetConfig('', 'branch.main.gerritserver',
                          'https://chromium-review.googlesource.com')
        scm.GIT.SetConfig('', 'remote.origin.url',
                          'https://chromium.googlesource.com/infra/infra')
        self.calls = [
            (('SetReview', 'chromium-review.googlesource.com',
              'infra%2Finfra~123', None, {
                  'Commit-Queue': vote
              }, notify, None), ''),
        ]

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_cmd_set_commit_gerrit_clear(self):
        self._cmd_set_commit_gerrit_common(0)
        self.assertEqual(0, git_cl.main(['set-commit', '-c']))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_cmd_set_commit_gerrit_dry(self):
        self._cmd_set_commit_gerrit_common(1, notify=False)
        self.assertEqual(0, git_cl.main(['set-commit', '-d']))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_cmd_set_commit_gerrit(self):
        self._cmd_set_commit_gerrit_common(2)
        self.assertEqual(0, git_cl.main(['set-commit']))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_description_display(self):
        mock.patch('git_cl.Changelist', ChangelistMock).start()
        ChangelistMock.desc = 'foo\n'

        self.assertEqual(0, git_cl.main(['description', '-d']))
        self.assertEqual('foo\n', sys.stdout.getvalue())

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    @mock.patch('sys.stderr', io.StringIO())
    def test_StatusFieldOverrideIssueMissingArgs(self):
        try:
            self.assertEqual(git_cl.main(['status', '--issue', '1']), 0)
        except SystemExitMock:
            self.assertIn('--field must be given when --issue is set.',
                          sys.stderr.getvalue())

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_StatusFieldOverrideIssue(self):
        def assertIssue(cl_self, *_args):
            self.assertEqual(cl_self.issue, 1)
            return 'foobar'

        mock.patch('git_cl.Changelist.FetchDescription', assertIssue).start()
        self.assertEqual(
            git_cl.main(['status', '--issue', '1', '--field', 'desc']), 0)
        self.assertEqual(sys.stdout.getvalue(), 'foobar\n')

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_SetCloseOverrideIssue(self):
        def assertIssue(cl_self, *_args):
            self.assertEqual(cl_self.issue, 1)
            return 'foobar'

        mock.patch('git_cl.Changelist.FetchDescription', assertIssue).start()
        mock.patch('git_cl.Changelist.CloseIssue', lambda *_: None).start()
        self.assertEqual(git_cl.main(['set-close', '--issue', '1']), 0)

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_description(self):
        scm.GIT.SetConfig('', 'remote.origin.url',
                          'https://chromium.googlesource.com/my/repo')
        gerrit_util.GetChangeDetail.return_value = {
            'current_revision': 'sha1',
            'revisions': {
                'sha1': {
                    'commit': {
                        'message': 'foobar'
                    },
                }
            },
        }
        self.assertEqual(
            0,
            git_cl.main([
                'description',
                'https://chromium-review.googlesource.com/c/my/repo/+/123123',
                '-d'
            ]))
        self.assertEqual('foobar\n', sys.stdout.getvalue())

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_description_set_raw(self):
        mock.patch('git_cl.Changelist', ChangelistMock).start()
        mock.patch('git_cl.sys.stdin', io.StringIO('hihi')).start()

        self.assertEqual(0, git_cl.main(['description', '-n', 'hihi']))
        self.assertEqual('hihi', ChangelistMock.desc)

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_description_appends_bug_line(self):
        current_desc = 'Some.\n\nChange-Id: xxx'

        def RunEditor(desc, _, **kwargs):
            self.assertEqual(
                '# Enter a description of the change.\n'
                '# This will be displayed on the codereview site.\n'
                '# The first line will also be used as the subject of the review.\n'
                '#--------------------This line is 72 characters long'
                '--------------------\n'
                'Some.\n\nChange-Id: xxx\nBug: ', desc)
            # Simulate user changing something.
            return 'Some.\n\nChange-Id: xxx\nBug: 123'

        def UpdateDescription(_, desc, force=False):
            self.assertEqual(desc, 'Some.\n\nChange-Id: xxx\nBug: 123')

        mock.patch('git_cl.Changelist.FetchDescription',
                   lambda *args: current_desc).start()
        mock.patch('git_cl.Changelist.UpdateDescription',
                   UpdateDescription).start()
        mock.patch('git_cl.gclient_utils.RunEditor', RunEditor).start()

        scm.GIT.SetConfig('', 'branch.main.gerritissue', '123')
        self.assertEqual(0, git_cl.main(['description']))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_description_does_not_append_bug_line_if_fixed_is_present(self):
        current_desc = 'Some.\n\nFixed: 123\nChange-Id: xxx'

        def RunEditor(desc, _, **kwargs):
            self.assertEqual(
                '# Enter a description of the change.\n'
                '# This will be displayed on the codereview site.\n'
                '# The first line will also be used as the subject of the review.\n'
                '#--------------------This line is 72 characters long'
                '--------------------\n'
                'Some.\n\nFixed: 123\nChange-Id: xxx', desc)
            return desc

        mock.patch('git_cl.Changelist.FetchDescription',
                   lambda *args: current_desc).start()
        mock.patch('git_cl.gclient_utils.RunEditor', RunEditor).start()

        scm.GIT.SetConfig('', 'branch.main.gerritissue', '123')
        self.assertEqual(0, git_cl.main(['description']))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_description_set_stdin(self):
        mock.patch('git_cl.Changelist', ChangelistMock).start()
        mock.patch('git_cl.sys.stdin',
                   io.StringIO('hi \r\n\t there\n\nman')).start()

        self.assertEqual(0, git_cl.main(['description', '-n', '-']))
        self.assertEqual('hi\n\t there\n\nman', ChangelistMock.desc)

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_archive(self):
        self.calls = [
            ((['git', 'for-each-ref', '--format=%(refname)', 'refs/heads'], ),
             'refs/heads/main\nrefs/heads/foo\nrefs/heads/bar'),
            ((['git', 'for-each-ref', '--format=%(refname)',
               'refs/tags'], ), ''),
            ((['git', 'tag', 'git-cl-archived-456-foo', 'foo'], ), ''),
            ((['git', 'branch', '-D', 'foo'], ), '')
        ]

        mock.patch(
            'git_cl.get_cl_statuses',
            lambda branches, fine_grained, max_processes: [
                (MockChangelistWithBranchAndIssue('main', 1), 'open'),
                (MockChangelistWithBranchAndIssue('foo', 456), 'closed'),
                (MockChangelistWithBranchAndIssue('bar', 789), 'open')
            ]).start()

        self.assertEqual(0, git_cl.main(['archive', '-f']))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_archive_tag_collision(self):
        self.calls = [
            ((['git', 'for-each-ref', '--format=%(refname)', 'refs/heads'], ),
             'refs/heads/main\nrefs/heads/foo\nrefs/heads/bar'),
            ((['git', 'for-each-ref', '--format=%(refname)',
               'refs/tags'], ), 'refs/tags/git-cl-archived-456-foo'),
            ((['git', 'tag', 'git-cl-archived-456-foo-2', 'foo'], ), ''),
            ((['git', 'branch', '-D', 'foo'], ), '')
        ]

        mock.patch(
            'git_cl.get_cl_statuses',
            lambda branches, fine_grained, max_processes: [
                (MockChangelistWithBranchAndIssue('main', 1), 'open'),
                (MockChangelistWithBranchAndIssue('foo', 456), 'closed'),
                (MockChangelistWithBranchAndIssue('bar', 789), 'open')
            ]).start()

        self.assertEqual(0, git_cl.main(['archive', '-f']))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_archive_current_branch_fails(self):
        self.calls = [
            ((['git', 'for-each-ref', '--format=%(refname)',
               'refs/heads'], ), 'refs/heads/main'),
            ((['git', 'for-each-ref', '--format=%(refname)',
               'refs/tags'], ), ''),
        ]

        mock.patch(
            'git_cl.get_cl_statuses',
            lambda branches, fine_grained, max_processes: [
                (MockChangelistWithBranchAndIssue('main', 1), 'closed')
            ]).start()

        self.assertEqual(1, git_cl.main(['archive', '-f']))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_archive_dry_run(self):
        self.calls = [
            ((['git', 'for-each-ref', '--format=%(refname)', 'refs/heads'], ),
             'refs/heads/main\nrefs/heads/foo\nrefs/heads/bar'),
            ((['git', 'for-each-ref', '--format=%(refname)',
               'refs/tags'], ), ''),
        ]

        mock.patch(
            'git_cl.get_cl_statuses',
            lambda branches, fine_grained, max_processes: [
                (MockChangelistWithBranchAndIssue('main', 1), 'open'),
                (MockChangelistWithBranchAndIssue('foo', 456), 'closed'),
                (MockChangelistWithBranchAndIssue('bar', 789), 'open')
            ]).start()

        self.assertEqual(0, git_cl.main(['archive', '-f', '--dry-run']))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_archive_no_tags(self):
        self.calls = [
            ((['git', 'for-each-ref', '--format=%(refname)', 'refs/heads'], ),
             'refs/heads/main\nrefs/heads/foo\nrefs/heads/bar'),
            ((['git', 'for-each-ref', '--format=%(refname)',
               'refs/tags'], ), ''), ((['git', 'branch', '-D', 'foo'], ), '')
        ]

        mock.patch(
            'git_cl.get_cl_statuses',
            lambda branches, fine_grained, max_processes: [
                (MockChangelistWithBranchAndIssue('main', 1), 'open'),
                (MockChangelistWithBranchAndIssue('foo', 456), 'closed'),
                (MockChangelistWithBranchAndIssue('bar', 789), 'open')
            ]).start()

        self.assertEqual(0, git_cl.main(['archive', '-f', '--notags']))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_archive_tag_cleanup_on_branch_deletion_error(self):
        self.calls = [
            ((['git', 'for-each-ref', '--format=%(refname)', 'refs/heads'], ),
             'refs/heads/main\nrefs/heads/foo\nrefs/heads/bar'),
            ((['git', 'for-each-ref', '--format=%(refname)',
               'refs/tags'], ), ''),
            ((['git', 'tag', 'git-cl-archived-456-foo',
               'foo'], ), 'refs/tags/git-cl-archived-456-foo'),
            ((['git', 'branch', '-D', 'foo'], ), CERR1),
            ((['git', 'tag', '-d', 'git-cl-archived-456-foo'], ),
             'refs/tags/git-cl-archived-456-foo'),
        ]

        mock.patch(
            'git_cl.get_cl_statuses',
            lambda branches, fine_grained, max_processes: [
                (MockChangelistWithBranchAndIssue('main', 1), 'open'),
                (MockChangelistWithBranchAndIssue('foo', 456), 'closed'),
                (MockChangelistWithBranchAndIssue('bar', 789), 'open')
            ]).start()

        self.assertEqual(0, git_cl.main(['archive', '-f']))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_archive_with_format(self):
        self.calls = [
            ((['git', 'for-each-ref', '--format=%(refname)', 'refs/heads'], ),
             'refs/heads/main\nrefs/heads/foo\nrefs/heads/bar'),
            ((['git', 'for-each-ref', '--format=%(refname)',
               'refs/tags'], ), ''),
            ((['git', 'tag', 'archived/12-foo', 'foo'], ), ''),
            ((['git', 'branch', '-D', 'foo'], ), ''),
        ]

        mock.patch(
            'git_cl.get_cl_statuses',
            lambda branches, fine_grained, max_processes: [
                (MockChangelistWithBranchAndIssue('foo', 12), 'closed')
            ]).start()

        self.assertEqual(
            0, git_cl.main(['archive', '-f', '-p',
                            'archived/{issue}-{branch}']))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_cmd_issue_erase_existing(self):
        scm.GIT.SetConfig('', 'branch.main.gerritissue', '123')
        scm.GIT.SetConfig('', 'branch.main.gerritserver',
                          'https://chromium-review.googlesource.com')
        self.calls = [
            ((['git', 'log', '-1', '--format=%B'], ), 'This is a description'),
        ]
        self.assertEqual(0, git_cl.main(['issue', '0']))
        self.assertIsNone(scm.GIT.GetConfig('root', 'branch.main.gerritissue'))
        self.assertIsNone(scm.GIT.GetConfig('root', 'branch.main.gerritserver'))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_cmd_issue_erase_existing_with_change_id(self):
        scm.GIT.SetConfig('', 'branch.main.gerritissue', '123')
        scm.GIT.SetConfig('', 'branch.main.gerritserver',
                          'https://chromium-review.googlesource.com')
        mock.patch(
            'git_cl.Changelist.FetchDescription',
            lambda _: 'This is a description\n\nChange-Id: Ideadbeef').start()
        self.calls = [
            ((['git', 'log', '-1', '--format=%B'], ),
             'This is a description\n\nChange-Id: Ideadbeef'),
            ((['git', 'commit', '--amend', '-m',
               'This is a description\n'], ), ''),
        ]
        self.assertEqual(0, git_cl.main(['issue', '0']))
        self.assertIsNone(scm.GIT.GetConfig('root', 'branch.main.gerritissue'))
        self.assertIsNone(scm.GIT.GetConfig('root', 'branch.main.gerritserver'))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_cmd_issue_json(self):
        scm.GIT.SetConfig('', 'branch.main.gerritissue', '123')
        scm.GIT.SetConfig('', 'branch.main.gerritserver',
                          'https://chromium-review.googlesource.com')
        scm.GIT.SetConfig('', 'remote.origin.url',
                          'https://chromium.googlesource.com/chromium/src')
        self.calls = [(
            (
                'write_json',
                'output.json',
                {
                    'issue': 123,
                    'issue_url': 'https://chromium-review.googlesource.com/123',
                    'gerrit_host': 'chromium-review.googlesource.com',
                    'gerrit_project': 'chromium/src',
                },
            ),
            '',
        )]
        self.assertEqual(0, git_cl.main(['issue', '--json', 'output.json']))

    def _common_GerritCommitMsgHookCheck(self):
        mock.patch('git_cl.os.path.abspath',
                   lambda path: self._mocked_call(['abspath', path])).start()
        mock.patch('git_cl.os.path.exists',
                   lambda path: self._mocked_call(['exists', path])).start()
        mock.patch('git_cl.gclient_utils.FileRead',
                   lambda path: self._mocked_call(['FileRead', path])).start()
        mock.patch(
            'git_cl.gclient_utils.rm_file_or_tree',
            lambda path: self._mocked_call(['rm_file_or_tree', path])).start()
        mock.patch(
            'gclient_utils.AskForData',
            lambda prompt: self._mocked_call('ask_for_data', prompt)).start()
        return git_cl.Changelist(issue=123)

    def test_GerritCommitMsgHookCheck_custom_hook(self):
        cl = self._common_GerritCommitMsgHookCheck()
        self.calls += [
            ((['exists', os.path.join('.git', 'hooks', 'commit-msg')], ), True),
            ((['FileRead',
               os.path.join('.git', 'hooks',
                            'commit-msg')], ), '#!/bin/sh\necho "custom hook"')
        ]
        cl._GerritCommitMsgHookCheck(offer_removal=True)

    def test_GerritCommitMsgHookCheck_not_exists(self):
        cl = self._common_GerritCommitMsgHookCheck()
        self.calls += [
            ((['exists', os.path.join('.git', 'hooks',
                                      'commit-msg')], ), False),
        ]
        cl._GerritCommitMsgHookCheck(offer_removal=True)

    def test_GerritCommitMsgHookCheck(self):
        cl = self._common_GerritCommitMsgHookCheck()
        self.calls += [
            ((['exists', os.path.join('.git', 'hooks', 'commit-msg')], ), True),
            ((['FileRead',
               os.path.join('.git', 'hooks', 'commit-msg')], ),
             '...\n# From Gerrit Code Review\n...\nadd_ChangeId()\n'),
            (('ask_for_data', 'Do you want to remove it now? [Yes/No]: '),
             'Yes'),
            ((['rm_file_or_tree',
               os.path.join('.git', 'hooks', 'commit-msg')], ), ''),
        ]
        cl._GerritCommitMsgHookCheck(offer_removal=True)

    def test_GerritCmdLand(self):
        scm.GIT.SetConfig('', 'branch.main.gerritsquashhash', 'deadbeaf')
        scm.GIT.SetConfig('', 'branch.main.gerritserver',
                          'chromium-review.googlesource.com')
        self.calls += [
            ((['git', 'diff', 'deadbeaf'], ), ''),  # No diff.
        ]
        cl = git_cl.Changelist(issue=123)
        cl._GetChangeDetail = lambda *args, **kwargs: {
            'labels': {},
            'current_revision': 'deadbeaf',
        }
        cl._GetChangeCommit = lambda: {
            'commit':
            'deadbeef',
            'web_links': [{
                'name': 'gitiles',
                'url': 'https://git.googlesource.com/test/+/deadbeef'
            }],
        }
        cl.SubmitIssue = lambda: None
        self.assertEqual(
            0,
            cl.CMDLand(force=True,
                       bypass_hooks=True,
                       verbose=True,
                       parallel=False,
                       resultdb=False,
                       realm=None))
        self.assertIn(
            'Issue chromium-review.googlesource.com/123 has been submitted',
            sys.stdout.getvalue())
        self.assertIn('Landed as: https://git.googlesource.com/test/+/deadbeef',
                      sys.stdout.getvalue())

    def _mock_gerrit_changes_for_detail_cache(self):
        mock.patch('git_cl.Changelist.GetGerritHost', lambda _: 'host').start()

    def test_gerrit_change_detail_cache_simple(self):
        self._mock_gerrit_changes_for_detail_cache()
        gerrit_util.GetChangeDetail.side_effect = ['a', 'b']
        cl1 = git_cl.Changelist(issue=1)
        cl1._cached_remote_url = (
            True, 'https://chromium.googlesource.com/a/my/repo.git/')
        cl2 = git_cl.Changelist(issue=2)
        cl2._cached_remote_url = (True,
                                  'https://chromium.googlesource.com/ab/repo')
        self.assertEqual(cl1._GetChangeDetail(), 'a')  # Miss.
        self.assertEqual(cl1._GetChangeDetail(), 'a')
        self.assertEqual(cl2._GetChangeDetail(), 'b')  # Miss.

    def test_gerrit_change_detail_cache_options(self):
        self._mock_gerrit_changes_for_detail_cache()
        gerrit_util.GetChangeDetail.side_effect = ['cab', 'ad']
        cl = git_cl.Changelist(issue=1)
        cl._cached_remote_url = (True,
                                 'https://chromium.googlesource.com/repo/')
        self.assertEqual(cl._GetChangeDetail(options=['C', 'A', 'B']), 'cab')
        self.assertEqual(cl._GetChangeDetail(options=['A', 'B', 'C']), 'cab')
        self.assertEqual(cl._GetChangeDetail(options=['B', 'A']), 'cab')
        self.assertEqual(cl._GetChangeDetail(options=['C']), 'cab')
        self.assertEqual(cl._GetChangeDetail(options=['A']), 'cab')
        self.assertEqual(cl._GetChangeDetail(), 'cab')

        self.assertEqual(cl._GetChangeDetail(options=['A', 'D']), 'ad')
        self.assertEqual(cl._GetChangeDetail(options=['A']), 'cab')
        self.assertEqual(cl._GetChangeDetail(options=['D']), 'ad')
        self.assertEqual(cl._GetChangeDetail(), 'cab')

    def test_gerrit_description_caching(self):
        gerrit_util.GetChangeDetail.return_value = {
            'current_revision': 'rev1',
            'revisions': {
                'rev1': {
                    'commit': {
                        'message': 'desc1'
                    }
                },
            },
        }

        self._mock_gerrit_changes_for_detail_cache()
        cl = git_cl.Changelist(issue=1)
        cl._cached_remote_url = (
            True, 'https://chromium.googlesource.com/a/my/repo.git/')
        self.assertEqual(cl.FetchDescription(), 'desc1')
        self.assertEqual(cl.FetchDescription(), 'desc1')  # cache hit.

    def test_print_current_creds(self):
        class CookiesAuthenticatorMock(object):
            def __init__(self):
                self.gitcookies = {
                    'host.googlesource.com': ('user', 'pass'),
                    'host-review.googlesource.com': ('user', 'pass'),
                }

        mock.patch('git_cl.gerrit_util.CookiesAuthenticator',
                   CookiesAuthenticatorMock).start()
        git_cl._GitCookiesChecker().print_current_creds()
        self.assertEqual(list(sys.stdout.getvalue().splitlines()), [
            'Your .gitcookies have credentials for these hosts:',
            '                        Host\tUser\t Which file',
            '============================\t====\t===========',
            'host-review.googlesource.com\tuser\t.gitcookies',
            '       host.googlesource.com\tuser\t.gitcookies',
        ])
        sys.stdout.seek(0)
        sys.stdout.truncate(0)
        git_cl._GitCookiesChecker().print_current_creds()
        self.assertEqual(list(sys.stdout.getvalue().splitlines()), [
            'Your .gitcookies have credentials for these hosts:',
            '                        Host\tUser\t Which file',
            '============================\t====\t===========',
            'host-review.googlesource.com\tuser\t.gitcookies',
            '       host.googlesource.com\tuser\t.gitcookies',
        ])

    def _common_creds_check_mocks(self):
        def exists_mock(path):
            dirname = os.path.dirname(path)
            if dirname == os.path.expanduser('~'):
                dirname = '~'
            base = os.path.basename(path)
            if base == '.gitcookies':
                return self._mocked_call('os.path.exists',
                                         os.path.join(dirname, base))
            # git cl also checks for existence other files not relevant to this
            # test.
            return None

        mock.patch(
            'gclient_utils.AskForData',
            lambda prompt: self._mocked_call('ask_for_data', prompt)).start()
        mock.patch('os.path.exists', exists_mock).start()

    def test_creds_check_gitcookies_not_configured(self):
        self._common_creds_check_mocks()
        mock.patch('git_cl._GitCookiesChecker.get_hosts_with_creds',
                   lambda _: []).start()
        self.calls = [
            (('ask_for_data', 'Press Enter to setup .gitcookies, '
              'or Ctrl+C to abort'), ''),
        ]
        self.assertEqual(0, git_cl.main(['creds-check']))
        self.assertIn('\nConfigured git to use .gitcookies from',
                      sys.stdout.getvalue())

    def test_creds_check_gitcookies_configured_custom_broken(self):
        self._common_creds_check_mocks()

        custom_cookie_path = ('C:\\.gitcookies' if sys.platform == 'win32' else
                              '/custom/.gitcookies')
        scm.GIT.SetConfig('', 'http.cookiefile', custom_cookie_path)
        os.environ['GIT_COOKIES_PATH'] = '/official/.gitcookies'

        mock.patch('git_cl._GitCookiesChecker.get_hosts_with_creds',
                   lambda _: []).start()
        self.calls = [
            (('os.path.exists', custom_cookie_path), False),
            (('ask_for_data', 'Reconfigure git to use default .gitcookies? '
              'Press Enter to reconfigure, or Ctrl+C to abort'), ''),
        ]
        self.assertEqual(0, git_cl.main(['creds-check']))
        self.assertIn(
            'WARNING: You have configured custom path to .gitcookies: ',
            sys.stdout.getvalue())
        self.assertIn('However, your configured .gitcookies file is missing.',
                      sys.stdout.getvalue())

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_git_cl_comment_add_gerrit(self):
        git_new_branch.create_new_branch(None)  # hits mock from scm_mock.GIT.
        scm.GIT.SetConfig('', 'remote.origin.url',
                          'https://chromium.googlesource.com/infra/infra')
        self.calls = [
            (('SetReview', 'chromium-review.googlesource.com',
              'infra%2Finfra~10', 'msg', None, None, None), None),
        ]
        self.assertEqual(0, git_cl.main(['comment', '-i', '10', '-a', 'msg']))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    @mock.patch('git_cl.Changelist.GetBranch', return_value='foo')
    def test_git_cl_comments_fetch_gerrit(self, *_mocks):
        scm.GIT.SetConfig('', 'remote.origin.url',
                          'https://chromium.googlesource.com/infra/infra')
        gerrit_util.GetChangeDetail.return_value = {
            'owner': {
                'email': 'owner@example.com'
            },
            'current_revision':
            'ba5eba11',
            'revisions': {
                'deadbeaf': {
                    '_number': 1,
                },
                'ba5eba11': {
                    '_number': 2,
                },
            },
            'messages': [
                {
                    u'_revision_number': 1,
                    u'author': {
                        u'_account_id': 1111084,
                        u'email': u'could-be-anything@example.com',
                        u'name': u'LUCI CQ'
                    },
                    u'date': u'2017-03-15 20:08:45.000000000',
                    u'id': u'f5a6c25ecbd3b3b54a43ae418ed97eff046dc50b',
                    u'message':
                    u'Patch Set 1:\n\nDry run: CQ is trying the patch...',
                    u'tag': u'autogenerated:cv:dry-run'
                },
                {
                    u'_revision_number': 2,
                    u'author': {
                        u'_account_id': 11151243,
                        u'email': u'owner@example.com',
                        u'name': u'owner'
                    },
                    u'date': u'2017-03-16 20:00:41.000000000',
                    u'id': u'f5a6c25ecbd3b3b54a43ae418ed97eff046d1234',
                    u'message': u'PTAL',
                },
                {
                    u'_revision_number': 2,
                    u'author': {
                        u'_account_id': 148512,
                        u'email': u'reviewer@example.com',
                        u'name': u'reviewer'
                    },
                    u'date': u'2017-03-17 05:19:37.500000000',
                    u'id': u'f5a6c25ecbd3b3b54a43ae418ed97eff046d4568',
                    u'message': u'Patch Set 2: Code-Review+1',
                },
                {
                    u'_revision_number': 2,
                    u'author': {
                        u'_account_id': 42,
                        u'name': u'reviewer'
                    },
                    u'date': u'2017-03-17 05:19:37.900000000',
                    u'id': u'f5a6c25ecbd3b3b54a43ae418ed97eff046d0000',
                    u'message': u'A bot with no email set',
                },
            ]
        }
        self.calls = [
            (('GetChangeComments', 'chromium-review.googlesource.com',
              'infra%2Finfra~1'), {
                  '/COMMIT_MSG': [
                      {
                          'author': {
                              'email': u'reviewer@example.com'
                          },
                          'updated': u'2017-03-17 05:19:37.500000000',
                          'patch_set': 2,
                          'side': 'REVISION',
                          'message': 'Please include a bug link',
                      },
                  ],
                  'codereview.settings': [
                      {
                          'author': {
                              'email': u'owner@example.com'
                          },
                          'updated': u'2017-03-16 20:00:41.000000000',
                          'patch_set': 2,
                          'side': 'PARENT',
                          'line': 42,
                          'message': 'I removed this because it is bad',
                      },
                  ]
              }),
            (('GetChangeRobotComments', 'chromium-review.googlesource.com',
              'infra%2Finfra~1'), {}),
        ] * 2 + [(('write_json', 'output.json', [{
            u'date':
            u'2017-03-16 20:00:41.000000',
            u'message': (u'PTAL\n' + u'\n' + u'codereview.settings\n' +
                         u'  Base, Line 42: https://crrev.com/c/1/2/'
                         u'codereview.settings#b42\n' +
                         u'  I removed this because it is bad\n'),
            u'autogenerated':
            False,
            u'approval':
            False,
            u'disapproval':
            False,
            u'sender':
            u'owner@example.com'
        }, {
            u'date':
            u'2017-03-17 05:19:37.500000',
            u'message':
            (u'Patch Set 2: Code-Review+1\n' + u'\n' + u'/COMMIT_MSG\n' +
             u'  PS2, File comment: https://crrev.com/c/1/2//COMMIT_MSG#\n' +
             u'  Please include a bug link\n'),
            u'autogenerated':
            False,
            u'approval':
            False,
            u'disapproval':
            False,
            u'sender':
            u'reviewer@example.com'
        }]), '')]
        expected_comments_summary = [
            git_cl._CommentSummary(
                message=(u'PTAL\n' + u'\n' + u'codereview.settings\n' +
                         u'  Base, Line 42: https://crrev.com/c/1/2/' +
                         u'codereview.settings#b42\n' +
                         u'  I removed this because it is bad\n'),
                date=datetime.datetime(2017, 3, 16, 20, 0, 41, 0),
                autogenerated=False,
                disapproval=False,
                approval=False,
                sender=u'owner@example.com'),
            git_cl._CommentSummary(message=(
                u'Patch Set 2: Code-Review+1\n' + u'\n' + u'/COMMIT_MSG\n' +
                u'  PS2, File comment: https://crrev.com/c/1/2//COMMIT_MSG#\n' +
                u'  Please include a bug link\n'),
                                   date=datetime.datetime(
                                       2017, 3, 17, 5, 19, 37, 500000),
                                   autogenerated=False,
                                   disapproval=False,
                                   approval=False,
                                   sender=u'reviewer@example.com'),
        ]
        cl = git_cl.Changelist(issue=1, branchref='refs/heads/foo')
        self.assertEqual(cl.GetCommentsSummary(), expected_comments_summary)
        self.assertEqual(
            0, git_cl.main(['comments', '-i', '1', '-j', 'output.json']))

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_git_cl_comments_robot_comments(self):
        # git cl comments also fetches robot comments (which are considered a
        # type of autogenerated comment), and unlike other types of comments,
        # only robot comments from the latest patchset are shown.
        scm.GIT.SetConfig('', 'remote.origin.url',
                          'https://x.googlesource.com/infra/infra')
        gerrit_util.GetChangeDetail.return_value = {
            'owner': {
                'email': 'owner@example.com'
            },
            'current_revision':
            'ba5eba11',
            'revisions': {
                'deadbeaf': {
                    '_number': 1,
                },
                'ba5eba11': {
                    '_number': 2,
                },
            },
            'messages': [
                {
                    u'_revision_number': 1,
                    u'author': {
                        u'_account_id': 1111084,
                        u'email': u'commit-bot@chromium.org',
                        u'name': u'Commit Bot'
                    },
                    u'date': u'2017-03-15 20:08:45.000000000',
                    u'id': u'f5a6c25ecbd3b3b54a43ae418ed97eff046dc50b',
                    u'message':
                    u'Patch Set 1:\n\nDry run: CQ is trying the patch...',
                    u'tag': u'autogenerated:cq:dry-run'
                },
                {
                    u'_revision_number': 1,
                    u'author': {
                        u'_account_id': 123,
                        u'email': u'tricium@serviceaccount.com',
                        u'name': u'Tricium'
                    },
                    u'date': u'2017-03-16 20:00:41.000000000',
                    u'id': u'f5a6c25ecbd3b3b54a43ae418ed97eff046d1234',
                    u'message': u'(1 comment)',
                    u'tag': u'autogenerated:tricium',
                },
                {
                    u'_revision_number': 1,
                    u'author': {
                        u'_account_id': 123,
                        u'email': u'tricium@serviceaccount.com',
                        u'name': u'Tricium'
                    },
                    u'date': u'2017-03-16 20:00:41.000000000',
                    u'id': u'f5a6c25ecbd3b3b54a43ae418ed97eff046d1234',
                    u'message': u'(1 comment)',
                    u'tag': u'autogenerated:tricium',
                },
                {
                    u'_revision_number': 2,
                    u'author': {
                        u'_account_id': 123,
                        u'email': u'tricium@serviceaccount.com',
                        u'name': u'reviewer'
                    },
                    u'date': u'2017-03-17 05:30:37.000000000',
                    u'tag': u'autogenerated:tricium',
                    u'id': u'f5a6c25ecbd3b3b54a43ae418ed97eff046d4568',
                    u'message': u'(1 comment)',
                },
            ]
        }
        self.calls = [
            (('GetChangeComments', 'x-review.googlesource.com',
              'infra%2Finfra~1'), {}),
            (('GetChangeRobotComments', 'x-review.googlesource.com',
              'infra%2Finfra~1'), {
                  'codereview.settings': [
                      {
                          u'author': {
                              u'email': u'tricium@serviceaccount.com'
                          },
                          u'updated': u'2017-03-17 05:30:37.000000000',
                          u'robot_run_id': u'5565031076855808',
                          u'robot_id': u'Linter/Category',
                          u'tag': u'autogenerated:tricium',
                          u'patch_set': 2,
                          u'side': u'REVISION',
                          u'message': u'Linter warning message text',
                          u'line': 32,
                      },
                  ],
              }),
        ]
        expected_comments_summary = [
            git_cl._CommentSummary(
                date=datetime.datetime(2017, 3, 17, 5, 30, 37),
                message=(
                    u'(1 comment)\n\ncodereview.settings\n'
                    u'  PS2, Line 32: https://x-review.googlesource.com/c/1/2/'
                    u'codereview.settings#32\n'
                    u'  Linter warning message text\n'),
                sender=u'tricium@serviceaccount.com',
                autogenerated=True,
                approval=False,
                disapproval=False)
        ]
        cl = git_cl.Changelist(issue=1, branchref='refs/heads/foo')
        self.assertEqual(cl.GetCommentsSummary(), expected_comments_summary)

    def test_get_remote_url_with_mirror(self):
        original_os_path_isdir = os.path.isdir

        def selective_os_path_isdir_mock(path):
            if path == '/cache/this-dir-exists':
                return self._mocked_call('os.path.isdir', path)
            return original_os_path_isdir(path)

        mock.patch('os.path.isdir', selective_os_path_isdir_mock).start()

        url = 'https://chromium.googlesource.com/my/repo'
        scm.GIT.SetConfig('', 'remote.origin.url', '/cache/this-dir-exists')
        scm.GIT.SetConfig('/cache/this-dir-exists', 'remote.origin.url', url)
        self.calls = [
            (('os.path.isdir', '/cache/this-dir-exists'), True),
        ]
        cl = git_cl.Changelist(issue=1)
        self.assertEqual(cl.GetRemoteUrl(), url)
        self.assertEqual(cl.GetRemoteUrl(), url)  # Must be cached.

    def test_get_remote_url_non_existing_mirror(self):
        original_os_path_isdir = os.path.isdir

        def selective_os_path_isdir_mock(path):
            if path == '/cache/this-dir-doesnt-exist':
                return self._mocked_call('os.path.isdir', path)
            return original_os_path_isdir(path)

        mock.patch('os.path.isdir', selective_os_path_isdir_mock).start()
        mock.patch('logging.error',
                   lambda *a: self._mocked_call('logging.error', *a)).start()

        scm.GIT.SetConfig('', 'remote.origin.url',
                          '/cache/this-dir-doesnt-exist')
        self.calls = [
            (('os.path.isdir', '/cache/this-dir-doesnt-exist'), False),
            (('logging.error',
              'Remote "%(remote)s" for branch "%(branch)s" points to "%(url)s", '
              'but it doesn\'t exist.', {
                  'remote': 'origin',
                  'branch': 'main',
                  'url': '/cache/this-dir-doesnt-exist'
              }), None),
        ]
        cl = git_cl.Changelist(issue=1)
        self.assertIsNone(cl.GetRemoteUrl())

    def test_get_remote_url_misconfigured_mirror(self):
        original_os_path_isdir = os.path.isdir

        def selective_os_path_isdir_mock(path):
            if path == '/cache/this-dir-exists':
                return self._mocked_call('os.path.isdir', path)
            return original_os_path_isdir(path)

        mock.patch('os.path.isdir', selective_os_path_isdir_mock).start()
        mock.patch('logging.error',
                   lambda *a: self._mocked_call('logging.error', *a)).start()

        scm.GIT.SetConfig('', 'remote.origin.url', '/cache/this-dir-exists')
        self.calls = [
            (('os.path.isdir', '/cache/this-dir-exists'), True),
            (('logging.error',
              'Remote "%(remote)s" for branch "%(branch)s" points to '
              '"%(cache_path)s", but it is misconfigured.\n'
              '"%(cache_path)s" must be a git repo and must have a remote named '
              '"%(remote)s" pointing to the git host.', {
                  'remote': 'origin',
                  'cache_path': '/cache/this-dir-exists',
                  'branch': 'main'
              }), None),
        ]
        cl = git_cl.Changelist(issue=1)
        self.assertIsNone(cl.GetRemoteUrl())

    def test_gerrit_change_identifier_with_project(self):
        scm.GIT.SetConfig('', 'remote.origin.url',
                          'https://chromium.googlesource.com/a/my/repo.git/')
        cl = git_cl.Changelist(issue=123456)
        self.assertEqual(cl._GerritChangeIdentifier(), 'my%2Frepo~123456')

    def test_gerrit_change_identifier_without_project(self):
        mock.patch('logging.error',
                   lambda *a: self._mocked_call('logging.error', *a)).start()

        self.calls = [
            (('logging.error',
              'Remote "%(remote)s" for branch "%(branch)s" points to "%(url)s", '
              'but it doesn\'t exist.', {
                  'remote': 'origin',
                  'branch': 'main',
                  'url': ''
              }), None),
        ]
        cl = git_cl.Changelist(issue=123456)
        self.assertEqual(cl._GerritChangeIdentifier(), '123456')

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_gerrit_new_default(self):
        self._run_gerrit_upload_test(
            [],
            'desc ✔\n\nBUG=\n\nChange-Id: I123456789\n', [],
            squash=False,
            squash_mode='override_nosquash',
            change_id='I123456789',
            default_branch='main')

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def test_gerrit_nosquash_with_issue(self):
        self._run_gerrit_upload_test(
            [],
            'desc ✔\n\nBUG=\n\nChange-Id: I123456789\n', [],
            squash=False,
            squash_mode='override_nosquash',
            issue=123456,
            change_id='I123456789',
            default_branch='main')


class ChangelistTest(unittest.TestCase):
    LAST_COMMIT_SUBJECT = 'Fixes goat teleporter destination to be Australia'

    def _mock_run_git(commands):
        if commands == ['show', '-s', '--format=%s', 'HEAD', '--']:
            return ChangelistTest.LAST_COMMIT_SUBJECT

    def setUp(self):
        super(ChangelistTest, self).setUp()
        mock.patch('gclient_utils.FileRead').start()
        mock.patch('gclient_utils.FileWrite').start()
        mock.patch('gclient_utils.temporary_file', TemporaryFileMock()).start()
        mock.patch(
            'git_cl.Changelist.GetCodereviewServer',
            return_value='https://chromium-review.googlesource.com').start()
        mock.patch('git_cl.Changelist.GetAuthor', return_value='author').start()
        mock.patch('git_cl.Changelist.GetIssue', return_value=123456).start()
        mock.patch('git_cl.Changelist.GetPatchset', return_value=7).start()
        mock.patch('git_cl.Changelist.GetRemoteBranch',
                   return_value=('origin', 'refs/remotes/origin/main')).start()
        mock.patch('git_cl.PRESUBMIT_SUPPORT', 'PRESUBMIT_SUPPORT').start()
        mock.patch('git_cl.Settings.GetRoot', return_value='root').start()
        mock.patch('git_cl.Settings.GetIsGerrit', return_value=True).start()
        mock.patch('git_cl.time_time').start()
        mock.patch('metrics.collector').start()
        mock.patch('subprocess2.Popen').start()
        mock.patch('git_cl.Changelist.GetGerritProject',
                   return_value='project').start()
        mock.patch('sys.exit', side_effect=SystemExitMock).start()

        scm_mock.GIT(self)

        self.addCleanup(mock.patch.stopall)
        self.temp_count = 0
        gerrit_util._Authenticator._resolved = None

    def testRunHook(self):
        expected_results = {
            'more_cc': ['cc@example.com', 'more@example.com'],
            'errors': [],
            'notifications': [],
            'warnings': [],
        }
        gclient_utils.FileRead.return_value = json.dumps(expected_results)
        git_cl.time_time.side_effect = [100, 200, 300, 400]
        mockProcess = mock.Mock()
        mockProcess.wait.return_value = 0
        subprocess2.Popen.return_value = mockProcess

        cl = git_cl.Changelist()
        results = cl.RunHook(committing=True,
                             may_prompt=True,
                             verbose=2,
                             parallel=True,
                             upstream='upstream',
                             description='description',
                             all_files=True,
                             resultdb=False)

        self.assertEqual(expected_results, results)
        subprocess2.Popen.assert_any_call([
            'vpython3',
            'PRESUBMIT_SUPPORT',
            '--root',
            'root',
            '--upstream',
            'upstream',
            '--verbose',
            '--verbose',
            '--gerrit_url',
            'https://chromium-review.googlesource.com',
            '--gerrit_project',
            'project',
            '--gerrit_branch',
            'refs/heads/main',
            '--author',
            'author',
            '--issue',
            '123456',
            '--patchset',
            '7',
            '--commit',
            '--may_prompt',
            '--parallel',
            '--all_files',
            '--no_diffs',
            '--json_output',
            '/tmp/fake-temp2',
            '--description_file',
            '/tmp/fake-temp1',
        ])
        gclient_utils.FileWrite.assert_any_call('/tmp/fake-temp1',
                                                'description')
        metrics.collector.add_repeated('sub_commands', {
            'command': 'presubmit',
            'execution_time': 100,
            'exit_code': 0,
        })

    def testRunHook_FewerOptions(self):
        expected_results = {
            'more_cc': ['cc@example.com', 'more@example.com'],
            'errors': [],
            'notifications': [],
            'warnings': [],
        }
        gclient_utils.FileRead.return_value = json.dumps(expected_results)
        git_cl.time_time.side_effect = [100, 200, 300, 400]
        mockProcess = mock.Mock()
        mockProcess.wait.return_value = 0
        subprocess2.Popen.return_value = mockProcess

        git_cl.Changelist.GetAuthor.return_value = None
        git_cl.Changelist.GetIssue.return_value = None
        git_cl.Changelist.GetPatchset.return_value = None

        cl = git_cl.Changelist()
        results = cl.RunHook(committing=False,
                             may_prompt=False,
                             verbose=0,
                             parallel=False,
                             upstream='upstream',
                             description='description',
                             all_files=False,
                             resultdb=False)

        self.assertEqual(expected_results, results)
        subprocess2.Popen.assert_any_call([
            'vpython3',
            'PRESUBMIT_SUPPORT',
            '--root',
            'root',
            '--upstream',
            'upstream',
            '--gerrit_url',
            'https://chromium-review.googlesource.com',
            '--gerrit_project',
            'project',
            '--gerrit_branch',
            'refs/heads/main',
            '--upload',
            '--json_output',
            '/tmp/fake-temp2',
            '--description_file',
            '/tmp/fake-temp1',
        ])
        gclient_utils.FileWrite.assert_any_call('/tmp/fake-temp1',
                                                'description')
        metrics.collector.add_repeated('sub_commands', {
            'command': 'presubmit',
            'execution_time': 100,
            'exit_code': 0,
        })

    def testRunHook_FewerOptionsResultDB(self):
        expected_results = {
            'more_cc': ['cc@example.com', 'more@example.com'],
            'errors': [],
            'notifications': [],
            'warnings': [],
        }
        gclient_utils.FileRead.return_value = json.dumps(expected_results)
        git_cl.time_time.side_effect = [100, 200, 300, 400]
        mockProcess = mock.Mock()
        mockProcess.wait.return_value = 0
        subprocess2.Popen.return_value = mockProcess

        git_cl.Changelist.GetAuthor.return_value = None
        git_cl.Changelist.GetIssue.return_value = None
        git_cl.Changelist.GetPatchset.return_value = None

        cl = git_cl.Changelist()
        results = cl.RunHook(committing=False,
                             may_prompt=False,
                             verbose=0,
                             parallel=False,
                             upstream='upstream',
                             description='description',
                             all_files=False,
                             resultdb=True,
                             realm='chromium:public')

        self.assertEqual(expected_results, results)
        subprocess2.Popen.assert_any_call([
            'rdb',
            'stream',
            '-new',
            '-realm',
            'chromium:public',
            '--',
            'vpython3',
            'PRESUBMIT_SUPPORT',
            '--root',
            'root',
            '--upstream',
            'upstream',
            '--gerrit_url',
            'https://chromium-review.googlesource.com',
            '--gerrit_project',
            'project',
            '--gerrit_branch',
            'refs/heads/main',
            '--upload',
            '--json_output',
            '/tmp/fake-temp2',
            '--description_file',
            '/tmp/fake-temp1',
        ])

    def testRunHook_NoGerrit(self):
        mock.patch('git_cl.Settings.GetIsGerrit', return_value=False).start()

        expected_results = {
            'more_cc': ['cc@example.com', 'more@example.com'],
            'errors': [],
            'notifications': [],
            'warnings': [],
        }
        gclient_utils.FileRead.return_value = json.dumps(expected_results)
        git_cl.time_time.side_effect = [100, 200, 300, 400]
        mockProcess = mock.Mock()
        mockProcess.wait.return_value = 0
        subprocess2.Popen.return_value = mockProcess

        git_cl.Changelist.GetAuthor.return_value = None
        git_cl.Changelist.GetIssue.return_value = None
        git_cl.Changelist.GetPatchset.return_value = None

        cl = git_cl.Changelist()
        results = cl.RunHook(committing=False,
                             may_prompt=False,
                             verbose=0,
                             parallel=False,
                             upstream='upstream',
                             description='description',
                             all_files=False,
                             resultdb=False)

        self.assertEqual(expected_results, results)
        subprocess2.Popen.assert_any_call([
            'vpython3',
            'PRESUBMIT_SUPPORT',
            '--root',
            'root',
            '--upstream',
            'upstream',
            '--upload',
            '--json_output',
            '/tmp/fake-temp2',
            '--description_file',
            '/tmp/fake-temp1',
        ])
        gclient_utils.FileWrite.assert_any_call('/tmp/fake-temp1',
                                                'description')
        metrics.collector.add_repeated('sub_commands', {
            'command': 'presubmit',
            'execution_time': 100,
            'exit_code': 0,
        })

    @mock.patch('sys.exit', side_effect=SystemExitMock)
    def testRunHook_Failure(self, _mock):
        git_cl.time_time.side_effect = [100, 200]
        mockProcess = mock.Mock()
        mockProcess.wait.return_value = 2
        subprocess2.Popen.return_value = mockProcess

        cl = git_cl.Changelist()
        with self.assertRaises(SystemExitMock):
            cl.RunHook(committing=True,
                       may_prompt=True,
                       verbose=2,
                       parallel=True,
                       upstream='upstream',
                       description='description',
                       all_files=True,
                       resultdb=False)

        sys.exit.assert_called_once_with(2)

    def testRunPostUploadHook(self):
        cl = git_cl.Changelist()
        cl.RunPostUploadHook(2, 'upstream', 'description')

        subprocess2.Popen.assert_called_with([
            'vpython3',
            'PRESUBMIT_SUPPORT',
            '--root',
            'root',
            '--upstream',
            'upstream',
            '--verbose',
            '--verbose',
            '--gerrit_url',
            'https://chromium-review.googlesource.com',
            '--gerrit_project',
            'project',
            '--gerrit_branch',
            'refs/heads/main',
            '--author',
            'author',
            '--issue',
            '123456',
            '--patchset',
            '7',
            '--post_upload',
            '--description_file',
            '/tmp/fake-temp1',
        ])

        gclient_utils.FileWrite.assert_called_once_with('/tmp/fake-temp1',
                                                        'description')

    def testRunPostUploadHookPy3Only(self):
        cl = git_cl.Changelist()
        cl.RunPostUploadHook(2, 'upstream', 'description')

        subprocess2.Popen.assert_called_once_with([
            'vpython3',
            'PRESUBMIT_SUPPORT',
            '--root',
            'root',
            '--upstream',
            'upstream',
            '--verbose',
            '--verbose',
            '--gerrit_url',
            'https://chromium-review.googlesource.com',
            '--gerrit_project',
            'project',
            '--gerrit_branch',
            'refs/heads/main',
            '--author',
            'author',
            '--issue',
            '123456',
            '--patchset',
            '7',
            '--post_upload',
            '--description_file',
            '/tmp/fake-temp1',
        ])

        gclient_utils.FileWrite.assert_called_once_with('/tmp/fake-temp1',
                                                        'description')

    @mock.patch('git_cl.RunGit', _mock_run_git)
    def testDefaultTitleEmptyMessage(self):
        cl = git_cl.Changelist()
        cl.issue = 100
        options = optparse.Values({
            'squash': True,
            'title': None,
            'message': None,
            'force': None,
            'skip_title': None
        })

        mock.patch('gclient_utils.AskForData', lambda _: user_title).start()
        for user_title in ['', 'y', 'Y']:
            self.assertEqual(cl._GetTitleForUpload(options),
                             self.LAST_COMMIT_SUBJECT)

        for user_title in ['not empty', 'yes', 'YES']:
            self.assertEqual(cl._GetTitleForUpload(options), user_title)

    @mock.patch('git_cl.Changelist.GetMostRecentPatchset', return_value=2)
    @mock.patch('git_cl.RunGit')
    @mock.patch('git_cl.Changelist._PrepareChange')
    def testPrepareSquashedCommit(self, mockPrepareChange, mockRunGit, *_mocks):

        change_desc = git_cl.ChangeDescription('BOO!')
        reviewers = []
        ccs = []
        mockPrepareChange.return_value = (reviewers, ccs, change_desc)

        parent_hash = 'upstream-gerrit-hash'
        parent_orig_hash = 'upstream-last-upload-hash'
        parent_hash_root = 'root-commit'
        hash_to_push = 'new-squash-hash'
        hash_to_push_root = 'new-squash-hash-root'
        branchref = 'refs/heads/current-branch'
        end_hash = 'end-hash'
        tree_hash = 'tree-hash'

        def mock_run_git(commands):
            if {'commit-tree', tree_hash, '-p', parent_hash,
                    '-F'}.issubset(set(commands)):
                return hash_to_push
            if {'commit-tree', tree_hash, '-p', parent_hash_root,
                    '-F'}.issubset(set(commands)):
                return hash_to_push_root
            if commands == ['rev-parse', branchref]:
                return end_hash
            if commands == ['rev-parse', end_hash + ':']:
                return tree_hash

        mockRunGit.side_effect = mock_run_git
        cl = git_cl.Changelist(branchref=branchref)
        options = optparse.Values()

        new_upload = cl.PrepareSquashedCommit(options, parent_hash,
                                              parent_orig_hash)
        self.assertEqual(new_upload.reviewers, reviewers)
        self.assertEqual(new_upload.ccs, ccs)
        self.assertEqual(new_upload.commit_to_push, hash_to_push)
        self.assertEqual(new_upload.new_last_uploaded_commit, end_hash)
        self.assertEqual(new_upload.change_desc, change_desc)
        mockPrepareChange.assert_called_with(options, parent_orig_hash,
                                             end_hash)

    @mock.patch('git_cl.Settings.GetRoot', return_value='')
    @mock.patch('git_cl.Changelist.GetMostRecentPatchset', return_value=2)
    @mock.patch('git_cl.RunGitWithCode')
    @mock.patch('git_cl.RunGit')
    @mock.patch('git_cl.Changelist._PrepareChange')
    @mock.patch('git_cl.Changelist.GetCommonAncestorWithUpstream')
    def testPrepareCherryPickSquashedCommit(self,
                                            mockGetCommonAncestorWithUpstream,
                                            mockPrepareChange, mockRunGit,
                                            mockRunGitWithCode, *_mocks):
        cherry_pick_base_hash = '1a2bcherrypickbase'
        mockGetCommonAncestorWithUpstream.return_value = cherry_pick_base_hash

        change_desc = git_cl.ChangeDescription('BOO!')
        ccs = ['cc@review.cl']
        reviewers = ['reviewer@review.cl']
        mockPrepareChange.return_value = (reviewers, ccs, change_desc)

        branchref = 'refs/heads/current-branch'
        cl = git_cl.Changelist(branchref=branchref)
        options = optparse.Values()

        upstream_gerrit_hash = 'upstream-gerrit-hash'

        latest_tree_hash = 'tree-hash'
        hash_to_cp = 'squashed-hash'
        hash_to_push = 'hash-to-push'
        hash_to_save_as_last_upload = 'last-upload'

        def mock_run_git(commands):
            if commands == ['rev-parse', branchref]:
                return hash_to_save_as_last_upload
            if commands == ['rev-parse', branchref + ':']:
                return latest_tree_hash
            if {
                    'commit-tree', latest_tree_hash, '-p',
                    cherry_pick_base_hash, '-F'
            }.issubset(set(commands)):
                return hash_to_cp
            if commands == ['rev-parse', 'HEAD']:
                return hash_to_push

        mockRunGit.side_effect = mock_run_git

        def mock_run_git_with_code(commands):
            if commands == ['cherry-pick', hash_to_cp]:
                return 0, ''

        mockRunGitWithCode.side_effect = mock_run_git_with_code

        new_upload = cl.PrepareCherryPickSquashedCommit(options,
                                                        upstream_gerrit_hash)
        self.assertEqual(new_upload.reviewers, reviewers)
        self.assertEqual(new_upload.ccs, ccs)
        self.assertEqual(new_upload.commit_to_push, hash_to_push)
        self.assertEqual(new_upload.new_last_uploaded_commit,
                         hash_to_save_as_last_upload)
        self.assertEqual(new_upload.change_desc, change_desc)

        # Test failed cherry-pick

        def mock_run_git_with_code(commands):
            if commands == ['cherry-pick', hash_to_cp]:
                return 1, ''

        mockRunGitWithCode.side_effect = mock_run_git_with_code

        with self.assertRaises(SystemExitMock):
            cl.PrepareCherryPickSquashedCommit(options, cherry_pick_base_hash)

    @mock.patch('git_cl.Settings.GetDefaultCCList', return_value=[])
    @mock.patch('git_cl.Changelist.GetAffectedFiles', return_value=[])
    @mock.patch('git_cl.GenerateGerritChangeId', return_value='1a2b3c')
    @mock.patch('git_cl.Changelist.GetIssue', return_value=None)
    @mock.patch('git_cl.ChangeDescription.prompt')
    @mock.patch('git_cl.Changelist.RunHook')
    @mock.patch('git_cl.Changelist._GetDescriptionForUpload')
    @mock.patch('git_cl.Changelist.EnsureCanUploadPatchset')
    def testPrepareChange_new(self, mockEnsureCanUploadPatchset,
                              mockGetDescriptionForupload, mockRunHook,
                              mockPrompt, *_mocks):
        options = optparse.Values()

        options.force = False
        options.bypass_hooks = False
        options.verbose = False
        options.parallel = False
        options.preserve_tryjobs = False
        options.private = False
        options.no_autocc = False
        options.message_file = None
        options.commit_description = None
        options.cc = ['chicken@bok.farm']
        parent = '420parent'
        latest_tree = '420latest_tree'

        mockRunHook.return_value = {'more_cc': ['cow@moo.farm']}
        desc = 'AH!\nCC=cow2@moo.farm\nR=horse@apple.farm'
        mockGetDescriptionForupload.return_value = git_cl.ChangeDescription(
            desc)

        cl = git_cl.Changelist()
        reviewers, ccs, change_desc = cl._PrepareChange(options, parent,
                                                        latest_tree)
        self.assertEqual(reviewers, ['horse@apple.farm'])
        self.assertEqual(ccs,
                         ['cow@moo.farm', 'chicken@bok.farm', 'cow2@moo.farm'])
        self.assertEqual(change_desc._description_lines, [
            'AH!', 'CC=cow2@moo.farm', 'R=horse@apple.farm', '',
            'Change-Id: 1a2b3c'
        ])
        mockPrompt.assert_called_once()
        mockEnsureCanUploadPatchset.assert_called_once()
        mockRunHook.assert_called_once_with(committing=False,
                                            may_prompt=True,
                                            verbose=False,
                                            parallel=False,
                                            upstream='420parent',
                                            description=desc,
                                            all_files=False,
                                            end_commit='420latest_tree')

    @mock.patch('git_cl.Changelist.GetAffectedFiles', return_value=[])
    @mock.patch('git_cl.Changelist.GetIssue', return_value='123')
    @mock.patch('git_cl.ChangeDescription.prompt')
    @mock.patch('gerrit_util.GetChangeDetail')
    @mock.patch('git_cl.Changelist.RunHook')
    @mock.patch('git_cl.Changelist._GetDescriptionForUpload')
    @mock.patch('git_cl.Changelist.EnsureCanUploadPatchset')
    def testPrepareChange_existing(self, mockEnsureCanUploadPatchset,
                                   mockGetDescriptionForupload, mockRunHook,
                                   mockGetChangeDetail, mockPrompt, *_mocks):
        cl = git_cl.Changelist()
        options = optparse.Values()

        options.force = False
        options.bypass_hooks = False
        options.verbose = False
        options.parallel = False
        options.edit_description = False
        options.preserve_tryjobs = False
        options.private = False
        options.no_autocc = False
        options.cc = ['chicken@bok.farm']
        parent = '420parent'
        latest_tree = '420latest_tree'

        mockRunHook.return_value = {'more_cc': ['cow@moo.farm']}
        desc = 'AH!\nCC=cow2@moo.farm\nR=horse@apple.farm'
        mockGetDescriptionForupload.return_value = git_cl.ChangeDescription(
            desc)

        # Existing change
        gerrit_util.GetChangeDetail.return_value = {
            'change_id': ('123456789'),
            'current_revision': 'sha1_of_current_revision',
        }

        reviewers, ccs, change_desc = cl._PrepareChange(options, parent,
                                                        latest_tree)
        self.assertEqual(reviewers, ['horse@apple.farm'])
        self.assertEqual(ccs, ['chicken@bok.farm', 'cow2@moo.farm'])
        self.assertEqual(change_desc._description_lines, [
            'AH!', 'CC=cow2@moo.farm', 'R=horse@apple.farm', '',
            'Change-Id: 123456789'
        ])
        mockRunHook.assert_called_once_with(committing=False,
                                            may_prompt=True,
                                            verbose=False,
                                            parallel=False,
                                            upstream=parent,
                                            description=desc,
                                            all_files=False,
                                            end_commit=latest_tree)
        mockEnsureCanUploadPatchset.assert_called_once()

        # Test preserve_tryjob
        options.preserve_tryjobs = True
        # Test edit_description
        options.edit_description = True
        # Test private
        options.private = True
        options.no_autocc = True

        reviewers, ccs, change_desc = cl._PrepareChange(options, parent,
                                                        latest_tree)
        self.assertEqual(ccs, ['chicken@bok.farm', 'cow2@moo.farm'])
        mockPrompt.assert_called_once()
        self.assertEqual(change_desc._description_lines, [
            'AH!', 'CC=cow2@moo.farm', 'R=horse@apple.farm', '',
            'Change-Id: 123456789', 'Cq-Do-Not-Cancel-Tryjobs: true'
        ])

    @mock.patch('git_cl.Changelist.GetGerritHost', return_value='chromium')
    @mock.patch('git_cl.Settings.GetRunPostUploadHook', return_value=True)
    @mock.patch('git_cl.Changelist.SetPatchset')
    @mock.patch('git_cl.Changelist.RunPostUploadHook')
    @mock.patch('git_cl.gerrit_util.AddReviewers')
    def testPostUploadUpdates(self, mockAddReviewers, mockRunPostHook,
                              mockSetPatchset, *_mocks):

        cl = git_cl.Changelist(branchref='refs/heads/current-branch')
        options = optparse.Values()
        options.verbose = True
        options.no_python2_post_upload_hooks = True
        options.send_mail = False

        reviewers = ['monkey@vp.circus']
        ccs = ['cow@rds.corp']
        change_desc = git_cl.ChangeDescription('[stonks] honk honk')
        new_upload = git_cl._NewUpload(reviewers, ccs, 'pushed-commit',
                                       'last-uploaded-commit', 'parent-commit',
                                       change_desc, 2)

        cl.PostUploadUpdates(options, new_upload, '12345')
        mockSetPatchset.assert_called_once_with(3)
        self.assertEqual(
            scm.GIT.GetConfig('root', 'branch.current-branch.gerritsquashhash'),
            new_upload.commit_to_push)
        self.assertEqual(
            scm.GIT.GetConfig('root', 'branch.current-branch.last-upload-hash'),
            new_upload.new_last_uploaded_commit)

        mockAddReviewers.assert_called_once_with('chromium',
                                                 'project~123456',
                                                 reviewers=reviewers,
                                                 ccs=ccs,
                                                 notify=False)
        mockRunPostHook.assert_called_once_with(True, 'parent-commit',
                                                change_desc.description)


class CMDTestCaseBase(unittest.TestCase):
    _STATUSES = [
        'STATUS_UNSPECIFIED',
        'SCHEDULED',
        'STARTED',
        'SUCCESS',
        'FAILURE',
        'INFRA_FAILURE',
        'CANCELED',
    ]
    _CHANGE_DETAIL = {
        'project': 'depot_tools',
        'status': 'OPEN',
        'owner': {
            'email': 'owner@e.mail'
        },
        'current_revision': 'beeeeeef',
        'revisions': {
            'deadbeaf': {
                '_number': 6,
                'kind': 'REWORK',
            },
            'beeeeeef': {
                '_number': 7,
                'kind': 'NO_CODE_CHANGE',
                'fetch': {
                    'http': {
                        'url': 'https://chromium.googlesource.com/depot_tools',
                        'ref': 'refs/changes/56/123456/7'
                    }
                },
            },
        },
    }
    _DEFAULT_RESPONSE = {
        'builds': [{
            'id': str(100 + idx),
            'builder': {
                'project': 'chromium',
                'bucket': 'try',
                'builder': 'bot_' + status.lower(),
            },
            'createTime': '2019-10-09T08:00:0%d.854286Z' % (idx % 10),
            'tags': [],
            'status': status,
        } for idx, status in enumerate(_STATUSES)]
    }

    def setUp(self):
        super(CMDTestCaseBase, self).setUp()
        mock.patch('git_cl.sys.stdout', io.StringIO()).start()
        mock.patch('git_cl.uuid.uuid4', return_value='uuid4').start()
        mock.patch('git_cl.Changelist.GetIssue', return_value=123456).start()
        mock.patch(
            'git_cl.Changelist.GetCodereviewServer',
            return_value='https://chromium-review.googlesource.com').start()
        mock.patch('git_cl.Changelist.GetGerritHost',
                   return_value='chromium-review.googlesource.com').start()
        mock.patch('git_cl.Changelist.GetMostRecentPatchset',
                   return_value=7).start()
        mock.patch('git_cl.Changelist.GetMostRecentDryRunPatchset',
                   return_value=6).start()
        mock.patch('git_cl.Changelist.GetRemoteUrl',
                   return_value='https://chromium.googlesource.com/depot_tools'
                   ).start()
        mock.patch('auth.Authenticator',
                   return_value=AuthenticatorMock()).start()
        mock.patch('gerrit_util.GetChangeDetail',
                   return_value=self._CHANGE_DETAIL).start()
        mock.patch('git_cl._call_buildbucket',
                   return_value=self._DEFAULT_RESPONSE).start()
        mock.patch('git_common.is_dirty_git_tree', return_value=False).start()
        self.addCleanup(mock.patch.stopall)


@unittest.skipIf(gclient_utils.IsEnvCog(),
                'not supported in non-git environment')
class CMDPresubmitTestCase(CMDTestCaseBase):
    _RUN_HOOK_RETURN = {
        'errors': [],
        'more_cc': [],
        'notifications': [],
        'warnings': []
    }

    def setUp(self):
        super(CMDPresubmitTestCase, self).setUp()
        mock.patch('git_cl.Changelist.GetCommonAncestorWithUpstream',
                   return_value='upstream').start()
        mock.patch('git_cl.Changelist.FetchDescription',
                   return_value='fetch description').start()
        mock.patch('git_cl._create_description_from_log',
                   return_value='get description').start()
        mock.patch('git_cl.Changelist.RunHook',
                   return_value=self._RUN_HOOK_RETURN).start()

    def testDefaultCase(self):
        self.assertEqual(0, git_cl.main(['presubmit']))
        git_cl.Changelist.RunHook.assert_called_once_with(
            committing=True,
            may_prompt=False,
            verbose=0,
            parallel=None,
            upstream='upstream',
            description='fetch description',
            all_files=None,
            files=None,
            resultdb=None,
            realm=None)

    def testNoIssue(self):
        git_cl.Changelist.GetIssue.return_value = None
        self.assertEqual(0, git_cl.main(['presubmit']))
        git_cl.Changelist.RunHook.assert_called_once_with(
            committing=True,
            may_prompt=False,
            verbose=0,
            parallel=None,
            upstream='upstream',
            description='get description',
            all_files=None,
            files=None,
            resultdb=None,
            realm=None)

    def testCustomBranch(self):
        self.assertEqual(0, git_cl.main(['presubmit', 'custom_branch']))
        git_cl.Changelist.RunHook.assert_called_once_with(
            committing=True,
            may_prompt=False,
            verbose=0,
            parallel=None,
            upstream='custom_branch',
            description='fetch description',
            all_files=None,
            files=None,
            resultdb=None,
            realm=None)

    def testOptions(self):
        self.assertEqual(
            0,
            git_cl.main([
                'presubmit', '-v', '-v', '--all', '--parallel', '-u',
                '--resultdb', '--realm', 'chromium:public'
            ]))
        git_cl.Changelist.RunHook.assert_called_once_with(
            committing=False,
            may_prompt=False,
            verbose=2,
            parallel=True,
            upstream='upstream',
            description='fetch description',
            all_files=True,
            files=None,
            resultdb=True,
            realm='chromium:public')

    @mock.patch('git_cl.write_json')
    def testJson(self, mock_write_json):
        self.assertEqual(0, git_cl.main(['presubmit', '--json', 'file.json']))
        mock_write_json.assert_called_once_with('file.json',
                                                self._RUN_HOOK_RETURN)


class CMDTryResultsTestCase(CMDTestCaseBase):
    _DEFAULT_REQUEST = {
        'predicate': {
            "gerritChanges": [{
                "project": "depot_tools",
                "host": "chromium-review.googlesource.com",
                "patchset": 6,
                "change": 123456,
            }],
        },
        'fields': ('builds.*.id,builds.*.builder,builds.*.status' +
                   ',builds.*.createTime,builds.*.tags'),
    }

    _TRIVIAL_REQUEST = {
        'predicate': {
            "gerritChanges": [{
                "project": "depot_tools",
                "host": "chromium-review.googlesource.com",
                "patchset": 7,
                "change": 123456,
            }],
        },
        'fields': ('builds.*.id,builds.*.builder,builds.*.status' +
                   ',builds.*.createTime,builds.*.tags'),
    }

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def testNoJobs(self):
        git_cl._call_buildbucket.return_value = {}

        self.assertEqual(0, git_cl.main(['try-results']))
        self.assertEqual('No tryjobs scheduled.\n', sys.stdout.getvalue())
        git_cl._call_buildbucket.assert_called_once_with(
            mock.ANY, 'cr-buildbucket.appspot.com', 'SearchBuilds',
            self._DEFAULT_REQUEST)

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def testTrivialCommits(self):
        self.assertEqual(0, git_cl.main(['try-results']))
        git_cl._call_buildbucket.assert_called_with(
            mock.ANY, 'cr-buildbucket.appspot.com', 'SearchBuilds',
            self._DEFAULT_REQUEST)

        git_cl._call_buildbucket.return_value = {}
        self.assertEqual(0, git_cl.main(['try-results', '--patchset', '7']))
        git_cl._call_buildbucket.assert_called_with(
            mock.ANY, 'cr-buildbucket.appspot.com', 'SearchBuilds',
            self._TRIVIAL_REQUEST)
        self.assertEqual([
            'Successes:',
            '  bot_success            https://ci.chromium.org/b/103',
            'Infra Failures:',
            '  bot_infra_failure      https://ci.chromium.org/b/105',
            'Failures:',
            '  bot_failure            https://ci.chromium.org/b/104',
            'Canceled:',
            '  bot_canceled          ',
            'Started:',
            '  bot_started            https://ci.chromium.org/b/102',
            'Scheduled:',
            '  bot_scheduled          id=101',
            'Other:',
            '  bot_status_unspecified id=100',
            'Total: 7 tryjobs',
            'No tryjobs scheduled.',
        ],
                         sys.stdout.getvalue().splitlines())

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def testPrintToStdout(self):
        self.assertEqual(0, git_cl.main(['try-results']))
        self.assertEqual([
            'Successes:',
            '  bot_success            https://ci.chromium.org/b/103',
            'Infra Failures:',
            '  bot_infra_failure      https://ci.chromium.org/b/105',
            'Failures:',
            '  bot_failure            https://ci.chromium.org/b/104',
            'Canceled:',
            '  bot_canceled          ',
            'Started:',
            '  bot_started            https://ci.chromium.org/b/102',
            'Scheduled:',
            '  bot_scheduled          id=101',
            'Other:',
            '  bot_status_unspecified id=100',
            'Total: 7 tryjobs',
        ],
                         sys.stdout.getvalue().splitlines())
        git_cl._call_buildbucket.assert_called_once_with(
            mock.ANY, 'cr-buildbucket.appspot.com', 'SearchBuilds',
            self._DEFAULT_REQUEST)

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    def testPrintToStdoutWithMasters(self):
        self.assertEqual(0, git_cl.main(['try-results', '--print-master']))
        self.assertEqual([
            'Successes:',
            '  try bot_success            https://ci.chromium.org/b/103',
            'Infra Failures:',
            '  try bot_infra_failure      https://ci.chromium.org/b/105',
            'Failures:',
            '  try bot_failure            https://ci.chromium.org/b/104',
            'Canceled:',
            '  try bot_canceled          ',
            'Started:',
            '  try bot_started            https://ci.chromium.org/b/102',
            'Scheduled:',
            '  try bot_scheduled          id=101',
            'Other:',
            '  try bot_status_unspecified id=100',
            'Total: 7 tryjobs',
        ],
                         sys.stdout.getvalue().splitlines())
        git_cl._call_buildbucket.assert_called_once_with(
            mock.ANY, 'cr-buildbucket.appspot.com', 'SearchBuilds',
            self._DEFAULT_REQUEST)

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    @mock.patch('git_cl.write_json')
    def testWriteToJson(self, mockJsonDump):
        self.assertEqual(0, git_cl.main(['try-results', '--json', 'file.json']))
        git_cl._call_buildbucket.assert_called_once_with(
            mock.ANY, 'cr-buildbucket.appspot.com', 'SearchBuilds',
            self._DEFAULT_REQUEST)
        mockJsonDump.assert_called_once_with('file.json',
                                             self._DEFAULT_RESPONSE['builds'])

    def test_filter_failed_for_one_simple(self):
        self.assertEqual([], git_cl._filter_failed_for_retry([]))
        self.assertEqual([
            ('chromium', 'try', 'bot_failure'),
            ('chromium', 'try', 'bot_infra_failure'),
        ], git_cl._filter_failed_for_retry(self._DEFAULT_RESPONSE['builds']))

    def test_filter_failed_for_retry_many_builds(self):
        def _build(name, created_sec, status, experimental=False):
            assert 0 <= created_sec < 100, created_sec
            b = {
                'id': 112112,
                'builder': {
                    'project': 'chromium',
                    'bucket': 'try',
                    'builder': name,
                },
                'createTime': '2019-10-09T08:00:%02d.854286Z' % created_sec,
                'status': status,
                'tags': [],
            }
            if experimental:
                b['tags'].append({'key': 'cq_experimental', 'value': 'true'})
            return b

        builds = [
            _build('flaky-last-green', 1, 'FAILURE'),
            _build('flaky-last-green', 2, 'SUCCESS'),
            _build('flaky', 1, 'SUCCESS'),
            _build('flaky', 2, 'FAILURE'),
            _build('running', 1, 'FAILED'),
            _build('running', 2, 'SCHEDULED'),
            _build('yep-still-running', 1, 'STARTED'),
            _build('yep-still-running', 2, 'FAILURE'),
            _build('cq-experimental', 1, 'SUCCESS', experimental=True),
            _build('cq-experimental', 2, 'FAILURE', experimental=True),

            # Simulate experimental in CQ builder, which developer decided
            # to retry manually which resulted in 2nd build non-experimental.
            _build('sometimes-experimental', 1, 'FAILURE', experimental=True),
            _build('sometimes-experimental', 2, 'FAILURE', experimental=False),
        ]
        builds.sort(key=lambda b: b['status'])  # ~deterministic shuffle.
        self.assertEqual([
            ('chromium', 'try', 'flaky'),
            ('chromium', 'try', 'sometimes-experimental'),
        ], git_cl._filter_failed_for_retry(builds))


class CMDTryTestCase(CMDTestCaseBase):
    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    @mock.patch('git_cl.Changelist.SetCQState')
    def testSetCQDryRunByDefault(self, mockSetCQState):
        mockSetCQState.return_value = 0
        self.assertEqual(0, git_cl.main(['try']))
        git_cl.Changelist.SetCQState.assert_called_with(git_cl._CQState.DRY_RUN)
        self.assertEqual(
            sys.stdout.getvalue(), 'Scheduling CQ dry run on: '
            'https://chromium-review.googlesource.com/123456\n')

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    @mock.patch('git_cl._call_buildbucket')
    def testScheduleOnBuildbucket(self, mockCallBuildbucket):
        mockCallBuildbucket.return_value = {}

        self.assertEqual(
            0,
            git_cl.main([
                'try', '-B', 'luci.chromium.try', '-b', 'win', '-p', 'key=val',
                '-p', 'json=[{"a":1}, null]'
            ]))
        self.assertIn('Scheduling jobs on:\n'
                      '  chromium/try: win', git_cl.sys.stdout.getvalue())

        expected_request = {
            "requests": [{
                "scheduleBuild": {
                    "requestId":
                    "uuid4",
                    "builder": {
                        "project": "chromium",
                        "builder": "win",
                        "bucket": "try",
                    },
                    "gerritChanges": [{
                        "project": "depot_tools",
                        "host": "chromium-review.googlesource.com",
                        "patchset": 7,
                        "change": 123456,
                    }],
                    "properties": {
                        "category": "git_cl_try",
                        "json": [{
                            "a": 1
                        }, None],
                        "key": "val",
                    },
                    "tags": [
                        {
                            "value": "win",
                            "key": "builder"
                        },
                        {
                            "value": "git_cl_try",
                            "key": "user_agent"
                        },
                    ],
                },
            }],
        }
        mockCallBuildbucket.assert_called_with(mock.ANY,
                                               'cr-buildbucket.appspot.com',
                                               'Batch', expected_request)

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    @mock.patch('git_cl._call_buildbucket')
    def testScheduleOnBuildbucketWithRevision(self, mockCallBuildbucket):
        mockCallBuildbucket.return_value = {}
        mock.patch('git_cl.Changelist.GetRemoteBranch',
                   return_value=('origin', 'refs/remotes/origin/main')).start()

        self.assertEqual(
            0,
            git_cl.main([
                'try', '-B', 'luci.chromium.try', '-b', 'win', '-b', 'linux',
                '-p', 'key=val', '-p', 'json=[{"a":1}, null]', '-r',
                'beeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeef'
            ]))
        self.assertIn(
            'Scheduling jobs on:\n'
            '  chromium/try: linux\n'
            '  chromium/try: win', git_cl.sys.stdout.getvalue())

        expected_request = {
            "requests": [{
                "scheduleBuild": {
                    "requestId":
                    "uuid4",
                    "builder": {
                        "project": "chromium",
                        "builder": "linux",
                        "bucket": "try",
                    },
                    "gerritChanges": [{
                        "project": "depot_tools",
                        "host": "chromium-review.googlesource.com",
                        "patchset": 7,
                        "change": 123456,
                    }],
                    "properties": {
                        "category": "git_cl_try",
                        "json": [{
                            "a": 1
                        }, None],
                        "key": "val",
                    },
                    "tags": [
                        {
                            "value": "linux",
                            "key": "builder"
                        },
                        {
                            "value": "git_cl_try",
                            "key": "user_agent"
                        },
                    ],
                    "gitilesCommit": {
                        "host": "chromium.googlesource.com",
                        "project": "depot_tools",
                        "id": "beeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeef",
                        "ref": "refs/heads/main",
                    }
                },
            }, {
                "scheduleBuild": {
                    "requestId":
                    "uuid4",
                    "builder": {
                        "project": "chromium",
                        "builder": "win",
                        "bucket": "try",
                    },
                    "gerritChanges": [{
                        "project": "depot_tools",
                        "host": "chromium-review.googlesource.com",
                        "patchset": 7,
                        "change": 123456,
                    }],
                    "properties": {
                        "category": "git_cl_try",
                        "json": [{
                            "a": 1
                        }, None],
                        "key": "val",
                    },
                    "tags": [
                        {
                            "value": "win",
                            "key": "builder"
                        },
                        {
                            "value": "git_cl_try",
                            "key": "user_agent"
                        },
                    ],
                    "gitilesCommit": {
                        "host": "chromium.googlesource.com",
                        "project": "depot_tools",
                        "id": "beeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeef",
                        "ref": "refs/heads/main",
                    }
                },
            }],
        }
        mockCallBuildbucket.assert_called_with(mock.ANY,
                                               'cr-buildbucket.appspot.com',
                                               'Batch', expected_request)

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    @mock.patch('sys.stderr', io.StringIO())
    def testScheduleOnBuildbucket_WrongBucket(self):
        with self.assertRaises(SystemExit):
            git_cl.main([
                'try', '-B', 'not-a-bucket', '-b', 'win', '-p', 'key=val', '-p',
                'json=[{"a":1}, null]'
            ])
        self.assertIn('Invalid bucket: not-a-bucket.', sys.stderr.getvalue())

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    @mock.patch('git_cl._call_buildbucket')
    @mock.patch('git_cl._fetch_tryjobs')
    def testScheduleOnBuildbucketRetryFailed(self, mockFetchTryJobs,
                                             mockCallBuildbucket):
        git_cl._fetch_tryjobs.side_effect = lambda *_, **kw: {
            7: [],
            6: [{
                'id': 112112,
                'builder': {
                    'project': 'chromium',
                    'bucket': 'try',
                    'builder': 'linux',
                },
                'createTime': '2019-10-09T08:00:01.854286Z',
                'tags': [],
                'status': 'FAILURE',
            }],
        }[kw['patchset']]
        mockCallBuildbucket.return_value = {}

        self.assertEqual(0, git_cl.main(['try', '--retry-failed']))
        self.assertIn('Scheduling jobs on:\n'
                      '  chromium/try: linux', git_cl.sys.stdout.getvalue())

        expected_request = {
            "requests": [{
                "scheduleBuild": {
                    "requestId":
                    "uuid4",
                    "builder": {
                        "project": "chromium",
                        "bucket": "try",
                        "builder": "linux",
                    },
                    "gerritChanges": [{
                        "project": "depot_tools",
                        "host": "chromium-review.googlesource.com",
                        "patchset": 7,
                        "change": 123456,
                    }],
                    "properties": {
                        "category": "git_cl_try",
                    },
                    "tags": [
                        {
                            "value": "linux",
                            "key": "builder"
                        },
                        {
                            "value": "git_cl_try",
                            "key": "user_agent"
                        },
                        {
                            "value": "1",
                            "key": "retry_failed"
                        },
                    ],
                },
            }],
        }
        mockCallBuildbucket.assert_called_with(mock.ANY,
                                               'cr-buildbucket.appspot.com',
                                               'Batch', expected_request)

    def test_parse_bucket(self):
        test_cases = [
            {
                'bucket': 'chromium/try',
                'result': ('chromium', 'try'),
            },
            {
                'bucket': 'luci.chromium.try',
                'result': ('chromium', 'try'),
                'has_warning': True,
            },
            {
                'bucket': 'skia.primary',
                'result': ('skia', 'skia.primary'),
                'has_warning': True,
            },
            {
                'bucket': 'not-a-bucket',
                'result': (None, None),
            },
        ]

        for test_case in test_cases:
            git_cl.sys.stdout.truncate(0)
            self.assertEqual(test_case['result'],
                             git_cl._parse_bucket(test_case['bucket']))
            if test_case.get('has_warning'):
                expected_warning = 'WARNING Please use %s/%s to specify the bucket' % (
                    test_case['result'])
                self.assertIn(expected_warning, git_cl.sys.stdout.getvalue())


class CMDUploadTestCase(CMDTestCaseBase):
    def setUp(self):
        super(CMDUploadTestCase, self).setUp()
        mock.patch('git_cl._fetch_tryjobs').start()
        mock.patch('git_cl._trigger_tryjobs', return_value={}).start()
        mock.patch('git_cl.Changelist.CMDUpload', return_value=0).start()
        mock.patch('git_cl.Settings.GetRoot', return_value='').start()
        mock.patch('git_cl.Settings.GetSquashGerritUploads',
                   return_value=True).start()
        self.addCleanup(mock.patch.stopall)

class MakeRequestsHelperTestCase(unittest.TestCase):
    def exampleGerritChange(self):
        return {
            'host': 'chromium-review.googlesource.com',
            'project': 'depot_tools',
            'change': 1,
            'patchset': 2,
        }

    def testMakeRequestsHelperNoOptions(self):
        # Basic test for the helper function _make_tryjob_schedule_requests;
        # it shouldn't throw AttributeError even when options doesn't have any
        # of the expected values; it will use default option values.
        changelist = ChangelistMock(gerrit_change=self.exampleGerritChange())
        jobs = [('chromium', 'try', 'my-builder')]
        options = optparse.Values()
        requests = git_cl._make_tryjob_schedule_requests(changelist,
                                                         jobs,
                                                         options,
                                                         patchset=None)

        # requestId is non-deterministic. Just assert that it's there and has
        # a particular length.
        self.assertEqual(len(requests[0]['scheduleBuild'].pop('requestId')), 36)
        self.assertEqual(requests, [{
            'scheduleBuild': {
                'builder': {
                    'bucket': 'try',
                    'builder': 'my-builder',
                    'project': 'chromium'
                },
                'gerritChanges': [self.exampleGerritChange()],
                'properties': {
                    'category': 'git_cl_try'
                },
                'tags': [{
                    'key': 'builder',
                    'value': 'my-builder'
                }, {
                    'key': 'user_agent',
                    'value': 'git_cl_try'
                }]
            }
        }])

    def testMakeRequestsHelperPresubmitSetsDryRunProperty(self):
        changelist = ChangelistMock(gerrit_change=self.exampleGerritChange())
        jobs = [('chromium', 'try', 'presubmit')]
        options = optparse.Values()
        requests = git_cl._make_tryjob_schedule_requests(changelist,
                                                         jobs,
                                                         options,
                                                         patchset=None)
        self.assertEqual(requests[0]['scheduleBuild']['properties'], {
            'category': 'git_cl_try',
            'dry_run': 'true'
        })

    def testMakeRequestsHelperRevisionSet(self):
        # Gitiles commit is specified when revision is in options.
        changelist = ChangelistMock(gerrit_change=self.exampleGerritChange())
        jobs = [('chromium', 'try', 'my-builder')]
        options = optparse.Values({'revision': 'ba5eba11'})
        requests = git_cl._make_tryjob_schedule_requests(changelist,
                                                         jobs,
                                                         options,
                                                         patchset=None)
        self.assertEqual(
            requests[0]['scheduleBuild']['gitilesCommit'], {
                'host': 'chromium.googlesource.com',
                'id': 'ba5eba11',
                'project': 'depot_tools',
                'ref': 'refs/heads/main',
            })

    def testMakeRequestsHelperRetryFailedSet(self):
        # An extra tag is added when retry_failed is in options.
        changelist = ChangelistMock(gerrit_change=self.exampleGerritChange())
        jobs = [('chromium', 'try', 'my-builder')]
        options = optparse.Values({'retry_failed': 'true'})
        requests = git_cl._make_tryjob_schedule_requests(changelist,
                                                         jobs,
                                                         options,
                                                         patchset=None)
        self.assertEqual(requests[0]['scheduleBuild']['tags'],
                         [{
                             'key': 'builder',
                             'value': 'my-builder'
                         }, {
                             'key': 'user_agent',
                             'value': 'git_cl_try'
                         }, {
                             'key': 'retry_failed',
                             'value': '1'
                         }])

    def testMakeRequestsHelperCategorySet(self):
        # The category property can be overridden with options.
        changelist = ChangelistMock(gerrit_change=self.exampleGerritChange())
        jobs = [('chromium', 'try', 'my-builder')]
        options = optparse.Values({'category': 'my-special-category'})
        requests = git_cl._make_tryjob_schedule_requests(changelist,
                                                         jobs,
                                                         options,
                                                         patchset=None)
        self.assertEqual(requests[0]['scheduleBuild']['properties'],
                         {'category': 'my-special-category'})


class CMDFormatTestCase(unittest.TestCase):
    def setUp(self):
        super(CMDFormatTestCase, self).setUp()
        mock.patch('git_cl.RunCommand').start()
        mock.patch('clang_format.FindClangFormatToolInChromiumTree').start()
        mock.patch('clang_format.FindClangFormatScriptInChromiumTree').start()
        mock.patch('git_cl.settings').start()
        self._top_dir = tempfile.mkdtemp()
        self.addCleanup(mock.patch.stopall)

    def tearDown(self):
        shutil.rmtree(self._top_dir)
        super(CMDFormatTestCase, self).tearDown()

    def _make_temp_file(self, fname, contents):
        gclient_utils.FileWrite(os.path.join(self._top_dir, fname),
                                ('\n'.join(contents)))

    def _make_yapfignore(self, contents):
        self._make_temp_file('.yapfignore', contents)

    def _check_yapf_filtering(self, files, expected):
        self.assertEqual(
            expected,
            git_cl._FilterYapfIgnoredFiles(
                files, git_cl._GetYapfIgnorePatterns(self._top_dir)))

    def _run_command_mock(self, return_value):
        def f(*args, **kwargs):
            if 'stdin' in kwargs:
                self.assertIsInstance(kwargs['stdin'], bytes)
            return return_value

        return f

    def testClangFormatDiffFull(self):
        self._make_temp_file('test.cc', ['// test'])
        git_cl.settings.GetFormatFullByDefault.return_value = False
        diff_file = [os.path.join(self._top_dir, 'test.cc')]
        mock_opts = mock.Mock(full=True, dry_run=True, diff=False)

        # Diff
        git_cl.RunCommand.side_effect = self._run_command_mock('  // test')
        return_value = git_cl._RunClangFormatDiff(mock_opts, diff_file,
                                                  self._top_dir, 'HEAD')
        self.assertEqual(2, return_value)

        # No diff
        git_cl.RunCommand.side_effect = self._run_command_mock('// test')
        return_value = git_cl._RunClangFormatDiff(mock_opts, diff_file,
                                                  self._top_dir, 'HEAD')
        self.assertEqual(0, return_value)

    def testClangFormatDiff(self):
        git_cl.settings.GetFormatFullByDefault.return_value = False
        # A valid file is required, so use this test.
        clang_format.FindClangFormatToolInChromiumTree.return_value = __file__
        mock_opts = mock.Mock(full=False, dry_run=True, diff=False)

        # Diff
        git_cl.RunCommand.side_effect = self._run_command_mock('error')
        return_value = git_cl._RunClangFormatDiff(mock_opts, ['.'],
                                                  self._top_dir, 'HEAD')
        self.assertEqual(2, return_value)

        # No diff
        git_cl.RunCommand.side_effect = self._run_command_mock('')
        return_value = git_cl._RunClangFormatDiff(mock_opts, ['.'],
                                                  self._top_dir, 'HEAD')
        self.assertEqual(0, return_value)

    def testYapfignoreExplicit(self):
        self._make_yapfignore(['foo/bar.py', 'foo/bar/baz.py'])
        files = [
            'bar.py',
            'foo/bar.py',
            'foo/baz.py',
            'foo/bar/baz.py',
            'foo/bar/foobar.py',
        ]
        expected = [
            'bar.py',
            'foo/baz.py',
            'foo/bar/foobar.py',
        ]
        self._check_yapf_filtering(files, expected)

    def testYapfignoreSingleWildcards(self):
        self._make_yapfignore(['*bar.py', 'foo*', 'baz*.py'])
        files = [
            'bar.py',  # Matched by *bar.py.
            'bar.txt',
            'foobar.py',  # Matched by *bar.py, foo*.
            'foobar.txt',  # Matched by foo*.
            'bazbar.py',  # Matched by *bar.py, baz*.py.
            'bazbar.txt',
            'foo/baz.txt',  # Matched by foo*.
            'bar/bar.py',  # Matched by *bar.py.
            'baz/foo.py',  # Matched by baz*.py, foo*.
            'baz/foo.txt',
        ]
        expected = [
            'bar.txt',
            'bazbar.txt',
            'baz/foo.txt',
        ]
        self._check_yapf_filtering(files, expected)

    def testYapfignoreMultiplewildcards(self):
        self._make_yapfignore(['*bar*', '*foo*baz.txt'])
        files = [
            'bar.py',  # Matched by *bar*.
            'bar.txt',  # Matched by *bar*.
            'abar.py',  # Matched by *bar*.
            'foobaz.txt',  # Matched by *foo*baz.txt.
            'foobaz.py',
            'afoobaz.txt',  # Matched by *foo*baz.txt.
        ]
        expected = [
            'foobaz.py',
        ]
        self._check_yapf_filtering(files, expected)

    def testYapfignoreComments(self):
        self._make_yapfignore(['test.py', '#test2.py'])
        files = [
            'test.py',
            'test2.py',
        ]
        expected = [
            'test2.py',
        ]
        self._check_yapf_filtering(files, expected)

    def testYapfHandleUtf8(self):
        self._make_yapfignore(['test.py', 'test_🌐.py'])
        files = [
            'test.py',
            'test_🌐.py',
            'test2.py',
        ]
        expected = [
            'test2.py',
        ]
        self._check_yapf_filtering(files, expected)

    def testYapfignoreBlankLines(self):
        self._make_yapfignore(['test.py', '', '', 'test2.py'])
        files = [
            'test.py',
            'test2.py',
            'test3.py',
        ]
        expected = [
            'test3.py',
        ]
        self._check_yapf_filtering(files, expected)

    def testYapfignoreWhitespace(self):
        self._make_yapfignore([' test.py '])
        files = [
            'test.py',
            'test2.py',
        ]
        expected = [
            'test2.py',
        ]
        self._check_yapf_filtering(files, expected)

    def testYapfignoreNoFiles(self):
        self._make_yapfignore(['test.py'])
        self._check_yapf_filtering([], [])

    def testYapfignoreMissingYapfignore(self):
        files = [
            'test.py',
        ]
        expected = [
            'test.py',
        ]
        self._check_yapf_filtering(files, expected)

    @mock.patch('gclient_paths.GetPrimarySolutionPath')
    def testRunMetricsXMLFormatSkipIfPresubmit(self, find_top_dir):
        """Verifies that it skips the formatting if opts.presubmit is True."""
        find_top_dir.return_value = self._top_dir
        mock_opts = mock.Mock(full=True,
                              dry_run=True,
                              diff=False,
                              presubmit=True)
        files = [
            os.path.join(self._top_dir, 'tools', 'metrics', 'ukm', 'ukm.xml'),
        ]
        return_value = git_cl._RunMetricsXMLFormat(mock_opts, files,
                                                   self._top_dir, 'HEAD')
        git_cl.RunCommand.assert_not_called()
        self.assertEqual(0, return_value)

    @mock.patch('gclient_paths.GetPrimarySolutionPath')
    def testRunMetricsFormatWithUkm(self, find_top_dir):
        """Checks if the command line arguments do not contain the input path.
        """
        find_top_dir.return_value = self._top_dir
        mock_opts = mock.Mock(full=True,
                              dry_run=False,
                              diff=False,
                              presubmit=False)
        files = [
            os.path.join(self._top_dir, 'tools', 'metrics', 'ukm', 'ukm.xml'),
        ]
        git_cl._RunMetricsXMLFormat(mock_opts, files, self._top_dir, 'HEAD')
        git_cl.RunCommand.assert_called_with([
            mock.ANY,
            os.path.join(self._top_dir, 'tools', 'metrics', 'ukm',
                         'pretty_print.py'),
            '--non-interactive',
        ],
                                             cwd=self._top_dir)

    @mock.patch('gclient_paths.GetPrimarySolutionPath')
    def testRunMetricsFormatWithHistograms(self, find_top_dir):
        """Checks if the command line arguments contain the input file paths."""
        find_top_dir.return_value = self._top_dir
        mock_opts = mock.Mock(full=True,
                              dry_run=False,
                              diff=False,
                              presubmit=False)
        files = [
            os.path.join(self._top_dir, 'tools', 'metrics', 'histograms',
                         'enums.xml'),
            os.path.join(self._top_dir, 'tools', 'metrics', 'histograms',
                         'test_data', 'enums.xml'),
        ]
        git_cl._RunMetricsXMLFormat(mock_opts, files, self._top_dir, 'HEAD')

        pretty_print_path = os.path.join(self._top_dir, 'tools', 'metrics',
                                         'histograms', 'pretty_print.py')
        git_cl.RunCommand.assert_has_calls([
            mock.call(
                [mock.ANY, pretty_print_path, '--non-interactive', files[0]],
                cwd=self._top_dir),
            mock.call(
                [mock.ANY, pretty_print_path, '--non-interactive', files[1]],
                cwd=self._top_dir),
        ])


@unittest.skipIf(gclient_utils.IsEnvCog(),
                'not supported in non-git environment')
class CMDStatusTestCase(CMDTestCaseBase):
    # Return branch names a,..,f with comitterdates in increasing order, i.e.
    # 'f' is the most-recently changed branch.
    def _mock_run_git(commands):
        if commands == [
                'for-each-ref', '--format=%(refname) %(committerdate:unix)',
                'refs/heads'
        ]:
            branches_and_committerdates = [
                'refs/heads/a 1',
                'refs/heads/b 2',
                'refs/heads/c 3',
                'refs/heads/d 4',
                'refs/heads/e 5',
                'refs/heads/f 6',
            ]
            return '\n'.join(branches_and_committerdates)

    # Mock the status in such a way that the issue number gives us an
    # indication of the commit date (simplifies manual debugging).
    def _mock_get_cl_statuses(branches, fine_grained, max_processes):
        for c in branches:
            c.issue = (100 + int(c.GetCommitDate()))
            yield (c, 'open')

    @mock.patch('git_cl.Changelist.EnsureAuthenticated')
    @mock.patch('git_cl.Changelist.FetchDescription', lambda cl, pretty: 'x')
    @mock.patch('git_cl.Changelist.GetIssue', lambda cl: cl.issue)
    @mock.patch('git_cl.RunGit', _mock_run_git)
    @mock.patch('git_cl.get_cl_statuses', _mock_get_cl_statuses)
    @mock.patch('git_cl.Settings.GetRoot', return_value='')
    @mock.patch('git_cl.Settings.IsStatusCommitOrderByDate', return_value=False)
    @mock.patch('scm.GIT.GetBranch', return_value='a')
    def testStatus(self, *_mocks):
        self.assertEqual(0, git_cl.main(['status', '--no-branch-color']))
        self.maxDiff = None
        self.assertEqual(
            sys.stdout.getvalue(), 'Branches associated with reviews:\n'
            '    * a : https://crrev.com/c/101 (open)\n'
            '      b : https://crrev.com/c/102 (open)\n'
            '      c : https://crrev.com/c/103 (open)\n'
            '      d : https://crrev.com/c/104 (open)\n'
            '      e : https://crrev.com/c/105 (open)\n'
            '      f : https://crrev.com/c/106 (open)\n\n'
            'Current branch: a\n'
            'Issue number: 101 (https://chromium-review.googlesource.com/101)\n'
            'Issue description:\n'
            'x\n')

    @mock.patch('git_cl.Changelist.EnsureAuthenticated')
    @mock.patch('git_cl.Changelist.FetchDescription', lambda cl, pretty: 'x')
    @mock.patch('git_cl.Changelist.GetIssue', lambda cl: cl.issue)
    @mock.patch('git_cl.RunGit', _mock_run_git)
    @mock.patch('git_cl.get_cl_statuses', _mock_get_cl_statuses)
    @mock.patch('git_cl.Settings.GetRoot', return_value='')
    @mock.patch('git_cl.Settings.IsStatusCommitOrderByDate', return_value=False)
    @mock.patch('scm.GIT.GetBranch', return_value='a')
    def testStatusByDate(self, *_mocks):
        self.assertEqual(
            0, git_cl.main(['status', '--no-branch-color', '--date-order']))
        self.maxDiff = None
        self.assertEqual(
            sys.stdout.getvalue(), 'Branches associated with reviews:\n'
            '      f : https://crrev.com/c/106 (open)\n'
            '      e : https://crrev.com/c/105 (open)\n'
            '      d : https://crrev.com/c/104 (open)\n'
            '      c : https://crrev.com/c/103 (open)\n'
            '      b : https://crrev.com/c/102 (open)\n'
            '    * a : https://crrev.com/c/101 (open)\n\n'
            'Current branch: a\n'
            'Issue number: 101 (https://chromium-review.googlesource.com/101)\n'
            'Issue description:\n'
            'x\n')

    @mock.patch('git_cl.Changelist.EnsureAuthenticated')
    @mock.patch('git_cl.Changelist.FetchDescription', lambda cl, pretty: 'x')
    @mock.patch('git_cl.Changelist.GetIssue', lambda cl: cl.issue)
    @mock.patch('git_cl.RunGit', _mock_run_git)
    @mock.patch('git_cl.get_cl_statuses', _mock_get_cl_statuses)
    @mock.patch('git_cl.Settings.GetRoot', return_value='')
    @mock.patch('git_cl.Settings.IsStatusCommitOrderByDate', return_value=True)
    @mock.patch('scm.GIT.GetBranch', return_value='a')
    def testStatusByDate(self, *_mocks):
        self.assertEqual(0, git_cl.main(['status', '--no-branch-color']))
        self.maxDiff = None
        self.assertEqual(
            sys.stdout.getvalue(), 'Branches associated with reviews:\n'
            '      f : https://crrev.com/c/106 (open)\n'
            '      e : https://crrev.com/c/105 (open)\n'
            '      d : https://crrev.com/c/104 (open)\n'
            '      c : https://crrev.com/c/103 (open)\n'
            '      b : https://crrev.com/c/102 (open)\n'
            '    * a : https://crrev.com/c/101 (open)\n\n'
            'Current branch: a\n'
            'Issue number: 101 (https://chromium-review.googlesource.com/101)\n'
            'Issue description:\n'
            'x\n')


@unittest.skipIf(gclient_utils.IsEnvCog(),
                'not supported in non-git environment')
class CMDOwnersTestCase(CMDTestCaseBase):
    def setUp(self):
        super(CMDOwnersTestCase, self).setUp()
        self.owners_by_path = {
            'foo': ['a@example.com'],
            'bar': ['b@example.com', 'c@example.com'],
        }
        mock.patch('git_cl.Settings.GetRoot', return_value='root').start()
        mock.patch('git_cl.Changelist.GetAuthor', return_value='author').start()
        mock.patch('git_cl.Changelist.GetAffectedFiles',
                   return_value=list(self.owners_by_path)).start()
        mock.patch('git_cl.Changelist.GetCommonAncestorWithUpstream',
                   return_value='upstream').start()
        mock.patch('git_cl.Changelist.GetGerritHost',
                   return_value='host').start()
        mock.patch('git_cl.Changelist.GetGerritProject',
                   return_value='project').start()
        mock.patch('git_cl.Changelist.GetRemoteBranch',
                   return_value=('origin', 'refs/remotes/origin/main')).start()
        mock.patch('owners_client.OwnersClient.BatchListOwners',
                   return_value=self.owners_by_path).start()
        mock.patch('gerrit_util.IsCodeOwnersEnabledOnHost',
                   return_value=True).start()
        self.addCleanup(mock.patch.stopall)

    def testShowAllNoArgs(self):
        self.assertEqual(0, git_cl.main(['owners', '--show-all']))
        self.assertEqual('No files specified for --show-all. Nothing to do.\n',
                         git_cl.sys.stdout.getvalue())

    def testShowAll(self):
        self.assertEqual(
            0, git_cl.main(['owners', '--show-all', 'foo', 'bar', 'baz']))
        owners_client.OwnersClient.BatchListOwners.assert_called_once_with(
            ['foo', 'bar', 'baz'])
        self.assertEqual(
            '\n'.join([
                'Owners for foo:',
                ' - a@example.com',
                'Owners for bar:',
                ' - b@example.com',
                ' - c@example.com',
                'Owners for baz:',
                ' - No owners found',
                '',
            ]), sys.stdout.getvalue())

    def testBatch(self):
        self.assertEqual(0, git_cl.main(['owners', '--batch']))
        self.assertIn('a@example.com', sys.stdout.getvalue())
        self.assertIn('b@example.com', sys.stdout.getvalue())


class CMDLintTestCase(CMDTestCaseBase):
    bad_indent = '\n'.join([
        '// Copyright 1999 <a@example.com>',
        'namespace foo {',
        '  class a;',
        '}',
        '',
    ])
    filesInCL = ['foo', 'bar']

    def setUp(self):
        super(CMDLintTestCase, self).setUp()
        mock.patch('git_cl.sys.stderr', io.StringIO()).start()
        mock.patch('codecs.open', mock.mock_open()).start()
        mock.patch('os.path.isfile', return_value=True).start()

    def testLintSingleFile(self, *_mock):
        codecs.open().read.return_value = self.bad_indent
        self.assertEqual(1, git_cl.main(['lint', 'pdf.h']))
        self.assertIn('pdf.h:3:  (cpplint) Do not indent within a namespace',
                      git_cl.sys.stderr.getvalue())

    def testLintMultiFiles(self, *_mock):
        codecs.open().read.return_value = self.bad_indent
        self.assertEqual(1, git_cl.main(['lint', 'pdf.h', 'pdf.cc']))
        self.assertIn('pdf.h:3:  (cpplint) Do not indent within a namespace',
                      git_cl.sys.stderr.getvalue())
        self.assertIn('pdf.cc:3:  (cpplint) Do not indent within a namespace',
                      git_cl.sys.stderr.getvalue())

    @unittest.skipIf(gclient_utils.IsEnvCog(),
                    'not supported in non-git environment')
    @mock.patch('git_cl.Changelist.GetAffectedFiles',
                return_value=['chg-1.h', 'chg-2.cc'])
    @mock.patch('git_cl.Changelist.GetCommonAncestorWithUpstream',
                return_value='upstream')
    @mock.patch('git_cl.Settings.GetRoot', return_value='.')
    @mock.patch('git_cl.FindCodereviewSettingsFile', return_value=None)
    def testLintChangelist(self, *_mock):
        codecs.open().read.return_value = self.bad_indent
        self.assertEqual(1, git_cl.main(['lint']))
        self.assertIn('chg-1.h:3:  (cpplint) Do not indent within a namespace',
                      git_cl.sys.stderr.getvalue())
        self.assertIn('chg-2.cc:3:  (cpplint) Do not indent within a namespace',
                      git_cl.sys.stderr.getvalue())


class CMDCherryPickTestCase(CMDTestCaseBase):

    def setUp(self):
        super(CMDTestCaseBase, self).setUp()

    def testCreateCommitMessage(self):
        orig_message = """Foo the bar

This change foo's the bar.

Bug: 123456
Change-Id: I25699146b24c7ad8776f17775f489b9d41499595
"""
        expected_message = """Cherry pick "Foo the bar"

Original change's description:
> Foo the bar
> 
> This change foo's the bar.
> 
> Bug: 123456
> Change-Id: I25699146b24c7ad8776f17775f489b9d41499595

Change-Id: I25699146b24c7ad8776f17775f489b9d41499595
"""
        self.assertEqual(git_cl._create_commit_message(orig_message),
                         expected_message)

    def testCreateCommitMessageWithBug(self):
        bug = "987654"
        orig_message = """Foo the bar

This change foo's the bar.

Bug: 123456
Change-Id: I25699146b24c7ad8776f17775f489b9d41499595
"""
        expected_message = f"""Cherry pick "Foo the bar"

Original change's description:
> Foo the bar
> 
> This change foo's the bar.
> 
> Bug: 123456
> Change-Id: I25699146b24c7ad8776f17775f489b9d41499595

Bug: {bug}
Change-Id: I25699146b24c7ad8776f17775f489b9d41499595
"""
        self.assertEqual(git_cl._create_commit_message(orig_message, bug),
                         expected_message)


@unittest.skipIf(gclient_utils.IsEnvCog(),
                 'not supported in non-git environment')
class CMDSplitTestCase(CMDTestCaseBase):

    def setUp(self):
        super(CMDTestCaseBase, self).setUp()
        mock.patch('git_cl.Settings.GetRoot', return_value='root').start()

    @mock.patch("split_cl.SplitCl", return_value=0)
    @mock.patch("git_cl.OptionParser.error", side_effect=ParserErrorMock)
    def testDescriptionFlagRequired(self, _, mock_split_cl):
        # --description-file is mandatory...
        self.assertRaises(ParserErrorMock, git_cl.main, ['split'])
        self.assertEqual(mock_split_cl.call_count, 0)

        self.assertEqual(git_cl.main(['split', '--description=SomeFile.txt']),
                         0)
        self.assertEqual(mock_split_cl.call_count, 1)

        # Unless we're doing a dry run
        mock_split_cl.reset_mock()
        self.assertEqual(git_cl.main(['split', '-n']), 0)
        self.assertEqual(mock_split_cl.call_count, 1)


if __name__ == '__main__':
    logging.basicConfig(
        level=logging.DEBUG if '-v' in sys.argv else logging.ERROR)
    unittest.main()
