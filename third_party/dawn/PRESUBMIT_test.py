#!/usr/bin/env python3
# Copyright 2026 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os
import sys
import unittest

# Import PRESUBMIT first before depot_tools modifies sys.path
import PRESUBMIT

# Add depot_tools to path to import mocks
sys.path.append(
    os.path.join(os.path.dirname(__file__), 'third_party', 'depot_tools'))

from testing_support.presubmit_canned_checks_test_mocks import (
    MockAffectedFile, MockInputApi, MockOutputApi)


class MockOutputApiWithLocations(MockOutputApi):

    class PresubmitResultLocation(object):

        def __init__(self, file_path, start_line, end_line):
            self.file_path = file_path
            self.start_line = start_line
            self.end_line = end_line

    class PresubmitResult(object):

        def __init__(self, message, items=None, long_text='', locations=None):
            self.message = message
            self.items = items
            self.long_text = long_text
            self.locations = locations or []

        def __repr__(self):
            return self.message

    class PresubmitError(PresubmitResult):

        def __init__(self, message, items=None, long_text='', locations=None):
            super().__init__(message, items, long_text, locations)
            self.type = 'error'

    class PresubmitPromptWarning(PresubmitResult):

        def __init__(self, message, items=None, long_text='', locations=None):
            super().__init__(message, items, long_text, locations)
            self.type = 'warning'


class CheckChangeTodoHasOwnerTest(unittest.TestCase):

    def _create_mock_input_api(self):
        mock_input_api = MockInputApi()

        # Mock AffectedFiles to support file_filter
        def mock_affected_files(include_dirs=False,
                                include_deletes=True,
                                file_filter=None):
            if file_filter:
                return [f for f in mock_input_api.files if file_filter(f)]
            return mock_input_api.files

        mock_input_api.change.AffectedFiles = mock_affected_files
        return mock_input_api

    def testNoTodo(self):
        mock_input_api = self._create_mock_input_api()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                'void Foo() {',
                '    int x = 0;',
                '}',
            ])
        ]
        errors = PRESUBMIT.CheckChangeTodoHasOwner(mock_input_api,
                                                   MockOutputApi())
        self.assertEqual(0, len(errors))

    def testValidTodo(self):
        mock_input_api = self._create_mock_input_api()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                '// TODO(crbug.com/123): fix this',
                '// TODO: owner - fix this',
            ])
        ]
        errors = PRESUBMIT.CheckChangeTodoHasOwner(mock_input_api,
                                                   MockOutputApi())
        self.assertEqual(0, len(errors))

    def testInvalidTodo(self):
        mock_input_api = self._create_mock_input_api()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                '// TODO: fix this',
            ])
        ]
        errors = PRESUBMIT.CheckChangeTodoHasOwner(mock_input_api,
                                                   MockOutputApi())
        self.assertEqual(1, len(errors))
        self.assertIn('Found TODO with no issue number', errors[0].message)

    def testDawnUnsafeTodoIgnored(self):
        mock_input_api = self._create_mock_input_api()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                'DAWN_UNSAFE_TODO(ptr[0] = 0);',
            ])
        ]
        errors = PRESUBMIT.CheckChangeTodoHasOwner(mock_input_api,
                                                   MockOutputApi())
        self.assertEqual(0, len(errors))

    def testTodoAndDawnUnsafeTodo(self):
        mock_input_api = self._create_mock_input_api()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                'DAWN_UNSAFE_TODO(ptr[0] = 0); // TODO: fix this',
            ])
        ]
        errors = PRESUBMIT.CheckChangeTodoHasOwner(mock_input_api,
                                                   MockOutputApi())
        self.assertEqual(1, len(errors))
        self.assertIn('Found TODO with no issue number', errors[0].message)

    def testMixedTodos(self):
        mock_input_api = self._create_mock_input_api()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                '// TODO: fix this',
                'DAWN_UNSAFE_TODO(ptr[0] = 0);',
            ])
        ]
        errors = PRESUBMIT.CheckChangeTodoHasOwner(mock_input_api,
                                                   MockOutputApi())
        self.assertEqual(1, len(errors))
        self.assertIn('src/dawn/Foo.cpp:1', errors[0].message)
        self.assertNotIn('src/dawn/Foo.cpp:2', errors[0].message)

    def testPresubmitFilesIgnored(self):
        mock_input_api = self._create_mock_input_api()
        mock_input_api.files = [
            MockAffectedFile('PRESUBMIT.py', [
                '// TODO: fix this',
                'DAWN_UNSAFE_TODO(ptr[0] = 0);',
            ]),
            MockAffectedFile('PRESUBMIT_test.py', [
                '// TODO: fix this',
                'DAWN_UNSAFE_TODO(ptr[0] = 0);',
            ])
        ]
        errors = PRESUBMIT.CheckChangeTodoHasOwner(mock_input_api,
                                                   MockOutputApi())
        self.assertEqual(0, len(errors))


