#!/usr/bin/env vpython3
# Copyright (c) 2022 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Smoke tests for gclient.py and the no-sync experiment

Shell out 'gclient' and run git tests.
"""

import json
import logging
import os
import sys
import unittest

import gclient_smoketest_base
import gclient

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT_DIR)

import subprocess2
from testing_support import fake_repos


def write(filename, content):
    """Writes the content of a file and create the directories as needed."""
    filename = os.path.abspath(filename)
    dirname = os.path.dirname(filename)
    if not os.path.isdir(dirname):
        os.makedirs(dirname)
    with open(filename, 'w') as f:
        f.write(content)


class GClientSmokeGIT(gclient_smoketest_base.GClientSmokeBase):
    """Smoke tests for the no-sync experiment."""

    FAKE_REPOS_CLASS = fake_repos.FakeRepoNoSyncDEPS

    def setUp(self):
        super(GClientSmokeGIT, self).setUp()
        self.env['PATH'] = (os.path.join(ROOT_DIR, 'testing_support') +
                            os.pathsep + self.env['PATH'])
        self.enabled = self.FAKE_REPOS.set_up_git()
        if not self.enabled:
            self.skipTest('git fake repos not available')

    def testNoSync_SkipSyncNoDEPSChange(self):
        """No DEPS changes will skip sync"""
        config_template = ''.join([
            'solutions = [{'
            '  "name"        : "src",'
            '  "url"         : %(git_base)r + "repo_1",'
            '  "deps_file"   : "DEPS",'
            '  "managed"     : True,'
            '  "custom_vars" : %(custom_vars)s,'
            '}]'
        ])
        self.gclient([
            'config', '--spec', config_template % {
                'git_base': self.git_base,
                'custom_vars': {
                    'mac': True
                }
            }
        ])

        output_json = os.path.join(self.root_dir, 'output.json')

        revision_1 = self.FAKE_REPOS.git_hashes['repo_1'][1][0]  # DEPS 1
        revision_2 = self.FAKE_REPOS.git_hashes['repo_1'][2][0]  # DEPS 1
        patch_ref = self.FAKE_REPOS.git_hashes['repo_1'][3][0]  # DEPS 2

        # Previous run did a sync at revision_1
        write(os.path.join(self.root_dir, gclient.PREVIOUS_SYNC_COMMITS_FILE),
              json.dumps({'src': revision_1}))
        write(os.path.join(self.root_dir, gclient.PREVIOUS_CUSTOM_VARS_FILE),
              json.dumps({'src': {
                  'mac': True
              }}))

        # We checkout src at revision_2 which has a different DEPS
        # but that should not matter because patch_ref and revision_1
        # have the same DEPS
        self.gclient([
            'sync', '--output-json', output_json, '--revision',
            'src@%s' % revision_2, '--patch-ref',
            '%srepo_1@refs/heads/main:%s' % (self.git_base, patch_ref),
            '--experiment', 'no-sync'
        ])

        with open(output_json) as f:
            output_json = json.load(f)
        expected = {
            'solutions': {
                'src/': {
                    'revision': revision_2,
                    'scm': 'git',
                    'url': '%srepo_1' % self.git_base,
                    'was_processed': True,
                    'was_synced': False,
                },
            },
        }
        self.assertEqual(expected, output_json)

    def testNoSync_NoSyncNotEnablted(self):
        """No DEPS changes will skip sync"""
        config_template = ''.join([
            'solutions = [{'
            '  "name"        : "src",'
            '  "url"         : %(git_base)r + "repo_1",'
            '  "deps_file"   : "DEPS",'
            '  "managed"     : True,'
            '  "custom_vars" : %(custom_vars)s,'
            '}]'
        ])
        self.gclient([
            'config', '--spec', config_template % {
                'git_base': self.git_base,
                'custom_vars': {
                    'mac': True
                }
            }
        ])

        output_json = os.path.join(self.root_dir, 'output.json')

        revision_1 = self.FAKE_REPOS.git_hashes['repo_1'][1][0]  # DEPS 1
        revision_2 = self.FAKE_REPOS.git_hashes['repo_1'][2][0]  # DEPS 1
        patch_ref = self.FAKE_REPOS.git_hashes['repo_1'][3][0]  # DEPS 2

        # Previous run did a sync at revision_1
        write(os.path.join(self.root_dir, gclient.PREVIOUS_SYNC_COMMITS_FILE),
              json.dumps({'src': revision_1}))
        write(os.path.join(self.root_dir, gclient.PREVIOUS_CUSTOM_VARS_FILE),
              json.dumps({'src': {
                  'mac': True
              }}))

        self.gclient([
            'sync', '--output-json', output_json, '--revision',
            'src@%s' % revision_2, '--patch-ref',
            '%srepo_1@refs/heads/main:%s' % (self.git_base, patch_ref)
        ])

        with open(output_json) as f:
            output_json = json.load(f)
        repo2_rev = self.FAKE_REPOS.git_hashes['repo_2'][1][0]
        expected = {
            'solutions': {
                'src/': {
                    'revision': revision_2,
                    'scm': 'git',
                    'url': '%srepo_1' % self.git_base,
                    'was_processed': True,
                    'was_synced': True,
                },
                'src/repo2/': {
                    'revision': repo2_rev,
                    'scm': 'git',
                    'url': '%srepo_2@%s' % (self.git_base, repo2_rev[:7]),
                    'was_processed': True,
                    'was_synced': True,
                },
            },
        }
        self.assertEqual(expected, output_json)

    def testNoSync_CustomVarsDiff(self):
        """We do not skip syncs if there are different custom_vars"""
        config_template = ''.join([
            'solutions = [{'
            '  "name"        : "src",'
            '  "url"         : %(git_base)r + "repo_1",'
            '  "deps_file"   : "DEPS",'
            '  "managed"     : True,'
            '  "custom_vars" : %(custom_vars)s,'
            '}]'
        ])
        self.gclient([
            'config', '--spec', config_template % {
                'git_base': self.git_base,
                'custom_vars': {
                    'mac': True
                }
            }
        ])

        output_json = os.path.join(self.root_dir, 'output.json')

        revision_1 = self.FAKE_REPOS.git_hashes['repo_1'][1][0]  # DEPS 1
        revision_2 = self.FAKE_REPOS.git_hashes['repo_1'][2][0]  # DEPS 2
        patch_ref = self.FAKE_REPOS.git_hashes['repo_1'][3][0]  # DEPS 1

        # Previous run did a sync at revision_1
        write(os.path.join(self.root_dir, gclient.PREVIOUS_SYNC_COMMITS_FILE),
              json.dumps({'src': revision_1}))
        # No PREVIOUS_CUSTOM_VARS

        # We checkout src at revision_2 which has a different DEPS
        # but that should not matter because patch_ref and revision_1
        # have the same DEPS
        self.gclient([
            'sync', '--output-json', output_json, '--revision',
            'src@%s' % revision_2, '--patch-ref',
            '%srepo_1@refs/heads/main:%s' % (self.git_base, patch_ref),
            '--experiment', 'no-sync'
        ])

        with open(output_json) as f:
            output_json = json.load(f)
        repo2_rev = self.FAKE_REPOS.git_hashes['repo_2'][1][0]
        expected = {
            'solutions': {
                'src/': {
                    'revision': revision_2,
                    'scm': 'git',
                    'url': '%srepo_1' % self.git_base,
                    'was_processed': True,
                    'was_synced': True,
                },
                'src/repo2/': {
                    'revision': repo2_rev,
                    'scm': 'git',
                    'url': '%srepo_2@%s' % (self.git_base, repo2_rev[:7]),
                    'was_processed': True,
                    'was_synced': True,
                },
            },
        }
        self.assertEqual(expected, output_json)

    def testNoSync_DEPSDiff(self):
        """We do not skip syncs if there are DEPS changes."""
        self.gclient(['config', self.git_base + 'repo_1', '--name', 'src'])

        output_json = os.path.join(self.root_dir, 'output.json')

        revision_1 = self.FAKE_REPOS.git_hashes['repo_1'][1][0]  # DEPS 1
        revision_2 = self.FAKE_REPOS.git_hashes['repo_1'][2][0]  # DEPS 2
        patch_ref = self.FAKE_REPOS.git_hashes['repo_1'][3][0]  # DEPS 1

        # Previous run did a sync at revision_1
        write(os.path.join(self.root_dir, gclient.PREVIOUS_SYNC_COMMITS_FILE),
              json.dumps({'src': revision_2}))

        # We checkout src at revision_1 which has the same DEPS
        # but that should not matter because patch_ref and revision_2
        # have different DEPS
        self.gclient([
            'sync', '--output-json', output_json, '--revision',
            'src@%s' % revision_1, '--patch-ref',
            '%srepo_1@refs/heads/main:%s' % (self.git_base, patch_ref),
            '--experiment', 'no-sync'
        ])

        with open(output_json) as f:
            output_json = json.load(f)
        repo2_rev = self.FAKE_REPOS.git_hashes['repo_2'][1][0]
        expected = {
            'solutions': {
                'src/': {
                    'revision': revision_1,
                    'scm': 'git',
                    'url': '%srepo_1' % self.git_base,
                    'was_processed': True,
                    'was_synced': True,
                },
                'src/repo2/': {
                    'revision': repo2_rev,
                    'scm': 'git',
                    'url': '%srepo_2@%s' % (self.git_base, repo2_rev[:7]),
                    'was_processed': True,
                    'was_synced': True,
                },
            },
        }
        self.assertEqual(expected, output_json)


if __name__ == '__main__':
    if '-v' in sys.argv:
        logging.basicConfig(level=logging.DEBUG)
    unittest.main()
