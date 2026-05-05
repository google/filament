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

"""CI Dawn builders using CMake for the build system instead of GN."""

load("@chromium-luci//builder_config.star", "builder_config")
load("@chromium-luci//ci.star", "ci")
load("@chromium-luci//consoles.star", "consoles")
load("@chromium-luci//gardener_rotations.star", "gardener_rotations")
load("//cmake_shared.star", "cmake_builder_defaults")
load("//constants.star", "siso")

ci.defaults.set(
    executable = "recipe:dawn/cmake",
    builder_group = "ci",
    bucket = "ci",
    pool = "luci.chromium.gpu.ci",
    builderless = True,
    triggered_by = ["primary-poller"],
    build_numbers = True,
    contact_team_email = "chrome-gpu-infra@google.com",
    service_account = "dawn-ci-builder@chops-service-accounts.iam.gserviceaccount.com",
    shadow_service_account = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
    siso_project = siso.project.DEFAULT_TRUSTED,
    shadow_siso_project = siso.project.DEFAULT_UNTRUSTED,
    siso_remote_jobs = siso.remote_jobs.DEFAULT,
    notifies = ["gardener-notifier"],
    gardener_rotations = gardener_rotations.rotation("dawn", None, None),
)

def dawn_ci_linux_cmake_builder(**kwargs):
    kwargs = cmake_builder_defaults.apply_linux_cmake_builder_defaults(kwargs)

    # TODO(crbug.com/459517292): Remove this and rely on file-wide defaults
    # once we move Linux CMake builders into the luci.chromium.gpu.* pools.
    kwargs.setdefault("pool", "luci.flex.ci")
    ci.builder(**kwargs)

def dawn_ci_mac_cmake_builder(**kwargs):
    """Adds a Dawn/Mac/CMake CI builder.

    Args:
        **kwargs: Builder arguments to forward on to ci.builder()
    """
    kwargs = cmake_builder_defaults.apply_mac_cmake_builder_defaults(kwargs)
    ci.builder(**kwargs)

def dawn_ci_win_cmake_builder(**kwargs):
    """Adds a Dawn/Win/CMake CI builder.

    Args:
        **kwargs: Builder arguments to forward on to ci.builder()
    """
    kwargs = cmake_builder_defaults.apply_win_cmake_builder_defaults(kwargs)
    ci.builder(**kwargs)

dawn_ci_linux_cmake_builder(
    name = "dawn-linux-x64-sws-cmake-dbg",
    description_html = "Compiles and tests debug Dawn test binaries for Linux/x64 using CMake and Clang",
    schedule = "triggered",
    properties = {
        "asan": False,
        "clang": True,
        "debug": True,
        "target_cpu": "x64",
        "ubsan": False,
    },
    # Not actually used by the recipe, but needed for chromium-luci mirroring
    # code to work.
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|cmake|dbg",
        short_name = "x64",
    ),
)

dawn_ci_linux_cmake_builder(
    name = "dawn-linux-x64-sws-cmake-rel",
    description_html = "Compiles and tests release Dawn test binaries for Linux/x64 using CMake and Clang",
    schedule = "triggered",
    properties = {
        "asan": False,
        "clang": True,
        "debug": False,
        "target_cpu": "x64",
        "ubsan": False,
    },
    # Not actually used by the recipe, but needed for chromium-luci mirroring
    # code to work.
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|cmake|rel",
        short_name = "x64",
    ),
)

dawn_ci_linux_cmake_builder(
    name = "dawn-linux-x64-sws-cmake-asan",
    description_html = "Compiles and tests release Dawn test binaries for Linux/x64 using CMake and Clang with ASan and UBSan enabled",
    schedule = "triggered",
    properties = {
        "asan": True,
        "clang": True,
        "debug": False,
        "target_cpu": "x64",
        "ubsan": True,
    },
    # Not actually used by the recipe, but needed for chromium-luci mirroring
    # code to work.
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|cmake|asan",
        short_name = "x64",
    ),
)

dawn_ci_mac_cmake_builder(
    name = "dawn-mac-x64-sws-cmake-dbg",
    description_html = "Compiles and tests debug Dawn test binaries for Mac/x64 using CMake and Clang",
    schedule = "triggered",
    properties = {
        "asan": False,
        "clang": True,
        "debug": True,
        "target_cpu": "x64",
        "ubsan": False,
    },
    # Not actually used by the recipe, but needed for chromium-luci mirroring
    # code to work.
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    console_view_entry = consoles.console_view_entry(
        category = "mac|build|clang|cmake|dbg",
        short_name = "x64",
    ),
)

dawn_ci_mac_cmake_builder(
    name = "dawn-mac-x64-sws-cmake-rel",
    description_html = "Compiles and tests release Dawn test binaries for Mac/x64 using CMake and Clang",
    schedule = "triggered",
    properties = {
        "asan": False,
        "clang": True,
        "debug": False,
        "target_cpu": "x64",
        "ubsan": False,
    },
    # Not actually used by the recipe, but needed for chromium-luci mirroring
    # code to work.
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    console_view_entry = consoles.console_view_entry(
        category = "mac|build|clang|cmake|rel",
        short_name = "x64",
    ),
)

dawn_ci_win_cmake_builder(
    name = "dawn-win-x64-sws-msvc-cmake-dbg",
    description_html = "Compiles and runs debug Dawn test binaries for Win/x64 using CMake and MSVC",
    schedule = "triggered",
    properties = {
        "asan": False,
        "clang": False,
        "debug": True,
        "target_cpu": "x64",
        "ubsan": False,
    },
    # Not actually used by the recipe, but needed for chromium-luci mirroring
    # code to work.
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|build|msvc|cmake|dbg",
        short_name = "x64",
    ),
)

dawn_ci_win_cmake_builder(
    name = "dawn-win-x64-sws-msvc-cmake-rel",
    description_html = "Compiles and runs release Dawn test binaries for Win/x64 using CMake and MSVC",
    schedule = "triggered",
    properties = {
        "asan": False,
        "clang": False,
        "debug": False,
        "target_cpu": "x64",
        "ubsan": False,
    },
    # Not actually used by the recipe, but needed for chromium-luci mirroring
    # code to work.
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
            apply_configs = [],
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
    ),
    console_view_entry = consoles.console_view_entry(
        category = "win|build|msvc|cmake|rel",
        short_name = "x64",
    ),
)