class CheckUnsafeBuffersSafetyCommentsTest(unittest.TestCase):

    def testNoUsage(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                'void Foo() {',
                '    int x = 0;',
                '}',
            ])
        ]
        errors = PRESUBMIT.CheckUnsafeBuffersSafetyComments(
            mock_input_api, MockOutputApiWithLocations())
        self.assertEqual(0, len(errors))

    def testValidUsageSameLine(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                'void Foo() {',
                '    DAWN_UNSAFE_BUFFERS(ptr[0] = 0); // SAFETY: safe',
                '}',
            ])
        ]
        errors = PRESUBMIT.CheckUnsafeBuffersSafetyComments(
            mock_input_api, MockOutputApiWithLocations())
        self.assertEqual(0, len(errors))

    def testValidUsagePrecedingLine(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                'void Foo() {',
                '    // SAFETY: safe',
                '    DAWN_UNSAFE_BUFFERS(ptr[0] = 0);',
                '}',
            ])
        ]
        errors = PRESUBMIT.CheckUnsafeBuffersSafetyComments(
            mock_input_api, MockOutputApiWithLocations())
        self.assertEqual(0, len(errors))

    def testValidUsagePrecedingLineWithOtherComments(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                'void Foo() {',
                '    // SAFETY: safe',
                '    // some other info',
                '    DAWN_UNSAFE_BUFFERS(ptr[0] = 0);',
                '}',
            ])
        ]
        errors = PRESUBMIT.CheckUnsafeBuffersSafetyComments(
            mock_input_api, MockOutputApiWithLocations())
        self.assertEqual(0, len(errors))

    def testInvalidUsageNoComment(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                'void Foo() {',
                '    DAWN_UNSAFE_BUFFERS(ptr[0] = 0);',
                '}',
            ])
        ]
        errors = PRESUBMIT.CheckUnsafeBuffersSafetyComments(
            mock_input_api, MockOutputApiWithLocations())
        self.assertEqual(1, len(errors))
        self.assertEqual(1, len(errors[0].locations))
        loc = errors[0].locations[0]
        self.assertEqual('src/dawn/Foo.cpp', loc.file_path)
        self.assertEqual(2, loc.start_line)
        self.assertEqual(2, loc.end_line)
        self.assertIn('DAWN_UNSAFE_BUFFERS usage must be accompanied',
                      errors[0].items[0])

    def testInvalidUsageCommentNotSafety(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                'void Foo() {',
                '    // this is a comment but not safety',
                '    DAWN_UNSAFE_BUFFERS(ptr[0] = 0);',
                '}',
            ])
        ]
        errors = PRESUBMIT.CheckUnsafeBuffersSafetyComments(
            mock_input_api, MockOutputApiWithLocations())
        self.assertEqual(1, len(errors))

    def testInvalidUsageCommentSeparatedByCode(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                'void Foo() {',
                '    // SAFETY: safe',
                '    int x = 0;',
                '    DAWN_UNSAFE_BUFFERS(ptr[0] = 0);',
                '}',
            ])
        ]
        errors = PRESUBMIT.CheckUnsafeBuffersSafetyComments(
            mock_input_api, MockOutputApiWithLocations())
        self.assertEqual(1, len(errors))

    def testIgnoreCommentedUsage(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                'void Foo() {',
                '    // DAWN_UNSAFE_BUFFERS(ptr[0] = 0);',
                '}',
            ])
        ]
        errors = PRESUBMIT.CheckUnsafeBuffersSafetyComments(
            mock_input_api, MockOutputApiWithLocations())
        self.assertEqual(0, len(errors))

    def testNonCppFilesIgnored(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.txt', [
                'DAWN_UNSAFE_BUFFERS(ptr[0] = 0);',
            ])
        ]
        errors = PRESUBMIT.CheckUnsafeBuffersSafetyComments(
            mock_input_api, MockOutputApiWithLocations())
        self.assertEqual(0, len(errors))


