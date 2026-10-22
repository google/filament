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

"""Builders that run as trusted service accounts."""

# Realm for pool with bots that run as trusted robots.
luci.realm(
    name = "pools/trusted-robots",
)

luci.builder(
    name = "cts-roller",
    bucket = "ci",
    swarming_host = "chrome-swarming.appspot.com",
    # Run at 5 UTC on weekdays - which is 10pm PST
    schedule = "0 5 * * 1-5",
    executable = "recipe:dawn/roll_cts",
    execution_timeout = 9 * time.hour,
    dimensions = {
        "cpu": "x86-64",
        "os": "Ubuntu-24.04",
        "pool": "luci.dawn.trusted-robots",
    },
    properties = {
        "gardener_rotations": ["dawn"],
        "repo_name": "dawn",
        "runhooks": True,
        # TODO(crbug.com/343503161): Remove sheriff_rotations after SoM is updated.
        "sheriff_rotations": ["dawn"],
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
