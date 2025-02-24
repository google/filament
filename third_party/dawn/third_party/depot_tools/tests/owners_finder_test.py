#!/usr/bin/env vpython3
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unit tests for owners_finder.py."""

import os
import sys
import unittest
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from testing_support import filesystem_mock

import owners_finder
import owners_client

ben = 'ben@example.com'
brett = 'brett@example.com'
darin = 'darin@example.com'
jochen = 'jochen@example.com'
john = 'john@example.com'
ken = 'ken@example.com'
peter = 'peter@example.com'
tom = 'tom@example.com'
nonowner = 'nonowner@example.com'


def owners_file(*email_addresses, **kwargs):
    s = ''
    if kwargs.get('comment'):
        s += '# %s\n' % kwargs.get('comment')
    if kwargs.get('noparent'):
        s += 'set noparent\n'
    return s + '\n'.join(email_addresses) + '\n'


class TestClient(owners_client.OwnersClient):
    def __init__(self):
        super(TestClient, self).__init__()
        self.owners_by_path = {
            'DEPS': [ken, peter, tom],
            'base/vlog.h': [ken, peter, tom],
            'chrome/browser/defaults.h': [brett, ben, ken, peter, tom],
            'chrome/gpu/gpu_channel.h': [ken, ben, brett, ken, peter, tom],
            'chrome/renderer/gpu/gpu_channel_host.h':
            [peter, ben, brett, ken, tom],
            'chrome/renderer/safe_browsing/scorer.h':
            [peter, ben, brett, ken, tom],
            'content/content.gyp': [john, darin],
            'content/bar/foo.cc': [john, darin],
            'content/baz/froboz.h': [brett, john, darin],
            'content/baz/ugly.cc': [brett, john, darin],
            'content/baz/ugly.h': [brett, john, darin],
            'content/common/common.cc': [jochen, john, darin],
            'content/foo/foo.cc': [jochen, john, darin],
            'content/views/pie.h': [ben, john, self.EVERYONE],
        }

    def ListOwners(self, path):
        path = path.replace(os.sep, '/')
        return self.owners_by_path[path]


class OutputInterceptedOwnersFinder(owners_finder.OwnersFinder):
    def __init__(self, files, author, reviewers, client, disable_color=False):
        super(OutputInterceptedOwnersFinder,
              self).__init__(files,
                             author,
                             reviewers,
                             client,
                             disable_color=disable_color)
        self.output = []
        self.indentation_stack = []

    def resetText(self):
        self.output = []
        self.indentation_stack = []

    def indent(self):
        self.indentation_stack.append(self.output)
        self.output = []

    def unindent(self):
        block = self.output
        self.output = self.indentation_stack.pop()
        self.output.append(block)

    def writeln(self, text=''):
        self.output.append(text)


class _BaseTestCase(unittest.TestCase):
    default_files = [
        'base/vlog.h', 'chrome/browser/defaults.h', 'chrome/gpu/gpu_channel.h',
        'chrome/renderer/gpu/gpu_channel_host.h',
        'chrome/renderer/safe_browsing/scorer.h', 'content/content.gyp',
        'content/bar/foo.cc', 'content/baz/ugly.cc', 'content/baz/ugly.h',
        'content/views/pie.h'
    ]

    def ownersFinder(self, files, author=nonowner, reviewers=None):
        reviewers = reviewers or []
        return OutputInterceptedOwnersFinder(files,
                                             author,
                                             reviewers,
                                             TestClient(),
                                             disable_color=True)

    def defaultFinder(self):
        return self.ownersFinder(self.default_files)


class OwnersFinderTests(_BaseTestCase):
    def test_constructor(self):
        self.assertNotEqual(self.defaultFinder(), None)

    def test_skip_files_owned_by_reviewers(self):
        files = [
            'chrome/browser/defaults.h',  # owned by brett
            'content/bar/foo.cc',  # not owned by brett
        ]
        finder = self.ownersFinder(files, reviewers=[brett])
        self.assertEqual(finder.unreviewed_files, {'content/bar/foo.cc'})

    def test_skip_files_owned_by_author(self):
        files = [
            'chrome/browser/defaults.h',  # owned by brett
            'content/bar/foo.cc',  # not owned by brett
        ]
        finder = self.ownersFinder(files, author=brett)
        self.assertEqual(finder.unreviewed_files, {'content/bar/foo.cc'})

    def test_native_path_sep(self):
        # Create a path with backslashes on Windows to make sure these are
        # handled. This test is a harmless duplicate on other platforms.
        native_slashes_path = 'chrome/browser/defaults.h'.replace('/', os.sep)
        files = [
            native_slashes_path,  # owned by brett
            'content/bar/foo.cc',  # not owned by brett
        ]
        finder = self.ownersFinder(files, reviewers=[brett])
        self.assertEqual(finder.unreviewed_files, {'content/bar/foo.cc'})

    @mock.patch('owners_client.OwnersClient.ScoreOwners')
    def test_reset(self, mockScoreOwners):
        mockScoreOwners.return_value = [
            brett, darin, john, peter, ken, ben, tom
        ]
        finder = self.defaultFinder()
        for _ in range(2):
            expected = [brett, darin, john, peter, ken, ben, tom]
            self.assertEqual(finder.owners_queue, expected)
            self.assertEqual(
                finder.unreviewed_files, {
                    'base/vlog.h', 'chrome/browser/defaults.h',
                    'chrome/gpu/gpu_channel.h',
                    'chrome/renderer/gpu/gpu_channel_host.h',
                    'chrome/renderer/safe_browsing/scorer.h',
                    'content/content.gyp', 'content/bar/foo.cc',
                    'content/baz/ugly.cc', 'content/baz/ugly.h'
                })
            self.assertEqual(finder.selected_owners, set())
            self.assertEqual(finder.deselected_owners, set())
            self.assertEqual(finder.reviewed_by, {})
            self.assertEqual(finder.output, [])

            finder.select_owner(john)
            finder.reset()
            finder.resetText()

    @mock.patch('owners_client.OwnersClient.ScoreOwners')
    def test_select(self, mockScoreOwners):
        mockScoreOwners.return_value = [
            brett, darin, john, peter, ken, ben, tom
        ]
        finder = self.defaultFinder()
        finder.select_owner(john)
        self.assertEqual(finder.owners_queue, [brett, peter, ken, ben, tom])
        self.assertEqual(finder.selected_owners, {john})
        self.assertEqual(finder.deselected_owners, {darin})
        self.assertEqual(
            finder.reviewed_by, {
                'content/bar/foo.cc': john,
                'content/baz/ugly.cc': john,
                'content/baz/ugly.h': john,
                'content/content.gyp': john
            })
        self.assertEqual(finder.output,
                         ['Selected: ' + john, 'Deselected: ' + darin])

        finder = self.defaultFinder()
        finder.select_owner(darin)
        self.assertEqual(finder.owners_queue, [brett, peter, ken, ben, tom])
        self.assertEqual(finder.selected_owners, {darin})
        self.assertEqual(finder.deselected_owners, {john})
        self.assertEqual(
            finder.reviewed_by, {
                'content/bar/foo.cc': darin,
                'content/baz/ugly.cc': darin,
                'content/baz/ugly.h': darin,
                'content/content.gyp': darin
            })
        self.assertEqual(finder.output,
                         ['Selected: ' + darin, 'Deselected: ' + john])

        finder = self.defaultFinder()
        finder.select_owner(brett)
        expected = [darin, john, peter, ken, tom]
        self.assertEqual(finder.owners_queue, expected)
        self.assertEqual(finder.selected_owners, {brett})
        self.assertEqual(finder.deselected_owners, {ben})
        self.assertEqual(
            finder.reviewed_by, {
                'chrome/browser/defaults.h': brett,
                'chrome/gpu/gpu_channel.h': brett,
                'chrome/renderer/gpu/gpu_channel_host.h': brett,
                'chrome/renderer/safe_browsing/scorer.h': brett,
                'content/baz/ugly.cc': brett,
                'content/baz/ugly.h': brett
            })
        self.assertEqual(finder.output,
                         ['Selected: ' + brett, 'Deselected: ' + ben])

    @mock.patch('owners_client.OwnersClient.ScoreOwners')
    def test_deselect(self, mockScoreOwners):
        mockScoreOwners.return_value = [
            brett, darin, john, peter, ken, ben, tom
        ]
        finder = self.defaultFinder()
        finder.deselect_owner(john)
        self.assertEqual(finder.owners_queue, [brett, peter, ken, ben, tom])
        self.assertEqual(finder.selected_owners, {darin})
        self.assertEqual(finder.deselected_owners, {john})
        self.assertEqual(
            finder.reviewed_by, {
                'content/bar/foo.cc': darin,
                'content/baz/ugly.cc': darin,
                'content/baz/ugly.h': darin,
                'content/content.gyp': darin
            })
        self.assertEqual(finder.output,
                         ['Deselected: ' + john, 'Selected: ' + darin])

    def test_print_file_info(self):
        finder = self.defaultFinder()
        finder.print_file_info('chrome/browser/defaults.h')
        self.assertEqual(finder.output, ['chrome/browser/defaults.h [5]'])
        finder.resetText()

        finder.print_file_info('chrome/renderer/gpu/gpu_channel_host.h')
        self.assertEqual(finder.output,
                         ['chrome/renderer/gpu/gpu_channel_host.h [5]'])

    def test_print_file_info_detailed(self):
        finder = self.defaultFinder()
        finder.print_file_info_detailed('chrome/browser/defaults.h')
        self.assertEqual(
            finder.output,
            ['chrome/browser/defaults.h', [ben, brett, ken, peter, tom]])
        finder.resetText()

        finder.print_file_info_detailed(
            'chrome/renderer/gpu/gpu_channel_host.h')
        self.assertEqual(finder.output, [
            'chrome/renderer/gpu/gpu_channel_host.h',
            [ben, brett, ken, peter, tom]
        ])


if __name__ == '__main__':
    unittest.main()
