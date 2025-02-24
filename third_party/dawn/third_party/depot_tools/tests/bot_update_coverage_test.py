#!/usr/bin/env vpython3
# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import codecs
import copy
import os
import sys
import unittest

sys.path.insert(
    0,
    os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                 'recipes', 'recipe_modules', 'bot_update', 'resources'))
import bot_update

# TODO: Should fix these warnings.
# pylint: disable=line-too-long


class MockedPopen(object):
    """A fake instance of a called subprocess.

    This is meant to be used in conjunction with MockedCall.
    """
    def __init__(self, args=None, kwargs=None):
        self.args = args or []
        self.kwargs = kwargs or {}
        self.return_value = None
        self.fails = False

    def returns(self, rv):
        """Set the return value when this popen is called.

        rv can be a string, or a callable (eg function).
        """
        self.return_value = rv
        return self

    def check(self, args, kwargs):
        """Check to see if the given args/kwargs call match this instance.

        This does a partial match, so that a call to "git clone foo" will match
        this instance if this instance was recorded as "git clone"
        """
        if any(input_arg != expected_arg
               for (input_arg, expected_arg) in zip(args, self.args)):
            return False
        return self.return_value

    def __call__(self, args, kwargs):
        """Actually call this popen instance."""
        if hasattr(self.return_value, '__call__'):
            return self.return_value(*args, **kwargs)
        return self.return_value


class MockedCall(object):
    """A fake instance of bot_update.call().

    This object is pre-seeded with "answers" in self.expectations.  The type
    is a MockedPopen object, or any object with a __call__() and check() method.
    The check() method is used to check to see if the correct popen object is
    chosen (can be a partial match, eg a "git clone" popen module would match
    a "git clone foo" call).
    By default, if no answers have been pre-seeded, the call() returns successful
    with an empty string.
    """
    def __init__(self, fake_filesystem):
        self.expectations = []
        self.records = []

    def expect(self, args=None, kwargs=None):
        args = args or []
        kwargs = kwargs or {}
        popen = MockedPopen(args, kwargs)
        self.expectations.append(popen)
        return popen

    def __call__(self, *args, **kwargs):
        self.records.append((args, kwargs))
        for popen in self.expectations:
            if popen.check(args, kwargs):
                self.expectations.remove(popen)
                return popen(args, kwargs)
        return ''


class MockedGclientSync():
    """A class producing a callable instance of gclient sync."""
    def __init__(self, fake_filesystem):
        self.records = []

    def __call__(self, *args, **_):
        self.records.append(args)


class FakeFile():
    def __init__(self):
        self.contents = ''

    def write(self, buf):
        self.contents += buf

    def read(self):
        return self.contents

    def __enter__(self):
        return self

    def __exit__(self, _, __, ___):
        pass


class FakeFilesystem():
    def __init__(self):
        self.files = {}

    def open(self, target, mode='r', encoding=None):
        if 'w' in mode:
            self.files[target] = FakeFile()
            return self.files[target]
        return self.files[target]


def fake_git(*args, **kwargs):
    return bot_update.call('git', *args, **kwargs)


