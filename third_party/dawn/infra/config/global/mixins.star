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

"""Mixin declarations

Mixins are used to define common properties that can be applied to multiple
tests.

These are the Dawn-specific mixins. Mixins shared with Chromium (e.g. for
Swarming dimensions) are pulled in via the @chromium-targets Starlark package
and are found at Chromium's //infra/config/targets/mixins.star file.
"""

load("@chromium-luci//targets.star", "targets")

targets.mixin(
    name = "adapter_vendor_id_sws",
    args = [
        "--adapter-vendor-id=0x1AE0",
    ],
)

targets.mixin(
    name = "clusterfuzz_wire_trace_dir",
    args = [
        "--wire-trace-dir=${ISOLATED_OUTDIR}/clusterfuzz",
    ],
)

targets.mixin(
    name = "dawn_end2end_real_hardware_gtests_common_args",
    args = [
        "--use-gpu-in-tests",
        "--exclusive-device-type-preference=discrete,integrated",
        "--test-launcher-retry-limit=0",
        "--test-launcher-batch-limit=512",
    ],
    linux_args = [
        "--no-xvfb",
    ],
    win_args = [
        # TODO(crbug.com/454365243): Remove this filter when including these
        # tests does not contribute to OOM issues.
        "--gtest_filter=-*WebGPU_WebGPU_backend_on*",
    ],
)

targets.mixin(
    name = "dawn_end2end_sws_tsan_gtest_common_args",
    args = [
        # We are only want to run on SwiftShader for now. Since SwiftShader
        # is only meant as a temporary solution, this should either be
        # removed in favor of LLVMPipe or TSAN testing should run on
        # real hardware.
        "--adapter-vendor-id=0x1AE0",
        # //testing/test_env.py automatically tries to run an additional
        # symbolization script if sanitizers are enabled, but this script
        # implicitly depends on tests producing Chromium's proprietary
        # test result format instead of the one natively produced by gtest.
        # TSAN stacks are still usable without this extra symbolization,
        # though.
        "--skip-symbolization-script=1",
    ],
)

targets.mixin(
    name = "disable_dxc",
    args = [
        "--disable-toggles=use_dxc",
    ],
)

targets.mixin(
    name = "enable_backend_validation",
    args = [
        "--enable-backend-validation",
    ],
)

targets.mixin(
    name = "result_adapter_gtest_json",
    resultdb = targets.resultdb(
        result_format = "gtest_json",
    ),
)

targets.mixin(
    name = "result_adapter_single",
    resultdb = targets.resultdb(
        result_format = "single",
    ),
)

targets.mixin(
    name = "tint_fuzzer_corpus_common_args",
    args = [
        "--append-cwd-as-build",
    ],
)

targets.mixin(
    name = "tint_fuzzer_corpus_generate_args",
    args = [
        "-generate",
        "-out",
        "${ISOLATED_OUTDIR}/clusterfuzz",
    ],
)

targets.mixin(
    name = "tint_ir_merge",
    merge = targets.merge(
        script = "//scripts/merge_scripts/generate_tint_fuzz_corpora.py",
        args = [
            "--fuzzer-name",
            "tint_ir_fuzzer",
        ],
    ),
)

targets.mixin(
    name = "tint_wgsl_merge",
    merge = targets.merge(
        script = "//scripts/merge_scripts/generate_tint_fuzz_corpora.py",
        args = [
            "--fuzzer-name",
            "tint_wgsl_fuzzer",
        ],
    ),
)

targets.mixin(
    name = "true_noop_merge",
    merge = targets.merge(
        script = "//scripts/merge_scripts/true_noop_merge.py",
    ),
)

targets.mixin(
    name = "use_wire",
    args = [
        "--use-wire",
    ],
)

targets.mixin(
    name = "win_snapdragon_x_elite_gtest_args",
    args = [
        # Only use the physical GPU. On these devices, SwiftShader (0x1AE0),
        # WARP (0x1414), and some unknown "integrated GPU" (0x5143) are all
        # reported in addition to this.
        "--adapter-vendor-id=0x4D4F4351",
    ],
)

targets.mixin(
    name = "wire_trace_merge",
    merge = targets.merge(
        script = "//scripts/merge_scripts/generate_wire_trace_fuzz_corpora.py",
        args = [
            "--fuzzer-name",
            "dawn_wire_server_and_frontend_fuzzer",
            "--fuzzer-name",
            "dawn_wire_server_and_vulkan_backend_fuzzer",
            "--fuzzer-name",
            "dawn_wire_server_and_d3d12_backend_fuzzer",
        ],
    ),
)
