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

"""Starlark code related to common CQ location filters for standalone builders."""

_WEBGPU_CTS_LOCATIONS_EXCLUDED_FROM_CQ = [
    # WebGPU CTS expectations, only affects builders that run WebGPU CTS, i.e.
    # Chromium builders.
    cq.location_filter(
        path_regexp = "webgpu-cts/[^/]*expectations.txt",
        exclude = True,
    ),
]

# These are commonly touched locations which do not have relevant testing on
# builders which do not use `tools/run` for tests.
_GO_LOCATIONS_EXCLUDED_FROM_CQ = [
    # Tools written in Go.
    cq.location_filter(
        path_regexp = "tools/src/.+",
        exclude = True,
    ),
    # Go dependencies.
    cq.location_filter(
        path_regexp = "go\\.(mod|sum)",
        exclude = True,
    ),
]

_GITHUB_LOCATIONS_EXCLUDED_FROM_CQ = [
    cq.location_filter(
        path_regexp = "\\.github/.+",
        exclude = True,
    ),
]

# These are commonly touched locations which do not have relevant testing on
# standard CQ builders.
_COMMON_LOCATIONS_EXCLUDED_FROM_CQ = _WEBGPU_CTS_LOCATIONS_EXCLUDED_FROM_CQ

_NON_CMAKE_NON_FUZZ_LOCATIONS_EXCLUDED_FROM_CQ = [
    cq.location_filter(
        path_regexp = "test/tint/.+",
        exclude = True,
    ),
]

_CHROMIUM_CQ_FILE_EXCLUSIONS = _GITHUB_LOCATIONS_EXCLUDED_FROM_CQ + _GO_LOCATIONS_EXCLUDED_FROM_CQ

_CMAKE_CQ_FILE_EXCLUSIONS = (_COMMON_LOCATIONS_EXCLUDED_FROM_CQ + _GO_LOCATIONS_EXCLUDED_FROM_CQ + _GITHUB_LOCATIONS_EXCLUDED_FROM_CQ)

# The `gn analyze` step automatically run as part of the gn_v2_trybot recipe
# will already stop the build if no compilation is needed, but we can save some
# resources by not starting a build at all if we know no relevant files are
# touched.
_GN_CLANG_CQ_FILE_EXCLUSIONS = _COMMON_LOCATIONS_EXCLUDED_FROM_CQ + _NON_CMAKE_NON_FUZZ_LOCATIONS_EXCLUDED_FROM_CQ

# Fuzz builders rely on both Go tools and Tint test data.
_GN_CLANG_CQ_FUZZ_FILE_EXCLUSIONS = _WEBGPU_CTS_LOCATIONS_EXCLUDED_FROM_CQ

# Standard CQ builders that don't run Node tests don't rely on Go code.
_GN_CLANG_NO_NODE_CQ_FILE_EXCLUSIONS = _GN_CLANG_CQ_FILE_EXCLUSIONS + _GO_LOCATIONS_EXCLUDED_FROM_CQ

_GN_MSVC_CQ_FILE_EXCLUSIONS = _COMMON_LOCATIONS_EXCLUDED_FROM_CQ + _NON_CMAKE_NON_FUZZ_LOCATIONS_EXCLUDED_FROM_CQ

exclusion_filters = struct(
    chromium_cq_file_exclusions = _CHROMIUM_CQ_FILE_EXCLUSIONS,
    cmake_cq_file_exclusions = _CMAKE_CQ_FILE_EXCLUSIONS,
    gn_clang_cq_file_exclusions = _GN_CLANG_CQ_FILE_EXCLUSIONS,
    gn_clang_cq_fuzz_file_exclusions = _GN_CLANG_CQ_FUZZ_FILE_EXCLUSIONS,
    gn_clang_no_node_cq_file_exclusions = _GN_CLANG_NO_NODE_CQ_FILE_EXCLUSIONS,
    gn_msvc_cq_file_exclusions = _GN_MSVC_CQ_FILE_EXCLUSIONS,
)
