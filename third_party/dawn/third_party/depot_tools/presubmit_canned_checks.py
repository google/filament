# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Generic presubmit checks that can be reused by other presubmit checks."""

import datetime
import functools
import io as _io
import os as _os
import time

import metadata.discover
import metadata.validate

# TODO: Should fix these warnings.
# pylint: disable=line-too-long

_HERE = _os.path.dirname(_os.path.abspath(__file__))

# These filters will be disabled if callers do not explicitly supply a
# (possibly-empty) list.  Ideally over time this list could be driven to zero.
# TODO(pkasting): If some of these look like "should never enable", move them
# to OFF_UNLESS_MANUALLY_ENABLED_LINT_FILTERS.
#
# Justifications for each filter:
#
# - build/include       : Too many; fix in the future
#                         TODO(pkasting): Try enabling subcategories
# - build/include_order : Not happening; #ifdefed includes
# - build/namespaces    : TODO(pkasting): Try re-enabling
# - readability/casting : Mistakes a whole bunch of function pointers
# - runtime/int         : Can be fixed long term; volume of errors too high
# - whitespace/braces   : We have a lot of explicit scoping in chrome code
OFF_BY_DEFAULT_LINT_FILTERS = [
    '-build/include',
    '-build/include_order',
    '-build/namespaces',
    '-readability/casting',
    '-runtime/int',
    '-whitespace/braces',
]

# These filters will be disabled unless callers explicitly enable them, because
# they are undesirable in some way.
#
# Justifications for each filter:
# - build/c++11         : Include file and feature blocklists are
#                         google3-specific
# - build/header_guard  : Checked by CheckForIncludeGuards
# - readability/todo    : Chromium puts bug links, not usernames, in TODOs
# - runtime/references  : No longer banned by Google style guide
# - whitespace/...      : Most whitespace issues handled by clang-format
OFF_UNLESS_MANUALLY_ENABLED_LINT_FILTERS = [
    '-build/c++11',
    '-build/header_guard',
    '-readability/todo',
    '-runtime/references',
    '-whitespace/braces',
    '-whitespace/comma',
    '-whitespace/end_of_line',
    '-whitespace/forcolon',
    '-whitespace/indent',
    '-whitespace/line_length',
    '-whitespace/newline',
    '-whitespace/operators',
    '-whitespace/parens',
    '-whitespace/semicolon',
    '-whitespace/tab',
]

_CORP_LINK_KEYWORD = '.corp.google'

### Description checks


def CheckChangeHasBugFieldFromChange(change, output_api, show_suggestions=True):
    """Requires that the changelist have a Bug: field. If show_suggestions is
    False then only report on incorrect tags, not missing tags."""
    bugs = change.BugsFromDescription()
    results = []
    if bugs:
        if any(b.startswith('b/') for b in bugs):
            results.append(
                output_api.PresubmitNotifyResult(
                    'Buganizer bugs should be prefixed with b:, not b/.'))
    elif show_suggestions:
        results.append(
            output_api.PresubmitNotifyResult(
                'If this change has an associated bug, add Bug: [bug number] '
                'or Fixed: [bug number].'))

    if 'Fixes' in change.GitFootersFromDescription():
        results.append(
            output_api.PresubmitError(
                'Fixes: is the wrong footer tag, use Fixed: instead.'))
    return results


def CheckChangeHasBugField(input_api, output_api):
    return CheckChangeHasBugFieldFromChange(input_api.change, output_api)


def CheckChangeHasNoUnwantedTagsFromChange(change, output_api):
    UNWANTED_TAGS = {
        'FIXED': {
            'why': 'is not supported',
            'instead': 'Use "Fixed:" instead.'
        },
        # TODO: BUG, ISSUE
    }

    errors = []
    for tag, desc in UNWANTED_TAGS.items():
        if tag in change.tags:
            subs = tag, desc['why'], desc.get('instead', '')
            errors.append(('%s= %s. %s' % subs).rstrip())

    return [output_api.PresubmitError('\n'.join(errors))] if errors else []


def CheckChangeHasNoUnwantedTags(input_api, output_api):
    return CheckChangeHasNoUnwantedTagsFromChange(input_api.change, output_api)


def CheckDoNotSubmitInDescription(input_api, output_api):
    """Checks that the user didn't add 'DO NOT ''SUBMIT' to the CL description.
    """
    # Keyword is concatenated to avoid presubmit check rejecting the CL.
    keyword = 'DO NOT ' + 'SUBMIT'
    if keyword in input_api.change.DescriptionText():
        return [
            output_api.PresubmitError(
                keyword + ' is present in the changelist description.')
        ]

    return []


def CheckCorpLinksInDescription(input_api, output_api):
    """Checks that the description doesn't contain corp links."""
    if _CORP_LINK_KEYWORD in input_api.change.DescriptionText():
        return [
            output_api.PresubmitPromptWarning(
                'Corp link is present in the changelist description.')
        ]

    return []


def CheckChangeHasDescription(input_api, output_api):
    """Checks the CL description is not empty."""
    text = input_api.change.DescriptionText()
    if text.strip() == '':
        if input_api.is_committing and not input_api.no_diffs:
            return [output_api.PresubmitError('Add a description to the CL.')]

        return [
            output_api.PresubmitNotifyResult('Add a description to the CL.')
        ]
    return []


def CheckChangeWasUploaded(input_api, output_api):
    """Checks that the issue was uploaded before committing."""
    if input_api.is_committing and not input_api.change.issue:
        message = 'Issue wasn\'t uploaded. Please upload first.'
        if input_api.no_diffs:
            # Make this just a message with presubmit --all and --files
            return [output_api.PresubmitNotifyResult(message)]
        return [output_api.PresubmitError(message)]
    return []


def CheckDescriptionUsesColonInsteadOfEquals(input_api, output_api):
    """Checks that the CL description uses a colon after 'Bug' and 'Fixed' tags
    instead of equals.

    crbug.com only interprets the lines "Bug: xyz" and "Fixed: xyz" but not
    "Bug=xyz" or "Fixed=xyz".
    """
    text = input_api.change.DescriptionText()
    if input_api.re.search(r'^(Bug|Fixed)=',
                           text,
                           flags=input_api.re.IGNORECASE
                           | input_api.re.MULTILINE):
        return [
            output_api.PresubmitError('Use Bug:/Fixed: instead of Bug=/Fixed=')
        ]
    return []


### Content checks


def CheckAuthorizedAuthor(input_api, output_api, bot_allowlist=None):
    """For non-googler/chromites committers, verify the author's email address is
    in AUTHORS.
    """
    if input_api.is_committing or input_api.no_diffs:
        error_type = output_api.PresubmitError
    else:
        error_type = output_api.PresubmitPromptWarning

    author = input_api.change.author_email
    if not author:
        input_api.logging.info('No author, skipping AUTHOR check')
        return []

    # This is used for CLs created by trusted robot accounts.
    if bot_allowlist and author in bot_allowlist:
        return []

    authors_path = input_api.os_path.join(input_api.PresubmitLocalPath(),
                                          'AUTHORS')
    author_re = input_api.re.compile(r'[^#]+\s+\<(.+?)\>\s*$')
    valid_authors = []
    with _io.open(authors_path, encoding='utf-8') as fp:
        for line in fp:
            m = author_re.match(line)
            if m:
                valid_authors.append(m.group(1).lower())

    if not any(
            input_api.fnmatch.fnmatch(author.lower(), valid)
            for valid in valid_authors):
        input_api.logging.info('Valid authors are %s', ', '.join(valid_authors))
        return [
            error_type((
                # pylint: disable=line-too-long
                '%s is not in AUTHORS file. If you are a new contributor, please visit\n'
                'https://chromium.googlesource.com/chromium/src/+/refs/heads/main/docs/contributing.md#Legal-stuff\n'
                # pylint: enable=line-too-long
                'and read the "Legal stuff" section.\n'
                'If you are a chromite, verify that the contributor signed the '
                'CLA.') % author)
        ]
    return []


def CheckDoNotSubmitInFiles(input_api, output_api):
    """Checks that the user didn't add 'DO NOT ''SUBMIT' to any files."""
    # We want to check every text file, not just source files.
    file_filter = lambda x: x

    # Keyword is concatenated to avoid presubmit check rejecting the CL.
    keyword = 'DO NOT ' + 'SUBMIT'

    def DoNotSubmitRule(extension, line):
        try:
            return keyword not in line
        # Fallback to True for non-text content
        except UnicodeDecodeError:
            return True

    errors = _FindNewViolationsOfRule(DoNotSubmitRule, input_api, file_filter)
    text = '\n'.join('Found %s in %s' % (keyword, loc) for loc in errors)
    if text:
        return [output_api.PresubmitError(text)]
    return []


def CheckCorpLinksInFiles(input_api, output_api, source_file_filter=None):
    """Checks that files do not contain a corp link."""
    errors = _FindNewViolationsOfRule(
        lambda _, line: _CORP_LINK_KEYWORD not in line, input_api,
        source_file_filter)
    text = '\n'.join('Found corp link in %s' % loc for loc in errors)
    if text:
        return [output_api.PresubmitPromptWarning(text)]
    return []


def CheckLargeScaleChange(input_api, output_api):
    """Checks if the change should go through the LSC process."""
    size = len(input_api.AffectedFiles())
    if size <= 100:
        return []
    return [
        output_api.PresubmitPromptWarning(
            f'This change contains {size} files.\n'
            'Consider using the LSC (large scale change) process.\n'
            'See https://chromium.googlesource.com/chromium/src/+/HEAD/docs/process/lsc/lsc_workflow.md.'  # pylint: disable=line-too-long
        )
    ]


def GetCppLintFilters(lint_filters=None):
    filters = OFF_UNLESS_MANUALLY_ENABLED_LINT_FILTERS[:]
    if lint_filters is None:
        lint_filters = OFF_BY_DEFAULT_LINT_FILTERS
    filters.extend(lint_filters)
    return filters


def CheckChangeLintsClean(input_api,
                          output_api,
                          source_file_filter=None,
                          lint_filters=None,
                          verbose_level=None):
    """Checks that all '.cc' and '.h' files pass cpplint.py."""
    _RE_IS_TEST = input_api.re.compile(r'.*tests?.(cc|h)$')
    result = []

    cpplint = input_api.cpplint
    # Access to a protected member _XX of a client class
    # pylint: disable=protected-access
    cpplint._cpplint_state.ResetErrorCounts()

    cpplint._SetFilters(','.join(GetCppLintFilters(lint_filters)))

    # Use VS error format on Windows to make it easier to step through the
    # results.
    if input_api.platform == 'win32':
        cpplint._SetOutputFormat('vs7')

    if source_file_filter == None:
        # The only valid extensions for cpplint are .cc, .h, .cpp, .cu, and .ch.
        # Only process those extensions which are used in Chromium.
        INCLUDE_CPP_FILES_ONLY = (r'.*\.(cc|h|cpp)$', )
        source_file_filter = lambda x: input_api.FilterSourceFile(
            x,
            files_to_check=INCLUDE_CPP_FILES_ONLY,
            files_to_skip=input_api.DEFAULT_FILES_TO_SKIP)

    # We currently are more strict with normal code than unit tests; 4 and 5 are
    # the verbosity level that would normally be passed to cpplint.py through
    # --verbose=#. Hopefully, in the future, we can be more verbose.
    files = [
        f.AbsoluteLocalPath()
        for f in input_api.AffectedSourceFiles(source_file_filter)
    ]
    for file_name in files:
        if _RE_IS_TEST.match(file_name):
            level = 5
        else:
            level = 4

        verbose_level = verbose_level or level
        cpplint.ProcessFile(file_name, verbose_level)

    if cpplint._cpplint_state.error_count > 0:
        # cpplint errors currently cannot be counted as errors during upload
        # presubmits because some directories only run cpplint during upload and
        # therefore are far from cpplint clean.
        if input_api.is_committing:
            res_type = output_api.PresubmitError
        else:
            res_type = output_api.PresubmitPromptWarning
        result = [
            res_type('Changelist failed cpplint.py check. '
                     'Search the output for "(cpplint)"')
        ]

    return result


