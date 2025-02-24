#!/usr/bin/env vpython3
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for git_cache.py"""

from io import StringIO
import logging
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

DEPOT_TOOLS_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, DEPOT_TOOLS_ROOT)

from testing_support import coverage_utils
import git_cache


class GitCacheTest(unittest.TestCase):
    def setUp(self):
        self.cache_dir = tempfile.mkdtemp(prefix='git_cache_test_')
        self.addCleanup(shutil.rmtree, self.cache_dir, ignore_errors=True)
        self.origin_dir = tempfile.mkdtemp(suffix='origin.git')
        self.addCleanup(shutil.rmtree, self.origin_dir, ignore_errors=True)
        git_cache.Mirror.SetCachePath(self.cache_dir)

        # Ensure git_cache works with safe.bareRepository.
        mock.patch.dict(
            'os.environ', {
                'GIT_CONFIG_GLOBAL': os.path.join(self.cache_dir, '.gitconfig'),
            }).start()
        self.addCleanup(mock.patch.stopall)
        self.git([
            'config', '--file',
            os.path.join(self.cache_dir, '.gitconfig'), '--add',
            'safe.bareRepository', 'explicit'
        ])

    def git(self, cmd, cwd=None):
        cwd = cwd or self.origin_dir
        git = 'git.bat' if sys.platform == 'win32' else 'git'
        subprocess.check_call([git] + cmd, cwd=cwd)

    def testParseFetchSpec(self):
        testData = [([], []),
                    (['main'], [('+refs/heads/main:refs/heads/main',
                                 r'\+refs/heads/main:.*')]),
                    (['main/'], [('+refs/heads/main:refs/heads/main',
                                  r'\+refs/heads/main:.*')]),
                    (['+main'], [('+refs/heads/main:refs/heads/main',
                                  r'\+refs/heads/main:.*')]),
                    (['master'], [('+refs/heads/master:refs/heads/master',
                                   r'\+refs/heads/master:.*')]),
                    (['master/'], [('+refs/heads/master:refs/heads/master',
                                    r'\+refs/heads/master:.*')]),
                    (['+master'], [('+refs/heads/master:refs/heads/master',
                                    r'\+refs/heads/master:.*')]),
                    (['refs/heads/*'], [('+refs/heads/*:refs/heads/*',
                                         r'\+refs/heads/\*:.*')]),
                    (['foo/bar/*',
                      'baz'], [('+refs/heads/foo/bar/*:refs/heads/foo/bar/*',
                                r'\+refs/heads/foo/bar/\*:.*'),
                               ('+refs/heads/baz:refs/heads/baz',
                                r'\+refs/heads/baz:.*')]),
                    (['refs/foo/*:refs/bar/*'], [('+refs/foo/*:refs/bar/*',
                                                  r'\+refs/foo/\*:.*')])]

        mirror = git_cache.Mirror('test://phony.example.biz')
        for fetch_specs, expected in testData:
            mirror = git_cache.Mirror('test://phony.example.biz',
                                      refs=fetch_specs)
            self.assertEqual(mirror.fetch_specs, set(expected))

    def testPopulate(self):
        self.git(['init', '-q'])
        with open(os.path.join(self.origin_dir, 'foo'), 'w') as f:
            f.write('touched\n')
        self.git(['add', 'foo'])
        self.git([
            '-c', 'user.name=Test user', '-c', 'user.email=joj@test.com',
            'commit', '-m', 'foo'
        ])

        mirror = git_cache.Mirror(self.origin_dir)
        mirror.populate()

    def testPopulateResetFetchConfig(self):
        self.git(['init', '-q'])
        with open(os.path.join(self.origin_dir, 'foo'), 'w') as f:
            f.write('touched\n')
        self.git(['add', 'foo'])
        self.git([
            '-c', 'user.name=Test user', '-c', 'user.email=joj@test.com',
            'commit', '-m', 'foo'
        ])

        mirror = git_cache.Mirror(self.origin_dir)
        mirror.populate()

        # Add a bad refspec to the cache's fetch config.
        cache_dir = os.path.join(self.cache_dir,
                                 mirror.UrlToCacheDir(self.origin_dir))
        self.git([
            '--git-dir', cache_dir, 'config', '--add', 'remote.origin.fetch',
            '+refs/heads/foo:refs/heads/foo'
        ],
                 cwd=cache_dir)

        mirror.populate(reset_fetch_config=True)

    def testPopulateTwice(self):
        self.git(['init', '-q'])
        with open(os.path.join(self.origin_dir, 'foo'), 'w') as f:
            f.write('touched\n')
        self.git(['add', 'foo'])
        self.git([
            '-c', 'user.name=Test user', '-c', 'user.email=joj@test.com',
            'commit', '-m', 'foo'
        ])

        mirror = git_cache.Mirror(self.origin_dir)
        mirror.populate()

        mirror.populate()

    @mock.patch('sys.stdout', StringIO())
    def testPruneRequired(self):
        self.git(['init', '-q'])
        with open(os.path.join(self.origin_dir, 'foo'), 'w') as f:
            f.write('touched\n')
        self.git(['checkout', '-b', 'foo'])
        self.git(['add', 'foo'])
        self.git([
            '-c', 'user.name=Test user', '-c', 'user.email=joj@test.com',
            'commit', '-m', 'foo'
        ])
        mirror = git_cache.Mirror(self.origin_dir)
        mirror.populate()
        self.git(['checkout', '-b', 'foo_tmp', 'foo'])
        self.git(['branch', '-D', 'foo'])
        self.git(['checkout', '-b', 'foo/bar', 'foo_tmp'])
        mirror.populate()
        self.assertNotIn(git_cache.GIT_CACHE_CORRUPT_MESSAGE,
                         sys.stdout.getvalue())

    @mock.patch('sys.stdout', StringIO())
    def testBadInit(self):
        self.git(['init', '-q'])
        with open(os.path.join(self.origin_dir, 'foo'), 'w') as f:
            f.write('touched\n')
        self.git(['add', 'foo'])
        self.git([
            '-c', 'user.name=Test user', '-c', 'user.email=joj@test.com',
            'commit', '-m', 'foo'
        ])

        mirror = git_cache.Mirror(self.origin_dir)

        # Simulate init being interrupted during fetch phase.
        with mock.patch.object(mirror, '_fetch'):
            mirror.populate()

        # Corrupt message is not expected at this point since it was
        # "interrupted".
        self.assertNotIn(git_cache.GIT_CACHE_CORRUPT_MESSAGE,
                         sys.stdout.getvalue())

        # We call mirror.populate() without _fetch patched. This time, a
        # sentient file should prompt cache deletion.
        mirror.populate()
        self.assertIn(git_cache.GIT_CACHE_CORRUPT_MESSAGE,
                      sys.stdout.getvalue())

    def _makeGitRepoWithTag(self):
        self.git(['init', '-q'])
        with open(os.path.join(self.origin_dir, 'foo'), 'w') as f:
            f.write('touched\n')
        self.git(['add', 'foo'])
        self.git([
            '-c', 'user.name=Test user', '-c', 'user.email=joj@test.com',
            'commit', '-m', 'foo'
        ])
        self.git(['tag', 'TAG'])
        self.git(['pack-refs'])

    def testPopulateFetchTagsByDefault(self):
        self._makeGitRepoWithTag()

        # Default behaviour includes tags.
        mirror = git_cache.Mirror(self.origin_dir)
        mirror.populate()

        cache_dir = os.path.join(self.cache_dir,
                                 mirror.UrlToCacheDir(self.origin_dir))
        self.assertTrue(os.path.exists(cache_dir + '/refs/tags/TAG'))

    def testPopulateFetchWithoutTags(self):
        self._makeGitRepoWithTag()

        # Ask to not include tags.
        mirror = git_cache.Mirror(self.origin_dir)
        mirror.populate(no_fetch_tags=True)

        cache_dir = os.path.join(self.cache_dir,
                                 mirror.UrlToCacheDir(self.origin_dir))
        self.assertFalse(os.path.exists(cache_dir + '/refs/tags/TAG'))

    def testPopulateResetFetchConfigEmptyFetchConfig(self):
        self.git(['init', '-q'])
        with open(os.path.join(self.origin_dir, 'foo'), 'w') as f:
            f.write('touched\n')
        self.git(['add', 'foo'])
        self.git([
            '-c', 'user.name=Test user', '-c', 'user.email=joj@test.com',
            'commit', '-m', 'foo'
        ])

        mirror = git_cache.Mirror(self.origin_dir)
        mirror.populate(reset_fetch_config=True)


class GitCacheDirTest(unittest.TestCase):
    def setUp(self):
        try:
            delattr(git_cache.Mirror, 'cachepath')
        except AttributeError:
            pass
        super(GitCacheDirTest, self).setUp()

    def tearDown(self):
        try:
            delattr(git_cache.Mirror, 'cachepath')
        except AttributeError:
            pass
        super(GitCacheDirTest, self).tearDown()

    def test_git_config_read(self):
        (fd, tmpFile) = tempfile.mkstemp()
        old = git_cache.Mirror._GIT_CONFIG_LOCATION
        try:
            try:
                os.write(fd, b'[cache]\n  cachepath="hello world"\n')
            finally:
                os.close(fd)

            git_cache.Mirror._GIT_CONFIG_LOCATION = ['-f', tmpFile]

            self.assertEqual(git_cache.Mirror.GetCachePath(), 'hello world')
        finally:
            git_cache.Mirror._GIT_CONFIG_LOCATION = old
            os.remove(tmpFile)

    def test_environ_read(self):
        path = os.environ.get('GIT_CACHE_PATH')
        config = os.environ.get('GIT_CONFIG')
        try:
            os.environ['GIT_CACHE_PATH'] = 'hello world'
            os.environ['GIT_CONFIG'] = 'disabled'

            self.assertEqual(git_cache.Mirror.GetCachePath(), 'hello world')
        finally:
            for name, val in zip(('GIT_CACHE_PATH', 'GIT_CONFIG'),
                                 (path, config)):
                if val is None:
                    os.environ.pop(name, None)
                else:
                    os.environ[name] = val

    def test_manual_set(self):
        git_cache.Mirror.SetCachePath('hello world')
        self.assertEqual(git_cache.Mirror.GetCachePath(), 'hello world')

    def test_unconfigured(self):
        path = os.environ.get('GIT_CACHE_PATH')
        config = os.environ.get('GIT_CONFIG')
        try:
            os.environ.pop('GIT_CACHE_PATH', None)
            os.environ['GIT_CONFIG'] = 'disabled'

            with self.assertRaisesRegexp(RuntimeError, 'cache\.cachepath'):
                git_cache.Mirror.GetCachePath()

            # negatively cached value still raises
            with self.assertRaisesRegexp(RuntimeError, 'cache\.cachepath'):
                git_cache.Mirror.GetCachePath()
        finally:
            for name, val in zip(('GIT_CACHE_PATH', 'GIT_CONFIG'),
                                 (path, config)):
                if val is None:
                    os.environ.pop(name, None)
                else:
                    os.environ[name] = val


class MirrorTest(unittest.TestCase):
    def test_same_cache_for_authenticated_and_unauthenticated_urls(self):
        # GoB can fetch a repo via two different URLs; if the url contains '/a/'
        # it forces authenticated access instead of allowing anonymous access,
        # even in the case where a repo is public. We want this in order to make
        # sure bots are authenticated and get the right quotas. However, we
        # only want to maintain a single cache for the repo.
        self.assertEqual(
            git_cache.Mirror.UrlToCacheDir(
                'https://chromium.googlesource.com/a/chromium/src.git'),
            'chromium.googlesource.com-chromium-src')

    def test_ssh_url_in_UrlToCacheDir_and_CacheDirToUrl(self):
        ssh_url = "git@github.com:chromium/chromium.git"
        self.assertEqual(git_cache.Mirror.UrlToCacheDir(ssh_url),
                         "git@github.com__chromium-chromium")
        self.assertEqual(
            git_cache.Mirror.CacheDirToUrl(
                git_cache.Mirror.UrlToCacheDir(ssh_url)), ssh_url[:-4])


if __name__ == '__main__':
    logging.basicConfig(
        level=logging.DEBUG if '-v' in sys.argv else logging.ERROR)
    sys.exit(
        coverage_utils.covered_main(
            (os.path.join(DEPOT_TOOLS_ROOT, 'git_cache.py')),
            required_percentage=0))
