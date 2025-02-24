#!/usr/bin/env python3
# Copyright (c) 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os.path
import subprocess
import sys
import unittest
from unittest import mock

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT_DIR)

from testing_support.presubmit_canned_checks_test_mocks import (
    MockFile, MockAffectedFile, MockInputApi, MockOutputApi, MockChange)

import presubmit_canned_checks

# TODO: Should fix these warnings.
# pylint: disable=line-too-long


class InclusiveLanguageCheckTest(unittest.TestCase):
    def testBlockedTerms(self):
        input_api = MockInputApi()
        input_api.change.RepositoryRoot = lambda: ''
        input_api.presubmit_local_path = ''

        input_api.files = [
            MockFile(
                os.path.normpath(
                    'infra/inclusive_language_presubmit_exempt_dirs.txt'), [
                        'some/dir 2 1',
                        'some/other/dir 2 1',
                    ]),
            MockFile(
                os.path.normpath('some/ios/file.mm'),
                [
                    'TEST(SomeClassTest, SomeInteraction, blacklist) {',  # nocheck
                    '}'
                ]),
            MockFile(
                os.path.normpath('some/mac/file.mm'),
                [
                    'TEST(SomeClassTest, SomeInteraction, BlackList) {',  # nocheck
                    '}'
                ]),
            MockFile(os.path.normpath('another/ios_file.mm'),
                     ['class SomeTest : public testing::Test blocklist {};']),
            MockFile(os.path.normpath('some/ios/file_egtest.mm'),
                     ['- (void)testSomething { V(whitelist); }']),  # nocheck
            MockFile(
                os.path.normpath('some/ios/file_unittest.mm'),
                ['TEST_F(SomeTest, Whitelist) { V(allowlist); }']),  # nocheck
            MockFile(
                os.path.normpath('some/doc/file.md'),
                [
                    '# Title',
                    'Some markdown text includes master.',  # nocheck
                ]),
            MockFile(
                os.path.normpath('some/doc/ok_file.md'),
                [
                    '# Title',
                    # This link contains a '//' which the matcher thinks is a
                    # C-style comment, and the 'master' term appears after the
                    # '//' in the URL, so it gets ignored as a side-effect.
                    '[Ignored](https://git/project.git/+/master/foo)',  # nocheck
                ]),
            MockFile(
                os.path.normpath('some/doc/branch_name_file.md'),
                [
                    '# Title',
                    # Matches appearing before `//` still trigger the check.
                    '[src/master](https://git/p.git/+/master/foo)',  # nocheck
                ]),
            MockFile(
                os.path.normpath('some/java/file/TestJavaDoc.java'),
                [
                    '/**',
                    ' * This line contains the word master,',  # nocheck
                    '* ignored because this is a comment. See {@link',
                    ' * https://s/src/+/master:tools/README.md}',  # nocheck
                    ' */'
                ]),
            MockFile(
                os.path.normpath('some/java/file/TestJava.java'),
                [
                    'class TestJava {',
                    '  public String master;',  # nocheck
                    '}'
                ]),
            MockFile(
                os.path.normpath('some/html/file.html'),
                [
                    '<-- an existing html multiline comment',
                    'says "master" here',  # nocheck
                    'in the comment -->'
                ])
        ]

        errors = presubmit_canned_checks.CheckInclusiveLanguage(
            input_api, MockOutputApi())
        self.assertEqual(1, len(errors))
        self.assertTrue(
            os.path.normpath('some/ios/file.mm') in errors[0].message)
        self.assertTrue(
            os.path.normpath('another/ios_file.mm') not in errors[0].message)
        self.assertTrue(
            os.path.normpath('some/mac/file.mm') in errors[0].message)
        self.assertTrue(
            os.path.normpath('some/ios/file_egtest.mm') in errors[0].message)
        self.assertTrue(
            os.path.normpath('some/ios/file_unittest.mm') in errors[0].message)
        self.assertTrue(
            os.path.normpath('some/doc/file.md') not in errors[0].message)
        self.assertTrue(
            os.path.normpath('some/doc/ok_file.md') not in errors[0].message)
        self.assertTrue(
            os.path.normpath('some/doc/branch_name_file.md') not in
            errors[0].message)
        self.assertTrue(
            os.path.normpath('some/java/file/TestJavaDoc.java') not in
            errors[0].message)
        self.assertTrue(
            os.path.normpath('some/java/file/TestJava.java') not in
            errors[0].message)
        self.assertTrue(
            os.path.normpath('some/html/file.html') not in errors[0].message)

    def testBlockedTermsWithLegacy(self):
        input_api = MockInputApi()
        input_api.change.RepositoryRoot = lambda: ''
        input_api.presubmit_local_path = ''

        input_api.files = [
            MockFile(
                os.path.normpath(
                    'infra/inclusive_language_presubmit_exempt_dirs.txt'), [
                        'some/ios 2 1',
                        'some/other/dir 2 1',
                    ]),
            MockFile(
                os.path.normpath('some/ios/file.mm'),
                [
                    'TEST(SomeClassTest, SomeInteraction, blacklist) {',  # nocheck
                    '}'
                ]),
            MockFile(
                os.path.normpath('some/ios/subdir/file.mm'),
                [
                    'TEST(SomeClassTest, SomeInteraction, blacklist) {',  # nocheck
                    '}'
                ]),
            MockFile(
                os.path.normpath('some/mac/file.mm'),
                [
                    'TEST(SomeClassTest, SomeInteraction, BlackList) {',  # nocheck
                    '}'
                ]),
            MockFile(os.path.normpath('another/ios_file.mm'),
                     ['class SomeTest : public testing::Test blocklist {};']),
            MockFile(os.path.normpath('some/ios/file_egtest.mm'),
                     ['- (void)testSomething { V(whitelist); }']),  # nocheck
            MockFile(
                os.path.normpath('some/ios/file_unittest.mm'),
                ['TEST_F(SomeTest, Whitelist) { V(allowlist); }']),  # nocheck
        ]

        errors = presubmit_canned_checks.CheckInclusiveLanguage(
            input_api, MockOutputApi())
        self.assertEqual(1, len(errors))
        self.assertTrue(
            os.path.normpath('some/ios/file.mm') not in errors[0].message)
        self.assertTrue(
            os.path.normpath('some/ios/subdir/file.mm') in errors[0].message)
        self.assertTrue(
            os.path.normpath('another/ios_file.mm') not in errors[0].message)
        self.assertTrue(
            os.path.normpath('some/mac/file.mm') in errors[0].message)
        self.assertTrue(
            os.path.normpath('some/ios/file_egtest.mm') not in
            errors[0].message)
        self.assertTrue(
            os.path.normpath('some/ios/file_unittest.mm') not in
            errors[0].message)

    def testBlockedTermsWithNocheck(self):
        input_api = MockInputApi()
        input_api.change.RepositoryRoot = lambda: ''
        input_api.presubmit_local_path = ''

        input_api.files = [
            MockFile(
                os.path.normpath(
                    'infra/inclusive_language_presubmit_exempt_dirs.txt'), [
                        'some/dir 2 1',
                        'some/other/dir 2 1',
                    ]),
            MockFile(
                os.path.normpath('some/ios/file.mm'),
                [
                    'TEST(SomeClassTest, SomeInteraction, ',
                    ' blacklist) { // nocheck',  # nocheck
                    '}'
                ]),
            MockFile(
                os.path.normpath('some/mac/file.mm'),
                [
                    'TEST(SomeClassTest, SomeInteraction, ',
                    'BlackList) { // nocheck',  # nocheck
                    '}'
                ]),
            MockFile(os.path.normpath('another/ios_file.mm'),
                     ['class SomeTest : public testing::Test blocklist {};']),
            MockFile(os.path.normpath('some/ios/file_egtest.mm'),
                     ['- (void)testSomething { ', 'V(whitelist); } // nocheck'
                      ]),  # nocheck
            MockFile(
                os.path.normpath('some/ios/file_unittest.mm'),
                [
                    'TEST_F(SomeTest, Whitelist) // nocheck',  # nocheck
                    ' { V(allowlist); }'
                ]),
            MockFile(
                os.path.normpath('some/doc/file.md'),
                [
                    'Master in markdown <!-- nocheck -->',  # nocheck
                    '## Subheading is okay'
                ]),
            MockFile(
                os.path.normpath('some/java/file/TestJava.java'),
                [
                    'class TestJava {',
                    '  public String master; // nocheck',  # nocheck
                    '}'
                ]),
            MockFile(
                os.path.normpath('some/html/file.html'),
                [
                    '<-- an existing html multiline comment',
                    'says "master" here --><!-- nocheck -->',  # nocheck
                    '<!-- in the comment -->'
                ])
        ]

        errors = presubmit_canned_checks.CheckInclusiveLanguage(
            input_api, MockOutputApi())
        self.assertEqual(0, len(errors))

    def testTopLevelDirExcempt(self):
        input_api = MockInputApi()
        input_api.change.RepositoryRoot = lambda: ''
        input_api.presubmit_local_path = ''

        input_api.files = [
            MockFile(
                os.path.normpath(
                    'infra/inclusive_language_presubmit_exempt_dirs.txt'), [
                        '. 2 1',
                        'some/other/dir 2 1',
                    ]),
            MockFile(
                os.path.normpath('presubmit_canned_checks_test.py'),
                [
                    'TEST(SomeClassTest, SomeInteraction, blacklist) {',  # nocheck
                    '}'
                ]),
            MockFile(
                os.path.normpath('presubmit_canned_checks.py'),
                ['- (void)testSth { V(whitelist); } // nocheck']),  # nocheck
        ]

        errors = presubmit_canned_checks.CheckInclusiveLanguage(
            input_api, MockOutputApi())
        self.assertEqual(1, len(errors))
        self.assertTrue(
            os.path.normpath('presubmit_canned_checks_test.py') in
            errors[0].message)
        self.assertTrue(
            os.path.normpath('presubmit_canned_checks.py') not in
            errors[0].message)

    def testChangeIsForSomeOtherRepo(self):
        input_api = MockInputApi()
        input_api.change.RepositoryRoot = lambda: 'v8'
        input_api.presubmit_local_path = ''

        input_api.files = [
            MockFile(
                os.path.normpath('some_file'),
                [
                    '# this is a blacklist',  # nocheck
                ]),
        ]
        errors = presubmit_canned_checks.CheckInclusiveLanguage(
            input_api, MockOutputApi())
        self.assertEqual([], errors)

    def testDirExemptWithComment(self):
        input_api = MockInputApi()
        input_api.change.RepositoryRoot = lambda: ''
        input_api.presubmit_local_path = ''

        input_api.files = [
            MockFile(
                os.path.normpath(
                    'infra/inclusive_language_presubmit_exempt_dirs.txt'), [
                        '# this is a comment',
                        'dir1',
                        '# dir2',
                    ]),

            # this should be excluded
            MockFile(
                os.path.normpath('dir1/1.py'),
                [
                    'TEST(SomeClassTest, SomeInteraction, blacklist) {',  # nocheck
                    '}'
                ]),

            # this should not be excluded
            MockFile(os.path.normpath('dir2/2.py'),
                     ['- (void)testSth { V(whitelist); }']),  # nocheck
        ]

        errors = presubmit_canned_checks.CheckInclusiveLanguage(
            input_api, MockOutputApi())
        self.assertEqual(1, len(errors))
        self.assertTrue(os.path.normpath('dir1/1.py') not in errors[0].message)
        self.assertTrue(os.path.normpath('dir2/2.py') in errors[0].message)