def CheckChangeHasNoCR(input_api, output_api, source_file_filter=None):
    """Checks no '\r' (CR) character is in any source files."""
    cr_files = []
    for f in input_api.AffectedSourceFiles(source_file_filter):
        if '\r' in input_api.ReadFile(f, 'rb'):
            cr_files.append(f.LocalPath())
    if cr_files:
        return [
            output_api.PresubmitPromptWarning(
                'Found a CR character in these files:', items=cr_files)
        ]
    return []


def CheckChangeHasOnlyOneEol(input_api, output_api, source_file_filter=None):
    """Checks the files ends with one and only one \n (LF)."""
    eof_files = []
    for f in input_api.AffectedSourceFiles(source_file_filter):
        contents = input_api.ReadFile(f, 'rb')
        # Check that the file ends in one and only one newline character.
        if len(contents) > 1 and (contents[-1:] != '\n'
                                  or contents[-2:-1] == '\n'):
            eof_files.append(f.LocalPath())

    if eof_files:
        return [
            output_api.PresubmitPromptWarning(
                'These files should end in one (and only one) newline character:',
                items=eof_files)
        ]
    return []


def CheckChangeHasNoCrAndHasOnlyOneEol(input_api,
                                       output_api,
                                       source_file_filter=None):
    """Runs both CheckChangeHasNoCR and CheckChangeHasOnlyOneEOL in one pass.

    It is faster because it is reading the file only once.
    """
    cr_files = []
    eof_files = []
    for f in input_api.AffectedSourceFiles(source_file_filter):
        contents = input_api.ReadFile(f, 'rb')
        if '\r' in contents:
            cr_files.append(f.LocalPath())
        # Check that the file ends in one and only one newline character.
        if len(contents) > 1 and (contents[-1:] != '\n'
                                  or contents[-2:-1] == '\n'):
            eof_files.append(f.LocalPath())
    outputs = []
    if cr_files:
        outputs.append(
            output_api.PresubmitPromptWarning(
                'Found a CR character in these files:', items=cr_files))
    if eof_files:
        outputs.append(
            output_api.PresubmitPromptWarning(
                'These files should end in one (and only one) newline character:',
                items=eof_files))
    return outputs


def CheckGenderNeutral(input_api, output_api, source_file_filter=None):
    """Checks that there are no gendered pronouns in any of the text files to be
    submitted.
    """
    if input_api.no_diffs:
        return []

    gendered_re = input_api.re.compile(
        r'(^|\s|\(|\[)([Hh]e|[Hh]is|[Hh]ers?|[Hh]im|[Ss]he|[Gg]uys?)\\b')

    errors = []
    for f in input_api.AffectedFiles(include_deletes=False,
                                     file_filter=source_file_filter):
        for line_num, line in f.ChangedContents():
            if gendered_re.search(line):
                errors.append('%s (%d): %s' % (f.LocalPath(), line_num, line))

    if errors:
        return [
            output_api.PresubmitPromptWarning('Found a gendered pronoun in:',
                                              long_text='\n'.join(errors))
        ]
    return []


def _ReportErrorFileAndLine(filename, line_num, dummy_line):
    """Default error formatter for _FindNewViolationsOfRule."""
    return '%s:%s' % (filename, line_num)


def _GenerateAffectedFileExtList(input_api, source_file_filter):
    """Generate a list of (file, extension) tuples from affected files.

    The result can be fed to _FindNewViolationsOfRule() directly, or
    could be filtered before doing that.

    Args:
        input_api: object to enumerate the affected files.
        source_file_filter: a filter to be passed to the input api.
    Yields:
        A list of (file, extension) tuples, where |file| is an affected
        file, and |extension| its file path extension.
    """
    for f in input_api.AffectedFiles(include_deletes=False,
                                     file_filter=source_file_filter):
        extension = str(f.LocalPath()).rsplit('.', 1)[-1]
        yield (f, extension)


def _FindNewViolationsOfRuleForList(callable_rule,
                                    file_ext_list,
                                    error_formatter=_ReportErrorFileAndLine):
    """Find all newly introduced violations of a per-line rule (a callable).

  Prefer calling _FindNewViolationsOfRule() instead of this function, unless
    the list of affected files need to be filtered in a special way.

    Arguments:
        callable_rule: a callable taking a file extension and line of input and
            returning True if the rule is satisfied and False if there was a problem.
        file_ext_list: a list of input (file, extension) tuples, as returned by
            _GenerateAffectedFileExtList().
        error_formatter: a callable taking (filename, line_number, line) and
            returning a formatted error string.

    Returns:
        A list of the newly-introduced violations reported by the rule.
    """
    errors = []
    for f, extension in file_ext_list:
        # For speed, we do two passes, checking first the full file.  Shelling
        # out to the SCM to determine the changed region can be quite expensive
        # on Win32.  Assuming that most files will be kept problem-free, we can
        # skip the SCM operations most of the time.
        if all(callable_rule(extension, line) for line in f.NewContents()):
            continue  # No violation found in full text: can skip considering diff.

        for line_num, line in f.ChangedContents():
            if not callable_rule(extension, line):
                errors.append(error_formatter(f.LocalPath(), line_num, line))

    return errors


def _FindNewViolationsOfRule(callable_rule,
                             input_api,
                             source_file_filter=None,
                             error_formatter=_ReportErrorFileAndLine):
    """Find all newly introduced violations of a per-line rule (a callable).

    Arguments:
        callable_rule: a callable taking a file extension and line of input and
            returning True if the rule is satisfied and False if there was a problem.
        input_api: object to enumerate the affected files.
        source_file_filter: a filter to be passed to the input api.
        error_formatter: a callable taking (filename, line_number, line) and
            returning a formatted error string.

    Returns:
        A list of the newly-introduced violations reported by the rule.
    """
    if input_api.no_diffs:
        return []
    return _FindNewViolationsOfRuleForList(
        callable_rule,
        _GenerateAffectedFileExtList(input_api, source_file_filter),
        error_formatter)


def CheckChangeHasNoTabs(input_api, output_api, source_file_filter=None):
    """Checks that there are no tab characters in any of the text files to be
    submitted.
    """
    # In addition to the filter, make sure that makefiles are skipped.
    if not source_file_filter:
        # It's the default filter.
        source_file_filter = input_api.FilterSourceFile

    def filter_more(affected_file):
        basename = input_api.os_path.basename(affected_file.LocalPath())
        return (not (basename in ('Makefile', 'makefile')
                     or basename.endswith('.mk'))
                and source_file_filter(affected_file))

    tabs = _FindNewViolationsOfRule(lambda _, line: '\t' not in line, input_api,
                                    filter_more)

    if tabs:
        return [
            output_api.PresubmitPromptWarning('Found a tab character in:',
                                              long_text='\n'.join(tabs))
        ]
    return []


def CheckChangeTodoHasOwner(input_api, output_api, source_file_filter=None):
    """Checks that the user didn't add `TODO(name)` or `TODO: name -` without
    an owner.
    """
    legacyTODO = '\\s*\\(.+\\)\\s*:'
    modernTODO = ':\\s*[^\\s]+\\s*\\-'
    unowned_todo = input_api.re.compile('TODO(?!(%s|%s))' %
                                        (legacyTODO, modernTODO))
    errors = _FindNewViolationsOfRule(lambda _, x: not unowned_todo.search(x),
                                      input_api, source_file_filter)
    errors = ['Found TODO with no owner in ' + x for x in errors]
    if errors:
        return [output_api.PresubmitPromptWarning('\n'.join(errors))]
    return []


def CheckChangeHasNoStrayWhitespace(input_api,
                                    output_api,
                                    source_file_filter=None):
    """Checks that there is no stray whitespace at source lines end."""
    errors = _FindNewViolationsOfRule(lambda _, line: line.rstrip() == line,
                                      input_api, source_file_filter)
    if errors:
        return [
            output_api.PresubmitPromptWarning(
                'Found line ending with white spaces in:',
                long_text='\n'.join(errors))
        ]
    return []


def CheckLongLines(input_api, output_api, maxlen, source_file_filter=None):
    """Checks that there aren't any lines longer than maxlen characters in any of
    the text files to be submitted.
    """
    if input_api.no_diffs:
        return []
    maxlens = {
        'java': 100,
        # This is specifically for Android's handwritten makefiles (Android.mk).
        'mk': 200,
        'rs': 100,
        '': maxlen,
    }

    # To avoid triggering on the magic string, break it up.
    LINT_THEN_CHANGE_EXCEPTION = ('LI'
                                  'NT.ThenChange(')

    # Language specific exceptions to max line length.
    # '.h' is considered an obj-c file extension, since OBJC_EXCEPTIONS are a
    # superset of CPP_EXCEPTIONS.
    CPP_FILE_EXTS = ('c', 'cc')
    CPP_EXCEPTIONS = ('#define', '#endif', '#if', '#include', '#pragma',
                      '// ' + LINT_THEN_CHANGE_EXCEPTION)
    HTML_FILE_EXTS = ('html', )
    HTML_EXCEPTIONS = ('<g ', '<link ', '<path ',
                       '<!-- ' + LINT_THEN_CHANGE_EXCEPTION)
    JAVA_FILE_EXTS = ('java', )
    JAVA_EXCEPTIONS = ('import ', 'package ',
                       '// ' + LINT_THEN_CHANGE_EXCEPTION)
    JS_FILE_EXTS = ('js', )
    JS_EXCEPTIONS = ("GEN('#include", 'import ',
                     '// ' + LINT_THEN_CHANGE_EXCEPTION)
    TS_FILE_EXTS = ('ts', )
    TS_EXCEPTIONS = ('import ', '// ' + LINT_THEN_CHANGE_EXCEPTION)
    OBJC_FILE_EXTS = ('h', 'm', 'mm')
    OBJC_EXCEPTIONS = ('#define', '#endif', '#if', '#import', '#include',
                       '#pragma', '// ' + LINT_THEN_CHANGE_EXCEPTION)
    PY_FILE_EXTS = ('py', )
    PY_EXCEPTIONS = ('import', 'from', '# ' + LINT_THEN_CHANGE_EXCEPTION)

    LANGUAGE_EXCEPTIONS = [
        (CPP_FILE_EXTS, CPP_EXCEPTIONS),
        (HTML_FILE_EXTS, HTML_EXCEPTIONS),
        (JAVA_FILE_EXTS, JAVA_EXCEPTIONS),
        (JS_FILE_EXTS, JS_EXCEPTIONS),
        (TS_FILE_EXTS, TS_EXCEPTIONS),
        (OBJC_FILE_EXTS, OBJC_EXCEPTIONS),
        (PY_FILE_EXTS, PY_EXCEPTIONS),
    ]

    def no_long_lines(file_extension, line):
        """Returns True if the line length is okay."""
        # Check for language specific exceptions.
        if any(file_extension in exts and line.lstrip().startswith(exceptions)
               for exts, exceptions in LANGUAGE_EXCEPTIONS):
            return True

        file_maxlen = maxlens.get(file_extension, maxlens[''])
        # Stupidly long symbols that needs to be worked around if takes 66% of
        # line.
        long_symbol = file_maxlen * 2 / 3
        # Hard line length limit at 50% more.
        extra_maxlen = file_maxlen * 3 / 2

        line_len = len(line)
        if line_len <= file_maxlen:
            return True

        # Allow long URLs of any length.
        if any((url in line) for url in ('file://', 'http://', 'https://')):
            return True

        if 'presubmit: ignore-long-line' in line:
            return True

        if line_len > extra_maxlen:
            return False

        if 'url(' in line and file_extension == 'css':
            return True

        if '<include' in line and file_extension in ('css', 'html', 'js'):
            return True

        return input_api.re.match(
            r'.*[A-Za-z][A-Za-z_0-9]{%d,}.*' % long_symbol, line)

    def is_global_pylint_directive(line, pos):
        """True iff the pylint directive starting at line[pos] is global."""
        # Any character before |pos| that is not whitespace or '#' indidcates
        # this is a local directive.
        return not any(c not in " \t#" for c in line[:pos])

    def check_python_long_lines(affected_files, error_formatter):
        errors = []
        global_check_enabled = True

        for f in affected_files:
            file_path = f.LocalPath()
            for idx, line in enumerate(f.NewContents()):
                line_num = idx + 1
                line_is_short = no_long_lines(PY_FILE_EXTS[0], line)

                pos = line.find('pylint: disable=line-too-long')
                if pos >= 0:
                    if is_global_pylint_directive(line, pos):
                        global_check_enabled = False  # Global disable
                    else:
                        continue  # Local disable.

                do_check = global_check_enabled

                pos = line.find('pylint: enable=line-too-long')
                if pos >= 0:
                    if is_global_pylint_directive(line, pos):
                        global_check_enabled = True  # Global enable
                        do_check = True  # Ensure it applies to current line as well.
                    else:
                        do_check = True  # Local enable

                if do_check and not line_is_short:
                    errors.append(error_formatter(file_path, line_num, line))

        return errors

    def format_error(filename, line_num, line):
        return '%s, line %s, %s chars' % (filename, line_num, len(line))

    file_ext_list = list(
        _GenerateAffectedFileExtList(input_api, source_file_filter))

    errors = []

    # For non-Python files, a simple line-based rule check is enough.
    non_py_file_ext_list = [
        x for x in file_ext_list if x[1] not in PY_FILE_EXTS
    ]
    if non_py_file_ext_list:
        errors += _FindNewViolationsOfRuleForList(no_long_lines,
                                                  non_py_file_ext_list,
                                                  error_formatter=format_error)

    # However, Python files need more sophisticated checks that need parsing
    # the whole source file.
    py_file_list = [x[0] for x in file_ext_list if x[1] in PY_FILE_EXTS]
    if py_file_list:
        errors += check_python_long_lines(py_file_list,
                                          error_formatter=format_error)
    if errors:
        msg = 'Found %d lines longer than %s characters (first 5 shown).' % (
            len(errors), maxlen)
        return [output_api.PresubmitPromptWarning(msg, items=errors[:5])]

    return []


