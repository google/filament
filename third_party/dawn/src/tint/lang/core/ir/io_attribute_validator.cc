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

#include "src/tint/lang/core/ir/io_attribute_validator.h"

#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"

namespace tint::core::ir::validator {
namespace {

/// A BuiltInChecker is the interface used to check that a usage of a builtin attribute meets the
/// basic spec rules, i.e. correct shader stage, data type, and IO direction.
/// It does not test more sophisticated rules like location and builtins being mutually exclusive or
/// that the correct properties are enabled.
struct BuiltInChecker {
    /// What combination of stage and IO direction is this builtin legal for
    EnumSet<IOAttributeUsage> valid_usages;

    /// What values for depth_mode are valid for this builtin.
    /// Currently, kUndefined is the only valid option for non-frag_depth
    EnumSet<BuiltinDepthMode> valid_depth_modes =
        EnumSet<BuiltinDepthMode>{BuiltinDepthMode::kUndefined};

    /// Implements logic for checking if the given type is valid or not. Is not a data entry (i.e. a
    /// type or set of types), because types are part of the IR module and created at runtime.
    using TypeCheckFn = bool(const core::type::Type* type, const Properties& props);

    /// @see #TypeCheckFn
    TypeCheckFn* const type_check;

    /// Message for logging if the type check fails. Cannot be easily generated at runtime, because
    /// the type check is a function, not just a data entry.
    const char* type_error;
};

constexpr BuiltInChecker kPointSizeChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::F32>();
    },
    .type_error = "must be a f32",
};

/// returns true if the number of elements in @p ty is valid for use in clip_distances without
/// Property::kAllowClipDistancesOnF32.
constexpr auto ClipDistancesElementsCheck = [](const core::type::Type* ty) -> bool {
    const auto elems = ty->Elements();
    return elems.type && elems.type->Is<core::type::F32>() && elems.count <= 8;
};

constexpr BuiltInChecker kClipDistancesChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::Array>() && ClipDistancesElementsCheck(ty);
    },
    .type_error = "must be an array<f32, N>, where N <= 8",
};

constexpr BuiltInChecker kClipDistancesAllowF32ScalarAndVectorChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ((ty->Is<core::type::Array>() || ty->Is<core::type::Vector>()) &&
                ClipDistancesElementsCheck(ty)) ||
               ty->Is<core::type::F32>();
    },
    .type_error = "must be a f32 or either a vecN<f32> or an array<f32, N>, where N <= 8",
};

constexpr BuiltInChecker kCullDistanceChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::Array>() && ty->DeepestElement()->Is<core::type::F32>();
    },
    .type_error = "must be an array of f32",
};

constexpr BuiltInChecker kFragDepthChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentOutputUsage},
    .valid_depth_modes =
        EnumSet<BuiltinDepthMode>{BuiltinDepthMode::kUndefined, BuiltinDepthMode::kGreater,
                                  BuiltinDepthMode::kLess},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::F32>();
    },
    .type_error = "must be a f32",
};

constexpr BuiltInChecker kFrontFacingChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::Bool>();
    },
    .type_error = "must be a bool",
};

constexpr BuiltInChecker kGlobalInvocationIdChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        auto* vec = ty->As<core::type::Vector>();
        return vec && vec->Width() == 3 && vec->Type()->Is<core::type::U32>();
    },
    .type_error = "must be an vec3<u32>",
};

constexpr BuiltInChecker kInstanceIndexChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kLocalInvocationIdChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        auto* vec = ty->As<core::type::Vector>();
        return vec && vec->Width() == 3 && vec->Type()->Is<core::type::U32>();
    },
    .type_error = "must be an vec3<u32>",
};

constexpr BuiltInChecker kLocalInvocationIndexChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kNumSubgroupsChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kNumWorkgroupsChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        auto* vec = ty->As<core::type::Vector>();
        return vec && vec->Width() == 3 && vec->Type()->Is<core::type::U32>();
    },
    .type_error = "must be an vec3<u32>",
};

constexpr BuiltInChecker kPositionChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage,
                                              IOAttributeUsage::kFragmentInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        auto* vec = ty->As<core::type::Vector>();
        return vec && vec->Width() == 4 && vec->Type()->Is<core::type::F32>();
    },
    .type_error = "must be an vec4<f32>",
};

