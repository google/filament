#!/usr/bin/env lucicfg
#
# Copyright 2021 The Dawn & Tint Authors
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

"""
main.star: lucicfg configuration for Dawn's standalone builers.
"""

load("//project.star", "ACTIVE_MILESTONES")

# Use LUCI Scheduler BBv2 names and add Scheduler realms configs.
lucicfg.enable_experiment("crbug.com/1182002")

lucicfg.config(fail_on_warnings = True)

luci.project(
    name = "dawn",
    buildbucket = "cr-buildbucket.appspot.com",
    logdog = "luci-logdog.appspot.com",
    milo = "luci-milo.appspot.com",
    notify = "luci-notify.appspot.com",
    scheduler = "luci-scheduler.appspot.com",
    swarming = "chromium-swarm.appspot.com",
    acls = [
        acl.entry(
            roles = [
                acl.PROJECT_CONFIGS_READER,
                acl.LOGDOG_READER,
                acl.BUILDBUCKET_READER,
                acl.SCHEDULER_READER,
            ],
            groups = "all",
        ),
        acl.entry(
            roles = [
                acl.SCHEDULER_OWNER,
            ],
            groups = [
                "project-dawn-admins",
                "project-dawn-schedulers",
            ],
        ),
        acl.entry(
            roles = [
                acl.LOGDOG_WRITER,
            ],
            groups = "luci-logdog-chromium-writers",
        ),
    ],
    bindings = [
        luci.binding(
            roles = "role/configs.validator",
            users = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
        ),
        luci.binding(
            roles = "role/swarming.taskServiceAccount",
            users = "dawn-automated-expectations@chops-service-accounts.iam.gserviceaccount.com",
        ),
    ],
)

luci.logdog(gs_bucket = "chromium-luci-logdog")

luci.bucket(
    name = "ci",
    acls = [
        acl.entry(
            roles = [
                acl.BUILDBUCKET_READER,
            ],
            groups = "all",
        ),
        acl.entry(
            acl.BUILDBUCKET_TRIGGERER,
        ),
    ],
)

# Allow LED users to trigger swarming tasks directly when debugging ci
# builders.
luci.binding(
    realm = "ci",
    roles = "role/swarming.taskTriggerer",
    groups = "flex-ci-led-users",
)

luci.bucket(
    name = "try",
    acls = [
        acl.entry(
            acl.BUILDBUCKET_TRIGGERER,
            groups = [
                "project-dawn-tryjob-access",
                "service-account-cq",
            ],
        ),
    ],
)

# Allow LED users to trigger swarming tasks directly when debugging try
# builders.
luci.binding(
    realm = "try",
    roles = "role/swarming.taskTriggerer",
    groups = "flex-try-led-users",
)

# Shadow buckets for LED jobs.
luci.bucket(
    name = "ci.shadow",
    shadows = "ci",
    constraints = luci.bucket_constraints(
        pools = ["luci.flex.ci"],
    ),
    bindings = [
        luci.binding(
            roles = "role/buildbucket.creator",
            groups = [
                "mdb/chrome-build-access-sphinx",
                "mdb/chrome-troopers",
                "chromium-led-users",
                "flex-ci-led-users",
            ],
            users = [
                "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
            ],
        ),
        luci.binding(
            roles = "role/buildbucket.triggerer",
            users = [
                "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
            ],
        ),
        # Allow ci builders to create invocations in their own builds.
        luci.binding(
            roles = "role/resultdb.invocationCreator",
            users = [
                "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
            ],
        ),
    ],
    dynamic = True,
)

luci.bucket(
    name = "try.shadow",
    shadows = "try",
    constraints = luci.bucket_constraints(
        pools = ["luci.flex.try"],
        service_accounts = [
            "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
        ],
    ),
    bindings = [
        luci.binding(
            roles = "role/buildbucket.creator",
            groups = [
                "mdb/chrome-build-access-sphinx",
                "mdb/chrome-troopers",
                "chromium-led-users",
                "flex-ci-led-users",
            ],
            users = [
                "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
            ],
        ),
        luci.binding(
            roles = "role/buildbucket.triggerer",
            users = [
                "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
            ],
        ),
        # Allow try builders to create invocations in their own builds.
        luci.binding(
            roles = "role/resultdb.invocationCreator",
            groups = [
                "project-dawn-tryjob-access",
            ],
            users = [
                "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
            ],
        ),
    ],
    dynamic = True,
)

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

