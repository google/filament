// Copyright 2021 The Dawn & Tint Authors
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

#include "dawn/native/Limits.h"

#include <algorithm>
#include <array>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Log.h"
#include "dawn/common/Math.h"
#include "dawn/native/Instance.h"

// clang-format off
// TODO(crbug.com/dawn/685):
// For now, only expose these tiers until metrics can determine better ones.
//                                                compat  tier0  tier1
#define LIMITS_WORKGROUP_STORAGE_SIZE(X)                                         \
    X(v1, Maximum, maxComputeWorkgroupStorageSize, 16384, 16384, 32768, 49152, 65536)

// Tiers for limits related to workgroup size.
// TODO(crbug.com/dawn/685): Define these better. For now, use two tiers where one
// is available on nearly all desktop platforms.
//                                                                 compat        tier0       tier1
#define LIMITS_WORKGROUP_SIZE(X)                                                                    \
    X(v1, Maximum,           maxComputeInvocationsPerWorkgroup,       128,         256,       1024) \
    X(v1, Maximum,                    maxComputeWorkgroupSizeX,       128,         256,       1024) \
    X(v1, Maximum,                    maxComputeWorkgroupSizeY,       128,         256,       1024) \
    X(v1, Maximum,                    maxComputeWorkgroupSizeZ,        64,          64,         64) \
    X(v1, Maximum,            maxComputeWorkgroupsPerDimension,     65535,       65535,      65535)

// Tiers are 128MB, 512MB, 1GB, 2GB-4, 4GB-4.
//                                                 compat     tier0      tier1
#define LIMITS_STORAGE_BUFFER_BINDING_SIZE(X)                                                        \
    X(v1, Maximum, maxStorageBufferBindingSize, 134217728, 134217728, 536870912, 1073741824, 2147483644, 4294967292)

// Tiers are 256MB, 1GB, 2GB, 4GB.
//                                    compat      tier0       tier1
#define LIMITS_MAX_BUFFER_SIZE(X)                                                         \
    X(v1, Maximum, maxBufferSize, 0x10000000, 0x10000000, 0x40000000, 0x80000000, 0x100000000)

// Tiers for limits related to resource bindings.
// TODO(crbug.com/dawn/685): Define these better. For now, use two tiers where one
// offers slightly better than default limits.
//                                                                     compat      tier0       tier1
#define LIMITS_RESOURCE_BINDINGS(X)                                                                   \
    X(v1,     Maximum,   maxDynamicUniformBuffersPerPipelineLayout,         8,         8,         10) \
    X(v1,     Maximum,   maxDynamicStorageBuffersPerPipelineLayout,         4,         4,          8) \
    X(v1,     Maximum,            maxSampledTexturesPerShaderStage,        16,        16,         16) \
    X(v1,     Maximum,                   maxSamplersPerShaderStage,        16,        16,         16) \
    X(v1,     Maximum,            maxStorageTexturesPerShaderStage,         4,         4,          8) \
    X(compat, Maximum,           maxStorageTexturesInFragmentStage,         4,         4,          8) \
    X(compat, Maximum,             maxStorageTexturesInVertexStage,         0,         4,          8) \
    X(v1,     Maximum,             maxUniformBuffersPerShaderStage,        12,        12,         12)

// Tiers for limits related to storage buffer bindings. Should probably be merged with
// LIMITS_RESOURCE_BINDINGS.
// TODO(crbug.com/dawn/685): Define these better. For now, use two tiers where one
// offers slightly better than default limits.
//
#define LIMITS_STORAGE_BUFFER_BINDINGS(X)                                                              \
    X(v1,     Maximum,             maxStorageBuffersPerShaderStage,         8,         8,          10) \
    X(compat, Maximum,            maxStorageBuffersInFragmentStage,         4,         8,          10) \
    X(compat, Maximum,              maxStorageBuffersInVertexStage,         0,         8,          10)