class DescriptionChecksTest(unittest.TestCase):
    def testCheckDescriptionUsesColonInsteadOfEquals(self):
        input_api = MockInputApi()
        input_api.change.RepositoryRoot = lambda: ''
        input_api.presubmit_local_path = ''

        # Verify error in case of the attempt to use "Bug=".
        input_api.change = MockChange([], 'Broken description\nBug=123')
        errors = presubmit_canned_checks.CheckDescriptionUsesColonInsteadOfEquals(
            input_api, MockOutputApi())
        self.assertEqual(1, len(errors))
        self.assertTrue('Bug=' in errors[0].message)

        # Verify error in case of the attempt to use "Fixed=".
        input_api.change = MockChange([], 'Broken description\nFixed=123')
        errors = presubmit_canned_checks.CheckDescriptionUsesColonInsteadOfEquals(
            input_api, MockOutputApi())
        self.assertEqual(1, len(errors))
        self.assertTrue('Fixed=' in errors[0].message)

        # Verify error in case of the attempt to use the lower case "bug=".
        input_api.change = MockChange([],
                                      'Broken description lowercase\nbug=123')
        errors = presubmit_canned_checks.CheckDescriptionUsesColonInsteadOfEquals(
            input_api, MockOutputApi())
        self.assertEqual(1, len(errors))
        self.assertTrue('Bug=' in errors[0].message)

        # Verify no error in case of "Bug:"
        input_api.change = MockChange([], 'Correct description\nBug: 123')
        errors = presubmit_canned_checks.CheckDescriptionUsesColonInsteadOfEquals(
            input_api, MockOutputApi())
        self.assertEqual(0, len(errors))

        # Verify no error in case of "Fixed:"
        input_api.change = MockChange([], 'Correct description\nFixed: 123')
        errors = presubmit_canned_checks.CheckDescriptionUsesColonInsteadOfEquals(
            input_api, MockOutputApi())
        self.assertEqual(0, len(errors))


