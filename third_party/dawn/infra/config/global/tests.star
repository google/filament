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

"""Test declarations

Tests define a target to be built and executed on a builder. Tests can
be referenced by a suite or bundle to include the test in the
suite/bundle. Tests also define a bundle containing just the test
itself, so they can be used wherever a bundle is expected.
"""

load("@chromium-luci//targets.star", "targets")

targets.tests.gtest_test(
    name = "dawn_end2end_capture_replay_tests",
    mixins = [
        "result_adapter_gtest_json",
        "true_noop_merge",
        targets.mixin(
            swarming = targets.swarming(
                # Primarily needed for Windows.
                shards = 2,
            ),
        ),
    ],
    args = [
        "--check-capture-replay",
    ],
    binary = "dawn_end2end_tests",
)

targets.tests.gtest_test(
    name = "dawn_end2end_implicit_sync_tests",
    mixins = [
        "result_adapter_gtest_json",
        "true_noop_merge",
        targets.mixin(
            swarming = targets.swarming(
                # Primarily needed for Windows.
                shards = 2,
            ),
        ),
    ],
    args = [
        "--enable-implicit-device-sync",
    ],
    binary = "dawn_end2end_tests",
)

targets.tests.gtest_test(
    name = "dawn_end2end_no_dxc_tests",
    mixins = [
        "disable_dxc",
        "result_adapter_gtest_json",
        "true_noop_merge",
        targets.mixin(
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    ],
    binary = "dawn_end2end_tests",
)

targets.tests.gtest_test(
    name = "dawn_end2end_no_dxc_validation_layers_tests",
    mixins = [
        "disable_dxc",
        "enable_backend_validation",
        "result_adapter_gtest_json",
        "true_noop_merge",
        targets.mixin(
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
    ],
    binary = "dawn_end2end_tests",
)

targets.tests.gtest_test(
    name = "dawn_end2end_skip_validation_tests",
    mixins = [
        "result_adapter_gtest_json",
        "true_noop_merge",
        targets.mixin(
            swarming = targets.swarming(
                # Primarily needed for Windows.
                shards = 2,
            ),
        ),
    ],
    args = [
        "--enable-toggles=skip_validation",
    ],
    binary = "dawn_end2end_tests",
)

targets.tests.gtest_test(
    name = "dawn_end2end_swangle_tests",
    mixins = [
        "result_adapter_gtest_json",
        "true_noop_merge",
        targets.mixin(
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    ],
    args = [
        "--backend=opengles",
        "--use-angle=swiftshader",
        "--enable-toggles=gl_force_es_31_and_no_extensions",
    ],
    binary = "dawn_end2end_tests",
)

targets.tests.gtest_test(
    name = "dawn_end2end_sws_tests",
    mixins = [
        "adapter_vendor_id_sws",
        "result_adapter_gtest_json",
        "true_noop_merge",
        targets.mixin(
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
    ],
    binary = "dawn_end2end_tests",
)

targets.tests.gtest_test(
    name = "dawn_end2end_tests",
    mixins = [
        "result_adapter_gtest_json",
        "true_noop_merge",
        targets.mixin(
            swarming = targets.swarming(
                # Primarily needed for Windows.
                shards = 2,
            ),
        ),
    ],
    binary = "dawn_end2end_tests",
)

targets.tests.gtest_test(
    name = "dawn_end2end_validation_layers_tests",
    mixins = [
        "enable_backend_validation",
        "result_adapter_gtest_json",
        "true_noop_merge",
        targets.mixin(
            swarming = targets.swarming(
                # Primarily needed for Windows.
                shards = 3,
            ),
        ),
    ],
    binary = "dawn_end2end_tests",
)

targets.tests.gtest_test(
    name = "dawn_end2end_warp_tests",
    mixins = [
        "result_adapter_gtest_json",
        "true_noop_merge",
        targets.mixin(
            swarming = targets.swarming(
                # Very slow on debug builds.
                shards = 6,
            ),
        ),
    ],
    args = [
        "--adapter-vendor-id=0x1414",
    ],
    binary = "dawn_end2end_tests",
)

targets.tests.gtest_test(
    name = "dawn_end2end_wire_tests",
    mixins = [
        "result_adapter_gtest_json",
        "true_noop_merge",
        "use_wire",
        targets.mixin(
            swarming = targets.swarming(
                # Primarily needed for Windows.
                shards = 2,
            ),
        ),
    ],
    binary = "dawn_end2end_tests",
)

targets.tests.isolated_script_test(
    name = "dawn_node_software_d3d12_cts",
    mixins = [
        "result_adapter_single",
        "true_noop_merge",
    ],
    args = [
        "-backend",
        "d3d12",
        "-adapter",
        "Microsoft Basic Render Driver",
        "webgpu:api,operation,adapter,requestDevice:default:*",
    ],
    binary = "dawn_node_cts",
)

targets.tests.isolated_script_test(
    name = "dawn_node_sws_cts",
    mixins = [
        "result_adapter_single",
        "true_noop_merge",
    ],
    args = [
        "-backend",
        "vulkan",
        "-adapter",
        "SwiftShader",
        "webgpu:api,operation,adapter,requestDevice:default:*",
    ],
    binary = "dawn_node_cts",
)

# This is run as a gtest instead of an isolated script since on the bots
# these are used more as a smoke test/to ensure that they continue to run
# rather than for actual perf results.
targets.tests.gtest_test(
    name = "dawn_perf_tests",
    mixins = [
        "result_adapter_gtest_json",
        "true_noop_merge",
        targets.mixin(
            linux_args = [
                "--no-xvfb",
            ],
        ),
    ],
    args = [
        "--test-launcher-print-test-stdio=always",
        "--test-launcher-jobs=1",
        "--test-launcher-retry-limit=0",
        # Tell the tests to only run one step for faster iteration.
        "--override-steps=1",
    ],
    binary = "dawn_perf_tests",
)

targets.tests.gtest_test(
    name = "dawn_unittests",
    mixins = [
        "result_adapter_gtest_json",
        "true_noop_merge",
    ],
    binary = "dawn_unittests",
)

targets.tests.gtest_test(
    name = "dawn_wire_trace_end2end_sws_tests",
    mixins = [
        "adapter_vendor_id_sws",
        "clusterfuzz_wire_trace_dir",
        "result_adapter_gtest_json",
        "use_wire",
        "wire_trace_merge",
    ],
    binary = "dawn_end2end_tests",
)

targets.tests.gtest_test(
    name = "dawn_wire_trace_unittests",
    mixins = [
        "clusterfuzz_wire_trace_dir",
        "result_adapter_gtest_json",
        "use_wire",
        "wire_trace_merge",
    ],
    binary = "dawn_unittests",
)

targets.tests.gtest_test(
    name = "dawn_wire_unittests",
    mixins = [
        "result_adapter_gtest_json",
        "true_noop_merge",
        "use_wire",
    ],
    binary = "dawn_unittests",
)

targets.tests.isolated_script_test(
    name = "tint_benchmark",
    mixins = [
        "result_adapter_single",
        "true_noop_merge",
    ],
    binary = "benchmarks",
)

targets.tests.isolated_script_test(
    name = "tint_ir_fuzzer_corpus_check_tests",
    mixins = [
        "result_adapter_single",
        "tint_fuzzer_corpus_common_args",
        "true_noop_merge",
    ],
    args = [
        "-check",
        "-ir",
    ],
    binary = "fuzzer_corpus_tests",
)

targets.tests.isolated_script_test(
    name = "tint_ir_fuzzer_corpus_generate_tests",
    mixins = [
        "result_adapter_single",
        "tint_fuzzer_corpus_common_args",
        "tint_fuzzer_corpus_generate_args",
        "tint_ir_merge",
    ],
    args = [
        "-ir",
    ],
    binary = "fuzzer_corpus_tests",
)

targets.tests.gtest_test(
    name = "tint_unittests",
    mixins = [
        "result_adapter_gtest_json",
        "true_noop_merge",
    ],
    binary = "tint_unittests",
)

targets.tests.isolated_script_test(
    name = "tint_wgsl_fuzzer_corpus_check_tests",
    mixins = [
        "result_adapter_single",
        "tint_fuzzer_corpus_common_args",
        "true_noop_merge",
        targets.mixin(
            swarming = targets.swarming(
                # These tests normally take ~15 minutes, but can flakily hit the
                # default 20 minute I/O timeout and cannot currently be sharded.
                io_timeout_sec = 1800,
            ),
        ),
    ],
    args = [
        "-check",
    ],
    binary = "fuzzer_corpus_tests",
)

targets.tests.isolated_script_test(
    name = "tint_wgsl_fuzzer_corpus_generate_tests",
    mixins = [
        "result_adapter_single",
        "tint_fuzzer_corpus_common_args",
        "tint_fuzzer_corpus_generate_args",
        "tint_wgsl_merge",
    ],
    binary = "fuzzer_corpus_tests",
)
