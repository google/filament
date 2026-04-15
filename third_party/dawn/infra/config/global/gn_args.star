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

"""Dawn GN arg definitions."""

load("@chromium-luci//gn_args.star", "gn_args")

gn_args.config(
    name = "arm64",
    args = {
        "target_cpu": "arm64",
    },
)

gn_args.config(
    name = "asan",
    args = {
        "is_asan": True,
    },
)

gn_args.config(
    name = "clang",
    args = {
        "is_clang": True,
    },
)

gn_args.config(
    name = "component",
    args = {
        "is_component_build": True,
    },
)

gn_args.config(
    name = "dawn_no_d3d12",
    args = {
        "dawn_enable_d3d12": False,
    },
)

gn_args.config(
    name = "dawn_no_swiftshader",
    args = {
        "dawn_use_swiftshader": False,
    },
)

gn_args.config(
    name = "dawn_node_bindings",
    args = {
        "dawn_build_node_bindings": True,
    },
)

gn_args.config(
    name = "dawn_swiftshader",
    args = {
        "dawn_use_swiftshader": True,
    },
)

gn_args.config(
    name = "debug",
    args = {
        "is_debug": True,
    },
)

gn_args.config(
    name = "libfuzzer",
    args = {
        "use_libfuzzer": True,
    },
)

gn_args.config(
    name = "linux",
    args = {
        "target_os": "linux",
    },
)

gn_args.config(
    name = "linux_clang",
    configs = [
        "clang",
        "dawn_no_d3d12",
        "linux",
        "siso",
        "tint_hlsl_writer",
        "tint_msl_writer",
        "tint_spv_reader_writer",
        "tint_wgsl_reader_writer",
    ],
)

gn_args.config(
    name = "mac",
    args = {
        "target_os": "mac",
    },
)

gn_args.config(
    name = "mac_clang",
    configs = [
        "clang",
        "dawn_no_d3d12",
        "mac",
        "siso",
        "tint_hlsl_writer",
        "tint_msl_writer",
        "tint_spv_reader_writer",
        "tint_wgsl_reader_writer",
    ],
)

gn_args.config(
    name = "minimal_symbols",
    args = {
        "symbol_level": 1,
    },
)

gn_args.config(
    name = "msvc",
    args = {
        "is_clang": False,
    },
)

gn_args.config(
    name = "no_custom_libcxx",
    args = {
        "use_custom_libcxx": False,
    },
)

gn_args.config(
    name = "non_component",
    args = {
        "is_component_build": False,
    },
)

gn_args.config(
    name = "release",
    args = {
        "is_debug": False,
    },
)

gn_args.config(
    name = "release_with_dchecks",
    args = {
        "dcheck_always_on": True,
    },
    configs = [
        "release",
    ],
)

gn_args.config(
    name = "siso",
    args = {
        "use_reclient": False,
        "use_remoteexec": True,
        "use_siso": True,
    },
)

gn_args.config(
    name = "tint_build_ir_binary",
    args = {
        "tint_build_ir_binary": True,
    },
)

gn_args.config(
    name = "tint_hlsl_writer",
    args = {
        "tint_build_hlsl_writer": True,
    },
)

gn_args.config(
    name = "tint_msl_writer",
    args = {
        "tint_build_msl_writer": True,
    },
)

gn_args.config(
    name = "tint_spv_reader",
    args = {
        "tint_build_spv_reader": True,
    },
)

gn_args.config(
    name = "tint_spv_reader_writer",
    configs = [
        "tint_spv_reader",
        "tint_spv_writer",
    ],
)

gn_args.config(
    name = "tint_spv_writer",
    args = {
        "tint_build_spv_writer": True,
    },
)

gn_args.config(
    name = "tint_wgsl_reader",
    args = {
        "tint_build_wgsl_reader": True,
    },
)

gn_args.config(
    name = "tint_wgsl_reader_writer",
    configs = [
        "tint_wgsl_reader",
        "tint_wgsl_writer",
    ],
)

gn_args.config(
    name = "tint_wgsl_writer",
    args = {
        "tint_build_wgsl_writer": True,
    },
)

gn_args.config(
    name = "tsan",
    args = {
        "is_tsan": True,
    },
)

gn_args.config(
    name = "win",
    args = {
        "target_os": "win",
    },
)

gn_args.config(
    name = "win_clang",
    configs = [
        "clang",
        "siso",
        "tint_hlsl_writer",
        "tint_msl_writer",
        "tint_spv_reader_writer",
        "tint_wgsl_reader_writer",
        "win",
    ],
)

gn_args.config(
    name = "win_msvc",
    configs = [
        "msvc",
        "no_custom_libcxx",
        "siso",
        "tint_hlsl_writer",
        "tint_msl_writer",
        "tint_spv_reader_writer",
        "tint_wgsl_reader_writer",
        "win",
    ],
)

gn_args.config(
    name = "x64",
    args = {
        "target_cpu": "x64",
    },
)

gn_args.config(
    name = "x86",
    args = {
        "target_cpu": "x86",
    },
)