def get_dimension(os, builder_name = None):
    """Returns the dimension to use for the input os and optional builder name"""
    if os.category == os_category.LINUX:
        return "Ubuntu-22.04"
    elif os.category == os_category.MAC:
        return "Mac-11|Mac-12|Mac-13|Mac-14"
    elif os.category == os_category.WINDOWS:
        return "Windows-10"

    return "Invalid Dimension"

reclient = struct(
    instance = struct(
        DEFAULT_TRUSTED = "rbe-chromium-trusted",
        DEFAULT_UNTRUSTED = "rbe-chromium-untrusted",
    ),
    jobs = struct(
        HIGH_JOBS_FOR_CI = 250,
        LOW_JOBS_FOR_CQ = 150,
    ),
)

# File exclusion filters meant for use on cmake and msvc trybots since these
# files do not affect compilation for either.
cmake_msvc_file_exclusions = [
    # WebGPU CTS expectations, only affects builders that run WebGPU CTS.
    cq.location_filter(
        path_regexp = "webgpu-cts/[^/]*expectations.txt",
        exclude = True,
    ),
    # Tools written in Go.
    cq.location_filter(
        path_regexp = "tools/src/.+",
        exclude = True,
    ),
    # Go dependencies.
    cq.location_filter(
        path_regexp = "go\\.(mod|sum)",
        exclude = True,
    ),
]

luci.notifier(
    name = "gardener-notifier",
    notify_rotation_urls = [
        "https://chrome-ops-rotation-proxy.appspot.com/current/grotation:webgpu-gardener",
    ],
    on_occurrence = ["FAILURE", "INFRA_FAILURE"],
)

# Recipes

def get_builder_executable(use_gn):
    """Get standard executable for builders

    Returns:
      A luci.recipe
    """
    return luci.recipe(
        name = "dawn/gn" if use_gn else "dawn/cmake",
        cipd_package = "infra/recipe_bundles/chromium.googlesource.com/chromium/tools/build",
        cipd_version = "refs/heads/main",
    )

def get_presubmit_executable():
    """Get standard executable for presubmit

    Returns:
      A luci.recipe
    """
    return luci.recipe(
        name = "run_presubmit",
        cipd_package = "infra/recipe_bundles/chromium.googlesource.com/chromium/tools/build",
        cipd_version = "refs/heads/main",
    )

def get_os_from_arg(arg):
    """Get OS enum for a builder name string

    Args:
      arg: builder name string to get enum for

    Returns:
      An OS enum struct

    """

    if arg.find("linux") != -1:
        return os.LINUX
    if arg.find("win") != -1:
        return os.WINDOWS
    if arg.find("mac") != -1:
        return os.MAC
    return os.MAC

def get_default_caches(os, clang):
    """Get standard caches for builders

    Args:
      os: OS enum for the builder
      clang: is this builder running clang

    Returns:
      A list of caches
    """
    caches = []
    if os.category == os_category.MAC:
        # Cache for mac_toolchain tool and XCode.app
        caches.append(swarming.cache(name = "osx_sdk", path = "osx_sdk"))
    elif os.category == os_category.WINDOWS:
        # Cache for win_toolchain tool
        caches.append(swarming.cache(name = "win_toolchain", path = "win_toolchain"))

    return caches

def get_default_dimensions(os, builder_name):
    """Get dimensions for a builder that don't depend on being CI vs Try

    Args:
      os: OS enum for the builder

    Returns:
      A dimension dict
    """
    dimensions = {}

    # We have 32bit test configurations but some of our toolchain is 64bit (like CIPD)
    dimensions["cpu"] = "x86-64"
    dimensions["os"] = get_dimension(os, builder_name)

    return dimensions

def get_common_properties(os, clang, reclient_instance, reclient_jobs):
    """Add the common properties for a builder that don't depend on being CI vs Try

    Args:
      os: OS enum for the builder

    Returns:
      A properties dict
    """
    properties = {}
    msvc = os.category == os_category.WINDOWS and not clang

    if not msvc:
        reclient_props = {
            "instance": reclient_instance,
            "jobs": reclient_jobs,
            "metrics_project": "chromium-reclient-metrics",
            "scandeps_server": True,
        }
        properties["$build/reclient"] = reclient_props

    return properties

