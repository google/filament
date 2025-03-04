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

#ifndef SRC_TINT_LANG_GLSL_WRITER_WRITER_H_
#define SRC_TINT_LANG_GLSL_WRITER_WRITER_H_

#include <string>

#include "src/tint/lang/glsl/writer/common/options.h"
#include "src/tint/lang/glsl/writer/common/output.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/result/result.h"

// Forward declarations
namespace tint {
class Program;
}  // namespace tint
namespace tint::core::ir {
class Module;
}  // namespace tint::core::ir

namespace tint::glsl::writer {

/// Check if the module @p ir is supported by the GLSL backend with @p options.
/// @param ir the module
/// @param options the writer options
/// @returns Success or a failure message indicating why GLSL generation would fail
Result<SuccessType> CanGenerate(const core::ir::Module& ir, const Options& options);

/// Generate GLSL for a program, according to a set of configuration options.
/// The result will contain the GLSL and supplementary information, or failure.
/// information.
/// @param ir the IR module to translate to GLSL
/// @param options the configuration options to use when generating GLSL
/// @param entry_point the entry point to generate GLSL for
/// @returns the resulting GLSL and supplementary information, or failure
Result<Output> Generate(core::ir::Module& ir,
                        const Options& options,
                        const std::string& entry_point);

}  // namespace tint::glsl::writer

#endif  // SRC_TINT_LANG_GLSL_WRITER_WRITER_H_
