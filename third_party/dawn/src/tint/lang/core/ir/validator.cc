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
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/binary.h"
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
#include "src/tint/lang/core/type/array_count.h"
#include "src/tint/lang/core/type/binding_array.h"
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
#include "src/tint/lang/core/type/u16.h"
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
#include "src/tint/utils/internal_limits.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/result.h"
#include "src/tint/utils/rtti/castable.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/styled_text_printer.h"
#include "src/tint/utils/text/text_style.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::core::ir {

struct ValidatedType {
    const core::type::Type* ty;
    Capabilities caps;
};

namespace {

/// Prints out the current IR state, iff ir.dump_ir_when_validating is set.
void DumpIRIfEnabled([[maybe_unused]] const Module& ir,
                     [[maybe_unused]] const std::string_view msg) {
#if TINT_ENABLE_IR_DUMPING
    if (ir.dump_ir_when_validating) {
        auto printer = StyledTextPrinter::Create(stdout);
        std::cout << "=========================================================\n";
        std::cout << "== IR dump " << msg << ":\n";
        std::cout << "=========================================================\n";
        printer->Print(Disassembler(ir).Text());
    }
#endif
}

using SupportedStages = tint::EnumSet<Function::PipelineStage>;

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

/// @returns true if @p ty meets the basic function parameter rules (i.e. one of constructible,
///          pointer, handle).
///
/// Note: Does not handle corner cases like if certain capabilities are
/// enabled.
bool IsValidFunctionParamType(const core::type::Type* ty) {
    if (ty->IsConstructible() || ty->IsHandle()) {
        return true;
    }

    if (auto* ptr = ty->As<core::type::Pointer>()) {
        return ptr->AddressSpace() != core::AddressSpace::kHandle;
    }
    return false;
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

/// Helper that walks the members of a struct, called from WalkTypeAndMembers and its helpers
/// @param ctx a context object to pass to the impl function
/// @param str the struct to walk the members of
/// @param impl an impl function to be run, see WalkTypeAndMembers for details
template <typename CTX, typename IMPL>
void WalkStructMembers(CTX& ctx, const core::type::Struct* str, IMPL&& impl) {
    for (auto* member : str->Members()) {
        WalkTypeAndMembers(ctx, member->Type(), member->Attributes(), impl);
    }
}

/// Helper that walks an array's element type, called from WalkTypeAndMembers and its helpers
/// @param ctx a context object to pass to the impl function
/// @param arr the array to walk the element type of
/// @param impl an impl function to be run, see WalkTypeAndMembers for details
template <typename CTX, typename IMPL>
void WalkArrayElements(CTX& ctx, const core::type::Array* arr, IMPL&& impl) {
    tint::Switch(
        arr->ElemType(), [&](const core::type::Struct* s) { WalkStructMembers(ctx, s, impl); },
        [&](const core::type::Array* a) { WalkArrayElements(ctx, a, impl); });
}

/// Helper for walking a type that maybe a struct, calling an impl function for the type and each of
/// its members.
/// @param ctx a context object to pass to the implementation function
/// @param type the type to walk
/// @param attr the attributes for @p type
/// @param impl a function with the signature `void(const core::type::Type*, const IOAttributes&,
///             CTX&)` that is called for each type.
template <typename CTX, typename IMPL>
void WalkTypeAndMembers(CTX& ctx,
                        const core::type::Type* type,
                        const IOAttributes& attr,
                        IMPL&& impl) {
    impl(ctx, type, attr);
    tint::Switch(
        type, [&](const core::type::Struct* s) { WalkStructMembers(ctx, s, impl); },
        [&](const core::type::Array* a) { WalkArrayElements(ctx, a, impl); });
}

/// @returns true if the type or any contained types are atomic
/// @param ty root of the types to walks
bool ContainsAtomic(const core::type::Type* ty) {
    bool found = false;
    WalkTypeAndMembers(found, ty, IOAttributes{},
                       [&](bool& ctx, const core::type::Type* t, const IOAttributes&) {
                           if (t != nullptr && t->Is<core::type::Atomic>()) {
                               ctx = true;
                           }
                       });
    return found;
}

/// The IO direction of an operation.
enum class IODirection : uint8_t { kInput, kOutput, kResource };

/// @returns a human-readable string for an IODirection
std::string_view ToString(IODirection value) {
    switch (value) {
        case IODirection::kInput:
            return "input";
        case IODirection::kOutput:
            return "output";
        case IODirection::kResource:
            return "resource";
    }
    TINT_ICE() << "Unknown enum passed to ToString(IODirection)";
}

IODirection IODirectionFor(AddressSpace address_space) {
    switch (address_space) {
        case AddressSpace::kIn:
            return IODirection::kInput;
        case AddressSpace::kOut:
            return IODirection::kOutput;
        case AddressSpace::kHandle:
            return IODirection::kResource;
        default:
            TINT_ICE() << "Unexpected address_space '" << ToString(address_space)
                       << "' passed to IODirectionFrom()";
    }
}

/// The kind of shader IO being validated.
enum class ShaderIOKind : uint8_t {
    kInputParam,
    kResultValue,
    kModuleScopeVar,
};

/// @returns text describing the shader IO kind for error logging
std::string ToString(ShaderIOKind value) {
    switch (value) {
        case ShaderIOKind::kInputParam:
            return "input param";
        case ShaderIOKind::kResultValue:
            return "return value";
        case ShaderIOKind::kModuleScopeVar:
            return "module scope variable";
    }
    TINT_ICE() << "Unknown enum passed to ToString(ShaderIOKind)";
}

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

/// @returns a human-readable string for an IOAttributeUsage
std::string ToString(IOAttributeUsage value) {
    switch (value) {
        case IOAttributeUsage::kComputeInputUsage:
            return "compute shader input";
        case IOAttributeUsage::kComputeOutputUsage:
            return "compute shader output";
        case IOAttributeUsage::kComputeResourceUsage:
            return "compute shader resource";
        case IOAttributeUsage::kFragmentInputUsage:
            return "fragment shader input";
        case IOAttributeUsage::kFragmentOutputUsage:
            return "fragment shader output";
        case IOAttributeUsage::kFragmentResourceUsage:
            return "fragment shader resource";
        case IOAttributeUsage::kVertexInputUsage:
            return "vertex shader input";
        case IOAttributeUsage::kVertexOutputUsage:
            return "vertex shader output";
        case IOAttributeUsage::kVertexResourceUsage:
            return "vertex shader resourcee";
        case IOAttributeUsage::kUndefinedUsage:
            return "non-entry point usage";
    }
    TINT_ICE() << "Unknown enum passed to ToString(IOAttribute)";
}

/// @returns the IOAttributeUsage for a given Function::PipelineStage + IODirection tuple
IOAttributeUsage IOAttributeUsageFor(Function::PipelineStage stage, IODirection direction) {
    switch (stage) {
        case Function::PipelineStage::kCompute:
            switch (direction) {
                case IODirection::kInput:
                    return IOAttributeUsage::kComputeInputUsage;
                case IODirection::kOutput:
                    return IOAttributeUsage::kComputeOutputUsage;
                case IODirection::kResource:
                    return IOAttributeUsage::kComputeResourceUsage;
            }
            break;
        case Function::PipelineStage::kFragment:
            switch (direction) {
                case IODirection::kInput:
                    return IOAttributeUsage::kFragmentInputUsage;
                case IODirection::kOutput:
                    return IOAttributeUsage::kFragmentOutputUsage;
                case IODirection::kResource:
                    return IOAttributeUsage::kFragmentResourceUsage;
            }
            break;
        case Function::PipelineStage::kVertex:
            switch (direction) {
                case IODirection::kInput:
                    return IOAttributeUsage::kVertexInputUsage;
                case IODirection::kOutput:
                    return IOAttributeUsage::kVertexOutputUsage;
                case IODirection::kResource:
                    return IOAttributeUsage::kVertexResourceUsage;
            }
            break;
        case Function::PipelineStage::kUndefined:
            return IOAttributeUsage::kUndefinedUsage;
    }
    TINT_ICE() << "Unknown IOAttribute usage " << ToString(direction) << " for a "
               << ToString(stage) << " entry point";
}

/// @returns the Function::PipelineStage for an IOAttributeUsage
[[maybe_unused]] Function::PipelineStage PipelineStageFor(IOAttributeUsage usage) {
    switch (usage) {
        case IOAttributeUsage::kComputeInputUsage:
        case IOAttributeUsage::kComputeOutputUsage:
        case IOAttributeUsage::kComputeResourceUsage:
            return Function::PipelineStage::kCompute;
        case IOAttributeUsage::kFragmentInputUsage:
        case IOAttributeUsage::kFragmentOutputUsage:
        case IOAttributeUsage::kFragmentResourceUsage:
            return Function::PipelineStage::kFragment;
        case IOAttributeUsage::kVertexInputUsage:
        case IOAttributeUsage::kVertexOutputUsage:
        case IOAttributeUsage::kVertexResourceUsage:
            return Function::PipelineStage::kVertex;
        case IOAttributeUsage::kUndefinedUsage:
            return Function::PipelineStage::kUndefined;
    }
    TINT_ICE() << "Unknown IOAttribute usage " << ToString(usage);
}

/// @returns the IODirection for an IOAttributeUsage
[[maybe_unused]] IODirection IODirectionFor(IOAttributeUsage usage) {
    switch (usage) {
        case IOAttributeUsage::kComputeInputUsage:
        case IOAttributeUsage::kFragmentInputUsage:
        case IOAttributeUsage::kVertexInputUsage:
            return IODirection::kInput;
        case IOAttributeUsage::kComputeOutputUsage:
        case IOAttributeUsage::kFragmentOutputUsage:
        case IOAttributeUsage::kVertexOutputUsage:
            return IODirection::kOutput;
        case IOAttributeUsage::kComputeResourceUsage:
        case IOAttributeUsage::kFragmentResourceUsage:
        case IOAttributeUsage::kVertexResourceUsage:
        case IOAttributeUsage::kUndefinedUsage:
            return IODirection::kResource;  // Technically it could also be a kInput or kOutput, but
                                            // no validation depends on differentiate between these
                                            // cases currently.
    }
    TINT_ICE() << "Unknown IOAttribute usage " << ToString(usage);
}

/// A BuiltInChecker is the interface used to check that a usage of a builtin attribute meets the
/// basic spec rules, i.e. correct shader stage, data type, and IO direction.
/// It does not test more sophisticated rules like location and builtins being mutually exclusive or
/// that the correct capabilities are enabled.
struct BuiltInChecker {
    /// What combination of stage and IO direction is this builtin legal for
    EnumSet<IOAttributeUsage> valid_usages;

    /// What values for depth_mode are valid for this builtin.
    /// Currently, kUndefined is the only valid option for non-frag_depth
    EnumSet<BuiltinDepthMode> valid_depth_modes =
        EnumSet<BuiltinDepthMode>{BuiltinDepthMode::kUndefined};

    /// Implements logic for checking if the given type is valid or not. Is not a data entry (i.e. a
    /// type or set of types), because types are part of the IR module and created at runtime.
    using TypeCheckFn = bool(const core::type::Type* type, const Capabilities& cap);

    /// @see #TypeCheckFn
    TypeCheckFn* const type_check;

    /// Message for logging if the type check fails. Cannot be easily generated at runtime, because
    /// the type check is a function, not just a data entry.
    const char* type_error;
};

constexpr BuiltInChecker kPointSizeChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->Is<core::type::F32>();
    },
    .type_error = "must be a f32",
};

/// returns true if the number of elements in @p ty is valid for use in clip_distances without
/// Capability::kAllowClipDistancesOnF32.
constexpr auto ClipDistancesElementsCheck = [](const core::type::Type* ty) -> bool {
    const auto elems = ty->Elements();
    return elems.type && elems.type->Is<core::type::F32>() && elems.count <= 8;
};

