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

#include "dawn/native/Error.h"
#include "dawn/native/Features.h"
#include "dawn/native/VisitableMembers.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

struct CombinedLimits {
    Limits v1;
    DawnExperimentalSubgroupLimits experimentalSubgroupLimits;
    DawnExperimentalImmediateDataLimits experimentalImmediateDataLimits;
    DawnTexelCopyBufferRowAlignmentLimits texelCopyBufferRowAlignmentLimits;
};

// Populate |limits| with the default limits.
void GetDefaultLimits(Limits* limits, wgpu::FeatureLevel featureLevel);

// Returns a copy of |limits| where all undefined values are replaced
// with their defaults. Also clamps to the defaults if the provided limits
// are worse.
Limits ReifyDefaultLimits(const Limits& limits, wgpu::FeatureLevel featureLevel);

// Validate that |requiredLimits| are no better than |supportedLimits|.
MaybeError ValidateLimits(wgpu::FeatureLevel featureLevel,
                          const Limits& supportedLimits,
                          const Limits& requiredLimits);

// Returns a copy of |limits| where limit tiers are applied.
Limits ApplyLimitTiers(Limits limits);

// If there are new limit member needed at shader compilation time
// Simply append a new X(type, name) here.
#define LIMITS_FOR_COMPILATION_REQUEST_MEMBERS(X)  \
    X(uint32_t, maxComputeWorkgroupSizeX)          \
    X(uint32_t, maxComputeWorkgroupSizeY)          \
    X(uint32_t, maxComputeWorkgroupSizeZ)          \
    X(uint32_t, maxComputeInvocationsPerWorkgroup) \
    X(uint32_t, maxComputeWorkgroupStorageSize)

struct LimitsForCompilationRequest {
    static LimitsForCompilationRequest Create(const Limits& limits);
    DAWN_VISITABLE_MEMBERS(LIMITS_FOR_COMPILATION_REQUEST_MEMBERS)
};

// Enforce restriction for limit values, including:
//   1. Enforce internal Dawn constants for some limits to ensure they don't go over fixed-size
//      arrays in Dawn's internal code;
//   2. Additional enforcement for dependent limits, e.g. maxStorageBufferBindingSize and
//      maxUniformBufferBindingSize must not be larger than maxBufferSize.
void NormalizeLimits(Limits* limits);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_LIMITS_H_
