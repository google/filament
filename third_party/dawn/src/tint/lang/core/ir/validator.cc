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

#include "src/tint/lang/core/ir/validator.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <utility>

#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/bitcast.h"
#include "src/tint/lang/core/ir/block_param.h"
#include "src/tint/lang/core/ir/break_if.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/constexpr_if.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/continue.h"
#include "src/tint/lang/core/ir/control_instruction.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/instruction_result.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/member_builtin_call.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/override.h"
#include "src/tint/lang/core/ir/phony.h"
#include "src/tint/lang/core/ir/referenced_functions.h"
#include "src/tint/lang/core/ir/referenced_module_vars.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/store_vector_element.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/terminate_invocation.h"
#include "src/tint/lang/core/ir/type/array_count.h"
#include "src/tint/lang/core/ir/unary.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/unused.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/function.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/u64.h"
#include "src/tint/lang/core/type/u8.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/containers/predicates.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/result.h"
#include "src/tint/utils/rtti/castable.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/text_style.h"

/// If set to 1 then the Tint will dump the IR when validating.
#define TINT_DUMP_IR_WHEN_VALIDATING 0
#if TINT_DUMP_IR_WHEN_VALIDATING
#include <iostream>
#include "src/tint/utils/text/styled_text_printer.h"
#endif

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::core::ir {

struct ValidatedType {
    const core::type::Type* ty;
    Capabilities caps;
};

namespace {

/// @returns the parent block of @p block
const Block* ParentBlockOf(const Block* block) {
    if (auto* parent = block->Parent()) {
        return parent->Block();
    }
    return nullptr;
}

/// @returns true if @p block directly or transitively holds the instruction @p inst
bool TransitivelyHolds(const Block* block, const Instruction* inst) {
    for (auto* b = inst->Block(); b; b = ParentBlockOf(b)) {
        if (b == block) {
            return true;
        }
    }
    return false;
}

/// @return true if @param attr does not have invariant decoration or if it also has position
/// decoration
bool InvariantOnlyIfAlsoPosition(const tint::core::IOAttributes& attr) {
    return !attr.invariant || attr.builtin == BuiltinValue::kPosition;
}

/// @returns true if @p ty meets the basic function parameter rules (i.e. one of constructible,
///          pointer, handle).
///
/// Note: Does not handle corner cases like if certain capabilities are
/// enabled.
bool IsValidFunctionParamType(const core::type::Type* ty) {
    return ty->IsConstructible() || ty->Is<core::type::Pointer>() || ty->IsHandle();
}

/// @returns true if @p ty is a non-struct and decorated with @builtin(position), or if it is a
/// struct and one of its members is decorated, otherwise false.
/// @param attr attributes attached to data
/// @param ty type of the data being tested
bool IsPositionPresent(const IOAttributes& attr, const core::type::Type* ty) {
    if (auto* ty_struct = ty->As<core::type::Struct>()) {
        for (const auto* mem : ty_struct->Members()) {
            if (mem->Attributes().builtin == BuiltinValue::kPosition) {
                return true;
            }
        }
        return false;
    }

    return attr.builtin == BuiltinValue::kPosition;
}

/// Utility for running checks on attributes.
/// If the type that the attributes are attached to is a struct, the check is run over the members,
/// otherwise it run on the attributes directly.
///
/// @param msg_anchor what to associate errors with, e.g. the 'foo' of AddError(foo)
/// @param ty_attr the directly attached attributes
/// @param ty the type of the thing that the attributes are attached to
/// @param is_not_struct_impl has the signature 'void(const MSG_ANCHOR*, const IOAttributes&)' and
///        is called when @p ty is not a struct
/// @param is_struct_impl has the signature 'void(const MSG_ANCHOR*, const IOAttributes&)' and is
///        called when @p ty is a struct
template <typename MSG_ANCHOR, typename IS_NOT_STRUCT, typename IS_STRUCT>
void CheckIOAttributes(const MSG_ANCHOR* msg_anchor,
                       const IOAttributes& ty_attr,
                       const core::type::Type* ty,
                       IS_NOT_STRUCT&& is_not_struct_impl,
                       IS_STRUCT&& is_struct_impl) {
    if (auto* ty_struct = ty->As<core::type::Struct>()) {
        for (const auto* mem : ty_struct->Members()) {
            is_struct_impl(msg_anchor, mem->Attributes());
        }
    } else {
        is_not_struct_impl(msg_anchor, ty_attr);
    }
}

/// Helper for calling CheckIOAttributes on a function return
/// @param func function whose return is to be tested
/// See @ref CheckIOAttributes for more details
template <typename IS_NOT_STRUCT, typename IS_STRUCT>
void CheckFunctionReturnAttributes(const Function* func,
                                   IS_NOT_STRUCT&& is_not_struct_impl,
                                   IS_STRUCT&& is_struct_impl) {
    CheckIOAttributes(func, func->ReturnAttributes(), func->ReturnType(),
                      std::forward<IS_NOT_STRUCT>(is_not_struct_impl),
                      std::forward<IS_STRUCT>(is_struct_impl));
}

/// Helper for calling CheckIOAttributes on a function param
/// @param param function param to be tested
/// See @ref CheckIOAttributes for more details
template <typename IS_NOT_STRUCT, typename IS_STRUCT>
void CheckFunctionParamAttributes(const FunctionParam* param,
                                  IS_NOT_STRUCT&& is_not_struct_impl,
                                  IS_STRUCT&& is_struct_impl) {
    CheckIOAttributes(param, param->Attributes(), param->Type(),
                      std::forward<IS_NOT_STRUCT>(is_not_struct_impl),
                      std::forward<IS_STRUCT>(is_struct_impl));
}

/// Utility for running checks on attributes and type.
/// If the type that the attributes are attached to is a struct, the check is run over the members,
/// otherwise it run on the attributes directly.
///
/// @param msg_anchor what to associate errors with, e.g. the 'foo' of AddError(foo)
/// @param ty_attr the directly attached attributes
/// @param ty the type of the thing that the attributes are attached to
/// @param is_not_struct_impl has the signature 'void(const MSG_ANCHOR*, const IOAttributes&, const
/// core::type::Type* ty)'
///        and is called when @p ty is not a struct
/// @param is_struct_impl has the signature 'void(const MSG_ANCHOR*, const IOAttributes&, const
/// core::type::Type* ty)'
///        and is called when @p ty is a struct
template <typename MSG_ANCHOR, typename IS_NOT_STRUCT, typename IS_STRUCT>
void CheckIOAttributesAndType(const MSG_ANCHOR* msg_anchor,
                              const IOAttributes& ty_attr,
                              const core::type::Type* ty,
                              IS_NOT_STRUCT&& is_not_struct_impl,
                              IS_STRUCT&& is_struct_impl) {
    if (auto* ty_struct = ty->As<core::type::Struct>()) {
        for (const auto* mem : ty_struct->Members()) {
            is_struct_impl(msg_anchor, mem->Attributes(), mem->Type());
        }
    } else {
        is_not_struct_impl(msg_anchor, ty_attr, ty);
    }
}

// Wrapper for CheckIOAttributesAndType, when the struct and non-struct impl are the same
/// See @ref IOAttributesAndType for more details
template <typename MSG_ANCHOR, typename IMPL>
void CheckIOAttributesAndType(const MSG_ANCHOR* msg_anchor,
                              const IOAttributes& ty_attr,
                              const core::type::Type* ty,
                              IMPL&& impl) {
    CheckIOAttributesAndType(msg_anchor, ty_attr, ty, impl, impl);
}

/// Helper for calling IOAttributesAndType on a function param
/// @param param function param to be tested
/// See @ref IOAttributesAndType for more details
template <typename IS_NOT_STRUCT, typename IS_STRUCT>
void CheckFunctionParamAttributesAndType(const FunctionParam* param,
                                         IS_NOT_STRUCT&& is_not_struct_impl,
                                         IS_STRUCT&& is_struct_impl) {
    CheckIOAttributesAndType(param, param->Attributes(), param->Type(),
                             std::forward<IS_NOT_STRUCT>(is_not_struct_impl),
                             std::forward<IS_STRUCT>(is_struct_impl));
}

/// Helper for calling IOAttributesAndType on a function param
/// @param param function param to be tested
/// See @ref IOAttributesAndType for more details
template <typename IMPL>
void CheckFunctionParamAttributesAndType(const FunctionParam* param, IMPL&& impl) {
    CheckIOAttributesAndType(param, param->Attributes(), param->Type(), std::forward<IMPL>(impl));
}

/// A BuiltinChecker is the interface used to check that a usage of a builtin attribute meets the
/// basic spec rules, i.e. correct shader stage, data type, and IO direction.
/// It does not test more sophisticated rules like location and builtins being mutually exclusive or
/// the correct capabilities are enabled.
struct BuiltinChecker {
    /// User friendly name to print in logging messages
    const char* name;

    /// What type of entry point is this builtin legal for
    EnumSet<Function::PipelineStage> stages;

    enum IODirection : uint8_t { kInput, kOutput };
    /// Is this expected to be a param going into the entry point or a result coming out
    IODirection direction;

    /// Implements logic for checking if the given type is valid or not
    using TypeCheckFn = bool(const core::type::Type* type);

    /// @see #TypeCheckFn
    TypeCheckFn* const type_check;

    /// Message that should logged if the type check fails
    const char* type_error;
};

std::string_view ToString(BuiltinChecker::IODirection value) {
    switch (value) {
        case BuiltinChecker::IODirection::kInput:
            return "input";
        case BuiltinChecker::IODirection::kOutput:
            return "output";
    }
    TINT_ICE() << "Unknown enum passed to ToString(BuiltinChecker::IODirection)";
}

constexpr BuiltinChecker kPointSizeChecker{
    /* name */ "__point_size",
    /* stages */ EnumSet<Function::PipelineStage>(Function::PipelineStage::kVertex),
    /* direction */ BuiltinChecker::IODirection::kOutput,
    /* type_check */ [](const core::type::Type* ty) -> bool { return ty->Is<core::type::F32>(); },
    /* type_error */ "__point_size must be a f32",
};

constexpr BuiltinChecker kCullDistanceChecker{
    /* name */ "__cull_distance",
    /* stages */ EnumSet<Function::PipelineStage>(Function::PipelineStage::kVertex),
    /* direction */ BuiltinChecker::IODirection::kOutput,
    /* type_check */
    [](const core::type::Type* ty) -> bool {
        return ty->Is<core::type::Array>() && ty->DeepestElement()->Is<core::type::F32>();
    },
    /* type_error */ "__cull_distance must be an array of f32",
};

constexpr BuiltinChecker kFragDepthChecker{
    /* name */ "frag_depth",
    /* stages */ EnumSet<Function::PipelineStage>(Function::PipelineStage::kFragment),
    /* direction */ BuiltinChecker::IODirection::kOutput,
    /* type_check */ [](const core::type::Type* ty) -> bool { return ty->Is<core::type::F32>(); },
    /* type_error */ "frag_depth must be a f32",
};

constexpr BuiltinChecker kFrontFacingChecker{
    /* name */ "front_facing",
    /* stages */ EnumSet<Function::PipelineStage>(Function::PipelineStage::kFragment),
    /* direction */ BuiltinChecker::IODirection::kInput,
    /* type_check */ [](const core::type::Type* ty) -> bool { return ty->Is<core::type::Bool>(); },
    /* type_error */ "front_facing must be a bool",
};

constexpr BuiltinChecker kGlobalInvocationIdChecker{
    /* name */ "global_invocation_id",
    /* stages */ EnumSet<Function::PipelineStage>(Function::PipelineStage::kCompute),
    /* direction */ BuiltinChecker::IODirection::kInput,
    /* type_check */
    [](const core::type::Type* ty) -> bool {
        return ty->IsUnsignedIntegerVector() && ty->Elements().count == 3;
    },
    /* type_error */ "global_invocation_id must be an vec3<u32>",
};

constexpr BuiltinChecker kInstanceIndexChecker{
    /* name */ "instance_index",
    /* stages */ EnumSet<Function::PipelineStage>(Function::PipelineStage::kVertex),
    /* direction */ BuiltinChecker::IODirection::kInput,
    /* type_check */ [](const core::type::Type* ty) -> bool { return ty->Is<core::type::U32>(); },
    /* type_error */ "instance_index must be an u32",
};

constexpr BuiltinChecker kLocalInvocationIdChecker{
    /* name */ "local_invocation_id",
    /* stages */ EnumSet<Function::PipelineStage>(Function::PipelineStage::kCompute),
    /* direction */ BuiltinChecker::IODirection::kInput,
    /* type_check */
    [](const core::type::Type* ty) -> bool {
        return ty->IsUnsignedIntegerVector() && ty->Elements().count == 3;
    },
    /* type_error */ "local_invocation_id must be an vec3<u32>",
};

constexpr BuiltinChecker kLocalInvocationIndexChecker{
    /* name */ "local_invocation_index",
    /* stages */ EnumSet<Function::PipelineStage>(Function::PipelineStage::kCompute),
    /* direction */ BuiltinChecker::IODirection::kInput,
    /* type_check */ [](const core::type::Type* ty) -> bool { return ty->Is<core::type::U32>(); },
    /* type_error */ "local_invocation_index must be an u32",
};

constexpr BuiltinChecker kNumWorkgroupsChecker{
    /* name */ "num_workgroups",
    /* stages */ EnumSet<Function::PipelineStage>(Function::PipelineStage::kCompute),
    /* direction */ BuiltinChecker::IODirection::kInput,
    /* type_check */
    [](const core::type::Type* ty) -> bool {
        return ty->IsUnsignedIntegerVector() && ty->Elements().count == 3;
    },
    /* type_error */ "num_workgroups must be an vec3<u32>",
};

constexpr BuiltinChecker kSampleIndexChecker{
    /* name */ "sample_index",
    /* stages */ EnumSet<Function::PipelineStage>(Function::PipelineStage::kFragment),
    /* direction */ BuiltinChecker::IODirection::kInput,
    /* type_check */ [](const core::type::Type* ty) -> bool { return ty->Is<core::type::U32>(); },
    /* type_error */ "sample_index must be an u32",
};

constexpr BuiltinChecker kSubgroupIdChecker{
    /* name */ "subgroup_id",
    /* stages */
    EnumSet<Function::PipelineStage>(Function::PipelineStage::kCompute),
    /* direction */ BuiltinChecker::IODirection::kInput,
    /* type_check */ [](const core::type::Type* ty) -> bool { return ty->Is<core::type::U32>(); },
    /* type_error */ "subgroup_id must be an u32",
};

constexpr BuiltinChecker kSubgroupInvocationIdChecker{
    /* name */ "subgroup_invocation_id",
    /* stages */
    EnumSet<Function::PipelineStage>(Function::PipelineStage::kFragment,
                                     Function::PipelineStage::kCompute),
    /* direction */ BuiltinChecker::IODirection::kInput,
    /* type_check */ [](const core::type::Type* ty) -> bool { return ty->Is<core::type::U32>(); },
    /* type_error */ "subgroup_invocation_id must be an u32",
};

constexpr BuiltinChecker kSubgroupSizeChecker{
    /* name */ "subgroup_size",
    /* stages */
    EnumSet<Function::PipelineStage>(Function::PipelineStage::kFragment,
                                     Function::PipelineStage::kCompute),
    /* direction */ BuiltinChecker::IODirection::kInput,
    /* type_check */ [](const core::type::Type* ty) -> bool { return ty->Is<core::type::U32>(); },
    /* type_error */ "subgroup_size must be an u32",
};

constexpr BuiltinChecker kVertexIndexChecker{
    /* name */ "vertex_index",
    /* stages */ EnumSet<Function::PipelineStage>(Function::PipelineStage::kVertex),
    /* direction */ BuiltinChecker::IODirection::kInput,
    /* type_check */ [](const core::type::Type* ty) -> bool { return ty->Is<core::type::U32>(); },
    /* type_error */ "vertex_index must be an u32",
};

constexpr BuiltinChecker kWorkgroupIdChecker{
    /* name */ "workgroup_id",
    /* stages */ EnumSet<Function::PipelineStage>(Function::PipelineStage::kCompute),
    /* direction */ BuiltinChecker::IODirection::kInput,
    /* type_check */
    [](const core::type::Type* ty) -> bool {
        return ty->IsUnsignedIntegerVector() && ty->Elements().count == 3;
    },
    /* type_error */ "workgroup_id must be an vec3<u32>",
};

constexpr BuiltinChecker kPrimitiveIdChecker{
    /* name */ "primitive_id",
    /* stages */ EnumSet<Function::PipelineStage>(Function::PipelineStage::kFragment),
    /* direction */ BuiltinChecker::IODirection::kInput,
    /* type_check */
    [](const core::type::Type* ty) -> bool { return ty->Is<core::type::U32>(); },
    /* type_error */ "primitive_id must be an u32",
};

constexpr BuiltinChecker kBarycentricCoordChecker{
    /* name */ "barycentric_coord",
    /* stages */ EnumSet<Function::PipelineStage>(Function::PipelineStage::kFragment),
    /* direction */ BuiltinChecker::IODirection::kInput,
    /* type_check */
    [](const core::type::Type* ty) -> bool {
        return ty->IsFloatVector() && ty->Elements().count == 3;
    },
    /* type_error */ "barycentric_coord must be an vec3<f32>",
};

/// @returns an appropriate BuiltInCheck for @p builtin, ICEs when one isn't defined
const BuiltinChecker& BuiltinCheckerFor(BuiltinValue builtin) {
    switch (builtin) {
        case BuiltinValue::kPointSize:
            return kPointSizeChecker;
        case BuiltinValue::kCullDistance:
            return kCullDistanceChecker;
        case BuiltinValue::kFragDepth:
            return kFragDepthChecker;
        case BuiltinValue::kFrontFacing:
            return kFrontFacingChecker;
        case BuiltinValue::kGlobalInvocationId:
            return kGlobalInvocationIdChecker;
        case BuiltinValue::kInstanceIndex:
            return kInstanceIndexChecker;
        case BuiltinValue::kLocalInvocationId:
            return kLocalInvocationIdChecker;
        case BuiltinValue::kLocalInvocationIndex:
            return kLocalInvocationIndexChecker;
        case BuiltinValue::kNumWorkgroups:
            return kNumWorkgroupsChecker;
        case BuiltinValue::kSampleIndex:
            return kSampleIndexChecker;
        case BuiltinValue::kSubgroupId:
            return kSubgroupIdChecker;
        case BuiltinValue::kSubgroupInvocationId:
            return kSubgroupInvocationIdChecker;
        case BuiltinValue::kSubgroupSize:
            return kSubgroupSizeChecker;
        case BuiltinValue::kVertexIndex:
            return kVertexIndexChecker;
        case BuiltinValue::kWorkgroupId:
            return kWorkgroupIdChecker;
        case BuiltinValue::kPrimitiveId:
            return kPrimitiveIdChecker;
        case BuiltinValue::kBarycentricCoord:
            return kBarycentricCoordChecker;
        case BuiltinValue::kPosition:
            TINT_ICE() << "BuiltinValue::kPosition requires special handling, so does not have a "
                          "checker defined";
        case BuiltinValue::kSampleMask:
            TINT_ICE() << "BuiltinValue::kSampleMask requires special handling, so does not have a "
                          "checker defined";
        default:
            TINT_ICE() << builtin << " is does not have a checker defined for it";
    }
}

/// Validates the basic spec rules for @builtin(position) usage
/// @param stage the shader stage the builtin is being used
/// @param is_input the IO direction of usage, true if input, false if output
/// @param ty the data type being decorated by the builtin
/// @returns Success if a valid usage, or reason for invalidity in Failure
Result<SuccessType, std::string> ValidatePositionBuiltIn(Function::PipelineStage stage,
                                                         bool is_input,
                                                         const core::type::Type* ty) {
    if (stage != Function::PipelineStage::kVertex && stage != Function::PipelineStage::kFragment) {
        return std::string("position must be used in a fragment or vertex shader entry point");
    }

    if (stage == Function::PipelineStage::kVertex && is_input) {
        return std::string("position must be an output for a vertex entry point");
    }

    if (stage == Function::PipelineStage::kFragment && !is_input) {
        return std::string("position must be an input for a fragment entry point");
    }

    if (!ty->IsFloatVector() || ty->Elements().count != 4 ||
        !ty->Element(0)->Is<core::type::F32>()) {
        return std::string("position must be an vec4<f32>");
    }

    return Success;
}

/// Validates the basic spec rules for @builtin(sample_mask) usage
/// @param stage the shader stage the builtin is being used
/// @param ty the data type being decorated by the builtin
/// @returns Success if a valid usage, or reason for invalidity in Failure
Result<SuccessType, std::string> ValidateSampleMaskBuiltIn(Function::PipelineStage stage,
                                                           const core::type::Type* ty) {
    if (stage != Function::PipelineStage::kFragment) {
        return std::string("sample_mask must be used in a fragment entry point");
    }

    if (!ty->Is<core::type::U32>()) {
        return std::string("sample_mask must be an u32");
    }

    return Success;
}

/// Validates the basic spec rules for @builtin(clip_distance) usage
/// @param stage the shader stage the builtin is being used
/// @param is_input the IO direction of usage, true if input, false if output
/// @param capabilities the optional capabilities that are allowed
/// @param ty the data type being decorated by the builtin
/// @returns Success if a valid usage, or reason for invalidity in Failure
Result<SuccessType, std::string> ValidateBuiltinClipDistances(Function::PipelineStage stage,
                                                              bool is_input,
                                                              const Capabilities& capabilities,
                                                              const core::type::Type* ty) {
    if (stage != Function::PipelineStage::kVertex) {
        return std::string("clip_distances must be used in a vertex shader entry point");
    }

    if (is_input) {
        return std::string("clip_distances must be an output of a shader entry point");
    }

    auto is_valid_array = [&] {
        const auto elems = ty->Elements();
        return elems.type && elems.type->Is<core::type::F32>() && elems.count <= 8;
    };

    if (capabilities.Contains(Capability::kAllowClipDistancesOnF32)) {
        if (!ty->Is<core::type::F32>() && !is_valid_array()) {
            return std::string("clip_distances must be an f32 or an array<f32, N>, where N <= 8");
        }
    } else if (!is_valid_array()) {
        return std::string("clip_distances must be an array<f32, N>, where N <= 8");
    }

    return Success;
}

/// Validates the basic spec rules for builtin usage
/// @param builtin the builtin to test
/// @param stage the shader stage the builtin is being used
/// @param is_input the IO direction of usage, true if input, false if output
/// @param ty the data type being decorated by the builtin
/// @returns Success if a valid usage, or reason for invalidity in Failure
Result<SuccessType, std::string> ValidateBuiltIn(BuiltinValue builtin,
                                                 Function::PipelineStage stage,
                                                 bool is_input,
                                                 const Capabilities& capabilities,
                                                 const core::type::Type* ty) {
    // This is not an entry point function, either it is dead code and thus never called, or any
    // issues will be detected when validating the calling entry point.
    if (stage == Function::PipelineStage::kUndefined) {
        return Success;
    }

    // Some builtins have multiple contexts that they are valid in, so have special handling
    // instead of making the checker/lookup table more complex.
    switch (builtin) {
        case BuiltinValue::kPosition:
            return ValidatePositionBuiltIn(stage, is_input, ty);
        case BuiltinValue::kSampleMask:
            return ValidateSampleMaskBuiltIn(stage, ty);
        case BuiltinValue::kClipDistances:
            return ValidateBuiltinClipDistances(stage, is_input, capabilities, ty);
        default:
            break;
    }

    const auto& checker = BuiltinCheckerFor(builtin);
    std::stringstream msg;
    if (!checker.stages.Contains(stage)) {
        auto stages_size = checker.stages.Size();
        switch (stages_size) {
            case 1:
                msg << checker.name << " must be used in a " << ToString(*checker.stages.begin())
                    << " shader entry point";
                break;
            case 2:
                msg << checker.name << " must be used in a " << ToString(*checker.stages.begin())
                    << " or " << ToString(*(++checker.stages.begin())) << " shader entry point";
                break;
            default:
                TINT_ICE() << "Unexpected number of stages set, " << stages_size;
        }
        return msg.str();
    }

    auto io_direction =
        is_input ? BuiltinChecker::IODirection::kInput : BuiltinChecker::IODirection::kOutput;
    if (io_direction != checker.direction) {
        msg << checker.name << " must be an " << ToString(checker.direction)
            << " of a shader entry point";
        return msg.str();
    }

    if (!checker.type_check(ty)) {
        return std::string(checker.type_error);
    }

    return Success;
}

// Annotations that can be associated with a value that are used for shader IO, e.g. binding_points,
// @location, being in workgroup address space, etc.
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

/// @returns text describing the annotation for error logging
std::string ToString(IOAnnotation value) {
    switch (value) {
        case IOAnnotation::kBindingPoint:
            return "@group + @binding";
        case IOAnnotation::kLocation:
            return "@location";
        case IOAnnotation::kBuiltin:
            return "built-in";
        case IOAnnotation::kWorkgroup:
            return "<workgroup>";
        case IOAnnotation::kColor:
            return "@color";
    }
    TINT_ICE() << "Unknown enum passed to ToString(IOAnnotation)";
}

/// @returns a human-readable string of all the entries in a set of IOAnnotations
std::string ToString(const EnumSet<IOAnnotation>& values) {
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

/// Adds appropriate entries to annotations, based on what values are present in attributes
/// @param annotations the set to updated
/// @param attr the attributes to be examined
/// @returns Success if none of the values being added where already present, otherwise returns the
/// first non-unique value as a Failure
Result<SuccessType, IOAnnotation> AddIOAnnotationsFromIOAttributes(
    EnumSet<IOAnnotation>& annotations,
    const IOAttributes& attr) {
    if (attr.location.has_value()) {
        if (annotations.Contains(IOAnnotation::kLocation)) {
            return IOAnnotation::kLocation;
        }
        annotations.Add(IOAnnotation::kLocation);
    }

    if (attr.builtin.has_value()) {
        if (annotations.Contains(IOAnnotation::kBuiltin)) {
            return IOAnnotation::kBuiltin;
        }
        annotations.Add(IOAnnotation::kBuiltin);
    }

    if (attr.color.has_value()) {
        if (annotations.Contains(IOAnnotation::kColor)) {
            return IOAnnotation::kColor;
        }
        annotations.Add(IOAnnotation::kColor);
    }

    return Success;
}

/// The core IR validator.
class Validator {
  public:
    /// Create a core validator
    /// @param mod the module to be validated
    /// @param capabilities the optional capabilities that are allowed
    explicit Validator(const Module& mod, Capabilities capabilities);

    /// Destructor
    ~Validator();

    /// Runs the validator over the module provided during construction
    /// @returns success or failure
    Result<SuccessType> Run();

  private:
    /// Runs validation to confirm the structural soundness of the module.
    /// Also runs any validation that is not dependent on the entire module being
    /// sound and sets up data structures for later checks.
    void RunStructuralSoundnessChecks();

    /// Checks that there is no direct or indirect recursion.
    /// Depends on CheckStructuralSoundness() having previously been run.
    void CheckForRecursion();

    /// Checks that there are no orphaned instructions
    /// Depends on CheckStructuralSoundness() having previously been run
    void CheckForOrphanedInstructions();

    /// Checks that there are no discards called by non-fragment entrypoints
    /// Depends on CheckStructuralSoundness() having previously been run
    void CheckForNonFragmentDiscards();

    /// @returns the IR disassembly, performing a disassemble if this is the first call.
    ir::Disassembler& Disassemble();

    /// Adds an error for the @p inst and highlights the instruction in the disassembly
    /// @param inst the instruction
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const Instruction* inst);

    /// Adds an error for the @p inst operand at @p idx and highlights the operand in the
    /// disassembly
    /// @param inst the instruction
    /// @param idx the operand index
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const Instruction* inst, size_t idx);

    /// Adds an error for the @p inst result at @p idx and highlgihts the result in the disassembly
    /// @param inst the instruction
    /// @param idx the result index
    /// @returns the diagnostic
    diag::Diagnostic& AddResultError(const Instruction* inst, size_t idx);

    /// Adds an error for the @p block and highlights the block header in the disassembly
    /// @param blk the block
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const Block* blk);

    /// Adds an error for the @p param and highlights the parameter in the disassembly
    /// @param param the parameter
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const BlockParam* param);

    /// Adds an error for the @p func and highlights the function in the disassembly
    /// @param func the function
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const Function* func);

    /// Adds an error for the @p param and highlights the parameter in the disassembly
    /// @param param the parameter
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const FunctionParam* param);

    /// Adds an error the @p block and highlights the block header in the disassembly
    /// @param src the source lines to highlight
    /// @returns the diagnostic
    diag::Diagnostic& AddError(Source src);

    /// Adds a note to @p inst and highlights the instruction in the disassembly
    /// @param inst the instruction
    diag::Diagnostic& AddNote(const Instruction* inst);

    /// Adds a note to @p func and highlights the function in the disassembly
    /// @param func the function
    diag::Diagnostic& AddNote(const Function* func);

    /// Adds a note to @p inst for operand @p idx and highlights the operand in the disassembly
    /// @param inst the instruction
    /// @param idx the operand index
    diag::Diagnostic& AddOperandNote(const Instruction* inst, size_t idx);

    /// Adds a note to @p inst for result @p idx and highlights the result in the disassembly
    /// @param inst the instruction
    /// @param idx the result index
    diag::Diagnostic& AddResultNote(const Instruction* inst, size_t idx);

    /// Adds a note to @p blk and highlights the block in the disassembly
    /// @param blk the block
    diag::Diagnostic& AddNote(const Block* blk);

    /// Adds a note to the diagnostics
    /// @param src the source lines to highlight
    diag::Diagnostic& AddNote(Source src = {});

    /// Adds a note to the diagnostics highlighting where the value instruction or block is
    /// declared, if it has a source location.
    /// @param decl the value instruction or block
    void AddDeclarationNote(const CastableBase* decl);

    /// Adds a note to the diagnostics highlighting where the block is declared, if it has a source
    /// location.
    /// @param block the block
    void AddDeclarationNote(const Block* block);

    /// Adds a note to the diagnostics highlighting where the block parameter is declared, if it
    /// has a source location.
    /// @param param the block parameter
    void AddDeclarationNote(const BlockParam* param);

    /// Adds a note to the diagnostics highlighting where the function is declared, if it has a
    /// source location.
    /// @param fn the function
    void AddDeclarationNote(const Function* fn);

    /// Adds a note to the diagnostics highlighting where the function parameter is declared, if it
    /// has a source location.
    /// @param param the function parameter
    void AddDeclarationNote(const FunctionParam* param);

    /// Adds a note to the diagnostics highlighting where the instruction is declared, if it has a
    /// source location.
    /// @param inst the inst
    void AddDeclarationNote(const Instruction* inst);

    /// Adds a note to the diagnostics highlighting where instruction result was declared, if it has
    /// a source location.
    /// @param res the res
    void AddDeclarationNote(const InstructionResult* res);

    /// @param decl the type, value, instruction or block to get the name for
    /// @returns the styled name for the given value, instruction or block
    StyledText NameOf(const CastableBase* decl);

    // @param ty the type to get the name for
    /// @returns the styled name for the given type
    StyledText NameOf(const core::type::Type* ty);

    /// @param v the value to get the name for
    /// @returns the styled name for the given value
    StyledText NameOf(const Value* v);

    /// @param inst the instruction to get the name for
    /// @returns the styled  name for the given instruction
    StyledText NameOf(const Instruction* inst);

    /// @param block the block to get the name for
    /// @returns the styled  name for the given block
    StyledText NameOf(const Block* block);

    /// Checks the given result is not null and its type is not null
    /// @param inst the instruction
    /// @param idx the result index
    /// @returns true if the result is not null
    bool CheckResult(const Instruction* inst, size_t idx);

    /// Checks the results (and their types) for @p inst are not null. If count is specified then
    /// number of results is checked to be exact.
    /// @param inst the instruction
    /// @param count the number of results to check
    /// @returns true if the results count is as expected and none are null
    bool CheckResults(const ir::Instruction* inst, std::optional<size_t> count);

    /// Checks the given operand is not null and its type is not null
    /// @param inst the instruction
    /// @param idx the operand index
    /// @returns true if the operand is not null
    bool CheckOperand(const Instruction* inst, size_t idx);

    /// Checks the number of operands provided to @p inst and that none of them are null. Also
    /// checks that the types for the operands are not null
    /// @param inst the instruction
    /// @param min_count the minimum number of operands to expect
    /// @param max_count the maximum number of operands to expect, if not set, than only the minimum
    /// number is checked.
    /// @returns true if the number of operands is in the expected range and none are null
    bool CheckOperands(const ir::Instruction* inst,
                       size_t min_count,
                       std::optional<size_t> max_count);

    /// Checks the operands (and their types) for @p inst are not null. If count is specified then
    /// number of operands is checked to be exact.
    /// @param inst the instruction
    /// @param count the number of operands to check
    /// @returns true if the operands count is as expected and none are null
    bool CheckOperands(const ir::Instruction* inst, std::optional<size_t> count);

    /// Checks the number of results for @p inst are exactly equal to @p num_results and the number
    /// of operands is correctly. Both results and operands are confirmed to be non-null.
    /// @param inst the instruction
    /// @param num_results expected number of results for the instruction
    /// @param min_operands the minimum number of operands to expect
    /// @param max_operands the maximum number of operands to expect, if not set, than only the
    /// minimum number is checked.
    /// @returns true if the result and operand counts are as expected and none are null
    bool CheckResultsAndOperandRange(const ir::Instruction* inst,
                                     size_t num_results,
                                     size_t min_operands,
                                     std::optional<size_t> max_operands);

    /// Checks the number of results and operands for @p inst are exactly equal to num_results
    /// and num_operands, respectively, and that none of them are null.
    /// @param inst the instruction
    /// @param num_results expected number of results for the instruction
    /// @param num_operands expected number of operands for the instruction
    /// @returns true if the result and operand counts are as expected and none are null
    bool CheckResultsAndOperands(const ir::Instruction* inst,
                                 size_t num_results,
                                 size_t num_operands);

    /// Checks that @p type is allowed by the spec, and does not use any types that are prohibited
    /// by the target capabilities.
    /// @param type the type
    /// @param diag a function that creates an error diagnostic for the source of the type
    /// @param ignore_caps a set of capabilities to ignore for this check
    void CheckType(const core::type::Type* type,
                   std::function<diag::Diagnostic&()> diag,
                   Capabilities ignore_caps = {});

    /// Validates the root block
    /// @param blk the block
    void CheckRootBlock(const Block* blk);

    /// Validates the given instruction is only used in the root block.
    /// @param inst the instruction
    void CheckOnlyUsedInRootBlock(const Instruction* inst);

    /// Validates the given function
    /// @param func the function to validate
    void CheckFunction(const Function* func);

    /// Validates the workgroup_size attribute for a given function
    /// @param func the function to validate
    void CheckWorkgroupSize(const Function* func);

    /// Validates the specific function as a vertex entry point
    /// @param ep the function to validate
    void CheckVertexEntryPoint(const Function* ep);

    /// Helper for calling IOAttributesAndType on a function return
    /// @param func function's return to be tested
    /// See @ref IOAttributesAndType for more details
    template <typename IS_NOT_STRUCT, typename IS_STRUCT>
    void CheckFunctionReturnAttributesAndType(const Function* func,
                                              IS_NOT_STRUCT&& is_not_struct_impl,
                                              IS_STRUCT&& is_struct_impl) {
        CheckIOAttributesAndType(func, func->ReturnAttributes(), func->ReturnType(),
                                 std::forward<IS_NOT_STRUCT>(is_not_struct_impl),
                                 std::forward<IS_STRUCT>(is_struct_impl));
    }

    /// Helper for calling IOAttributesAndType on a function return
    /// @param func function's return to be tested
    /// See @ref IOAttributesAndType for more details
    template <typename IMPL>
    void CheckFunctionReturnAttributesAndType(const Function* func, IMPL&& impl) {
        if (func->ReturnType()->Is<core::type::Struct>() &&
            func->ReturnAttributes().builtin.has_value()) {
            const char* name;
            switch (func->ReturnAttributes().builtin.value()) {
                case BuiltinValue::kPosition:
                    name = "position";
                    break;
                case BuiltinValue::kSampleMask:
                    name = "sample_mask";
                    break;
                case BuiltinValue::kClipDistances:
                    name = "clip_distances";
                    break;
                default:
                    name = BuiltinCheckerFor(func->ReturnAttributes().builtin.value()).name;
                    break;
            }
            AddError(func) << name << " cannot be attached to a structure";
        }

        CheckIOAttributesAndType(func, func->ReturnAttributes(), func->ReturnType(),
                                 std::forward<IMPL>(impl));
    }

    /// @returns a function that validates rules for invariant decorations
    /// @param err error message to log when check fails
    template <typename MSG_ANCHOR>
    auto CheckInvariantFunc(const std::string& err) {
        return [this, err](const MSG_ANCHOR* msg_anchor, const IOAttributes& attr) {
            if (!InvariantOnlyIfAlsoPosition(attr)) {
                AddError(msg_anchor) << err;
            }
        };
    }

    /// @returns a function that validates builtins on function params
    auto CheckBuiltinFunctionParam(const std::string& err) {
        return [this, err](const FunctionParam* param, const IOAttributes& attr,
                           const core::type::Type* ty) {
            if (!attr.builtin.has_value()) {
                return;
            }
            auto result = ValidateBuiltIn(attr.builtin.value(), param->Function()->Stage(), true,
                                          capabilities_, ty);
            if (result != Success) {
                AddError(param) << err << result.Failure();
            }
        };
    }

    /// @returns a function that validates builtins on function returns
    auto CheckBuiltinFunctionReturn(const std::string& err) {
        return [this, err](const Function* func, const IOAttributes& attr,
                           const core::type::Type* ty) {
            if (!attr.builtin.has_value()) {
                return;
            }
            auto result =
                ValidateBuiltIn(attr.builtin.value(), func->Stage(), false, capabilities_, ty);
            if (result != Success) {
                AddError(func) << err << result.Failure();
            }
        };
    }

    /// @returns a function that validates that type is bool iff decorated with
    /// @builtin(front_facing)
    /// @param err error message to log when check fails
    template <typename MSG_ANCHOR>
    auto CheckFrontFacingIfBoolFunc(const std::string& err) {
        return [this, err](const MSG_ANCHOR* msg_anchor, const IOAttributes& attr,
                           const core::type::Type* ty) {
            if (ty->Is<core::type::Bool>() && attr.builtin != BuiltinValue::kFrontFacing) {
                AddError(msg_anchor) << err;
            }
        };
    }

    /// @returns a function that validates that type is not bool
    /// @param err error message to log when check fails
    template <typename MSG_ANCHOR>
    auto CheckNotBool(const std::string& err) {
        return [this, err](const MSG_ANCHOR* msg_anchor, [[maybe_unused]] const IOAttributes& attr,
                           const core::type::Type* ty) {
            if (ty->Is<core::type::Bool>()) {
                AddError(msg_anchor) << err;
            }
        };
    }

    /// Validates the given instruction
    /// @param inst the instruction to validate
    void CheckInstruction(const Instruction* inst);

    /// Validates the given override
    /// @param o the override to validate
    void CheckOverride(const Override* o);

    /// Validates the given var
    /// @param var the var to validate
    void CheckVar(const Var* var);

    /// Validates binding_point usage for pointers
    /// @param binding_point the binding information associated with pointer
    /// @param address_space the address space of pointer
    /// @param target_str string to insert in error message describing what has a binding_point,
    /// defaults to 'variable'
    /// @returns Success if a valid usage, or reason for invalidity in Failure
    Result<SuccessType, std::string> ValidateBindingPoint(
        const std::optional<struct BindingPoint>& binding_point,
        AddressSpace address_space,
        const std::string& target_str = "variable");

    /// Validates shader IO annotations for entry point input/output
    /// Note: Call is required to ensure that the value being validated is associated with an entry
    ///       point function
    /// @param ty type of the value under test
    /// @param binding_point the binding information associated with the value
    /// @param attr IO attributes associated with the values
    /// @param target_str string to insert in error message describing what has a binding_point,
    /// something like 'input param' or 'return value'
    /// @returns Success if one, and only one, shader IO is present, otherwise a Failure with the
    /// error reason is returned
    Result<SuccessType, std::string> ValidateShaderIOAnnotations(
        const core::type::Type* ty,
        const std::optional<struct BindingPoint>& binding_point,
        const core::IOAttributes& attr,
        const std::string& target_str);

    /// Validates the given let
    /// @param l the let to validate
    void CheckLet(const Let* l);

    /// Validates the given call
    /// @param call the call to validate
    void CheckCall(const Call* call);

    /// Validates the given bitcast
    /// @param bitcast the bitcast to validate
    void CheckBitcast(const Bitcast* bitcast);

    /// Validates that there exists the required bitcast override
    /// @param bitcast the bitcast to validate types of
    void CheckBitcastTypes(const Bitcast* bitcast);

    /// Validates the given builtin call
    /// @param call the call to validate
    void CheckBuiltinCall(const BuiltinCall* call);

    /// Validates the given member builtin call
    /// @param call the member call to validate
    void CheckMemberBuiltinCall(const MemberBuiltinCall* call);

    /// Validates the given construct
    /// @param construct the construct to validate
    void CheckConstruct(const Construct* construct);

    /// Validates the given convert
    /// @param convert the convert to validate
    void CheckConvert(const Convert* convert);

    /// Validates the given discard
    /// @note Does not validate that the discard is in a fragment shader, that
    /// needs to be handled later in the validation.
    /// @param discard the discard to validate
    void CheckDiscard(const Discard* discard);

    /// Validates the given user call
    /// @param call the call to validate
    void CheckUserCall(const UserCall* call);

    /// Validates the given access
    /// @param a the access to validate
    void CheckAccess(const Access* a);

    /// Validates the given binary
    /// @param b the binary to validate
    void CheckBinary(const Binary* b);

    /// Validates the given unary
    /// @param u the unary to validate
    void CheckUnary(const Unary* u);

    /// Validates the given if
    /// @param if_ the if to validate
    void CheckIf(const If* if_);

    /// Validates the given loop
    /// @param l the loop to validate
    void CheckLoop(const Loop* l);

    /// Validates the loop body block
    /// @param l the loop to validate
    void CheckLoopBody(const Loop* l);

    /// Validates the loop continuing block
    /// @param l the loop to validate
    void CheckLoopContinuing(const Loop* l);

    /// Validates the given switch
    /// @param s the switch to validate
    void CheckSwitch(const Switch* s);

    /// Validates the given swizzle
    /// @param s the swizzle to validate
    void CheckSwizzle(const Swizzle* s);

    /// Validates the given terminator
    /// @param b the terminator to validate
    void CheckTerminator(const Terminator* b);

    /// Validates the break if instruction
    /// @param b the break if to validate
    void CheckBreakIf(const BreakIf* b);

    /// Validates the continue instruction
    /// @param c the continue to validate
    void CheckContinue(const Continue* c);

    /// Validates the given exit
    /// @param e the exit to validate
    void CheckExit(const Exit* e);

    /// Validates the next iteration instruction
    /// @param n the next iteration to validate
    void CheckNextIteration(const NextIteration* n);

    /// Validates the given exit if
    /// @param e the exit if to validate
    void CheckExitIf(const ExitIf* e);

    /// Validates the given return
    /// @param r the return to validate
    void CheckReturn(const Return* r);

    /// Validates the given unreachable
    /// @param u the unreachable to validate
    void CheckUnreachable(const Unreachable* u);

    /// Validates the @p exit targets a valid @p control instruction where the instruction may jump
    /// over if control instructions.
    /// @param exit the exit to validate
    /// @param control the control instruction targeted
    void CheckControlsAllowingIf(const Exit* exit, const Instruction* control);

    /// Validates the given exit switch
    /// @param s the exit switch to validate
    void CheckExitSwitch(const ExitSwitch* s);

    /// Validates the given exit loop
    /// @param l the exit loop to validate
    void CheckExitLoop(const ExitLoop* l);

    /// Validates the given load
    /// @param l the load to validate
    void CheckLoad(const Load* l);

    /// Validates the given store
    /// @param s the store to validate
    void CheckStore(const Store* s);

    /// Validates the given load vector element
    /// @param l the load vector element to validate
    void CheckLoadVectorElement(const LoadVectorElement* l);

    /// Validates the given store vector element
    /// @param s the store vector element to validate
    void CheckStoreVectorElement(const StoreVectorElement* s);

    /// Validates the given phony assignment
    /// @param p the phony assignment to validate
    void CheckPhony(const Phony* p);

    /// Validates that the number and types of the source instruction operands match the target's
    /// values.
    /// @param source_inst the source instruction
    /// @param source_operand_offset the index of the first operand of the source instruction
    /// @param source_operand_count the number of operands of the source instruction
    /// @param target the receiver of the operand values
    /// @param target_values the receiver of the operand values
    void CheckOperandsMatchTarget(const Instruction* source_inst,
                                  size_t source_operand_offset,
                                  size_t source_operand_count,
                                  const CastableBase* target,
                                  VectorRef<const Value*> target_values);

    /// @param inst the instruction
    /// @param idx the operand index
    /// @returns the vector pointer type for the given instruction operand
    const core::type::Type* GetVectorPtrElementType(const Instruction* inst, size_t idx);

    /// Executes all the pending tasks
    void ProcessTasks();

    /// Queues the block to be validated with ProcessTasks()
    /// @param blk the block to validate
    void QueueBlock(const Block* blk);

    /// Queues the list of instructions starting with @p inst to be validated
    /// @param inst the first instruction
    void QueueInstructions(const Instruction* inst);

    /// Begins validation of the block @p blk, and its instructions.
    /// BeginBlock() pushes a new scope for values.
    /// Must be paired with a call to EndBlock().
    void BeginBlock(const Block* blk);

    /// Ends validation of the block opened with BeginBlock() and closes the block's scope for
    /// values.
    void EndBlock();

    /// Get the function that contains an instruction.
    /// @param inst the instruction
    /// @returns the function
    const ir::Function* ContainingFunction(const ir::Instruction* inst) {
        if (inst->Block() == mod_.root_block) {
            return nullptr;
        }

        return block_to_function_.GetOrAdd(inst->Block(), [&] {  //
            return ContainingFunction(inst->Block()->Parent());
        });
    }

    /// Get any endpoints that call a function.
    /// @param f the function
    /// @returns all end points that call the function
    Hashset<const ir::Function*, 4> ContainingEndPoints(const ir::Function* f) {
        if (!f) {
            return {};
        }

        Hashset<const ir::Function*, 4> result{};
        Hashset<const ir::Function*, 4> visited{f};

        auto call_sites = user_func_calls_.GetOr(f, Hashset<const ir::UserCall*, 4>()).Vector();
        while (!call_sites.IsEmpty()) {
            auto call_site = call_sites.Pop();
            auto calling_function = ContainingFunction(call_site);
            if (!calling_function) {
                continue;
            }

            if (visited.Contains(calling_function)) {
                continue;
            }
            visited.Add(calling_function);

            if (calling_function->IsEntryPoint()) {
                result.Add(calling_function);
            }

            for (auto new_call_sites :
                 user_func_calls_.GetOr(f, Hashset<const ir::UserCall*, 4>())) {
                call_sites.Push(new_call_sites);
            }
        }

        return result;
    }

    /// ScopeStack holds a stack of values that are currently in scope
    struct ScopeStack {
        void Push() { stack_.Push({}); }
        void Pop() { stack_.Pop(); }
        void Add(const Value* value) { stack_.Back().Add(value); }
        bool Contains(const Value* value) {
            return stack_.Any([&](auto& v) { return v.Contains(value); });
        }
        bool IsEmpty() const { return stack_.IsEmpty(); }

      private:
        Vector<Hashset<const Value*, 8>, 4> stack_;
    };

    const Module& mod_;
    Capabilities capabilities_;
    std::optional<ir::Disassembler> disassembler_;  // Use Disassemble()
    diag::List diagnostics_;
    Hashset<const Function*, 4> all_functions_;
    Hashset<const Instruction*, 4> visited_instructions_;
    Hashmap<const Loop*, const Continue*, 4> first_continues_;
    Vector<const ControlInstruction*, 8> control_stack_;
    Vector<const Block*, 8> block_stack_;
    ScopeStack scope_stack_;
    Vector<std::function<void()>, 16> tasks_;
    SymbolTable symbols_ = SymbolTable::Wrap(mod_.symbols);
    core::type::Manager type_mgr_ = core::type::Manager::Wrap(mod_.Types());
    Hashmap<const ir::Block*, const ir::Function*, 64> block_to_function_{};
    Hashmap<const ir::Function*, Hashset<const ir::UserCall*, 4>, 4> user_func_calls_;
    Hashset<const ir::Discard*, 4> discards_;
    core::ir::ReferencedModuleVars<const Module> referenced_module_vars_;
    Hashset<OverrideId, 8> seen_override_ids_;
    Hashset<std::string, 4> entry_point_names_;
    Hashset<ValidatedType, 16> validated_types_{};
};

