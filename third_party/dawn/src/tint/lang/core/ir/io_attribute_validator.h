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

#ifndef SRC_TINT_LANG_CORE_IR_IO_ATTRIBUTE_VALIDATOR_H_
#define SRC_TINT_LANG_CORE_IR_IO_ATTRIBUTE_VALIDATOR_H_

#include <string>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/io_attributes.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/utils/containers/enum_set.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/vector.h"

namespace tint::core::ir::validator {

/// The IO direction of an operation.
enum class IODirection : uint8_t {
    kInput,
    kOutput,
    kResource,
};
std::string_view ToString(IODirection value);

/// The kind of shader IO being validated.
enum class ShaderIOKind : uint8_t {
    kInputParam,
    kResultValue,
    kModuleScopeVar,
};
std::string ToString(ShaderIOKind value);

/// How an attribute is being used, a tuple of the shader stage and IO direction
enum class IOAttributeUsage : uint8_t {
    kComputeInputUsage,
    kComputeOutputUsage,
    kComputeResourceUsage,
    kFragmentInputUsage,
    kFragmentOutputUsage,
    kFragmentResourceUsage,
    kVertexInputUsage,
    kVertexOutputUsage,
    kVertexResourceUsage,
    kUndefinedUsage,
};
std::string ToString(IOAttributeUsage value);

/// Annotations that can be associated with a value that are used for shader IO,
/// e.g. binding_points, @location, being in workgroup address space, etc.
/// These are a subset of IOAttributes.
enum class IOAnnotation : uint8_t {
    /// @group + @binding
    kBindingPoint,
    /// @location
    kLocation,
    /// @builtin(...)
    kBuiltin,
    /// Pointer to Workgroup address space
    kWorkgroup,
    /// @color
    kColor,
};
std::string ToString(IOAnnotation value);

/// Adds appropriate entries to annotations, based on what values are present in attributes
/// @param annotations the set to updated
/// @param attr the attributes to be examined
/// @returns Success if none of the values being added where already present, otherwise returns the
/// first non-unique value as a Failure
Result<SuccessType, IOAnnotation> AddIOAnnotationsFromIOAttributes(
    EnumSet<IOAnnotation>& annotations,
    const IOAttributes& attr);

/// @returns a human-readable string of all the entries in a EnumSet
template <typename T>
std::string ToString(const EnumSet<T>& values) {
    std::stringstream result;
    result << "[ ";
    bool first = true;
    for (auto v : values) {
        if (!first) {
            result << ", ";
        }
        first = false;
        result << ToString(v);
    }
    result << " ]";
    return result.str();
}

/// State for validating IO attributes that needs to shared across impl invocations within the same
/// entry point.
struct IOAttributeContext {
    Hashmap<BuiltinValue, uint32_t, 4> input_builtins;
    Hashmap<BuiltinValue, uint32_t, 4> output_builtins;
};

/// IOAttributeChecker is the interface used to check that a usage of an IO attribute
/// meets the spec rules for a given context.
struct IOAttributeChecker {
    /// What kinda of IO attribute is being checked
    IOAttributeKind kind;

    /// What combination of stage and IO direction is this attribute legal for.
    EnumSet<IOAttributeUsage> valid_usages;

    /// What type of shader IO values is this attribute legal for.
    EnumSet<ShaderIOKind> valid_io_kinds;

    /// Implements the validation logic for a specific attribute.
    using CheckFn = Result<SuccessType, std::string>(const core::type::Type* ty,
                                                     const IOAttributes& attr,
                                                     const Properties& prop,
                                                     IOAttributeUsage usage);

    /// The validation function.
    CheckFn* const check;

    /// Implements logic for checking if the given type is valid or not. Is not a data entry (i.e. a
    /// type or set of types), because types are part of the IR module and created at runtime.
    using TypeCheckFn = bool(const core::type::Type* type, const Properties& props);

    /// @see #TypeCheckFn
    TypeCheckFn* const type_check;

    /// Message for logging if the type check fails. Cannot be easily generated at runtime, because
    /// the type check is a function, not just a data entry.
    const char* type_error;
};

/// @returns all the appropriate IOAttributeCheckers for @p attr
Vector<const IOAttributeChecker*, 4> IOAttributeCheckersFor(const IOAttributes& attr,
                                                            bool skip_builtin);
/// @returns the IOAttributeUsage for a given Function::PipelineStage + IODirection tuple
IOAttributeUsage IOAttributeUsageFor(Function::PipelineStage stage, IODirection direction);

/// @returns the IODirection for an AddressSpace
IODirection IODirectionFor(AddressSpace address_space);

}  // namespace tint::core::ir::validator

#endif  // SRC_TINT_LANG_CORE_IR_IO_ATTRIBUTE_VALIDATOR_H_