constexpr BuiltInChecker kClipDistancesChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->Is<core::type::Array>() && ClipDistancesElementsCheck(ty);
    },
    .type_error = "must be an array<f32, N>, where N <= 8",
};

constexpr BuiltInChecker kClipDistancesAllowF32ScalarAndVectorChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ((ty->Is<core::type::Array>() || ty->Is<core::type::Vector>()) &&
                ClipDistancesElementsCheck(ty)) ||
               ty->Is<core::type::F32>();
    },
    .type_error = "must be a f32 or either a vecN<f32> or an array<f32, N>, where N <= 8",
};

constexpr BuiltInChecker kCullDistanceChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->Is<core::type::Array>() && ty->DeepestElement()->Is<core::type::F32>();
    },
    .type_error = "must be an array of f32",
};

constexpr BuiltInChecker kFragDepthChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentOutputUsage},
    .valid_depth_modes =
        EnumSet<BuiltinDepthMode>{BuiltinDepthMode::kUndefined, BuiltinDepthMode::kAny,
                                  BuiltinDepthMode::kGreater, BuiltinDepthMode::kLess},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->Is<core::type::F32>();
    },
    .type_error = "must be a f32",
};

constexpr BuiltInChecker kFrontFacingChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->Is<core::type::Bool>();
    },
    .type_error = "must be a bool",
};

constexpr BuiltInChecker kGlobalInvocationIdChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->IsUnsignedIntegerVector() && ty->Elements().count == 3;
    },
    .type_error = "must be an vec3<u32>",
};

constexpr BuiltInChecker kInstanceIndexChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexInputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kLocalInvocationIdChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->IsUnsignedIntegerVector() && ty->Elements().count == 3;
    },
    .type_error = "must be an vec3<u32>",
};

constexpr BuiltInChecker kLocalInvocationIndexChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kNumSubgroupsChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kNumWorkgroupsChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->IsUnsignedIntegerVector() && ty->Elements().count == 3;
    },
    .type_error = "must be an vec3<u32>",
};

constexpr BuiltInChecker kPositionChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage,
                                              IOAttributeUsage::kFragmentInputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->IsFloatVector() && ty->Elements().count == 4;
    },
    .type_error = "must be an vec4<f32>",
};

constexpr BuiltInChecker kSampleIndexChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kSampleMaskChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage,
                                              IOAttributeUsage::kFragmentOutputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kSubgroupIdChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kSubgroupInvocationIdChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage,
                                              IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kSubgroupSizeChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage,
                                              IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kVertexIndexChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexInputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kWorkgroupIdChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->IsUnsignedIntegerVector() && ty->Elements().count == 3;
    },
    .type_error = "must be an vec3<u32>",
};

constexpr BuiltInChecker kPrimitiveIndexChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kBarycentricCoordChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage},
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->IsFloatVector() && ty->Elements().count == 3;
    },
    .type_error = "must be an vec3<f32>",
};

/// @returns an appropriate BuiltInCheck for @p builtin, ICEs when one isn't defined
const BuiltInChecker& BuiltinCheckerFor(BuiltinValue builtin, const Capabilities& capabilities) {
    switch (builtin) {
        case BuiltinValue::kPointSize:
            return kPointSizeChecker;
        case BuiltinValue::kClipDistances:
            if (capabilities.Contains(Capability::kAllowClipDistancesOnF32ScalarAndVector)) {
                return kClipDistancesAllowF32ScalarAndVectorChecker;
            }
            return kClipDistancesChecker;
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
        case BuiltinValue::kGlobalInvocationIndex:
        case BuiltinValue::kWorkgroupIndex:
            return kLocalInvocationIndexChecker;
        case BuiltinValue::kNumSubgroups:
            return kNumSubgroupsChecker;
        case BuiltinValue::kNumWorkgroups:
            return kNumWorkgroupsChecker;
        case BuiltinValue::kPosition:
            return kPositionChecker;
        case BuiltinValue::kSampleIndex:
            return kSampleIndexChecker;
        case BuiltinValue::kSampleMask:
            return kSampleMaskChecker;
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
        case BuiltinValue::kPrimitiveIndex:
            return kPrimitiveIndexChecker;
        case BuiltinValue::kBarycentricCoord:
            return kBarycentricCoordChecker;
        default:
            TINT_ICE() << builtin << " is does not have a checker defined for it";
    }
}

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
                                                     const Capabilities& cap,
                                                     IOAttributeUsage usage);

    /// The validation function.
    CheckFn* const check;

    /// Implements logic for checking if the given type is valid or not. Is not a data entry (i.e. a
    /// type or set of types), because types are part of the IR module and created at runtime.
    using TypeCheckFn = bool(const core::type::Type* type, const Capabilities& cap);

    /// @see #TypeCheckFn
    TypeCheckFn* const type_check;

    /// Message for logging if the type check fails. Cannot be easily generated at runtime, because
    /// the type check is a function, not just a data entry.
    const char* type_error;
};

constexpr IOAttributeChecker kInvariantChecker{
    .kind = IOAttributeKind::kInvariant,
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage,
                                              IOAttributeUsage::kFragmentInputUsage},
    .valid_io_kinds = EnumSet<ShaderIOKind>{ShaderIOKind::kInputParam, ShaderIOKind::kResultValue,
                                            ShaderIOKind::kModuleScopeVar},
    .check = [](const core::type::Type*,
                const IOAttributes& attr,
                const Capabilities&,
                IOAttributeUsage) -> Result<SuccessType, std::string> {
        if (attr.builtin != BuiltinValue::kPosition) {
            return {"invariant can only decorate a value if it is also decorated with position"};
        }
        return Success;
    },
    .type_check = kPositionChecker.type_check,
    .type_error = kPositionChecker.type_error,
};

constexpr IOAttributeChecker kBuiltinChecker{
    .kind = IOAttributeKind::kBuiltin,
    .valid_usages =
        EnumSet<IOAttributeUsage>{
            IOAttributeUsage::kComputeInputUsage,
            IOAttributeUsage::kComputeOutputUsage,
            IOAttributeUsage::kFragmentInputUsage,
            IOAttributeUsage::kFragmentOutputUsage,
            IOAttributeUsage::kVertexInputUsage,
            IOAttributeUsage::kVertexOutputUsage,
        },
    .valid_io_kinds = EnumSet<ShaderIOKind>{ShaderIOKind::kInputParam, ShaderIOKind::kResultValue,
                                            ShaderIOKind::kModuleScopeVar},
    .check = [](const core::type::Type* ty,
                const IOAttributes& attr,
                const Capabilities& cap,
                IOAttributeUsage usage) -> Result<SuccessType, std::string> {
        if (!attr.builtin.has_value()) {
            return Success;
        }

        const auto builtin = attr.builtin.value();
        const auto& checker = BuiltinCheckerFor(builtin, cap);
        if (usage != IOAttributeUsage::kUndefinedUsage && !checker.valid_usages.Contains(usage)) {
            std::stringstream msg;
            msg << ToString(builtin) << " cannot be used on a " << ToString(usage) << ". ";
            if (checker.valid_usages.Size() == 1) {
                const auto v = *checker.valid_usages.begin();
                msg << "It can only be used on a " << ToString(v) << ".";
            } else {
                msg << "It can only be used on one of " << ToString(checker.valid_usages);
            }
            return msg.str();
        }

        if (!checker.type_check(ty, cap)) {
            std::stringstream msg;
            msg << ToString(builtin) << " " << checker.type_error;
            return msg.str();
        }

        const auto depth_mode = attr.depth_mode.value_or(BuiltinDepthMode::kUndefined);
        if (!checker.valid_depth_modes.Contains(depth_mode)) {
            std::stringstream msg;
            msg << ToString(builtin) << " cannot have a depth mode of " << ToString(depth_mode)
                << ". ";
            if (checker.valid_depth_modes.Size() == 1) {
                const auto v = *checker.valid_depth_modes.begin();
                msg << "It can only be " << ToString(v) << ".";
            } else {
                msg << "It must be one of " << ToString(checker.valid_depth_modes);
            }
            return msg.str();
        }

        if (builtin == BuiltinValue::kPointSize &&
            !cap.Contains(Capability::kAllowPointSizeBuiltin)) {
            return std::string{"use of point_size builtin requires kAllowPointSizeBuiltin"};
        }

        return Success;
    },
    .type_check = [](const core::type::Type*, const Capabilities&) -> bool { return true; },
    .type_error = nullptr,
};

constexpr IOAttributeChecker kColorChecker{
    .kind = IOAttributeKind::kColor,
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage},
    .valid_io_kinds =
        EnumSet<ShaderIOKind>{ShaderIOKind::kInputParam, ShaderIOKind::kModuleScopeVar},
    .check = [](const core::type::Type*, const IOAttributes&, const Capabilities&, IOAttributeUsage)
        -> Result<SuccessType, std::string> { return Success; },
    .type_check = [](const core::type::Type* ty, const Capabilities&) -> bool {
        return ty->IsNumericScalarOrVector();
    },
    .type_error = "must be a numeric scalar or vector",
};

constexpr IOAttributeChecker kInputAttachmentIndexChecker{
    .kind = IOAttributeKind::kInputAttachmentIndex,
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentResourceUsage},
    .valid_io_kinds = EnumSet<ShaderIOKind>{ShaderIOKind::kModuleScopeVar},
    .check = [](const core::type::Type*, const IOAttributes&, const Capabilities&, IOAttributeUsage)
        -> Result<SuccessType, std::string> { return Success; },
    .type_check = [](const core::type::Type* ty, const Capabilities& cap) -> bool {
        return cap.Contains(Capability::kAllowAnyInputAttachmentIndexType) ||
               ty->Is<core::type::InputAttachment>();
    },
    .type_error = "must be an input_attachment",
};

constexpr IOAttributeChecker kDepthModeChecker{
    .kind = IOAttributeKind::kDepthMode,
    .valid_usages = kBuiltinChecker.valid_usages,
    .valid_io_kinds = kBuiltinChecker.valid_io_kinds,
    // kBuiltInChecker does the checking of the depth_mode value for the specific builtin.
    .check = [](const core::type::Type*,
                const IOAttributes& attr,
                const Capabilities&,
                IOAttributeUsage) -> Result<SuccessType, std::string> {
        if (!attr.builtin.has_value()) {
            return {"cannot have a depth_mode without a builtin"};
        }
        return Success;
    },
    .type_check = [](const core::type::Type*, const Capabilities&) -> bool { return true; },
    .type_error = nullptr,
};

// kBlendSrcChecker, kLocationChecker, kInterpolationChecker, and kBindingPointChecker are
// intentionally not implemented

/// @returns all the appropriate IOAttributeCheckers for @p attr
Vector<const IOAttributeChecker*, 4> IOAttributeCheckersFor(const IOAttributes& attr,
                                                            bool skip_builtin) {
    Vector<const IOAttributeChecker*, 4> checkers{};
    if (attr.invariant) {
        checkers.Push(&kInvariantChecker);
    }
    if (!skip_builtin && attr.builtin.has_value()) {
        checkers.Push(&kBuiltinChecker);
    }
    if (attr.color.has_value()) {
        checkers.Push(&kColorChecker);
    }
    if (attr.input_attachment_index.has_value()) {
        checkers.Push(&kInputAttachmentIndexChecker);
    }
    if (attr.depth_mode.has_value()) {
        checkers.Push(&kDepthModeChecker);
    }

    // attr.blend_src, attr.location, attr.interpolation, and attr.binding_point are intentionally
    // skipped, because their rules are not amenable to implementation via IOAttributeChecker.
    return checkers;
}

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

/// State for validating blend_src attributes shared across multiple passes within the same entry
/// point.
struct BlendSrcContext {
    Function::PipelineStage stage;
    Hashmap<uint32_t, const CastableBase*, 4> locations;
    Hashset<uint32_t, 2> blend_srcs;
    const core::type::Type* blend_src_type = nullptr;
    IODirection dir;
};

