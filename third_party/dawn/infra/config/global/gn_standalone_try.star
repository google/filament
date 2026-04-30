# Copyright 2025 The Dawn & Tint Authors
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

"""Try Dawn builders using GN and a standalone Dawn checkout (instead of Chromium)."""

load("@chromium-luci//builders.star", "os")
load("@chromium-luci//try.star", "try_")
load("//constants.star", "siso")
load("//location_filters.star", "exclusion_filters")
load("//project.star", "ACTIVE_MILESTONES")

try_.defaults.set(
    executable = "recipe:dawn/gn_v2_trybot",
    builder_group = "try",
    bucket = "try",
    pool = "luci.chromium.gpu.try",
    builderless = True,
    build_numbers = True,
    list_view = "try",
    cq_group = "Dawn-CQ",
    contact_team_email = "chrome-gpu-infra@google.com",
    service_account = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
    siso_project = siso.project.DEFAULT_UNTRUSTED,
    siso_remote_jobs = siso.remote_jobs.DEFAULT,
)

################################################################################
# CQ Builders                                                                  #
################################################################################

## Templates

def apply_cq_builder_defaults(kwargs):
    kwargs.setdefault("max_concurrent_builds", 3)
    return kwargs

def apply_linux_cq_builder_defaults(kwargs):
    kwargs = apply_cq_builder_defaults(kwargs)
    kwargs.setdefault("os", os.LINUX_DEFAULT)
    kwargs.setdefault("ssd", None)
    return kwargs

def apply_mac_cq_builder_defaults(kwargs):
    kwargs = apply_cq_builder_defaults(kwargs)
    kwargs.setdefault("os", os.MAC_DEFAULT)
    kwargs.setdefault("cpu", "arm64")
    return kwargs

def apply_win_cq_builder_defaults(kwargs):
    kwargs = apply_cq_builder_defaults(kwargs)
    kwargs.setdefault("os", os.WINDOWS_DEFAULT)

    # This can be changed to prefer SSDs once the GPU Windows GCE fleet has
    # been switched to primarily using SSDs.
    kwargs.setdefault("ssd", None)
    return kwargs

def apply_functional_builder_with_node_defaults(kwargs):
    kwargs.setdefault("tryjob", try_.job(
        location_filters = exclusion_filters.gn_clang_cq_file_exclusions,
    ))
    return kwargs

def apply_functional_builder_without_node_defaults(kwargs):
    kwargs.setdefault("tryjob", try_.job(
        location_filters = exclusion_filters.gn_clang_no_node_cq_file_exclusions,
    ))
    return kwargs

def apply_fuzz_builder_defaults(kwargs):
    kwargs.setdefault("tryjob", try_.job(
        location_filters = exclusion_filters.gn_clang_cq_fuzz_file_exclusions,
    ))
    return kwargs

def add_builder_to_main_and_milestone_cq_groups(kwargs):
    # Dawn standalone builders run fine unbranched on branched CLs.
    try_.builder(**kwargs)
    for milestone in ACTIVE_MILESTONES.keys():
        luci.cq_tryjob_verifier(
            cq_group = "Dawn-CQ-" + milestone,
            builder = "dawn:try/" + kwargs["name"],
        )

def dawn_linux_functional_cq_tester(**kwargs):
    kwargs = apply_linux_cq_builder_defaults(kwargs)
    kwargs = apply_functional_builder_with_node_defaults(kwargs)
    add_builder_to_main_and_milestone_cq_groups(kwargs)

def dawn_mac_functional_cq_tester(**kwargs):
    kwargs = apply_mac_cq_builder_defaults(kwargs)
    kwargs = apply_functional_builder_with_node_defaults(kwargs)
    add_builder_to_main_and_milestone_cq_groups(kwargs)

def dawn_win_functional_cq_tester(**kwargs):
    kwargs = apply_win_cq_builder_defaults(kwargs)
    kwargs = apply_functional_builder_with_node_defaults(kwargs)
    add_builder_to_main_and_milestone_cq_groups(kwargs)

def dawn_linux_functional_cq_tester_without_node(**kwargs):
    kwargs = apply_linux_cq_builder_defaults(kwargs)
    kwargs = apply_functional_builder_without_node_defaults(kwargs)
    add_builder_to_main_and_milestone_cq_groups(kwargs)

