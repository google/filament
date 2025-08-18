// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_HLSL_WRITER_RAISE_PROMOTE_INITIALIZERS_H_
#define SRC_TINT_LANG_HLSL_WRITER_RAISE_PROMOTE_INITIALIZERS_H_

#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::hlsl::writer::raise {

/// The capabilities that the transform can support.
const core::ir::Capabilities kPromoteInitializersCapabilities{
    core::ir::Capability::kAllowVectorElementPointer,
    core::ir::Capability::kAllowClipDistancesOnF32,
    core::ir::Capability::kAllowDuplicateBindings,
    core::ir::Capability::kAllowNonCoreTypes,
};

/// PromoteInitializers is a transform that moves inline struct and array initializers to a `let`
/// unless the initializer is already in a `let ` or `var`. For any `var` at the module scope it
/// will recursively break any array or struct initializers out of the constant into their own
/// `let`.
///
/// After this transform the `Capability::kAllowModuleScopeLets` must be enabled and any downstream
/// transform/printer must under stand `let` and `construct` instructions at the module scope.
/// (`construct` can just be skipped as they will be inlined, but the instruction still has to be
/// handled.)
///
/// For example:
///
/// ```wgsl
/// struct A {
///   b: f32,
/// }
/// struct S {
///   a: A
/// }
/// var<private> p = S(A(1.f));
/// ```
///
/// Essentially creates:
///
/// ```wgsl
/// struct S {
///   a: i32,
/// }
/// let v: A = A(1.f);
/// let v_1: S = S(v);
/// var p = v_1;
/// ```
///
/// @param module the module to transform
/// @returns error on failure
Result<SuccessType> PromoteInitializers(core::ir::Module& module);

}  // namespace tint::hlsl::writer::raise

#endif  // SRC_TINT_LANG_HLSL_WRITER_RAISE_PROMOTE_INITIALIZERS_H_
