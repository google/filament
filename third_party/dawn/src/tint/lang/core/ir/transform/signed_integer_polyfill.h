// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_SIGNED_INTEGER_POLYFILL_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_SIGNED_INTEGER_POLYFILL_H_

#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/reflection.h"
#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}  // namespace tint::core::ir

namespace tint::core::ir::transform {

/// The set of polyfills that should be applied.
struct SignedIntegerPolyfillConfig {
    /// Should signed negation be polyfilled to avoid integer overflow?
    bool signed_negation = false;

    /// Should signed arithmetic be polyfilled to avoid integer overflow?
    bool signed_arithmetic = false;

    /// Should signed shiftleft be polyfilled to avoid integer overflow?
    bool signed_shiftleft = false;

    /// Reflection for this class
    TINT_REFLECT(SignedIntegerPolyfillConfig, signed_negation, signed_arithmetic, signed_shiftleft);
};

/// SignedIntegerPolyfill is a transform that replaces signed integer instructions with polyfills.
/// @param module the module to transform
/// @returns success or failure
Result<SuccessType> SignedIntegerPolyfill(core::ir::Module& module,
                                          const SignedIntegerPolyfillConfig& cfg);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_SIGNED_INTEGER_POLYFILL_H_