// TODO(crbug.com/dawn/685):
// These limits aren't really tiered and could probably be grouped better.
// All Chrome platforms support 64 (iOS is 32) so there's are really only two exposed
// buckets: 64 and 128.
//                                                                compat   tier0  tier1  tier2
#define LIMITS_ATTACHMENTS(X)   \
    X(v1, Maximum,            maxColorAttachmentBytesPerSample,       32,     32,    64,   128)

// Tiers for limits related to inter-stage shader variables.
//                                                                compat      tier0       tier1
#define LIMITS_INTER_STAGE_SHADER_VARIABLES(X) \
    X(v1, Maximum,               maxInterStageShaderVariables,        15,        16,         28) \

// Tiered limits for texture dimensions.
// TODO(crbug.com/dawn/685): Define these better. For now, use two tiers where some dimensions
// offers slightly better than default limits.
//                                                                 compat      tier0       tier1
#define LIMITS_TEXTURE_DIMENSIONS(X) \
    X(v1, Maximum,                       maxTextureDimension1D,      4096,      8192,      16384) \
    X(v1, Maximum,                       maxTextureDimension2D,      4096,      8192,      16384) \
    X(v1, Maximum,                       maxTextureDimension3D,      1024,      2048,       2048) \
    X(v1, Maximum,                       maxTextureArrayLayers,       256,       256,       2048)

// Tired limits for immediate data sizes.
//                                 compat  tier0  tier1
#define LIMITS_IMMEDIATE_SIZE(X) \
  X(v1, Maximum, maxImmediateSize,       0,    0,    16,  32,  64)

// TODO(crbug.com/dawn/685):
// These limits don't have tiers yet. Define two tiers with the same values since the macros
// in this file expect more than one tier.
//                                                                                         compat      tier0      tier1
#define LIMITS_OTHER(X)                                                                                                             \
    X(v1,                              Maximum,                                     maxBindGroups,         4,         4,          4) \
    X(v1,                              Maximum,                    maxBindGroupsPlusVertexBuffers,        24,        24,         24) \
    X(v1,                              Maximum,                           maxBindingsPerBindGroup,      1000,      1000,       1000) \
    X(v1,                              Maximum,                       maxUniformBufferBindingSize,     16384,     65536,      65536) \
    X(v1,                            Alignment,                 minUniformBufferOffsetAlignment,       256,       256,        256) \
    X(v1,                            Alignment,                 minStorageBufferOffsetAlignment,       256,       256,        256) \
    X(v1,                              Maximum,                                  maxVertexBuffers,         8,         8,          8) \
    X(v1,                              Maximum,                               maxVertexAttributes,        16,        16,         30) \
    X(v1,                              Maximum,                        maxVertexBufferArrayStride,      2048,      2048,       2048) \
    X(v1,                              Maximum,                               maxColorAttachments,         4,         8,          8)

// clang-format on

#define LIMITS_EACH_GROUP(X)               \
    X(LIMITS_WORKGROUP_STORAGE_SIZE)       \
    X(LIMITS_WORKGROUP_SIZE)               \
    X(LIMITS_STORAGE_BUFFER_BINDING_SIZE)  \
    X(LIMITS_MAX_BUFFER_SIZE)              \
    X(LIMITS_RESOURCE_BINDINGS)            \
    X(LIMITS_STORAGE_BUFFER_BINDINGS)      \
    X(LIMITS_ATTACHMENTS)                  \
    X(LIMITS_INTER_STAGE_SHADER_VARIABLES) \
    X(LIMITS_TEXTURE_DIMENSIONS)           \
    X(LIMITS_IMMEDIATE_SIZE)               \
    X(LIMITS_OTHER)

#define LIMITS(X)                          \
    LIMITS_WORKGROUP_STORAGE_SIZE(X)       \
    LIMITS_WORKGROUP_SIZE(X)               \
    LIMITS_STORAGE_BUFFER_BINDING_SIZE(X)  \
    LIMITS_MAX_BUFFER_SIZE(X)              \
    LIMITS_RESOURCE_BINDINGS(X)            \
    LIMITS_STORAGE_BUFFER_BINDINGS(X)      \
    LIMITS_ATTACHMENTS(X)                  \
    LIMITS_INTER_STAGE_SHADER_VARIABLES(X) \
    LIMITS_TEXTURE_DIMENSIONS(X)           \
    LIMITS_IMMEDIATE_SIZE(X)               \
    LIMITS_OTHER(X)

