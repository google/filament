#!/usr/bin/env python3
# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
angle_presubmit_utils_unittest.py: Top-level unittest script for ANGLE presubmit checks.
"""

import importlib.machinery
import os
import pathlib
import sys
import tempfile
import unittest
from angle_presubmit_utils import *

def SetCWDToAngleFolder():
    angle_folder = "angle"
    cwd = os.path.dirname(os.path.abspath(__file__))
    cwd = cwd.split(angle_folder)[0] + angle_folder
    os.chdir(cwd)


SetCWDToAngleFolder()

loader = importlib.machinery.SourceFileLoader('PRESUBMIT', 'PRESUBMIT.py')
PRESUBMIT = loader.load_module()


class CommitMessageFormattingCheckTest(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        super(CommitMessageFormattingCheckTest, self).__init__(*args, **kwargs)
        self.output_api = OutputAPI_mock()

    def run_check_commit_message_formatting(self, commit_msg):
        input_api = InputAPI_mock(commit_msg)
        return PRESUBMIT._CheckCommitMessageFormatting(input_api, self.output_api)

    def test_correct_commit_message(self):
        commit_msg = """a

b

Bug: angleproject:4662
Change-Id: I966c79d96175da9eee92ef6da20db50d488137b2
"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 0)

    def test_missing_description_body_and_description_summary(self):
        commit_msg = """Change-Id: I966c79d96175da9eee92ef6da20db50d488137b2"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 1)
        self.assertEqual(
            errors[0],
            self.output_api.PresubmitError(
                "Commit 1:Please ensure that your" +
                " description summary and description body are not blank."))

    def test_missing_description_body(self):
        commit_msg = """
        a

b: d
c: e
"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 0)

    def test_missing_tag_paragraph(self):
        commit_msg = """a

bd
efgh"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 1)
        self.assertEqual(
            errors[0],
            self.output_api.PresubmitError(
                "Commit 1:Please ensure that there are tags (e.g., Bug:, Test:) in your description."
            ))

    def test_missing_tag_paragraph_and_description_body(self):
        commit_msg = "a"
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 1)
        self.assertEqual(
            errors[0],
            self.output_api.PresubmitError(
                "Commit 1:Please ensure that there are tags (e.g., Bug:, Test:) in your description."
            ))

    def test_missing_blank_line_between_description_summary_and_description_body(self):
        commit_msg = """a
b

Change-Id: I925cdb45779a9cdebe4e14f9e81e4211ade37c12"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 1)
        self.assertEqual(errors[0], self.output_api.PresubmitError(
          "Commit 1:Please ensure the summary is only 1 line and there is 1 blank line" + \
          " between the summary and description body."))

    def test_missing_blank_line_between_description_body_and_tags_paragraph(self):
        commit_msg = """a

b
Change-Id: I925cdb45779a9cdebe4e14f9e81e4211ade37c12"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 0)

    def test_multiple_blank_lines_before_and_after_commit_message(self):
        commit_msg = """


                a

                  b

Change-Id: I925cdb45779a9cdebe4e14f9e81e4211ade37c12
"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 0)

    def test_newlines_within_description_body(self):
        commit_msg = """a

b

d

e

for

Change-Id: I443c36aaa8956c20da1abddf7aea613659e2cd5b"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 0)

    # Summary description in warning threshold(at 65 characters)
    def test_summmary_description_in_warning_thresholds(self):
        commit_msg = """aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa

b

Change-Id: I925cdb45779a9cdebe4e14f9e81e4211ade37c12
"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 1)
        self.assertEqual(
            errors[0],
            self.output_api.PresubmitPromptWarning(
                "Commit 1:Your description summary should be on one line of 64 or less characters."
            ))

    # Summary description in error threshold(at 71 characters)
    def test_summary_description_in_error_threshold(self):
        commit_msg = """aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa

b

Change-Id: I925cdb45779a9cdebe4e14f9e81e4211ade37c12"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 1)
        self.assertEqual(
            errors[0],
            self.output_api.PresubmitError(
                "Commit 1:Please ensure that your description summary is on one line of 64 or less characters."
            ))

    def test_description_body_exceeds_line_count_limit(self):
        commit_msg = """a

bbbbbbbb bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb


