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

load("@chromium-luci//builders.star", "os")
load("@chromium-luci//chromium_luci.star", "chromium_luci")
load("@chromium-luci//consoles.star", "consoles")
load("//project.star", "ACTIVE_MILESTONES")

# Use LUCI Scheduler BBv2 names and add Scheduler realms configs.
lucicfg.enable_experiment("crbug.com/1182002")

lucicfg.config(
    config_dir = "generated",
    tracked_files = [
        "builders/*/*/*",
        "builders/*/*/*/*",
        "builders/gn_args_locations.json",
        "luci/commit-queue.cfg",
        "luci/cr-buildbucket.cfg",
        "luci/luci-analysis.cfg",
        "luci/luci-analysis-dev.cfg",
        "luci/luci-bisection.cfg",
        "luci/luci-bisection-dev.cfg",
        "luci/luci-logdog.cfg",
        "luci/luci-milo.cfg",
        "luci/luci-notify.cfg",
        "luci/luci-scheduler.cfg",
        "luci/project.cfg",
        "luci/realms.cfg",
        # No current need for other generated files such as mixins.pyl.
        "testing/gn_isolate_map.pyl",
    ],
    fail_on_warnings = True,
)

luci.project(
    name = "dawn",
    config_dir = "luci",
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
        # Allow any Dawn build to trigger tests running under the testing service
        # used on shared Chromium testing pools.
        luci.binding(
            roles = "role/swarming.taskServiceAccount",
            users = [
                "chromium-tester@chops-service-accounts.iam.gserviceaccount.com",
            ],
        ),
    ],
)

chromium_luci.configure_project(
    name = "dawn",
    is_main = True,
    platforms = {},
)

chromium_luci.configure_builder_health_indicators(
    unhealthy_period_days = 7,
    pending_time_p50_min = 20,
)

chromium_luci.configure_ci(
    test_results_bq_dataset_name = "chromium",
    resultdb_index_by_timestamp = True,
)

chromium_luci.configure_try(
    test_results_bq_dataset_name = "chromium",
    resultdb_index_by_timestamp = True,
)

chromium_luci.configure_builders(
    os_dimension_overrides = {
        os.LINUX_DEFAULT: os.LINUX_JAMMY,
        os.MAC_DEFAULT: "Mac-15|Mac-26",
        os.WINDOWS_DEFAULT: os.WINDOWS_10,
    },
)

chromium_luci.configure_per_builder_outputs(
    root_dir = "builders",
)

chromium_luci.configure_recipe_experiments(
    # This can be removed once all builders use the chromium-luci wrappers for
    # creating builders instead of directly calling luci.builder().
    require_builder_wrappers = False,
)

chromium_luci.configure_targets(
    generate_pyl_files = chromium_luci.pyl_generation_configuration(),
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
    bindings = [
        # Allow CI builders to create invocations in their own builds.
        luci.binding(
            roles = "role/resultdb.invocationCreator",
            users = [
                "dawn-ci-builder@chops-service-accounts.iam.gserviceaccount.com",
            ],
        ),
    ],
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
    bindings = [
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
)

# Allows builders to write baselines and query ResultDB for new tests.
luci.realm(
    name = "@project",
    bindings = [
        luci.binding(
            roles = "role/resultdb.baselineWriter",
            users = [
                "dawn-ci-builder@chops-service-accounts.iam.gserviceaccount.com",
                "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
            ],
        ),
        luci.binding(
            roles = "role/resultdb.baselineReader",
            users = [
                "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
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
        pools = ["luci.flex.ci", "luci.chromium.gpu.ci"],
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
        pools = ["luci.flex.try", "luci.chromium.gpu.try"],
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

luci.gitiles_poller(
    name = "primary-poller",
    bucket = "ci",
    repo = "https://dawn.googlesource.com/dawn",
    refs = [
        "refs/heads/main",
    ],
)

# Notifiers

luci.notifier(
    name = "gardener-notifier",
    notify_rotation_urls = [
        "https://chrome-ops-rotation-proxy.appspot.com/current/grotation:webgpu-gardener",
    ],
    on_occurrence = ["FAILURE", "INFRA_FAILURE"],
)

# CQ Setup

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

# Views

luci.milo(
    logo = "https://storage.googleapis.com/chrome-infra-public/logo/dawn-logo.png",
)

consoles.console_view(
    name = "ci",
    title = "Dawn CI Builders",
    repo = "https://dawn.googlesource.com/dawn",
    refs = ["refs/heads/main"],
)

consoles.list_view(
    name = "exp",
    title = "Dawn experimental CI Builders",
)

consoles.list_view(
    name = "try",
    title = "Dawn try Builders",
)

# Run other non-builder setup.
exec("@chromium-targets//mixins.star")
exec("//binaries.star")
exec("//bundles.star")
exec("//compile_targets.star")
exec("//gn_args.star")
exec("//mixins.star")
exec("//recipes.star")
exec("//tests.star")

# Handle any other builders defined in other files.
exec("//bazel_ci.star")
exec("//bazel_try.star")
exec("//chromium_try.star")
exec("//cmake_ci.star")
exec("//cmake_try.star")
exec("//gn_standalone_ci.star")
exec("//gn_standalone_try.star")
exec("//trusted_robots.star")

# Just copy LUCI Bisection config to generated outputs.
lucicfg.emit(
    dest = "luci/luci-bisection.cfg",
    data = io.read_file("luci-bisection.cfg"),
)

lucicfg.emit(
    dest = "luci/luci-bisection-dev.cfg",
    data = io.read_file("luci-bisection-dev.cfg"),
)

# Just copy LUCI Analysis config to generated outputs.
lucicfg.emit(
    dest = "luci/luci-analysis.cfg",
    data = io.read_file("luci-analysis.cfg"),
)

lucicfg.emit(
    dest = "luci/luci-analysis-dev.cfg",
    data = io.read_file("luci-analysis-dev.cfg"),
)