/// State for validating IO attributes that needs to shared across impl invocations within the same
/// entry point.
struct IOAttributeContext {
    Hashmap<BuiltinValue, uint32_t, 4> input_builtins;
    Hashmap<BuiltinValue, uint32_t, 4> output_builtins;
};

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

    /// Checks that entry points do not use instructions that are not supported by their stage.
    /// Depends on CheckStructuralSoundness() having previously been run
    void CheckStageRestrictedInstructions();

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

    /// Adds an error for the castable base @p base and highlights it in the disassembly
    /// @param base the declaration to add an error for
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const CastableBase* base);

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
                   Capabilities ignore_caps = {},
                   Capabilities allow_caps = {});

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

    /// Validates the subgroup_size attribute for a given function
    /// @param func the function to validate
    void CheckSubgroupSize(const Function* func);

    /// Validates the specific function as a vertex entry point
    /// @param ep the function to validate
    void CheckPositionPresentForVertexOutput(const Function* ep);

    /// Validates the spec rules for IO attribute usage for a function.
    /// @param func the function to validate
    void ValidateIOAttributes(const Function* func);

    /// Implementation for validating the spec rules for IO attribute usage.
    /// @param ctx context object shared between the multiple invocations of this per entry point
    /// @param msg_anchor the object to anchor the error message to
    /// @param ty the data type being decorated by the attributes
    /// @param attr the attributes to test
    /// @param stage the shader stage the builtin is being used
    /// @param dir is value being used as an input or an output
    /// @param io_kind is the type of shader IO object the attribute is attached to
    void ValidateIOAttributesImpl(IOAttributeContext& ctx,
                                  const CastableBase* msg_anchor,
                                  const core::type::Type* ty,
                                  const IOAttributes& attr,
                                  Function::PipelineStage stage,
                                  IODirection dir,
                                  ShaderIOKind io_kind);

    /// Validates that a type is a bool only if it is decorated with @builtin(front_facing).
    /// @param msg_anchor where to attach errors to
    /// @param attr the IO attributes
    /// @param ty the type
    /// @param err error message to log when check fails
    void CheckFrontFacingIfBool(const CastableBase* msg_anchor,
                                const IOAttributes& attr,
                                const core::type::Type* ty,
                                const std::string& err);

    /// Validates that a type is not a bool.
    /// @param msg_anchor where to attach errors to
    /// @param ty the type
    /// @param err error message to log when check fails
    void CheckNotBool(const CastableBase* msg_anchor,
                      const core::type::Type* ty,
                      const std::string& err);

    /// Validates the given instruction
    /// @param inst the instruction to validate
    void CheckInstruction(const Instruction* inst);

    /// Validates the given override
    /// @param o the override to validate
    void CheckOverride(const Override* o);

    /// Validates the given var
    /// @param var the var to validate
    void CheckVar(const Var* var);

    /// Validates annotations related to shader IO
    /// @param msg_anchor where to attach errors to
    /// @param ty type of the value under test
    /// @param binding_point the binding information associated with the value
    /// @param attr IO attributes associated with the values
    /// @param kind the kind Shader IO being performed
    void ValidateShaderIOAnnotations(const CastableBase* msg_anchor,
                                     const core::type::Type* ty,
                                     const std::optional<BindingPoint>& binding_point,
                                     const IOAttributes& attr,
                                     ShaderIOKind kind);

    /// Validates the blend_src attribute for a given type, responsible for traversal of inner types
    /// and checking rules that span across a multiple attribute instances.
    /// @param ctx the blend_src context.
    /// @param target the object that has the struct ty.
    /// @param ty the ty to validate.
    /// @param attr the IO attributes for the object.
    void CheckBlendSrc(BlendSrcContext& ctx,
                       const CastableBase* target,
                       const core::type::Type* ty,
                       const IOAttributes& attr);

    /// Validates the details of a single attribute instance.
    /// @param ctx the blend_src context.
    /// @param target the object that has the struct type.
    /// @param ty the type to validate.
    /// @param attr the IO attributes for the object.
    void CheckBlendSrcImpl(BlendSrcContext& ctx,
                           const CastableBase* target,
                           const core::type::Type* ty,
                           const IOAttributes& attr);

    /// Validates location attributes on entry point IO.
    /// @param locations the map of locations used so far for the current IO direction.
    /// @param target the object that has the location attribute.
    /// @param attr the IO attributes for the object.
    /// @param stage the pipeline stage of the entry point.
    /// @param type the type of the IO object.
    /// @param dir the IO direction (input or output).
    void CheckLocation(Hashmap<uint32_t, const CastableBase*, 4>& locations,
                       const CastableBase* target,
                       const IOAttributes& attr,
                       Function::PipelineStage stage,
                       const core::type::Type* type,
                       IODirection dir);

    /// Validates interpolation attributes on entry point IO.
    /// @param anchor where to attach error messages to.
    /// @param ty the type of the IO object
    /// @param attr the IO attributes of the object.
    /// @param stage the shader stage
    /// @param dir the direction of the IO usage
    void CheckInterpolation(const CastableBase* anchor,
                            const core::type::Type* ty,
                            const IOAttributes& attr,
                            Function::PipelineStage stage,
                            IODirection dir);

    /// Validates binding_point attributes on entry point IO.
    /// @param anchor where to attach error messages to.
    /// @param ty the type of the IO object
    /// @param attr the IO attributes of the object
    /// @param io_kind the type of shader IO object binding point is attached to
    void CheckBindingPoint(const CastableBase* anchor,
                           const core::type::Type* ty,
                           const IOAttributes& attr,
                           const ShaderIOKind& io_kind);

    /// Validates the given let
    /// @param l the let to validate
    void CheckLet(const Let* l);

    /// Validates the given call
    /// @param call the call to validate
    void CheckCall(const Call* call);

    /// Validates the given builtin call
    /// @param call the call to validate
    void CheckBuiltinCall(const BuiltinCall* call);

    /// Validates a core builtin call
    /// @param call the call to validate
    /// @param overload the call intrinsic overload
    void CheckCoreBuiltinCall(const CoreBuiltinCall* call,
                              const core::intrinsic::Overload& overload);

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

    /// @returns true if @p ty and its elements can be loaded
    bool CanLoad(const core::type::Type* ty);

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
    Hashmap<const ir::Instruction*, SupportedStages, 4> stage_restricted_instructions_;
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
    CheckStageRestrictedInstructions();

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

