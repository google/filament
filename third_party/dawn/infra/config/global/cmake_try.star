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

"""Try Dawn builders using CMake for the build system instead of GN."""

load("@chromium-luci//try.star", "try_")
load("//cmake_shared.star", "cmake_builder_defaults")
load("//constants.star", "siso")
load("//location_filters.star", "exclusion_filters")
load("//project.star", "ACTIVE_MILESTONES")

try_.defaults.set(
    executable = "recipe:dawn/cmake",
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

## Templates

def apply_cq_builder_defaults(kwargs):
    # There are fewer optimizations such as the use of `gn analyze` for CMake
    # builders, so allow more concurrent builds than GN equivalents.
    kwargs.setdefault("max_concurrent_builds", 5)
    kwargs.setdefault("tryjob", try_.job(
        location_filters = exclusion_filters.cmake_cq_file_exclusions,
    ))
    return kwargs

def apply_linux_cq_builder_defaults(kwargs):
    """Sets default arguments for Linux CMake CQ builders.

    Args:
        kwargs: The kwargs for creating a try builder.

    Returns:
        |kwargs| with Linux/CMake defaults set.
    """
    kwargs = cmake_builder_defaults.apply_linux_cmake_builder_defaults(kwargs)
    kwargs = apply_cq_builder_defaults(kwargs)

    # TODO(crbug.com/459517292): Remove this and rely on file-wide defaults
    # once we move Linux CMake builders into the luci.chromium.gpu.* pools.
    kwargs.setdefault("pool", "luci.flex.try")
    return kwargs

def apply_mac_cq_builder_defaults(kwargs):
    """Sets default arguments for Mac CMake CQ builders.

    Args:
        kwargs: The kwargs for creating a try builder.

    Returns:
        |kwargs| with Mac/CMake defaults set.
    """
    kwargs = cmake_builder_defaults.apply_mac_cmake_builder_defaults(kwargs)
    kwargs = apply_cq_builder_defaults(kwargs)
    return kwargs

def apply_win_cq_builder_defaults(kwargs):
    """Sets default arguments for Win CMake CQ builders.

    Args:
        kwargs: The kwargs for creating a try builder.

    Returns:
        |kwargs| with Win/CMake defaults set.
    """
    kwargs = cmake_builder_defaults.apply_win_cmake_builder_defaults(kwargs)
    kwargs = apply_cq_builder_defaults(kwargs)
    return kwargs

def add_builder_to_main_and_milestone_cq_groups(kwargs):
    # Dawn standalone builders run fine unbranched on branched CLs.
    try_.builder(**kwargs)
    for milestone in ACTIVE_MILESTONES.keys():
        # TODO(crbug.com/459517292): Figure out why the legacy builders were
        # marked as experimental and remove the need for that.
        luci.cq_tryjob_verifier(
            cq_group = "Dawn-CQ-" + milestone,
            builder = "dawn:try/" + kwargs["name"],
            experiment_percentage = 100,
        )

def dawn_linux_cmake_cq_tester(**kwargs):
    kwargs = apply_linux_cq_builder_defaults(kwargs)
    add_builder_to_main_and_milestone_cq_groups(kwargs)

def dawn_mac_cmake_cq_tester(**kwargs):
    kwargs = apply_mac_cq_builder_defaults(kwargs)
    add_builder_to_main_and_milestone_cq_groups(kwargs)

def dawn_win_cmake_cq_tester(**kwargs):
    kwargs = apply_win_cq_builder_defaults(kwargs)
    add_builder_to_main_and_milestone_cq_groups(kwargs)

## CQ Builders

dawn_linux_cmake_cq_tester(
    name = "dawn-cq-linux-x64-cmake-asan",
    description_html = "Compiles and tests release Dawn test binaries for Linux/x64 using CMake and Clang with ASan and UBSan enabled. Blocks CL submission",
    mirrors = [
        "ci/dawn-linux-x64-sws-cmake-asan",
    ],
    properties = {
        "asan": True,
        "clang": True,
        "debug": False,
        "target_cpu": "x64",
        "ubsan": True,
    },
)

dawn_linux_cmake_cq_tester(
    name = "dawn-cq-linux-x64-cmake-dbg",
    description_html = "Compiles and tests debug Dawn test binaries for Linux/x64 using CMake and Clang. Blocks CL submission",
    mirrors = [
        "ci/dawn-linux-x64-sws-cmake-dbg",
    ],
    properties = {
        "asan": False,
        "clang": True,
        "debug": True,
        "target_cpu": "x64",
        "ubsan": False,
    },
)

dawn_linux_cmake_cq_tester(
    name = "dawn-cq-linux-x64-cmake-rel",
    description_html = "Compiles and tests release Dawn test binaries for Linux/x64 using CMake and Clang. Blocks CL submission",
    mirrors = [
        "ci/dawn-linux-x64-sws-cmake-rel",
    ],
    properties = {
        "asan": False,
        "clang": True,
        "debug": False,
        "target_cpu": "x64",
        "ubsan": False,
    },
)

dawn_mac_cmake_cq_tester(
    name = "dawn-cq-mac-x64-cmake-dbg",
    description_html = "Compiles and tests debug Dawn test binaries for Mac/x64 using CMake and Clang. Blocks CL submission",
    mirrors = [
        "ci/dawn-mac-x64-sws-cmake-dbg",
    ],
    properties = {
        "asan": False,
        "clang": True,
        "debug": True,
        "target_cpu": "x64",
        "ubsan": False,
    },
)

dawn_mac_cmake_cq_tester(
    name = "dawn-cq-mac-x64-cmake-rel",
    description_html = "Compiles and tests release Dawn test binaries for Mac/x64 using CMake and Clang. Blocks CL submission",
    mirrors = [
        "ci/dawn-mac-x64-sws-cmake-rel",
    ],
    properties = {
        "asan": False,
        "clang": True,
        "debug": False,
        "target_cpu": "x64",
        "ubsan": False,
    },
)

dawn_win_cmake_cq_tester(
    name = "dawn-cq-win-x64-msvc-cmake-dbg",
    description_html = "Compiles and tests debug Dawn test binaries for Win/x64 using CMake and MSVC. Blocks CL submission",
    mirrors = [
        "ci/dawn-win-x64-sws-msvc-cmake-dbg",
    ],
    properties = {
        "asan": False,
        "clang": False,
        "debug": True,
        "target_cpu": "x64",
        "ubsan": False,
    },
)

dawn_win_cmake_cq_tester(
    name = "dawn-cq-win-x64-msvc-cmake-rel",
    description_html = "Compiles and tests release Dawn test binaries for Win/x64 using CMake and MSVC. Blocks CL submission",
    mirrors = [
        "ci/dawn-win-x64-sws-msvc-cmake-rel",
    ],
    properties = {
        "asan": False,
        "clang": False,
        "debug": False,
        "target_cpu": "x64",
        "ubsan": False,
    },
)
