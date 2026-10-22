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

#ifndef SRC_TINT_CMD_FUZZ_COMMON_INIT_H_
#define SRC_TINT_CMD_FUZZ_COMMON_INIT_H_

#include "src/tint/cmd/fuzz/common/options.h"
#include "src/utils/compiler.h"

namespace tint::fuzz::common {

/// FuzzerType distinguishes between fuzzers based on their input format
enum class FuzzerType {
    /// IR-input fuzzer
    kIR,
    /// WGSL-input fuzzer
    kWGSL,
};

/// ParseFuzzerOptions parses the command line arguments and populates the fuzzer options.
/// @param type the type of fuzzer
/// @param argc the number of command line arguments
/// @param argv the command line arguments
/// @param options the options to populate
/// @returns 0 on success, or a non-zero value on failure
int ParseFuzzerOptions(FuzzerType type, int* argc, char*** argv, Options* options);

}  // namespace tint::fuzz::common

#endif  // SRC_TINT_CMD_FUZZ_COMMON_INIT_H_
