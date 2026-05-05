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

"""Chromium trybots that are exposed for use in Dawn."""

load("//location_filters.star", "exclusion_filters")
load("//project.star", "ACTIVE_MILESTONES")

## Templates

def _expose_builder_for_active_milestones(*, base_builder_name, platform, verifier_factory, min_milestone = None, **kwargs):
    """Helper to create tryjob verifiers on all active milestones.

    Args:
        base_builder_name: The name of builder to add without the project/group
            prefix.
        platform: The platform that the builder tests as a string. Used to omit
            builders on branches that do not need to test that platform.
        verifier_factory: The factory to call to actually expose the builder.
        min_milestone: An optional int representing the first milestone branch
            that the verifier should be added to.
        **kwargs: Other keyword arguments to pass to |verifier_factory|.
    """
    for milestone, details in ACTIVE_MILESTONES.items():
        if platform not in details.platforms:
            continue
        if min_milestone != None and int(milestone[1:]) < min_milestone:
            continue
        verifier_factory(
            cq_group = "Dawn-CQ-" + milestone,
            builder = "{}:try/{}".format(details.chromium_project, base_builder_name),
            **kwargs
        )

def cq_chromium_trybot(**kwargs):
    """Exposes a Chromium trybot as a CQ Dawn trybot.

    Args:
        **kwargs: Keyword arguments to pass to luci.cq_tryjob_verifier().
    """
    kwargs.setdefault("cq_group", "Dawn-CQ")
    kwargs.setdefault("includable_only", False)
    kwargs.setdefault("location_filters", exclusion_filters.chromium_cq_file_exclusions)
    luci.cq_tryjob_verifier(**kwargs)

def cq_branch_verifier_chromium_trybot(*, base_builder_name, platform, min_milestone = None, **kwargs):
    """Exposes a Chromium trybot as a CQ Dawn trybot for active milestone branches.

    Args:
        base_builder_name: The name of builder to add without the project/group
            prefix.
        platform: The platform that the builder tests as a string. Used to omit
            builders on branches that do not need to test that platform.
        min_milestone: An optional int representing the first milestone branch
            that the verifier should be added to.
        **kwargs: Other keyword arguments to pass to cq_chromium_trybot()
    """
    _expose_builder_for_active_milestones(
        base_builder_name = base_builder_name,
        platform = platform,
        verifier_factory = cq_chromium_trybot,
        min_milestone = min_milestone,
        **kwargs
    )

def manual_only_chromium_trybot(**kwargs):
    """Exposes a Chromium trybot as a manual-only Dawn trybot.

    Args:
        **kwargs: Keyword arguments to pass to luci.cq_tryjob_verifier().
    """
    kwargs.setdefault("cq_group", "Dawn-CQ")
    kwargs.setdefault("includable_only", True)
    luci.cq_tryjob_verifier(**kwargs)

def manual_only_branch_verifier_chromium_trybot(*, base_builder_name, platform, min_milestone = None, **kwargs):
    """Exposes a Chromium trybot as a manual-only Dawn trybot for active milestone branches.

    Args:
        base_builder_name: The name of builder to add without the project/group
            prefix.
        platform: The platform that the builder tests as a string. Used to omit
            builders on branches that do not need to test that platform.
        min_milestone: An optional int representing the first milestone branch
            that the verifier should be added to.
        **kwargs: Other keyword arguments to pass to manual_only_chromium_trybot()
    """
    _expose_builder_for_active_milestones(
        base_builder_name = base_builder_name,
        platform = platform,
        verifier_factory = manual_only_chromium_trybot,
        min_milestone = min_milestone,
        **kwargs
    )

# Note that DEPS builders are used for branches because ToT builders do not make
# sense on branches and the DEPS versions already exist.

#############
## Android ##
#############

cq_chromium_trybot(builder = "chromium:try/android-dawn-arm-rel")
cq_branch_verifier_chromium_trybot(base_builder_name = "dawn-android-arm-deps-rel", platform = "android")

