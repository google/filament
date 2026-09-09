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

"""CI Dawn builders using Bazel for the build system instead of GN or CMake."""

load("@chromium-luci//builder_config.star", "builder_config")
load("@chromium-luci//ci.star", "ci")
load("@chromium-luci//consoles.star", "consoles")
load("@chromium-luci//gardener_rotations.star", "gardener_rotations")
load("//bazel_shared.star", "bazel_builder_defaults")
load("//constants.star", "siso")

ci.defaults.set(
    executable = "recipe:dawn/bazel",
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

def dawn_ci_linux_bazel_builder(**kwargs):
    kwargs = bazel_builder_defaults.apply_linux_bazel_builder_defaults(kwargs)
    ci.builder(**kwargs)

def dawn_ci_mac_bazel_builder(**kwargs):
    kwargs = bazel_builder_defaults.apply_mac_bazel_builder_defaults(kwargs)
    ci.builder(**kwargs)

## CI Builders

dawn_ci_linux_bazel_builder(
    name = "dawn-linux-x64-bazel-dbg",
    description_html = "Compiles Tint targets for Linux/x64 using Bazel in Debug mode",
    properties = {
        "debug": True,
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
        category = "linux|build|clang|bazel|dbg",
        short_name = "x64",
    ),
)

dawn_ci_linux_bazel_builder(
    name = "dawn-linux-x64-bazel-rel",
    description_html = "Compiles Tint targets for Linux/x64 using Bazel",
    properties = {
        "debug": False,
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
        category = "linux|build|clang|bazel|rel",
        short_name = "x64",
    ),
)

dawn_ci_mac_bazel_builder(
    name = "dawn-mac-arm64-bazel-dbg",
    description_html = "Compiles Tint targets for Mac/arm64 using Bazel in Debug mode",
    properties = {
        "debug": True,
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
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    console_view_entry = consoles.console_view_entry(
        category = "mac|build|clang|bazel|dbg",
        short_name = "a64",
    ),
)

dawn_ci_mac_bazel_builder(
    name = "dawn-mac-arm64-bazel-rel",
    description_html = "Compiles Tint targets for Mac/arm64 using Bazel",
    properties = {
        "debug": False,
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
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.MAC,
        ),
    ),
    console_view_entry = consoles.console_view_entry(
        category = "mac|build|clang|bazel|rel",
        short_name = "a64",
    ),
)
