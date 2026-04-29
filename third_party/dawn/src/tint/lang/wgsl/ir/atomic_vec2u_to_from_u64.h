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

#ifndef SRC_TINT_LANG_WGSL_IR_ATOMIC_VEC2U_TO_FROM_U64_H_
#define SRC_TINT_LANG_WGSL_IR_ATOMIC_VEC2U_TO_FROM_U64_H_

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/utils/result.h"

namespace tint::wgsl::ir::transform {

/// The direction of the atomic_vec2u_u64 transform.
enum class AtomicVec2uU64Direction {
    /// `atomic<vec2<u32>>` to `atomic<u64>`
    kToU64,
    /// `atomic<u64>` to `atomic<vec2<u32>>`
    kFromU64,
};

/// AtomicVec2uU64 is a transform that converts `atomic<vec2<u32>>` to and from `atomic<u64>`.
/// This is used to polyfill 64-bit atomic operations which are not natively supported in WGSL.
/// @param mod the module to transform
/// @param direction the direction of the transformation
/// @return success or failure
Result<SuccessType> AtomicVec2uToFromU64(core::ir::Module& mod, AtomicVec2uU64Direction direction);

}  // namespace tint::wgsl::ir::transform

#endif  // SRC_TINT_LANG_WGSL_IR_ATOMIC_VEC2U_TO_FROM_U64_H_