Validator::Validator(const Module& mod, Capabilities capabilities)
    : mod_(mod), capabilities_(capabilities), referenced_module_vars_(mod) {}

Validator::~Validator() = default;

Disassembler& Validator::Disassemble() {
    if (!disassembler_) {
        disassembler_.emplace(ir::Disassembler(mod_));
    }
    return *disassembler_;
}

Result<SuccessType> Validator::Run() {
    RunStructuralSoundnessChecks();

    CheckForRecursion();
    CheckForOrphanedInstructions();
    CheckForNonFragmentDiscards();

    if (diagnostics_.ContainsErrors()) {
        diagnostics_.AddNote(Source{}) << "# Disassembly\n" << Disassemble().Text();
        return Failure{diagnostics_.Str()};
    }
    return Success;
}

void Validator::CheckForRecursion() {
    if (diagnostics_.ContainsErrors()) {
        return;
    }

    ReferencedFunctions<const Module> referenced_functions(mod_);
    for (auto& func : mod_.functions) {
        auto& refs = referenced_functions.TransitiveReferences(func);
        if (refs.Contains(func)) {
            // TODO(434684891): Consider improving this error with more information.
            AddError(func) << "recursive function calls are not allowed";
            return;
        }
    }
}

void Validator::CheckForOrphanedInstructions() {
    if (diagnostics_.ContainsErrors()) {
        return;
    }

    // Check for orphaned instructions.
    for (auto* inst : mod_.Instructions()) {
        if (!visited_instructions_.Contains(inst)) {
            AddError(inst) << "orphaned instruction: " << inst->FriendlyName();
        }
    }
}