def CheckLicense(input_api,
                 output_api,
                 license_re_param=None,
                 project_name=None,
                 source_file_filter=None,
                 accept_empty_files=True):
    """Verifies the license header."""

    # Early-out if the license_re is guaranteed to match everything.
    if license_re_param and license_re_param == '.*':
        return []

    current_year = int(input_api.time.strftime('%Y'))

    if license_re_param:
        new_license_re = license_re = license_re_param
    else:
        project_name = project_name or 'Chromium'

        # Accept any year number from 2006 to the current year, or the special
        # 2006-20xx string used on the oldest files. 2006-20xx is deprecated,
        # but tolerated on old files. On new files the current year must be
        # specified.
        allowed_years = (str(s)
                         for s in reversed(range(2006, current_year + 1)))
        years_re = '(' + '|'.join(
            allowed_years) + '|2006-2008|2006-2009|2006-2010)'

        # Reduce duplication between the two regex expressions.
        key_line = (
            'Use of this source code is governed by a BSD-style license '
            'that can be')
        # The (c) is deprecated, but tolerate it until it's removed from all
        # files. "All rights reserved" is also deprecated, but tolerate it until
        # it's removed from all files.
        license_re = (r'.*? Copyright (\(c\) )?%(year)s The %(project)s Authors'
                      r'(\. All rights reserved\.)?\n'
                      r'.*? %(key_line)s\n'
                      r'.*? found in the LICENSE file\.(?: \*/)?\n') % {
                          'year': years_re,
                          'project': project_name,
                          'key_line': key_line,
                      }
        # On new files don't tolerate any digression from the ideal.
        new_license_re = (r'.*? Copyright %(year)s The %(project)s Authors\n'
                          r'.*? %(key_line)s\n'
                          r'.*? found in the LICENSE file\.(?: \*/)?\n') % {
                              'year': years_re,
                              'project': project_name,
                              'key_line': key_line,
                          }

    license_re = input_api.re.compile(license_re, input_api.re.MULTILINE)
    new_license_re = input_api.re.compile(new_license_re,
                                          input_api.re.MULTILINE)
    bad_files = []
    wrong_year_new_files = []
    bad_new_files = []
    for f in input_api.AffectedSourceFiles(source_file_filter):
        # Only examine the first 1,000 bytes of the file to avoid expensive and
        # fruitless regex searches over long files with no license.
        # re.match would also avoid this but can't be used because some files
        # have a shebang line ahead of the license. The \r\n fixup is because it
        # is possible on Windows to copy/paste the license in such a way that
        # \r\n line endings are inserted. This leads to confusing license error
        # messages - it's better to let the separate \r\n check handle those.
        contents = input_api.ReadFile(f, 'r')[:1000].replace('\r\n', '\n')
        if accept_empty_files and not contents:
            continue
        if f.Action() == 'A':
            # Stricter checking for new files (but might actually be moved).
            match = new_license_re.search(contents)
            if not match:
                # License is totally wrong.
                bad_new_files.append(f.LocalPath())
            elif not license_re_param and match.groups()[0] != str(
                    current_year):
                # If we're using the built-in license_re on a new file then make
                # sure the year is correct.
                wrong_year_new_files.append(f.LocalPath())
        elif not license_re.search(contents):
            bad_files.append(f.LocalPath())
    results = []
    if bad_new_files:
        # We can't distinguish between Google and thirty-party files, so this has to be a
        # warning rather than an error.
        if license_re_param:
            warning_message = ('License on new files must match:\n\n%s\n' %
                               license_re_param)
        else:
            # Verbatim text that can be copy-pasted into new files (possibly
            # adjusting the leading comment delimiter).
            new_license_text = (
                '// Copyright %(year)s The %(project)s Authors\n'
                '// %(key_line)s\n'
                '// found in the LICENSE file.\n') % {
                    'year': current_year,
                    'project': project_name,
                    'key_line': key_line,
                }
            warning_message = (
                'License on new files must be:\n\n%s\n' % new_license_text +
                '(adjusting the comment delimiter accordingly).\n\n' +
                'If this is a moved file, then update the license but do not ' +
                'update the year.\n\n' +
                'If this is a third-party file then ignore this warning.\n\n')
        warning_message += 'Found a bad license header in these new or moved files:'
        results.append(
            output_api.PresubmitPromptWarning(warning_message,
                                              items=bad_new_files))
    if wrong_year_new_files:
        # We can't distinguish between new and moved files, so this has to be a
        # warning rather than an error.
        results.append(
            output_api.PresubmitPromptWarning(
                'License doesn\'t list the current year. If this is a new file, '
                'use the current year. If this is a moved file then ignore this '
                'warning.',
                items=wrong_year_new_files))
    if bad_files:
        results.append(
            output_api.PresubmitPromptWarning(
                'License must match:\n%s\n' % license_re.pattern +
                'Found a bad license header in these files:',
                items=bad_files))
    return results


def CheckChromiumDependencyMetadata(input_api, output_api, file_filter=None):
    """Check files for Chromium third party dependency metadata have sufficient
    information, and are correctly formatted.

    See the README.chromium.template at
    https://chromium.googlesource.com/chromium/src/+/main/third_party/README.chromium.template
    """
    # If the file filter is unspecified, filter to known Chromium metadata
    # files.
    if file_filter is None:
        file_filter = lambda f: metadata.discover.is_metadata_file(f.LocalPath(
        ))

    # The repo's root directory is required to check license files.
    repo_root_dir = input_api.change.RepositoryRoot()

    outputs = []
    for f in input_api.AffectedFiles(file_filter=file_filter):
        if f.Action() == 'D':
            # No need to validate a deleted file.
            continue

        errors, warnings = metadata.validate.check_file(
            filepath=f.AbsoluteLocalPath(),
            repo_root_dir=repo_root_dir,
            reader=input_api.ReadFile,
            is_open_source_project=True,
        )

        for warning in warnings:
            outputs.append(output_api.PresubmitPromptWarning(warning, [f]))

        for error in errors:
            outputs.append(output_api.PresubmitError(error, [f]))

    return outputs


### Other checks


_IGNORE_FREEZE_FOOTER = 'Ignore-Freeze'

_FREEZE_TZ = datetime.timezone(-datetime.timedelta(hours=8), 'PST')
_CHROMIUM_FREEZE_START = datetime.datetime(
    2024, 12, 20, 17, 1, tzinfo=_FREEZE_TZ)
_CHROMIUM_FREEZE_END = datetime.datetime(2025, 1, 5, 17, 0, tzinfo=_FREEZE_TZ)
_CHROMIUM_FREEZE_DETAILS = 'Holiday freeze'

def CheckInfraFreeze(input_api,
                     output_api,
                     files_to_include=None,
                     files_to_exclude=None):
    return CheckChromiumInfraFreeze(input_api, output_api, files_to_include,
                                    files_to_exclude)

def CheckChromiumInfraFreeze(input_api,
                             output_api,
                             files_to_include=None,
                             files_to_exclude=None):
    """Prevent modifications during infra code freeze.

    Args:
        input_api: The InputApi.
        output_api: The OutputApi.
        files_to_include: A list of regexes identifying the files to
            include in the freeze if not matched by a regex in
            files_to_exclude. The regexes will be matched against the
            paths of the affected files under the directory containing
            the PRESUBMIT.py relative to the client root, using / as the
            path separator. If not provided, all files under the
            directory containing the PRESUBMIT.py will be included.
        files_to_exclude: A list of regexes identifying the files to
            exclude from the freeze. The regexes will be matched against
            the paths of the affected files under the directory
            containing the PRESUBMIT.py relative to the client root,
            using / as the path separator. If not provided, no files
            will be excluded.
    """
    # Not in the freeze time range
    now = datetime.datetime.now(_FREEZE_TZ)
    if now < _CHROMIUM_FREEZE_START or now >= _CHROMIUM_FREEZE_END:
        input_api.logging.info('No freeze is in effect')
        return []

    # The CL is ignoring the freeze
    if _IGNORE_FREEZE_FOOTER in input_api.change.GitFootersFromDescription():
        input_api.logging.info('Freeze is being ignored')
        return []

    def file_filter(affected_file):
        files_to_check = files_to_include or ['.*']
        files_to_skip = files_to_exclude or []
        return input_api.FilterSourceFile(affected_file,
                                          files_to_check=files_to_check,
                                          files_to_skip=files_to_skip)

    # Compute the affected files that are covered by the freeze
    files = [
        af.LocalPath()
        for af in input_api.AffectedFiles(file_filter=file_filter)
    ]

    # The Cl does not touch ny files covered by the freeze
    if not files:
        input_api.logging.info('No affected files are covered by freeze')
        return []

    # Don't report errors when on the presubmit --all bot or when testing with
    # presubmit --files.
    if input_api.no_diffs:
        report_type = output_api.PresubmitPromptWarning
    else:
        report_type = output_api.PresubmitError
    return [
        report_type(
            'There is a prod infra freeze in effect from {} until {}:\n'
            '\t{}\n\n'
            'The following files cannot be modified:\n  {}.\n\n'
            'Add "{}: <reason>" to the end of your commit message to override.'.
            format(_CHROMIUM_FREEZE_START, _CHROMIUM_FREEZE_END,
                   _CHROMIUM_FREEZE_DETAILS, '\n  '.join(files),
                   _IGNORE_FREEZE_FOOTER))
    ]


def CheckDoNotSubmit(input_api, output_api):
    return (CheckDoNotSubmitInDescription(input_api, output_api) +
            CheckDoNotSubmitInFiles(input_api, output_api))