namespace dawn::native {
namespace {
template <uint32_t A, uint32_t B>
constexpr void StaticAssertSame() {
    static_assert(A == B, "Mismatching tier count in limit group.");
}

template <uint32_t I, uint32_t... Is>
constexpr uint32_t ReduceSameValue(std::integer_sequence<uint32_t, I, Is...>) {
    [[maybe_unused]] int unused[] = {0, (StaticAssertSame<I, Is>(), 0)...};
    return I;
}

enum class LimitClass {
    Alignment,
    Maximum,
};

template <LimitClass C>
struct CheckLimit;

template <>
struct CheckLimit<LimitClass::Alignment> {
    template <typename T>
    static bool IsBetter(T lhs, T rhs) {
        return lhs < rhs;
    }

    template <typename T>
    static MaybeError Validate(T supported, T required) {
        DAWN_INVALID_IF(IsBetter(required, supported),
                        "Required limit (%u) is lower than the supported limit (%u).", required,
                        supported);
        DAWN_INVALID_IF(!IsPowerOfTwo(required), "Required limit (%u) is not a power of two.",
                        required);
        return {};
    }
};

template <>
struct CheckLimit<LimitClass::Maximum> {
    template <typename T>
    static bool IsBetter(T lhs, T rhs) {
        return lhs > rhs;
    }

    template <typename T>
    static MaybeError Validate(T supported, T required) {
        DAWN_INVALID_IF(IsBetter(required, supported),
                        "Required limit (%u) is greater than the supported limit (%u).", required,
                        supported);
        return {};
    }
};

template <typename T>
bool IsLimitUndefined(T value) {
    static_assert(sizeof(T) != sizeof(T), "IsLimitUndefined not implemented for this type");
    return false;
}

template <>
bool IsLimitUndefined<uint32_t>(uint32_t value) {
    return value == wgpu::kLimitU32Undefined;
}

template <>
bool IsLimitUndefined<uint64_t>(uint64_t value) {
    return value == wgpu::kLimitU64Undefined;
}

}  // namespace

void GetDefaultLimits(CombinedLimits* limits, wgpu::FeatureLevel featureLevel) {
    DAWN_ASSERT(limits != nullptr);
#define X(Scope, Better, limitName, compat, base, ...) \
    limits->Scope.limitName = featureLevel == wgpu::FeatureLevel::Compatibility ? compat : base;
    LIMITS(X)
#undef X
}

CombinedLimits ReifyDefaultLimits(const CombinedLimits& limits, wgpu::FeatureLevel featureLevel) {
    CombinedLimits out;
#define X(Scope, Class, limitName, compat, base, ...)                                        \
    {                                                                                        \
        const auto defaultLimit = static_cast<decltype(limits.Scope.limitName)>(             \
            featureLevel == wgpu::FeatureLevel::Compatibility ? compat : base);              \
        if (IsLimitUndefined(limits.Scope.limitName) ||                                      \
            CheckLimit<LimitClass::Class>::IsBetter(defaultLimit, limits.Scope.limitName)) { \
            /* If the limit is undefined or the default is better, use the default */        \
            out.Scope.limitName = defaultLimit;                                              \
        } else {                                                                             \
            out.Scope.limitName = limits.Scope.limitName;                                    \
        }                                                                                    \
    }
    LIMITS(X)
#undef X
    return out;
}

MaybeError ValidateAndUnpackLimitsIn(const Limits* chainedLimits,
                                     const std::unordered_set<wgpu::FeatureName>& supportedFeatures,
                                     CombinedLimits* out) {
    DAWN_ASSERT(chainedLimits != nullptr);
    DAWN_ASSERT(out != nullptr);

    Limits* chainedLimitsPtr = const_cast<Limits*>(chainedLimits);
    UnpackedPtr<Limits> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(chainedLimitsPtr));

    // copy required v1 limits.
    out->v1 = **unpacked;
    out->v1.nextInChain = nullptr;