class ChromiumDependencyMetadataCheckTest(unittest.TestCase):
    def testDefaultFileFilter(self):
        """Checks the default file filter limits the scope to Chromium dependency
        metadata files.
        """
        input_api = MockInputApi()
        input_api.change.RepositoryRoot = lambda: ''
        input_api.files = [
            MockFile(os.path.normpath('foo/README.md'), ['Shipped: no?']),
            MockFile(os.path.normpath('foo/main.py'), ['Shipped: yes?']),
        ]
        results = presubmit_canned_checks.CheckChromiumDependencyMetadata(
            input_api, MockOutputApi())
        self.assertEqual(len(results), 0)

    def testSkipDeletedFiles(self):
        """Checks validation is skipped for deleted files."""
        input_api = MockInputApi()
        input_api.change.RepositoryRoot = lambda: ''
        input_api.files = [
            MockFile(os.path.normpath('foo/README.chromium'), ['No fields'],
                     action='D'),
        ]
        results = presubmit_canned_checks.CheckChromiumDependencyMetadata(
            input_api, MockOutputApi())
        self.assertEqual(len(results), 0)

    def testFeedbackForNoMetadata(self):
        """Checks presubmit results are returned for files without any metadata."""
        input_api = MockInputApi()
        input_api.change.RepositoryRoot = lambda: ''
        input_api.files = [
            MockFile(os.path.normpath('foo/README.chromium'), ['No fields']),
        ]
        results = presubmit_canned_checks.CheckChromiumDependencyMetadata(
            input_api, MockOutputApi())
        self.assertEqual(len(results), 1)
        self.assertTrue("No dependency metadata" in results[0].message)

    def testFeedbackForInvalidMetadata(self):
        """Checks presubmit results are returned for files with invalid metadata."""
        input_api = MockInputApi()
        input_api.change.RepositoryRoot = lambda: ''
        test_file = MockFile(os.path.normpath('foo/README.chromium'),
                             ['Shipped: yes?'])
        input_api.files = [test_file]
        results = presubmit_canned_checks.CheckChromiumDependencyMetadata(
            input_api, MockOutputApi())

        # There should be 9 results due to
        # - missing 5 mandatory fields: Name, URL, Version, License, and
        #                               Security Critical
        # - 1 error for insufficent versioning info
        # - missing 2 required fields: License File, and
        #                              License Android Compatible
        # - Shipped should be only 'yes' or 'no'.
        self.assertEqual(len(results), 9)

        # Check each presubmit result is associated with the test file.
        for result in results:
            self.assertEqual(len(result.items), 1)
            self.assertEqual(result.items[0], test_file)