cq_chromium_trybot(builder = "chromium:try/android-dawn-arm64-rel")
cq_branch_verifier_chromium_trybot(base_builder_name = "dawn-android-arm64-deps-rel", platform = "android")

manual_only_chromium_trybot(builder = "chromium:try/android-dawn-arm64-exp-rel")

###########
## Linux ##
###########

cq_chromium_trybot(builder = "chromium:try/dawn-chromium-presubmit", disable_reuse = True, location_filters = [])
cq_branch_verifier_chromium_trybot(base_builder_name = "dawn-chromium-presubmit", platform = "linux", min_milestone = 130, disable_reuse = True, location_filters = [])

cq_chromium_trybot(builder = "chromium:try/linux-dawn-rel")
cq_branch_verifier_chromium_trybot(base_builder_name = "dawn-linux-x64-deps-rel", platform = "linux", min_milestone = 112)

# No DEPS equivalent to expose for branches.
manual_only_chromium_trybot(builder = "chromium:try/dawn-try-linux-x64-intel-uhd770-rel")

manual_only_chromium_trybot(builder = "chromium:try/linux-dawn-nvidia-1660-exp-rel")

#########
## Mac ##
#########

cq_chromium_trybot(builder = "chromium:try/mac-dawn-rel")
cq_branch_verifier_chromium_trybot(base_builder_name = "dawn-mac-x64-deps-rel", platform = "mac", min_milestone = 112)

cq_chromium_trybot(builder = "chromium:try/mac-arm64-dawn-rel")
cq_branch_verifier_chromium_trybot(base_builder_name = "dawn-mac-arm64-deps-rel", platform = "mac", min_milestone = 122)

manual_only_chromium_trybot(builder = "chromium:try/dawn-try-mac-amd-exp")

manual_only_chromium_trybot(builder = "chromium:try/dawn-try-mac-arm64-m2-exp")

# DEPS builder that runs tests on a superset of this config is added by the
# CQ Mac/ARM64 builder.
manual_only_chromium_trybot(builder = "chromium:try/dawn-try-mac-arm64-rel")

manual_only_chromium_trybot(builder = "chromium:try/dawn-try-mac-intel-exp")

#############
## Windows ##
#############

cq_chromium_trybot(builder = "chromium:try/win-dawn-rel")
cq_branch_verifier_chromium_trybot(base_builder_name = "dawn-win10-x64-deps-rel", platform = "win", min_milestone = 112)

cq_chromium_trybot(builder = "chromium:try/win11-arm64-dawn-rel")
cq_branch_verifier_chromium_trybot(base_builder_name = "dawn-win11-arm64-deps-rel", platform = "win", min_milestone = 126)

# This entry can be removed in favor of win11-arm64-dawn-rel once tests are
# stable enough to add to that builder.
manual_only_chromium_trybot(builder = "chromium:try/dawn-try-win11-arm64-snapdragon-x-elite-rel")

manual_only_chromium_trybot(builder = "chromium:try/dawn-try-win-x64-intel-exp")

# No DEPS equivalent to expose for branches.
manual_only_chromium_trybot(builder = "chromium:try/dawn-try-win-x64-intel-uhd770-rel")

manual_only_chromium_trybot(builder = "chromium:try/dawn-try-win-x64-nvidia-exp")

manual_only_chromium_trybot(builder = "chromium:try/dawn-try-win10-x86-rel")

# Normally, only DEPS variants of Dawn ToT CQ bots are branched.
# Due to limited capacity, the ToT dawn-try-win10-x86-rel is not a part of Dawn CQ.
# However, Dawn Win10 x86 CI bots are branched.
# So, dawn-win10-x86-deps-rel is made available to be manually triggered on branches,
# in order to facilitate investigating regressions on the branch CI builders.
manual_only_branch_verifier_chromium_trybot(base_builder_name = "dawn-win10-x86-deps-rel", platform = "win")
