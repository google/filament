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

#ifndef SRC_TINT_LANG_CORE_IR_VALIDATOR_H_
#define SRC_TINT_LANG_CORE_IR_VALIDATOR_H_

#include <vector>

#include "src/tint/utils/containers/enum_set.h"
#include "src/tint/utils/result.h"

// Forward declarations
namespace tint::core::ir {
class Module;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// Enumerator of optional IR capabilities.
enum class Capability : uint8_t {
    /// Allows 8-bit integer types.
    kAllow8BitIntegers,
    /// Allows 16-bit integer types.
    kAllow16BitIntegers,
    /// Allows 64-bit integer types.
    kAllow64BitIntegers,
    /// Allows ClipDistances on f32 and vecN<f32> parameters
    kAllowClipDistancesOnF32ScalarAndVector,
    /// Allows handle vars to not have binding points
    kAllowHandleVarsWithoutBindings,
    /// Allows module scoped lets
    kAllowModuleScopeLets,
    /// Allows multiple entry points in the module.
    kAllowMultipleEntryPoints,
    /// Allow overrides
    kAllowOverrides,
    /// Allows ref types
    kAllowRefTypes,
    /// Allows access instructions to create pointers to vector elements.
    kAllowVectorElementPointer,
    /// Allows phony assignment instructions to be used.
    kAllowPhonyInstructions,
    /// Allows lets to have any type, used by MSL backend for module scoped vars
    kAllowAnyLetType,
    /// Allows input_attachment_index to be associated with any type, used by
    /// SPIRV backend for spirv.image.
    kAllowAnyInputAttachmentIndexType,
    /// Allows binding points to be non-unique. Used after BindingRemapper is
    /// invoked by MSL & GLSL backends.
    kAllowDuplicateBindings,
    /// Allows module scope `var`s to exist without an IO annotation
    kAllowUnannotatedModuleIOVariables,
    /// Allows non-core types in the IR module
    kAllowNonCoreTypes,
    /// Allows matrix annotations on structure members
    kAllowStructMatrixDecorations,
    /// Allows @location on structs, matrices, and arrays that have numeric elements
    kAllowLocationForNumericElements,
    /// Allows a pointer to a handle type
    kAllowPointerToHandle,
    /// Allows ShaderIO specific features, like blend_src on non-struct members.
    /// These are not separate capabilities, because they are enabled/disabled in lockstep with each
    /// other.
    /// TODO(448417342): Validate in/out address space usage based on this capability
    kLoosenValidationForShaderIO,
    /// Allows the PointSize builtin to be used.
    kAllowPointSizeBuiltin,
    /// Allows MSL specific entry point variance.
    /// Specifically pointers and handle address space variables inside structures, private address
    /// space variables in function scopes, workgroup address space pointers as entry point inputs,
    /// binding point on non-module scope variables in entry point interface.
    kMslAllowEntryPointInterface,
};

/// Capabilities is a set of Capability
using Capabilities = EnumSet<Capability>;

/// Validates the module @p ir is correctly formed
/// @param mod the module to validate
/// @param capabilities the optional capabilities that are allowed
/// @param msg the msg to accompany the output
/// @returns success or failure
Result<SuccessType> Validate(const Module& mod,
                             Capabilities capabilities = {},
                             std::string_view msg = "");

/// Validates the module @p ir is correctly formed, iff required by the build configuration.
/// @param mod the module to transform
/// @param capabilities the optional capabilities that are allowed
/// @param msg the msg to accompany the output
void AssertValid(const Module& mod, Capabilities capabilities = {}, std::string_view msg = "");

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_VALIDATOR_H_