constexpr BuiltInChecker kSampleIndexChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kSampleMaskChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage,
                                              IOAttributeUsage::kFragmentOutputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kSubgroupIdChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kSubgroupInvocationIdChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage,
                                              IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kSubgroupSizeChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage,
                                              IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kVertexIndexChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kWorkgroupIdChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kComputeInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        auto* vec = ty->As<core::type::Vector>();
        return vec && vec->Width() == 3 && vec->Type()->Is<core::type::U32>();
    },
    .type_error = "must be an vec3<u32>",
};

constexpr BuiltInChecker kPrimitiveIndexChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->Is<core::type::U32>();
    },
    .type_error = "must be an u32",
};

constexpr BuiltInChecker kBarycentricCoordChecker{
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage},
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        auto* vec = ty->As<core::type::Vector>();
        return vec && vec->Width() == 3 && vec->Type()->Is<core::type::F32>();
    },
    .type_error = "must be an vec3<f32>",
};

/// @returns an appropriate BuiltInCheck for @p builtin, ICEs when one isn't defined
const BuiltInChecker& BuiltinCheckerFor(BuiltinValue builtin, const Properties& properties) {
    switch (builtin) {
        case BuiltinValue::kPointSize:
            return kPointSizeChecker;
        case BuiltinValue::kClipDistances:
            if (properties.Contains(Property::kAllowClipDistancesOnF32ScalarAndVector)) {
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

constexpr IOAttributeChecker kInvariantChecker{
    .kind = IOAttributeKind::kInvariant,
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kVertexOutputUsage,
                                              IOAttributeUsage::kFragmentInputUsage},
    .valid_io_kinds = EnumSet<ShaderIOKind>{ShaderIOKind::kInputParam, ShaderIOKind::kResultValue,
                                            ShaderIOKind::kModuleScopeVar},
    .check = [](const core::type::Type*,
                const IOAttributes& attr,
                const Properties&,
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
                const Properties& prop,
                IOAttributeUsage usage) -> Result<SuccessType, std::string> {
        if (!attr.builtin.has_value()) {
            return Success;
        }

        const auto builtin = attr.builtin.value();
        const auto& checker = BuiltinCheckerFor(builtin, prop);
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

        if (!checker.type_check(ty, prop)) {
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
            !prop.Contains(Property::kAllowPointSizeBuiltin)) {
            return std::string{"use of point_size builtin requires kAllowPointSizeBuiltin"};
        }

        return Success;
    },
    .type_check = [](const core::type::Type*, const Properties&) -> bool { return true; },
    .type_error = nullptr,
};

constexpr IOAttributeChecker kColorChecker{
    .kind = IOAttributeKind::kColor,
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentInputUsage},
    .valid_io_kinds =
        EnumSet<ShaderIOKind>{ShaderIOKind::kInputParam, ShaderIOKind::kModuleScopeVar,
                              ShaderIOKind::kResultValue},
    .check = [](const core::type::Type*, const IOAttributes&, const Properties&, IOAttributeUsage)
        -> Result<SuccessType, std::string> { return Success; },
    .type_check = [](const core::type::Type* ty, const Properties&) -> bool {
        return ty->IsNumericScalarOrVector();
    },
    .type_error = "must be a numeric scalar or vector",
};

constexpr IOAttributeChecker kInputAttachmentIndexChecker{
    .kind = IOAttributeKind::kInputAttachmentIndex,
    .valid_usages = EnumSet<IOAttributeUsage>{IOAttributeUsage::kFragmentResourceUsage},
    .valid_io_kinds = EnumSet<ShaderIOKind>{ShaderIOKind::kModuleScopeVar},
    .check = [](const core::type::Type*, const IOAttributes&, const Properties&, IOAttributeUsage)
        -> Result<SuccessType, std::string> { return Success; },
    .type_check = [](const core::type::Type* ty, const Properties& props) -> bool {
        return props.Contains(Property::kAllowAnyInputAttachmentIndexType) ||
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
                const Properties&,
                IOAttributeUsage) -> Result<SuccessType, std::string> {
        if (!attr.builtin.has_value()) {
            return {"cannot have a depth_mode without a builtin"};
        }
        return Success;
    },
    .type_check = [](const core::type::Type*, const Properties&) -> bool { return true; },
    .type_error = nullptr,
};

// kBlendSrcChecker, kLocationChecker, kInterpolationChecker, and kBindingPointChecker are
// intentionally not implemented

}  // namespace

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

}  // namespace tint::core::ir::validator
