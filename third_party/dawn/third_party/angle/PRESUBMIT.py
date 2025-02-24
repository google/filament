# Copyright 2019 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Top-level presubmit script for code generation.

See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details on the presubmit API built into depot_tools.
"""

import itertools
import os
import re
import shutil
import subprocess
import sys
import tempfile
import textwrap
import pathlib

# This line is 'magic' in that git-cl looks for it to decide whether to
# use Python3 instead of Python2 when running the code in this file.
USE_PYTHON3 = True

# Fragment of a regular expression that matches C/C++ and Objective-C++ implementation files and headers.
_IMPLEMENTATION_AND_HEADER_EXTENSIONS = r'\.(c|cc|cpp|cxx|mm|h|hpp|hxx)$'

# Fragment of a regular expression that matches C++ and Objective-C++ header files.
_HEADER_EXTENSIONS = r'\.(h|hpp|hxx)$'

_PRIMARY_EXPORT_TARGETS = [
    '//:libEGL',
    '//:libGLESv1_CM',
    '//:libGLESv2',
    '//:translator',
]


def _SplitIntoMultipleCommits(description_text):
    paragraph_split_pattern = r"(?m)(^\s*$\n)"
    multiple_paragraphs = re.split(paragraph_split_pattern, description_text)
    multiple_commits = [""]
    change_id_pattern = re.compile(r"(?m)^Change-Id: [a-zA-Z0-9]*$")
    for paragraph in multiple_paragraphs:
        multiple_commits[-1] += paragraph
        if change_id_pattern.search(paragraph):
            multiple_commits.append("")
    if multiple_commits[-1] == "":
        multiple_commits.pop()
    return multiple_commits


def _CheckCommitMessageFormatting(input_api, output_api):

    def _IsLineBlank(line):
        return line.isspace() or line == ""

    def _PopBlankLines(lines, reverse=False):
        if reverse:
            while len(lines) > 0 and _IsLineBlank(lines[-1]):
                lines.pop()
        else:
            while len(lines) > 0 and _IsLineBlank(lines[0]):
                lines.pop(0)

    def _IsTagLine(line):
        return ":" in line

    def _CheckTabInCommit(lines):
        return all([line.find("\t") == -1 for line in lines])

    allowlist_strings = ['Revert', 'Roll', 'Manual roll', 'Reland', 'Re-land']
    summary_linelength_warning_lower_limit = 65
    summary_linelength_warning_upper_limit = 70
    description_linelength_limit = 72

    git_output = input_api.change.DescriptionText()

    multiple_commits = _SplitIntoMultipleCommits(git_output)
    errors = []

    for k in range(len(multiple_commits)):
        commit_msg_lines = multiple_commits[k].splitlines()
        commit_number = len(multiple_commits) - k
        commit_tag = "Commit " + str(commit_number) + ":"
        commit_msg_line_numbers = {}
        for i in range(len(commit_msg_lines)):
            commit_msg_line_numbers[commit_msg_lines[i]] = i + 1
        _PopBlankLines(commit_msg_lines, True)
        _PopBlankLines(commit_msg_lines, False)
        allowlisted = False
        if len(commit_msg_lines) > 0:
            for allowlist_string in allowlist_strings:
                if commit_msg_lines[0].startswith(allowlist_string):
                    allowlisted = True
                    break
        if allowlisted:
            continue

        if not _CheckTabInCommit(commit_msg_lines):
            errors.append(
                output_api.PresubmitError(commit_tag + "Tabs are not allowed in commit message."))

        # the tags paragraph is at the end of the message
        # the break between the tags paragraph is the first line without ":"
        # this is sufficient because if a line is blank, it will not have ":"
        last_paragraph_line_count = 0
        while len(commit_msg_lines) > 0 and _IsTagLine(commit_msg_lines[-1]):
            last_paragraph_line_count += 1
            commit_msg_lines.pop()
        if last_paragraph_line_count == 0:
            errors.append(
                output_api.PresubmitError(
                    commit_tag +
                    "Please ensure that there are tags (e.g., Bug:, Test:) in your description."))
        if len(commit_msg_lines) > 0:
            if not _IsLineBlank(commit_msg_lines[-1]):
                output_api.PresubmitError(commit_tag +
                                          "Please ensure that there exists 1 blank line " +
                                          "between tags and description body.")
            else:
                # pop the blank line between tag paragraph and description body
                commit_msg_lines.pop()
                if len(commit_msg_lines) > 0 and _IsLineBlank(commit_msg_lines[-1]):
                    errors.append(
                        output_api.PresubmitError(
                            commit_tag + 'Please ensure that there exists only 1 blank line '
                            'between tags and description body.'))
                    # pop all the remaining blank lines between tag and description body
                    _PopBlankLines(commit_msg_lines, True)
        if len(commit_msg_lines) == 0:
            errors.append(
                output_api.PresubmitError(commit_tag +
                                          'Please ensure that your description summary'
                                          ' and description body are not blank.'))
            continue

        if summary_linelength_warning_lower_limit <= len(commit_msg_lines[0]) \
        <= summary_linelength_warning_upper_limit:
            errors.append(
                output_api.PresubmitPromptWarning(
                    commit_tag + "Your description summary should be on one line of " +
                    str(summary_linelength_warning_lower_limit - 1) + " or less characters."))
        elif len(commit_msg_lines[0]) > summary_linelength_warning_upper_limit:
            errors.append(
                output_api.PresubmitError(
                    commit_tag + "Please ensure that your description summary is on one line of " +
                    str(summary_linelength_warning_lower_limit - 1) + " or less characters."))
        commit_msg_lines.pop(0)  # get rid of description summary
        if len(commit_msg_lines) == 0:
            continue
        if not _IsLineBlank(commit_msg_lines[0]):
            errors.append(
                output_api.PresubmitError(commit_tag +
                                          'Please ensure the summary is only 1 line and '
                                          'there is 1 blank line between the summary '
                                          'and description body.'))
        else:
            commit_msg_lines.pop(0)  # pop first blank line
            if len(commit_msg_lines) == 0:
                continue
            if _IsLineBlank(commit_msg_lines[0]):
                errors.append(
                    output_api.PresubmitError(commit_tag +
                                              'Please ensure that there exists only 1 blank line '
                                              'between description summary and description body.'))
                # pop all the remaining blank lines between
                # description summary and description body
                _PopBlankLines(commit_msg_lines)

        # loop through description body
        while len(commit_msg_lines) > 0:
            line = commit_msg_lines.pop(0)
            # lines starting with 4 spaces, quotes or lines without space(urls)
            # are exempt from length check
            if line.startswith("    ") or line.startswith("> ") or " " not in line:
                continue
            if len(line) > description_linelength_limit:
                errors.append(
                    output_api.PresubmitError(
                        commit_tag + 'Line ' + str(commit_msg_line_numbers[line]) +
                        ' is too long.\n' + '"' + line + '"\n' + 'Please wrap it to ' +
                        str(description_linelength_limit) + ' characters. ' +
                        "Lines without spaces or lines starting with 4 spaces are exempt."))
                break
    return errors


def _CheckChangeHasBugField(input_api, output_api):
    """Requires that the changelist have a Bug: field from a known project."""
    bugs = input_api.change.BugsFromDescription()

    # The bug must be in the form of "project:number".  None is also accepted, which is used by
    # rollers as well as in very minor changes.
    if len(bugs) == 1 and bugs[0] == 'None':
        return []

    projects = [
        'angleproject:', 'chromium:', 'dawn:', 'fuchsia:', 'skia:', 'swiftshader:', 'tint:', 'b/'
    ]
    bug_regex = re.compile(r"([a-z]+[:/])(\d+)")
    errors = []
    extra_help = False

    if not bugs:
        errors.append('Please ensure that your description contains\n'
                      'Bug: bugtag\n'
                      'directly above the Change-Id tag (no empty line in-between)')
        extra_help = True

    for bug in bugs:
        if bug == 'None':
            errors.append('Invalid bug tag "None" in presence of other bug tags.')
            continue

        match = re.match(bug_regex, bug)
        if match == None or bug != match.group(0) or match.group(1) not in projects:
            errors.append('Incorrect bug tag "' + bug + '".')
            extra_help = True

    if extra_help:
        change_ids = re.findall('^Change-Id:', input_api.change.FullDescriptionText(), re.M)
        if len(change_ids) > 1:
            errors.append('Note: multiple Change-Id tags found in description')

        errors.append('''Acceptable bugtags:
    project:bugnumber - where project is one of ({projects})
    b/bugnumber - for Buganizer/IssueTracker bugs
'''.format(projects=', '.join(p[:-1] for p in projects if p != 'b/')))

    return [output_api.PresubmitError('\n\n'.join(errors))] if errors else []


def _CheckCodeGeneration(input_api, output_api):

    class Msg(output_api.PresubmitError):
        """Specialized error message"""

        def __init__(self, message, **kwargs):
            super(output_api.PresubmitError, self).__init__(
                message,
                long_text='Please ensure your ANGLE repositiory is synced to tip-of-tree\n'
                'and all ANGLE DEPS are fully up-to-date by running gclient sync.\n'
                '\n'
                'If that fails, run scripts/run_code_generation.py to refresh generated hashes.\n'
                '\n'
                'If you are building ANGLE inside Chromium you must bootstrap ANGLE\n'
                'before gclient sync. See the DevSetup documentation for more details.\n',
                **kwargs)

    code_gen_path = input_api.os_path.join(input_api.PresubmitLocalPath(),
                                           'scripts/run_code_generation.py')
    cmd_name = 'run_code_generation'
    cmd = [input_api.python3_executable, code_gen_path, '--verify-no-dirty']
    test_cmd = input_api.Command(name=cmd_name, cmd=cmd, kwargs={}, message=Msg)
    if input_api.verbose:
        print('Running ' + cmd_name)
    return input_api.RunTests([test_cmd])


# Taken directly from Chromium's PRESUBMIT.py
def _CheckNewHeaderWithoutGnChange(input_api, output_api):
    """Checks that newly added header files have corresponding GN changes.
  Note that this is only a heuristic. To be precise, run script:
  build/check_gn_headers.py.
  """

    def headers(f):
        return input_api.FilterSourceFile(f, files_to_check=(r'.+%s' % _HEADER_EXTENSIONS,))

    new_headers = []
    for f in input_api.AffectedSourceFiles(headers):
        if f.Action() != 'A':
            continue
        new_headers.append(f.LocalPath())

    def gn_files(f):
        return input_api.FilterSourceFile(f, files_to_check=(r'.+\.gn',))

    all_gn_changed_contents = ''
    for f in input_api.AffectedSourceFiles(gn_files):
        for _, line in f.ChangedContents():
            all_gn_changed_contents += line

    problems = []
    for header in new_headers:
        basename = input_api.os_path.basename(header)
        if basename not in all_gn_changed_contents:
            problems.append(header)

    if problems:
        return [
            output_api.PresubmitPromptWarning(
                'Missing GN changes for new header files',
                items=sorted(problems),
                long_text='Please double check whether newly added header files need '
                'corresponding changes in gn or gni files.\nThis checking is only a '
                'heuristic. Run build/check_gn_headers.py to be precise.\n'
                'Read https://crbug.com/661774 for more info.')
        ]
    return []


def _CheckExportValidity(input_api, output_api):
    outdir = tempfile.mkdtemp()
    # shell=True is necessary on Windows, as otherwise subprocess fails to find
    # either 'gn' or 'vpython3' even if they are findable via PATH.
    use_shell = input_api.is_windows
    try:
        try:
            subprocess.check_output(['gn', 'gen', outdir], shell=use_shell)
        except subprocess.CalledProcessError as e:
            return [
                output_api.PresubmitError('Unable to run gn gen for export_targets.py: %s' %
                                          e.output.decode())
            ]
        export_target_script = os.path.join(input_api.PresubmitLocalPath(), 'scripts',
                                            'export_targets.py')
        try:
            subprocess.check_output(
                ['vpython3', export_target_script, outdir] + _PRIMARY_EXPORT_TARGETS,
                stderr=subprocess.STDOUT,
                shell=use_shell)
        except subprocess.CalledProcessError as e:
            if input_api.is_committing:
                return [
                    output_api.PresubmitError('export_targets.py failed: %s' % e.output.decode())
                ]
            return [
                output_api.PresubmitPromptWarning(
                    'export_targets.py failed, this may just be due to your local checkout: %s' %
                    e.output.decode())
            ]
        return []
    finally:
        shutil.rmtree(outdir)


def _CheckTabsInSourceFiles(input_api, output_api):
    """Forbids tab characters in source files due to a WebKit repo requirement."""

    def implementation_and_headers_including_third_party(f):
        # Check third_party files too, because WebKit's checks don't make exceptions.
        return input_api.FilterSourceFile(
            f,
            files_to_check=(r'.+%s' % _IMPLEMENTATION_AND_HEADER_EXTENSIONS,),
            files_to_skip=[f for f in input_api.DEFAULT_FILES_TO_SKIP if not "third_party" in f])

    files_with_tabs = []
    for f in input_api.AffectedSourceFiles(implementation_and_headers_including_third_party):
        for (num, line) in f.ChangedContents():
            if '\t' in line:
                files_with_tabs.append(f)
                break

    if files_with_tabs:
        return [
            output_api.PresubmitError(
                'Tab characters in source files.',
                items=sorted(files_with_tabs),
                long_text=
                'Tab characters are forbidden in ANGLE source files because WebKit\'s Subversion\n'
                'repository does not allow tab characters in source files.\n'
                'Please remove tab characters from these files.')
        ]
    return []


# https://stackoverflow.com/a/196392
def is_ascii(s):
    return all(ord(c) < 128 for c in s)


def _CheckNonAsciiInSourceFiles(input_api, output_api):
    """Forbids non-ascii characters in source files."""

    def implementation_and_headers(f):
        return input_api.FilterSourceFile(
            f, files_to_check=(r'.+%s' % _IMPLEMENTATION_AND_HEADER_EXTENSIONS,))

    files_with_non_ascii = []
    for f in input_api.AffectedSourceFiles(implementation_and_headers):
        for (num, line) in f.ChangedContents():
            if not is_ascii(line):
                files_with_non_ascii.append("%s: %s" % (f, line))
                break

    if files_with_non_ascii:
        return [
            output_api.PresubmitError(
                'Non-ASCII characters in source files.',
                items=sorted(files_with_non_ascii),
                long_text='Non-ASCII characters are forbidden in ANGLE source files.\n'
                'Please remove non-ASCII characters from these files.')
        ]
    return []


def _CheckCommentBeforeTestInTestFiles(input_api, output_api):
    """Require a comment before TEST_P() and other tests."""

    def test_files(f):
        return input_api.FilterSourceFile(
            f, files_to_check=(r'^src/tests/.+\.cpp$', r'^src/.+_unittest\.cpp$'))

    tests_with_no_comment = []
    for f in input_api.AffectedSourceFiles(test_files):
        diff = f.GenerateScmDiff()
        last_line_was_comment = False
        for line in diff.splitlines():
            # Skip removed lines
            if line.startswith('-'):
                continue

            new_line_is_comment = line.startswith(' //') or line.startswith('+//')
            new_line_is_test_declaration = (
                line.startswith('+TEST_P(') or line.startswith('+TEST(') or
                line.startswith('+TYPED_TEST('))

            if new_line_is_test_declaration and not last_line_was_comment:
                tests_with_no_comment.append(line[1:])

            last_line_was_comment = new_line_is_comment

    if tests_with_no_comment:
        return [
            output_api.PresubmitError(
                'Tests without comment.',
                items=sorted(tests_with_no_comment),
                long_text='ANGLE requires a comment describing what a test does.')
        ]
    return []


def _CheckWildcardInTestExpectationFiles(input_api, output_api):
    """Require wildcard as API tag (i.e. in foo.bar/*) in expectations when no additional feature is
    enabled."""

    def expectation_files(f):
        return input_api.FilterSourceFile(
            f, files_to_check=[r'^src/tests/angle_end2end_tests_expectations.txt$'])

    expectation_pattern = re.compile(r'^.*:\s*[a-zA-Z0-9._*]+\/([^ ]*)\s*=.*$')

    expectations_without_wildcard = []
    for f in input_api.AffectedSourceFiles(expectation_files):
        diff = f.GenerateScmDiff()
        for line in diff.splitlines():
            # Only look at new lines
            if not line.startswith('+'):
                continue

            match = re.match(expectation_pattern, line[1:].strip())
            if match is None:
                continue

            tag = match.group(1)

            # The tag is in the following general form:
            #
            #     FRONTENDAPI_BACKENDAPI[_FEATURE]*
            #
            # Any part of the above may be a wildcard.  Warn about usage of FRONTEND_BACKENDAPI as
            # the tag.  Instead, the backend should be specified before the : and `*` used as the
            # tag.  If any additional tags are present, it's a specific expectation that should
            # remain specific (and not wildcarded).  NoFixture is an exception as X_Y_NoFixture is
            # the generic form of the tags of tests that don't use the fixture.

            sections = [section for section in tag.split('_') if section != 'NoFixture']

            # Allow '*_...', or 'FRONTENDAPI_*_...'.
            if '*' in sections[0] or (len(sections) > 1 and '*' in sections[1]):
                continue

            # Warn if no additional tags are present
            if len(sections) == 2:
                expectations_without_wildcard.append(line[1:])

    if expectations_without_wildcard:
        return [
            output_api.PresubmitError(
                'Use wildcard in API tags (after /) in angle_end2end_tests_expectations.txt.',
                items=expectations_without_wildcard,
                long_text="""ANGLE prefers end2end expections to use the following form:

1234 MAC OPENGL : Foo.Bar/* = SKIP

instead of:

1234 MAC OPENGL : Foo.Bar/ES2_OpenGL = SKIP
1234 MAC OPENGL : Foo.Bar/ES3_OpenGL = SKIP

Expectatations that are specific (such as Foo.Bar/ES2_OpenGL_SomeFeature) are allowed.""")
        ]
    return []


def _CheckShaderVersionInShaderLangHeader(input_api, output_api):
    """Requires an update to ANGLE_SH_VERSION when ShaderLang.h or ShaderVars.h change."""

    def headers(f):
        return input_api.FilterSourceFile(
            f,
            files_to_check=(r'^include/GLSLANG/ShaderLang.h$', r'^include/GLSLANG/ShaderVars.h$'))

    headers_changed = input_api.AffectedSourceFiles(headers)
    if len(headers_changed) == 0:
        return []

    # Skip this check for reverts and rolls.  Unlike
    # _CheckCommitMessageFormatting, relands are still checked because the
    # original change might have incremented the version correctly, but the
    # rebase over a new version could accidentally remove that (because another
    # change in the meantime identically incremented it).
    git_output = input_api.change.DescriptionText()
    multiple_commits = _SplitIntoMultipleCommits(git_output)
    for commit in multiple_commits:
        if commit.startswith('Revert') or commit.startswith('Roll'):
            return []

    diffs = '\n'.join(f.GenerateScmDiff() for f in headers_changed)
    versions = dict(re.findall(r'^([-+])#define ANGLE_SH_VERSION\s+(\d+)', diffs, re.M))

    if len(versions) != 2 or int(versions['+']) <= int(versions['-']):
        return [
            output_api.PresubmitError(
                'ANGLE_SH_VERSION should be incremented when ShaderLang.h or ShaderVars.h change.',
            )
        ]
    return []


def _CheckGClientExists(input_api, output_api, search_limit=None):
    presubmit_path = pathlib.Path(input_api.PresubmitLocalPath())

    for current_path in itertools.chain([presubmit_path], presubmit_path.parents):
        gclient_path = current_path.joinpath('.gclient')
        if gclient_path.exists() and gclient_path.is_file():
            return []
        # search_limit parameter is used in unit tests to prevent searching all the way to root
        # directory for reproducibility.
        elif search_limit != None and current_path == search_limit:
            break

    return [
        output_api.PresubmitError(
            'Missing .gclient file.',
            long_text=textwrap.fill(
                width=100,
                text='The top level directory of the repository must contain a .gclient file.'
                ' You can follow the steps outlined in the link below to get set up for ANGLE'
                ' development:') +
            '\n\nhttps://chromium.googlesource.com/angle/angle/+/refs/heads/main/doc/DevSetup.md')
    ]

def CheckChangeOnUpload(input_api, output_api):
    results = []
    results.extend(input_api.canned_checks.CheckForCommitObjects(input_api, output_api))
    results.extend(_CheckTabsInSourceFiles(input_api, output_api))
    results.extend(_CheckNonAsciiInSourceFiles(input_api, output_api))
    results.extend(_CheckCommentBeforeTestInTestFiles(input_api, output_api))
    results.extend(_CheckWildcardInTestExpectationFiles(input_api, output_api))
    results.extend(_CheckShaderVersionInShaderLangHeader(input_api, output_api))
    results.extend(_CheckCodeGeneration(input_api, output_api))
    results.extend(_CheckChangeHasBugField(input_api, output_api))
    results.extend(input_api.canned_checks.CheckChangeHasDescription(input_api, output_api))
    results.extend(_CheckNewHeaderWithoutGnChange(input_api, output_api))
    results.extend(_CheckExportValidity(input_api, output_api))
    results.extend(
        input_api.canned_checks.CheckPatchFormatted(
            input_api, output_api, result_factory=output_api.PresubmitError))
    results.extend(_CheckCommitMessageFormatting(input_api, output_api))
    results.extend(_CheckGClientExists(input_api, output_api))

    return results


def CheckChangeOnCommit(input_api, output_api):
    return CheckChangeOnUpload(input_api, output_api)