class CheckUpdateOwnersFileReferences(unittest.TestCase):
    def testShowsWarningIfDeleting(self):
        input_api = MockInputApi()
        input_api.files = [
            MockFile(os.path.normpath('foo/OWNERS'), [], [], action='D'),
        ]
        results = presubmit_canned_checks.CheckUpdateOwnersFileReferences(
            input_api, MockOutputApi())
        self.assertEqual(1, len(results))
        self.assertEqual('warning', results[0].type)
        self.assertEqual(1, len(results[0].items))

    def testShowsWarningIfMoving(self):
        input_api = MockInputApi()
        input_api.files = [
            MockFile(os.path.normpath('new_directory/OWNERS'), [], [],
                     action='A'),
            MockFile(os.path.normpath('old_directory/OWNERS'), [], [],
                     action='D'),
        ]
        results = presubmit_canned_checks.CheckUpdateOwnersFileReferences(
            input_api, MockOutputApi())
        self.assertEqual(1, len(results))
        self.assertEqual('warning', results[0].type)
        self.assertEqual(1, len(results[0].items))

    def testNoWarningIfAdding(self):
        input_api = MockInputApi()
        input_api.files = [
            MockFile(os.path.normpath('foo/OWNERS'), [], [], action='A'),
        ]
        results = presubmit_canned_checks.CheckUpdateOwnersFileReferences(
            input_api, MockOutputApi())
        self.assertEqual(0, len(results))


