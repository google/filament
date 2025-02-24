#!/usr/bin/env lucicfg

# Enable LUCI Realms support.
lucicfg.enable_experiment("crbug.com/1085650")

luci.project(
    name = "swiftshader",
    acls = [
        acl.entry(
            acl.PROJECT_CONFIGS_READER,
            groups = "all",
        ),
    ],
)

luci.cq_group(
    name = "SwiftShader-CQ",
    watch = cq.refset(
        repo = "https://swiftshader.googlesource.com/SwiftShader",
        refs = ["refs/heads/master"],
    ),
    acls = [
        # Committers gonna commit.
        acl.entry(
            acl.CQ_COMMITTER,
            groups = "project-swiftshader-committers",
        ),
        # Ability to launch CQ dry runs manually.
        acl.entry(
            acl.CQ_DRY_RUNNER,
            groups = "project-swiftshader-tryjob-access",
        ),
        # Ability to automatically trigger new patchset runs on CV.
        acl.entry(
            roles = acl.CQ_NEW_PATCHSET_RUN_TRIGGERER,
            groups = "project-swiftshader-tryjob-access",
        ),
    ],
    verifiers = [
        luci.cq_tryjob_verifier(
            builder = "chromium:try/linux-swangle-try-tot-swiftshader-x64",
            mode_allowlist = [cq.MODE_DRY_RUN, cq.MODE_FULL_RUN, cq.MODE_NEW_PATCHSET_RUN],
        ),
        luci.cq_tryjob_verifier(
            builder = "chromium:try/win-swangle-try-tot-swiftshader-x86",
            mode_allowlist = [cq.MODE_DRY_RUN, cq.MODE_FULL_RUN, cq.MODE_NEW_PATCHSET_RUN],
        ),
        luci.cq_tryjob_verifier(
            builder = "chromium:try/win-swangle-try-tot-swiftshader-x64",
            mode_allowlist = [cq.MODE_DRY_RUN, cq.MODE_FULL_RUN, cq.MODE_NEW_PATCHSET_RUN],
        ),
    ],
)

luci.cq(
    status_host = "chromium-cq-status.appspot.com",
)