def add_ci_builder(name, os, properties):
    """Add a CI builder

    Args:
      name: builder's name in string form
      os: OS enum for the builder
      properties: properties dictionary
    """
    clang = properties["clang"]
    fuzzer = ("gen_fuzz_corpus" in properties) and properties["gen_fuzz_corpus"]

    dimensions_ci = get_default_dimensions(os, name)
    dimensions_ci["pool"] = "luci.flex.ci"
    properties_ci = get_common_properties(
        os,
        clang,
        reclient.instance.DEFAULT_TRUSTED,
        reclient.jobs.HIGH_JOBS_FOR_CI,
    )
    properties_ci.update(properties)
    schedule_ci = None
    if fuzzer:
        schedule_ci = "0 0 0 * * * *"
    triggered_by_ci = None
    if not fuzzer:
        triggered_by_ci = ["primary-poller"]
    luci.builder(
        name = name,
        bucket = "ci",
        schedule = schedule_ci,
        triggered_by = triggered_by_ci,
        executable = get_builder_executable(use_gn = "cmake" not in name),
        properties = properties_ci,
        dimensions = dimensions_ci,
        caches = get_default_caches(os, clang),
        notifies = ["gardener-notifier"],
        service_account = "dawn-ci-builder@chops-service-accounts.iam.gserviceaccount.com",
        shadow_service_account = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
    )

def add_try_builder(name, os, properties):
    """Add a Try builder

    Args:
      name: builder's name in string form
      os: OS enum for the builder
      properties: properties dictionary
    """
    clang = properties["clang"]

    dimensions_try = get_default_dimensions(os, name)
    dimensions_try["pool"] = "luci.flex.try"
    properties_try = get_common_properties(
        os,
        clang,
        reclient.instance.DEFAULT_UNTRUSTED,
        reclient.jobs.LOW_JOBS_FOR_CQ,
    )
    properties_try.update(properties)
    luci.builder(
        name = name,
        bucket = "try",
        executable = get_builder_executable(use_gn = "cmake" not in name),
        properties = properties_try,
        dimensions = dimensions_try,
        caches = get_default_caches(os, clang),
        service_account = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
    )

def dawn_standalone_builder(name, clang, debug, cpu, fuzzer):
    """Adds both the CI and Try standalone builders as appropriate

    Args:
      name: builder's name in string form
      clang: is this builder running clang
      debug: is this builder generating debug builds
      cpu: string representing the target CPU architecture
      fuzzer: enable building fuzzer corpus

    """
    os = get_os_from_arg(name)

    properties = {
        "clang": clang,
        "debug": debug,
        "target_cpu": cpu,
        "gen_fuzz_corpus": fuzzer,
    }

    add_ci_builder(name, os, properties)
    if not fuzzer:
        add_try_builder(name, os, properties)

    config = ""
    if clang:
        config = "clang"
    elif os.category == os_category.WINDOWS:
        config = "msvc"

    category = ""
    if fuzzer:
        category += "cron|"
    category += os.console_name

    if os.category != os_category.MAC:
        category += "|" + config
        if config != "msvc":
            category += "|dbg" if debug else "|rel"

    short_name = "dbg" if debug else "rel"
    if os.category != os_category.MAC:
        if config != "msvc":
            short_name = cpu

    luci.console_view_entry(
        console_view = "ci",
        builder = "ci/" + name,
        category = category,
        short_name = short_name,
    )

    if not fuzzer:
        luci.list_view_entry(
            list_view = "try",
            builder = "try/" + name,
        )

        additional_filters = []
        if config == "msvc":
            additional_filters = cmake_msvc_file_exclusions

        luci.cq_tryjob_verifier(
            cq_group = "Dawn-CQ",
            builder = "dawn:try/" + name,
            location_filters = [
                cq.location_filter(path_regexp = ".*"),
                cq.location_filter(
                    path_regexp = "\\.github/.+",
                    exclude = True,
                ),
            ] + additional_filters,
        )

        # These builders run fine unbranched on branch CLs, so add them to the
        # branch groups as well.
        for milestone in ACTIVE_MILESTONES.keys():
            luci.cq_tryjob_verifier(
                cq_group = "Dawn-CQ-" + milestone,
                builder = "dawn:try/" + name,
            )

