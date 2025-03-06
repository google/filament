# Copyright 2022 The Dawn & Tint Authors
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

import re

NONINCLUSIVE_REGEXES = [
    r"(?i)black[-_]?list",
    r"(?i)white[-_]?list",
    r"(?i)gr[ea]y[-_]?list",
    r"(?i)(first class citizen)",
    r"(?i)black[-_]?hat",
    r"(?i)white[-_]?hat",
    r"(?i)gr[ea]y[-_]?hat",
    r"(?i)master",
    r"(?i)slave",
    r"(?i)\bhim\b",
    r"(?i)\bhis\b",
    r"(?i)\bshe\b",
    r"(?i)\bher\b",
    r"(?i)\bguys\b",
    r"(?i)\bhers\b",
    r"(?i)\bman\b",
    r"(?i)\bwoman\b",
    r"(?i)\she\s",
    r"(?i)\she$",
    r"(?i)^he\s",
    r"(?i)^he$",
    r"(?i)\she['|\u2019]d\s",
    r"(?i)\she['|\u2019]d$",
    r"(?i)^he['|\u2019]d\s",
    r"(?i)^he['|\u2019]d$",
    r"(?i)\she['|\u2019]s\s",
    r"(?i)\she['|\u2019]s$",
    r"(?i)^he['|\u2019]s\s",
    r"(?i)^he['|\u2019]s$",
    r"(?i)\she['|\u2019]ll\s",
    r"(?i)\she['|\u2019]ll$",
    r"(?i)^he['|\u2019]ll\s",
    r"(?i)^he['|\u2019]ll$",
    r"(?i)grandfather",
    r"(?i)\bmitm\b",
    r"(?i)\bcrazy\b",
    r"(?i)\binsane\b",
    r"(?i)\bblind\sto\b",
    r"(?i)\bflying\sblind\b",
    r"(?i)\bblind\seye\b",
    r"(?i)\bcripple\b",
    r"(?i)\bcrippled\b",
    r"(?i)\bdumb\b",
    r"(?i)\bdummy\b",
    r"(?i)\bparanoid\b",
    r"(?i)\bsane\b",
    r"(?i)\bsanity\b",
    r"(?i)red[-_]?line",
]

NONINCLUSIVE_REGEX_LIST = []
for reg in NONINCLUSIVE_REGEXES:
    NONINCLUSIVE_REGEX_LIST.append(re.compile(reg))

LINT_FILTERS = []


def _CheckNonInclusiveLanguage(input_api, output_api, source_file_filter=None):
    """Checks the files for non-inclusive language."""

    matches = []
    for f in input_api.AffectedFiles(include_deletes=False,
                                     file_filter=source_file_filter):
        line_num = 0
        for line in f.NewContents():
            line_num += 1
            for reg in NONINCLUSIVE_REGEX_LIST:
                match = reg.search(line)
                if match:
                    matches.append(
                        "{} ({}): found non-inclusive language: {}".format(
                            f.LocalPath(), line_num, match.group(0)))

    if len(matches):
        return [
            output_api.PresubmitPromptWarning("Non-inclusive language found:",
                                              items=matches)
        ]

    return []


def _NonInclusiveFileFilter(file):
    filter_list = [
        "Doxyfile",  # References to main pages
        "PRESUBMIT.py",  # Non-inclusive language check data
        "PRESUBMIT.py.tint",  # Non-inclusive language check data
        "docs/dawn/debug_markers.md",  # External URL
        "docs/dawn/infra.md",  # Infra settings
        "docs/tint/spirv-input-output-variables.md",  # External URL
        "infra/config/global/generated/cr-buildbucket.cfg",  # Infra settings
        "infra/config/global/main.star",  # Infra settings
        "infra/kokoro/windows/build.bat",  # External URL
        "src/dawn/common/GPUInfo.cpp",  # External URL
        "src/dawn/native/metal/BackendMTL.mm",  # OSX Constant
        "src/dawn/native/vulkan/SamplerVk.cpp",  # External URL
        "src/dawn/native/vulkan/TextureVk.cpp",  # External URL
        "src/tools/src/cmd/run-cts/main.go",  # Terminal type name
        "src/dawn/samples/ComputeBoids.cpp",  # External URL
        "src/dawn/tests/end2end/DepthBiasTests.cpp",  # External URL
        "src/tint/transform/canonicalize_entry_point_io.cc",  # External URL
        "test/tint/samples/compute_boids.wgsl",  # External URL
        "third_party/gn/dxc/BUILD.gn",  # Third party file
        "third_party/khronos/EGL-Registry/api/KHR/khrplatform.h",  # Third party file
        "tools/roll-all",  # Branch name
        "tools/src/container/key.go",  # External URL
        "go.sum",  # External URL
    ]
    return file.LocalPath().replace('\\', '/') not in filter_list


def _CheckNoStaleGen(input_api, output_api):
    results = []
    try:
        go = input_api.os_path.join(input_api.change.RepositoryRoot(), "tools",
                                    "golang", "bin", "go")
        if input_api.is_windows:
            go += '.exe'
        input_api.subprocess.check_call_out(
            [go, "run", "tools/src/cmd/gen/main.go", "--check-stale"],
            stdout=input_api.subprocess.PIPE,
            stderr=input_api.subprocess.PIPE,
            cwd=input_api.change.RepositoryRoot())
    except input_api.subprocess.CalledProcessError as e:
        if input_api.is_committing:
            results.append(output_api.PresubmitError('%s' % (e, )))
        else:
            results.append(output_api.PresubmitPromptWarning('%s' % (e, )))
    return results


def _DoCommonChecks(input_api, output_api):
    results = []
    results.extend(
        input_api.canned_checks.CheckForCommitObjects(input_api, output_api))
    results.extend(_CheckNoStaleGen(input_api, output_api))

    result_factory = output_api.PresubmitPromptWarning
    if input_api.is_committing:
        result_factory = output_api.PresubmitError

    # Check for formatting.
    results.extend(
        input_api.canned_checks.CheckPatchFormatted(
            input_api,
            output_api,
            check_python=True,
            result_factory=result_factory))
    results.extend(
        input_api.canned_checks.CheckGNFormatted(input_api, output_api))
    results.extend(
        input_api.canned_checks.CheckChangeHasNoCrAndHasOnlyOneEol(
            input_api, output_api))
    results.extend(
        input_api.canned_checks.CheckChangeHasNoTabs(input_api, output_api))
    results.extend(
        input_api.canned_checks.CheckChangeTodoHasOwner(input_api, output_api))
    results.extend(
        input_api.canned_checks.CheckChangeHasNoStrayWhitespace(
            input_api, output_api))

    results.extend(
        input_api.canned_checks.CheckChangeHasDescription(
            input_api, output_api))
    results.extend(
        input_api.canned_checks.CheckDoNotSubmit(input_api, output_api))
    # Note, the verbose_level here should match what is set in tools/lint so
    # the same set of lint errors are reported on the CQ and Kokoro bots.
    results.extend(
        input_api.canned_checks.CheckChangeLintsClean(
            input_api, output_api, lint_filters=LINT_FILTERS, verbose_level=1))
    results.extend(
        _CheckNonInclusiveLanguage(input_api, output_api,
                                   _NonInclusiveFileFilter))
    return results


CheckChangeOnUpload = _DoCommonChecks
CheckChangeOnCommit = _DoCommonChecks