def dawn_win_functional_cq_tester_without_node(**kwargs):
    kwargs = apply_win_cq_builder_defaults(kwargs)
    kwargs = apply_functional_builder_without_node_defaults(kwargs)
    add_builder_to_main_and_milestone_cq_groups(kwargs)

def dawn_linux_fuzz_cq_tester(**kwargs):
    kwargs = apply_linux_cq_builder_defaults(kwargs)
    kwargs = apply_fuzz_builder_defaults(kwargs)
    add_builder_to_main_and_milestone_cq_groups(kwargs)

## Functional testers

dawn_linux_functional_cq_tester(
    name = "dawn-cq-linux-x64-dbg",
    description_html = "Tests debug Dawn on Linux/x64 on multiple hardware configs. Blocks CL submission",
    mirrors = [
        "ci/dawn-linux-x64-builder-dbg",
        "ci/dawn-linux-x64-sws-dbg",
    ],
    gn_args = "ci/dawn-linux-x64-builder-dbg",
)

dawn_linux_functional_cq_tester(
    name = "dawn-cq-linux-x64-rel",
    description_html = "Tests release Dawn on Linux/x64 on multiple hardware configs. Blocks CL submission",
    mirrors = [
        "ci/dawn-linux-x64-builder-rel",
        "ci/dawn-linux-x64-intel-uhd630-rel",
        "ci/dawn-linux-x64-intel-uhd770-rel",
        "ci/dawn-linux-x64-nvidia-gtx1660-rel",
        "ci/dawn-linux-x64-sws-rel",
    ],
    gn_args = "ci/dawn-linux-x64-builder-rel",
)

dawn_linux_functional_cq_tester_without_node(
    name = "dawn-cq-linux-x86-dbg",
    description_html = "Tests debug Dawn on Linux/x86 on multiple hardware configs. Blocks CL submission",
    mirrors = [
        "ci/dawn-linux-x86-builder-dbg",
        "ci/dawn-linux-x86-sws-dbg",
    ],
    gn_args = "ci/dawn-linux-x86-builder-dbg",
)

dawn_linux_functional_cq_tester_without_node(
    name = "dawn-cq-linux-x86-rel",
    description_html = "Tests release Dawn on Linux/x86 on multiple hardware configs. Blocks CL submission",
    mirrors = [
        "ci/dawn-linux-x86-builder-rel",
        "ci/dawn-linux-x86-sws-rel",
    ],
    gn_args = "ci/dawn-linux-x86-builder-rel",
)

dawn_mac_functional_cq_tester(
    name = "dawn-cq-mac-arm64-rel",
    description_html = "Tests release Dawn on Mac/arm64 on multiple hardware configs. Blocks CL submission",
    mirrors = [
        "ci/dawn-mac-arm64-builder-rel",
        "ci/dawn-mac-arm64-apple-m2-rel",
    ],
    gn_args = "ci/dawn-mac-arm64-builder-rel",
)

dawn_mac_functional_cq_tester(
    name = "dawn-cq-mac-x64-dbg",
    description_html = "Tests debug Dawn on Mac/x64 on multiple hardware configs. Blocks CL submission",
    mirrors = [
        "ci/dawn-mac-x64-builder-dbg",
        "ci/dawn-mac-x64-sws-dbg",
    ],
    gn_args = "ci/dawn-mac-x64-builder-dbg",
)

dawn_mac_functional_cq_tester(
    name = "dawn-cq-mac-x64-rel",
    description_html = "Tests release Dawn on Mac/x64 on multiple hardware configs. Blocks CL submission",
    mirrors = [
        "ci/dawn-mac-x64-builder-rel",
        "ci/dawn-mac-x64-amd-5300m-rel",
        "ci/dawn-mac-x64-amd-555x-rel",
        "ci/dawn-mac-x64-intel-uhd630-rel",
        "ci/dawn-mac-x64-sws-rel",
    ],
    gn_args = "ci/dawn-mac-x64-builder-rel",
)