def dawn_cmake_standalone_builder(name, clang, debug, cpu, asan, ubsan, experimental = False):
    """Adds both the CI and Try standalone builders as appropriate for the CMake build

    Args:
      name: builder's name in string form
      clang: is this builder running clang
      debug: is this builder generating debug builds
      cpu: string representing the target CPU architecture
      asan: is this builder building with asan enabled
      ubsan: is this builder building with ubsan enabled
    """
    os = get_os_from_arg(name)

    properties = {
        "clang": clang,
        "debug": debug,
        "target_cpu": cpu,
        "asan": asan,
        "ubsan": ubsan,
    }

    add_ci_builder(name, os, properties)
    add_try_builder(name, os, properties)

    config = ""
    if clang:
        config = "clang"
    elif os.category == os_category.WINDOWS:
        config = "msvc"

    category = ""
    category += os.console_name

    if os.category != os_category.MAC:
        category += "|" + config
        if config != "msvc":
            category += "|dbg" if debug else "|rel"

    short_name = "dbg" if debug else "rel"
    if os.category != os_category.MAC:
        if config != "msvc":
            short_name = cpu

    luci.console_view_entry(
        console_view = "ci",
        builder = "ci/" + name,
        category = category,
        short_name = short_name,
    )

    luci.list_view_entry(
        list_view = "try",
        builder = "try/" + name,
    )

    # Only add CQ verifiers for non-ASAN and non-UBSAN bots to minimize CQ load.
    if not asan and not ubsan:
        luci.cq_tryjob_verifier(
            experiment_percentage = 100 if experimental else None,
            cq_group = "Dawn-CQ",
            builder = "dawn:try/" + name,
            location_filters = [
                cq.location_filter(path_regexp = ".*"),
                cq.location_filter(
                    path_regexp = "\\.github/.+",
                    exclude = True,
                ),
            ] + cmake_msvc_file_exclusions,
        )

    # These builders run fine unbranched on branch CLs, so add them to the
    # branch groups as well.
    for milestone in ACTIVE_MILESTONES.keys():
        luci.cq_tryjob_verifier(
            experiment_percentage = 100,  # Temporarily make this experimental
            cq_group = "Dawn-CQ-" + milestone,
            builder = "dawn:try/" + name,
        )

def _add_branch_verifiers(builder_name, os, min_milestone = None, includable_only = False, disable_reuse = False):
    for milestone, details in ACTIVE_MILESTONES.items():
        if os not in details.platforms:
            continue
        if min_milestone != None and int(milestone[1:]) < min_milestone:
            continue
        luci.cq_tryjob_verifier(
            cq_group = "Dawn-CQ-" + milestone,
            builder = "{}:try/{}".format(details.chromium_project, builder_name),
            includable_only = includable_only,
            disable_reuse = disable_reuse,
        )

# We use the DEPS version for branches because ToT builders do not make sense on
# branches and the DEPS versions already exist.
_os_arch_to_branch_builder = {
    "linux": "dawn-linux-x64-deps-rel",
    "mac": "dawn-mac-x64-deps-rel",
    "mac-arm64": "dawn-mac-arm64-deps-rel",
    "win": "dawn-win10-x64-deps-rel",
    "win-arm64": "dawn-win11-arm64-deps-rel",
    "android-arm": "dawn-android-arm-deps-rel",
    "android-arm64": "dawn-android-arm64-deps-rel",
}

_os_arch_to_dawn_cq_builder = {
    "linux": "linux-dawn-rel",
    "mac": "mac-dawn-rel",
    "mac-arm64": "mac-arm64-dawn-rel",
    "win": "win-dawn-rel",
    "win-arm64": "win11-arm64-dawn-rel",
    "android-arm": "android-dawn-arm-rel",
    "android-arm64": "android-dawn-arm64-rel",
}

# The earliest milestone that the builder is relevant for
_os_arch_to_min_milestone = {
    "linux": 112,
    "mac": 112,
    "mac-arm64": 122,
    "win": 112,
    "win-arm64": 126,
    "android-arm": None,
    "android-arm64": None,
}

def chromium_dawn_tryjob(os, arch = None):
    """Adds a tryjob that tests against Chromium

    Args:
      os: string for the OS, should be one or linux|mac|win
      arch: string for the arch, or None
    """

    if arch:
        luci.cq_tryjob_verifier(
            cq_group = "Dawn-CQ",
            builder = "chromium:try/{builder}".format(builder =
                                                          _os_arch_to_dawn_cq_builder["{os}-{arch}".format(os = os, arch = arch)]),
            location_filters = [
                cq.location_filter(path_regexp = ".*"),
                cq.location_filter(
                    path_regexp = "\\.github/.+",
                    exclude = True,
                ),
            ],
        )
        _add_branch_verifiers(
            _os_arch_to_branch_builder["{os}-{arch}".format(os = os, arch = arch)],
            os,
            _os_arch_to_min_milestone["{os}-{arch}".format(os = os, arch = arch)],
        )
    else:
        luci.cq_tryjob_verifier(
            cq_group = "Dawn-CQ",
            builder = "chromium:try/{}-dawn-rel".format(os),
            location_filters = [
                cq.location_filter(path_regexp = ".*"),
                cq.location_filter(
                    path_regexp = "\\.github/.+",
                    exclude = True,
                ),
            ],
        )
        _add_branch_verifiers(_os_arch_to_branch_builder[os], os)

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

