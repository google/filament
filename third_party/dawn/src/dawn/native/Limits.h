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

#ifndef SRC_DAWN_NATIVE_LIMITS_H_
#define SRC_DAWN_NATIVE_LIMITS_H_

#include <unordered_set>

#include "dawn/native/ChainUtils.h"
#include "dawn/native/Error.h"
#include "dawn/native/Features.h"
#include "dawn/native/Serializable.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

// TODO(crbug.com/421950205): Replace this with dawn::utils::ComboLimits.
struct CombinedLimits {
    Limits v1;
    CompatibilityModeLimits compat;
    DawnHostMappedPointerLimits hostMappedPointerLimits;
    DawnTexelCopyBufferRowAlignmentLimits texelCopyBufferRowAlignmentLimits;
};

// Populate |limits| with the default limits.
void GetDefaultLimits(CombinedLimits* limits, wgpu::FeatureLevel featureLevel);

// Returns a copy of |limits| where all undefined values are replaced
// with their defaults. Also clamps to the defaults if the provided limits
// are worse.
CombinedLimits ReifyDefaultLimits(const CombinedLimits& limits, wgpu::FeatureLevel featureLevel);

// Fixup limits after device creation
void EnforceLimitSpecInvariants(CombinedLimits* limits, wgpu::FeatureLevel featureLevel);

// Validate that |requiredLimits| are no better than |supportedLimits|.
MaybeError ValidateLimits(const CombinedLimits& supportedLimits,
                          const CombinedLimits& requiredLimits);

// Validtate that the |chainedLimits| are valid and unpack them into |out|.
MaybeError ValidateAndUnpackLimitsIn(const Limits* chainedLimits,
                                     const std::unordered_set<wgpu::FeatureName>& supportedFeatures,
                                     CombinedLimits* out);

// Unpack |chainedLimits| into |out|.
void UnpackLimitsIn(const Limits* chainedLimits, CombinedLimits* out);

// Apply limit tiers to |limits|
void ApplyLimitTiers(CombinedLimits* limits);

// Returns a copy of |limits| where limit tiers are applied.
CombinedLimits ApplyLimitTiers(const CombinedLimits& limits);

// If there are new limit member needed at shader compilation time
// Simply append a new X(type, name) here.
#define LIMITS_FOR_COMPILATION_REQUEST_MEMBERS(X)  \
    X(uint32_t, maxComputeWorkgroupSizeX)          \
    X(uint32_t, maxComputeWorkgroupSizeY)          \
    X(uint32_t, maxComputeWorkgroupSizeZ)          \
    X(uint32_t, maxComputeInvocationsPerWorkgroup) \
    X(uint32_t, maxComputeWorkgroupStorageSize)

DAWN_SERIALIZABLE(struct, LimitsForCompilationRequest, LIMITS_FOR_COMPILATION_REQUEST_MEMBERS) {
    static LimitsForCompilationRequest Create(const Limits& limits);
};

#define LIMITS_FOR_SHADER_MODULE_PARSE_REQUEST_MEMBERS(X) \
    X(uint32_t, maxVertexAttributes)                      \
    X(uint32_t, maxInterStageShaderVariables)             \
    X(uint32_t, maxColorAttachments)

DAWN_SERIALIZABLE(struct,
                  LimitsForShaderModuleParseRequest,
                  LIMITS_FOR_SHADER_MODULE_PARSE_REQUEST_MEMBERS) {
    static LimitsForShaderModuleParseRequest Create(const Limits& limits);
};

// Enforce restriction for limit values, including:
//   1. Enforce internal Dawn constants for some limits to ensure they don't go over fixed-size
//      arrays in Dawn's internal code;
//   2. Additional enforcement for dependent limits, e.g. maxStorageBufferBindingSize and
//      maxUniformBufferBindingSize must not be larger than maxBufferSize.
void NormalizeLimits(CombinedLimits* limits);

// Fill |outputLimits| with |supportedFeatures| and |combinedLimits|.
MaybeError FillLimits(Limits* outputLimits,
                      const FeaturesSet& supportedFeatures,
                      const CombinedLimits& combinedLimits);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_LIMITS_H_