dawn_win_functional_cq_tester(
    name = "dawn-cq-win-x64-dbg",
    description_html = "Tests debug Dawn on Win/x64 on multiple hardware configs. Blocks CL submission",
    mirrors = [
        "ci/dawn-win-x64-builder-dbg",
        "ci/dawn-win-x64-sws-dbg",
    ],
    gn_args = "ci/dawn-win-x64-builder-dbg",
)

dawn_win_functional_cq_tester(
    name = "dawn-cq-win-arm64-rel",
    description_html = "Tests release Dawn on Win/ARM64 configs. Blocks CL submission",
    mirrors = [
        "ci/dawn-win-arm64-builder-rel",
        "ci/dawn-win-arm64-qualcomm-snapdragonxelite-rel",
    ],
    gn_args = "ci/dawn-win-arm64-builder-rel",
)

dawn_win_functional_cq_tester(
    name = "dawn-cq-win-x64-msvc-dbg",
    description_html = "Tests debug Dawn built with MSVC on Win/x64 on multiple hardware configs. Blocks CL submission",
    mirrors = [
        "ci/dawn-win-x64-builder-msvc-dbg",
        "ci/dawn-win-x64-sws-msvc-dbg",
    ],
    gn_args = "ci/dawn-win-x64-builder-msvc-dbg",
)

dawn_win_functional_cq_tester(
    name = "dawn-cq-win-x64-msvc-rel",
    description_html = "Tests release Dawn built with MSVC on Win/x64 on multiple hardware configs. Blocks CL submission",
    mirrors = [
        "ci/dawn-win-x64-builder-msvc-rel",
        "ci/dawn-win-x64-sws-msvc-rel",
    ],
    gn_args = "ci/dawn-win-x64-builder-msvc-rel",
)

dawn_win_functional_cq_tester(
    name = "dawn-cq-win-x64-rel",
    description_html = "Tests release Dawn on Win/x64 on multiple hardware configs. Blocks CL submission",
    mirrors = [
        "ci/dawn-win-x64-builder-rel",
        "ci/dawn-win-x64-intel-uhd630-rel",
        # TODO(crbug.com/458768121): Add the UHD 770 config when capacity has
        # recovered.
        "ci/dawn-win-x64-nvidia-gtx1660-rel",
        "ci/dawn-win-x64-sws-rel",
    ],
    gn_args = "ci/dawn-win-x64-builder-rel",
)

dawn_win_functional_cq_tester_without_node(
    name = "dawn-cq-win-x86-dbg",
    description_html = "Tests debug Dawn on Win/x86 on multiple hardware configs. Blocks CL submission",
    mirrors = [
        "ci/dawn-win-x86-builder-dbg",
        "ci/dawn-win-x86-sws-dbg",
    ],
    gn_args = "ci/dawn-win-x86-builder-dbg",
)

dawn_win_functional_cq_tester_without_node(
    name = "dawn-cq-win-x86-rel",
    description_html = "Tests release Dawn on Win/x86 on multiple hardware configs. Blocks CL submission",
    mirrors = [
        "ci/dawn-win-x86-builder-rel",
        "ci/dawn-win-x86-intel-uhd630-rel",
        "ci/dawn-win-x86-nvidia-gtx1660-rel",
        "ci/dawn-win-x86-sws-rel",
    ],
    gn_args = "ci/dawn-win-x86-builder-rel",
)

## Fuzz testers

dawn_linux_fuzz_cq_tester(
    name = "dawn-cq-linux-x64-fuzz-dbg",
    description_html = "Compiles and runs debug Dawn binaries for 'tools/run fuzz' for Linux/x64. Blocks CL submission.",
    mirrors = [
        "ci/dawn-linux-x64-fuzz-dbg",
    ],
    gn_args = "ci/dawn-linux-x64-fuzz-dbg",
)

dawn_linux_fuzz_cq_tester(
    name = "dawn-cq-linux-x64-fuzz-rel",
    description_html = "Compiles and runs release Dawn binaries for 'tools/run fuzz' for Linux/x64. Blocks CL submission.",
    mirrors = [
        "ci/dawn-linux-x64-fuzz-rel",
    ],
    gn_args = "ci/dawn-linux-x64-fuzz-rel",
)

