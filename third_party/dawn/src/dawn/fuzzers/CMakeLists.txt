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

if (NOT DAWN_BUILD_FUZZERS)
    return()
endif()

function(set_rtti target)
    if (NOT DAWN_ENABLE_RTTI)
        target_compile_options(${target} PRIVATE -fno-rtti)
    endif()
endfunction()

add_library(dawn_wire_server_fuzzer_common STATIC
    DawnWireServerFuzzer.cpp
    DawnWireServerFuzzer.h
)

target_link_libraries(dawn_wire_server_fuzzer_common PRIVATE
    dawn_common
    dawn_native
    dawn_wire
    dawn_proc
    dawn_internal_config
)
set_rtti(dawn_wire_server_fuzzer_common)

add_executable(dawn_wire_server_and_frontend_fuzzer
    DawnWireServerAndFrontendFuzzer.cpp
)

target_link_libraries(dawn_wire_server_and_frontend_fuzzer PRIVATE
    dawn_wire_server_fuzzer_common
    dawn_common
    dawn_native
    dawn_internal_config
)
set_rtti(dawn_wire_server_and_frontend_fuzzer)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(dawn_wire_server_and_frontend_fuzzer PRIVATE "-fsanitize=fuzzer")
    target_link_options(dawn_wire_server_and_frontend_fuzzer PRIVATE "-fsanitize=fuzzer")
endif()

if (DAWN_ENABLE_VULKAN)
    add_executable(dawn_wire_server_and_vulkan_backend_fuzzer
        DawnWireServerAndVulkanBackendFuzzer.cpp
    )

    target_link_libraries(dawn_wire_server_and_vulkan_backend_fuzzer PRIVATE
        dawn_wire_server_fuzzer_common
        dawn_common
        dawn_native
        dawn_internal_config
    )
    set_rtti(dawn_wire_server_and_vulkan_backend_fuzzer)

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(dawn_wire_server_and_vulkan_backend_fuzzer PRIVATE "-fsanitize=fuzzer")
        target_link_options(dawn_wire_server_and_vulkan_backend_fuzzer PRIVATE "-fsanitize=fuzzer")
    endif()
endif()

if (DAWN_ENABLE_D3D12)
    add_executable(dawn_wire_server_and_d3d12_backend_fuzzer
        DawnWireServerAndD3D12BackendFuzzer.cpp
    )

    target_link_libraries(dawn_wire_server_and_d3d12_backend_fuzzer PRIVATE
        dawn_wire_server_fuzzer_common
        dawn_common
        dawn_native
        dawn_internal_config
    )
    set_rtti(dawn_wire_server_and_d3d12_backend_fuzzer)

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(dawn_wire_server_and_d3d12_backend_fuzzer PRIVATE "-fsanitize=fuzzer")
        target_link_options(dawn_wire_server_and_d3d12_backend_fuzzer PRIVATE "-fsanitize=fuzzer")
    endif()
endif()