class CheckNoNewGitFilesAddedInDependenciesTest(unittest.TestCase):

    @mock.patch('presubmit_canned_checks._readDeps')
    def testNonNested(self, readDeps):
        readDeps.return_value = '''deps = {
      'src/foo': {'url': 'bar', 'condition': 'non_git_source'},
      'src/components/foo/bar': {'url': 'bar', 'condition': 'non_git_source'},
    }'''

        input_api = MockInputApi()
        input_api.files = [
            MockFile('components/foo/file1.java', ['otherFunction']),
            MockFile('components/foo/file2.java', ['hasSyncConsent']),
            MockFile('chrome/foo/file3.java', ['canSyncFeatureStart']),
            MockFile('chrome/foo/file4.java', ['isSyncFeatureEnabled']),
            MockFile('chrome/foo/file5.java', ['isSyncFeatureActive']),
        ]
        results = presubmit_canned_checks.CheckNoNewGitFilesAddedInDependencies(
            input_api, MockOutputApi())

        self.assertEqual(0, len(results))

    @mock.patch('presubmit_canned_checks._readDeps')
    def testCollision(self, readDeps):
        readDeps.return_value = '''deps = {
      'src/foo': {'url': 'bar', 'condition': 'non_git_source'},
      'src/baz': {'url': 'baz'},
    }'''

        input_api = MockInputApi()
        input_api.files = [
            MockAffectedFile('fo', 'content'),  # no conflict
            MockAffectedFile('foo', 'content'),  # conflict
            MockAffectedFile('foo/bar', 'content'),  # conflict
            MockAffectedFile('baz/qux', 'content'),  # conflict, but ignored
        ]
        results = presubmit_canned_checks.CheckNoNewGitFilesAddedInDependencies(
            input_api, MockOutputApi())

        self.assertEqual(2, len(results))
        self.assertIn('File: foo', str(results))
        self.assertIn('File: foo/bar', str(results))

    @mock.patch('presubmit_canned_checks._readDeps')
    def testNoDeps(self, readDeps):
        readDeps.return_value = ''  # Empty deps

        input_api = MockInputApi()
        input_api.files = [
            MockAffectedFile('fo', 'content'),  # no conflict
            MockAffectedFile('foo', 'content'),  # conflict
            MockAffectedFile('foo/bar', 'content'),  # conflict
            MockAffectedFile('baz/qux', 'content'),  # conflict, but ignored
        ]
        results = presubmit_canned_checks.CheckNoNewGitFilesAddedInDependencies(
            input_api, MockOutputApi())

        self.assertEqual(0, len(results))


if __name__ == '__main__':
    unittest.main()