    if (auto* compatibilityModeLimits = unpacked.Get<CompatibilityModeLimits>()) {
        out->compat = *compatibilityModeLimits;
        out->compat.nextInChain = nullptr;
    }

    // TODO(crbug.com/378361783): Add validation and default values to support requiring limits for
    // DawnTexelCopyBufferRowAlignmentLimits. Test this, see old test removed here:
    // https://dawn-review.googlesource.com/c/dawn/+/240934/11/src/dawn/tests/unittests/native/LimitsTests.cpp#b269
    if (unpacked.Get<DawnTexelCopyBufferRowAlignmentLimits>()) {
        dawn::WarningLog()
            << "DawnTexelCopyBufferRowAlignmentLimits is not supported in required limits";
    }

    if (unpacked.Get<DawnHostMappedPointerLimits>()) {
        dawn::WarningLog() << "DawnHostMappedPointerLimits is not supported in required limits";
    }

    return {};
}

void UnpackLimitsIn(const Limits* chainedLimits, CombinedLimits* out) {
    DAWN_ASSERT(chainedLimits != nullptr);
    DAWN_ASSERT(out != nullptr);

    Limits* chainedLimitsPtr = const_cast<Limits*>(chainedLimits);
    UnpackedPtr<Limits> unpacked = Unpack(chainedLimitsPtr);

    // copy required v1 limits.
    out->v1 = **unpacked;
    out->v1.nextInChain = nullptr;

    if (auto* compatibilityModeLimits = unpacked.Get<CompatibilityModeLimits>()) {
        out->compat = *compatibilityModeLimits;
        out->compat.nextInChain = nullptr;
    }
}