void Validator::CheckForNonFragmentDiscards() {
    if (diagnostics_.ContainsErrors()) {
        return;
    }

    // Check for discards in non-fragments
    for (const auto& d : discards_) {
        const auto* f = ContainingFunction(d);
        if (f->IsEntryPoint() && !f->IsFragment()) {
            AddError(d) << "cannot be called in non-fragment entry point";
        } else {
            for (const Function* ep : ContainingEndPoints(f)) {
                if (!ep->IsFragment()) {
                    AddError(d) << "cannot be called in non-fragment entry point";
                }
            }
        }
    }
}

void Validator::RunStructuralSoundnessChecks() {
    scope_stack_.Push();
    TINT_DEFER({
        scope_stack_.Pop();
        TINT_ASSERT(scope_stack_.IsEmpty());
        TINT_ASSERT(tasks_.IsEmpty());
        TINT_ASSERT(control_stack_.IsEmpty());
        TINT_ASSERT(block_stack_.IsEmpty());
    });
    CheckRootBlock(mod_.root_block);

    for (auto& func : mod_.functions) {
        if (!all_functions_.Add(func.Get())) {
            AddError(func) << "function " << NameOf(func.Get())
                           << " added to module multiple times";
        }
        scope_stack_.Add(func);
    }

    for (auto& func : mod_.functions) {
        block_to_function_.Add(func->Block(), func);
        CheckFunction(func);
    }
}