class BotUpdateUnittests(unittest.TestCase):
    DEFAULT_PARAMS = {
        'solutions': [{
            'name': 'somename',
            'url': 'https://fake.com'
        }],
        'revisions': {},
        'first_sln': 'somename',
        'target_os': None,
        'target_os_only': None,
        'target_cpu': None,
        'patch_root': None,
        'patch_refs': [],
        'gerrit_rebase_patch_ref': None,
        'no_fetch_tags': False,
        'refs': [],
        'git_cache_dir': '',
        'cleanup_dir': None,
        'gerrit_reset': None,
        'enforce_fetch': False,
        'experiments': [],
    }

    def setUp(self):
        sys.platform = 'linux2'  # For consistency, ya know?
        self.filesystem = FakeFilesystem()
        self.call = MockedCall(self.filesystem)
        self.gclient = MockedGclientSync(self.filesystem)
        self.call.expect((sys.executable, '-u', bot_update.GCLIENT_PATH,
                          'sync')).returns(self.gclient)
        self.old_call = getattr(bot_update, 'call')
        self.params = copy.deepcopy(self.DEFAULT_PARAMS)
        setattr(bot_update, 'call', self.call)
        setattr(bot_update, 'git', fake_git)

        self.old_os_cwd = os.getcwd
        setattr(os, 'getcwd', lambda: '/b/build/foo/build')

        setattr(bot_update, 'open', self.filesystem.open)
        self.old_codecs_open = codecs.open
        setattr(codecs, 'open', self.filesystem.open)

    def tearDown(self):
        setattr(bot_update, 'call', self.old_call)
        setattr(os, 'getcwd', self.old_os_cwd)
        delattr(bot_update, 'open')
        setattr(codecs, 'open', self.old_codecs_open)

    def overrideSetupForWindows(self):
        sys.platform = 'win'
        self.call.expect((sys.executable, '-u', bot_update.GCLIENT_PATH,
                          'sync')).returns(self.gclient)

    def testBasic(self):
        bot_update.ensure_checkout(**self.params)
        return self.call.records

    def testBasicCachepackOffloading(self):
        os.environ['PACKFILE_OFFLOADING'] = '1'
        bot_update.ensure_checkout(**self.params)
        os.environ.pop('PACKFILE_OFFLOADING')
        return self.call.records

    def testBasicRevision(self):
        self.params['revisions'] = {
            'src': 'HEAD',
            'src/v8': 'deadbeef',
            'somename': 'DNE'
        }
        bot_update.ensure_checkout(**self.params)
        args = self.gclient.records[0]
        idx_first_revision = args.index('--revision')
        idx_second_revision = args.index('--revision', idx_first_revision + 1)
        idx_third_revision = args.index('--revision', idx_second_revision + 1)
        self.assertEqual(args[idx_first_revision + 1], 'somename@unmanaged')
        self.assertEqual(args[idx_second_revision + 1],
                         'src@refs/remotes/origin/main')
        self.assertEqual(args[idx_third_revision + 1], 'src/v8@deadbeef')
        return self.call.records

    def testTagsByDefault(self):
        bot_update.ensure_checkout(**self.params)
        found = False
        for record in self.call.records:
            args = record[0]
            if args[:3] == ('git', 'cache', 'populate'):
                self.assertFalse('--no-fetch-tags' in args)
                found = True
        self.assertTrue(found)
        return self.call.records

    def testNoTags(self):
        params = self.params
        params['no_fetch_tags'] = True
        bot_update.ensure_checkout(**params)
        found = False
        for record in self.call.records:
            args = record[0]
            if args[:3] == ('git', 'cache', 'populate'):
                self.assertTrue('--no-fetch-tags' in args)
                found = True
        self.assertTrue(found)
        return self.call.records

    def testGclientNoSyncExperiment(self):
        ref = 'refs/changes/12/345/6'
        repo = 'https://chromium.googlesource.com/v8/v8'
        self.params['patch_refs'] = ['%s@%s' % (repo, ref)]
        self.params['experiments'] = bot_update.EXP_NO_SYNC
        bot_update.ensure_checkout(**self.params)
        args = self.gclient.records[0]
        idx = args.index('--experiment')
        self.assertEqual(args[idx + 1], bot_update.EXP_NO_SYNC)

    def testApplyPatchOnGclient(self):
        ref = 'refs/changes/12/345/6'
        repo = 'https://chromium.googlesource.com/v8/v8'
        self.params['patch_refs'] = ['%s@%s' % (repo, ref)]
        bot_update.ensure_checkout(**self.params)
        args = self.gclient.records[0]
        idx = args.index('--patch-ref')
        self.assertEqual(args[idx + 1], self.params['patch_refs'][0])
        self.assertNotIn('--patch-ref', args[idx + 1:])
        # Assert we're not patching in bot_update.py
        for record in self.call.records:
            self.assertNotIn('git fetch ' + repo, ' '.join(record[0]))

    def testPatchRefs(self):
        self.params['patch_refs'] = [
            'https://chromium.googlesource.com/chromium/src@refs/changes/12/345/6',
            'https://chromium.googlesource.com/v8/v8@refs/changes/1/234/56'
        ]
        bot_update.ensure_checkout(**self.params)
        args = self.gclient.records[0]
        patch_refs = set(args[i + 1] for i in range(len(args))
                         if args[i] == '--patch-ref' and i + 1 < len(args))
        self.assertIn(self.params['patch_refs'][0], patch_refs)
        self.assertIn(self.params['patch_refs'][1], patch_refs)

    def testGitCheckoutBreaksLocks(self):
        self.overrideSetupForWindows()
        path = '/b/build/foo/build/.git'
        lockfile = 'index.lock'
        removed = []
        old_os_walk = os.walk
        old_os_remove = os.remove
        setattr(os, 'walk', lambda _: [(path, None, [lockfile])])
        setattr(os, 'remove', removed.append)
        bot_update.ensure_checkout(**self.params)
        setattr(os, 'walk', old_os_walk)
        setattr(os, 'remove', old_os_remove)
        self.assertTrue(os.path.join(path, lockfile) in removed)

    def testParsesRevisions(self):
        revisions = [
            'f671d3baeb64d9dba628ad582e867cf1aebc0207',
            'src@deadbeef',
            'https://foo.googlesource.com/bar@12345',
            'bar@refs/experimental/test@example.com/test',
        ]
        expected_results = {
            'root': 'f671d3baeb64d9dba628ad582e867cf1aebc0207',
            'src': 'deadbeef',
            'https://foo.googlesource.com/bar.git': '12345',
            'bar': 'refs/experimental/test@example.com/test',
        }
        actual_results = bot_update.parse_revisions(revisions, 'root')
        self.assertEqual(expected_results, actual_results)


class CallUnitTest(unittest.TestCase):
    def testCall(self):
        ret = bot_update.call(sys.executable, '-c', 'print(1)')
        self.assertEqual(u'1\n', ret)


if __name__ == '__main__':
    unittest.main()
