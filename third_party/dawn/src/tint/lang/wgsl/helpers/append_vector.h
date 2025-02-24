// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_HELPERS_APPEND_VECTOR_H_
#define SRC_TINT_LANG_WGSL_HELPERS_APPEND_VECTOR_H_

#include "src/tint/lang/wgsl/program/program_builder.h"

// Forward declarations
namespace tint::ast {
class CallExpression;
class Expression;
}  // namespace tint::ast
namespace tint::sem {
class Call;
}  // namespace tint::sem

namespace tint::wgsl {

/// A helper function used to append a vector with an additional scalar.
/// If the scalar's type does not match the target vector element type,
/// then it is value-converted (via ValueConstructor) before being added.
/// All types must have been assigned to the expressions and their child nodes
/// before calling.
/// @param builder the program builder.
/// @param vector the vector to be appended. May be a scalar, `vec2` or `vec3`.
/// @param scalar the scalar to append to the vector. Must be a scalar.
/// @returns a vector expression containing the elements of `vector` followed by
/// the single element of `scalar` cast to the `vector` element type.
const sem::Call* AppendVector(ProgramBuilder* builder,
                              const ast::Expression* vector,
                              const ast::Expression* scalar);

}  // namespace tint::wgsl

#endif  // SRC_TINT_LANG_WGSL_HELPERS_APPEND_VECTOR_H_