luci.gitiles_poller(
    name = "primary-poller",
    bucket = "ci",
    repo = "https://dawn.googlesource.com/dawn",
    refs = [
        "refs/heads/main",
    ],
)

luci.list_view_entry(
    list_view = "try",
    builder = "try/presubmit",
)

luci.builder(
    name = "presubmit",
    bucket = "try",
    executable = get_presubmit_executable(),
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

luci.builder(
    name = "cts-roller",
    bucket = "ci",
    # Run at 5 UTC - which is 10pm PST
    schedule = "0 5 * * *",
    executable = luci.recipe(
        name = "dawn/roll_cts",
        cipd_package = "infra/recipe_bundles/chromium.googlesource.com/chromium/tools/build",
        cipd_version = "refs/heads/main",
    ),
    execution_timeout = 9 * time.hour,
    dimensions = {
        "cpu": "x86-64",
        "os": get_dimension(os.LINUX),
        "pool": "luci.flex.ci",
    },
    properties = {
        "repo_name": "dawn",
        "runhooks": True,
    },
    caches = [
        swarming.cache("golang"),
        swarming.cache("gocache"),
        swarming.cache("nodejs"),
        swarming.cache("npmcache"),
    ],
    notifies = ["gardener-notifier"],
    service_account = "dawn-automated-expectations@chops-service-accounts.iam.gserviceaccount.com",
)

luci.console_view_entry(
    console_view = "ci",
    builder = "ci/cts-roller",
    category = "cron|roll",
    short_name = "cts",
)

dawn_standalone_builder("linux-clang-dbg-x64", clang = True, debug = True, cpu = "x64", fuzzer = False)
dawn_standalone_builder("linux-clang-dbg-x86", clang = True, debug = True, cpu = "x86", fuzzer = False)
dawn_standalone_builder("linux-clang-rel-x64", clang = True, debug = False, cpu = "x64", fuzzer = False)
dawn_standalone_builder("linux-clang-rel-x86", clang = True, debug = False, cpu = "x86", fuzzer = False)
dawn_standalone_builder("mac-dbg", clang = True, debug = True, cpu = "x64", fuzzer = False)
dawn_standalone_builder("mac-rel", clang = True, debug = False, cpu = "x64", fuzzer = False)
dawn_standalone_builder("win-clang-dbg-x64", clang = True, debug = True, cpu = "x64", fuzzer = False)
dawn_standalone_builder("win-clang-dbg-x86", clang = True, debug = True, cpu = "x86", fuzzer = False)
dawn_standalone_builder("win-clang-rel-x64", clang = True, debug = False, cpu = "x64", fuzzer = False)
dawn_standalone_builder("win-clang-rel-x86", clang = True, debug = False, cpu = "x86", fuzzer = False)
dawn_standalone_builder("win-msvc-dbg-x64", clang = False, debug = True, cpu = "x64", fuzzer = False)
dawn_standalone_builder("win-msvc-rel-x64", clang = False, debug = False, cpu = "x64", fuzzer = False)
dawn_standalone_builder("cron-linux-clang-rel-x64", clang = True, debug = False, cpu = "x64", fuzzer = True)

dawn_cmake_standalone_builder("cmake-linux-clang-dbg-x64", clang = True, debug = True, cpu = "x64", asan = False, ubsan = False)
dawn_cmake_standalone_builder("cmake-linux-clang-dbg-x64-asan", clang = True, debug = True, cpu = "x64", asan = True, ubsan = False)
dawn_cmake_standalone_builder("cmake-linux-clang-dbg-x64-ubsan", clang = True, debug = True, cpu = "x64", asan = False, ubsan = True)
dawn_cmake_standalone_builder("cmake-linux-clang-rel-x64", clang = True, debug = False, cpu = "x64", asan = False, ubsan = False)
dawn_cmake_standalone_builder("cmake-linux-clang-rel-x64-asan", clang = True, debug = False, cpu = "x64", asan = True, ubsan = False)
dawn_cmake_standalone_builder("cmake-linux-clang-rel-x64-ubsan", clang = True, debug = False, cpu = "x64", asan = False, ubsan = True)
dawn_cmake_standalone_builder("cmake-mac-dbg", clang = True, debug = True, cpu = "x64", asan = False, ubsan = False, experimental = False)
dawn_cmake_standalone_builder("cmake-mac-rel", clang = True, debug = False, cpu = "x64", asan = False, ubsan = False, experimental = False)
dawn_cmake_standalone_builder("cmake-win-msvc-dbg-x64", clang = False, debug = True, cpu = "x64", asan = False, ubsan = False)
dawn_cmake_standalone_builder("cmake-win-msvc-rel-x64", clang = False, debug = False, cpu = "x64", asan = False, ubsan = False)

chromium_dawn_tryjob("linux")
chromium_dawn_tryjob("mac")
chromium_dawn_tryjob("mac", "arm64")
chromium_dawn_tryjob("win")
chromium_dawn_tryjob("win", "arm64")
chromium_dawn_tryjob("android", "arm")
chromium_dawn_tryjob("android", "arm64")

clang_tidy_dawn_tryjob()

luci.cq_tryjob_verifier(
    cq_group = "Dawn-CQ",
    builder = "chromium:try/dawn-try-linux-x64-intel-uhd770-rel",
    includable_only = True,
)

luci.cq_tryjob_verifier(
    cq_group = "Dawn-CQ",
    builder = "chromium:try/dawn-try-win-x64-intel-uhd770-rel",
    includable_only = True,
)

luci.cq_tryjob_verifier(
    cq_group = "Dawn-CQ",
    builder = "chromium:try/dawn-try-win10-x86-rel",
    includable_only = True,
)
_add_branch_verifiers("dawn-win10-x86-deps-rel", "win", includable_only = True)

luci.cq_tryjob_verifier(
    cq_group = "Dawn-CQ",
    builder = "chromium:try/dawn-try-mac-arm64-rel",
    includable_only = True,
)

# Experimental builders that usually don't actually run any tests, but will when
# qualifying a new configuration.
luci.cq_tryjob_verifier(
    cq_group = "Dawn-CQ",
    builder = "chromium:try/android-dawn-arm64-exp-rel",
    includable_only = True,
)

luci.cq_tryjob_verifier(
    cq_group = "Dawn-CQ",
    builder = "chromium:try/dawn-try-mac-arm64-m2-exp",
    includable_only = True,
)

luci.cq_tryjob_verifier(
    cq_group = "Dawn-CQ",
    builder = "chromium:try/dawn-try-mac-intel-exp",
    includable_only = True,
)

luci.cq_tryjob_verifier(
    cq_group = "Dawn-CQ",
    builder = "chromium:try/dawn-try-mac-amd-exp",
    includable_only = True,
)

luci.cq_tryjob_verifier(
    cq_group = "Dawn-CQ",
    builder = "chromium:try/dawn-try-win-x64-intel-exp",
    includable_only = True,
)

luci.cq_tryjob_verifier(
    cq_group = "Dawn-CQ",
    builder = "chromium:try/dawn-try-win-x64-nvidia-exp",
    includable_only = True,
)

luci.cq_tryjob_verifier(
    cq_group = "Dawn-CQ",
    builder = "chromium:try/linux-dawn-nvidia-1660-exp-rel",
    includable_only = True,
)

# This is separate from the "presubmit" builder since we need branch-specific
# branch builders unlike stock presubmit.
luci.cq_tryjob_verifier(
    cq_group = "Dawn-CQ",
    builder = "chromium:try/dawn-chromium-presubmit",
    disable_reuse = True,
)
_add_branch_verifiers("dawn-chromium-presubmit", "linux", min_milestone = 130, disable_reuse = True)

# Views

luci.milo(
    logo = "https://storage.googleapis.com/chrome-infra-public/logo/dawn-logo.png",
)

luci.console_view(
    name = "ci",
    title = "Dawn CI Builders",
    repo = "https://dawn.googlesource.com/dawn",
    refs = ["refs/heads/main"],
)

luci.list_view(
    name = "try",
    title = "Dawn try Builders",
)

# CQ

luci.cq(
    status_host = "chromium-cq-status.appspot.com",
    submit_max_burst = 4,
    submit_burst_delay = 480 * time.second,
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