dawn_linux_fuzz_cq_tester(
    name = "dawn-cq-linux-x86-fuzz-dbg",
    description_html = "Compiles and runs debug Dawn binaries for 'tools/run fuzz' for Linux/x86. Blocks CL submission.",
    mirrors = [
        "ci/dawn-linux-x86-fuzz-dbg",
    ],
    gn_args = "ci/dawn-linux-x86-fuzz-dbg",
)

dawn_linux_fuzz_cq_tester(
    name = "dawn-cq-linux-x86-fuzz-rel",
    description_html = "Compiles and runs release Dawn binaries for 'tools/run fuzz' for Linux/x86. Blocks CL submission.",
    mirrors = [
        "ci/dawn-linux-x86-fuzz-rel",
    ],
    gn_args = "ci/dawn-linux-x86-fuzz-rel",
)

################################################################################
# Manual Trybots                                                               #
################################################################################

## Templates

def dawn_linux_manual_builder(*, name, **kwargs):
    return try_.builder(
        name = name,
        max_concurrent_builds = 1,
        os = os.LINUX_DEFAULT,
        ssd = None,
        **kwargs
    )

def dawn_mac_manual_builder(*, name, **kwargs):
    kwargs.setdefault("cpu", "arm64")
    return try_.builder(
        name = name,
        max_concurrent_builds = 1,
        os = os.MAC_DEFAULT,
        **kwargs
    )

def dawn_win_manual_builder(*, name, **kwargs):
    return try_.builder(
        name = name,
        max_concurrent_builds = 1,
        os = os.WINDOWS_DEFAULT,
        # This can be changed to prefer SSDs once the GPU Windows GCE fleet has
        # been switched to primarily using SSDs.
        ssd = None,
        **kwargs
    )

## Functional testers

dawn_linux_manual_builder(
    name = "dawn-try-linux-x64-intel-uhd630-rel",
    description_html = "Tests release Dawn on Linux/x64 on Intel CPUs w/ UHD 630 GPUs. Manual only.",
    mirrors = [
        "ci/dawn-linux-x64-builder-rel",
        "ci/dawn-linux-x64-intel-uhd630-rel",
    ],
    gn_args = "ci/dawn-linux-x64-builder-rel",
)

dawn_linux_manual_builder(
    name = "dawn-try-linux-x64-intel-uhd770-rel",
    description_html = "Tests release Dawn on Linux/x64 on Intel CPUs w/ UHD 770 GPUs. Manual only.",
    mirrors = [
        "ci/dawn-linux-x64-builder-rel",
        "ci/dawn-linux-x64-intel-uhd770-rel",
    ],
    gn_args = "ci/dawn-linux-x64-builder-rel",
)

dawn_linux_manual_builder(
    name = "dawn-try-linux-x64-nvidia-gtx1660-rel",
    description_html = "Tests release Dawn on Linux/x64 on NVIDIA GTX 1660 GPUs. Manual only.",
    mirrors = [
        "ci/dawn-linux-x64-builder-rel",
        "ci/dawn-linux-x64-nvidia-gtx1660-rel",
    ],
    gn_args = "ci/dawn-linux-x64-builder-rel",
)

dawn_linux_manual_builder(
    name = "dawn-try-linux-x64-sws-dbg",
    description_html = "Tests debug Dawn on Linux/x64 with SwiftShader. Manual only.",
    mirrors = [
        "ci/dawn-linux-x64-builder-dbg",
        "ci/dawn-linux-x64-sws-dbg",
    ],
    gn_args = "ci/dawn-linux-x64-builder-dbg",
)

dawn_linux_manual_builder(
    name = "dawn-try-linux-x64-sws-rel",
    description_html = "Tests release Dawn on Linux/x64 with SwiftShader. Manual only.",
    mirrors = [
        "ci/dawn-linux-x64-builder-rel",
        "ci/dawn-linux-x64-sws-rel",
    ],
    gn_args = "ci/dawn-linux-x64-builder-rel",
)

dawn_linux_manual_builder(
    name = "dawn-try-linux-x64-sws-tsan",
    description_html = "Tests release Dawn on Linux/x64 with SwiftShader with TSAN. Manual only.",
    mirrors = [
        "ci/dawn-linux-x64-builder-tsan",
        "ci/dawn-linux-x64-sws-tsan",
    ],
    gn_args = "ci/dawn-linux-x64-builder-tsan",
)

