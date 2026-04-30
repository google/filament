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

"""Legacy builder definitions that were originally in main.star.

Moved due to them causing issues while working on crbug.com/385317083. Any
builders that will be kept after that migration should be grouped and moved into
appropriately named files.
"""

load("//project.star", "ACTIVE_MILESTONES")

os_category = struct(
    LINUX = "Linux",
    MAC = "Mac",
    WINDOWS = "Windows",
)

def os_enum(category, console_name):
    return struct(category = category, console_name = console_name)

os = struct(
    LINUX = os_enum(os_category.LINUX, "linux"),
    MAC = os_enum(os_category.MAC, "mac"),
    WINDOWS = os_enum(os_category.WINDOWS, "win"),
)

def get_dimension(os):
    """Returns the dimension to use for the input os.

    Args:
        os: An os enum to check against.

    Returns:
        A string containing the dimensions the given OS should target.
    """
    if os.category == os_category.LINUX:
        return "Ubuntu-24.04"
    elif os.category == os_category.MAC:
        return "Mac-12|Mac-13|Mac-14|Mac-15"
    elif os.category == os_category.WINDOWS:
        return "Windows-10"

    return "Invalid Dimension"

luci.notifier(
    name = "gardener-notifier",
    notify_rotation_urls = [
        "https://chrome-ops-rotation-proxy.appspot.com/current/grotation:webgpu-gardener",
    ],
    on_occurrence = ["FAILURE", "INFRA_FAILURE"],
)

# Recipes

def clang_tidy_dawn_tryjob():
    """Adds a tryjob that runs clang tidy on new patchset upload."""
    luci.cq_tryjob_verifier(
        cq_group = "Dawn-CQ",
        builder = "chromium:try/tricium-clang-tidy",
        owner_whitelist = ["project-dawn-tryjob-access"],
        experiment_percentage = 100,
        disable_reuse = True,
        mode_allowlist = [cq.MODE_NEW_PATCHSET_RUN],
        location_filters = [
            cq.location_filter(path_regexp = r".+\.h"),
            cq.location_filter(path_regexp = r".+\.c"),
            cq.location_filter(path_regexp = r".+\.cc"),
            cq.location_filter(path_regexp = r".+\.cpp"),
        ],
    )

luci.list_view_entry(
    list_view = "try",
    builder = "try/presubmit",
)

luci.builder(
    name = "presubmit",
    bucket = "try",
    executable = "recipe:run_presubmit",
    dimensions = {
        "cpu": "x86-64",
        "os": get_dimension(os.LINUX),
        "pool": "luci.flex.try",
    },
    properties = {
        "repo_name": "dawn",
        "runhooks": True,
    },
    service_account = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
)

# The following standalone builders have been replaced with functionally
# equivalent ones using the gn_v2 recipe. See crbug.com/385317083.
# * cron-linux-clang-rel-x64
#   * dawn-linux-x64-sws-clusterfuzz
# * linux-clang-dbg-x64
#   * dawn-cq-linux-x64-dbg
#   * dawn-cq-linux-x64-fuzz-dbg
# * linux-clang-dbg-x86
#   * dawn-cq-linux-x86-dbg
#   * dawn-cq-linux-x86-fuzz-dbg
# * linux-clang-rel-x64
#   * dawn-cq-linux-x64-rel
#   * dawn-cq-linux-x64-fuzz-rel
# * linux-clang-rel-x86
#   * dawn-cq-linux-x86-rel
#   * dawn-cq-linux-x86-fuzz-rel
# * mac-dbg
#   * dawn-cq-mac-x64-dbg
# * mac-rel
#   * dawn-cq-mac-x64-rel
# * win-clang-dbg-x64
#   * dawn-cq-win-x64-dbg
# * win-clang-dbg-x86
#   * dawn-cq-win-x86-dbg
# * win-clang-rel-x64
#   * dawn-cq-win-x64-rel
# * win-clang-rel-x86
#   * dawn-cq-win-x86-rel
# * win-msvc-dbg-x64
#   * dawn-cq-win-x64-msvc-dbg
# * win-msvc-rel-x64
#   * dawn-cq-win-x64-msvc-rel

