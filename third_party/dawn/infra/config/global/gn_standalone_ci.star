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

"""CI Dawn builders using GN and a standalone Dawn checkout (instead of Chromium)."""

load("@chromium-luci//builder_config.star", "builder_config")
load("@chromium-luci//builders.star", "os")
load("@chromium-luci//ci.star", "ci")
load("@chromium-luci//consoles.star", "consoles")
load("@chromium-luci//gn_args.star", "gn_args")
load("//constants.star", "siso")

ci.defaults.set(
    executable = "recipe:dawn/gn_v2",
    builder_group = "ci",
    bucket = "ci",
    pool = "luci.chromium.gpu.ci",
    triggered_by = ["primary-poller"],
    build_numbers = True,
    contact_team_email = "chrome-gpu-infra@google.com",
    service_account = "dawn-ci-builder@chops-service-accounts.iam.gserviceaccount.com",
    shadow_service_account = "dawn-try-builder@chops-service-accounts.iam.gserviceaccount.com",
    siso_project = siso.project.DEFAULT_TRUSTED,
    siso_remote_jobs = siso.remote_jobs.DEFAULT,
    thin_tester_cores = 2,
)

ci.builder(
    name = "dawn-linux-x64-builder-rel",
    description_html = "Compiles Dawn test binaries for Linux/x64",
    schedule = "triggered",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
    ),
    gn_args = gn_args.config(
        configs = [
            "clang",
            "component",
            "dawn_no_d3d12",
            "dawn_swiftshader",
            "linux",
            "release",
            "siso",
            "tint_hlsl_writer",
            "tint_msl_writer",
            "tint_spv_reader_writer",
            "tint_wgsl_reader_writer",
            "x64",
        ],
    ),
    cores = 8,
    os = os.LINUX_DEFAULT,
    console_view_entry = consoles.console_view_entry(
        category = "linux|build|clang|rel",
        short_name = "x64",
    ),
)

ci.thin_tester(
    name = "dawn-linux-x64-sws-rel",
    description_html = "Tests Dawn on Linux/x64 with SwiftShader",
    parent = "dawn-linux-x64-builder-rel",
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "dawn",
        ),
        chromium_config = builder_config.chromium_config(
            config = "dawn_base",
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.INTEL,
            target_bits = 64,
            target_platform = builder_config.target_platform.LINUX,
        ),
        run_tests_serially = True,
    ),
    console_view_entry = consoles.console_view_entry(
        category = "linux|test|clang|rel|x64",
        short_name = "sws",
    ),
)
