// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_READER_PROGRAM_TO_IR_PROGRAM_TO_IR_H_
#define SRC_TINT_LANG_WGSL_READER_PROGRAM_TO_IR_PROGRAM_TO_IR_H_

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/utils/result.h"

// Forward Declarations
namespace tint {
class Program;
}  // namespace tint

namespace tint::wgsl::reader {

/// Builds a WGSL-dialect core::ir::Module from the given Program
/// @param program the Program to use.
/// @returns the WGSL-dialect IR module.
///
/// @note this assumes the `program.IsValid()`, and has had const-eval done so
/// any abstract values have been calculated and converted into the relevant
/// concrete types.
Result<core::ir::Module> ProgramToIR(const Program& program);

}  // namespace tint::wgsl::reader

#endif  // SRC_TINT_LANG_WGSL_READER_PROGRAM_TO_IR_PROGRAM_TO_IR_H_
