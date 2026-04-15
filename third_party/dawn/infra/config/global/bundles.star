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

"""Bundle declarations

Bundles are groupings of tests and/or compile targets that can be referenced by
builders or other bundles.
"""

load("@chromium-luci//targets.star", "targets")

targets.bundle(
    name = "dawn_end2end_real_hardware_common_gtests",
    targets = [
        "dawn_end2end_capture_replay_tests",
        "dawn_end2end_implicit_sync_tests",
        "dawn_end2end_skip_validation_tests",
        "dawn_end2end_tests",
        "dawn_end2end_validation_layers_tests",
        "dawn_end2end_wire_tests",
    ],
    mixins = [
        "dawn_end2end_real_hardware_gtests_common_args",
    ],
)

targets.bundle(
    name = "dawn_end2end_real_hardware_win_only_gtests",
    targets = [
        "dawn_end2end_no_dxc_tests",
        "dawn_end2end_no_dxc_validation_layers_tests",
    ],
    mixins = [
        "dawn_end2end_real_hardware_gtests_common_args",
    ],
)

targets.bundle(
    name = "dawn_end2end_sws_tsan_gtests",
    targets = [
        "dawn_end2end_implicit_sync_tests",
        "dawn_end2end_skip_validation_tests",
        "dawn_end2end_tests",
        "dawn_end2end_wire_tests",
    ],
    mixins = [
        "dawn_end2end_sws_tsan_gtest_common_args",
        # Increase sharding due to TSan slowness.
        targets.mixin(
            swarming = targets.swarming(
                shards = 5,
            ),
        ),
    ],
)

targets.bundle(
    name = "real_hardware_common_gtests",
    targets = [
        "dawn_end2end_real_hardware_common_gtests",
        "dawn_real_hardware_common_perf_tests",
    ],
)

targets.bundle(
    name = "real_hardware_win_gtests",
    targets = [
        "dawn_end2end_real_hardware_win_only_gtests",
        "real_hardware_common_gtests",
    ],
)

targets.bundle(
    name = "dawn_real_hardware_common_perf_tests",
    targets = [
        "dawn_perf_tests",
    ],
)

targets.bundle(
    name = "swiftshader_gtests",
    targets = [
        "dawn_end2end_swangle_tests",
        "dawn_end2end_sws_tests",
        "dawn_unittests",
        "dawn_wire_unittests",
        "tint_unittests",
    ],
)

targets.bundle(
    name = "swiftshader_isolated_scripts",
    targets = [
        "dawn_node_sws_cts",
        "tint_benchmark",
    ],
)

targets.bundle(
    name = "tint_fuzzer_corpus_check_tests",
    targets = [
        "tint_ir_fuzzer_corpus_check_tests",
        "tint_wgsl_fuzzer_corpus_check_tests",
    ],
)

targets.bundle(
    name = "tint_fuzzer_corpus_generate_tests",
    targets = [
        "tint_ir_fuzzer_corpus_generate_tests",
        "tint_wgsl_fuzzer_corpus_generate_tests",
    ],
)

targets.bundle(
    name = "win_software_renderer_gtests",
    targets = [
        "swiftshader_gtests",
        "dawn_end2end_warp_tests",
    ],
)

targets.bundle(
    name = "win_software_renderer_isolated_scripts",
    targets = [
        "dawn_node_software_d3d12_cts",
        "tint_benchmark",
    ],
)

targets.bundle(
    name = "wire_trace_gtests",
    targets = [
        "dawn_wire_trace_end2end_sws_tests",
        "dawn_wire_trace_unittests",
    ],
)