# The following CMake builders have been replaced with functionally equivalent
# ones defined using chromium-luci code. See crbug.com/459517292.
# * cmake-linux-clang-dbg-x64
#   * dawn-linux-x64-sws-cmake-dbg
#   * dawn-cq-linux-x64-sws-cmake-dbg
# * cmake-linux-clang-rel-x64-asan
#   * dawn-linux-x64-sws-cmake-asan
#   * dawn-cq-linux-x64-sws-cmake-asan
# * cmake-linux-clang-rel-x64-ubsan
#   * dawn-linux-x64-sws-cmake-asan
#   * dawn-cq-linux-x64-sws-cmake-asan
# * cmake-linux-clang-rel-x64
#   * dawn-linux-x64-sws-cmake-rel
#   * dawn-cq-linux-x64-sws-cmake-rel
# * cmake-mac-dbg
#   * dawn-mac-x64-sws-cmake-dbg
#   * dawn-cq-mac-x64-sws-cmake-rel
# * cmake-mac-rel
#   * dawn-mac-x64-sws-cmake-rel
#   * dawn-cq-mac-x64-sws-cmake-rel
# * cmake-win-msvc-dbg-x64
#   * dawn-win-x64-sws-msvc-cmake-dbg
#   * dawn-cq-win-x64-msvc-cmake-dbg
# * cmake-win-msvc-rel-x64
#   * dawn-win-x64-sws-msvc-cmake-rel
#   * dawn-cq-win-x64-msvc-cmake-rel

# The following CMake builders have been removed due to deciding that they were
# not providing value in go/dawn-standalone-builders-dd.
# * cmake-linux-clang-dbg-x64-asan
# * cmake-linux-clang-dbg-x64-ubsan

clang_tidy_dawn_tryjob()

# CQ

luci.cq(
    status_host = "chromium-cq-status.appspot.com",
    submit_max_burst = 4,
    submit_burst_delay = 480 * time.second,
    gerrit_listener_type = cq.GERRIT_LISTENER_TYPE_LEGACY_POLLER,
)

def _create_dawn_cq_group(name, refs, refs_exclude = None):
    luci.cq_group(
        name = name,
        watch = cq.refset(
            "https://dawn.googlesource.com/dawn",
            refs = refs,
            refs_exclude = refs_exclude,
        ),
        acls = [
            acl.entry(
                acl.CQ_COMMITTER,
                groups = "project-dawn-submit-access",
            ),
            acl.entry(
                acl.CQ_DRY_RUNNER,
                groups = "project-dawn-tryjob-access",
            ),
            acl.entry(
                acl.CQ_NEW_PATCHSET_RUN_TRIGGERER,
                groups = "project-dawn-tryjob-access",
            ),
        ],
        verifiers = [
            luci.cq_tryjob_verifier(
                builder = "dawn:try/presubmit",
                disable_reuse = True,
            ),
        ],
        retry_config = cq.retry_config(
            single_quota = 1,
            global_quota = 2,
            failure_weight = 1,
            transient_failure_weight = 1,
            timeout_weight = 2,
        ),
        user_limit_default = cq.user_limit(
            name = "default-limit",
            run = cq.run_limits(max_active = 4),
        ),
    )

def _create_branch_groups():
    for milestone, details in ACTIVE_MILESTONES.items():
        _create_dawn_cq_group(
            "Dawn-CQ-" + milestone,
            [details.ref],
        )

_create_dawn_cq_group(
    "Dawn-CQ",
    ["refs/heads/.+"],
    [details.ref for details in ACTIVE_MILESTONES.values()],
)
_create_branch_groups()