dawn_linux_manual_builder(
    name = "dawn-try-linux-x86-sws-dbg",
    description_html = "Tests debug Dawn on Linux/x86 with SwiftShader. Manual only.",
    mirrors = [
        "ci/dawn-linux-x86-builder-dbg",
        "ci/dawn-linux-x86-sws-dbg",
    ],
    gn_args = "ci/dawn-linux-x86-builder-dbg",
)

dawn_linux_manual_builder(
    name = "dawn-try-linux-x86-sws-rel",
    description_html = "Tests release Dawn on Linux/x86 with SwiftShader. Manual only.",
    mirrors = [
        "ci/dawn-linux-x86-builder-rel",
        "ci/dawn-linux-x86-sws-rel",
    ],
    gn_args = "ci/dawn-linux-x86-builder-rel",
)

dawn_mac_manual_builder(
    name = "dawn-try-mac-arm64-apple-m2-rel",
    description_html = "Tests release Dawn on Mac/arm64 on Apple M2 devices. Manual only.",
    mirrors = [
        "ci/dawn-mac-arm64-builder-rel",
        "ci/dawn-mac-arm64-apple-m2-rel",
    ],
    gn_args = "ci/dawn-mac-arm64-builder-rel",
)

dawn_mac_manual_builder(
    name = "dawn-try-mac-x64-amd-5300m-rel",
    description_html = "Tests release Dawn on Mac/x64 on 16\" 2019 Macbook Pros w/ 5300M GPUs. Manual only.",
    mirrors = [
        "ci/dawn-mac-x64-builder-rel",
        "ci/dawn-mac-x64-amd-5300m-rel",
    ],
    gn_args = "ci/dawn-mac-x64-builder-rel",
)

dawn_mac_manual_builder(
    name = "dawn-try-mac-x64-amd-555x-rel",
    description_html = "Tests release Dawn on Mac/x64 on 15\" 2019 Macbook Pros w/ AMD Radeon Pro 555X GPUs. Manual only.",
    mirrors = [
        "ci/dawn-mac-x64-builder-rel",
        "ci/dawn-mac-x64-amd-555x-rel",
    ],
    gn_args = "ci/dawn-mac-x64-builder-rel",
)

dawn_mac_manual_builder(
    name = "dawn-try-mac-x64-intel-uhd630-exp-rel",
    description_html = "Tests release Dawn on Mac/x64 on 2018 Mac Minis w/ Intel UHD 630 GPUs w/ experimental OS configs. Manual only.",
    mirrors = [
        "ci/dawn-mac-x64-builder-rel",
        "ci/dawn-mac-x64-intel-uhd630-exp-rel",
    ],
    gn_args = "ci/dawn-mac-x64-builder-rel",
)

dawn_mac_manual_builder(
    name = "dawn-try-mac-x64-intel-uhd630-rel",
    description_html = "Tests release Dawn on Mac/x64 on 2018 Mac Minis w/ Intel UHD 630 GPUs. Manual only.",
    mirrors = [
        "ci/dawn-mac-x64-builder-rel",
        "ci/dawn-mac-x64-intel-uhd630-rel",
    ],
    gn_args = "ci/dawn-mac-x64-builder-rel",
)

dawn_mac_manual_builder(
    name = "dawn-try-mac-x64-sws-dbg",
    description_html = "Tests debug Dawn on Mac/x64 with SwiftShader. Manual only.",
    mirrors = [
        "ci/dawn-mac-x64-builder-dbg",
        "ci/dawn-mac-x64-sws-dbg",
    ],
    gn_args = "ci/dawn-mac-x64-builder-dbg",
)

dawn_mac_manual_builder(
    name = "dawn-try-mac-x64-sws-rel",
    description_html = "Tests release Dawn on Mac/x64 with SwiftShader. Manual only.",
    mirrors = [
        "ci/dawn-mac-x64-builder-rel",
        "ci/dawn-mac-x64-sws-rel",
    ],
    gn_args = "ci/dawn-mac-x64-builder-rel",
)