diag::Diagnostic& Validator::AddError(const Instruction* inst) {
    diagnostics_.ReserveAdditional(2);  // Ensure diagnostics don't resize alive after AddNote()
    auto src = Disassemble().InstructionSource(inst);
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";
    }
    return diag;
}

diag::Diagnostic& Validator::AddError(const Instruction* inst, size_t idx) {
    diagnostics_.ReserveAdditional(2);  // Ensure diagnostics don't resize alive after AddNote()
    auto src =
        Disassemble().OperandSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";
    }
    return diag;
}

diag::Diagnostic& Validator::AddResultError(const Instruction* inst, size_t idx) {
    diagnostics_.ReserveAdditional(2);  // Ensure diagnostics don't resize alive after AddNote()
    auto src =
        Disassemble().ResultSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";
    }
    return diag;
}

diag::Diagnostic& Validator::AddError(const Block* blk) {
    auto src = Disassemble().BlockSource(blk);
    return AddError(src);
}

diag::Diagnostic& Validator::AddError(const BlockParam* param) {
    auto src = Disassemble().BlockParamSource(param);
    return AddError(src);
}

diag::Diagnostic& Validator::AddError(const Function* func) {
    auto src = Disassemble().FunctionSource(func);
    return AddError(src);
}

diag::Diagnostic& Validator::AddError(const FunctionParam* param) {
    auto src = Disassemble().FunctionParamSource(param);
    return AddError(src);
}

diag::Diagnostic& Validator::AddNote(const Instruction* inst) {
    auto src = Disassemble().InstructionSource(inst);
    return AddNote(src);
}

diag::Diagnostic& Validator::AddNote(const Function* func) {
    auto src = Disassemble().FunctionSource(func);
    return AddNote(src);
}

diag::Diagnostic& Validator::AddOperandNote(const Instruction* inst, size_t idx) {
    auto src =
        Disassemble().OperandSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    return AddNote(src);
}

diag::Diagnostic& Validator::AddResultNote(const Instruction* inst, size_t idx) {
    auto src =
        Disassemble().ResultSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    return AddNote(src);
}

diag::Diagnostic& Validator::AddNote(const Block* blk) {
    auto src = Disassemble().BlockSource(blk);
    return AddNote(src);
}

diag::Diagnostic& Validator::AddError(Source src) {
    auto& diag = diagnostics_.AddError(src);
    diag.owned_file = Disassemble().File();
    return diag;
}

diag::Diagnostic& Validator::AddNote(Source src) {
    auto& diag = diagnostics_.AddNote(src);
    diag.owned_file = Disassemble().File();
    return diag;
}

void Validator::AddDeclarationNote(const CastableBase* decl) {
    tint::Switch(
        decl,  //
        [&](const Block* block) { AddDeclarationNote(block); },
        [&](const BlockParam* param) { AddDeclarationNote(param); },
        [&](const Function* fn) { AddDeclarationNote(fn); },
        [&](const FunctionParam* param) { AddDeclarationNote(param); },
        [&](const Instruction* inst) { AddDeclarationNote(inst); },
        [&](const InstructionResult* res) { AddDeclarationNote(res); });
}

void Validator::AddDeclarationNote(const Block* block) {
    auto src = Disassemble().BlockSource(block);
    if (src.file) {
        AddNote(src) << NameOf(block) << " declared here";
    }
}

void Validator::AddDeclarationNote(const BlockParam* param) {
    auto src = Disassemble().BlockParamSource(param);
    if (src.file) {
        AddNote(src) << NameOf(param) << " declared here";
    }
}

void Validator::AddDeclarationNote(const Function* fn) {
    AddNote(fn) << NameOf(fn) << " declared here";
}

void Validator::AddDeclarationNote(const FunctionParam* param) {
    auto src = Disassemble().FunctionParamSource(param);
    if (src.file) {
        AddNote(src) << NameOf(param) << " declared here";
    }
}

void Validator::AddDeclarationNote(const Instruction* inst) {
    auto src = Disassemble().InstructionSource(inst);
    if (src.file) {
        AddNote(src) << NameOf(inst) << " declared here";
    }
}

void Validator::AddDeclarationNote(const InstructionResult* res) {
    if (auto* inst = res->Instruction()) {
        auto results = inst->Results();
        for (size_t i = 0; i < results.Length(); i++) {
            if (results[i] == res) {
                AddResultNote(res->Instruction(), i) << NameOf(res) << " declared here";
                return;
            }
        }
    }
}

StyledText Validator::NameOf(const CastableBase* decl) {
    return tint::Switch(
        decl,  //
        [&](const core::type::Type* ty) { return NameOf(ty); },
        [&](const Value* value) { return NameOf(value); },
        [&](const Instruction* inst) { return NameOf(inst); },
        [&](const Block* block) { return NameOf(block); },  //
        TINT_ICE_ON_NO_MATCH);
}

StyledText Validator::NameOf(const core::type::Type* ty) {
    auto name = ty ? ty->FriendlyName() : "undef";
    return StyledText{} << style::Type(name);
}

StyledText Validator::NameOf(const Value* value) {
    return Disassemble().NameOf(value);
}

StyledText Validator::NameOf(const Instruction* inst) {
    auto name = inst ? inst->FriendlyName() : "undef";
    return StyledText{} << style::Instruction(name);
}

StyledText Validator::NameOf(const Block* block) {
    auto parent_name = block->Parent() ? block->Parent()->FriendlyName() : "undef";
    return StyledText{} << style::Instruction(parent_name) << " block "
                        << Disassemble().NameOf(block);
}

bool Validator::CheckResult(const Instruction* inst, size_t idx) {
    auto* result = inst->Result(idx);
    if (DAWN_UNLIKELY(result == nullptr)) {
        AddResultError(inst, idx) << "result is undefined";
        return false;
    }

    if (DAWN_UNLIKELY(result->Type() == nullptr)) {
        AddResultError(inst, idx) << "result type is undefined";
        return false;
    }

    if (DAWN_UNLIKELY(result->Instruction() == nullptr)) {
        AddResultError(inst, idx) << "result instruction is undefined";
        return false;
    }

    if (DAWN_UNLIKELY(result->Instruction() != inst)) {
        AddResultError(inst, idx)
            << "result instruction does not match instruction (possible double usage)";
        return false;
    }

    return true;
}

bool Validator::CheckResults(const ir::Instruction* inst, std::optional<size_t> count = {}) {
    if (count.has_value()) {
        if (DAWN_UNLIKELY(inst->Results().Length() != count.value())) {
            AddError(inst) << "expected exactly " << count.value() << " results, got "
                           << inst->Results().Length();
            return false;
        }
    }

    bool passed = true;
    for (size_t i = 0; i < inst->Results().Length(); i++) {
        if (DAWN_UNLIKELY(!CheckResult(inst, i))) {
            passed = false;
        }
    }
    return passed;
}

bool Validator::CheckOperand(const Instruction* inst, size_t idx) {
    auto* operand = inst->Operand(idx);

    if (DAWN_UNLIKELY(operand == nullptr)) {
        // var instructions are allowed to have a nullptr initializers.
        // terminator instructions use nullptr operands to signal 'undef'.
        if (inst->IsAnyOf<Terminator, Var>()) {
            return true;
        }

        AddError(inst, idx) << "operand is undefined";
        return false;
    }

    // ir::Unused is a internal value used by some transforms to track unused entries, and is
    // removed as part of generating an output shader.
    if (DAWN_UNLIKELY(operand->Is<ir::Unused>())) {
        return true;
    }

    if (DAWN_UNLIKELY(operand->Type() == nullptr)) {
        AddError(inst, idx) << "operand type is undefined";
        return false;
    }

    if (DAWN_UNLIKELY(!operand->Alive())) {
        AddError(inst, idx) << "operand is not alive";
        return false;
    }

    if (DAWN_UNLIKELY(!operand->HasUsage(inst, idx))) {
        AddError(inst, idx) << "operand missing usage";
        return false;
    }

    if (auto fn = operand->As<Function>(); fn && !all_functions_.Contains(fn)) {
        AddError(inst, idx) << NameOf(operand) << " is not part of the module";
        return false;
    }

    if (DAWN_UNLIKELY(!operand->Is<ir::Unused>() && !operand->Is<Constant>() &&
                      !scope_stack_.Contains(operand))) {
        AddError(inst, idx) << NameOf(operand) << " is not in scope";
        AddDeclarationNote(operand);
        return false;
    }

    return true;
}

bool Validator::CheckOperands(const ir::Instruction* inst,
                              size_t min_count,
                              std::optional<size_t> max_count) {
    if (DAWN_UNLIKELY(inst->Operands().Length() < min_count)) {
        if (max_count.has_value()) {
            AddError(inst) << "expected between " << min_count << " and " << max_count.value()
                           << " operands, got " << inst->Operands().Length();
        } else {
            AddError(inst) << "expected at least " << min_count << " operands, got "
                           << inst->Operands().Length();
        }
        return false;
    }

    if (DAWN_UNLIKELY(max_count.has_value() && inst->Operands().Length() > max_count.value())) {
        AddError(inst) << "expected between " << min_count << " and " << max_count.value()
                       << " operands, got " << inst->Operands().Length();
        return false;
    }

    bool passed = true;
    for (size_t i = 0; i < inst->Operands().Length(); i++) {
        if (DAWN_UNLIKELY(!CheckOperand(inst, i))) {
            passed = false;
        }
    }
    return passed;
}

bool Validator::CheckOperands(const ir::Instruction* inst, std::optional<size_t> count = {}) {
    if (count.has_value()) {
        if (DAWN_UNLIKELY(inst->Operands().Length() != count.value())) {
            AddError(inst) << "expected exactly " << count.value() << " operands, got "
                           << inst->Operands().Length();
            return false;
        }
    }

    bool passed = true;
    for (size_t i = 0; i < inst->Operands().Length(); i++) {
        if (DAWN_UNLIKELY(!CheckOperand(inst, i))) {
            passed = false;
        }
    }
    return passed;
}

bool Validator::CheckResultsAndOperandRange(const ir::Instruction* inst,
                                            size_t num_results,
                                            size_t min_operands,
                                            std::optional<size_t> max_operands = {}) {
    // Intentionally avoiding short-circuiting here
    bool results_passed = CheckResults(inst, num_results);
    bool operands_passed = CheckOperands(inst, min_operands, max_operands);
    return results_passed && operands_passed;
}

bool Validator::CheckResultsAndOperands(const ir::Instruction* inst,
                                        size_t num_results,
                                        size_t num_operands) {
    // Intentionally avoiding short-circuiting here
    bool results_passed = CheckResults(inst, num_results);
    bool operands_passed = CheckOperands(inst, num_operands);
    return results_passed && operands_passed;
}

void Validator::CheckType(const core::type::Type* root,
                          std::function<diag::Diagnostic&()> diag,
                          Capabilities ignore_caps) {
    if (root == nullptr) {
        return;
    }

    if (!capabilities_.Contains(Capability::kAllowNonCoreTypes)) {
        // Check for core types, which are the only types declared in the `tint::core` namespace.
        if (!std::string_view(root->TypeInfo().name).starts_with("tint::core")) {
            diag() << "non-core types not allowed in core IR";
            return;
        }
    }

    if (!validated_types_.Add(ValidatedType{root, ignore_caps})) {
        return;
    }

    auto visit = [&](const core::type::Type* type) {
        if (type->IsAbstract()) {
            diag() << "abstracts are not permitted";
            return false;
        }

        return tint::Switch(
            type,
            [&](const core::type::Struct* str) {
                if (capabilities_.Contains(Capability::kAllowStructMatrixDecorations)) {
                    return true;
                }

                for (auto* member : str->Members()) {
                    if (member->RowMajor()) {
                        diag() << "Row major annotation not allowed on structures";
                        return false;
                    }
                    if (member->HasMatrixStride()) {
                        diag() << "Matrix stride annotation not allowed on structures";
                        return false;
                    }
                    if (member->Size() < member->Type()->Size()) {
                        diag() << "struct member " << member->Index()
                               << " with size=" << member->Size()
                               << " must be at least as large as the type with size "
                               << member->Type()->Size();
                        return false;
                    }
                }
                return true;
            },
            [&](const core::type::Reference* ref) {
                if (ref->StoreType()->Is<core::type::Void>()) {
                    diag() << "references to void are not permitted";
                    return false;
                }

                // Reference types are guarded by the AllowRefTypes capability.
                if (!capabilities_.Contains(Capability::kAllowRefTypes) ||
                    ignore_caps.Contains(Capability::kAllowRefTypes)) {
                    diag() << "reference types are not permitted here";
                    return false;
                } else if (type != root) {
                    // If they are allowed, reference types still cannot be nested.
                    diag() << "nested reference types are not permitted";
                    return false;
                }
                return true;
            },
            [&](const core::type::Pointer* ptr) {
                if (ptr->StoreType()->Is<core::type::Void>()) {
                    diag() << "pointers to void are not permitted";
                    return false;
                }

                if (ptr->AddressSpace() == AddressSpace::kUniform ||
                    ptr->AddressSpace() == AddressSpace::kHandle) {
                    if (ptr->Access() != core::Access::kRead) {
                        diag() << "uniform and handle pointers must be read access";
                        return false;
                    }
                }

                if (ptr->AddressSpace() == AddressSpace::kHandle) {
                    if (!ptr->StoreType()->IsHandle()) {
                        diag() << "the 'handle' address space can only be used for handle types";
                        return false;
                    }
                } else {
                    if (ptr->StoreType()->IsHandle()) {
                        diag() << "handle types can only be declared in the 'handle' address space";
                        return false;
                    }
                }

                if (type != root) {
                    // Nesting pointer types inside structures is guarded by a capability.
                    if (!(capabilities_.Contains(
                            Capability::kAllowPointersAndHandlesInStructures))) {
                        diag() << "nested pointer types are not permitted";
                        return false;
                    }
                }
                return true;
            },
            [&](const core::type::U64*) {
                // u64 types are guarded by the Allow64BitIntegers capability.
                if (!capabilities_.Contains(Capability::kAllow64BitIntegers)) {
                    diag() << "64-bit integer types are not permitted";
                    return false;
                }
                return true;
            },
            [&](const core::type::I8*) {
                // i8 types are guarded by the Allow8BitIntegers capability.
                if (!capabilities_.Contains(Capability::kAllow8BitIntegers)) {
                    diag() << "8-bit integer types are not permitted";
                    return false;
                }
                return true;
            },
            [&](const core::type::U8*) {
                // u8 types are guarded by the Allow8BitIntegers capability.
                if (!capabilities_.Contains(Capability::kAllow8BitIntegers)) {
                    diag() << "8-bit integer types are not permitted";
                    return false;
                }
                return true;
            },
            [&](const core::type::Array* arr) {
                if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
                    auto* mv = root->As<core::type::MemoryView>();
                    if (mv && mv->AddressSpace() != AddressSpace::kStorage) {
                        diag() << "runtime arrays must be in the 'storage' address space";
                        return false;
                    }
                }
                return true;
            },
            [&](const core::type::Vector* v) {
                if (!v->Type()->IsScalar()) {
                    diag() << "vector elements, " << NameOf(type) << ", must be scalars";
                    return false;
                }
                return true;
            },
            [&](const core::type::Matrix* m) {
                if (!m->Type()->IsFloatScalar()) {
                    diag() << "matrix elements, " << NameOf(type) << ", must be float scalars";
                    return false;
                }
                return true;
            },
            [&](const core::type::SampledTexture* s) {
                if (!s->Type()->IsAnyOf<core::type::F32, core::type::I32, core::type::U32>()) {
                    diag() << "invalid sampled texture sample type: " << NameOf(s->Type());
                    return false;
                }
                return true;
            },
            [&](const core::type::MultisampledTexture* ms) {
                if (!ms->Type()->IsAnyOf<core::type::F32, core::type::I32, core::type::U32>()) {
                    diag() << "invalid multisampled texture sample type: " << NameOf(ms->Type());
                    return false;
                }

                switch (ms->Dim()) {
                    case core::type::TextureDimension::k2d:
                        break;
                    default:
                        diag() << "invalid multisampled texture dimension: "
                               << style::Literal(ToString(ms->Dim()));
                        return false;
                }
                return true;
            },
            [&](const core::type::StorageTexture* s) {
                switch (s->Dim()) {
                    case core::type::TextureDimension::kCube:
                    case core::type::TextureDimension::kCubeArray:
                        diag() << "dimension " << style::Literal(ToString(s->Dim()))
                               << " for storage textures does not in WGSL yet";
                        return false;
                    case core::type::TextureDimension::kNone:
                        diag() << "invalid texture dimension "
                               << style::Literal(ToString(s->Dim()));
                        return false;
                    default:
                        return true;
                }
            },
            [&](const core::type::InputAttachment* i) {
                if (!i->Type()->IsAnyOf<core::type::F32, core::type::I32, core::type::U32>()) {
                    diag() << "invalid input attachment component type: " << NameOf(i->Type());
                    return false;
                }
                return true;
            },
            [&](const core::type::SubgroupMatrix* m) {
                if (!m->Type()
                         ->IsAnyOf<core::type::F16, core::type::F32, core::type::I8,
                                   core::type::I32, core::type::U8, core::type::U32>()) {
                    diag() << "invalid subgroup matrix component type: " << NameOf(m->Type());
                    return false;
                }
                return true;
            },
            [](Default) { return true; });
    };

    Vector<const core::type::Type*, 8> stack{root};
    Hashset<const core::type::Type*, 8> seen{};
    while (!stack.IsEmpty()) {
        auto* ty = stack.Pop();
        if (!ty) {
            continue;
        }
        if (!visit(ty)) {
            return;
        }

        if (auto* view = ty->As<core::type::MemoryView>(); view && seen.Add(view)) {
            stack.Push(view->StoreType());
            continue;
        }

        auto type_count = ty->Elements();
        if (type_count.type && seen.Add(type_count.type)) {
            stack.Push(type_count.type);
            continue;
        }

        for (uint32_t i = 0; i < type_count.count; i++) {
            if (auto* subtype = ty->Element(i); subtype && seen.Add(subtype)) {
                stack.Push(subtype);
            }
        }
    }
}