Change-Id: I443c36aaa8956c20da1abddf7aea613659e2cd5b"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 2)
        self.assertEqual(
            errors[0],
            self.output_api.PresubmitError(
                "Commit 1:Please ensure that there exists only 1 blank line between tags and description body."
            ))
        self.assertEqual(
            errors[1],
            self.output_api.PresubmitError("""Commit 1:Line 3 is too long.
"bbbbbbbb bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
Please wrap it to 72 characters. Lines without spaces or lines starting with 4 spaces are exempt."""
                                          ))

    def test_description_body_exceeds_line_count_limit_but_with_4_spaces_prefix(self):
        commit_msg = """a

cc

dddd

    bbbbbbbbbbbbbbbbbbbbbbb bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb

Change-Id: I443c36aaa8956c20da1abddf7aea613659e2cd5b"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 0)

    def test_description_body_exceeds_line_count_limit_but_without_space(self):
        commit_msg = """a

cc

dddd

bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb

a: d"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 0)

    def test_description_body_exceeds_line_count_limit_but_in_quote(self):
        commit_msg = """a

cc

dddd

> bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb

a: d"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 0)

    def test_tabs_in_commit_message(self):
        commit_msg = """																a

bbbbbbbbbbbbbbbbbbbb

Change-Id: I443c36aaa8956c20da1abddf7aea613659e2cd5b"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 1)
        self.assertEqual(
            errors[0],
            self.output_api.PresubmitError("Commit 1:Tabs are not allowed in commit message."))

    def test_allowlist_revert(self):
        commit_msg = """Revert "sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssa

bbbbbbbbbbbbbbbbbbbb

Change-Id: I443c36aaa8956c20da1abddf7aea613659e2cd5b"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 0)

    def test_allowlist_roll(self):
        commit_msg = """Roll sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssadd

bbbbbbbbbbbbbbbbbbbb

Change-Id: I443c36aaa8956c20da1abddf7aea613659e2cd5b"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 0)

    def test_allowlist_reland(self):
        commit_msg = """Reland sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssadd

bbbbbbbbbbbbbbbbbbbb

Change-Id: I443c36aaa8956c20da1abddf7aea613659e2cd5b"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 0)

    def test_allowlist_reland2(self):
        commit_msg = """Reland: sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssadd

bbbbbbbbbbbbbbbbbbbb

Change-Id: I443c36aaa8956c20da1abddf7aea613659e2cd5b"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 0)

    def test_multiple_commits_with_errors_in_multiple_commits(self):
        commit_msg = """a

bbbbbbbb bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb

Change-Id: I443c36aaa8956c20da1abddf7aea613659e2cd5b

a

cccccccccccccccccccccccccccccc cccccccccccccccccccccccccccccccccccccccccccc

Change-Id: I443c36aaa8956c20da1abddf7aea613659e2cd5b"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 2)
        self.assertEqual(
            errors[0],
            self.output_api.PresubmitError("""Commit 2:Line 3 is too long.
"bbbbbbbb bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
Please wrap it to 72 characters. Lines without spaces or lines starting with 4 spaces are exempt."""
                                          ))
        self.assertEqual(
            errors[1],
            self.output_api.PresubmitError("""Commit 1:Line 4 is too long.
"cccccccccccccccccccccccccccccc cccccccccccccccccccccccccccccccccccccccccccc"
Please wrap it to 72 characters. Lines without spaces or lines starting with 4 spaces are exempt."""
                                          ))

    def test_multiple_commits_with_error_in_one_commit(self):
        commit_msg = """a

bbbbbbbb bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb

Change-Id: I443c36aaa8956c20da1abddf7aea613659e2cd5b

Roll sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssadd

bbbbbbbbbbbbbbbbbbbb

Change-Id: I443c36aaa8956c20da1abddf7aea613659e2cd5b"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 1)
        self.assertEqual(
            errors[0],
            self.output_api.PresubmitError("""Commit 2:Line 3 is too long.
"bbbbbbbb bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
Please wrap it to 72 characters. Lines without spaces or lines starting with 4 spaces are exempt."""
                                          ))

    def test_multiple_commits_with_no_error(self):
        commit_msg = """Reland sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssadd

bbbbbbbbbbbbbbbbbbbb

Change-Id: I443c36aaa8956c20da1abddf7aea613659e2cd5b

Roll sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssadd

bbbbbbbbbbbbbbbbbbbb

Change-Id: I443c36aaa8956c20da1abddf7aea613659e2cd5b"""
        errors = self.run_check_commit_message_formatting(commit_msg)
        self.assertEqual(len(errors), 0)