def CheckTreeIsOpen(input_api,
                    output_api,
                    url=None,
                    closed=None,
                    json_url=None):
    """Check whether to allow commit without prompt.

    Supports two styles:
        1. Checks that an url's content doesn't match a regexp that would mean that
            the tree is closed. (old)
        2. Check the json_url to decide whether to allow commit without prompt.
    Args:
        input_api: input related apis.
        output_api: output related apis.
        url: url to use for regex based tree status.
        closed: regex to match for closed status.
        json_url: url to download json style status.
    """
    if not input_api.is_committing or \
        'PRESUBMIT_SKIP_NETWORK' in _os.environ:
        return []
    try:
        if json_url:
            connection = input_api.urllib_request.urlopen(json_url)
            status = input_api.json.loads(connection.read())
            connection.close()
            if not status['can_commit_freely']:
                short_text = 'Tree state is: ' + status['general_state']
                long_text = status['message'] + '\n' + json_url
                if input_api.no_diffs:
                    return [
                        output_api.PresubmitPromptWarning(short_text,
                                                          long_text=long_text)
                    ]
                return [
                    output_api.PresubmitError(short_text, long_text=long_text)
                ]
        else:
            # TODO(bradnelson): drop this once all users are gone.
            connection = input_api.urllib_request.urlopen(url)
            status = connection.read()
            connection.close()
            if input_api.re.match(closed, status):
                long_text = status + '\n' + url
                return [
                    output_api.PresubmitError('The tree is closed.',
                                              long_text=long_text)
                ]
    except IOError as e:
        return [
            output_api.PresubmitError('Error fetching tree status.',
                                      long_text=str(e))
        ]
    return []


def GetUnitTestsInDirectory(input_api,
                            output_api,
                            directory,
                            files_to_check=None,
                            files_to_skip=None,
                            env=None,
                            run_on_python2=False,
                            run_on_python3=True,
                            skip_shebang_check=True):
    """Lists all files in a directory and runs them. Doesn't recurse.

    It's mainly a wrapper for RunUnitTests. Use files_to_check and files_to_skip
    to filter tests accordingly. run_on_python2, run_on_python3, and
    skip_shebang_check are no longer used but have to be retained because of the
    many callers in other repos that pass them in.
    """
    del run_on_python2
    del run_on_python3
    del skip_shebang_check

    unit_tests = []
    test_path = input_api.os_path.abspath(
        input_api.os_path.join(input_api.PresubmitLocalPath(), directory))

    def check(filename, filters):
        return any(True for i in filters if input_api.re.match(i, filename))

    to_run = found = 0
    for filename in input_api.os_listdir(test_path):
        found += 1
        fullpath = input_api.os_path.join(test_path, filename)
        if not input_api.os_path.isfile(fullpath):
            continue
        if files_to_check and not check(filename, files_to_check):
            continue
        if files_to_skip and check(filename, files_to_skip):
            continue
        unit_tests.append(input_api.os_path.join(directory, filename))
        to_run += 1
    input_api.logging.debug('Found %d files, running %d unit tests' %
                            (found, to_run))
    if not to_run:
        return [
            output_api.PresubmitPromptWarning(
                'Out of %d files, found none that matched c=%r, s=%r in directory %s'
                % (found, files_to_check, files_to_skip, directory))
        ]
    return GetUnitTests(input_api, output_api, unit_tests, env)


def GetUnitTests(input_api,
                 output_api,
                 unit_tests,
                 env=None,
                 run_on_python2=False,
                 run_on_python3=True,
                 skip_shebang_check=True):
    """Runs all unit tests in a directory.

    On Windows, sys.executable is used for unit tests ending with ".py".
    run_on_python2, run_on_python3, and skip_shebang_check are no longer used but
    have to be retained because of the many callers in other repos that pass them
    in.
    """
    del run_on_python2
    del run_on_python3
    del skip_shebang_check

    # We don't want to hinder users from uploading incomplete patches, but we do
    # want to report errors as errors when doing presubmit --all testing.
    if input_api.is_committing or input_api.no_diffs:
        message_type = output_api.PresubmitError
    else:
        message_type = output_api.PresubmitPromptWarning

    results = []
    for unit_test in unit_tests:
        cmd = [unit_test]
        if input_api.verbose:
            cmd.append('--verbose')
        kwargs = {'cwd': input_api.PresubmitLocalPath()}
        if env:
            kwargs['env'] = env
        if not unit_test.endswith('.py'):
            results.append(
                input_api.Command(name=unit_test,
                                  cmd=cmd,
                                  kwargs=kwargs,
                                  message=message_type))
        else:
            results.append(
                input_api.Command(name=unit_test,
                                  cmd=cmd,
                                  kwargs=kwargs,
                                  message=message_type))
    return results


def GetUnitTestsRecursively(input_api,
                            output_api,
                            directory,
                            files_to_check,
                            files_to_skip,
                            run_on_python2=False,
                            run_on_python3=True,
                            skip_shebang_check=True):
    """Gets all files in the directory tree (git repo) that match files_to_check.

    Restricts itself to only find files within the Change's source repo, not
    dependencies. run_on_python2, run_on_python3, and skip_shebang_check are no
    longer used but have to be retained because of the many callers in other repos
    that pass them in.
    """
    del run_on_python2
    del run_on_python3
    del skip_shebang_check

    def check(filename):
        return (any(input_api.re.match(f, filename) for f in files_to_check) and
                not any(input_api.re.match(f, filename) for f in files_to_skip))

    tests = []

    to_run = found = 0
    for filepath in input_api.change.AllFiles(directory):
        found += 1
        if check(filepath):
            to_run += 1
            tests.append(filepath)
    input_api.logging.debug('Found %d files, running %d' % (found, to_run))
    if not to_run:
        return [
            output_api.PresubmitPromptWarning(
                'Out of %d files, found none that matched c=%r, s=%r in directory %s'
                % (found, files_to_check, files_to_skip, directory))
        ]

    return GetUnitTests(input_api, output_api, tests)


def GetPythonUnitTests(input_api, output_api, unit_tests, python3=False):
    """Run the unit tests out of process, capture the output and use the result
    code to determine success.

    DEPRECATED.
    """
    # We don't want to hinder users from uploading incomplete patches.
    if input_api.is_committing or input_api.no_diffs:
        message_type = output_api.PresubmitError
    else:
        message_type = output_api.PresubmitNotifyResult
    results = []
    for unit_test in unit_tests:
        # Run the unit tests out of process. This is because some unit tests
        # stub out base libraries and don't clean up their mess. It's too easy
        # to get subtle bugs.
        cwd = None
        env = None
        unit_test_name = unit_test
        # 'python -m test.unit_test' doesn't work. We need to change to the
        # right directory instead.
        if '.' in unit_test:
            # Tests imported in submodules (subdirectories) assume that the
            # current directory is in the PYTHONPATH. Manually fix that.
            unit_test = unit_test.replace('.', '/')
            cwd = input_api.os_path.dirname(unit_test)
            unit_test = input_api.os_path.basename(unit_test)
            env = input_api.environ.copy()
            # At least on Windows, it seems '.' must explicitly be in PYTHONPATH
            backpath = [
                '.',
                input_api.os_path.pathsep.join(['..'] * (cwd.count('/') + 1))
            ]
            if env.get('PYTHONPATH'):
                backpath.append(env.get('PYTHONPATH'))
            env['PYTHONPATH'] = input_api.os_path.pathsep.join((backpath))
            env.pop('VPYTHON_CLEAR_PYTHONPATH', None)
        cmd = [input_api.python3_executable, '-m', '%s' % unit_test]
        results.append(
            input_api.Command(name=unit_test_name,
                              cmd=cmd,
                              kwargs={
                                  'env': env,
                                  'cwd': cwd
                              },
                              message=message_type))
    return results


def RunUnitTestsInDirectory(input_api, *args, **kwargs):
    """Run tests in a directory serially.

    For better performance, use GetUnitTestsInDirectory and then
    pass to input_api.RunTests.
    """
    return input_api.RunTests(
        GetUnitTestsInDirectory(input_api, *args, **kwargs), False)


def RunUnitTests(input_api, *args, **kwargs):
    """Run tests serially.

    For better performance, use GetUnitTests and then pass to
    input_api.RunTests.
    """
    return input_api.RunTests(GetUnitTests(input_api, *args, **kwargs), False)


def RunPythonUnitTests(input_api, *args, **kwargs):
    """Run python tests in a directory serially.

    DEPRECATED
    """
    return input_api.RunTests(GetPythonUnitTests(input_api, *args, **kwargs),
                              False)


def _FetchAllFiles(input_api, files_to_check, files_to_skip):
    """Hack to fetch all files."""

    # We cannot use AffectedFiles here because we want to test every python
    # file on each single python change. It's because a change in a python file
    # can break another unmodified file.
    # Use code similar to InputApi.FilterSourceFile()
    def Find(filepath, filters):
        if input_api.platform == 'win32':
            filepath = filepath.replace('\\', '/')

        for item in filters:
            if input_api.re.match(item, filepath):
                return True
        return False

    files = []
    path_len = len(input_api.PresubmitLocalPath())
    for dirpath, dirnames, filenames in input_api.os_walk(
            input_api.PresubmitLocalPath()):
        # Passes dirnames in block list to speed up search.
        for item in dirnames[:]:
            filepath = input_api.os_path.join(dirpath, item)[path_len + 1:]
            if Find(filepath, files_to_skip):
                dirnames.remove(item)
        for item in filenames:
            filepath = input_api.os_path.join(dirpath, item)[path_len + 1:]
            if Find(filepath,
                    files_to_check) and not Find(filepath, files_to_skip):
                files.append(filepath)
    return files