void Validator::CheckStageRestrictedInstructions() {
    if (diagnostics_.ContainsErrors()) {
        return;
    }

    // Check for instructions being used in stages that do not support them.
    for (const auto& i : stage_restricted_instructions_) {
        const auto& inst = i.key;
        const auto& stages = i.value;
        const auto* f = ContainingFunction(inst);
        if (f == nullptr) {
            continue;
        }

        if (f->IsEntryPoint() && !stages.Contains(f->Stage())) {
            AddError(inst) << "cannot be used in a " << f->Stage() << " shader";
        } else {
            for (const Function* ep : ContainingEndPoints(f)) {
                if (!stages.Contains(ep->Stage())) {
                    AddError(inst) << "cannot be used in a " << ep->Stage() << " shader";
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
        if (!all_functions_.Add(func)) {
            AddError(func) << "function " << NameOf(func) << " added to module multiple times";
        }
        scope_stack_.Add(func);
    }

    for (auto& func : mod_.functions) {
        block_to_function_.Add(func->Block(), func);
        CheckFunction(func);
    }
}

diag::Diagnostic& Validator::AddError(const Instruction* inst) {
    auto src = Disassemble().InstructionSource(inst);
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference, so we
        // need to get a new reference to the error diagnostic here.
        return *(diagnostics_.end() - 2);
    }
    return diag;
}

diag::Diagnostic& Validator::AddError(const Instruction* inst, size_t idx) {
    auto src =
        Disassemble().OperandSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference, so we
        // need to get a new reference to the error diagnostic here.
        return *(diagnostics_.end() - 2);
    }
    return diag;
}

diag::Diagnostic& Validator::AddResultError(const Instruction* inst, size_t idx) {
    auto src =
        Disassemble().ResultSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference, so we
        // need to get a new reference to the error diagnostic here.
        return *(diagnostics_.end() - 2);
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

diag::Diagnostic& Validator::AddError(const CastableBase* base) {
    diag::Diagnostic* diag = nullptr;
    tint::Switch(
        base,  //
        [&](const Block* block) { diag = &AddError(block); },
        [&](const BlockParam* param) { diag = &AddError(param); },
        [&](const Function* fn) { diag = &AddError(fn); },
        [&](const FunctionParam* param) { diag = &AddError(param); },
        [&](const Instruction* inst) { diag = &AddError(inst); },
        [&](const InstructionResult* res) { diag = &AddError(res); });
    TINT_ASSERT(diag);
    return *diag;
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

    if (!inst->Is<core::ir::Call>() && result->Type()->Is<core::type::Void>()) {
        AddResultError(inst, idx) << "result type cannot be void";
        return false;
    }

    if (inst->Is<core::ir::ControlInstruction>()) {
        if (result->Type()->Is<core::type::Pointer>()) {
            AddResultError(inst, idx) << "result type cannot be a pointer";
            return false;
        }
        if (!result->Type()->IsConstructible()) {
            AddResultError(inst, idx) << "result type must be constructable";
            return false;
        }
    }

    if (result->Type()->Is<core::type::Void>() && mod_.NameOf(result)) {
        AddResultError(inst, idx) << "void results must not have names";
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
    Hashset<const InstructionResult*, 4> seen_instruction_results;
    for (size_t i = 0; i < inst->Results().Length(); i++) {
        if (DAWN_UNLIKELY(!CheckResult(inst, i))) {
            passed = false;
        }

        if (!seen_instruction_results.Add(inst->Result(i))) {
            AddResultError(inst, i) << "result was seen previously as a result";
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
                          Capabilities ignore_caps,
                          Capabilities allow_caps) {
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

    AddressSpace addrspace = AddressSpace::kUndefined;
    if (auto* mv = root->As<core::type::MemoryView>()) {
        addrspace = mv->AddressSpace();
    }

    auto visit = [&](const core::type::Type* type) {
        if (type->IsAbstract()) {
            diag() << "abstracts are not permitted";
            return false;
        }

        return tint::Switch(
            type,
            [&](const core::type::Struct* str) {
                uint32_t cur_offset = 0;
                for (auto* member : str->Members()) {
                    if (member->Type()->Is<core::type::Void>()) {
                        diag() << "struct member " << member->Index() << " cannot have void type";
                        return false;
                    }

                    if (!capabilities_.Contains(Capability::kMslAllowEntryPointInterface)) {
                        if (member->Type()->Is<core::type::Pointer>()) {
                            diag() << "struct member " << member->Index()
                                   << " cannot be a pointer type";
                            return false;
                        }

                        if (member->Type()->Is<core::type::Texture>()) {
                            diag() << "struct member " << member->Index()
                                   << " cannot be a texture type";
                            return false;
                        }

                        if (member->Type()->Is<core::type::Sampler>()) {
                            diag() << "struct member " << member->Index()
                                   << " cannot be a sampler type";
                            return false;
                        }
                    }

                    if (auto* arr = member->Type()->As<core::type::Array>();
                        arr && arr->Count()->Is<core::type::RuntimeArrayCount>()) {
                        if (member != str->Members().Back()) {
                            diag() << "runtime-sized arrays can only be the last member of a "
                                      "struct";
                            return false;
                        }
                    }

                    if (member->Align() == 0) {
                        diag() << "struct member must not have an alignment of 0";
                        return false;
                    }
                    if (member->Type()->Align() == 0) {
                        diag() << "struct member type must not have an alignment of 0";
                        return false;
                    }

                    if (!capabilities_.Contains(Capability::kAllowStructMatrixDecorations)) {
                        if (member->RowMajor()) {
                            diag() << "Row major annotation not allowed on structures";
                            return false;
                        }
                        if (member->HasMatrixStride()) {
                            diag() << "Matrix stride annotation not allowed on structures";
                            return false;
                        }
                    }

                    // TODO(448608979): Remove guard once updated to handle RowMajor correctly
                    if (!member->RowMajor()) {
                        if (member->Size() < member->Type()->Size()) {
                            diag() << "struct member " << member->Index()
                                   << " with size=" << member->Size()
                                   << " must be at least as large as the type with size "
                                   << member->Type()->Size();
                            return false;
                        }

                        if (member->Align() % member->Type()->Align() != 0) {
                            diag() << "struct member alignment (" << member->Align()
                                   << ") must be divisible by type alignment ("
                                   << member->Type()->Align() << ")";
                            return false;
                        }
                    }

                    auto padding = member->Offset() - cur_offset;
                    if (padding >= internal_limits::kMaxStructMemberPadding) {
                        diag() << "struct member padding (" << padding
                               << ") is larger then the max ("
                               << internal_limits::kMaxStructMemberPadding << ")";
                        return false;
                    }
                    cur_offset += padding + member->MinimumRequiredSize();
                }
                if (str->Size() < cur_offset) {
                    diag() << "struct size (" << str->Size()
                           << ") is smaller than the end of the last member (" << cur_offset << ")";
                    return false;
                }

                auto padding = str->Size() - cur_offset;
                if (padding >= internal_limits::kMaxStructMemberPadding) {
                    diag() << "struct padding (" << padding << ") is larger then the max ("
                           << internal_limits::kMaxStructMemberPadding << ")";
                    return false;
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

                if (ptr->AddressSpace() == AddressSpace::kWorkgroup) {
                    if (ptr->Access() != core::Access::kReadWrite) {
                        diag() << "workgroup pointers must be read_write access";
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

                if (ptr->AddressSpace() == core::AddressSpace::kImmediate) {
                    if (ptr->Access() != core::Access::kRead) {
                        diag() << "immediate pointers must be read access";
                        return false;
                    }
                }

                if (ptr->StoreType()->Is<core::type::Pointer>()) {
                    diag() << "pointers to pointers are not allowed";
                    return false;
                }
                return true;
            },
            [&](const core::type::U64*) {
                // u64 types are guarded by the Allow64BitIntegers capability.
                if (!capabilities_.Contains(Capability::kAllow64BitIntegers) &&
                    !allow_caps.Contains(Capability::kAllow64BitIntegers)) {
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
            [&](const core::type::U16*) {
                // u16 types are guarded by the Allow16BitIntegers capability.
                if (!capabilities_.Contains(Capability::kAllow16BitIntegers)) {
                    diag() << "16-bit integer types are not permitted";
                    return false;
                }
                return true;
            },
            [&](const core::type::Array* arr) {
                if (!arr->ElemType()->HasCreationFixedFootprint()) {
                    diag() << "array elements, " << NameOf(type)
                           << ", must have creation-fixed footprint";
                    return false;
                }
                if (auto* count = arr->Count()->As<core::type::ConstantArrayCount>()) {
                    if (count->value == 0) {
                        diag() << "array requires a constant array size > 0";
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
            [&](const core::type::Atomic* a) {
                // Prior to lowering we allow for atomic operations on vec2u to support the
                // AtomicVec2UMinMax feature.
                if (auto* vec = a->Type()->As<core::type::Vector>()) {
                    if (vec->Width() == 2 && vec->Type()->Is<core::type::U32>()) {
                        return true;
                    }
                }

                if (!a->Type()->IsAnyOf<core::type::I32, core::type::U32, core::type::U64>()) {
                    diag() << "atomic subtype must be i32, u32 or u64 type is "
                           << NameOf(a->Type());
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
                if (!(addrspace == AddressSpace::kUndefined ||
                      addrspace == AddressSpace::kFunction)) {
                    diag() << "invalid address space for subgroup matrix : " << addrspace;
                    return false;
                }
                return true;
            },
            [&](const core::type::BindingArray* t) {
                if (!t->Count()->Is<core::type::ConstantArrayCount>()) {
                    diag() << "binding_array count must be a constant expression";
                    return false;
                }

                auto count = t->Count()->As<core::type::ConstantArrayCount>()->value;
                if (count == 0) {
                    diag() << "binding array requires a constant array size > 0";
                    return false;
                }

                if (!(addrspace == AddressSpace::kUndefined ||
                      addrspace == AddressSpace::kHandle) &&
                    !capabilities_.Contains(Capability::kMslAllowEntryPointInterface)) {
                    diag() << "invalid address space for binding_array : " << addrspace;
                    return false;
                }

                if (!capabilities_.Contains(Capability::kAllowNonCoreTypes)) {
                    if (!t->ElemType()->Is<core::type::SampledTexture>()) {
                        diag() << "binding_array element type must be a sampled texture type";
                        return false;
                    }
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

        // Visit the elements of a composite type.
        auto type_count = ty->Elements();
        if (type_count.type) {
            // Every element has the same type (e.g. array, vector, matrix, ...), so validate that
            // type once if it has not been seen before.
            if (seen.Add(type_count.type)) {
                stack.Push(type_count.type);
            }
        } else {
            // Different elements have different types (e.g. a struct), so we need to validate each
            // of them if they have not been seen before.
            for (uint32_t i = 0; i < type_count.count; i++) {
                if (auto* subtype = ty->Element(i); subtype && seen.Add(subtype)) {
                    stack.Push(subtype);
                }
            }
        }
    }
}

void Validator::CheckRootBlock(const Block* blk) {
    block_stack_.Push(blk);
    TINT_DEFER(block_stack_.Pop());

    Hashset<const core::ir::Value*, 8> pipeline_evaluatable{};

    auto add_evaluatable = [&](const Instruction* inst, const bool is_creatable) {
        if (auto* res = inst->Result(0); res != nullptr && is_creatable) {
            pipeline_evaluatable.Add(res);
        }
    };

    for (auto* inst : *blk) {
        if (inst->Block() != blk) {
            AddError(inst) << "instruction in root block does not have root block as parent";
            continue;
        }

        auto is_pipeline_creatable = true;
        for (auto* op : inst->Operands()) {
            if (!op) {
                continue;
            }
            if (op->Is<core::ir::Constant>()) {
                continue;
            }
            if (pipeline_evaluatable.Contains(op)) {
                continue;
            }
            is_pipeline_creatable = false;
            break;
        }

        if (!is_pipeline_creatable) {
            AddError(inst) << "instruction is not evaluatable at pipeline creation time";
        }

        tint::Switch(
            inst,  //
            [&](const core::ir::Override* o) {
                if (capabilities_.Contains(Capability::kAllowOverrides)) {
                    CheckInstruction(o);
                    add_evaluatable(o, is_pipeline_creatable);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            },
            [&](const core::ir::Var* var) { CheckInstruction(var); },
            [&](const core::ir::Let* let) {
                if (capabilities_.Contains(Capability::kAllowModuleScopeLets)) {
                    CheckInstruction(let);
                    add_evaluatable(let, is_pipeline_creatable);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            },
            [&](const core::ir::Construct* c) {
                if (capabilities_.Contains(Capability::kAllowModuleScopeLets) ||
                    capabilities_.Contains(Capability::kAllowOverrides)) {
                    CheckInstruction(c);
                    CheckOnlyUsedInRootBlock(inst);
                    add_evaluatable(c, is_pipeline_creatable);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            },
            [&](Default) {
                // Note, this validation around kAllowOverrides is looser than it could be. There
                // are only certain expressions and builtins which can be used in an override, which
                // currently isn't checked.
                if (capabilities_.Contains(Capability::kAllowOverrides) &&
                    inst->IsAnyOf<core::ir::Unary, core::ir::Binary, core::ir::BuiltinCall,
                                  core::ir::Convert, core::ir::Swizzle, core::ir::Access,
                                  core::ir::ConstExprIf>()) {
                    CheckInstruction(inst);
                    // If overrides are allowed we can have certain regular instructions in the root
                    // block, with the caveat that those instructions can _only_ be used in the root
                    // block.
                    CheckOnlyUsedInRootBlock(inst);
                    add_evaluatable(inst, is_pipeline_creatable);
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
            auto ptr_ty = param->Type()->As<core::type::Pointer>();
            bool allowed_ptr_to_handle =
                capabilities_.Contains(Capability::kAllowPointerToHandle) && ptr_ty != nullptr &&
                ptr_ty->StoreType()->IsHandle();

            auto struct_ty = param->Type()->As<core::type::Struct>();
            if (!allowed_ptr_to_handle &&
                (!capabilities_.Contains(Capability::kMslAllowEntryPointInterface) ||
                 (struct_ty == nullptr) ||
                 struct_ty->Members().Any([](const core::type::StructMember* m) {
                     return !IsValidFunctionParamType(m->Type());
                 }))) {
                AddError(param) << "function parameter type, " << NameOf(param->Type())
                                << ", must be constructible, a pointer, or a handle";
            }
        }
        if (func->IsEntryPoint() &&
            !capabilities_.Contains(Capability::kMslAllowEntryPointInterface)) {
            if (param->Type()->Is<core::type::Pointer>()) {
                AddError(param) << "entry point parameters cannot be pointers";
            }
        }

        if (func->IsFragment()) {
            WalkTypeAndMembers(param, param->Type(), param->Attributes(),
                               [this](const auto* p, const auto* t, const auto& a) {
                                   CheckFrontFacingIfBool(
                                       p, a, t,
                                       "fragment entry point params can only be a bool if "
                                       "decorated with @builtin(front_facing)");
                               });
        } else if (func->IsEntryPoint()) {
            WalkTypeAndMembers(
                param, param->Type(), param->Attributes(),
                [this](const auto* p, const auto* t, const auto&) {
                    CheckNotBool(p, t,
                                 "entry point params can only be a bool for fragment shaders");
                });
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

        if (address_space == AddressSpace::kPixelLocal) {
            if (!mv->StoreType()->Is<core::type::Struct>()) {
                AddError(param) << "pixel_local param must be of type struct";
            }
        }

        if (func->IsEntryPoint()) {
            ValidateShaderIOAnnotations(param, param->Type(), param->BindingPoint(),
                                        param->Attributes(), ShaderIOKind::kInputParam);
        } else {
            if (param->BindingPoint().has_value()) {
                AddError(param)
                    << "input param to non-entry point function has a binding point set";
            }

            if (param->Builtin().has_value()) {
                AddError(param) << "builtins can only be decorated on entry point params";
            }
        }

        if (!capabilities_.Contains(Capability::kMslAllowEntryPointInterface) &&
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

    // void needs to be filtered out, since it isn't constructible, but used in the IR when no
    // return is specified.
    if (DAWN_UNLIKELY(!func->ReturnType()->Is<core::type::Void>() &&
                      !func->ReturnType()->IsConstructible())) {
        AddError(func) << "function return type must be constructible";
    }

    ValidateIOAttributes(func);

    if (func->IsEntryPoint()) {
        if (DAWN_UNLIKELY(mod_.NameOf(func).Name().empty())) {
            AddError(func) << "entry points must have names";
        }
    }

    CheckWorkgroupSize(func);

    CheckSubgroupSize(func);

    if (func->Stage() == Function::PipelineStage::kCompute) {
        if (DAWN_UNLIKELY(func->ReturnType() && !func->ReturnType()->Is<core::type::Void>())) {
            AddError(func) << "compute entry point must not have a return type, found "
                           << NameOf(func->ReturnType());
        }
    }

    if (func->IsEntryPoint()) {
        ValidateShaderIOAnnotations(func, func->ReturnType(), std::nullopt,
                                    func->ReturnAttributes(), ShaderIOKind::kResultValue);

        WalkTypeAndMembers(
            func, func->ReturnType(), func->ReturnAttributes(),
            [this](const Function* f, const core::type::Type* t, const IOAttributes&) {
                CheckNotBool(f, t, "entry point returns can not be 'bool'");
            });

        Hashset<BindingPoint, 4> binding_points{};
        const Var* user_declared_immediate = nullptr;

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

            auto address_space = mv->AddressSpace();
            switch (address_space) {
                case AddressSpace::kImmediate:
                    if (user_declared_immediate) {
                        AddError(var)
                            << "multiple user-declared immediate data variables referenced "
                               "by entry point "
                            << NameOf(func);
                    }
                    user_declared_immediate = var;
                    continue;
                case AddressSpace::kWorkgroup:
                    if (!func->IsCompute()) {
                        AddError(var) << "workgroup variable cannot be used in a " << func->Stage()
                                      << " shader";
                    }
                    continue;
                case AddressSpace::kPixelLocal:
                    if (!func->IsFragment()) {
                        AddError(var) << "pixel_local variable cannot be used in a "
                                      << func->Stage() << " shader";
                    }
                    continue;
                case AddressSpace::kIn:
                case AddressSpace::kOut:
                    break;
                default:
                    continue;
            }

            if (func->IsFragment() && address_space == AddressSpace::kIn) {
                WalkTypeAndMembers(
                    var, ty, attr, [this](const auto* v, const auto* t, const auto& a) {
                        CheckFrontFacingIfBool(
                            v, a, t,
                            "input address space values referenced by fragment shaders "
                            "can only be 'bool' if decorated with "
                            "@builtin(front_facing)");
                    });
            } else {
                WalkTypeAndMembers(
                    var, ty, attr, [this](const auto* v, const auto* t, const auto&) {
                        CheckNotBool(
                            v, t,
                            "IO address space values referenced by shader entry points can "
                            "only be 'bool' if in the input space, used only by fragment "
                            "shaders and decorated with @builtin(front_facing)");
                    });
            }
        }
    }

    if (func->IsVertex()) {
        CheckPositionPresentForVertexOutput(func);
    }

    QueueBlock(func->Block());
    ProcessTasks();
}

void Validator::ValidateIOAttributes(const Function* func) {
    const auto stage = func->Stage();
    struct Task {
        const CastableBase* anchor;
        const core::type::Type* type;
        const IOAttributes& attr;
        IODirection dir;
        ShaderIOKind io_kind;
    };
    Vector<Task, 16> tasks;

    // Gather parameters.
    for (auto* param : func->Params()) {
        tasks.Push({param, param->Type(), param->Attributes(), IODirection::kInput,
                    ShaderIOKind::kInputParam});
    }

    // Gather return value.
    tasks.Push({func, func->ReturnType(), func->ReturnAttributes(), IODirection::kOutput,
                ShaderIOKind::kResultValue});

    // Gather referenced module variables.
    for (auto* var : referenced_module_vars_.TransitiveReferences(func)) {
        auto* mv = var->Result()->Type()->As<core::type::MemoryView>();
        if (mv == nullptr) {
            continue;
        }
        if (mv->AddressSpace() == AddressSpace::kIn || mv->AddressSpace() == AddressSpace::kOut ||
            mv->AddressSpace() == AddressSpace::kHandle) {
            tasks.Push({var, mv->StoreType(), var->Attributes(), IODirectionFor(mv->AddressSpace()),
                        ShaderIOKind::kModuleScopeVar});
        }
    }

    if (stage != Function::PipelineStage::kUndefined) {
        // Shared context for blend_src and location validation
        BlendSrcContext input_ctx{func->Stage(), {}, {}, nullptr, IODirection::kInput};
        BlendSrcContext output_ctx{func->Stage(), {}, {}, nullptr, IODirection::kOutput};

        // First pass: pre-populate location hashes for blend_src.
        for (const auto& task : tasks) {
            auto& ctx = task.dir == IODirection::kInput ? input_ctx : output_ctx;
            WalkTypeAndMembers(
                ctx, task.type, task.attr,
                [task](BlendSrcContext& c, const core::type::Type*, const IOAttributes& a) {
                    if (a.blend_src.has_value() && a.location.has_value()) {
                        c.locations.Add(a.location.value(), task.anchor);
                    }
                });
        }

        // Second pass: validate blend_src usages.
        for (const auto& task : tasks) {
            auto& ctx = task.dir == IODirection::kInput ? input_ctx : output_ctx;
            CheckBlendSrc(ctx, task.anchor, task.type, task.attr);
        }

        if (!output_ctx.blend_srcs.IsEmpty()) {
            if (output_ctx.blend_srcs.Count() != 2) {
                AddError(func) << "if any @blend_src is used on an output, then @blend_src(0) and "
                                  "@blend_src(1) must be used";
            }
        }

        // Third pass: validate all non-blend_src location usages.
        for (const auto& task : tasks) {
            if (task.dir == IODirection::kInput) {
                CheckLocation(input_ctx.locations, task.anchor, task.attr, func->Stage(), task.type,
                              task.dir);
            } else if (task.dir == IODirection::kOutput) {
                CheckLocation(output_ctx.locations, task.anchor, task.attr, func->Stage(),
                              task.type, task.dir);
            }
        }
    }

    // Validate all the interpolation usages.
    for (const auto& task : tasks) {
        CheckInterpolation(task.anchor, task.type, task.attr, stage, task.dir);
    }

    if (stage != Function::PipelineStage::kUndefined) {
        // Validate all the binding_point usages, and ensure things that require binding_point have
        // them.
        for (const auto& task : tasks) {
            CheckBindingPoint(task.anchor, task.type, task.attr, task.io_kind);
        }
    }

    IOAttributeContext impl_ctx{.input_builtins = {}, .output_builtins = {}};
    // Validate all remaining attributes on IO objects
    for (const auto& task : tasks) {
        ValidateIOAttributesImpl(impl_ctx, task.anchor, task.type, task.attr, stage, task.dir,
                                 task.io_kind);
    }
}

void Validator::ValidateIOAttributesImpl(IOAttributeContext& ctx,
                                         const CastableBase* msg_anchor,
                                         const core::type::Type* ty,
                                         const IOAttributes& attr,
                                         Function::PipelineStage stage,
                                         IODirection dir,
                                         ShaderIOKind io_kind) {
    bool skip_builtins = capabilities_.Contains(Capability::kLoosenValidationForShaderIO) &&
                         io_kind == ShaderIOKind::kModuleScopeVar;
    const IOAttributeUsage usage = IOAttributeUsageFor(stage, dir);
    WalkTypeAndMembers(
        *this, ty, attr,
        [&ctx, msg_anchor, usage, io_kind, skip_builtins, dir](
            Validator& v, const core::type::Type* t, const IOAttributes& a) {
            const auto checkers = IOAttributeCheckersFor(a, skip_builtins);
            if (checkers.IsEmpty()) {
                return;
            }

            if (a.builtin.has_value() && !skip_builtins) {
                const auto& builtin = a.builtin.value();

                uint32_t count = 0;
                switch (dir) {
                    case IODirection::kInput:
                        count = ++(ctx.input_builtins.GetOrAddZeroEntry(builtin).value);
                        break;
                    case IODirection::kOutput:
                        count = ++(ctx.output_builtins.GetOrAddZeroEntry(builtin).value);
                        break;
                    default:
                        // This shouldn't ever happen, but this will get caught later in the
                        // checker, so just ignoring
                        break;
                }
                if (v.capabilities_.Contains(Capability::kAllowClipDistancesOnF32ScalarAndVector) &&
                    builtin == BuiltinValue::kClipDistances) {
                    if (count > 2) {
                        v.AddError(msg_anchor)
                            << "too many instances of builtin 'clip_distances' on entry point "
                            << ToString(dir)
                            << ", only two allowed with 'kAllowClipDistancesOnF32ScalarAndVector' "
                               "capability enabled";
                    }
                } else {
                    if (count > 1) {
                        v.AddError(msg_anchor)
                            << "duplicate instance of builtin '" << ToString(builtin)
                            << "' on entry point " << ToString(dir)
                            << ", must be unique per entry point i/o direction";
                    }
                }
            }

            auto failed = tint::Hashset<const IOAttributeChecker*, 4>();

            if (usage != IOAttributeUsage::kUndefinedUsage) {
                for (const auto* checker : checkers) {
                    if (!checker->valid_usages.Contains(usage)) {
                        failed.Add(checker);

                        std::stringstream msg;
                        msg << ToString(checker->kind) << " IO attributes cannot be declared for a "
                            << ToString(usage) << ". ";
                        if (checker->valid_usages.Size() == 1) {
                            const auto& u = *checker->valid_usages.begin();
                            msg << "They can only be used for a " << ToString(u) << ".";
                        } else {
                            msg << "They can only be used for " << ToString(checker->valid_usages);
                        }
                        v.AddError(msg_anchor) << msg.str();
                    }
                }
            }

            for (const auto& checker : checkers) {
                if (failed.Contains(checker)) {
                    continue;
                }

                if (!checker->valid_io_kinds.Contains(io_kind)) {
                    failed.Add(checker);

                    std::stringstream msg;
                    msg << ToString(checker->kind) << " IO attributes cannot be declared on a "
                        << ToString(io_kind) << ". ";
                    if (checker->valid_io_kinds.Size() == 1) {
                        const auto& k = *checker->valid_io_kinds.begin();
                        msg << "They can only be used on a " << ToString(k) << ".";
                    } else {
                        msg << "They can only be used on " << ToString(checker->valid_io_kinds);
                    }
                    v.AddError(msg_anchor) << msg.str();
                }
            }

            for (const auto& checker : checkers) {
                if (failed.Contains(checker)) {
                    continue;
                }

                if (!checker->type_check(t, v.capabilities_)) {
                    failed.Add(checker);
                    v.AddError(msg_anchor) << ToString(checker->kind) << " " << checker->type_error;
                }
            }

            for (const auto& checker : checkers) {
                if (failed.Contains(checker)) {
                    continue;
                }

                if (auto res = checker->check(t, a, v.capabilities_, usage); res != Success) {
                    failed.Add(checker);
                    v.AddError(msg_anchor) << res.Failure();
                }
            }
        });
}

void Validator::CheckFrontFacingIfBool(const CastableBase* msg_anchor,
                                       const IOAttributes& attr,
                                       const core::type::Type* ty,
                                       const std::string& err) {
    if (ty->Is<core::type::Bool>() && attr.builtin != BuiltinValue::kFrontFacing) {
        AddError(msg_anchor) << err;
    }
}

void Validator::CheckNotBool(const CastableBase* msg_anchor,
                             const core::type::Type* ty,
                             const std::string& err) {
    if (ty->Is<core::type::Bool>()) {
        AddError(msg_anchor) << err;
    }
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

    uint64_t total_size = 1;

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
            total_size *= c->Value()->ValueAs<uint64_t>();

            constexpr uint64_t kMaxGridSize = 0xffffffff;
            if (total_size > kMaxGridSize) {
                AddError(func) << "workgroup grid size cannot exceed 0x" << std::hex
                               << kMaxGridSize;
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

            // Since above, it is already checked if the value is in the root block, it is assumed
            // to be pipeline creatable here, i.e. const/override or derived from consts and
            // overrides.
            // If that is not true, that indicates an issue in CheckRootBlock().
            continue;
        }

        AddError(func) << "@workgroup_size must be an InstructionResult or a Constant";
    }
}

void Validator::CheckSubgroupSize(const Function* func) {
    // @subgroup_size is optional
    if (!func->SubgroupSize().has_value()) {
        return;
    }

    if (!func->IsCompute()) {
        AddError(func) << "@subgroup_size only valid on compute entry point";
        return;
    }

    auto subgroup_size = func->SubgroupSize().value();
    if (subgroup_size == nullptr) {
        AddError(func) << "a @subgroup_size param must have a value";
        return;
    }

    if (!subgroup_size->Type()) {
        AddError(func) << "a @subgroup_size param is missing a type";
        return;
    }

    auto* ty = subgroup_size->Type();
    if (!ty->IsAnyOf<core::type::I32, core::type::U32>()) {
        AddError(func) << "@subgroup_size param must be an 'i32' or 'u32', received " << NameOf(ty);
        return;
    }

    if (auto* c = subgroup_size->As<ir::Constant>()) {
        int64_t value = c->Value()->ValueAs<int64_t>();
        if (value <= 0) {
            AddError(func) << "@subgroup_size param must be greater than 0";
            return;
        }

        if (!IsPowerOfTwo<int64_t>(value)) {
            AddError(func) << "@subgroup_size param must be a power of 2";
            return;
        }

        return;
    }

    if (!capabilities_.Contains(Capability::kAllowOverrides)) {
        AddError(func) << "@subgroup_size param is not a constant value, and IR capability "
                          "'kAllowOverrides' is not set";
        return;
    }

    if (auto* r = subgroup_size->As<ir::InstructionResult>()) {
        if (!r->Instruction()) {
            AddError(func) << "instruction for @subgroup_size param is not defined";
            return;
        }

        if (r->Instruction()->Block() != mod_.root_block) {
            AddError(func) << "@subgroup_size param defined by non-module scope value";
            return;
        }

        if (r->Instruction()->Is<core::ir::Override>()) {
            return;
        }
    }

    AddError(func) << "@subgroup_size must be an InstructionResult or a Constant";
}

void Validator::CheckPositionPresentForVertexOutput(const Function* ep) {
    if (IsPositionPresent(ep->ReturnAttributes(), ep->ReturnType())) {
        return;
    }

    for (const auto& var : referenced_module_vars_.TransitiveReferences(ep)) {
        const auto* ty = var->Result()->Type()->UnwrapPtrOrRef();
        if (!ty) {
            continue;
        }

        const auto attr = var->Attributes();
        if (IsPositionPresent(attr, ty)) {
            return;
        }
    }
    AddError(ep) << "position must be declared for vertex entry point output";
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

            if (param->Type()->Is<core::type::Void>()) {
                AddError(param) << "block parameter type cannot be void";
            }

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
    if (diagnostics_.ContainsErrors()) {
        return;
    }

    tasks_.Push([this, inst] {
        // Tasks are processed LIFO, so push the next instruction to the stack before checking the
        // current instruction, which may need to add more blocks to the stack itself.
        if (inst->next) {
            QueueInstructions(inst->next);
        }
        CheckInstruction(inst);
    });
}

void Validator::CheckInstruction(const Instruction* inst) {
    visited_instructions_.Add(inst);
    if (!inst->Alive()) {
        AddError(inst) << "destroyed instruction found in instruction list";
        return;
    }

    Capabilities allowed_types{};
    if (auto* call = inst->As<core::ir::CoreBuiltinCall>();
        call && call->Func() == core::BuiltinFn::kBitcast) {
        allowed_types.Add(Capability::kAllow64BitIntegers);
    }

    auto results = inst->Results();
    for (size_t i = 0; i < results.Length(); ++i) {
        auto* res = results[i];
        if (!res) {
            continue;
        }

        CheckType(
            res->Type(), [&]() -> diag::Diagnostic& { return AddResultError(inst, i); }, {},
            allowed_types);
    }

    auto ops = inst->Operands();
    for (size_t i = 0; i < ops.Length(); ++i) {
        auto* op = ops[i];
        if (!op) {
            continue;
        }

        CheckType(
            op->Type(), [&]() -> diag::Diagnostic& { return AddError(inst, i); }, {},
            allowed_types);
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
        if (!capabilities_.Contains(Capability::kMslAllowEntryPointInterface) ||
            mv->AddressSpace() != AddressSpace::kPrivate) {
            AddError(var) << "vars in a function scope must be in the 'function' address space";
            return;
        }
    }

    if (mv->AddressSpace() != AddressSpace::kStorage &&
        mv->AddressSpace() != AddressSpace::kHandle) {
        if (!capabilities_.Contains(Capability::kMslAllowEntryPointInterface)) {
            if (!mv->StoreType()->HasFixedFootprint()) {
                AddResultError(var, 0) << "vars not in the 'storage' or 'handle' address spaces "
                                          "must have a fixed footprint";
                return;
            }
        }
    }

    if (ContainsAtomic(mv->StoreType())) {
        if (mv->AddressSpace() == AddressSpace::kStorage) {
            if (mv->Access() != core::Access::kReadWrite) {
                AddError(var)
                    << "atomic variables in 'storage' address space must have 'read_write' "
                       "access mode";
                return;
            }
        } else if (mv->AddressSpace() != AddressSpace::kWorkgroup) {
            AddError(var)
                << "atomic variables must be in the 'workgroup' or 'storage' address space";
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

    CheckBindingPoint(var, var->Result(0)->Type(), var->Attributes(),
                      ShaderIOKind::kModuleScopeVar);

    if (var->Block() == mod_.root_block && mv->AddressSpace() == AddressSpace::kFunction) {
        AddError(var) << "vars in the 'function' address space must be in a function scope";
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
        if (mv->Access() != core::Access::kReadWrite && mv->Access() != core::Access::kRead) {
            AddError(var)
                << "vars in the 'storage' address space must have access 'read' or 'read-write'";
            return;
        }
    }

    if (mv->AddressSpace() == AddressSpace::kUniform) {
        if (!(mv->StoreType()->IsConstructible() || mv->StoreType()->Is<core::type::Buffer>()) ||
            !mv->StoreType()->IsHostShareable()) {
            AddError(var) << "vars in the 'uniform' address space must be host-shareable and "
                             "constructible or a buffer";
            return;
        }
    }

    if (mv->AddressSpace() == AddressSpace::kImmediate) {
        if (mv->StoreType() && !mv->StoreType()->IsHostShareable()) {
            AddError(var) << "vars in the 'immediate' address space must be host-shareable";
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
        if (mv->AddressSpace() == AddressSpace::kIn || mv->AddressSpace() == AddressSpace::kOut) {
            ValidateShaderIOAnnotations(var, var->Result()->Type(), var->BindingPoint(),
                                        var->Attributes(), ShaderIOKind::kModuleScopeVar);
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

void Validator::CheckBlendSrc(BlendSrcContext& ctx,
                              const CastableBase* target,
                              const core::type::Type* ty,
                              const IOAttributes& attr) {
    if (attr.blend_src.has_value()) {
        if (!capabilities_.Contains(Capability::kLoosenValidationForShaderIO)) {
            AddError(target) << "blend_src cannot be used on non-struct-member types";
        }
        CheckBlendSrcImpl(ctx, target, ty, attr);
    }

    if (auto* s = ty->As<core::type::Struct>()) {
        if (s->Members().Any([](auto* m) { return m->Attributes().blend_src.has_value(); })) {
            auto location_count = 0u;
            for (const auto* mem : s->Members()) {
                auto& mem_attr = mem->Attributes();
                if (mem_attr.location.has_value()) {
                    location_count++;
                }
                CheckBlendSrcImpl(ctx, target, mem->Type(), mem_attr);
            }

            if (location_count != 2) {
                AddError(target)
                    << "structs with blend_src members must have exactly 2 members with "
                       "location annotations";
            }
            return;
        }
    }

    // Reject blend_src on nested members
    if (!capabilities_.Contains(Capability::kLoosenValidationForShaderIO)) {
        WalkTypeAndMembers(
            ctx, ty, attr,
            [&target, this](BlendSrcContext&, const core::type::Type*, const IOAttributes& a) {
                if (a.blend_src.has_value()) {
                    AddError(target)
                        << "blend_src cannot be used on members of non-top level structs";
                }
            });
    }
}

void Validator::CheckBlendSrcImpl(BlendSrcContext& ctx,
                                  const CastableBase* target,
                                  const core::type::Type* ty,
                                  const IOAttributes& attr) {
    if (!attr.blend_src.has_value()) {
        return;
    }

    auto bs_val = attr.blend_src.value();
    if (bs_val != 0 && bs_val != 1) {
        AddError(target) << "blend_src value must be 0 or 1";
    }
    if (!ctx.blend_srcs.Add(bs_val)) {
        AddError(target) << "duplicate blend_src(" << bs_val << ") on entry point "
                         << ToString(ctx.dir);
    }

    if (ctx.dir != IODirection::kOutput || ctx.stage != Function::PipelineStage::kFragment) {
        AddError(target) << "blend_src can only be used on fragment shader outputs";
        return;
    }
    if (!attr.location.has_value() || attr.location.value() != 0) {
        AddError(target) << "struct members with blend_src must be located at 0";
    }

    if (!ctx.blend_src_type) {
        if (!ty->IsNumericScalarOrVector()) {
            AddError(target) << "blend_src must be a numeric scalar or vector, but has type "
                             << ty->FriendlyName();
        }
        ctx.blend_src_type = ty;
    } else if (ctx.blend_src_type != ty) {
        AddError(target) << "blend_src type " << ty->FriendlyName()
                         << " does not match other blend_src type "
                         << ctx.blend_src_type->FriendlyName();
    }
}

void Validator::CheckLocation(Hashmap<uint32_t, const CastableBase*, 4>& locations,
                              const CastableBase* target,
                              const IOAttributes& attr,
                              const Function::PipelineStage stage,
                              const core::type::Type* type,
                              const IODirection dir) {
    struct WalkContext {
        Validator* validator;
        Hashmap<uint32_t, const CastableBase*, 4>& locations;
        const CastableBase* target;
        const Function::PipelineStage stage;
        const IODirection dir;
    };
    WalkContext ctx{this, locations, target, stage, dir};

    WalkTypeAndMembers(
        ctx, type, attr,
        [](WalkContext& context, const core::type::Type* ty, const IOAttributes& attribute) {
            if (ty->Is<core::type::Struct>()) {
                return;
            }

            if (attribute.blend_src) {
                // locations associated with a blend_src usage should already be
                // pre-populated in locations
                return;
            }

            if (attribute.location.has_value()) {
                if (context.stage == Function::PipelineStage::kCompute &&
                    context.dir == IODirection::kInput) {
                    context.validator->AddError(context.target)
                        << "location attribute is not valid for compute shader inputs";
                }

                auto loc = attribute.location.value();
                if (const auto conflict = context.locations.Get(loc)) {
                    context.validator->AddError(context.target)
                        << "duplicate location(" << loc << ") on entry point "
                        << ToString(context.dir);
                    context.validator->AddDeclarationNote(*conflict.value);
                } else {
                    context.locations.Add(loc, context.target);
                }
            }
        });
}

void Validator::CheckInterpolation(const CastableBase* anchor,
                                   const core::type::Type* ty,
                                   const IOAttributes& attr,
                                   const Function::PipelineStage stage,
                                   const IODirection dir) {
    bool ctx = false;

    WalkTypeAndMembers(
        ctx, ty, attr,
        [this, anchor, stage, dir](bool& in_location_composite, const core::type::Type* t,
                                   const IOAttributes& a) {
            bool has_location = a.location.has_value() || in_location_composite;
            if (!has_location) {
                if (auto* str = t->As<core::type::Struct>()) {
                    has_location |= str->Members().All(
                        [](const auto* mem) { return mem->Attributes().location.has_value(); });
                }
            }

            if (a.interpolation.has_value()) {
                has_location |= (capabilities_.Contains(Capability::kLoosenValidationForShaderIO) &&
                                 a.builtin.has_value());

                if (!capabilities_.Contains(Capability::kAllowLocationForNumericElements) &&
                    t->As<core::type::Struct>()) {
                    AddError(anchor) << "interpolation cannot be applied to a struct without "
                                        "'kAllowLocationForNumericElements' capability";
                }

                if (t->IsIntegerScalar()) {
                    if (a.interpolation.value().type != InterpolationType::kFlat) {
                        AddError(anchor)
                            << "interpolation attribute type must be flat for integral types";
                    }
                }

                if (!has_location) {
                    if (!capabilities_.Contains(Capability::kLoosenValidationForShaderIO)) {
                        AddError(anchor) << "interpolation attribute requires a location attribute";
                    } else {
                        AddError(anchor) << "interpolation attribute requires a location attribute "
                                            "(or location-like shader I/O annotation)";
                    }
                }
            } else if (has_location && t->IsIntegerScalarOrVector()) {
                // Integral vertex outputs and fragment inputs require flat interpolation.
                const bool needs_flat =
                    (stage == Function::PipelineStage::kVertex && dir == IODirection::kOutput) ||
                    (stage == Function::PipelineStage::kFragment && dir == IODirection::kInput);
                if (needs_flat) {
                    AddError(anchor) << "integral user-defined inputs and outputs must have an "
                                        "@interpolate(flat) attribute";
                }
            }

            if (t->IsAnyOf<core::type::Array, core::type::Struct>()) {
                in_location_composite |= a.location.has_value();
            }
        });
}

void Validator::CheckBindingPoint(const CastableBase* anchor,
                                  const core::type::Type* ty,
                                  const IOAttributes& attr,
                                  const ShaderIOKind& io_kind) {
    const auto& binding_point = attr.binding_point;
    auto address_space = AddressSpace::kUndefined;
    if (const auto* mv = ty->As<core::type::MemoryView>()) {
        address_space = mv->AddressSpace();
    } else {
        // ModuleScopeVars transform in MSL backends unwraps pointers to handles
        if (ty->IsHandle()) {
            address_space = AddressSpace::kHandle;
        }
    }

    if (binding_point.has_value() && io_kind != ShaderIOKind::kModuleScopeVar &&
        !capabilities_.Contains(Capability::kMslAllowEntryPointInterface)) {
        AddError(anchor) << "binding_points are only valid on resource variables";
    }

    switch (address_space) {
        case AddressSpace::kHandle:
            if (!capabilities_.Contains(Capability::kAllowHandleVarsWithoutBindings)) {
                if (!binding_point.has_value()) {
                    AddError(anchor)
                        << "a " << ToString(address_space) << " resource requires a binding point";
                }
            }
            break;
        case AddressSpace::kStorage:
        case AddressSpace::kUniform:
            if (!binding_point.has_value()) {
                AddError(anchor) << "a " << ToString(address_space)
                                 << " resource requires a binding point";
            }
            break;
        default:
            if (binding_point.has_value()) {
                AddError(anchor) << "a " << ToString(address_space)
                                 << " non-resource cannot have a binding point";
            }
            break;
    }
}

void Validator::ValidateShaderIOAnnotations(const CastableBase* msg_anchor,
                                            const core::type::Type* ty,
                                            const std::optional<BindingPoint>& binding_point,
                                            const IOAttributes& attr,
                                            ShaderIOKind kind) {
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
            AddError(msg_anchor) << ToString(kind) << " with void type should never be annotated";
        }
        return;  // Early return because later rules assume non-void types.
    }

    if (attr.location.has_value()) {
        if (capabilities_.Contains(Capability::kAllowLocationForNumericElements)) {
            std::function<bool(const core::type::Type*)> is_numeric =
                [&is_numeric](const core::type::Type* t) -> bool {
                t = t->UnwrapPtrOrRef();
                bool result = false;
                tint::Switch(
                    t,
                    [&](const core::type::Struct* s) {
                        for (auto* m : s->Members()) {
                            if (!is_numeric(m->Type())) {
                                return;
                            }
                        }
                        result = true;
                    },
                    [&](Default) {
                        auto* e = t->DeepestElement()->UnwrapPtrOrRef();
                        tint::Switch(
                            e, [&](const core::type::Struct* s) { result = is_numeric(s); },
                            [&](Default) { result = e->IsNumericScalarOrVector(); });
                    });
                return result;
            };
            if (!is_numeric(ty)) {
                AddError(msg_anchor)
                    << ToString(kind)
                    << " with a location attribute must contain only numeric elements "
                    << ty->FriendlyName();
                return;
            }
        } else {
            if (!ty->UnwrapPtrOrRef()->IsNumericScalarOrVector()) {
                AddError(msg_anchor) << ToString(kind)
                                     << " with a location attribute must be a numeric scalar or "
                                        "vector, but has type "
                                     << ty->FriendlyName();
                return;
            }
        }
    }

    if (auto* ty_struct = ty->UnwrapPtrOrRef()->As<core::type::Struct>()) {
        for (const auto* mem : ty_struct->Members()) {
            EnumSet<IOAnnotation> mem_annotations = annotations;
            auto add_result = AddIOAnnotationsFromIOAttributes(mem_annotations, mem->Attributes());
            if (add_result != Success) {
                AddError(msg_anchor)
                    << ToString(kind)
                    << " struct member has same IO annotation, as top-level struct, '"
                    << ToString(add_result.Failure()) << "'";
                return;
            }

            if (mem->Attributes().location.has_value()) {
                if (capabilities_.Contains(Capability::kAllowLocationForNumericElements)) {
                    if (!mem->Type()->UnwrapPtrOrRef()->IsNumericScalarOrVector() &&
                        !mem->Type()->UnwrapPtrOrRef()->Is<core::type::Struct>()) {
                        AddError(msg_anchor)
                            << ToString(kind)
                            << " struct member with a location attribute must be a numeric scalar, "
                               "a numeric vector or a struct, but has type "
                            << mem->Type()->FriendlyName();
                        return;
                    }
                } else {
                    if (!mem->Type()->UnwrapPtrOrRef()->IsNumericScalarOrVector()) {
                        AddError(msg_anchor) << ToString(kind)
                                             << " struct member with a location attribute must be "
                                                "a numeric scalar or vector, but has type "
                                             << mem->Type()->FriendlyName();
                        return;
                    }
                }
            }

            if (capabilities_.Contains(Capability::kMslAllowEntryPointInterface)) {
                if (auto* mv = mem->Type()->As<core::type::MemoryView>()) {
                    if (mv->AddressSpace() == AddressSpace::kWorkgroup) {
                        mem_annotations.Add(IOAnnotation::kWorkgroup);
                    }
                }
            }

            if (mem_annotations.Empty()) {
                AddError(msg_anchor) << ToString(kind)
                                     << " struct members must have at least one IO annotation, "
                                        "e.g. a binding point, a location, etc";
            } else if (mem_annotations.Size() > 1) {
                AddError(msg_anchor)
                    << ToString(kind) << " struct member has more than one IO annotation, "
                    << ToString(mem_annotations);
            }
        }
    } else {
        if (annotations.Empty()) {
            if (!(capabilities_.Contains(Capability::kAllowUnannotatedModuleIOVariables) &&
                  kind == ShaderIOKind::kModuleScopeVar)) {
                AddError(msg_anchor) << ToString(kind)
                                     << " must have at least one IO annotation, e.g. a binding "
                                        "point, a location, etc";
            }
        } else if (annotations.Size() > 1) {
            AddError(msg_anchor) << ToString(kind) << " has more than one IO annotation, "
                                 << ToString(annotations);
        }
    }
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
    if (!capabilities_.Contains(Capability::kAllowAnyLetType)) {
        if (auto* ptr = result_ty->As<core::type::Pointer>()) {
            if (ptr->AddressSpace() == AddressSpace::kHandle &&
                !capabilities_.Contains(Capability::kAllowPointerToHandle)) {
                AddError(l) << "handle pointer cannot be captured in a let";
            }
        } else if (!result_ty->IsConstructible()) {
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
        call,                                                            //
        [&](const BuiltinCall* c) { CheckBuiltinCall(c); },              //
        [&](const MemberBuiltinCall* c) { CheckMemberBuiltinCall(c); },  //
        [&](const Construct* c) { CheckConstruct(c); },                  //
        [&](const Convert* c) { CheckConvert(c); },                      //
        [&](const Discard* d) {                                          //
            stage_restricted_instructions_.Add(
                d, SupportedStages{Function::PipelineStage::kFragment});        //
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

    // Check evaluation stage of parameters that are required to be const-expressions.
    for (uint32_t i = 0; i < builtin->parameters.Length(); i++) {
        const auto& p = builtin->parameters[i];
        const auto* arg = call->Args()[i];
        if (p.is_const && !arg->Is<Constant>()) {
            AddError(call, BuiltinCall::kArgsOperandOffset + i)
                << "the " << style::Variable(p.usage) << " argument must be a constant";
            return;
        }
    }

    if (auto* bc = call->As<CoreBuiltinCall>()) {
        CheckCoreBuiltinCall(bc, builtin.Get());
    }

    // Track the stages that this builtin call is limited to, so that we can check them against the
    // entry points that they are used from.
    SupportedStages stages;
    if (builtin->info->flags.Contains(intrinsic::OverloadFlag::kSupportsComputePipeline)) {
        stages.Add(Function::PipelineStage::kCompute);
    }
    if (builtin->info->flags.Contains(intrinsic::OverloadFlag::kSupportsFragmentPipeline)) {
        stages.Add(Function::PipelineStage::kFragment);
    }
    if (builtin->info->flags.Contains(intrinsic::OverloadFlag::kSupportsVertexPipeline)) {
        stages.Add(Function::PipelineStage::kVertex);
    }
    stage_restricted_instructions_.Add(call, stages);
}

void Validator::CheckCoreBuiltinCall(const CoreBuiltinCall* call,
                                     const core::intrinsic::Overload& overload) {
    auto idx_for_usage = [&](core::ParameterUsage usage) -> std::optional<uint32_t> {
        for (uint32_t i = 0; i < overload.parameters.Length(); ++i) {
            auto& p = overload.parameters[i];
            if (p.usage == usage) {
                return int32_t(i);
            }
        }
        return std::nullopt;
    };

    auto check_arg_in_range = [&](core::ParameterUsage usage, int32_t min, int32_t max) {
        auto idx_opt = idx_for_usage(usage);
        if (!idx_opt.has_value()) {
            return;
        }
        uint32_t idx = idx_opt.value();
        TINT_ASSERT(idx < call->Args().size());

        auto* val = call->Args()[idx];
        auto* const_val = val->As<ir::Constant>();
        TINT_ASSERT(const_val);
        auto* cnst = const_val->Value();

        if (val->Type()->Is<core::type::Vector>()) {
            for (size_t i = 0; i < cnst->NumElements(); i++) {
                auto value = cnst->Index(i)->ValueAs<int32_t>();
                if (value < min || value > max) {
                    AddError(call, idx)
                        << value << " outside range of [" << min << ", " << max << "]";
                    return;
                }
            }
        } else {
            auto value = cnst->ValueAs<int32_t>();
            if (value < min || value > max) {
                AddError(call, idx) << value << " outside range of [" << min << ", " << max << "]";
                return;
            }
        }
    };

    if (core::IsTexture(call->Func())) {
        check_arg_in_range(core::ParameterUsage::kComponent, 0, 3);
        check_arg_in_range(core::ParameterUsage::kOffset, -8, 7);
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

    auto* result_type = construct->Result()->Type();

    if (!result_type->IsConstructible()) {
        // We only allow `construct` to create non-constructible types when they are structures that
        // contain pointers and handle types, with the corresponding capability enabled.
        if (!(result_type->Is<core::type::Struct>() &&
              capabilities_.Contains(Capability::kMslAllowEntryPointInterface))) {
            AddError(construct) << "type is not constructible";
            return;
        }
    }

    auto args = construct->Args();
    if (args.empty()) {
        // Zero-value constructors are valid for all constructible types.
        return;
    }

    auto check_args_match_elements = [&] {
        // Check that type type of each argument matches the expected element type of the composite.
        for (size_t i = 0; i < args.size(); i++) {
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
        if (args.size() > 1) {
            AddError(construct) << "scalar construct must not have more than one argument";
        }
        if (args[0]->Type() != result_type) {
            AddError(construct, 0u) << "scalar construct argument type " << NameOf(args[0]->Type())
                                    << " does not match result type " << NameOf(result_type);
        }
    } else if (auto* sg_mat = result_type->As<core::type::SubgroupMatrix>()) {
        if (args.size() > 1) {
            AddError(construct) << "subgroup matrix construct must not have more than 1 argument";
        } else {
            // 8-bit integer matrices use 32-bit shader scalar types in WGSL.
            // Some backends may support 8-bit integers, in which case they would pass an 8-bit
            // type for the constructor value instead.
            auto* scalar_ty = sg_mat->Type();
            if (scalar_ty->Is<core::type::I8>()) {
                scalar_ty = type_mgr_.i32();
            } else if (scalar_ty->Is<core::type::U8>()) {
                scalar_ty = type_mgr_.u32();
            }
            if (args[0]->Type() != scalar_ty && args[0]->Type() != sg_mat->Type()) {
                AddError(construct)
                    << "subgroup matrix construct argument type " << NameOf(args[0]->Type())
                    << " does not match matrix shader scalar type " << NameOf(scalar_ty);
            }
        }
    } else if (auto* vec = result_type->As<core::type::Vector>()) {
        auto table = intrinsic::Table<intrinsic::Dialect>(type_mgr_, symbols_);
        auto ctor_conv = intrinsic::VectorCtorConv(vec->Width());
        auto arg_types = Transform<4>(args, [&](auto* v) { return v->Type(); });
        auto match = table.Lookup(ctor_conv, Vector{vec->Type()}, std::move(arg_types),
                                  core::EvaluationStage::kConstant);
        if (match != Success || vec->Type() != arg_types[0]->DeepestElement()) {
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
    } else if (auto* arr = result_type->As<core::type::Array>()) {
        if (args.size() != arr->ConstantCount()) {
            AddError(construct) << "array has " << arr->ConstantCount().value()
                                << " elements, but construct provides " << args.size()
                                << " arguments";
            return;
        }
        check_args_match_elements();
    } else if (auto* str = As<core::type::Struct>(result_type)) {
        auto members = str->Members();
        if (args.size() != str->Members().Length()) {
            AddError(construct) << "structure has " << members.Length()
                                << " members, but construct provides " << args.size()
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

    if (call->Target()->ReturnType() != call->Result()->Type()) {
        AddError(call) << "result type does not match function return type";
        return;
    }

    auto args = call->Args();
    auto params = call->Target()->Params();
    if (args.size() != params.Length()) {
        AddError(call, UserCall::kFunctionOperandOffset)
            << "function has " << params.Length() << " parameters, but call provides "
            << args.size() << " arguments";
        return;
    }

    for (size_t i = 0; i < args.size(); i++) {
        bool allow_mismatch = false;
        if (auto* arg_buffer_ty = args[i]->Type()->UnwrapPtrOrRef()->As<core::type::Buffer>()) {
            auto* arg_ptr_ty = args[i]->Type()->As<core::type::Pointer>();
            if (auto* param_ptr_ty = params[i]->Type()->As<core::type::Pointer>()) {
                if (auto* param_buffer_ty =
                        param_ptr_ty->UnwrapPtrOrRef()->As<core::type::Buffer>()) {
                    allow_mismatch = arg_ptr_ty->AddressSpace() == param_ptr_ty->AddressSpace() &&
                                     arg_ptr_ty->Access() == param_ptr_ty->Access();
                    uint32_t arg_size = arg_buffer_ty->ConstantCount().value_or(0);
                    uint32_t param_size = param_buffer_ty->ConstantCount().value_or(0);
                    allow_mismatch &=
                        param_buffer_ty->Count()->Is<core::type::RuntimeArrayCount>() ||
                        param_size < arg_size;
                }
            }
        }
        if (!allow_mismatch && args[i]->Type() != params[i]->Type()) {
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

    for (size_t i = 0; i < a->Indices().size(); i++) {
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
    if (!CheckResultsAndOperands(b, Binary::kNumResults, Binary::kNumOperands)) {
        return;
    }

    if (b->Op() == core::BinaryOp::kLogicalAnd) {
        AddError(b) << "logical-and is not valid in the IR";
        return;
    }
    if (b->Op() == core::BinaryOp::kLogicalOr) {
        AddError(b) << "logical-or is not valid in the IR";
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
    if (!CheckResultsAndOperands(u, Unary::kNumResults, Unary::kNumOperands)) {
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
    CheckOperands(if_, If::kNumOperands);

    if (if_->Condition() && !if_->Condition()->Type()->Is<core::type::Bool>()) {
        AddError(if_, If::kConditionOperandOffset) << "condition type must be 'bool'";
    }

    if (if_->False() && if_->False()->Is<core::ir::MultiInBlock>()) {
        AddError(if_) << "if false block must be a block";
    }
    if (if_->True() && if_->True()->Is<core::ir::MultiInBlock>()) {
        AddError(if_) << "if true block must be a block";
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

    if (!l->Initializer()->IsEmpty()) {
        if (!l->Initializer()->Terminator() ||
            !l->Initializer()->Terminator()->Is<core::ir::NextIteration>()) {
            AddError(l->Initializer()) << "loop initializer must have a NextIteration terminator";
        }
    }

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

    if (!loop->Continuing()->Terminator()->IsAnyOf<NextIteration, BreakIf>()) {
        AddError(loop->Continuing())
            << "loop continuing terminator can only be next_iteration or break_if";
    }
}

void Validator::CheckSwitch(const Switch* s) {
    CheckResults(s);
    CheckOperands(s, Switch::kNumOperands);

    if (s->Condition() && !s->Condition()->Type()->IsIntegerScalar()) {
        auto* cond_ty = s->Condition() ? s->Condition()->Type() : nullptr;
        AddError(s, Switch::kConditionOperandOffset)
            << "condition type " << NameOf(cond_ty) << " must be an integer scalar";
    }

    tasks_.Push([this] { control_stack_.Pop(); });

    bool found_default = false;
    for (auto& cse : s->Cases()) {
        if (cse.block->Is<core::ir::MultiInBlock>()) {
            AddError(s) << "case block must be a block";
        }

        if (cse.selectors.IsEmpty()) {
            AddError(s) << "case does not have any selectors";
        }

        QueueBlock(cse.block);
        for (const auto& sel : cse.selectors) {
            if (sel.IsDefault()) {
                if (found_default) {
                    AddError(s) << "multiple default selectors in switch";
                }
                found_default = true;
            } else if (!sel.val->Type()->IsIntegerScalar()) {
                AddError(s) << "case selector type " << NameOf(sel.val->Type())
                            << " must be an integer scalar";
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
    if (b->Condition() == nullptr) {
        AddError(b) << "break_if condition cannot be nullptr";
        return;
    }

    if (!b->Condition()->Type() || !b->Condition()->Type()->Is<core::type::Bool>()) {
        AddError(b) << "condition must be a 'bool'";
        return;
    }

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
        CheckOperandsMatchTarget(b, b->ArgsOperandOffset(), next_iter_values.size(), body,
                                 body->Params());
    }

    auto exit_values = b->ExitValues();
    CheckOperandsMatchTarget(b, b->ArgsOperandOffset() + next_iter_values.size(),
                             exit_values.size(), loop, loop->Results());
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
        CheckOperandsMatchTarget(c, Continue::kArgsOperandOffset, c->Args().size(), cont,
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
    CheckOperandsMatchTarget(e, e->ArgsOperandOffset(), args.size(), e->ControlInstruction(),
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
        CheckOperandsMatchTarget(n, NextIteration::kArgsOperandOffset, n->Args().size(), body,
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

    if (func != ContainingFunction(ret)) {
        AddError(ret) << "function operand does not match containing function";
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

bool Validator::CanLoad(const core::type::Type* ty) {
    return tint::Switch(
        ty,  //
        [&](const core::type::Array* arr) {
            if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
                return false;
            }
            return CanLoad(arr->Elements().type);
        },
        [&](const core::type::Struct* str) {
            for (auto* member : str->Members()) {
                if (member->Type()->Is<core::type::Pointer>() &&
                    capabilities_.Contains(Capability::kMslAllowEntryPointInterface)) {
                    continue;
                }
                if (!CanLoad(member->Type())) {
                    return false;
                }
            }
            return true;
        },
        [&](Default) { return ty->IsConstructible() || ty->IsHandle(); });
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

        if (!CanLoad(mv->StoreType())) {
            AddError(l, Load::kFromOperandOffset)
                << "type " << NameOf(mv->StoreType()) << " cannot be loaded";
            return;
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
        auto* el_ty = GetVectorPtrElementType(l, LoadVectorElement::kFromOperandOffset);
        if (!el_ty) {
            return;
        }
        if (res->Type() != el_ty) {
            AddResultError(l, 0) << "result type " << NameOf(res->Type())
                                 << " does not match vector pointer element type " << NameOf(el_ty);
            return;
        }
    }

    if (!l->Index()->Type()->IsIntegerScalar()) {
        AddError(l, LoadVectorElement::kIndexOperandOffset)
            << "load vector element index must be an integer scalar";
    }
    if (auto* c = l->Index()->As<core::ir::Constant>()) {
        auto val = c->Value()->ValueAs<uint32_t>();

        auto* vec_ty = l->From()->Type()->UnwrapPtrOrRef()->As<core::type::Vector>();
        TINT_ASSERT(vec_ty);

        if (val >= vec_ty->Width()) {
            AddError(l, LoadVectorElement::kIndexOperandOffset)
                << "load vector element index must be in range [0, " << (vec_ty->Width() - 1)
                << "]";
        }
    }
}

void Validator::CheckStoreVectorElement(const StoreVectorElement* s) {
    if (!CheckResultsAndOperands(s, StoreVectorElement::kNumResults,
                                 StoreVectorElement::kNumOperands)) {
        return;
    }

    if (auto* value = s->Value()) {
        auto* el_ty = GetVectorPtrElementType(s, StoreVectorElement::kToOperandOffset);
        if (!el_ty) {
            return;
        }
        if (value->Type() != el_ty) {
            AddError(s, StoreVectorElement::kValueOperandOffset)
                << "value type " << NameOf(value->Type())
                << " does not match vector pointer element type " << NameOf(el_ty);
            return;
        }

        // The `GetVectorPtrElementType` has already validated that the pointer exists.
        auto* mv = s->To()->Type()->As<core::type::MemoryView>();
        if (mv->Access() != core::Access::kWrite && mv->Access() != core::Access::kReadWrite) {
            AddError(s, StoreVectorElement::kToOperandOffset)
                << "store_vector_element target operand has a non-writeable access type, "
                << style::Literal(ToString(mv->Access()));
            return;
        }
    }

    if (!s->Index()->Type()->IsIntegerScalar()) {
        AddError(s, StoreVectorElement::kIndexOperandOffset)
            << "store vector element index must be an integer scalar";
    }
    if (auto* c = s->Index()->As<core::ir::Constant>()) {
        auto val = c->Value()->ValueAs<uint32_t>();

        auto* vec_ty = s->To()->Type()->UnwrapPtrOrRef()->As<core::type::Vector>();
        TINT_ASSERT(vec_ty);

        if (val >= vec_ty->Width()) {
            AddError(s, StoreVectorElement::kIndexOperandOffset)
                << "store vector element index must be in range [0, " << (vec_ty->Width() - 1)
                << "]";
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
        AddError(inst, idx) << "missing element operand";
        return nullptr;
    }

    auto* type = operand->Type();
    if (DAWN_UNLIKELY(!type)) {
        AddError(inst, idx) << "missing operand type";
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

Result<SuccessType> Validate(const Module& mod, Capabilities capabilities, std::string_view msg) {
    DumpIRIfEnabled(mod, msg);
    Validator v(mod, capabilities);
    return v.Run();
}

void AssertValid(const Module& mod,
                 [[maybe_unused]] Capabilities capabilities,
                 std::string_view msg) {
    DumpIRIfEnabled(mod, msg);

#if TINT_ENABLE_IR_VALIDATION_ASSERTS
    if (mod.enable_validation_asserts) {
        Validator v(mod, capabilities);
        auto result = v.Run();
        if (result != Success) {
            TINT_ICE() << "\n========================================================="
                       << "\n== IR validation failed " << msg << ":"
                       << "\n=========================================================\n"
                       << result.Failure().reason;
        }
    }
#endif
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
