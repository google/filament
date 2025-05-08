# Copyright 2020 The Dawn & Tint Authors
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

###############################################################################
# Dawn sample utilities
###############################################################################
set(sample_util_depends
    webgpu_cpp
    dawn_internal_config
    dawn::dawn_common
    dawn::dawn_system_utils
    dawn::dawn_wgpu_utils
)
if (NOT EMSCRIPTEN)
    list(APPEND sample_util_depends
        dawn::dawn_test_utils
        dawn::dawn_proc
        dawn::dawn_glfw
        dawn::dawn_native
        glfw
    )
endif ()

dawn_add_library(
    dawn_sample_utils
    FORCE_STATIC
    ENABLE_EMSCRIPTEN
    HEADERS
        "SampleUtils.h"
    SOURCES
        "SampleUtils.cpp"
    DEPENDS
        ${sample_util_depends}
)

###############################################################################
# Dawn samples
#   - Note that not all samples are currently buildable for Emscripten.
###############################################################################

function(Sample)
    cmake_parse_arguments(PARSE_ARGV 0 arg
        "ENABLE_EMSCRIPTEN"
        "NAME"
        "SOURCES"
    )

    # Skip targets that shouldn't be built with Emscripten.
    if (EMSCRIPTEN AND NOT arg_ENABLE_EMSCRIPTEN)
        return()
    endif ()

    add_executable(${arg_NAME} ${arg_SOURCES})
    if (EMSCRIPTEN)
        set_target_properties(${arg_NAME} PROPERTIES
            SUFFIX ".html")
        target_link_options(${arg_NAME} PUBLIC
            # We need JSPI for Future implementation.
            "-sJSPI"
        )
    endif()
    target_link_libraries(${arg_NAME} PUBLIC dawn::dawn_sample_utils)
    common_compile_options(${arg_NAME})
endfunction()

Sample(
    NAME HelloTriangle
    ENABLE_EMSCRIPTEN
    SOURCES "HelloTriangle.cpp"
)

Sample(
    NAME ComputeBoids
    ENABLE_EMSCRIPTEN
    SOURCES "ComputeBoids.cpp"
)

Sample(
    NAME Animometer
    ENABLE_EMSCRIPTEN
    SOURCES "Animometer.cpp"
)

Sample(
    NAME DawnInfo
    SOURCES "DawnInfo.cpp"
)

Sample(
    NAME ManualSurfaceTest
    SOURCES "ManualSurfaceTest.cpp"
)
