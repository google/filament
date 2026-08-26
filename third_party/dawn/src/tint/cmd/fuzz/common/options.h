// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_CMD_FUZZ_COMMON_OPTIONS_H_
#define SRC_TINT_CMD_FUZZ_COMMON_OPTIONS_H_

#include <string>

namespace tint::fuzz::common {

/// Options for the fuzzer.
struct Options {
    /// If not empty, only run the fuzzers with the given substring.
    std::string filter;
    /// If true, the fuzzers will be run concurrently on separate threads.
    bool run_concurrently = false;
    /// If true, print the fuzzer name to stdout before running.
    bool verbose = false;
    /// If not empty, load DXC from this path when fuzzing HLSL generation.
    std::string dxc;
#if TINT_BUILD_FUZZER_VULKAN_SUPPORT
    /// If not empty, load the Vulkan ICD from this path when fuzzing SPIR-V generation.
    std::string vk_icd;
#endif
    /// If true, dump shader input/output text to stdout
    bool dump = false;
    /// If true, dump the IR whenever validation is performed.
    bool dump_ir_when_validating = false;
    /// If true, invalid identifiers will be stripped instead of erroring
    bool strip_invalid_identifiers = false;
    /// If true, the IR validator will be disabled.
    bool disable_ir_validator = false;
};

}  // namespace tint::fuzz::common

#endif  // SRC_TINT_CMD_FUZZ_COMMON_OPTIONS_H_
