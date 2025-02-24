# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Top-level presubmit script for depot tools.

See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts for
details on the presubmit API built into depot_tools.
"""

PRESUBMIT_VERSION = '2.0.0'

import fnmatch
import os
import sys

# CIPD ensure manifest for checking CIPD client itself.
CIPD_CLIENT_ENSURE_FILE_TEMPLATE = r'''
# Full supported.
$VerifiedPlatform linux-amd64 mac-amd64 windows-amd64 windows-386
# Best effort support.
$VerifiedPlatform linux-386 linux-ppc64 linux-ppc64le linux-s390x
$VerifiedPlatform linux-arm64 linux-armv6l
$VerifiedPlatform linux-mips64 linux-mips64le linux-mipsle

%s %s
'''

# Timeout for a test to be executed.
TEST_TIMEOUT_S = 450  # 7m 30s


def CheckPylint(input_api, output_api):
    """Gather all the pylint logic into one place to make it self-contained."""
    files_to_check = [
        r'^[^/]*\.py$',
        r'^testing_support/[^/]*\.py$',
        r'^tests/[^/]*\.py$',
        r'^recipe_modules/.*\.py$',  # Allow recursive search in recipe modules.
    ]
    files_to_skip = list(input_api.DEFAULT_FILES_TO_SKIP)
    if os.path.exists('.gitignore'):
        with open('.gitignore', encoding='utf-8') as fh:
            lines = [l.strip() for l in fh.readlines()]
            files_to_skip.extend([
                fnmatch.translate(l) for l in lines
                if l and not l.startswith('#')
            ])
    if os.path.exists('.git/info/exclude'):
        with open('.git/info/exclude', encoding='utf-8') as fh:
            lines = [l.strip() for l in fh.readlines()]
            files_to_skip.extend([
                fnmatch.translate(l) for l in lines
                if l and not l.startswith('#')
            ])
    disabled_warnings = [
        'R0401',  # Cyclic import
        'W0613',  # Unused argument
        'C0415',  # import-outside-toplevel
        'R1710',  # inconsistent-return-statements
        'E1101',  # no-member
        'E1120',  # no-value-for-parameter
        'R1708',  # stop-iteration-return
        'W1510',  # subprocess-run-check
        # Checks which should be re-enabled after Python 2 support is removed.
        'R0205',  # useless-object-inheritance
        'R1725',  # super-with-arguments
        'W0707',  # raise-missing-from
        'W1113',  # keyword-arg-before-vararg
    ]
    return input_api.RunTests(input_api.canned_checks.GetPylint(
        input_api,
        output_api,
        files_to_check=files_to_check,
        files_to_skip=files_to_skip,
        disabled_warnings=disabled_warnings,
        version='2.7'),
                              parallel=False)


def CheckRecipes(input_api, output_api):
    file_filter = lambda x: x.LocalPath() == 'infra/config/recipes.cfg'
    return input_api.canned_checks.CheckJsonParses(input_api,
                                                   output_api,
                                                   file_filter=file_filter)


def CheckUsePython3(input_api, output_api):
    results = []

    if sys.version_info.major != 3:
        results.append(
            output_api.PresubmitError(
                'Did not use Python3 for //tests/PRESUBMIT.py.'))

    return results


def CheckJsonFiles(input_api, output_api):
    return input_api.canned_checks.CheckJsonParses(input_api, output_api)


def CheckUnitTestsOnCommit(input_api, output_api):
    """Do not run integration tests on upload since they are way too slow."""

    input_api.SetTimeout(TEST_TIMEOUT_S)

    # Run only selected tests on Windows.
    test_to_run_list = [r'.*test\.py$']
    tests_to_skip_list = []
    if input_api.platform.startswith(('cygwin', 'win32')):
        print('Warning: skipping most unit tests on Windows')
        tests_to_skip_list.extend([
            r'.*auth_test\.py$',
            r'.*git_common_test\.py$',
            r'.*git_hyper_blame_test\.py$',
            r'.*git_map_test\.py$',
            r'.*ninjalog_uploader_test\.py$',
            r'.*recipes_test\.py$',
        ])

    tests = input_api.canned_checks.GetUnitTestsInDirectory(
        input_api,
        output_api,
        'tests',
        files_to_check=test_to_run_list,
        files_to_skip=tests_to_skip_list)

    return input_api.RunTests(tests)


def CheckCIPDManifest(input_api, output_api):
    # Validate CIPD manifests.
    root = input_api.os_path.normpath(
        input_api.os_path.abspath(input_api.PresubmitLocalPath()))
    rel_file = lambda rel: input_api.os_path.join(root, rel)
    cipd_manifests = set(
        rel_file(input_api.os_path.join(*x)) for x in (
            ('cipd_manifest.txt', ),
            ('bootstrap', 'manifest.txt'),
            ('bootstrap', 'manifest_bleeding_edge.txt'),

            # Also generate a file for the cipd client itself.
            (
                'cipd_client_version', ),
        ))
    affected_manifests = input_api.AffectedFiles(
        include_deletes=False,
        file_filter=lambda x: input_api.os_path.normpath(x.AbsoluteLocalPath()
                                                         ) in cipd_manifests)
    tests = []
    for path in affected_manifests:
        path = path.AbsoluteLocalPath()
        if path.endswith('.txt'):
            tests.append(
                input_api.canned_checks.CheckCIPDManifest(input_api,
                                                          output_api,
                                                          path=path))
        else:
            pkg = 'infra/tools/cipd/${platform}'
            ver = input_api.ReadFile(path)
            tests.append(
                input_api.canned_checks.CheckCIPDManifest(
                    input_api,
                    output_api,
                    content=CIPD_CLIENT_ENSURE_FILE_TEMPLATE % (pkg, ver)))
            tests.append(
                input_api.canned_checks.CheckCIPDClientDigests(
                    input_api, output_api, client_version_file=path))

    return input_api.RunTests(tests)


def CheckOwnersFormat(input_api, output_api):
    return input_api.canned_checks.CheckOwnersFormat(input_api, output_api)


def CheckOwnersOnUpload(input_api, output_api):
    return input_api.canned_checks.CheckOwners(input_api,
                                               output_api,
                                               allow_tbr=False)


def CheckDoNotSubmitOnCommit(input_api, output_api):
    return input_api.canned_checks.CheckDoNotSubmit(input_api, output_api)


def CheckPatchFormatted(input_api, output_api):
    # TODO(https://crbug.com/979330) If clang-format is fixed for non-chromium
    # repos, remove check_clang_format=False so that proto files can be
    # formatted
    return input_api.canned_checks.CheckPatchFormatted(input_api,
                                                       output_api,
                                                       check_clang_format=False)


def CheckFreezeOnCommit(input_api, output_api):
    return input_api.canned_checks.CheckInfraFreeze(input_api, output_api)
