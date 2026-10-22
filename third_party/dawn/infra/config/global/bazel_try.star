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

"""Try Dawn builders using Bazel for the build system instead of GN or CMake."""

load("@chromium-luci//try.star", "try_")
load("//bazel_shared.star", "bazel_builder_defaults")
load("//constants.star", "siso")
load("//location_filters.star", "inclusion_filters")

try_.defaults.set(
    executable = "recipe:dawn/bazel",
    builder_group = "try",
    bucket = "try",
    pool = "luci.chromium.gpu.try",
    builderless = True,
    build_numbers = True,
    list_view = "try",
    contact_team_email = "chrome-gpu-infra@google.com",
    service_account = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
    siso_project = siso.project.DEFAULT_UNTRUSTED,
    siso_remote_jobs = siso.remote_jobs.DEFAULT,
)

## Templates

def bazel_try_builder(**kwargs):
    """Declares a try builder and registers it as an optional verifier on CQ.

    Args:
        **kwargs: Arguments to pass to try_.builder.
    """
    kwargs.setdefault("max_concurrent_builds", 3)
    kwargs.setdefault("cq_settings", try_.cq_settings(
        location_filters = inclusion_filters.bazel_cq_file_inclusions,
    ))
    name = kwargs["name"]
    try_.builder(**kwargs)
    luci.cq_tryjob_verifier(
        cq_group = "Dawn-CQ",
        builder = "dawn:try/" + name,
        experiment_percentage = 0,
        location_filters = inclusion_filters.bazel_cq_file_inclusions,
    )

def bazel_linux_try_builder(**kwargs):
    kwargs = bazel_builder_defaults.apply_linux_bazel_builder_defaults(kwargs)
    bazel_try_builder(**kwargs)

def bazel_mac_try_builder(**kwargs):
    kwargs = bazel_builder_defaults.apply_mac_bazel_builder_defaults(kwargs)
    bazel_try_builder(**kwargs)

## CQ Builders

bazel_linux_try_builder(
    name = "dawn-cq-linux-x64-bazel-dbg",
    description_html = "Compiles Tint targets for Linux/x64 using Bazel in debug mode.",
    properties = {
        "debug": True,
    },
    mirrors = [
        "ci/dawn-linux-x64-bazel-dbg",
    ],
)

bazel_linux_try_builder(
    name = "dawn-cq-linux-x64-bazel-rel",
    description_html = "Compiles Tint targets for Linux/x64 using Bazel in release mode.",
    properties = {
        "debug": False,
    },
    mirrors = [
        "ci/dawn-linux-x64-bazel-rel",
    ],
)

bazel_mac_try_builder(
    name = "dawn-cq-mac-arm64-bazel-dbg",
    description_html = "Compiles Tint targets for Mac/arm64 using Bazel in debug mode.",
    properties = {
        "debug": True,
    },
    mirrors = [
        "ci/dawn-mac-arm64-bazel-dbg",
    ],
)

bazel_mac_try_builder(
    name = "dawn-cq-mac-arm64-bazel-rel",
    description_html = "Compiles Tint targets for Mac/arm64 using Bazel in release mode.",
    properties = {
        "debug": False,
    },
    mirrors = [
        "ci/dawn-mac-arm64-bazel-rel",
    ],
)