dawn_win_manual_builder(
    name = "dawn-try-win-arm64-qualcomm-snapdragonxelite-rel",
    description_html = "Tests release Dawn on Windows/arm64 on devices with Snapdragon X Elite SoCs. Manual only.",
    mirrors = [
        "ci/dawn-win-arm64-builder-rel",
        "ci/dawn-win-arm64-qualcomm-snapdragonxelite-rel",
    ],
    gn_args = "ci/dawn-win-arm64-builder-rel",
)

dawn_win_manual_builder(
    name = "dawn-try-win-x64-amd-rx5500xt-rel",
    description_html = "Tests release Dawn on Windows/x64 on AMD RX 5500 XT GPUs. Manual only.",
    mirrors = [
        "ci/dawn-win-x64-builder-rel",
        "ci/dawn-win-x64-amd-rx5500xt-rel",
    ],
    gn_args = "ci/dawn-win-x64-builder-rel",
)

dawn_win_manual_builder(
    name = "dawn-try-win-x64-intel-uhd630-asan",
    description_html = "Tests release Dawn on Windows/x64/ASAN on Intel CPUs w/ UHD 630. Manual only.",
    mirrors = [
        "ci/dawn-win-x64-builder-asan",
        "ci/dawn-win-x64-intel-uhd630-asan",
    ],
    gn_args = "ci/dawn-win-x64-builder-asan",
)

dawn_win_manual_builder(
    name = "dawn-try-win-x64-intel-uhd630-rel",
    description_html = "Tests release Dawn on Windows/x64 on Intel CPUs w/ UHD 630. Manual only.",
    mirrors = [
        "ci/dawn-win-x64-builder-rel",
        "ci/dawn-win-x64-intel-uhd630-rel",
    ],
    gn_args = "ci/dawn-win-x64-builder-rel",
)

dawn_win_manual_builder(
    name = "dawn-try-win-x64-intel-uhd770-rel",
    description_html = "Tests release Dawn on Windows/x64 on Intel CPUs w/ UHD 770. Manual only.",
    mirrors = [
        "ci/dawn-win-x64-builder-rel",
        "ci/dawn-win-x64-intel-uhd770-rel",
    ],
    gn_args = "ci/dawn-win-x64-builder-rel",
)

dawn_win_manual_builder(
    name = "dawn-try-win-x64-nvidia-gtx1660-asan",
    description_html = "Tests release Dawn on Windows/x64/ASAN on NVIDIA GTX 1660 GPUs. Manual only.",
    mirrors = [
        "ci/dawn-win-x64-builder-asan",
        "ci/dawn-win-x64-nvidia-gtx1660-asan",
    ],
    gn_args = "ci/dawn-win-x64-builder-asan",
)

dawn_win_manual_builder(
    name = "dawn-try-win-x64-nvidia-gtx1660-exp-rel",
    description_html = "Tests release Dawn on Windows/x64 on NVIDIA GTX 1660 GPUs w/ experimental OS/driver configs. Manual only.",
    mirrors = [
        "ci/dawn-win-x64-builder-rel",
        "ci/dawn-win-x64-nvidia-gtx1660-exp-rel",
    ],
    gn_args = "ci/dawn-win-x64-builder-rel",
)

dawn_win_manual_builder(
    name = "dawn-try-win-x64-nvidia-gtx1660-rel",
    description_html = "Tests release Dawn on Windows/x64 on NVIDIA GTX 1660 GPUs. Manual only.",
    mirrors = [
        "ci/dawn-win-x64-builder-rel",
        "ci/dawn-win-x64-nvidia-gtx1660-rel",
    ],
    gn_args = "ci/dawn-win-x64-builder-rel",
)

dawn_win_manual_builder(
    name = "dawn-try-win-x64-sws-dbg",
    description_html = "Tests debug Dawn on Windows/x64 with SwiftShader. Manual only.",
    mirrors = [
        "ci/dawn-win-x64-builder-dbg",
        "ci/dawn-win-x64-sws-dbg",
    ],
    gn_args = "ci/dawn-win-x64-builder-dbg",
)

dawn_win_manual_builder(
    name = "dawn-try-win-x64-sws-msvc-dbg",
    description_html = "Tests debug Dawn on Windows/x64 with SwiftShader using binaries built with MSVC. Manual only.",
    mirrors = [
        "ci/dawn-win-x64-builder-msvc-dbg",
        "ci/dawn-win-x64-sws-msvc-dbg",
    ],
    gn_args = "ci/dawn-win-x64-builder-msvc-dbg",
)