MaybeError ValidateLimits(const CombinedLimits& supportedLimits,
                          const CombinedLimits& requiredLimits) {
#define X(Scope, Class, limitName, ...)                                                           \
    if (!IsLimitUndefined(requiredLimits.Scope.limitName)) {                                      \
        DAWN_TRY_CONTEXT(CheckLimit<LimitClass::Class>::Validate(supportedLimits.Scope.limitName, \
                                                                 requiredLimits.Scope.limitName), \
                         "validating " #limitName);                                               \
    }
    LIMITS(X)
#undef X

    return {};
}

CombinedLimits ApplyLimitTiers(const CombinedLimits& limits) {
    CombinedLimits limitsCopy = limits;
    ApplyLimitTiers(&limitsCopy);
    return limitsCopy;
}

void ApplyLimitTiers(CombinedLimits* limits) {
#define X_TIER_COUNT(Scope, Better, limitName, ...) \
    , std::integer_sequence<uint64_t, __VA_ARGS__>{}.size()
#define GET_TIER_COUNT(LIMIT_GROUP) \
    ReduceSameValue(std::integer_sequence<uint32_t LIMIT_GROUP(X_TIER_COUNT)>{})

#define X_EACH_GROUP(LIMIT_GROUP)                                    \
    {                                                                \
        constexpr uint32_t kTierCount = GET_TIER_COUNT(LIMIT_GROUP); \
        for (uint32_t i = kTierCount; i != 0; --i) {                 \
            LIMIT_GROUP(X_CHECK_BETTER_AND_CLAMP)                    \
            /* Limits fit in tier and have been clamped. Break. */   \
            break;                                                   \
        }                                                            \
    }

#define X_CHECK_BETTER_AND_CLAMP(Scope, Class, limitName, ...)                                  \
    {                                                                                           \
        constexpr std::array<decltype(limits->Scope.limitName), kTierCount> tiers{__VA_ARGS__}; \
        auto tierValue = tiers[i - 1];                                                          \
        if (CheckLimit<LimitClass::Class>::IsBetter(tierValue, limits->Scope.limitName)) {      \
            /* The tier is better. Go to the next tier. */                                      \
            continue;                                                                           \
        } else if (tierValue != limits->Scope.limitName) {                                      \
            /* Better than the tier. Degrade |limits| to the tier. */                           \
            limits->Scope.limitName = tiers[i - 1];                                             \
        }                                                                                       \
    }

    LIMITS_EACH_GROUP(X_EACH_GROUP)

    // After tiering all limit values, enforce additional restriction by calling NormalizeLimits.
    // Since maxStorageBufferBindingSize and maxBufferSize tiers are not exactly aligned, it is
    // possible that tiered maxStorageBufferBindingSize is larger than tiered maxBufferSize. For
    // example, on a hypothetical device with both maxStorageBufferBindingSize and maxBufferSize
    // being 4GB-1, the tiered maxStorageBufferBindingSize would be 4GB-4 while the tiered
    // maxBufferSize being 2GB. NormalizeLimits will clamp the maxStorageBufferBindingSize to
    // maxBufferSize in such cases, although the result may or may not be one of predefined
    // maxStorageBufferBindingSize tiers.
    NormalizeLimits(limits);

#undef X_CHECK_BETTER_AND_CLAMP
#undef X_EACH_GROUP
#undef GET_TIER_COUNT
#undef X_TIER_COUNT
}

#define DAWN_INTERNAL_LIMITS_MEMBER_ASSIGNMENT(type, name) \
    { result.name = limits.name; }
#define DAWN_INTERNAL_LIMITS_FOREACH_MEMBER_ASSIGNMENT(MEMBERS) \
    MEMBERS(DAWN_INTERNAL_LIMITS_MEMBER_ASSIGNMENT)
LimitsForCompilationRequest LimitsForCompilationRequest::Create(const Limits& limits) {
    LimitsForCompilationRequest result;
    DAWN_INTERNAL_LIMITS_FOREACH_MEMBER_ASSIGNMENT(LIMITS_FOR_COMPILATION_REQUEST_MEMBERS)
    return result;
}
LimitsForShaderModuleParseRequest LimitsForShaderModuleParseRequest::Create(const Limits& limits) {
    LimitsForShaderModuleParseRequest result;
    DAWN_INTERNAL_LIMITS_FOREACH_MEMBER_ASSIGNMENT(LIMITS_FOR_SHADER_MODULE_PARSE_REQUEST_MEMBERS)
    return result;
}
#undef DAWN_INTERNAL_LIMITS_FOREACH_MEMBER_ASSIGNMENT
#undef DAWN_INTERNAL_LIMITS_MEMBER_ASSIGNMENT

void NormalizeLimits(CombinedLimits* limits) {
    // Enforce internal Dawn constants for some limits to ensure they don't go over fixed limits
    // in Dawn's internal code.
    limits->v1.maxVertexBufferArrayStride =
        std::min(limits->v1.maxVertexBufferArrayStride, kMaxVertexBufferArrayStride);
    limits->v1.maxColorAttachments =
        std::min(limits->v1.maxColorAttachments, uint32_t(kMaxColorAttachments));
    limits->v1.maxBindGroups = std::min(limits->v1.maxBindGroups, kMaxBindGroups);
    limits->v1.maxBindGroupsPlusVertexBuffers =
        std::min(limits->v1.maxBindGroupsPlusVertexBuffers, kMaxBindGroupsPlusVertexBuffers);
    limits->v1.maxVertexAttributes =
        std::min(limits->v1.maxVertexAttributes, uint32_t(kMaxVertexAttributes));
    limits->v1.maxVertexBuffers =
        std::min(limits->v1.maxVertexBuffers, uint32_t(kMaxVertexBuffers));
    limits->v1.maxSampledTexturesPerShaderStage =
        std::min(limits->v1.maxSampledTexturesPerShaderStage, kMaxSampledTexturesPerShaderStage);
    limits->v1.maxSamplersPerShaderStage =
        std::min(limits->v1.maxSamplersPerShaderStage, kMaxSamplersPerShaderStage);
    limits->v1.maxStorageBuffersPerShaderStage =
        std::min(limits->v1.maxStorageBuffersPerShaderStage, kMaxStorageBuffersPerShaderStage);
    limits->v1.maxStorageTexturesPerShaderStage =
        std::min(limits->v1.maxStorageTexturesPerShaderStage, kMaxStorageTexturesPerShaderStage);
    limits->v1.maxUniformBuffersPerShaderStage =
        std::min(limits->v1.maxUniformBuffersPerShaderStage, kMaxUniformBuffersPerShaderStage);
    limits->v1.maxImmediateSize =
        std::min(limits->v1.maxImmediateSize, kMaxSupportedImmediateDataBytes);

    if (limits->v1.maxDynamicUniformBuffersPerPipelineLayout >
        kMaxDynamicUniformBuffersPerPipelineLayout) {
        dawn::WarningLog() << "maxDynamicUniformBuffersPerPipelineLayout artificially reduced from "
                           << limits->v1.maxDynamicUniformBuffersPerPipelineLayout << " to "
                           << kMaxDynamicUniformBuffersPerPipelineLayout
                           << " to fit dynamic offset allocation limit.";
        limits->v1.maxDynamicUniformBuffersPerPipelineLayout =
            kMaxDynamicUniformBuffersPerPipelineLayout;
    }

    if (limits->v1.maxDynamicStorageBuffersPerPipelineLayout >
        kMaxDynamicStorageBuffersPerPipelineLayout) {
        dawn::WarningLog() << "maxDynamicStorageBuffersPerPipelineLayout artificially reduced from "
                           << limits->v1.maxDynamicStorageBuffersPerPipelineLayout << " to "
                           << kMaxDynamicStorageBuffersPerPipelineLayout
                           << " to fit dynamic offset allocation limit.";
        limits->v1.maxDynamicStorageBuffersPerPipelineLayout =
            kMaxDynamicStorageBuffersPerPipelineLayout;
    }

    limits->v1.maxDynamicUniformBuffersPerPipelineLayout =
        std::min(limits->v1.maxDynamicUniformBuffersPerPipelineLayout,
                 kMaxDynamicUniformBuffersPerPipelineLayout);
    limits->v1.maxDynamicStorageBuffersPerPipelineLayout =
        std::min(limits->v1.maxDynamicStorageBuffersPerPipelineLayout,
                 kMaxDynamicStorageBuffersPerPipelineLayout);
    // Compat limits.
    limits->compat.maxStorageBuffersInVertexStage =
        std::min(limits->compat.maxStorageBuffersInVertexStage, kMaxStorageBuffersPerShaderStage);
    limits->compat.maxStorageTexturesInVertexStage =
        std::min(limits->compat.maxStorageTexturesInVertexStage, kMaxStorageTexturesPerShaderStage);
    limits->compat.maxStorageBuffersInFragmentStage =
        std::min(limits->compat.maxStorageBuffersInFragmentStage, kMaxStorageBuffersPerShaderStage);
    limits->compat.maxStorageTexturesInFragmentStage = std::min(
        limits->compat.maxStorageTexturesInFragmentStage, kMaxStorageTexturesPerShaderStage);

    // Additional enforcement for dependent limits.
    limits->v1.maxStorageBufferBindingSize =
        std::min(limits->v1.maxStorageBufferBindingSize, limits->v1.maxBufferSize);
    limits->v1.maxUniformBufferBindingSize =
        std::min(limits->v1.maxUniformBufferBindingSize, limits->v1.maxBufferSize);
}

void EnforceLimitSpecInvariants(CombinedLimits* limits, wgpu::FeatureLevel featureLevel) {
    // In all feature levels, maxXXXPerStage is raised to maxXXXInStage
    // The reason for this is in compatibility mode, maxXXXPerStage defaults to = 4.
    // That means if the adapter has 8 maxXXXInStage and 8 maxXXXPerStage
    // and you request maxXXXInStage = 3 things work but, if you request
    // maxXXXInStage = 5 they'd fail because suddenly you're you'd also be required
    // to request maxXXXPerStage to 5. So, we auto-uprade the perStage limits.
    limits->v1.maxStorageBuffersPerShaderStage = Max(
        limits->v1.maxStorageBuffersPerShaderStage, limits->compat.maxStorageBuffersInVertexStage,
        limits->compat.maxStorageBuffersInFragmentStage);
    limits->v1.maxStorageTexturesPerShaderStage = Max(
        limits->v1.maxStorageTexturesPerShaderStage, limits->compat.maxStorageTexturesInVertexStage,
        limits->compat.maxStorageTexturesInFragmentStage);

    if (featureLevel != wgpu::FeatureLevel::Compatibility) {
        // In core mode the maxStorageXXXInYYYStage are always set to maxStorageXXXPerShaderStage
        // In compat they can vary but validation:
        //   In compat, user requests 3 and 5 respectively so result:
        //     device.limits.maxStorageBuffersInFragmentStage = 3;
        //     device.limits.maxStorageBuffersPerShaderStage = 5;
        //     It's ok to use 3 storage buffers in fragment stage but fails if 4 used.
        //   In core, user requests 3 and 5 respectively so result:
        //     device.limits.maxStorageBuffersInFragmentStage = 5;
        //     device.limits.maxStorageBuffersPerShaderStage = 5;
        //     It's ok to use 5 storage buffers in fragment stage because in core
        //     we originally only had maxStorageBuffersPerShaderStage
        limits->compat.maxStorageBuffersInFragmentStage =
            limits->v1.maxStorageBuffersPerShaderStage;
        limits->compat.maxStorageTexturesInFragmentStage =
            limits->v1.maxStorageTexturesPerShaderStage;
        limits->compat.maxStorageBuffersInVertexStage = limits->v1.maxStorageBuffersPerShaderStage;
        limits->compat.maxStorageTexturesInVertexStage =
            limits->v1.maxStorageTexturesPerShaderStage;
    }
}

MaybeError FillLimits(Limits* outputLimits,
                      const FeaturesSet& supportedFeatures,
                      const CombinedLimits& combinedLimits) {
    DAWN_ASSERT(outputLimits != nullptr);
    UnpackedPtr<Limits> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(outputLimits));
    {
        wgpu::ChainedStructOut* originalChain = unpacked->nextInChain;
        **unpacked = combinedLimits.v1;
        // Recover original chain.
        unpacked->nextInChain = originalChain;
    }

    if (auto* compatibilityModeLimits = unpacked.Get<CompatibilityModeLimits>()) {
        wgpu::ChainedStructOut* originalChain = compatibilityModeLimits->nextInChain;
        *compatibilityModeLimits = combinedLimits.compat;

        // Recover original chain.
        compatibilityModeLimits->nextInChain = originalChain;
    }

    if (auto* texelCopyBufferRowAlignmentLimits =
            unpacked.Get<DawnTexelCopyBufferRowAlignmentLimits>()) {
        wgpu::ChainedStructOut* originalChain = texelCopyBufferRowAlignmentLimits->nextInChain;
        if (!supportedFeatures.IsEnabled(wgpu::FeatureName::DawnTexelCopyBufferRowAlignment)) {
            // If the feature is not enabled, minTexelCopyBufferRowAlignment is default-initialized
            // to WGPU_LIMIT_U32_UNDEFINED.
            *texelCopyBufferRowAlignmentLimits = DawnTexelCopyBufferRowAlignmentLimits{};
        } else {
            *texelCopyBufferRowAlignmentLimits = combinedLimits.texelCopyBufferRowAlignmentLimits;
        }

        // Recover original chain.
        texelCopyBufferRowAlignmentLimits->nextInChain = originalChain;
    }

    if (auto* hostMappedPointerLimits = unpacked.Get<DawnHostMappedPointerLimits>()) {
        wgpu::ChainedStructOut* originalChain = hostMappedPointerLimits->nextInChain;
        if (!supportedFeatures.IsEnabled(wgpu::FeatureName::HostMappedPointer)) {
            // If the feature is not enabled, hostMappedPointerAlignment is default-initialized to
            // WGPU_LIMIT_U32_UNDEFINED.
            *hostMappedPointerLimits = DawnHostMappedPointerLimits{};
        } else {
            hostMappedPointerLimits->hostMappedPointerAlignment =
                combinedLimits.hostMappedPointerLimits.hostMappedPointerAlignment;
        }

        // Recover original chain.
        hostMappedPointerLimits->nextInChain = originalChain;
    }

    return {};
}

}  // namespace dawn::native
