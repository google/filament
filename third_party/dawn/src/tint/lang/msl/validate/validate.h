// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_MSL_VALIDATE_VALIDATE_H_
#define SRC_TINT_LANG_MSL_VALIDATE_VALIDATE_H_

#include <string>
#include <utility>

namespace tint::msl::validate {

/// The version of MSL to validate against.
/// Note: these must kept be in ascending order
enum class MslVersion {
    kMsl_2_3,
};

/// MslVersion less-than operator
inline bool operator<(MslVersion a, MslVersion b) {
    return static_cast<int>(a) < static_cast<int>(b);
}

/// The return structure of Validate()
struct Result {
    /// True if validation passed
    bool failed = false;
    /// Output of Metal compiler.
    std::string output;
};

/// Validate attempts to compile the shader with the Metal Shader Compiler,
/// verifying that the shader compiles successfully.
/// @param xcrun_path path to xcrun
/// @param source the generated MSL source
/// @param version the version of MSL to validate against
/// @return the result of the compile
Result Validate(const std::string& xcrun_path, const std::string& source, MslVersion version);

#if TINT_BUILD_IS_MAC
/// ValidateUsingMetal attempts to compile the shader with the runtime Metal Shader Compiler
/// API, verifying that the shader compiles successfully.
/// @param source the generated MSL source
/// @param version the version of MSL to validate against
/// @return the result of the compile
Result ValidateUsingMetal(const std::string& source, MslVersion version);
#endif  // TINT_BUILD_IS_MAC

}  // namespace tint::msl::validate

#endif  // SRC_TINT_LANG_MSL_VALIDATE_VALIDATE_H_