dawn_win_manual_builder(
    name = "dawn-try-win-x64-sws-msvc-rel",
    description_html = "Tests release Dawn on Windows/x64 with SwiftShader using binaries built with MSVC. Manual only.",
    mirrors = [
        "ci/dawn-win-x64-builder-msvc-rel",
        "ci/dawn-win-x64-sws-msvc-rel",
    ],
    gn_args = "ci/dawn-win-x64-builder-msvc-rel",
)

dawn_win_manual_builder(
    name = "dawn-try-win-x64-sws-rel",
    description_html = "Tests release Dawn on Windows/x64 with SwiftShader. Manual only.",
    mirrors = [
        "ci/dawn-win-x64-builder-rel",
        "ci/dawn-win-x64-sws-rel",
    ],
    gn_args = "ci/dawn-win-x64-builder-rel",
)

dawn_win_manual_builder(
    name = "dawn-try-win-x86-intel-uhd630-rel",
    description_html = "Tests release Dawn on Windows/x86 on Intel CPUs w/ UHD 630. Manual only.",
    mirrors = [
        "ci/dawn-win-x86-builder-rel",
        "ci/dawn-win-x86-intel-uhd630-rel",
    ],
    gn_args = "ci/dawn-win-x86-builder-rel",
)

dawn_win_manual_builder(
    name = "dawn-try-win-x86-nvidia-gtx1660-rel",
    description_html = "Tests release Dawn on Windows/x86 on NVIDIA GTX 1660 GPUs. Manual only.",
    mirrors = [
        "ci/dawn-win-x86-builder-rel",
        "ci/dawn-win-x86-nvidia-gtx1660-rel",
    ],
    gn_args = "ci/dawn-win-x86-builder-rel",
)

dawn_win_manual_builder(
    name = "dawn-try-win-x86-sws-dbg",
    description_html = "Tests debug Dawn on Windows/x86 with SwiftShader. Manual only.",
    mirrors = [
        "ci/dawn-win-x86-builder-dbg",
        "ci/dawn-win-x86-sws-dbg",
    ],
    gn_args = "ci/dawn-win-x86-builder-dbg",
)

dawn_win_manual_builder(
    name = "dawn-try-win-x86-sws-rel",
    description_html = "Tests release Dawn on Windows/x86 with SwiftShader. Manual only.",
    mirrors = [
        "ci/dawn-win-x86-builder-rel",
        "ci/dawn-win-x86-sws-rel",
    ],
    gn_args = "ci/dawn-win-x86-builder-rel",
)

## Fuzz testers

dawn_linux_manual_builder(
    name = "dawn-try-linux-x64-fuzz-dbg",
    description_html = "Runs debug Dawn fuzz tests on Linux/x64. Manual only.",
    mirrors = [
        "ci/dawn-linux-x64-fuzz-dbg",
    ],
    gn_args = "ci/dawn-linux-x64-fuzz-dbg",
)

dawn_linux_manual_builder(
    name = "dawn-try-linux-x64-fuzz-rel",
    description_html = "Runs release Dawn fuzz tests on Linux/x64. Manual only.",
    mirrors = [
        "ci/dawn-linux-x64-fuzz-rel",
    ],
    gn_args = "ci/dawn-linux-x64-fuzz-rel",
)

dawn_linux_manual_builder(
    name = "dawn-try-linux-x86-fuzz-dbg",
    description_html = "Runs debug Dawn fuzz tests on Linux/x86. Manual only.",
    mirrors = [
        "ci/dawn-linux-x86-fuzz-dbg",
    ],
    gn_args = "ci/dawn-linux-x86-fuzz-dbg",
)

dawn_linux_manual_builder(
    name = "dawn-try-linux-x86-fuzz-rel",
    description_html = "Runs release Dawn fuzz tests on Linux/x86. Manual only.",
    mirrors = [
        "ci/dawn-linux-x86-fuzz-rel",
    ],
    gn_args = "ci/dawn-linux-x86-fuzz-rel",
)