def GetPylint(input_api,
              output_api,
              files_to_check=None,
              files_to_skip=None,
              disabled_warnings=None,
              extra_paths_list=None,
              pylintrc=None,
              version='2.7'):
    """Run pylint on python files.

    The default files_to_check enforces looking only at *.py files.

    Currently only pylint version '2.6' and '2.7' are supported.
    """

    files_to_check = tuple(files_to_check or (r'.*\.py$', ))
    files_to_skip = tuple(files_to_skip or input_api.DEFAULT_FILES_TO_SKIP)
    extra_paths_list = extra_paths_list or []

    assert version in ('2.6', '2.7', '2.17', '3.2'), \
        'Unsupported pylint version: %s' % version

    if input_api.is_committing or input_api.no_diffs:
        error_type = output_api.PresubmitError
    else:
        error_type = output_api.PresubmitPromptWarning

    # Only trigger if there is at least one python file affected.
    def rel_path(regex):
        """Modifies a regex for a subject to accept paths relative to root."""
        def samefile(a, b):
            # Default implementation for platforms lacking os.path.samefile
            # (like Windows).
            return input_api.os_path.abspath(a) == input_api.os_path.abspath(b)

        samefile = getattr(input_api.os_path, 'samefile', samefile)
        if samefile(input_api.PresubmitLocalPath(),
                    input_api.change.RepositoryRoot()):
            return regex

        prefix = input_api.os_path.join(
            input_api.os_path.relpath(input_api.PresubmitLocalPath(),
                                      input_api.change.RepositoryRoot()), '')
        return input_api.re.escape(prefix) + regex

    src_filter = lambda x: input_api.FilterSourceFile(
        x, map(rel_path, files_to_check), map(rel_path, files_to_skip))
    if not input_api.AffectedSourceFiles(src_filter):
        input_api.logging.info('Skipping pylint: no matching changes.')
        return []

    if pylintrc is not None:
        pylintrc = input_api.os_path.join(input_api.PresubmitLocalPath(),
                                          pylintrc)
    else:
        pylintrc = input_api.os_path.join(_HERE, 'pylintrc')
        if input_api.os_path.exists(f'{pylintrc}-{version}'):
            pylintrc += f'-{version}'
    extra_args = ['--rcfile=%s' % pylintrc]
    if disabled_warnings:
        extra_args.extend(['-d', ','.join(disabled_warnings)])

    files = _FetchAllFiles(input_api, files_to_check, files_to_skip)
    if not files:
        return []
    files.sort()

    input_api.logging.info('Running pylint %s on %d files', version, len(files))
    input_api.logging.debug('Running pylint on: %s', files)
    env = input_api.environ.copy()
    env['PYTHONPATH'] = input_api.os_path.pathsep.join(extra_paths_list)
    env.pop('VPYTHON_CLEAR_PYTHONPATH', None)
    input_api.logging.debug('  with extra PYTHONPATH: %r', extra_paths_list)
    files_per_job = 10

    def GetPylintCmd(flist, extra, parallel):
        # Windows needs help running python files so we explicitly specify
        # the interpreter to use. It also has limitations on the size of
        # the command-line, so we pass arguments via a pipe.
        tool = input_api.os_path.join(_HERE, 'pylint-' + version)
        kwargs = {'env': env}
        if input_api.platform == 'win32':
            # On Windows, scripts on the current directory take precedence over
            # PATH. When `pylint.bat` calls `vpython3`, it will execute the
            # `vpython3` of the depot_tools under test instead of the one in the
            # bot. As a workaround, we run the tests from the parent directory
            # instead.
            cwd = input_api.change.RepositoryRoot()
            if input_api.os_path.basename(cwd) == 'depot_tools':
                kwargs['cwd'] = input_api.os_path.dirname(cwd)
                flist = [
                    input_api.os_path.join('depot_tools', f) for f in flist
                ]
            tool += '.bat'

        cmd = [tool, '--args-on-stdin']
        if len(flist) == 1:
            description = flist[0]
        else:
            description = '%s files' % len(flist)

        args = extra_args[:]
        if extra:
            args.extend(extra)
            description += ' using %s' % (extra, )
        if parallel:
            # Make sure we don't request more parallelism than is justified for
            # the number of files we have to process. PyLint child-process
            # startup time is significant.
            jobs = min(input_api.cpu_count, 1 + len(flist) // files_per_job)
            if jobs > 1:
                args.append('--jobs=%s' % jobs)
                description += ' on %d processes' % jobs

        kwargs['stdin'] = '\n'.join(args + flist).encode('utf-8')

        return input_api.Command(name='Pylint (%s)' % description,
                                 cmd=cmd,
                                 kwargs=kwargs,
                                 message=error_type,
                                 python3=True)

    # pylint's cycle detection doesn't work in parallel, so spawn a second,
    # single-threaded job for just that check. However, only do this if there
    # are actually enough files to process to justify parallelism in the first
    # place. Some PRESUBMITs explicitly mention cycle detection.
    if len(files) >= files_per_job and not any(
            'R0401' in a or 'cyclic-import' in a for a in extra_args):
        return [
            GetPylintCmd(files, ["--disable=cyclic-import"], True),
            GetPylintCmd(files, ["--disable=all", "--enable=cyclic-import"],
                         False),
        ]

    return [
        GetPylintCmd(files, [], True),
    ]


def RunPylint(input_api, *args, **kwargs):
    """Legacy presubmit function.

    For better performance, get all tests and then pass to
    input_api.RunTests.
    """
    return input_api.RunTests(GetPylint(input_api, *args, **kwargs), False)


def CheckDirMetadataFormat(input_api, output_api, dirmd_bin=None):
    # TODO(crbug.com/1102997): Remove OWNERS once DIR_METADATA migration is
    # complete.
    file_filter = lambda f: (input_api.basename(f.LocalPath()) in
                             ('DIR_METADATA', 'OWNERS'))
    affected_files = {
        f.AbsoluteLocalPath()
        for f in input_api.change.AffectedFiles(include_deletes=False,
                                                file_filter=file_filter)
    }
    if not affected_files:
        return []

    name = 'Validate metadata in OWNERS and DIR_METADATA files'

    if dirmd_bin is None:
        dirmd_bin = 'dirmd.bat' if input_api.is_windows else 'dirmd'

    # When running git cl presubmit --all this presubmit may be asked to check
    # ~7,500 files, leading to a command line that is about 500,000 characters.
    # This goes past the Windows 8191 character cmd.exe limit and causes cryptic
    # failures. To avoid these we break the command up into smaller pieces. The
    # non-Windows limit is chosen so that the code that splits up commands will
    # get some exercise on other platforms.
    # Depending on how long the command is on Windows the error may be:
    #     The command line is too long.
    # Or it may be:
    #     OSError: Execution failed with error: [WinError 206] The filename or
    #     extension is too long.
    # I suspect that the latter error comes from CreateProcess hitting its 32768
    # character limit.
    files_per_command = 50 if input_api.is_windows else 1000
    affected_files = sorted(affected_files)
    results = []
    for i in range(0, len(affected_files), files_per_command):
        kwargs = {}
        cmd = [dirmd_bin, 'validate'] + affected_files[i:i + files_per_command]
        results.extend(
            [input_api.Command(name, cmd, kwargs, output_api.PresubmitError)])
    return results


def CheckNoNewMetadataInOwners(input_api, output_api):
    """Check that no metadata is added to OWNERS files."""
    if input_api.no_diffs:
        return []

    _METADATA_LINE_RE = input_api.re.compile(
        r'^#\s*(TEAM|COMPONENT|OS|WPT-NOTIFY)+\s*:\s*\S+$',
        input_api.re.MULTILINE | input_api.re.IGNORECASE)
    affected_files = input_api.change.AffectedFiles(
        include_deletes=False,
        file_filter=lambda f: input_api.basename(f.LocalPath()) == 'OWNERS')

    errors = []
    for f in affected_files:
        for _, line in f.ChangedContents():
            if _METADATA_LINE_RE.search(line):
                errors.append(f.AbsoluteLocalPath())
                break

    if not errors:
        return []

    return [
        output_api.PresubmitError(
            'New metadata was added to the following OWNERS files, but should '
            'have been added to DIR_METADATA files instead:\n' +
            '\n'.join(errors) + '\n' +
            'See https://source.chromium.org/chromium/infra/infra/+/HEAD:'
            'go/src/infra/tools/dirmd/proto/dir_metadata.proto for details.')
    ]


def CheckOwnersDirMetadataExclusive(input_api, output_api):
    """Check that metadata in OWNERS files and DIR_METADATA files are mutually
    exclusive.
    """
    _METADATA_LINE_RE = input_api.re.compile(
        r'^#\s*(TEAM|COMPONENT|OS|WPT-NOTIFY)+\s*:\s*\S+$',
        input_api.re.MULTILINE)
    file_filter = (lambda f: input_api.basename(f.LocalPath()) in
                   ('OWNERS', 'DIR_METADATA'))
    affected_dirs = {
        input_api.os_path.dirname(f.AbsoluteLocalPath())
        for f in input_api.change.AffectedFiles(include_deletes=False,
                                                file_filter=file_filter)
    }

    errors = []
    for path in affected_dirs:
        owners_path = input_api.os_path.join(path, 'OWNERS')
        dir_metadata_path = input_api.os_path.join(path, 'DIR_METADATA')
        if (not input_api.os_path.isfile(dir_metadata_path)
                or not input_api.os_path.isfile(owners_path)):
            continue
        if _METADATA_LINE_RE.search(input_api.ReadFile(owners_path)):
            errors.append(owners_path)

    if not errors:
        return []

    return [
        output_api.PresubmitError(
            'The following OWNERS files should contain no metadata, as there is a '
            'DIR_METADATA file present in the same directory:\n' +
            '\n'.join(errors))
    ]


def CheckOwnersFormat(input_api, output_api):
    if input_api.gerrit and input_api.gerrit.IsCodeOwnersEnabledOnRepo():
        return []

    host = "none"
    project = "none"
    if input_api.gerrit:
        host = input_api.gerrit.host
        project = input_api.gerrit.project
    return [
        output_api.PresubmitError(
            f'code-owners is not enabled on {host}/{project}. '
            'Ask your host enable it on your gerrit '
            'host. Read more about code-owners at '
            'https://chromium-review.googlesource.com/'
            'plugins/code-owners/Documentation/index.html.')
    ]


def CheckOwners(input_api, output_api, source_file_filter=None, allow_tbr=True):
    # Skip OWNERS check when Owners-Override label is approved. This is intended
    # for global owners, trusted bots, and on-call sheriffs. Review is still
    # required for these changes.
    if (input_api.change.issue and input_api.gerrit.IsOwnersOverrideApproved(
            input_api.change.issue)):
        return []

    if input_api.gerrit and input_api.gerrit.IsCodeOwnersEnabledOnRepo():
        return []

    host = "none"
    project = "none"
    if input_api.gerrit:
        host = input_api.gerrit.host
        project = input_api.gerrit.project
    return [
        output_api.PresubmitError(
            f'code-owners is not enabled on {host}/{project}. '
            'Ask your host enable it on your gerrit '
            'host. Read more about code-owners at '
            'https://chromium-review.googlesource.com/'
            'plugins/code-owners/Documentation/index.html.')
    ]


def GetCodereviewOwnerAndReviewers(input_api,
                                   _email_regexp=None,
                                   approval_needed=True):
    """Return the owner and reviewers of a change, if any.

    If approval_needed is True, only reviewers who have approved the change
    will be returned.
    """
    # Recognizes 'X@Y' email addresses. Very simplistic.
    EMAIL_REGEXP = input_api.re.compile(r'^[\w\-\+\%\.]+\@[\w\-\+\%\.]+$')
    issue = input_api.change.issue
    if not issue:
        return None, (set() if approval_needed else _ReviewersFromChange(
            input_api.change))

    owner_email = input_api.gerrit.GetChangeOwner(issue)
    reviewers = set(
        r for r in input_api.gerrit.GetChangeReviewers(issue, approval_needed)
        if _match_reviewer_email(r, owner_email, EMAIL_REGEXP))
    input_api.logging.debug('owner: %s; approvals given by: %s', owner_email,
                            ', '.join(sorted(reviewers)))
    return owner_email, reviewers


def _ReviewersFromChange(change):
    """Return the reviewers specified in the |change|, if any."""
    reviewers = set()
    reviewers.update(change.ReviewersFromDescription())
    reviewers.update(change.TBRsFromDescription())

    # Drop reviewers that aren't specified in email address format.
    return set(reviewer for reviewer in reviewers if '@' in reviewer)


def _match_reviewer_email(r, owner_email, email_regexp):
    return email_regexp.match(r) and r != owner_email


def CheckSingletonInHeaders(input_api, output_api, source_file_filter=None):
    """Deprecated, must be removed."""
    return [
        output_api.PresubmitNotifyResult(
            'CheckSingletonInHeaders is deprecated, please remove it.')
    ]


def PanProjectChecks(input_api,
                     output_api,
                     excluded_paths=None,
                     text_files=None,
                     license_header=None,
                     project_name=None,
                     owners_check=True,
                     maxlen=80,
                     global_checks=True):
    """Checks that ALL chromium orbit projects should use.

    These are checks to be run on all Chromium orbit project, including:
        Chromium
        Native Client
        V8
    When you update this function, please take this broad scope into account.
    Args:
        input_api: Bag of input related interfaces.
        output_api: Bag of output related interfaces.
        excluded_paths: Don't include these paths in common checks.
        text_files: Which file are to be treated as documentation text files.
        license_header: What license header should be on files.
        project_name: What is the name of the project as it appears in the license.
        global_checks: If True run checks that are unaffected by other options or by
            the PRESUBMIT script's location, such as CheckChangeHasDescription.
            global_checks should be passed as False when this function is called from
            locations other than the project's root PRESUBMIT.py, to avoid redundant
            checking.
    Returns:
        A list of warning or error objects.
    """
    excluded_paths = tuple(excluded_paths or [])
    text_files = tuple(text_files or (
        r'.+\.txt$',
        r'.+\.json$',
    ))

    results = []
    # This code loads the default skip list (e.g. third_party, experimental,
    # etc) and add our skip list (breakpad, skia and v8 are still not following
    # google style and are not really living this repository). See
    # presubmit_support.py InputApi.FilterSourceFile for the (simple) usage.
    files_to_skip = input_api.DEFAULT_FILES_TO_SKIP + excluded_paths
    files_to_check = input_api.DEFAULT_FILES_TO_CHECK + text_files
    sources = lambda x: input_api.FilterSourceFile(x,
                                                   files_to_skip=files_to_skip)
    text_files = lambda x: input_api.FilterSourceFile(
        x, files_to_skip=files_to_skip, files_to_check=files_to_check)

    snapshot_memory = []

    def snapshot(msg):
        """Measures & prints performance warning if a rule is running slow."""
        dt2 = input_api.time.time()
        if snapshot_memory:
            delta_s = dt2 - snapshot_memory[0]
            if delta_s > 0.5:
                print("  %s took a long time: %.1fs" %
                      (snapshot_memory[1], delta_s))
        snapshot_memory[:] = (dt2, msg)

    snapshot("checking owners files format")
    try:
        if not 'PRESUBMIT_SKIP_NETWORK' in _os.environ and owners_check:
            snapshot("checking owners")
            results.extend(
                input_api.canned_checks.CheckOwnersFormat(
                    input_api, output_api))
            results.extend(
                input_api.canned_checks.CheckOwners(input_api,
                                                    output_api,
                                                    source_file_filter=None))
    except Exception as e:
        print('Failed to check owners - %s' % str(e))

    snapshot("checking long lines")
    results.extend(
        input_api.canned_checks.CheckLongLines(input_api,
                                               output_api,
                                               maxlen,
                                               source_file_filter=sources))
    snapshot("checking tabs")
    results.extend(
        input_api.canned_checks.CheckChangeHasNoTabs(
            input_api, output_api, source_file_filter=sources))
    snapshot("checking stray whitespace")
    results.extend(
        input_api.canned_checks.CheckChangeHasNoStrayWhitespace(
            input_api, output_api, source_file_filter=sources))
    snapshot("checking license")
    results.extend(
        input_api.canned_checks.CheckLicense(input_api,
                                             output_api,
                                             license_header,
                                             project_name,
                                             source_file_filter=sources))
    snapshot("checking corp links in files")
    results.extend(
        input_api.canned_checks.CheckCorpLinksInFiles(
            input_api, output_api, source_file_filter=sources))
    snapshot("checking large scale change")
    results.extend(
        input_api.canned_checks.CheckLargeScaleChange(input_api, output_api))

    if input_api.is_committing:
        if global_checks:
            # These changes verify state that is global to the tree and can
            # therefore be skipped when run from PRESUBMIT.py scripts deeper in
            # the tree. Skipping these saves a bit of time and avoids having
            # redundant output. This was initially designed for use by
            # third_party/blink/PRESUBMIT.py.
            snapshot("checking was uploaded")
            results.extend(
                input_api.canned_checks.CheckChangeWasUploaded(
                    input_api, output_api))
            snapshot("checking description")
            results.extend(
                input_api.canned_checks.CheckChangeHasDescription(
                    input_api, output_api))
            results.extend(
                input_api.canned_checks.CheckDoNotSubmitInDescription(
                    input_api, output_api))
            results.extend(
                input_api.canned_checks.CheckCorpLinksInDescription(
                    input_api, output_api))
        snapshot("checking do not submit in files")
        results.extend(
            input_api.canned_checks.CheckDoNotSubmitInFiles(
                input_api, output_api))

    if global_checks:
        results.extend(
            input_api.canned_checks.CheckNoNewGitFilesAddedInDependencies(
                input_api, output_api))
        if input_api.change.scm == 'git':
            snapshot("checking for commit objects in tree")
            results.extend(
                input_api.canned_checks.CheckForCommitObjects(
                    input_api, output_api))
            results.extend(
                input_api.canned_checks.CheckForRecursedeps(
                    input_api, output_api))

    snapshot("done")
    return results


def CheckPatchFormatted(input_api,
                        output_api,
                        bypass_warnings=True,
                        check_clang_format=True,
                        check_js=False,
                        check_python=None,
                        result_factory=None):
    result_factory = result_factory or output_api.PresubmitPromptWarning
    import git_cl

    display_args = []
    if not check_clang_format:
        display_args.append('--no-clang-format')

    if check_js:
        display_args.append('--js')

    # Explicitly setting check_python to will enable/disable python formatting
    # on all files. Leaving it as None will enable checking patch formatting
    # on files that have a .style.yapf file in a parent directory.
    if check_python is not None:
        if check_python:
            display_args.append('--python')
        else:
            display_args.append('--no-python')

    cmd = [
        '-C',
        input_api.change.RepositoryRoot(), 'cl', 'format', '--dry-run',
        '--presubmit'
    ] + display_args

    # Make sure the passed --upstream branch is applied to a dry run.
    if input_api.change.UpstreamBranch():
        cmd.extend(['--upstream', input_api.change.UpstreamBranch()])

    presubmit_subdir = input_api.os_path.relpath(
        input_api.PresubmitLocalPath(), input_api.change.RepositoryRoot())
    if presubmit_subdir.startswith('..') or presubmit_subdir == '.':
        presubmit_subdir = ''
    # If the PRESUBMIT.py is in a parent repository, then format the entire
    # subrepository. Otherwise, format only the code in the directory that
    # contains the PRESUBMIT.py.
    if presubmit_subdir:
        cmd.append(input_api.PresubmitLocalPath())

    code, _ = git_cl.RunGitWithCode(cmd, suppress_stderr=bypass_warnings)
    # bypass_warnings? Only fail with code 2.
    # As this is just a warning, ignore all other errors if the user
    # happens to have a broken clang-format, doesn't use git, etc etc.
    if code == 2 or (code and not bypass_warnings):
        if presubmit_subdir:
            short_path = presubmit_subdir
        else:
            short_path = input_api.basename(input_api.change.RepositoryRoot())
        display_args.append(presubmit_subdir)
        return [
            result_factory('The %s directory requires source formatting. '
                           'Please run: git cl format %s' %
                           (short_path, ' '.join(display_args)))
        ]
    return []


def CheckGNFormatted(input_api, output_api):
    import gn
    affected_files = input_api.AffectedFiles(
        include_deletes=False,
        file_filter=lambda x: x.LocalPath().endswith('.gn') or x.LocalPath(
        ).endswith('.gni') or x.LocalPath().endswith('.typemap'))
    warnings = []
    for f in affected_files:
        cmd = ['gn', 'format', '--dry-run', f.AbsoluteLocalPath()]
        rc = gn.main(cmd)
        if rc == 2:
            warnings.append(
                output_api.PresubmitPromptWarning(
                    '%s requires formatting. Please run:\n  gn format %s' %
                    (f.AbsoluteLocalPath(), f.LocalPath())))
    # It's just a warning, so ignore other types of failures assuming they'll be
    # caught elsewhere.
    return warnings


def CheckCIPDManifest(input_api, output_api, path=None, content=None):
    """Verifies that a CIPD ensure file manifest is valid against all platforms.

    Exactly one of "path" or "content" must be provided. An assertion will occur
    if neither or both are provided.

    Args:
        path (str): If provided, the filesystem path to the manifest to verify.
        content (str): If provided, the raw content of the manifest to veirfy.
    """
    cipd_bin = 'cipd' if not input_api.is_windows else 'cipd.bat'
    cmd = [cipd_bin, 'ensure-file-verify']
    kwargs = {}

    if input_api.is_windows:
        # Needs to be able to resolve "cipd.bat".
        kwargs['shell'] = True

    if input_api.verbose:
        cmd += ['-log-level', 'debug']

    if path:
        assert content is None, 'Cannot provide both "path" and "content".'
        cmd += ['-ensure-file', path]
        name = 'Check CIPD manifest %r' % path
    elif content:
        assert path is None, 'Cannot provide both "path" and "content".'
        cmd += ['-ensure-file=-']
        kwargs['stdin'] = content.encode('utf-8')
        # quick and dirty parser to extract checked packages.
        packages = [
            l.split()[0] for l in (ll.strip() for ll in content.splitlines())
            if ' ' in l and not l.startswith('$')
        ]
        name = 'Check CIPD packages from string: %r' % (packages, )
    else:
        raise Exception('Exactly one of "path" or "content" must be provided.')

    return input_api.Command(name, cmd, kwargs, output_api.PresubmitError)


def CheckCIPDPackages(input_api, output_api, platforms, packages):
    """Verifies that all named CIPD packages can be resolved against all supplied
    platforms.

    Args:
        platforms (list): List of CIPD platforms to verify.
        packages (dict): Mapping of package name to version.
    """
    manifest = []
    for p in platforms:
        manifest.append('$VerifiedPlatform %s' % (p, ))
    for k, v in packages.items():
        manifest.append('%s %s' % (k, v))
    return CheckCIPDManifest(input_api, output_api, content='\n'.join(manifest))


def CheckCIPDClientDigests(input_api, output_api, client_version_file):
    """Verifies that *.digests file was correctly regenerated.

    <client_version_file>.digests file contains pinned hashes of the CIPD client.
    It is consulted during CIPD client bootstrap and self-update. It should be
    regenerated each time CIPD client version file changes.

    Args:
        client_version_file (str): Path to a text file with CIPD client version.
    """
    cmd = [
        'cipd' if not input_api.is_windows else 'cipd.bat',
        'selfupdate-roll',
        '-check',
        '-version-file',
        client_version_file,
    ]
    if input_api.verbose:
        cmd += ['-log-level', 'debug']
    return input_api.Command(
        'Check CIPD client_version_file.digests file',
        cmd,
        {'shell': True} if input_api.is_windows else {},  # to resolve cipd.bat
        output_api.PresubmitError)


def CheckForCommitObjects(input_api, output_api):
    """Validates that commit objects match DEPS.

    Commit objects are put into the git tree typically by submodule tooling.
    Because we use gclient to handle external repository references instead,
    we want to ensure DEPS content and Git are in sync when desired.

    Args:
        input_api: Bag of input related interfaces.
        output_api: Bag of output related interfaces.

    Returns:
        A presubmit error if a commit object is not expected.
    """
    if input_api.change.scm != 'git':
        return [
            output_api.PresubmitNotifyResult(
                'Non-git workspace detected, skipping CheckForCommitObjects.')
        ]

    # Get DEPS file.
    try:
        deps_content = input_api.subprocess.check_output(
            ['git', 'show', 'HEAD:DEPS'], cwd=input_api.PresubmitLocalPath())
        deps = _ParseDeps(deps_content)
    except Exception:
        # No DEPS file, so skip this check.
        return []

    # set default
    if 'deps' not in deps:
        deps['deps'] = {}
    if 'git_dependencies' not in deps:
        deps['git_dependencies'] = 'DEPS'

    if deps['git_dependencies'] == 'SUBMODULES':
        # git submodule is source of truth, so no further action needed.
        return []

    def parse_tree_entry(ent):
        """Splits a tree entry into components

        Args:
            ent: a tree entry in the form "filemode type hash\tname"

        Returns:
            The tree entry split into component parts
        """
        tabparts = ent.split('\t', 1)
        spaceparts = tabparts[0].split(' ', 2)
        return (spaceparts[0], spaceparts[1], spaceparts[2], tabparts[1])

    full_tree = input_api.subprocess.check_output(
        ['git', 'ls-tree', '-r', '--full-tree', '-z', 'HEAD'],
        cwd=input_api.PresubmitLocalPath())

    # commit_tree_entries holds all commit entries (ie gitlink, submodule
    # record).
    commit_tree_entries = []
    for entry in full_tree.strip().split(b'\00'):
        if not entry.startswith(b'160000'):
            # Remove entries that we don't care about. 160000 indicates a
            # gitlink.
            continue
        tree_entry = parse_tree_entry(entry.decode('utf-8'))
        if tree_entry[1] == 'commit':
            commit_tree_entries.append(tree_entry)

    # No gitlinks found, return early.
    if len(commit_tree_entries) == 0:
        return []

    if deps['git_dependencies'] == 'DEPS':
        commit_tree_entries = [x[3] for x in commit_tree_entries]
        return [
            output_api.PresubmitError(
                'Commit objects present within tree.\n'
                'This may be due to submodule-related interactions;\n'
                'the presence of a commit object in the tree may lead to odd\n'
                'situations where files are inconsistently checked-out.\n'
                'Remove these commit entries and validate your changeset '
                'again:\n', commit_tree_entries)
        ]

    assert deps['git_dependencies'] == 'SYNC', 'unexpected git_dependencies.'

    # Create mapping HASH -> PATH
    git_submodules = {}
    for commit_tree_entry in commit_tree_entries:
        git_submodules[commit_tree_entry[2]] = commit_tree_entry[3]

    gitmodules_file = input_api.os_path.join(input_api.PresubmitLocalPath(),
                                             '.gitmodules')
    with open(gitmodules_file) as f:
        gitmodules_content = f.read()

    mismatch_entries = []
    deps_msg = ""
    for dep_path, dep in deps['deps'].items():
        if 'dep_type' in dep and dep['dep_type'] != 'git':
            continue

        url = dep if isinstance(dep, str) else dep['url']
        commit_hash = url.split('@')[-1]
        # Two exceptions were in made in two projects prior to this check
        # enforcement. We need to address those exceptions, but in the meantime
        # we can't fail this global presubmit check
        # https://chromium.googlesource.com/infra/infra/+/refs/heads/main/DEPS#45
        if dep_path == 'recipes-py' and commit_hash == 'refs/heads/main':
            continue

        # https://chromium.googlesource.com/angle/angle/+/refs/heads/main/DEPS#412
        if dep_path == 'third_party/dummy_chromium':
            continue

        if commit_hash in git_submodules:
            submodule_path = git_submodules.pop(commit_hash)
            if not dep_path.endswith(submodule_path):
                # DEPS entry path doesn't point to a gitlink.
                return [
                    output_api.PresubmitError(
                        f'Unexpected DEPS entry {dep_path}.\n'
                        f'Expected path to end with {submodule_path}.\n'
                        'Make sure DEPS paths match those in .gitmodules \n'
                        f'and a gitlink exists at {dep_path}.')
                ]
            if f'path = {submodule_path}' not in gitmodules_content:
                return [
                    output_api.PresubmitError(
                        f'No submodule with path {submodule_path} in '
                        '.gitmodules.\nMake sure entries in .gitmodules match '
                        'gitlink locations in the tree.')
                ]
        else:
            mismatch_entries.append(dep_path)
            deps_msg += f"\n [DEPS]      {dep_path} -> {commit_hash}"

    for commit_hash, path in git_submodules.items():
        mismatch_entries.append(path)
        deps_msg += f"\n [gitlink]   {path} -> {commit_hash}"

    if mismatch_entries:
        return [
            output_api.PresubmitError(
                'DEPS file indicates git submodule migration is in progress,\n'
                'but the commit objects do not match DEPS entries.\n\n'
                'To reset all git submodule git entries to match DEPS, run\n'
                'the following command in the root of this repository:\n'
                '    gclient gitmodules\n'
                'This will update AND stage the entries to the correct revisions.\n'
                'Then commit these staged changes only (`git commit` without -a).\n'
                '\n\n'
                'The following entries diverged: ' + deps_msg)
        ]

    return []


def CheckForRecursedeps(input_api, output_api):
    """Checks that DEPS entries in recursedeps exist in deps."""
    # Run only if DEPS has been modified.
    if all(f.LocalPath() != 'DEPS' for f in input_api.AffectedFiles()):
        return []
    # Get DEPS file.
    deps_file = input_api.os_path.join(input_api.PresubmitLocalPath(), 'DEPS')
    if not input_api.os_path.isfile(deps_file):
        # No DEPS file, carry on!
        return []

    with open(deps_file) as f:
        deps_content = f.read()
    deps = _ParseDeps(deps_content)

    if 'recursedeps' not in deps:
        # No recursedeps entry, carry on!
        return []

    existing_deps = deps.get('deps', {})

    errors = []
    for check_dep in deps['recursedeps']:
        if check_dep not in existing_deps:
            errors.append(
                output_api.PresubmitError(
                    f'Found recuredep entry {check_dep} but it is not found '
                    'in\n deps itself. Remove it from recurcedeps or add '
                    'deps entry.'))
    return errors


def _readDeps(input_api):
    """Read DEPS file from the checkout disk. Extracted for testability."""
    deps_file = input_api.os_path.join(input_api.PresubmitLocalPath(), 'DEPS')
    with open(deps_file) as f:
        return f.read()


def CheckNoNewGitFilesAddedInDependencies(input_api, output_api):
    """Check if there are Git files in any DEPS dependencies. Error is returned
    if there are."""
    try:
        deps = _ParseDeps(_readDeps(input_api))
    except FileNotFoundError:
        # If there's no DEPS file, there is nothing to check.
        return []

    dependency_paths = set()
    for path, dep in deps.get('deps', {}).items():
        if 'condition' in dep and 'non_git_source' in dep['condition']:
            # TODO(crbug.com/40738689): Remove src/ prefix
            dependency_paths.add(path[4:])  # 4 == len('src/')

    errors = []
    for file in input_api.AffectedFiles(include_deletes=False):
        path = file.LocalPath()
        # We are checking path, and all paths below up to root. E.g. if path is
        # a/b/c, we start with path == "a/b/c", followed by "a/b" and "a".
        while path:
            if path in dependency_paths:
                errors.append(
                    output_api.PresubmitError(
                        'You cannot place files tracked by Git inside a '
                        'first-party DEPS dependency (deps).\n'
                        f'Dependency: {path}\n'
                        f'File: {file.LocalPath()}'))
            path = _os.path.dirname(path)

    return errors


@functools.lru_cache(maxsize=None)
def _ParseDeps(contents):
    """Simple helper for parsing DEPS files."""

    # Stubs for handling special syntax in the root DEPS file.
    class _VarImpl:
        def __init__(self, local_scope):
            self._local_scope = local_scope

        def Lookup(self, var_name):
            """Implements the Var syntax."""
            try:
                return self._local_scope['vars'][var_name]
            except KeyError:
                raise Exception('Var is not defined: %s' % var_name)

    local_scope = {}
    global_scope = {
        'Var': _VarImpl(local_scope).Lookup,
        'Str': str,
    }

    exec(contents, global_scope, local_scope)
    return local_scope


def CheckVPythonSpec(input_api, output_api, file_filter=None):
    """Validates any changed .vpython and .vpython3 files with vpython
    verification tool.

    Args:
        input_api: Bag of input related interfaces.
        output_api: Bag of output related interfaces.
        file_filter: Custom function that takes a path (relative to client root) and
            returns boolean, which is used to filter files for which to apply the
            verification to. Defaults to any path ending with .vpython(3), which captures
            both global .vpython(3) and <script>.vpython(3) files.

    Returns:
        A list of input_api.Command objects containing verification commands.
    """
    file_filter = file_filter or (lambda f: f.LocalPath().endswith('.vpython')
                                  or f.LocalPath().endswith('.vpython3'))
    affected_files = input_api.AffectedTestableFiles(file_filter=file_filter)
    affected_files = map(lambda f: f.AbsoluteLocalPath(), affected_files)

    commands = []
    for f in affected_files:
        commands.append(
            input_api.Command('Verify %s' % f, [
                input_api.python3_executable, '-vpython-spec', f,
                '-vpython-tool', 'verify'
            ], {'stderr': input_api.subprocess.STDOUT},
                              output_api.PresubmitError))

    return commands


def CheckChangedLUCIConfigs(input_api, output_api):
    """Validates the changed config file against LUCI Config.

    Only return the warning and/or error for files in input_api.AffectedFiles().

    Assumes `lucicfg` binary is in PATH and the user is logged in.

    Returns:
        A list presubmit errors and/or warnings from the validation result of files
        in input_api.AffectedFiles()
    """
    import json
    import logging

    import auth
    import git_cl
    LUCI_CONFIG_HOST_NAME = 'config.luci.app'

    cl = git_cl.Changelist()
    if input_api.gerrit:
        if input_api.change.issue:
            remote_branch = input_api.gerrit.GetDestRef(input_api.change.issue)
        else:
            remote_branch = input_api.gerrit.branch
    else:
        remote, remote_branch = cl.GetRemoteBranch()
        if remote_branch.startswith('refs/remotes/%s/' % remote):
            remote_branch = remote_branch.replace('refs/remotes/%s/' % remote,
                                                  'refs/heads/', 1)
        if remote_branch.startswith('refs/remotes/branch-heads/'):
            remote_branch = remote_branch.replace('refs/remotes/branch-heads/',
                                                  'refs/branch-heads/', 1)

    if input_api.gerrit:
        host = input_api.gerrit.host
        project = input_api.gerrit.project
        gerrit_url = f'https://{host}/{project}'
        remote_host_url = gerrit_url.replace('-review.googlesource',
                                             '.googlesource')
    else:
        remote_host_url = cl.GetRemoteUrl()
    if not remote_host_url:
        return [
            output_api.PresubmitError(
                'Remote host url for git has not been defined')
        ]
    remote_host_url = remote_host_url.rstrip('/')
    if remote_host_url.endswith('.git'):
        remote_host_url = remote_host_url[:-len('.git')]

    # authentication
    try:
        acc_tkn = auth.Authenticator(audience='https://%s' %
                                     LUCI_CONFIG_HOST_NAME).get_id_token()
    except auth.LoginRequiredError as e:
        return [
            output_api.PresubmitError('Error in authenticating user.',
                                      long_text=str(e))
        ]

    def request(method, body, max_attempts=3):
        """"Calls luci-config v2 method and returns a parsed json response."""
        api_url = 'https://%s/prpc/config.service.v2.Configs/%s' % (
            LUCI_CONFIG_HOST_NAME, method)
        req = input_api.urllib_request.Request(api_url)
        req.add_header('Authorization', 'Bearer %s' % acc_tkn.token)
        req.data = json.dumps(body).encode('utf-8')
        req.add_header('Accept', 'application/json')
        req.add_header('Content-Type', 'application/json')

        attempt = 0
        res = None
        time_to_sleep = 1
        while True:
            try:
                res = input_api.urllib_request.urlopen(req)
                break
            except input_api.urllib_error.HTTPError as e:
                if attempt < max_attempts and e.code >= 500:
                    attempt += 1
                    logging.debug('error when calling %s: %s\n'
                                  'Sleeping for %d seconds and retrying...' %
                                  (api_url, e, time_to_sleep))
                    time.sleep(time_to_sleep)
                    time_to_sleep *= 2
                    continue
                raise

        assert res, 'response from luci-config v2 is impossible to be None'

        # The JSON response has a XSSI protection prefix )]}'
        return json.loads(res.read().decode('utf-8')[len(")]}'"):].strip())

    def format_config_set(cs):
        """convert luci-config v2 config_set object to v1 format."""
        rev = cs.get('revision', {})
        return {
            'config_set': cs.get('name'),
            'location': cs.get('url'),
            'revision': {
                'id': rev.get('id'),
                'url': rev.get('url'),
                'timestamp': rev.get('timestamp'),
                'committer_email': rev.get('committerEmail')
            }
        }

    try:
        config_sets = request('ListConfigSets', {}).get('configSets')
    except input_api.urllib_error.HTTPError as e:
        return [
            output_api.PresubmitError(
                'Config set request to luci-config failed', long_text=str(e))
        ]
    config_sets = [format_config_set(cs) for cs in config_sets]
    if not config_sets:
        return [
            output_api.PresubmitPromptWarning('No config_sets were returned')
        ]
    loc_pref = '%s/+/%s/' % (remote_host_url, remote_branch)
    logging.debug('Derived location prefix: %s', loc_pref)
    dir_to_config_set = {}
    for cs in config_sets:
        if cs['location'].startswith(loc_pref) or ('%s/' %
                                                   cs['location']) == loc_pref:
            path = cs['location'][len(loc_pref):].rstrip('/')
            d = input_api.os_path.join(*path.split('/')) if path else '.'
            dir_to_config_set[d] = cs['config_set']
    if not dir_to_config_set:
        warning_long_text_lines = [
            'No config_set found for %s.' % loc_pref,
            'Found the following:',
        ]
        for loc in sorted(cs['location'] for cs in config_sets):
            warning_long_text_lines.append('  %s' % loc)
        warning_long_text_lines.append('')
        warning_long_text_lines.append('If the requested location is internal,'
                                       ' the requester may not have access.')

        return [
            output_api.PresubmitPromptWarning(
                warning_long_text_lines[0],
                long_text='\n'.join(warning_long_text_lines))
        ]

    dir_to_fileSet = {}
    for f in input_api.AffectedFiles(include_deletes=False):
        for d in dir_to_config_set:
            if d != '.' and not f.LocalPath().startswith(d):
                continue  # file doesn't belong to this config set
            rel_path = f.LocalPath() if d == '.' else input_api.os_path.relpath(
                f.LocalPath(), start=d)
            fileSet = dir_to_fileSet.setdefault(d, set())
            fileSet.add(rel_path.replace(_os.sep, '/'))
            dir_to_fileSet[d] = fileSet

    outputs = []
    lucicfg = 'lucicfg' if not input_api.is_windows else 'lucicfg.bat'
    log_level = 'debug' if input_api.verbose else 'warning'
    repo_root = input_api.change.RepositoryRoot()
    for d, fileSet in dir_to_fileSet.items():
        config_set = dir_to_config_set[d]
        with input_api.CreateTemporaryFile() as f:
            cmd = [
                lucicfg, 'validate', d, '-config-set', config_set, '-log-level',
                log_level, '-json-output', f.name
            ]
            # return code is not important as the validation failure will be
            # retrieved from the output json file.
            out, _ = input_api.subprocess.communicate(
                cmd,
                stderr=input_api.subprocess.PIPE,
                shell=input_api.is_windows,  # to resolve *.bat
                cwd=repo_root,
            )
            logging.debug('running %s\nSTDOUT:\n%s\nSTDERR:\n%s', cmd, out[0],
                          out[1])
            try:
                result = json.load(f)
            except json.JSONDecodeError as e:
                outputs.append(
                    output_api.PresubmitError(
                        'Error when parsing lucicfg validate output',
                        long_text=str(e)))
            else:
                result = result.get('result', None)
                if result:
                    non_affected_file_msg_count = 0
                    for validation_result in (result.get('validation', None)
                                              or []):
                        for msg in (validation_result.get('messages', None)
                                    or []):
                            if d != '.' and msg['path'] not in fileSet:
                                non_affected_file_msg_count += 1
                                continue
                            sev = msg['severity']
                            if sev == 'WARNING':
                                out_f = output_api.PresubmitPromptWarning
                            elif sev in ('ERROR', 'CRITICAL'):
                                out_f = output_api.PresubmitError
                            else:
                                out_f = output_api.PresubmitNotifyResult
                            outputs.append(
                                out_f('Config validation for file(%s): %s' %
                                      (msg['path'], msg['text'])))
                    if non_affected_file_msg_count:
                        reproduce_cmd = [
                            lucicfg, 'validate',
                            repo_root if d == '.' else input_api.os_path.join(
                                repo_root, d), '-config-set', config_set
                        ]
                        outputs.append(
                            output_api.PresubmitPromptWarning(
                                'Found %d additional errors/warnings in files that are not '
                                'modified, run `%s` to reveal them' %
                                (non_affected_file_msg_count,
                                 ' '.join(reproduce_cmd))))
    return outputs


def CheckLucicfgGenOutput(input_api, output_api, entry_script):
    """Verifies configs produced by `lucicfg` are up-to-date and pass validation.

    Runs the check unconditionally, regardless of what files are modified. Examine
    input_api.AffectedFiles() yourself before using CheckLucicfgGenOutput if this
    is a concern.

    Assumes `lucicfg` binary is in PATH and the user is logged in.

    Args:
        entry_script: path to the entry-point *.star script responsible for
            generating a single config set. Either absolute or relative to the
            currently running PRESUBMIT.py script.

    Returns:
        A list of input_api.Command objects containing verification commands.
    """
    return [
        input_api.Command(
            'lucicfg validate "%s"' % entry_script,
            [
                'lucicfg' if not input_api.is_windows else 'lucicfg.bat',
                'validate',
                entry_script,
                '-log-level',
                'debug' if input_api.verbose else 'warning',
            ],
            {
                'stderr': input_api.subprocess.STDOUT,
                'shell': input_api.is_windows,  # to resolve *.bat
                'cwd': input_api.PresubmitLocalPath(),
            },
            output_api.PresubmitError)
    ]


def CheckJsonParses(input_api, output_api, file_filter=None):
    """Verifies that all JSON files at least parse as valid JSON. By default,
    file_filter will look for all files that end with .json"""
    import json
    if file_filter is None:
        file_filter = lambda x: x.LocalPath().endswith('.json')
    affected_files = input_api.AffectedFiles(include_deletes=False,
                                             file_filter=file_filter)
    warnings = []
    for f in affected_files:
        with _io.open(f.AbsoluteLocalPath(), encoding='utf-8') as j:
            try:
                json.load(j)
            except ValueError:
                # Just a warning for now, in case people are using JSON5
                # somewhere.
                warnings.append(
                    output_api.PresubmitPromptWarning(
                        '%s does not appear to be valid JSON.' % f.LocalPath()))
    return warnings


# string pattern, sequence of strings to show when pattern matches,
# error flag. True if match is a presubmit error, otherwise it's a warning.
_NON_INCLUSIVE_TERMS = (
    (
        # Note that \b pattern in python re is pretty particular. In this
        # regexp, 'class WhiteList ...' will match, but 'class FooWhiteList
        # ...' will not. This may require some tweaking to catch these cases
        # without triggering a lot of false positives. Leaving it naive and
        # less matchy for now.
        r'/(?i)\b((black|white)list|slave)\b',  # nocheck
        (
            'Please don\'t use blacklist, whitelist, '  # nocheck
            'or slave in your',  # nocheck
            'code and make every effort to use other terms. Using "// nocheck"',
            '"# nocheck" or "<!-- nocheck -->"',
            'at the end of the offending line will bypass this PRESUBMIT error',
            'but avoid using this whenever possible. Reach out to',
            'community@chromium.org if you have questions'),
        True), )


def _GetMessageForMatchingTerm(input_api, affected_file, line_number, line,
                               term, message):
    """Helper method for CheckInclusiveLanguage.

    Returns an string composed of the name of the file, the line number where the
    match has been found and the additional text passed as |message| in case the
    target type name matches the text inside the line passed as parameter.
    """
    result = []

    # A // nocheck comment will bypass this error.
    if line.endswith(" nocheck") or line.endswith("<!-- nocheck -->"):
        return result

    # Ignore C-style single-line comments about banned terms.
    if input_api.re.search(r"//.*$", line):
        line = input_api.re.sub(r"//.*$", "", line)

    # Ignore lines from C-style multi-line comments.
    if input_api.re.search(r"^\s*\*", line):
        return result

    # Ignore Python-style comments about banned terms.
    # This actually removes comment text from the first # on.
    if input_api.re.search(r"#.*$", line):
        line = input_api.re.sub(r"#.*$", "", line)

    matched = False
    if term[0:1] == '/':
        regex = term[1:]
        if input_api.re.search(regex, line):
            matched = True
    elif term in line:
        matched = True

    if matched:
        result.append('    %s:%d:' % (affected_file.LocalPath(), line_number))
        for message_line in message:
            result.append('      %s' % message_line)

    return result


def CheckInclusiveLanguage(input_api,
                           output_api,
                           excluded_directories_relative_path=None,
                           non_inclusive_terms=_NON_INCLUSIVE_TERMS):
    """Make sure that banned non-inclusive terms are not used."""

    # Presubmit checks may run on a bot where the changes are actually
    # in a repo that isn't chromium/src (e.g., when testing src + tip-of-tree
    # ANGLE), but this particular check only makes sense for changes to
    # chromium/src.
    if input_api.change.RepositoryRoot() != input_api.PresubmitLocalPath():
        return []
    if input_api.no_diffs:
        return []

    warnings = []
    errors = []

    if excluded_directories_relative_path is None:
        excluded_directories_relative_path = [
            'infra', 'inclusive_language_presubmit_exempt_dirs.txt'
        ]

    # Note that this matches exact path prefixes, and does not match
    # subdirectories. Only files directly in an excluded path will
    # match.
    def IsExcludedFile(affected_file, excluded_paths):
        local_dir = input_api.os_path.dirname(affected_file.LocalPath())

        # Excluded paths use forward slashes.
        if input_api.platform == 'win32':
            local_dir = local_dir.replace('\\', '/')

        return local_dir in excluded_paths

    def CheckForMatch(affected_file, line_num, line, term, message, error):
        problems = _GetMessageForMatchingTerm(input_api, affected_file,
                                              line_num, line, term, message)

        if problems:
            if error:
                errors.extend(problems)
            else:
                warnings.extend(problems)

    excluded_paths = []
    excluded_directories_relative_path = input_api.os_path.join(
        *excluded_directories_relative_path)
    dirs_file_path = input_api.os_path.join(input_api.change.RepositoryRoot(),
                                            excluded_directories_relative_path)
    f = input_api.ReadFile(dirs_file_path)

    for line in f.splitlines():
        words = line.split()
        # if a line starts with #, followed by a whitespace or line-end,
        # it's a comment line.
        if len(words) == 0 or words[0] == '#' or words[0] == '':
            continue

        # The first word in each line is a path.
        # Some exempt_dirs.txt files may have additional words in each line
        # (e.g., "third_party 1 2")
        #
        ## The additional words are present in legacy files for historical
        # reasons only. DO NOT parse or require these additional words.
        excluded_paths.append(words[0])

    excluded_paths = set(excluded_paths)
    for f in input_api.AffectedFiles():
        for line_num, line in f.ChangedContents():
            for term, message, error in non_inclusive_terms:
                if IsExcludedFile(f, excluded_paths):
                    continue
                CheckForMatch(f, line_num, line, term, message, error)

    result = []
    if (warnings):
        result.append(
            output_api.PresubmitPromptWarning(
                'Banned non-inclusive language was used.\n\n' +
                'If this is third-party code, please consider updating ' +
                excluded_directories_relative_path + ' if appropriate.\n\n' +
                '\n'.join(warnings)))
    if (errors):
        result.append(
            output_api.PresubmitError(
                'Banned non-inclusive language was used.\n\n' +
                'If this is third-party code, please consider updating ' +
                excluded_directories_relative_path + ' if appropriate.\n\n' +
                '\n'.join(errors)))
    return result


def CheckUpdateOwnersFileReferences(input_api, output_api):
    """Checks whether an OWNERS file is being (re)moved and if so asks the
    contributor to update any file:// references to it."""
    files = []
    # AffectedFiles() includes owner files, not AffectedSourceFiles().
    for f in input_api.AffectedFiles():
        # Moved files appear here as one deletion and one addition.
        if f.LocalPath().endswith('OWNERS') and f.Action() == 'D':
            files.append(f.LocalPath())
    if not files:
        return []
    return [
        output_api.PresubmitPromptWarning(
            'OWNERS files being moved/removed, please update any file:// ' +
            'references to them in other OWNERS files', files)
    ]
