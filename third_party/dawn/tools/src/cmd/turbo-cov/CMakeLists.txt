# Copyright 2022 The Dawn & Tint Authors
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

project(turbo-cov)

if (LLVM_SOURCE_DIR)
    set(LLVM_INCLUDE_TESTS OFF)
    if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") AND WIN32)
        set(HAVE_CXX_ATOMICS_WITHOUT_LIB TRUE)
        set(HAVE_CXX_ATOMICS64_WITHOUT_LIB TRUE)
        set(LLVM_HOST_TRIPLE "x86_64-pc-win32")
    endif()
    set(LLVM_TARGETS_TO_BUILD "X86") # At least one target needs to be provided
    add_subdirectory("${LLVM_SOURCE_DIR}" "llvm" EXCLUDE_FROM_ALL)

    add_executable(turbo-cov
        main.cpp
    )

    set_target_properties(turbo-cov PROPERTIES
        FOLDER "Tests"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        INCLUDE_DIRECTORIES "${LLVM_BINARY_DIR}/include;${LLVM_SOURCE_DIR}/include"
    )

    target_link_libraries(turbo-cov
        PRIVATE
            LLVMProfileData
            LLVMCoverage
    )

    common_compile_options(turbo-cov)
else (LLVM_SOURCE_DIR)
    message("not building turbo-cov as LLVM_SOURCE_DIR is undefined")
endif (LLVM_SOURCE_DIR)