class CheckBannedPatternsTest(unittest.TestCase):

    def testNoUsage(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                'void Foo() {',
                '    int x = 0;',
                '}',
            ])
        ]
        errors = PRESUBMIT.CheckNoBannedPatterns(mock_input_api,
                                                 MockOutputApiWithLocations())
        self.assertEqual(0, len(errors))

    def testBannedUnsafeTodo(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                'void Foo() {',
                '    DAWN_UNSAFE_TODO(ptr[0] = 0);',
                '}',
            ])
        ]
        errors = PRESUBMIT.CheckNoBannedPatterns(mock_input_api,
                                                 MockOutputApiWithLocations())
        self.assertEqual(1, len(errors))
        self.assertIn('A banned pattern was used.', errors[0].message)
        self.assertIn('src/dawn/Foo.cpp:2:', errors[0].message)
        self.assertIn('Do not introduce new instances of DAWN_UNSAFE_TODO',
                      errors[0].message)
        self.assertEqual(1, len(errors[0].locations))
        loc = errors[0].locations[0]
        self.assertEqual('src/dawn/Foo.cpp', loc.file_path)
        self.assertEqual(2, loc.start_line)
        self.assertEqual(2, loc.end_line)

    def testBannedPragma(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                '#pragma allow_unsafe_buffers',
            ])
        ]
        errors = PRESUBMIT.CheckNoBannedPatterns(mock_input_api,
                                                 MockOutputApiWithLocations())
        self.assertEqual(1, len(errors))
        self.assertIn('A banned pattern was used.', errors[0].message)
        self.assertIn('src/dawn/Foo.cpp:1:', errors[0].message)
        self.assertIn('#pragma allow_unsafe_buffers is discouraged',
                      errors[0].message)
        self.assertEqual(1, len(errors[0].locations))
        loc = errors[0].locations[0]
        self.assertEqual('src/dawn/Foo.cpp', loc.file_path)
        self.assertEqual(1, loc.start_line)
        self.assertEqual(1, loc.end_line)

    def testCommentedUsageIgnored(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                '// DAWN_UNSAFE_TODO(ptr[0] = 0);',
                '// #pragma allow_unsafe_buffers',
            ])
        ]
        errors = PRESUBMIT.CheckNoBannedPatterns(mock_input_api,
                                                 MockOutputApiWithLocations())
        self.assertEqual(0, len(errors))

    def testNonCppFilesIgnored(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.txt', [
                'DAWN_UNSAFE_TODO(ptr[0] = 0);',
                '#pragma allow_unsafe_buffers',
            ])
        ]
        errors = PRESUBMIT.CheckNoBannedPatterns(mock_input_api,
                                                 MockOutputApiWithLocations())
        self.assertEqual(0, len(errors))

    def testExplicitConstructor(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockAffectedFile('src/dawn/Foo.cpp', [
                'class C {',
                '    // NOLINTNEXTLINE(google-explicit-constructor)',
                '    C(int o);',
                '    // NOLINTNEXTLINE(misc-explicit-constructor)',
                '    C(int o);',
                '    // NOLINTNEXTLINE(cppcoreguidelines-explicit-constructor)',
                '    C(int o);',
                '    // NOLINTNEXTLINE(runtime/explicit)',
                '    C(int o);',
                '    explicit C(int o);',
                '    explicit(false) C(int o);',
                '    // NOLINTNEXTLINE(google-explicit-constructor)',
                '    operator int();',
                '    explicit operator int();',
                '}',
            ])
        ]
        errors = PRESUBMIT.CheckNoBannedPatterns(mock_input_api,
                                                 MockOutputApiWithLocations())
        self.assertEqual(3, len(errors))
        self.assertIn('A banned pattern was used.', errors[0].message)
        self.assertIn('src/dawn/Foo.cpp:4:', errors[0].message)
        self.assertIn('A banned pattern was used.', errors[1].message)
        self.assertIn('src/dawn/Foo.cpp:6:', errors[1].message)
        self.assertIn('A banned pattern was used.', errors[2].message)
        self.assertIn('src/dawn/Foo.cpp:8:', errors[2].message)


if __name__ == '__main__':
    unittest.main()
