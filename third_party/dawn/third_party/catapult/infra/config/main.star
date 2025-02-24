#!/usr/bin/env lucicfg
# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""LUCI project configuration catapult.

After modifying this file execute it ('./main.star') to regenerate the configs.
This is also enforced by PRESUBMIT.py script.
"""

lucicfg.check_version("1.31.1", "Please update depot_tools")

# Use LUCI Scheduler BBv2 names and add Scheduler realms configs.
lucicfg.enable_experiment("crbug.com/1182002")

lucicfg.config(
    config_dir = "generated",
    fail_on_warnings = True,
    lint_checks = ["default"],
)

luci.project(
    name = "catapult",
    buildbucket = "cr-buildbucket.appspot.com",
    logdog = "luci-logdog.appspot.com",
    swarming = "chromium-swarm.appspot.com",
    acls = [
        # Publicly readable.
        acl.entry(
            roles = [
                acl.BUILDBUCKET_READER,
                acl.LOGDOG_READER,
                acl.PROJECT_CONFIGS_READER,
            ],
            groups = "all",
        ),
        acl.entry(
            roles = [
                acl.CQ_COMMITTER,
            ],
            groups = "project-catapult-committers",
        ),
        acl.entry(
            roles = [
                acl.BUILDBUCKET_TRIGGERER,
            ],
            groups = [
                "project-chromium-tryjob-access",
                "service-account-cq",  # TODO: use project-scoped account
            ],
        ),
        acl.entry(
            roles = acl.CQ_DRY_RUNNER,
            groups = "project-chromium-tryjob-access",
        ),
        acl.entry(
            roles = acl.LOGDOG_WRITER,
            groups = "luci-logdog-chromium-writers",
        ),
    ],
    bindings = [
        luci.binding(
            roles = "role/configs.validator",
            users = "catapult-try-builder@chops-service-accounts.iam.gserviceaccount.com",
        ),
    ],
)

# Per-service tweaks.
luci.logdog(gs_bucket = "chromium-luci-logdog")

luci.bucket(name = "try")

# Allow LED users to trigger swarming tasks directly when debugging try
# builders.
luci.binding(
    realm = "try",
    roles = "role/swarming.taskTriggerer",
    groups = "flex-try-led-users",
)

luci.cq(
    status_host = "chromium-cq-status.appspot.com",
    submit_max_burst = 4,
    submit_burst_delay = 480 * time.second,
)

luci.cq_group(
    name = "catapult",
    watch = cq.refset(
        repo = "https://chromium.googlesource.com/catapult",
        refs = ["refs/heads/.+"],
    ),
    retry_config = cq.retry_config(
        single_quota = 1,
        global_quota = 2,
        failure_weight = 1,
        transient_failure_weight = 1,
        timeout_weight = 2,
    ),
)

# Matches any file under the 'dashboard' root directory.
DASHBOARD_RE = "dashboard/.+"

# Matches any file under the 'perf_issue_service' root directory.
PERF_ISSUE_SERVICE_RE = "perf_issue_service/.+"

def try_builder(
        name,
        os,
        is_dashboard = False,
        is_presubmit = False,
        is_perf_issue_service = False,
        experiment = None,
        properties = None,
        dimensions = None):
    """
    Declares a new builder in the 'try' bucket.

    Args:
      name: The name of this builder.
      os: The swarming `os` dimension.
      is_dashboard: True if this only processes
        the 'dashboard' portion of catapult.
      is_perf_issue_service: True if this only processes
        the 'perf_issue_service' portion of catapult.
      is_presubmit: True if this runs PRESUBMIT.
      experiment: Value 0-100 for the cq experiment %.
      properties: {key: value} dictionary for extra properties.
      dimensions: {key: value} dictionary for extra dimensions.
    """

    # TODO: switch to bbagent, delete $kitchen property
    props = {
        "$kitchen": {"devshell": True, "git_auth": True},
    }
    if properties:
        props.update(properties)

    dims = {
        "pool": "luci.flex.try",
        "os": "Ubuntu-22.04" if os == "Ubuntu" else os,
    }
    if dimensions:
        dims.update(dimensions)
    if os.startswith("Ubuntu"):
        dims["cpu"] = "x86-64"

    executable = luci.recipe(
        name = "catapult",
        cipd_package = "infra/recipe_bundles/chromium.googlesource.com/chromium/tools/build",
        use_bbagent = True,
    )
    if is_presubmit:
        executable = luci.recipe(
            name = "run_presubmit",
            cipd_package = "infra/recipe_bundles/chromium.googlesource.com/chromium/tools/build",
            use_bbagent = True,
        )
        props["repo_name"] = "catapult"
    if is_dashboard:
        props["dashboard_only"] = True
    if is_perf_issue_service:
        props["perf_issue_service_only"] = True

    luci.builder(
        name = name,
        bucket = "try",
        executable = executable,
        build_numbers = True,
        dimensions = dims,
        execution_timeout = 2 * time.hour,
        service_account = "catapult-try-builder@chops-service-accounts.iam.gserviceaccount.com",
        properties = props,
    )

    verifier_kwargs = {}

    # dashboard/ and perf_issue_service are individual services where
    # changes should not affect the other Catapult modules. Thus, the
    # Catapult Tryservers should *not* run if changes are only in
    # /dashboard or /perf_issue_service.
    # Presubmit sees all changes
    if not is_presubmit:
        verifier_kwargs["location_filters"] = [
            cq.location_filter(path_regexp = ".*"),
        ]
        if not is_dashboard:
            # non-dashboard try jobs should not run if changes are only
            # in DASHBOARD_RE
            verifier_kwargs["location_filters"].append(
                cq.location_filter(
                    path_regexp = DASHBOARD_RE,
                    exclude = True,
                ),
            )
        if not is_perf_issue_service:
            # non-perf_issue_service try jobs should not run if changes
            # are only in PERF_ISSUE_SERVICE_RE
            verifier_kwargs["location_filters"].append(
                cq.location_filter(
                    path_regexp = PERF_ISSUE_SERVICE_RE,
                    exclude = True,
                ),
            )
    if experiment != None:
        verifier_kwargs["experiment_percentage"] = experiment

    luci.cq_tryjob_verifier(
        builder = name,
        cq_group = "catapult",
        disable_reuse = is_presubmit,
        **verifier_kwargs
    )

try_builder("Catapult Linux Tryserver", "Ubuntu")

try_builder("Catapult Windows Tryserver", "Windows-10")

try_builder("Catapult Mac Tryserver", "Mac", dimensions = {"cpu": "x86-64"})

try_builder("Catapult Mac M1 Tryserver", "Mac", dimensions = {"cpu": "arm"})

try_builder("Catapult Android Tryserver", "Android", experiment = 100, dimensions = {"device_type": "walleye"}, properties = {"platform": "android"})

try_builder("Catapult Presubmit", "Ubuntu", is_presubmit = True)

try_builder("Dashboard Linux Tryserver", "Ubuntu", is_dashboard = True)

try_builder("Perf Issue Service Linux Tryserver", "Ubuntu", is_perf_issue_service = True, experiment = 100)