class GClientFileExistenceCheck(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        super(GClientFileExistenceCheck, self).__init__(*args, **kwargs)
        self.output_api = OutputAPI_mock()

    def run_gclient_presubmit(self, cwd, search_limit):
        input_api = InputAPI_mock('')
        input_api.cwd = cwd
        return PRESUBMIT._CheckGClientExists(input_api, self.output_api,
                                             pathlib.Path(search_limit))

    def test_gclient_in_cwd(self):
        with tempfile.TemporaryDirectory() as cwd:
            pathlib.Path(cwd).joinpath('.gclient').touch()
            errors = self.run_gclient_presubmit(cwd, cwd)
            self.assertEqual(len(errors), 0)

    def test_missing_gclient(self):
        with tempfile.TemporaryDirectory() as cwd:
            errors = self.run_gclient_presubmit(cwd, cwd)
            self.assertEqual(len(errors), 1)
            self.assertEqual(errors[0], self.output_api.PresubmitError('Missing .gclient file.'))

    def test_gclient_in_parent(self):
        with tempfile.TemporaryDirectory() as cwd:
            cwd = pathlib.Path(cwd)
            cwd.joinpath('.gclient').touch()
            inner = cwd.joinpath('inner')
            inner.mkdir()
            errors = self.run_gclient_presubmit(str(inner), str(cwd))
            self.assertEqual(len(errors), 0)


class CheckShaderVersionInShaderLangHeaderTest(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        super(CheckShaderVersionInShaderLangHeaderTest, self).__init__(*args, **kwargs)
        self.output_api = OutputAPI_mock()

    def run_shader_version_check_presubmit(self, commit_msg, diffs):
        affected_files = [AffectedFile_mock(diff) for diff in diffs]
        input_api = InputAPI_mock(commit_msg, affected_files)
        return PRESUBMIT._CheckShaderVersionInShaderLangHeader(input_api, self.output_api)

    def test_headers_not_changed(self):
        errors = self.run_shader_version_check_presubmit('', [])
        self.assertEqual(len(errors), 0)

    def test_shader_lang_changed_with_version_change(self):
        shader_lang_diff = """-#define ANGLE_SH_VERSION 100
+#define ANGLE_SH_VERSION 101
"""

        errors = self.run_shader_version_check_presubmit('', [shader_lang_diff])
        self.assertEqual(len(errors), 0)

    def test_both_changed_with_version_change(self):
        shader_lang_diff = """-#define ANGLE_SH_VERSION 100
+#define ANGLE_SH_VERSION 101
"""
        shader_vars_diff = """-any change"""

        errors = self.run_shader_version_check_presubmit('', [shader_lang_diff, shader_vars_diff])
        self.assertEqual(len(errors), 0)

    def test_shader_lang_changed_with_no_version_change(self):
        shader_lang_diff = """+some change"""

        errors = self.run_shader_version_check_presubmit('', [shader_lang_diff])
        self.assertEqual(len(errors), 1)
        self.assertEqual(
            errors[0],
            self.output_api.PresubmitError(
                'ANGLE_SH_VERSION should be incremented when ShaderLang.h or ShaderVars.h change.')
        )

    def test_shader_lang_changed_with_no_version_change(self):
        shader_lang_diff = """+some change
 #define ANGLE_SH_VERSION 100
-other changes"""

        errors = self.run_shader_version_check_presubmit('', [shader_lang_diff])
        self.assertEqual(len(errors), 1)
        self.assertEqual(
            errors[0],
            self.output_api.PresubmitError(
                'ANGLE_SH_VERSION should be incremented when ShaderLang.h or ShaderVars.h change.')
        )

    def test_shader_lang_changed_with_version_cosmetic_change(self):
        shader_lang_diff = """-#define ANGLE_SH_VERSION 100
+#define ANGLE_SH_VERSION 100 // cosmetic change
"""

        errors = self.run_shader_version_check_presubmit('', [shader_lang_diff])
        self.assertEqual(len(errors), 1)
        self.assertEqual(
            errors[0],
            self.output_api.PresubmitError(
                'ANGLE_SH_VERSION should be incremented when ShaderLang.h or ShaderVars.h change.')
        )

    def test_shader_lang_changed_with_version_decrement(self):
        shader_lang_diff = """-#define ANGLE_SH_VERSION 100
+#define ANGLE_SH_VERSION 99
"""

        errors = self.run_shader_version_check_presubmit('', [shader_lang_diff])
        self.assertEqual(len(errors), 1)
        self.assertEqual(
            errors[0],
            self.output_api.PresubmitError(
                'ANGLE_SH_VERSION should be incremented when ShaderLang.h or ShaderVars.h change.')
        )

    def test_shader_lang_changed_in_revert(self):
        shader_lang_diff = """-#define ANGLE_SH_VERSION 100
+#define ANGLE_SH_VERSION 99
"""

        errors = self.run_shader_version_check_presubmit('Revert some change', [shader_lang_diff])
        self.assertEqual(len(errors), 0)


if __name__ == '__main__':
    unittest.main()