void Validator::CheckRootBlock(const Block* blk) {
    block_stack_.Push(blk);
    TINT_DEFER(block_stack_.Pop());

    for (auto* inst : *blk) {
        if (inst->Block() != blk) {
            AddError(inst) << "instruction in root block does not have root block as parent";
            continue;
        }

        tint::Switch(
            inst,  //
            [&](const core::ir::Override* o) {
                if (capabilities_.Contains(Capability::kAllowOverrides)) {
                    CheckInstruction(o);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            },
            [&](const core::ir::Var* var) { CheckInstruction(var); },
            [&](const core::ir::Let* let) {
                if (capabilities_.Contains(Capability::kAllowModuleScopeLets)) {
                    CheckInstruction(let);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            },
            [&](const core::ir::Construct* c) {
                if (capabilities_.Contains(Capability::kAllowModuleScopeLets) ||
                    capabilities_.Contains(Capability::kAllowOverrides)) {
                    CheckInstruction(c);
                    CheckOnlyUsedInRootBlock(inst);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            },
            [&](Default) {
                // Note, this validation around kAllowOverrides is looser than it could be. There
                // are only certain expressions and builtins which can be used in an override, but
                // the current checks are only doing type level checking. To tighten this up will
                // require walking up the tree to make sure that operands are const/override and
                // builtins are allowed.
                if (capabilities_.Contains(Capability::kAllowOverrides) &&
                    inst->IsAnyOf<core::ir::Unary, core::ir::Binary, core::ir::BuiltinCall,
                                  core::ir::Convert, core::ir::Swizzle, core::ir::Access,
                                  core::ir::Bitcast, core::ir::ConstExprIf>()) {
                    CheckInstruction(inst);
                    // If overrides are allowed we can have certain regular instructions in the root
                    // block, with the caveat that those instructions can _only_ be used in the root
                    // block.
                    CheckOnlyUsedInRootBlock(inst);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            });
    }
    // Our ConstExprIfs in the root require us to process tasks here.
    ProcessTasks();
}

void Validator::CheckOnlyUsedInRootBlock(const Instruction* inst) {
    if (inst->Result(0)) {
        for (auto& usage : inst->Result(0)->UsagesSorted()) {
            if (usage.instruction->Block() != mod_.root_block) {
                AddError(inst) << "root block: instruction used outside of root block "
                               << inst->TypeInfo().name;
            }
        }
    }

    CheckInstruction(inst);
}

void Validator::CheckFunction(const Function* func) {
    // Scope holds the parameters and block
    scope_stack_.Push();
    TINT_DEFER(scope_stack_.Pop());

    if (func->IsEntryPoint()) {
        // Check that there is at most one entry point unless we allow multiple entry points.
        if (!capabilities_.Contains(Capability::kAllowMultipleEntryPoints)) {
            if (!entry_point_names_.IsEmpty()) {
                AddError(func) << "a module with multiple entry points requires the "
                                  "AllowMultipleEntryPoints capability";
                return;
            }
        }

        // Checking the name early, so its usage can be recorded, even if the function is malformed.
        const auto name = mod_.NameOf(func).Name();
        if (!entry_point_names_.Add(name)) {
            AddError(func) << "entry point name " << style::Function(name) << " is not unique";
        }
    }

    if (!func->Type() || !func->Type()->Is<core::type::Function>()) {
        AddError(func) << "functions must have type '<function>'";
        return;
    }

    if (!func->Block()) {
        AddError(func) << "root block for function is undefined";
        return;
    }

    if (func->Block()->Is<ir::MultiInBlock>()) {
        AddError(func) << "root block for function cannot be a multi-in block";
        return;
    }

    Hashset<const FunctionParam*, 4> param_set{};
    for (auto* param : func->Params()) {
        if (!param->Alive()) {
            AddError(param) << "destroyed parameter found in function parameter list";
            return;
        }
        if (!param->Function()) {
            AddError(param) << "function parameter has nullptr parent function";
            return;
        } else if (param->Function() != func) {
            AddError(param) << "function parameter has incorrect parent function";
            AddNote(param->Function()) << "parent function declared here";
            return;
        }

        if (!param->Type()) {
            AddError(param) << "function parameter has nullptr type";
            return;
        }

        if (!param_set.Add(param)) {
            AddError(param) << "function parameter is not unique";
            continue;
        }

        // References not allowed on function signatures even with Capability::kAllowRefTypes.
        CheckType(
            param->Type(), [&]() -> diag::Diagnostic& { return AddError(param); },
            Capabilities{Capability::kAllowRefTypes});

        if (!IsValidFunctionParamType(param->Type())) {
            auto struct_ty = param->Type()->As<core::type::Struct>();
            if (!capabilities_.Contains(Capability::kAllowPointersAndHandlesInStructures) ||
                (struct_ty == nullptr) ||
                struct_ty->Members().Any([](const core::type::StructMember* m) {
                    return !IsValidFunctionParamType(m->Type());
                })) {
                AddError(param) << "function parameter type, " << NameOf(param->Type())
                                << ", must be constructible, a pointer, or a handle";
            }
        }

        CheckFunctionParamAttributesAndType(param, CheckBuiltinFunctionParam(""));

        CheckFunctionParamAttributes(
            param,
            CheckInvariantFunc<FunctionParam>(
                "invariant can only decorate a param iff it is also decorated with position"),
            CheckInvariantFunc<FunctionParam>(
                "invariant can only decorate a param member iff it is also "
                "decorated with position"));

        if (func->IsFragment()) {
            CheckFunctionParamAttributesAndType(
                param,
                CheckFrontFacingIfBoolFunc<FunctionParam>(
                    "fragment entry point params can only be a bool if decorated with "
                    "@builtin(front_facing)"),
                CheckFrontFacingIfBoolFunc<FunctionParam>(
                    "fragment entry point param members can only be a bool if "
                    "decorated with @builtin(front_facing)"));
        } else if (func->IsEntryPoint()) {
            CheckFunctionParamAttributesAndType(
                param, CheckNotBool<FunctionParam>(
                           "entry point params can only be a bool for fragment shaders"));
        }

        AddressSpace address_space = AddressSpace::kUndefined;
        auto* mv = param->Type()->As<core::type::MemoryView>();
        if (mv) {
            address_space = mv->AddressSpace();
        } else {
            // ModuleScopeVars transform in MSL backends unwraps pointers to handles
            if (param->Type()->IsHandle()) {
                address_space = AddressSpace::kHandle;
            }
        }

        if (func->IsEntryPoint()) {
            {
                auto result = ValidateShaderIOAnnotations(param->Type(), param->BindingPoint(),
                                                          param->Attributes(), "input param");
                if (result != Success) {
                    AddError(param) << result.Failure();
                }
            }
            {
                auto result =
                    ValidateBindingPoint(param->BindingPoint(), address_space, "input param");
                if (result != Success) {
                    AddError(param) << result.Failure();
                }
            }
        } else {
            if (param->BindingPoint().has_value()) {
                AddError(param)
                    << "input param to non-entry point function has a binding point set";
            }
            if (param->Builtin().has_value()) {
                AddError(param) << "builtins can only be decorated on entry point params";
            }
        }

        if (!capabilities_.Contains(Capability::kAllowWorkspacePointerInputToEntryPoint) &&
            func->IsEntryPoint()) {
            if (mv && mv->Is<core::type::Pointer>() && address_space == AddressSpace::kWorkgroup) {
                AddError(param) << "input param to entry point cannot be a ptr in the 'workgroup' "
                                   "address space";
            }
        }

        scope_stack_.Add(param);
    }

    // References not allowed on function signatures even with Capability::kAllowRefTypes.
    CheckType(
        func->ReturnType(), [&]() -> diag::Diagnostic& { return AddError(func); },
        Capabilities{Capability::kAllowRefTypes});

    CheckFunctionReturnAttributesAndType(func, CheckBuiltinFunctionReturn(""));

    CheckFunctionReturnAttributes(
        func,
        CheckInvariantFunc<Function>(
            "invariant can only decorate outputs iff they are also position builtins"),
        CheckInvariantFunc<Function>(
            "invariant can only decorate output members iff they are also position builtins"));
    // void needs to be filtered out, since it isn't constructible, but used in the IR when no
    // return is specified.
    if (DAWN_UNLIKELY(!func->ReturnType()->Is<core::type::Void>() &&
                      !func->ReturnType()->IsConstructible())) {
        AddError(func) << "function return type must be constructible";
    }

    if (func->IsEntryPoint()) {
        if (DAWN_UNLIKELY(mod_.NameOf(func).Name().empty())) {
            AddError(func) << "entry points must have names";
        }
    }

    CheckWorkgroupSize(func);

    if (func->Stage() == Function::PipelineStage::kCompute) {
        if (DAWN_UNLIKELY(func->ReturnType() && !func->ReturnType()->Is<core::type::Void>())) {
            AddError(func) << "compute entry point must not have a return type, found "
                           << NameOf(func->ReturnType());
        }
    }

    if (func->IsEntryPoint()) {
        auto result = ValidateShaderIOAnnotations(func->ReturnType(), std::nullopt,
                                                  func->ReturnAttributes(), "return values");
        if (result != Success) {
            AddError(func) << result.Failure();
        }

        CheckFunctionReturnAttributesAndType(
            func, CheckFrontFacingIfBoolFunc<Function>("entry point returns can not be 'bool'"),
            CheckFrontFacingIfBoolFunc<Function>("entry point return members can not be 'bool'"));

        Hashset<BindingPoint, 4> binding_points{};

        for (auto var : referenced_module_vars_.TransitiveReferences(func)) {
            if (!capabilities_.Contains(Capability::kAllowDuplicateBindings) &&
                var->BindingPoint().has_value()) {
                auto bp = var->BindingPoint().value();
                if (!binding_points.Add(bp)) {
                    AddError(var) << "found non-unique binding point, " << bp
                                  << ", being referenced in entry point, " << NameOf(func);
                }
            }

            const auto* mv = var->Result()->Type()->As<core::type::MemoryView>();
            const auto* ty = var->Result()->Type()->UnwrapPtrOrRef();
            const auto attr = var->Attributes();
            if (!mv || !ty) {
                continue;
            }

            if (mv->AddressSpace() != AddressSpace::kIn &&
                mv->AddressSpace() != AddressSpace::kOut) {
                continue;
            }

            if (func->IsFragment() && mv->AddressSpace() == AddressSpace::kIn) {
                CheckIOAttributesAndType(
                    func, attr, ty,
                    CheckFrontFacingIfBoolFunc<Function>(
                        "input address space values referenced by fragment shaders can only be "
                        "'bool' if decorated with @builtin(front_facing)"));

            } else {
                CheckIOAttributesAndType(
                    func, attr, ty,
                    CheckNotBool<Function>(
                        "IO address space values referenced by shader entry points can only be "
                        "'bool' if in the input space, used only by fragment shaders and decorated "
                        "with @builtin(front_facing)"));
            }
        }
    }

    if (func->IsVertex()) {
        CheckVertexEntryPoint(func);
    }

    QueueBlock(func->Block());
    ProcessTasks();
}

void Validator::CheckWorkgroupSize(const Function* func) {
    if (!func->IsCompute()) {
        if (func->WorkgroupSize().has_value()) {
            AddError(func) << "@workgroup_size only valid on compute entry point";
        }
        return;
    }

    if (!func->WorkgroupSize().has_value()) {
        AddError(func) << "compute entry point requires @workgroup_size";
        return;
    }

    auto workgroup_sizes = func->WorkgroupSize().value();
    // The number parameters cannot be checked here, since it is stored internally as a 3 element
    // array, so will always have 3 elements at this point.
    TINT_ASSERT(workgroup_sizes.size() == 3);

    std::optional<const core::type::Type*> sizes_ty;
    for (auto* size : workgroup_sizes) {
        if (!size || !size->Type()) {
            AddError(func) << "a @workgroup_size param is undefined or missing a type";
            return;
        }

        auto* ty = size->Type();
        if (!ty->IsAnyOf<core::type::I32, core::type::U32>()) {
            AddError(func) << "@workgroup_size params must be an 'i32' or 'u32', received "
                           << NameOf(ty);
            return;
        }

        if (!sizes_ty.has_value()) {
            sizes_ty = ty;
        }

        if (sizes_ty != ty) {
            AddError(func) << "@workgroup_size params must be all 'i32's or all 'u32's";
            return;
        }

        if (auto* c = size->As<ir::Constant>()) {
            if (c->Value()->ValueAs<int64_t>() <= 0) {
                AddError(func) << "@workgroup_size params must be greater than 0";
                return;
            }
            continue;
        }

        if (!capabilities_.Contains(Capability::kAllowOverrides)) {
            AddError(func) << "@workgroup_size param is not a constant value, and IR capability "
                              "'kAllowOverrides' is not set";
            return;
        }

        if (auto* r = size->As<ir::InstructionResult>()) {
            if (!r->Instruction()) {
                AddError(func) << "instruction for @workgroup_size param is not defined";
                return;
            }

            if (r->Instruction()->Block() != mod_.root_block) {
                AddError(func) << "@workgroup_size param defined by non-module scope value";
                return;
            }

            if (r->Instruction()->Is<core::ir::Override>()) {
                continue;
            }

            // TODO(376624999): Finish implementing checking that this is a override/constant
            //  expression, i.e. calculated from only appropriate values/operations, once override
            //  implementation is complete
            // for each value/operation used to calculate param:
            //        if  not constant expression && not override expression:
            //            fail
            //    pass
        }
    }
}

void Validator::CheckVertexEntryPoint(const Function* ep) {
    bool contains_position = IsPositionPresent(ep->ReturnAttributes(), ep->ReturnType());

    for (auto var : referenced_module_vars_.TransitiveReferences(ep)) {
        const auto* ty = var->Result()->Type()->UnwrapPtrOrRef();
        const auto attr = var->Attributes();
        if (!ty) {
            continue;
        }

        if (!contains_position) {
            contains_position = IsPositionPresent(attr, ty);
        }

        CheckIOAttributes(
            ep, attr, ty,
            CheckInvariantFunc<Function>(
                "invariant can only decorate vars iff they are also position builtins"),
            CheckInvariantFunc<Function>(
                "invariant can only decorate members iff they are also position builtins"));

        // Builtin rules are not checked on module-scope variables, because they are often generated
        // as part of the backend transforms, and have different rules for correctness.
    }

    if (DAWN_UNLIKELY(!contains_position)) {
        AddError(ep) << "position must be declared for vertex entry point output";
    }
}

void Validator::ProcessTasks() {
    while (!tasks_.IsEmpty()) {
        tasks_.Pop()();
    }
}

void Validator::QueueBlock(const Block* blk) {
    tasks_.Push([this] { EndBlock(); });
    tasks_.Push([this, blk] { BeginBlock(blk); });
}

void Validator::BeginBlock(const Block* blk) {
    scope_stack_.Push();
    block_stack_.Push(blk);

    if (auto* mb = blk->As<MultiInBlock>()) {
        for (auto* param : mb->Params()) {
            if (!param->Alive()) {
                AddError(param) << "destroyed parameter found in block parameter list";
                return;
            }
            if (!param->Block()) {
                AddError(param) << "block parameter has nullptr parent block";
                return;
            } else if (param->Block() != mb) {
                AddError(param) << "block parameter has incorrect parent block";
                AddNote(param->Block()) << "parent block declared here";
                return;
            }

            // References not allowed on block parameters even with Capability::kAllowRefTypes.
            CheckType(
                param->Type(), [&]() -> diag::Diagnostic& { return AddError(param); },
                Capabilities{Capability::kAllowRefTypes});

            scope_stack_.Add(param);
        }
    }

    if (!blk->Terminator()) {
        AddError(blk) << "block does not end in a terminator instruction";
    }

    // Validate the instructions w.r.t. the parent block
    for (auto* inst : *blk) {
        if (inst->Block() != blk) {
            AddError(inst) << "block instruction does not have same block as parent";
            AddNote(blk) << "in block";
        }
    }

    // Enqueue validation of the instructions of the block
    if (!blk->IsEmpty()) {
        QueueInstructions(blk->Instructions());
    }
}

void Validator::EndBlock() {
    scope_stack_.Pop();
    block_stack_.Pop();
}

void Validator::QueueInstructions(const Instruction* inst) {
    tasks_.Push([this, inst] {
        CheckInstruction(inst);
        if (inst->next) {
            QueueInstructions(inst->next);
        }
    });
}

void Validator::CheckInstruction(const Instruction* inst) {
    visited_instructions_.Add(inst);
    if (!inst->Alive()) {
        AddError(inst) << "destroyed instruction found in instruction list";
        return;
    }

    auto results = inst->Results();
    for (size_t i = 0; i < results.Length(); ++i) {
        auto* res = results[i];
        if (!res) {
            continue;
        }
        CheckType(res->Type(), [&]() -> diag::Diagnostic& { return AddResultError(inst, i); });
    }

    auto ops = inst->Operands();
    for (size_t i = 0; i < ops.Length(); ++i) {
        auto* op = ops[i];
        if (!op) {
            continue;
        }

        CheckType(op->Type(), [&]() -> diag::Diagnostic& { return AddError(inst, i); });
    }

    tint::Switch(
        inst,                                                              //
        [&](const Access* a) { CheckAccess(a); },                          //
        [&](const Binary* b) { CheckBinary(b); },                          //
        [&](const Call* c) { CheckCall(c); },                              //
        [&](const If* if_) { CheckIf(if_); },                              //
        [&](const Let* let) { CheckLet(let); },                            //
        [&](const Load* load) { CheckLoad(load); },                        //
        [&](const LoadVectorElement* l) { CheckLoadVectorElement(l); },    //
        [&](const Loop* l) { CheckLoop(l); },                              //
        [&](const Phony* p) { CheckPhony(p); },                            //
        [&](const Store* s) { CheckStore(s); },                            //
        [&](const StoreVectorElement* s) { CheckStoreVectorElement(s); },  //
        [&](const Switch* s) { CheckSwitch(s); },                          //
        [&](const Swizzle* s) { CheckSwizzle(s); },                        //
        [&](const Terminator* b) { CheckTerminator(b); },                  //
        [&](const Unary* u) { CheckUnary(u); },                            //
        [&](const Override* o) { CheckOverride(o); },                      //
        [&](const Var* var) { CheckVar(var); },                            //
        [&](const Default) { AddError(inst) << "missing validation"; });

    for (auto* result : results) {
        scope_stack_.Add(result);
    }
}

void Validator::CheckOverride(const Override* o) {
    // Intentionally not checking operands, since Override may have a null operand
    if (!CheckResults(o, Override::kNumResults)) {
        return;
    }

    if (o->OverrideId().has_value()) {
        if (!seen_override_ids_.Add(o->OverrideId().value())) {
            AddError(o) << "duplicate override id encountered: " << o->OverrideId().value().value;
            return;
        }
    }

    if (!o->Result()->Type()->IsScalar()) {
        AddError(o) << "override type " << NameOf(o->Result()->Type()) << " is not a scalar";
        return;
    }

    if (o->Initializer()) {
        if (!CheckOperand(o, ir::Var::kInitializerOperandOffset)) {
            return;
        }
        if (o->Initializer()->Type() != o->Result()->Type()) {
            AddError(o) << "override type " << NameOf(o->Result()->Type())
                        << " does not match initializer type " << NameOf(o->Initializer()->Type());
            return;
        }
    }

    if (!o->OverrideId().has_value() && (o->Initializer() == nullptr)) {
        AddError(o) << "must have an id or an initializer";
        return;
    }
}

void Validator::CheckVar(const Var* var) {
    if (!CheckResultsAndOperands(var, Var::kNumResults, Var::kNumOperands)) {
        return;
    }

    auto* result_type = var->Result()->Type();
    auto* mv = result_type->As<core::type::MemoryView>();
    if (!mv) {
        AddError(var) << "result type " << NameOf(result_type)
                      << " must be a pointer or a reference";
        return;
    }

    if (var->Block() != mod_.root_block && mv->AddressSpace() != AddressSpace::kFunction) {
        if (!capabilities_.Contains(Capability::kAllowPrivateVarsInFunctions) ||
            mv->AddressSpace() != AddressSpace::kPrivate) {
            AddError(var) << "vars in a function scope must be in the 'function' address space";
            return;
        }
    }

    // Check that initializer and result type match
    if (var->Initializer()) {
        if (mv->AddressSpace() != AddressSpace::kFunction &&
            mv->AddressSpace() != AddressSpace::kPrivate &&
            mv->AddressSpace() != AddressSpace::kOut) {
            AddError(var) << "only variables in the function, private, or __out address space may "
                             "be initialized";
            return;
        }

        if (!CheckOperand(var, ir::Var::kInitializerOperandOffset)) {
            return;
        }

        if (var->Initializer()->Type() != result_type->UnwrapPtrOrRef()) {
            AddError(var) << "initializer type " << NameOf(var->Initializer()->Type())
                          << " does not match store type " << NameOf(result_type->UnwrapPtrOrRef());
            return;
        }
    }

    if (auto result = ValidateBindingPoint(var->BindingPoint(), mv->AddressSpace());
        result != Success) {
        AddError(var) << result.Failure();
        return;
    }

    if (var->Block() == mod_.root_block && mv->AddressSpace() == AddressSpace::kFunction) {
        AddError(var) << "vars in the 'function' address space must be in a function scope";
        return;
    }

    if (auto result = ValidateBindingPoint(var->BindingPoint(), mv->AddressSpace());
        result != Success) {
        AddError(var) << result.Failure();
        return;
    }

    if (mv->AddressSpace() == AddressSpace::kWorkgroup) {
        if (auto* ary = result_type->UnwrapPtr()->As<core::type::Array>()) {
            if (auto* count = ary->Count()->As<core::ir::type::ValueArrayCount>()) {
                if (!scope_stack_.Contains(count->value)) {
                    AddError(var) << NameOf(count->value) << " is not in scope";
                }
            }
        }
    }

    if (mv->AddressSpace() == AddressSpace::kStorage) {
        if (mv->StoreType() && !mv->StoreType()->IsHostShareable()) {
            AddError(var) << "vars in the 'storage' address space must be host-shareable";
            return;
        }
    }

    if (mv->AddressSpace() == AddressSpace::kUniform) {
        if (!mv->StoreType()->IsConstructible() || !mv->StoreType()->IsHostShareable()) {
            AddError(var)
                << "vars in the 'uniform' address space must be host-shareable and constructible";
            return;
        }
    }

    if (var->InputAttachmentIndex().has_value()) {
        if (mv->AddressSpace() != AddressSpace::kHandle) {
            AddError(var) << "'@input_attachment_index' is not valid for non-handle var";
            return;
        }
        if (!capabilities_.Contains(Capability::kAllowAnyInputAttachmentIndexType) &&
            !mv->UnwrapPtrOrRef()->Is<core::type::InputAttachment>()) {
            AddError(var)
                << "'@input_attachment_index' is only valid for 'input_attachment' type var";
            return;
        }
    }

    if (var->Block() == mod_.root_block) {
        if ((mv->AddressSpace() == AddressSpace::kIn || mv->AddressSpace() == AddressSpace::kOut) &&
            !capabilities_.Contains(Capability::kAllowUnannotatedModuleIOVariables)) {
            auto result = ValidateShaderIOAnnotations(var->Result()->Type(), var->BindingPoint(),
                                                      var->Attributes(), "module scope variable");
            if (result != Success) {
                AddError(var) << result.Failure();
            }
        }
    }

    if (mv->AddressSpace() == core::AddressSpace::kPixelLocal) {
        if (var->Block() == mod_.root_block) {
            if (!mv->StoreType()->Is<core::type::Struct>()) {
                AddError(var) << "pixel_local var must be of type struct";
                return;
            }
        }
    }
}

Result<SuccessType, std::string> Validator::ValidateBindingPoint(
    const std::optional<struct BindingPoint>& binding_point,
    AddressSpace address_space,
    const std::string& target_str) {
    switch (address_space) {
        case AddressSpace::kHandle:
            if (!capabilities_.Contains(Capability::kAllowHandleVarsWithoutBindings)) {
                if (!binding_point.has_value()) {
                    return "a resource " + target_str + " is missing binding point";
                }
            }
            break;
        case AddressSpace::kStorage:
        case AddressSpace::kUniform:
            if (!binding_point.has_value()) {
                return "a resource " + target_str + " is missing binding point";
            }
            break;
        default:
            if (binding_point.has_value()) {
                return "a non-resource " + target_str + " has binding point";
            }
            break;
    }
    return Success;
}

Result<SuccessType, std::string> Validator::ValidateShaderIOAnnotations(
    const core::type::Type* ty,
    const std::optional<struct BindingPoint>& binding_point,
    const core::IOAttributes& attr,
    const std::string& target_str) {
    EnumSet<IOAnnotation> annotations;

    // Since there is no entries in the set at this point, this should never fail.
    TINT_ASSERT(AddIOAnnotationsFromIOAttributes(annotations, attr) == Success);

    if (binding_point.has_value()) {
        annotations.Add(IOAnnotation::kBindingPoint);
    }
    if (auto* mv = ty->As<core::type::MemoryView>()) {
        if (mv->AddressSpace() == AddressSpace::kWorkgroup) {
            annotations.Add(IOAnnotation::kWorkgroup);
        }
    }

    if (ty->Is<core::type::Void>()) {
        if (!annotations.Empty()) {
            return target_str + " with void type should never be annotated";
        }
        return Success;
    }

    if (auto* ty_struct = ty->UnwrapPtrOrRef()->As<core::type::Struct>()) {
        for (const auto* mem : ty_struct->Members()) {
            EnumSet<IOAnnotation> mem_annotations = annotations;
            auto add_result = AddIOAnnotationsFromIOAttributes(mem_annotations, mem->Attributes());
            if (add_result != Success) {
                return target_str +
                       " struct member has same IO annotation, as top-level struct, '" +
                       ToString(add_result.Failure()) + "'";
            }

            if (capabilities_.Contains(Capability::kAllowPointersAndHandlesInStructures)) {
                if (auto* mv = mem->Type()->As<core::type::MemoryView>()) {
                    if (mv->AddressSpace() == AddressSpace::kWorkgroup) {
                        mem_annotations.Add(IOAnnotation::kWorkgroup);
                    }
                }
            }

            if (mem_annotations.Empty()) {
                return target_str +
                       " struct members must have at least one IO annotation, e.g. a binding "
                       "point, a location, etc";
            }

            if (mem_annotations.Size() > 1) {
                return target_str + " struct member has more than one IO annotation, " +
                       ToString(mem_annotations);
            }
        }
    } else {
        if (annotations.Empty()) {
            return target_str +
                   " must have at least one IO annotation, e.g. a binding point, a location, etc";
        }
        if (annotations.Size() > 1) {
            return target_str + " has more than one IO annotation, " + ToString(annotations);
        }
    }
    return Success;
}

void Validator::CheckLet(const Let* l) {
    if (!CheckResultsAndOperands(l, Let::kNumResults, Let::kNumOperands)) {
        return;
    }

    auto* value_ty = l->Value()->Type();
    if (capabilities_.Contains(Capability::kAllowAnyLetType)) {
        if (value_ty->Is<core::type::Void>()) {
            AddError(l) << "value type cannot be void";
        }
    } else {
        if (!value_ty->IsConstructible() && !value_ty->Is<core::type::Pointer>()) {
            AddError(l) << "value type, " << NameOf(value_ty)
                        << ", must be concrete constructible type or a pointer type";
        }
    }

    auto* result_ty = l->Result()->Type();
    if (capabilities_.Contains(Capability::kAllowAnyLetType)) {
        if (result_ty->Is<core::type::Void>()) {
            AddError(l) << "result type cannot be void";
        }
    } else {
        if (!result_ty->IsConstructible() && !result_ty->Is<core::type::Pointer>()) {
            AddError(l) << "result type, " << NameOf(result_ty)
                        << ", must be concrete constructible type or a pointer type";
        }
    }

    if (value_ty != result_ty) {
        AddError(l) << "result type " << NameOf(l->Result()->Type())
                    << " does not match value type " << NameOf(l->Value()->Type());
    }
}

void Validator::CheckCall(const Call* call) {
    tint::Switch(
        call,                                                                   //
        [&](const Bitcast* b) { CheckBitcast(b); },                             //
        [&](const BuiltinCall* c) { CheckBuiltinCall(c); },                     //
        [&](const MemberBuiltinCall* c) { CheckMemberBuiltinCall(c); },         //
        [&](const Construct* c) { CheckConstruct(c); },                         //
        [&](const Convert* c) { CheckConvert(c); },                             //
        [&](const Discard* d) {                                                 //
            discards_.Add(d);                                                   //
            CheckDiscard(d);                                                    //
        },                                                                      //
        [&](const UserCall* c) {                                                //
            if (c->Target()) {                                                  //
                auto calls =                                                    //
                    user_func_calls_.GetOr(c->Target(),                         //
                                           Hashset<const ir::UserCall*, 4>{});  //
                calls.Add(c);                                                   //
                user_func_calls_.Replace(c->Target(), calls);                   //
            }
            CheckUserCall(c);
        },
        [&](Default) {
            // Validation of custom IR instructions
        });
}

void Validator::CheckBitcast(const Bitcast* bitcast) {
    if (!CheckResultsAndOperands(bitcast, Bitcast::kNumResults, Bitcast::kNumOperands)) {
        return;
    }

    CheckBitcastTypes(bitcast);
}

void Validator::CheckBitcastTypes(const Bitcast* bitcast) {
    // Caller is responsible for checking results and operands
    const auto* val_type = bitcast->Operand(Bitcast::kValueOperandOffset)->Type();
    const auto* result_type = bitcast->Result()->Type();

    const auto add_error = [&]() {
        AddError(bitcast) << "bitcast is not defined for " << NameOf(val_type) << " -> "
                          << NameOf(result_type);
    };

    // Check that there exists an overload for the provided types
    if (val_type->IsAnyOf<core::type::U32, core::type::I32, core::type::F32>()) {
        // S, where S is i32, u32, or f32
        // S -> S, identity and reinterpretation
        if (result_type->IsAnyOf<core::type::U32, core::type::I32, core::type::F32>()) {
            return;
        }

        // S -> vec2<f16>
        if (auto* vec_type = result_type->As<core::type::Vector>()) {
            auto elements = vec_type->Elements();
            if (elements.count == 2 && Is<core::type::F16>(elements.type)) {
                return;
            }
        }
    } else if (val_type->Is<core::type::F16>()) {
        // f16 -> f16, identity
        if (result_type->Is<core::type::F16>()) {
            return;
        }
    } else if (val_type->Is<core::type::Vector>()) {
        auto val_elements = val_type->Elements();
        if (!val_elements.type) {
            // Malformed vector
            add_error();
            return;
        }

        std::optional<core::type::TypeAndCount> result_elements;
        if (result_type->As<core::type::Vector>()) {
            result_elements = result_type->Elements();
            if (!result_elements->type) {
                // Malformed vector
                add_error();
                return;
            }
        }

        if (val_elements.type->IsAnyOf<core::type::U32, core::type::I32, core::type::F32>()) {
            // vecN<S>, where S is i32, u32, or f32
            // vecN<S> -> vecN<s>, identity and reinterpretation cases
            if (result_elements.has_value() && val_elements.count == result_elements->count &&
                result_elements->type
                    ->IsAnyOf<core::type::U32, core::type::I32, core::type::F32>()) {
                return;
            }

            // vec2<S> -> vec4<f16>
            if (val_elements.count == 2) {
                if (result_elements.has_value() && result_elements->count == 4 &&
                    result_elements->type->Is<core::type::F16>()) {
                    return;
                }
            }
        } else if (val_elements.type->Is<core::type::F16>()) {
            if (result_elements.has_value()) {
                if (result_elements->type->Is<core::type::F16>()) {
                    // vecN<f16> -> vecN<f16>, identity
                    if (val_elements.count == result_elements->count) {
                        return;
                    }
                } else {
                    // vec4<f16> -> vec2<S>, where S is i32, u32, or f32
                    if (val_elements.count == 4 && result_elements->count == 2 &&
                        result_elements->type
                            ->IsAnyOf<core::type::U32, core::type::I32, core::type::F32>()) {
                        return;
                    }
                }
            } else {
                // vec2<f16> -> i32, u32, or f32
                if (val_elements.count == 2 &&
                    result_type->IsAnyOf<core::type::U32, core::type::I32, core::type::F32>()) {
                    return;
                }
            }
        }
    }

    // No matching case for val and result type combination
    add_error();
}

void Validator::CheckBuiltinCall(const BuiltinCall* call) {
    // This check cannot be more precise, since until intrinsic lookup below, it is unknown what
    // number of operands are expected, but still need to enforce things are in scope,
    // have types, etc.
    if (!CheckResults(call, BuiltinCall::kNumResults) || !CheckOperands(call)) {
        return;
    }

    // CheckOperands above ensures that all args are non-null and have a valid type
    auto args = Transform<8>(call->Args(), [&](const ir::Value* v) { return v->Type(); });

    intrinsic::Context context{
        call->TableData(),
        type_mgr_,
        symbols_,
    };

    auto builtin = core::intrinsic::LookupFn(context, call->FriendlyName().c_str(), call->FuncId(),
                                             call->ExplicitTemplateParams(), args,
                                             core::EvaluationStage::kRuntime);
    if (builtin != Success) {
        AddError(call) << builtin.Failure();
        return;
    }

    TINT_ASSERT(builtin->return_type);

    if (builtin->return_type != call->Result()->Type()) {
        AddError(call) << "call result type " << NameOf(call->Result()->Type())
                       << " does not match builtin return type " << NameOf(builtin->return_type);
        return;
    }
}

void Validator::CheckMemberBuiltinCall(const MemberBuiltinCall* call) {
    // This check cannot be more precise, since until intrinsic lookup below, it is unknown what
    // number of operands are expected, but still need to enforce things are in scope,
    // have types, etc.
    if (!CheckResults(call, MemberBuiltinCall::kNumResults) || !CheckOperands(call)) {
        return;
    }

    auto args = Vector<const core::type::Type*, 8>({call->Object()->Type()});
    for (auto* arg : call->Args()) {
        args.Push(arg->Type());
    }
    intrinsic::Context context{
        call->TableData(),
        type_mgr_,
        symbols_,
    };

    auto result = core::intrinsic::LookupMemberFn(context, call->FriendlyName().c_str(),
                                                  call->FuncId(), call->ExplicitTemplateParams(),
                                                  std::move(args), core::EvaluationStage::kRuntime);
    if (result != Success) {
        AddError(call) << result.Failure();
        return;
    }

    if (result->return_type != call->Result()->Type()) {
        AddError(call) << "member call result type " << NameOf(call->Result()->Type())
                       << " does not match builtin return type " << NameOf(result->return_type);
    }
}

void Validator::CheckConstruct(const Construct* construct) {
    if (!CheckResultsAndOperandRange(construct, Construct::kNumResults, Construct::kMinOperands)) {
        return;
    }

    if (!construct->Result()->Type()->IsConstructible() &&
        !capabilities_.Contains(Capability::kAllowPointersAndHandlesInStructures)) {
        AddError(construct) << "type is not constructible";
        return;
    }

    auto args = construct->Args();
    if (args.IsEmpty()) {
        // Zero-value constructors are valid for all constructible types.
        return;
    }

    auto* result_type = construct->Result()->Type();

    auto check_args_match_elements = [&] {
        // Check that type type of each argument matches the expected element type of the composite.
        for (size_t i = 0; i < args.Length(); i++) {
            if (args[i]->Is<ir::Unused>()) {
                continue;
            }
            auto* expected_type = result_type->Element(static_cast<uint32_t>(i));
            if (args[i]->Type() != expected_type) {
                AddError(construct, Construct::kArgsOperandOffset + i)
                    << "type " << NameOf(args[i]->Type()) << " of argument " << i
                    << " does not match expected type " << NameOf(expected_type);
            }
        }
    };

    if (result_type->Is<core::type::Scalar>()) {
        // The only valid non-zero scalar constructor is the identity operation.
        if (args.Length() > 1) {
            AddError(construct) << "scalar construct must not have more than one argument";
        }
        if (args[0]->Type() != result_type) {
            AddError(construct, 0u) << "scalar construct argument type " << NameOf(args[0]->Type())
                                    << " does not match result type " << NameOf(result_type);
        }
    } else if (auto* vec = result_type->As<core::type::Vector>()) {
        auto table = intrinsic::Table<intrinsic::Dialect>(type_mgr_, symbols_);
        auto ctor_conv = intrinsic::VectorCtorConv(vec->Width());
        auto arg_types = Transform<4>(args, [&](auto* v) { return v->Type(); });
        auto match = table.Lookup(ctor_conv, Vector{vec->Type()}, std::move(arg_types),
                                  core::EvaluationStage::kConstant);
        if (match != Success) {
            AddError(construct) << "no matching overload for " << vec->FriendlyName()
                                << " constructor";
        }
    } else if (auto* mat = result_type->As<core::type::Matrix>()) {
        auto table = intrinsic::Table<intrinsic::Dialect>(type_mgr_, symbols_);
        auto ctor_conv = intrinsic::MatrixCtorConv(mat->Columns(), mat->Rows());
        auto arg_types = Transform<8>(args, [&](auto* v) { return v->Type(); });
        auto match = table.Lookup(ctor_conv, Vector{mat->Type()}, std::move(arg_types),
                                  core::EvaluationStage::kConstant);
        if (match != Success) {
            AddError(construct) << "no matching overload for " << mat->FriendlyName()
                                << " constructor";
        }
    } else if (result_type->Is<core::type::Array>()) {
        check_args_match_elements();
    } else if (auto* str = As<core::type::Struct>(result_type)) {
        auto members = str->Members();
        if (args.Length() != str->Members().Length()) {
            AddError(construct) << "structure has " << members.Length()
                                << " members, but construct provides " << args.Length()
                                << " arguments";
            return;
        }
        check_args_match_elements();
    }
}

void Validator::CheckConvert(const Convert* convert) {
    if (!CheckResultsAndOperands(convert, Convert::kNumResults, Convert::kNumOperands)) {
        return;
    }

    auto* result_type = convert->Result()->Type();
    auto* value_type = convert->Operand(Convert::kValueOperandOffset)->Type();

    intrinsic::CtorConv conv_ty;
    Vector<const core::type::Type*, 1> template_type;
    tint::Switch(
        result_type,                                                             //
        [&](const core::type::I32*) { conv_ty = intrinsic::CtorConv::kI32; },    //
        [&](const core::type::U32*) { conv_ty = intrinsic::CtorConv::kU32; },    //
        [&](const core::type::F32*) { conv_ty = intrinsic::CtorConv::kF32; },    //
        [&](const core::type::F16*) { conv_ty = intrinsic::CtorConv::kF16; },    //
        [&](const core::type::Bool*) { conv_ty = intrinsic::CtorConv::kBool; },  //
        [&](const core::type::Vector* v) {
            conv_ty = intrinsic::VectorCtorConv(v->Width());
            template_type.Push(v->Type());
        },
        [&](const core::type::Matrix* m) {
            conv_ty = intrinsic::MatrixCtorConv(m->Columns(), m->Rows());
            template_type.Push(m->Type());
        },
        [&](Default) { conv_ty = intrinsic::CtorConv::kNone; });

    if (conv_ty == intrinsic::CtorConv::kNone) {
        AddError(convert) << "not defined for result type, " << NameOf(result_type);
        return;
    }

    auto table = intrinsic::Table<intrinsic::Dialect>(type_mgr_, symbols_);
    auto match =
        table.Lookup(conv_ty, template_type, Vector{value_type}, core::EvaluationStage::kOverride);
    if (match != Success || !match->info->flags.Contains(intrinsic::OverloadFlag::kIsConverter)) {
        AddError(convert) << "No defined converter for " << NameOf(value_type) << " -> "
                          << NameOf(result_type);
        return;
    }
}

void Validator::CheckDiscard(const tint::core::ir::Discard* discard) {
    CheckResultsAndOperands(discard, Discard::kNumResults, Discard::kNumOperands);
}

void Validator::CheckUserCall(const UserCall* call) {
    if (!CheckResultsAndOperandRange(call, UserCall::kNumResults, UserCall::kMinOperands)) {
        return;
    }

    if (!call->Target()) {
        AddError(call, UserCall::kFunctionOperandOffset) << "target not defined or not a function";
        return;
    }

    if (call->Target()->IsEntryPoint()) {
        AddError(call, UserCall::kFunctionOperandOffset)
            << "call target must not have a pipeline stage";
    }

    auto args = call->Args();
    auto params = call->Target()->Params();
    if (args.Length() != params.Length()) {
        AddError(call, UserCall::kFunctionOperandOffset)
            << "function has " << params.Length() << " parameters, but call provides "
            << args.Length() << " arguments";
        return;
    }

    for (size_t i = 0; i < args.Length(); i++) {
        if (args[i]->Type() != params[i]->Type()) {
            AddError(call, UserCall::kArgsOperandOffset + i)
                << "type " << NameOf(params[i]->Type()) << " of function parameter " << i
                << " does not match argument type " << NameOf(args[i]->Type());
        }
    }
}

void Validator::CheckAccess(const Access* a) {
    if (!CheckResultsAndOperandRange(a, Access::kNumResults, Access::kMinNumOperands)) {
        return;
    }

    auto* obj_view = a->Object()->Type()->As<core::type::MemoryView>();
    auto* ty = obj_view ? obj_view->StoreType() : a->Object()->Type();

    enum Kind : uint8_t { kPtr, kRef, kValue };

    auto kind_of = [&](const core::type::Type* type) {
        return tint::Switch(
            type,                                                //
            [&](const core::type::Pointer*) { return kPtr; },    //
            [&](const core::type::Reference*) { return kRef; },  //
            [&](Default) { return kValue; });
    };

    const Kind in_kind = kind_of(a->Object()->Type());
    auto desc_of = [&](Kind kind, const core::type::Type* type) {
        switch (kind) {
            case kPtr:
                return StyledText{}
                       << style::Type("ptr<", obj_view->AddressSpace(), ", ", type->FriendlyName(),
                                      ", ", obj_view->Access(), ">");
            case kRef:
                return StyledText{}
                       << style::Type("ref<", obj_view->AddressSpace(), ", ", type->FriendlyName(),
                                      ", ", obj_view->Access(), ">");
            default:
                return NameOf(type);
        }
    };

    for (size_t i = 0; i < a->Indices().Length(); i++) {
        auto err = [&]() -> diag::Diagnostic& {
            return AddError(a, i + Access::kIndicesOperandOffset);
        };
        auto note = [&]() -> diag::Diagnostic& {
            return AddOperandNote(a, i + Access::kIndicesOperandOffset);
        };

        auto* index = a->Indices()[i];
        if (DAWN_UNLIKELY(!index->Type() || !index->Type()->IsIntegerScalar())) {
            err() << "index type " << NameOf(index->Type()) << " must be an integer";
            return;
        }

        if (!capabilities_.Contains(Capability::kAllowVectorElementPointer)) {
            if (in_kind != kValue && ty->Is<core::type::Vector>()) {
                err() << "cannot obtain address of vector element";
                return;
            }
        }

        if (auto* const_index = index->As<ir::Constant>()) {
            auto* value = const_index->Value();
            if (!value->Type() || value->Type()->IsSignedIntegerScalar()) {
                // index is a signed integer scalar. Check that the index isn't negative.
                // If the index is unsigned, we can skip this.
                auto idx = value->ValueAs<AInt>();
                if (DAWN_UNLIKELY(idx < 0)) {
                    err() << "constant index must be positive, got " << idx;
                    return;
                }
            }

            auto idx = value->ValueAs<uint32_t>();
            auto* el = ty->Element(idx);
            if (DAWN_UNLIKELY(!el)) {
                // Is index in bounds?
                if (auto el_count = ty->Elements().count; el_count != 0 && idx >= el_count) {
                    err() << "index out of bounds for type " << desc_of(in_kind, ty);
                    note() << "acceptable range: [0.." << (el_count - 1) << "]";
                    return;
                }
                err() << "type " << desc_of(in_kind, ty) << " cannot be indexed";
                return;
            }
            ty = el;
        } else {
            auto* el = ty->Elements().type;
            if (DAWN_UNLIKELY(!el)) {
                err() << "type " << desc_of(in_kind, ty) << " cannot be dynamically indexed";
                return;
            }
            ty = el;
        }
    }

    auto* want = a->Result()->Type();
    auto* want_view = want->As<core::type::MemoryView>();
    bool ok = true;
    if (obj_view) {
        // Pointer source always means pointer result.
        ok = (want_view != nullptr) && ty == want_view->StoreType();
        if (ok) {
            // Also check that the address space and access modes match.
            ok = obj_view->Is<core::type::Pointer>() == want_view->Is<core::type::Pointer>() &&
                 obj_view->AddressSpace() == want_view->AddressSpace() &&
                 obj_view->Access() == want_view->Access();
        }
    } else {
        // Otherwise, result types should exactly match.
        ok = ty == want;
    }
    if (DAWN_UNLIKELY(!ok)) {
        AddError(a) << "result of access chain is type " << desc_of(in_kind, ty)
                    << " but instruction type is " << NameOf(want);
    }
}

void Validator::CheckBinary(const Binary* b) {
    if (!CheckResultsAndOperandRange(b, Binary::kNumResults, Binary::kNumOperands)) {
        return;
    }

    if (b->LHS() && b->RHS()) {
        intrinsic::Context context{b->TableData(), type_mgr_, symbols_};

        auto overload =
            core::intrinsic::LookupBinary(context, b->Op(), b->LHS()->Type(), b->RHS()->Type(),
                                          core::EvaluationStage::kRuntime, /* is_compound */ false);
        if (overload != Success) {
            AddError(b) << overload.Failure();
            return;
        }

        if (auto* result = b->Result(0)) {
            if (overload->return_type != result->Type()) {
                AddError(b) << "result value type " << NameOf(result->Type()) << " does not match "
                            << style::Instruction(Disassemble().NameOf(b->Op())) << " result type "
                            << NameOf(overload->return_type);
            }
        }
    }
}

void Validator::CheckUnary(const Unary* u) {
    if (!CheckResultsAndOperandRange(u, Unary::kNumResults, Unary::kNumOperands)) {
        return;
    }

    if (u->Val()) {
        intrinsic::Context context{u->TableData(), type_mgr_, symbols_};

        auto overload = core::intrinsic::LookupUnary(context, u->Op(), u->Val()->Type(),
                                                     core::EvaluationStage::kRuntime);
        if (overload != Success) {
            AddError(u) << overload.Failure();
            return;
        }

        if (auto* result = u->Result(0)) {
            if (overload->return_type != result->Type()) {
                AddError(u) << "result value type " << NameOf(result->Type()) << " does not match "
                            << style::Instruction(Disassemble().NameOf(u->Op())) << " result type "
                            << NameOf(overload->return_type);
            }
        }
    }
}

void Validator::CheckIf(const If* if_) {
    CheckResults(if_);
    CheckOperand(if_, If::kConditionOperandOffset);

    if (if_->Condition() && !if_->Condition()->Type()->Is<core::type::Bool>()) {
        AddError(if_, If::kConditionOperandOffset) << "condition type must be 'bool'";
    }

    tasks_.Push([this] { control_stack_.Pop(); });

    if (!if_->False()->IsEmpty()) {
        QueueBlock(if_->False());
    }

    QueueBlock(if_->True());

    tasks_.Push([this, if_] { control_stack_.Push(if_); });
}

void Validator::CheckLoop(const Loop* l) {
    CheckResults(l);
    CheckOperands(l, 0);

    // Note: Tasks are queued in reverse order of their execution
    tasks_.Push([this, l] {
        first_continues_.Remove(l);  // No need for this any more. Free memory.
        control_stack_.Pop();
    });
    if (!l->Initializer()->IsEmpty()) {
        tasks_.Push([this] { EndBlock(); });
    }
    tasks_.Push([this] { EndBlock(); });
    if (!l->Continuing()->IsEmpty()) {
        tasks_.Push([this] { EndBlock(); });
    }

    // ⎡Initializer              ⎤
    // ⎢    ⎡Body               ⎤⎥
    // ⎣    ⎣    [Continuing ]  ⎦⎦

    if (!l->Continuing()->IsEmpty()) {
        tasks_.Push([this, l] {
            CheckLoopContinuing(l);
            BeginBlock(l->Continuing());
        });
    }

    tasks_.Push([this, l] {
        CheckLoopBody(l);
        BeginBlock(l->Body());
    });
    if (!l->Initializer()->IsEmpty()) {
        tasks_.Push([this, l] { BeginBlock(l->Initializer()); });
    }
    tasks_.Push([this, l] { control_stack_.Push(l); });
}

void Validator::CheckLoopBody(const Loop* loop) {
    // If the body block has parameters, there must be an initializer block.
    if (!loop->Body()->Params().IsEmpty()) {
        if (!loop->HasInitializer()) {
            AddError(loop) << "loop with body block parameters must have an initializer";
        }
    }
}

void Validator::CheckLoopContinuing(const Loop* loop) {
    if (!loop->HasContinuing()) {
        return;
    }

    // Ensure that values used in the loop continuing are not from the loop body, after a
    // continue instruction.
    if (auto* first_continue = first_continues_.GetOr(loop, nullptr)) {
        // Find the instruction in the body block that is or holds the first continue
        // instruction.
        const Instruction* holds_continue = first_continue;
        while (holds_continue && holds_continue->Block() &&
               holds_continue->Block() != loop->Body()) {
            holds_continue = holds_continue->Block()->Parent();
        }

        // Check that all subsequent instruction values are not used in the continuing block.
        for (auto* inst = holds_continue; inst; inst = inst->next) {
            for (auto* result : inst->Results()) {
                result->ForEachUseUnsorted([&](Usage use) {
                    if (TransitivelyHolds(loop->Continuing(), use.instruction)) {
                        AddError(use.instruction, use.operand_index)
                            << NameOf(result)
                            << " cannot be used in continuing block as it is declared after the "
                               "first "
                            << style::Instruction("continue") << " in the loop's body";
                        AddDeclarationNote(result);
                        AddNote(first_continue)
                            << "loop body's first " << style::Instruction("continue");
                    }
                });
            }
        }
    }
}

void Validator::CheckSwitch(const Switch* s) {
    CheckOperand(s, Switch::kConditionOperandOffset);

    if (s->Condition() && !s->Condition()->Type()->IsIntegerScalar()) {
        auto* cond_ty = s->Condition() ? s->Condition()->Type() : nullptr;
        AddError(s, Switch::kConditionOperandOffset)
            << "condition type " << NameOf(cond_ty) << " must be an integer scalar";
    }

    tasks_.Push([this] { control_stack_.Pop(); });

    bool found_default = false;
    for (auto& cse : s->Cases()) {
        QueueBlock(cse.block);

        for (const auto& sel : cse.selectors) {
            if (sel.IsDefault()) {
                found_default = true;
            }
        }
    }

    if (!found_default) {
        AddError(s) << "missing default case for switch";
    }

    tasks_.Push([this, s] { control_stack_.Push(s); });
}

void Validator::CheckSwizzle(const Swizzle* s) {
    if (!CheckResultsAndOperands(s, Swizzle::kNumResults, Swizzle::kNumOperands)) {
        return;
    }

    auto* src_vec = s->Object()->Type()->As<core::type::Vector>();
    if (!src_vec) {
        AddError(s) << "object of swizzle, " << NameOf(s->Object()) << ", is not a vector, "
                    << NameOf(s->Object()->Type());
        return;
    }

    auto indices = s->Indices();
    if (indices.Length() < Swizzle::kMinNumIndices) {
        AddError(s) << "expected at least " << Swizzle::kMinNumIndices << " indices";
        return;
    }

    if (indices.Length() > Swizzle::kMaxNumIndices) {
        AddError(s) << "expected at most " << Swizzle::kMaxNumIndices << " indices";
        return;
    }

    auto elem_count = src_vec->Elements().count;
    for (auto& idx : indices) {
        if (idx > Swizzle::kMaxIndexValue || idx >= elem_count) {
            AddError(s) << "invalid index value";
            return;
        }
    }

    auto* elem_ty = src_vec->Elements().type;
    auto* expected_ty = type_mgr_.MatchWidth(elem_ty, indices.Length());
    auto* result_ty = s->Result()->Type();
    if (result_ty != expected_ty) {
        AddError(s) << "result type " << NameOf(result_ty) << " does not match expected type, "
                    << NameOf(expected_ty);
        return;
    }
}

void Validator::CheckTerminator(const Terminator* b) {
    // All terminators should have zero results
    if (!CheckResults(b, 0)) {
        return;
    }

    // Operands must be alive and in scope if they are not nullptr.
    if (!CheckOperands(b)) {
        return;
    }

    tint::Switch(
        b,                                                           //
        [&](const ir::BreakIf* i) { CheckBreakIf(i); },              //
        [&](const ir::Continue* c) { CheckContinue(c); },            //
        [&](const ir::Exit* e) { CheckExit(e); },                    //
        [&](const ir::NextIteration* n) { CheckNextIteration(n); },  //
        [&](const ir::Return* ret) { CheckReturn(ret); },            //
        [&](const ir::TerminateInvocation*) {},                      //
        [&](const ir::Unreachable* u) { CheckUnreachable(u); },      //
        [&](Default) { AddError(b) << "missing validation"; });

    if (b->next) {
        AddError(b) << "must be the last instruction in the block";
    }
}

void Validator::CheckBreakIf(const BreakIf* b) {
    auto* loop = b->Loop();
    if (loop == nullptr) {
        AddError(b) << "has no associated loop";
        return;
    }

    if (loop->Continuing() != b->Block()) {
        AddError(b) << "must only be called directly from loop continuing";
    }

    auto next_iter_values = b->NextIterValues();
    if (auto* body = loop->Body()) {
        CheckOperandsMatchTarget(b, b->ArgsOperandOffset(), next_iter_values.Length(), body,
                                 body->Params());
    }

    auto exit_values = b->ExitValues();
    CheckOperandsMatchTarget(b, b->ArgsOperandOffset() + next_iter_values.Length(),
                             exit_values.Length(), loop, loop->Results());
}

void Validator::CheckContinue(const Continue* c) {
    auto* loop = c->Loop();
    if (loop == nullptr) {
        AddError(c) << "has no associated loop";
        return;
    }
    if (!TransitivelyHolds(loop->Body(), c)) {
        if (control_stack_.Any(Eq<const ControlInstruction*>(loop))) {
            AddError(c) << "must only be called from loop body";
        } else {
            AddError(c) << "called outside of associated loop";
        }
    }

    if (auto* cont = loop->Continuing()) {
        CheckOperandsMatchTarget(c, Continue::kArgsOperandOffset, c->Args().Length(), cont,
                                 cont->Params());
    }

    first_continues_.Add(loop, c);
}

void Validator::CheckExit(const Exit* e) {
    if (e->ControlInstruction() == nullptr) {
        AddError(e) << "has no parent control instruction";
        return;
    }

    if (control_stack_.IsEmpty()) {
        AddError(e) << "found outside all control instructions";
        return;
    }

    auto args = e->Args();
    CheckOperandsMatchTarget(e, e->ArgsOperandOffset(), args.Length(), e->ControlInstruction(),
                             e->ControlInstruction()->Results());

    tint::Switch(
        e,                                                     //
        [&](const ir::ExitIf* i) { CheckExitIf(i); },          //
        [&](const ir::ExitLoop* l) { CheckExitLoop(l); },      //
        [&](const ir::ExitSwitch* s) { CheckExitSwitch(s); },  //
        [&](Default) { AddError(e) << "missing validation"; });
}

void Validator::CheckNextIteration(const NextIteration* n) {
    auto* loop = n->Loop();
    if (loop == nullptr) {
        AddError(n) << "has no associated loop";
        return;
    }
    if (!TransitivelyHolds(loop->Initializer(), n) && !TransitivelyHolds(loop->Continuing(), n)) {
        if (control_stack_.Any(Eq<const ControlInstruction*>(loop))) {
            AddError(n) << "must only be called from loop initializer or continuing";
        } else {
            AddError(n) << "called outside of associated loop";
        }
    }

    if (auto* body = loop->Body()) {
        CheckOperandsMatchTarget(n, NextIteration::kArgsOperandOffset, n->Args().Length(), body,
                                 body->Params());
    }
}

void Validator::CheckExitIf(const ExitIf* e) {
    if (control_stack_.Back() != e->If()) {
        AddError(e) << "if target jumps over other control instructions";
        AddNote(control_stack_.Back()) << "first control instruction jumped";
    }
}

void Validator::CheckReturn(const Return* ret) {
    if (!CheckOperands(ret, Return::kMinOperands, Return::kMaxOperands)) {
        return;
    }

    auto* func = ret->Func();
    if (func == nullptr) {
        // Func() returning nullptr after CheckResultsAndOperandRange is due to the first
        // operand being not a function
        AddError(ret) << "expected function for first operand";
        return;
    }

    if (func->ReturnType()->Is<core::type::Void>()) {
        if (ret->HasValue()) {
            AddError(ret) << "unexpected return value";
        }
    } else {
        if (!ret->Value()) {
            AddError(ret) << "expected return value";
        } else if (ret->Value()->Type() != func->ReturnType()) {
            AddError(ret) << "return value type " << NameOf(ret->Value()->Type())
                          << " does not match function return type " << NameOf(func->ReturnType());
        }
    }
}

void Validator::CheckUnreachable(const Unreachable* u) {
    CheckResultsAndOperands(u, Unreachable::kNumResults, Unreachable::kNumOperands);
}

void Validator::CheckControlsAllowingIf(const Exit* exit, const Instruction* control) {
    bool found = false;
    for (auto ctrl : tint::Reverse(control_stack_)) {
        if (ctrl == control) {
            found = true;
            break;
        }
        // A exit switch can step over if instructions, but no others.
        if (!ctrl->Is<ir::If>()) {
            AddError(exit) << control->FriendlyName()
                           << " target jumps over other control instructions";
            AddNote(ctrl) << "first control instruction jumped";
            return;
        }
    }
    if (!found) {
        AddError(exit) << control->FriendlyName() << " not found in parent control instructions";
    }
}

void Validator::CheckExitSwitch(const ExitSwitch* s) {
    CheckControlsAllowingIf(s, s->ControlInstruction());
}

void Validator::CheckExitLoop(const ExitLoop* l) {
    CheckControlsAllowingIf(l, l->ControlInstruction());

    const Instruction* inst = l;
    const Loop* control = l->Loop();
    while (inst) {
        // Found parent loop
        if (inst->Block()->Parent() == control) {
            if (inst->Block() == control->Continuing()) {
                AddError(l) << "loop exit jumps out of continuing block";
                if (control->Continuing() != l->Block()) {
                    AddNote(control->Continuing()) << "in continuing block";
                }
            } else if (inst->Block() == control->Initializer()) {
                AddError(l) << "loop exit not permitted in loop initializer";
                if (control->Initializer() != l->Block()) {
                    AddNote(control->Initializer()) << "in initializer block";
                }
            }
            break;
        }
        inst = inst->Block()->Parent();
    }
}

void Validator::CheckLoad(const Load* l) {
    if (!CheckResultsAndOperands(l, Load::kNumResults, Load::kNumOperands)) {
        return;
    }

    if (auto* from = l->From()) {
        auto* mv = from->Type()->As<core::type::MemoryView>();
        if (!mv) {
            AddError(l, Load::kFromOperandOffset)
                << "load source operand " << NameOf(from->Type()) << " is not a memory view";
            return;
        }

        if (mv->Access() != core::Access::kRead && mv->Access() != core::Access::kReadWrite) {
            AddError(l, Load::kFromOperandOffset)
                << "load source operand has a non-readable access type, "
                << style::Literal(ToString(mv->Access()));
            return;
        }

        if (l->Result()->Type() != mv->StoreType()) {
            AddError(l, Load::kFromOperandOffset)
                << "result type " << NameOf(l->Result()->Type())
                << " does not match source store type " << NameOf(mv->StoreType());
        }

        if (auto* arr = mv->StoreType()->As<core::type::Array>()) {
            if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
                AddError(l) << "cannot load a runtime-sized array";
                return;
            }
        }
    }
}

void Validator::CheckStore(const Store* s) {
    if (!CheckResultsAndOperands(s, Store::kNumResults, Store::kNumOperands)) {
        return;
    }

    if (auto* from = s->From()) {
        if (auto* to = s->To()) {
            auto* mv = As<core::type::MemoryView>(to->Type());
            if (!mv) {
                AddError(s, Store::kToOperandOffset)
                    << "store target operand " << NameOf(to->Type()) << " is not a memory view";
                return;
            }

            if (mv->Access() != core::Access::kWrite && mv->Access() != core::Access::kReadWrite) {
                AddError(s, Store::kToOperandOffset)
                    << "store target operand has a non-writeable access type, "
                    << style::Literal(ToString(mv->Access()));
                return;
            }

            auto* value_type = from->Type();
            auto* store_type = mv->StoreType();
            if (value_type != store_type) {
                AddError(s, Store::kFromOperandOffset)
                    << "value type " << NameOf(value_type) << " does not match store type "
                    << NameOf(store_type);
                return;
            }

            if (!store_type->IsConstructible()) {
                AddError(s) << "store type " << NameOf(store_type) << " is not constructible";
                return;
            }
        }
    }
}

void Validator::CheckLoadVectorElement(const LoadVectorElement* l) {
    if (!CheckResultsAndOperands(l, LoadVectorElement::kNumResults,
                                 LoadVectorElement::kNumOperands)) {
        return;
    }

    if (auto* res = l->Result(0)) {
        if (auto* el_ty = GetVectorPtrElementType(l, LoadVectorElement::kFromOperandOffset)) {
            if (res->Type() != el_ty) {
                AddResultError(l, 0)
                    << "result type " << NameOf(res->Type())
                    << " does not match vector pointer element type " << NameOf(el_ty);
            }
        }
    }
}

void Validator::CheckStoreVectorElement(const StoreVectorElement* s) {
    if (!CheckResultsAndOperands(s, StoreVectorElement::kNumResults,
                                 StoreVectorElement::kNumOperands)) {
        return;
    }

    if (auto* value = s->Value()) {
        if (auto* el_ty = GetVectorPtrElementType(s, StoreVectorElement::kToOperandOffset)) {
            if (value->Type() != el_ty) {
                AddError(s, StoreVectorElement::kValueOperandOffset)
                    << "value type " << NameOf(value->Type())
                    << " does not match vector pointer element type " << NameOf(el_ty);
            }
        }
    }
}

void Validator::CheckPhony(const Phony* p) {
    if (!capabilities_.Contains(Capability::kAllowPhonyInstructions)) {
        AddError(p) << "missing capability 'kAllowPhonyInstructions'";
        return;
    }

    if (!CheckResultsAndOperands(p, Phony::kNumResults, Phony::kNumOperands)) {
        return;
    }
}

void Validator::CheckOperandsMatchTarget(const Instruction* source_inst,
                                         size_t source_operand_offset,
                                         size_t source_operand_count,
                                         const CastableBase* target,
                                         VectorRef<const Value*> target_values) {
    if (source_operand_count != target_values.Length()) {
        auto values = [&](size_t n) { return n == 1 ? " value" : " values"; };
        AddError(source_inst) << "provides " << source_operand_count << values(source_operand_count)
                              << " but " << NameOf(target) << " expects " << target_values.Length()
                              << values(target_values.Length());
        AddDeclarationNote(target);
    }
    size_t count = std::min(source_operand_count, target_values.Length());
    for (size_t i = 0; i < count; i++) {
        auto* source_value = source_inst->Operand(source_operand_offset + i);
        auto* target_value = target_values[i];
        if (!source_value || !target_value) {
            continue;  // Caller should be checking operands are not null
        }
        auto* source_type = source_value->Type();
        auto* target_type = target_value->Type();
        if (source_type != target_type) {
            AddError(source_inst, source_operand_offset + i)
                << "operand with type " << NameOf(source_type) << " does not match "
                << NameOf(target) << " target type " << NameOf(target_type);
            AddDeclarationNote(target_value);
        }
    }
}

const core::type::Type* Validator::GetVectorPtrElementType(const Instruction* inst, size_t idx) {
    auto* operand = inst->Operands()[idx];
    if (DAWN_UNLIKELY(!operand)) {
        return nullptr;
    }

    auto* type = operand->Type();
    if (DAWN_UNLIKELY(!type)) {
        return nullptr;
    }

    auto* memory_view_ty = type->As<core::type::MemoryView>();
    if (DAWN_LIKELY(memory_view_ty)) {
        auto* vec_ty = memory_view_ty->StoreType()->As<core::type::Vector>();
        if (DAWN_LIKELY(vec_ty)) {
            return vec_ty->Type();
        }
    }

    AddError(inst, idx) << "operand " << NameOf(type) << " must be a pointer to a vector";
    return nullptr;
}

}  // namespace

Result<SuccessType> Validate(const Module& mod, Capabilities capabilities) {
    Validator v(mod, capabilities);
    auto res = v.Run();
    if (res != Success) {
        return res;
    }
    return Success;
}

Result<SuccessType> ValidateAndDumpIfNeeded([[maybe_unused]] const Module& ir,
                                            [[maybe_unused]] const char* msg,
                                            [[maybe_unused]] Capabilities capabilities,
                                            [[maybe_unused]] std::string_view timing) {
#if TINT_DUMP_IR_WHEN_VALIDATING
    auto printer = StyledTextPrinter::Create(stdout);
    std::cout << "=========================================================\n";
    std::cout << "== IR dump " << timing << " " << msg << ":\n";
    std::cout << "=========================================================\n";
    printer->Print(Disassembler(ir).Text());
#endif

#if TINT_ENABLE_IR_VALIDATION
    auto result = Validate(ir, capabilities);
    if (result != Success) {
        return result.Failure();
    }
#endif

    return Success;
}

}  // namespace tint::core::ir

namespace std {

template <>
struct hash<tint::core::ir::ValidatedType> {
    size_t operator()(const tint::core::ir::ValidatedType& v) const { return Hash(v.ty, v.caps); }
};

template <>
struct equal_to<tint::core::ir::ValidatedType> {
    bool operator()(const tint::core::ir::ValidatedType& a,
                    const tint::core::ir::ValidatedType& b) const {
        return a.ty->Equals(*(b.ty)) && a.caps == b.caps;
    }
};

}  // namespace std
