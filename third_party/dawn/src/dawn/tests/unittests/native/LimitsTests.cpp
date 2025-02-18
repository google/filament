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

#include <gtest/gtest.h>

#include "dawn/common/Constants.h"
#include "dawn/native/Limits.h"

namespace dawn {
namespace native {

// Test |GetDefaultLimits| returns the default for FeatureLeveL::Core.
TEST(Limits, GetDefaultLimits) {
    Limits limits = {};
    EXPECT_NE(limits.maxBindGroups, 4u);

    GetDefaultLimits(&limits, wgpu::FeatureLevel::Core);

    EXPECT_EQ(limits.maxBindGroups, 4u);
}

// Test |GetDefaultLimits| returns the default for FeatureLeveL::Compatibility.
// Compatibility default limits are lower than Core.
TEST(Limits, GetDefaultLimits_Compat) {
    Limits limits = {};
    EXPECT_NE(limits.maxColorAttachments, 4u);

    GetDefaultLimits(&limits, wgpu::FeatureLevel::Compatibility);

    EXPECT_EQ(limits.maxColorAttachments, 4u);
}

// Test |ReifyDefaultLimits| populates the default for wgpu::FeatureLevel::Core
// if values are undefined.
TEST(Limits, ReifyDefaultLimits_PopulatesDefault) {
    Limits limits;
    limits.maxComputeWorkgroupStorageSize = wgpu::kLimitU32Undefined;
    limits.maxStorageBufferBindingSize = wgpu::kLimitU64Undefined;

    Limits reified = ReifyDefaultLimits(limits, wgpu::FeatureLevel::Core);
    EXPECT_EQ(reified.maxComputeWorkgroupStorageSize, 16384u);
    EXPECT_EQ(reified.maxStorageBufferBindingSize, 134217728ul);
    EXPECT_EQ(reified.maxStorageBuffersInFragmentStage, 8u);
    EXPECT_EQ(reified.maxStorageTexturesInFragmentStage, 4u);
    EXPECT_EQ(reified.maxStorageBuffersInVertexStage, 8u);
    EXPECT_EQ(reified.maxStorageTexturesInVertexStage, 4u);
}

// Test |ReifyDefaultLimits| populates the default for wgpu::FeatureLevel::Compatibility
// if values are undefined. Compatibility default limits are lower than Core.
TEST(Limits, ReifyDefaultLimits_PopulatesDefault_Compat) {
    Limits limits;
    limits.maxTextureDimension1D = wgpu::kLimitU32Undefined;
    limits.maxStorageBufferBindingSize = wgpu::kLimitU64Undefined;

    Limits reified = ReifyDefaultLimits(limits, wgpu::FeatureLevel::Compatibility);
    EXPECT_EQ(reified.maxTextureDimension1D, 4096u);
    EXPECT_EQ(reified.maxStorageBufferBindingSize, 134217728ul);
    EXPECT_EQ(reified.maxStorageBuffersInFragmentStage, 0u);
    EXPECT_EQ(reified.maxStorageTexturesInFragmentStage, 0u);
    EXPECT_EQ(reified.maxStorageBuffersInVertexStage, 0u);
    EXPECT_EQ(reified.maxStorageTexturesInVertexStage, 0u);
}

// Test |ReifyDefaultLimits| clamps to the default if
// values are worse than the default.
TEST(Limits, ReifyDefaultLimits_Clamps) {
    Limits limits;
    limits.maxStorageBuffersPerShaderStage = 4;
    limits.minUniformBufferOffsetAlignment = 512;

    Limits reified = ReifyDefaultLimits(limits, wgpu::FeatureLevel::Core);
    EXPECT_EQ(reified.maxStorageBuffersPerShaderStage, 8u);
    EXPECT_EQ(reified.minUniformBufferOffsetAlignment, 256u);
}

// Test |ValidateLimits| works to validate limits are not better
// than supported.
TEST(Limits, ValidateLimits) {
    const wgpu::FeatureLevel featureLevel = wgpu::FeatureLevel::Core;
    // Start with the default for supported.
    Limits defaults;
    GetDefaultLimits(&defaults, featureLevel);

    // Test supported == required is valid.
    {
        Limits required = defaults;
        EXPECT_TRUE(ValidateLimits(featureLevel, defaults, required).IsSuccess());
    }

    // Test supported == required is valid, when they are not default.
    {
        Limits supported = defaults;
        Limits required = defaults;
        supported.maxBindGroups += 1;
        required.maxBindGroups += 1;
        EXPECT_TRUE(ValidateLimits(featureLevel, supported, required).IsSuccess());
    }

    // Test that default-initialized (all undefined) is valid.
    {
        Limits required = {};
        EXPECT_TRUE(ValidateLimits(featureLevel, defaults, required).IsSuccess());
    }

    // Test that better than supported is invalid for "maximum" limits.
    {
        Limits required = {};
        required.maxTextureDimension3D = defaults.maxTextureDimension3D + 1;
        MaybeError err = ValidateLimits(featureLevel, defaults, required);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }

    // Test that worse than supported is valid for "maximum" limits.
    {
        Limits required = {};
        required.maxComputeWorkgroupSizeX = defaults.maxComputeWorkgroupSizeX - 1;
        EXPECT_TRUE(ValidateLimits(featureLevel, defaults, required).IsSuccess());
    }

    // Test that better than min is invalid for "alignment" limits.
    {
        Limits required = {};
        required.minUniformBufferOffsetAlignment = defaults.minUniformBufferOffsetAlignment / 2;
        MaybeError err = ValidateLimits(featureLevel, defaults, required);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }

    // Test that worse than min and a power of two is valid for "alignment" limits.
    {
        Limits required = {};
        required.minStorageBufferOffsetAlignment = defaults.minStorageBufferOffsetAlignment * 2;
        EXPECT_TRUE(ValidateLimits(featureLevel, defaults, required).IsSuccess());
    }

    // Test that worse than min and not a power of two is invalid for "alignment" limits.
    {
        Limits required = {};
        required.minStorageBufferOffsetAlignment = defaults.minStorageBufferOffsetAlignment * 3;
        MaybeError err = ValidateLimits(featureLevel, defaults, required);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }
}

// Test that maxStorage(Buffers|Textures)In(Fragment/Vertex)Stage
// must be less than the requested dependent maxStorage(Buffers|Textures)PerShaderStage
TEST(Limits, PerStageDependentLimitsRequested) {
    const wgpu::FeatureLevel featureLevel = wgpu::FeatureLevel::Core;
    // Start with the default for supported.
    Limits defaults;
    GetDefaultLimits(&defaults, featureLevel);

    // Test at least one works.
    {
        Limits required = {};
        required.maxStorageBuffersInFragmentStage = 2;
        required.maxStorageBuffersPerShaderStage = 2;
        EXPECT_TRUE(ValidateLimits(featureLevel, defaults, required).IsSuccess());
    }

    // Test maxStorageBuffersInFragmentStage fails if greater than
    // requested maxStorageBuffersPerShaderStage
    {
        Limits required = {};
        required.maxStorageBuffersInFragmentStage = 2;
        required.maxStorageBuffersPerShaderStage = 1;
        MaybeError err = ValidateLimits(featureLevel, defaults, required);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }

    // Test maxStorageBuffersInVertexStage fails if greater than
    // requested maxStorageBuffersPerShaderStage
    {
        Limits required = {};
        required.maxStorageBuffersInVertexStage = 2;
        required.maxStorageBuffersPerShaderStage = 1;
        MaybeError err = ValidateLimits(featureLevel, defaults, required);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }

    // Test maxStorageTexturesInFragmentStage fails if greater than
    // requested maxStorageTexturesPerShaderStage
    {
        Limits required = {};
        required.maxStorageTexturesInFragmentStage = 2;
        required.maxStorageTexturesPerShaderStage = 1;
        MaybeError err = ValidateLimits(featureLevel, defaults, required);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }

    // Test maxStorageTexturesInVertexStage fails if greater than requested
    // maxStorageTexturesPerShaderStage
    {
        Limits required = {};
        required.maxStorageTexturesInVertexStage = 2;
        required.maxStorageTexturesPerShaderStage = 1;
        MaybeError err = ValidateLimits(featureLevel, defaults, required);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }
}

// Test that maxStorage(Buffers|Textures)In(Fragment/Vertex)Stage
// must be less than the default dependent maxStorage(Buffers|Textures)PerShaderStage
// if the dependent limit is not requested.
TEST(Limits, PerStageDependentLimitsDefault) {
    const wgpu::FeatureLevel featureLevel = wgpu::FeatureLevel::Core;
    // Start with the default for supported.
    Limits supported;
    GetDefaultLimits(&supported, featureLevel);

    const uint32_t kLimit = 100;

    // set the supported higher than the default
    supported.maxStorageBuffersInFragmentStage = kLimit;
    supported.maxStorageBuffersInVertexStage = kLimit;
    supported.maxStorageBuffersPerShaderStage = kLimit;
    supported.maxStorageTexturesInFragmentStage = kLimit;
    supported.maxStorageTexturesInVertexStage = kLimit;
    supported.maxStorageTexturesPerShaderStage = kLimit;

    // Check at least one works when limits match.
    {
        Limits required = {};
        required.maxStorageBuffersInFragmentStage = kLimit;
        required.maxStorageBuffersPerShaderStage = kLimit;
        EXPECT_TRUE(ValidateLimits(featureLevel, supported, required).IsSuccess());
    }

    // Test maxStorageBuffersInFragmentStage fails if greater than
    // requested maxStorageBuffersPerShaderStage
    {
        Limits required = {};
        required.maxStorageBuffersInFragmentStage = kLimit;
        MaybeError err = ValidateLimits(featureLevel, supported, required);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }

    // Test maxStorageBuffersInVertexStage fails if greater than
    // requested maxStorageBuffersPerShaderStage
    {
        Limits required = {};
        required.maxStorageBuffersInVertexStage = kLimit;
        MaybeError err = ValidateLimits(featureLevel, supported, required);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }

    // Test maxStorageTexturesInFragmentStage fails if greater than
    // requested maxStorageTexturesPerShaderStage
    {
        Limits required = {};
        required.maxStorageTexturesInFragmentStage = kLimit;
        MaybeError err = ValidateLimits(featureLevel, supported, required);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }

    // Test maxStorageTexturesInVertexStage fails if greater than requested
    // maxStorageTexturesPerShaderStage
    {
        Limits required = {};
        required.maxStorageTexturesInVertexStage = kLimit;
        MaybeError err = ValidateLimits(featureLevel, supported, required);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }
}

// Test that |ApplyLimitTiers| degrades limits to the next best tier.
TEST(Limits, ApplyLimitTiers) {
    auto SetLimitsStorageBufferBindingSizeTier2 = [](Limits* limits) {
        // Tier 2 of maxStorageBufferBindingSize is 1GB
        limits->maxStorageBufferBindingSize = 1073741824;
        // Also set the maxBufferSize to be large enough, as ApplyLimitTiers ensures tired
        // maxStorageBufferBindingSize no larger than tiered maxBufferSize.
        limits->maxBufferSize = 2147483648;
    };
    Limits limitsStorageBufferBindingSizeTier2;
    GetDefaultLimits(&limitsStorageBufferBindingSizeTier2, wgpu::FeatureLevel::Core);
    SetLimitsStorageBufferBindingSizeTier2(&limitsStorageBufferBindingSizeTier2);

    auto SetLimitsStorageBufferBindingSizeTier3 = [](Limits* limits) {
        // Tier 3 of maxStorageBufferBindingSize is 2GB-4
        limits->maxStorageBufferBindingSize = 2147483644;
        // Also set the maxBufferSize to be large enough, as ApplyLimitTiers ensures tired
        // maxStorageBufferBindingSize no larger than tiered maxBufferSize.
        limits->maxBufferSize = 2147483648;
    };
    Limits limitsStorageBufferBindingSizeTier3;
    GetDefaultLimits(&limitsStorageBufferBindingSizeTier3, wgpu::FeatureLevel::Core);
    SetLimitsStorageBufferBindingSizeTier3(&limitsStorageBufferBindingSizeTier3);

    auto SetLimitsComputeWorkgroupStorageSizeTier1 = [](Limits* limits) {
        limits->maxComputeWorkgroupStorageSize = 16384;
    };
    Limits limitsComputeWorkgroupStorageSizeTier1;
    GetDefaultLimits(&limitsComputeWorkgroupStorageSizeTier1, wgpu::FeatureLevel::Core);
    SetLimitsComputeWorkgroupStorageSizeTier1(&limitsComputeWorkgroupStorageSizeTier1);

    auto SetLimitsComputeWorkgroupStorageSizeTier3 = [](Limits* limits) {
        limits->maxComputeWorkgroupStorageSize = 65536;
    };
    Limits limitsComputeWorkgroupStorageSizeTier3;
    GetDefaultLimits(&limitsComputeWorkgroupStorageSizeTier3, wgpu::FeatureLevel::Core);
    SetLimitsComputeWorkgroupStorageSizeTier3(&limitsComputeWorkgroupStorageSizeTier3);

    // Test that applying tiers to limits that are exactly
    // equal to a tier returns the same values.
    {
        Limits limits = limitsStorageBufferBindingSizeTier2;
        EXPECT_EQ(ApplyLimitTiers(limits), limits);

        limits = limitsStorageBufferBindingSizeTier3;
        EXPECT_EQ(ApplyLimitTiers(limits), limits);
    }

    // Test all limits slightly worse than tier 3.
    {
        Limits limits = limitsStorageBufferBindingSizeTier3;
        limits.maxStorageBufferBindingSize -= 1;
        EXPECT_EQ(ApplyLimitTiers(limits), limitsStorageBufferBindingSizeTier2);
    }

    // Test that limits may match one tier exactly and be degraded in another tier.
    // Degrading to one tier does not affect the other tier.
    {
        Limits limits = limitsComputeWorkgroupStorageSizeTier3;
        // Set tier 3 and change one limit to be insufficent.
        SetLimitsStorageBufferBindingSizeTier3(&limits);
        limits.maxStorageBufferBindingSize -= 1;

        Limits tiered = ApplyLimitTiers(limits);

        // Check that |tiered| has the limits of memorySize tier 2
        Limits tieredWithMemorySizeTier2 = tiered;
        SetLimitsStorageBufferBindingSizeTier2(&tieredWithMemorySizeTier2);
        EXPECT_EQ(tiered, tieredWithMemorySizeTier2);

        // Check that |tiered| has the limits of bindingSpace tier 3
        Limits tieredWithBindingSpaceTier3 = tiered;
        SetLimitsComputeWorkgroupStorageSizeTier3(&tieredWithBindingSpaceTier3);
        EXPECT_EQ(tiered, tieredWithBindingSpaceTier3);
    }

    // Test that limits may be simultaneously degraded in two tiers independently.
    {
        Limits limits;
        GetDefaultLimits(&limits, wgpu::FeatureLevel::Core);
        SetLimitsComputeWorkgroupStorageSizeTier3(&limits);
        SetLimitsStorageBufferBindingSizeTier3(&limits);
        limits.maxComputeWorkgroupStorageSize =
            limitsComputeWorkgroupStorageSizeTier1.maxComputeWorkgroupStorageSize + 1;
        limits.maxStorageBufferBindingSize =
            limitsStorageBufferBindingSizeTier2.maxStorageBufferBindingSize + 1;

        Limits tiered = ApplyLimitTiers(limits);

        Limits expected = tiered;
        SetLimitsComputeWorkgroupStorageSizeTier1(&expected);
        SetLimitsStorageBufferBindingSizeTier2(&expected);
        EXPECT_EQ(tiered, expected);
    }
}

// Test that |ApplyLimitTiers| will hold the maxStorageBufferBindingSize no larger than
// maxBufferSize restriction.
TEST(Limits, TieredMaxStorageBufferBindingSizeNoLargerThanMaxBufferSize) {
    // Start with the default for supported.
    Limits defaults;
    GetDefaultLimits(&defaults, wgpu::FeatureLevel::Core);

    // Test reported maxStorageBufferBindingSize around 128MB, 1GB, 2GB-4 and 4GB-4.
    constexpr uint64_t storageSizeTier1 = 134217728ull;   // 128MB
    constexpr uint64_t storageSizeTier2 = 1073741824ull;  // 1GB
    constexpr uint64_t storageSizeTier3 = 2147483644ull;  // 2GB-4
    constexpr uint64_t storageSizeTier4 = 4294967292ull;  // 4GB-4
    constexpr uint64_t possibleReportedMaxStorageBufferBindingSizes[] = {
        storageSizeTier1,     storageSizeTier1 + 1, storageSizeTier2 - 1, storageSizeTier2,
        storageSizeTier2 + 1, storageSizeTier3 - 1, storageSizeTier3,     storageSizeTier3 + 1,
        storageSizeTier4 - 1, storageSizeTier4,     storageSizeTier4 + 1};
    // Test reported maxBufferSize around 256MB, 1GB, 2GB and 4GB, and a large 256GB.
    constexpr uint64_t bufferSizeTier1 = 0x10000000ull;    // 256MB
    constexpr uint64_t bufferSizeTier2 = 0x40000000ull;    // 1GB
    constexpr uint64_t bufferSizeTier3 = 0x80000000ull;    // 2GB
    constexpr uint64_t bufferSizeTier4 = 0x100000000ull;   // 4GB
    constexpr uint64_t bufferSizeLarge = 0x4000000000ull;  // 256GB
    constexpr uint64_t possibleReportedMaxBufferSizes[] = {
        bufferSizeTier1,     bufferSizeTier1 + 1, bufferSizeTier2 - 1, bufferSizeTier2,
        bufferSizeTier2 + 1, bufferSizeTier3 - 1, bufferSizeTier3,     bufferSizeTier3 + 1,
        bufferSizeTier4 - 1, bufferSizeTier4,     bufferSizeTier4 + 1, bufferSizeLarge};

    // Test that tiered maxStorageBufferBindingSize is no larger than tiered maxBufferSize.
    for (uint64_t reportedMaxStorageBufferBindingSizes :
         possibleReportedMaxStorageBufferBindingSizes) {
        for (uint64_t reportedMaxBufferSizes : possibleReportedMaxBufferSizes) {
            Limits limits = defaults;
            limits.maxStorageBufferBindingSize = reportedMaxStorageBufferBindingSizes;
            limits.maxBufferSize = reportedMaxBufferSizes;

            Limits tiered = ApplyLimitTiers(limits);

            EXPECT_LE(tiered.maxStorageBufferBindingSize, tiered.maxBufferSize);
        }
    }
}

// Test that |ApplyLimitTiers| will hold the maxUniformBufferBindingSize no larger than
// maxBufferSize restriction.
TEST(Limits, TieredMaxUniformBufferBindingSizeNoLargerThanMaxBufferSize) {
    // Start with the default for supported.
    Limits defaults;
    GetDefaultLimits(&defaults, wgpu::FeatureLevel::Core);

    // Test reported maxStorageBufferBindingSize around 64KB, and a large 1GB.
    constexpr uint64_t uniformSizeTier1 = 65536ull;       // 64KB
    constexpr uint64_t uniformSizeLarge = 1073741824ull;  // 1GB
    constexpr uint64_t possibleReportedMaxUniformBufferBindingSizes[] = {
        uniformSizeTier1, uniformSizeTier1 + 1, uniformSizeLarge};
    // Test reported maxBufferSize around 256MB, 1GB, 2GB and 4GB, and a large 256GB.
    constexpr uint64_t bufferSizeTier1 = 0x10000000ull;    // 256MB
    constexpr uint64_t bufferSizeTier2 = 0x40000000ull;    // 1GB
    constexpr uint64_t bufferSizeTier3 = 0x80000000ull;    // 2GB
    constexpr uint64_t bufferSizeTier4 = 0x100000000ull;   // 4GB
    constexpr uint64_t bufferSizeLarge = 0x4000000000ull;  // 256GB
    constexpr uint64_t possibleReportedMaxBufferSizes[] = {
        bufferSizeTier1,     bufferSizeTier1 + 1, bufferSizeTier2 - 1, bufferSizeTier2,
        bufferSizeTier2 + 1, bufferSizeTier3 - 1, bufferSizeTier3,     bufferSizeTier3 + 1,
        bufferSizeTier4 - 1, bufferSizeTier4,     bufferSizeTier4 + 1, bufferSizeLarge};

    // Test that tiered maxUniformBufferBindingSize is no larger than tiered maxBufferSize.
    for (uint64_t reportedMaxUniformBufferBindingSizes :
         possibleReportedMaxUniformBufferBindingSizes) {
        for (uint64_t reportedMaxBufferSizes : possibleReportedMaxBufferSizes) {
            Limits limits = defaults;
            limits.maxUniformBufferBindingSize = reportedMaxUniformBufferBindingSizes;
            limits.maxBufferSize = reportedMaxBufferSizes;

            Limits tiered = ApplyLimitTiers(limits);

            EXPECT_LE(tiered.maxUniformBufferBindingSize, tiered.maxBufferSize);
        }
    }
}

// Test |NormalizeLimits| works to enforce restriction of limits.
TEST(Limits, NormalizeLimits) {
    // Start with the default for supported.
    Limits defaults;
    GetDefaultLimits(&defaults, wgpu::FeatureLevel::Core);

    // Test specific limit values are clamped to internal Dawn constants.
    {
        Limits limits = defaults;
        limits.maxVertexBufferArrayStride = kMaxVertexBufferArrayStride + 1;
        limits.maxColorAttachments = uint32_t(kMaxColorAttachments) + 1;
        limits.maxBindGroups = kMaxBindGroups + 1;
        limits.maxBindGroupsPlusVertexBuffers = kMaxBindGroupsPlusVertexBuffers + 1;
        limits.maxVertexAttributes = uint32_t(kMaxVertexAttributes) + 1;
        limits.maxVertexBuffers = uint32_t(kMaxVertexBuffers) + 1;
        limits.maxSampledTexturesPerShaderStage = kMaxSampledTexturesPerShaderStage + 1;
        limits.maxSamplersPerShaderStage = kMaxSamplersPerShaderStage + 1;
        limits.maxStorageBuffersPerShaderStage = kMaxStorageBuffersPerShaderStage + 1;
        limits.maxStorageTexturesPerShaderStage = kMaxStorageTexturesPerShaderStage + 1;
        limits.maxUniformBuffersPerShaderStage = kMaxUniformBuffersPerShaderStage + 1;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxVertexBufferArrayStride, kMaxVertexBufferArrayStride);
        EXPECT_EQ(limits.maxColorAttachments, uint32_t(kMaxColorAttachments));
        EXPECT_EQ(limits.maxBindGroups, kMaxBindGroups);
        EXPECT_EQ(limits.maxBindGroupsPlusVertexBuffers, kMaxBindGroupsPlusVertexBuffers);
        EXPECT_EQ(limits.maxVertexAttributes, uint32_t(kMaxVertexAttributes));
        EXPECT_EQ(limits.maxVertexBuffers, uint32_t(kMaxVertexBuffers));
        EXPECT_EQ(limits.maxSampledTexturesPerShaderStage, kMaxSampledTexturesPerShaderStage);
        EXPECT_EQ(limits.maxSamplersPerShaderStage, kMaxSamplersPerShaderStage);
        EXPECT_EQ(limits.maxStorageBuffersPerShaderStage, kMaxStorageBuffersPerShaderStage);
        EXPECT_EQ(limits.maxStorageTexturesPerShaderStage, kMaxStorageTexturesPerShaderStage);
        EXPECT_EQ(limits.maxUniformBuffersPerShaderStage, kMaxUniformBuffersPerShaderStage);
    }

    // Test maxStorageBufferBindingSize is clamped to maxBufferSize.
    // maxStorageBufferBindingSize is no larger than maxBufferSize
    {
        constexpr uint64_t reportedMaxBufferSize = 2147483648;
        constexpr uint64_t reportedMaxStorageBufferBindingSize = reportedMaxBufferSize;
        Limits limits = defaults;
        limits.maxStorageBufferBindingSize = reportedMaxStorageBufferBindingSize;
        limits.maxBufferSize = reportedMaxBufferSize;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxBufferSize, reportedMaxBufferSize);
        EXPECT_EQ(limits.maxStorageBufferBindingSize, reportedMaxStorageBufferBindingSize);
    }
    {
        constexpr uint64_t reportedMaxBufferSize = 2147483648;
        constexpr uint64_t reportedMaxStorageBufferBindingSize = reportedMaxBufferSize - 1;
        Limits limits = defaults;
        limits.maxStorageBufferBindingSize = reportedMaxStorageBufferBindingSize;
        limits.maxBufferSize = reportedMaxBufferSize;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxBufferSize, reportedMaxBufferSize);
        EXPECT_EQ(limits.maxStorageBufferBindingSize, reportedMaxStorageBufferBindingSize);
    }
    // maxStorageBufferBindingSize is equal to maxBufferSize+1, expect clamping to maxBufferSize
    {
        constexpr uint64_t reportedMaxBufferSize = 2147483648;
        constexpr uint64_t reportedMaxStorageBufferBindingSize = reportedMaxBufferSize + 1;
        Limits limits = defaults;
        limits.maxStorageBufferBindingSize = reportedMaxStorageBufferBindingSize;
        limits.maxBufferSize = reportedMaxBufferSize;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxBufferSize, reportedMaxBufferSize);
        EXPECT_EQ(limits.maxStorageBufferBindingSize, reportedMaxBufferSize);
    }
    // maxStorageBufferBindingSize is much larger than maxBufferSize, expect clamping to
    // maxBufferSize
    {
        constexpr uint64_t reportedMaxBufferSize = 2147483648;
        constexpr uint64_t reportedMaxStorageBufferBindingSize = 4294967295;
        Limits limits = defaults;
        limits.maxStorageBufferBindingSize = reportedMaxStorageBufferBindingSize;
        limits.maxBufferSize = reportedMaxBufferSize;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxBufferSize, reportedMaxBufferSize);
        EXPECT_EQ(limits.maxStorageBufferBindingSize, reportedMaxBufferSize);
    }

    // Test maxUniformBufferBindingSize is clamped to maxBufferSize.
    // maxUniformBufferBindingSize is no larger than maxBufferSize
    {
        constexpr uint64_t reportedMaxBufferSize = 2147483648;
        constexpr uint64_t reportedMaxUniformBufferBindingSize = reportedMaxBufferSize - 1;
        Limits limits = defaults;
        limits.maxUniformBufferBindingSize = reportedMaxUniformBufferBindingSize;
        limits.maxBufferSize = reportedMaxBufferSize;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxBufferSize, reportedMaxBufferSize);
        EXPECT_EQ(limits.maxUniformBufferBindingSize, reportedMaxUniformBufferBindingSize);
    }
    {
        constexpr uint64_t reportedMaxBufferSize = 2147483648;
        constexpr uint64_t reportedMaxUniformBufferBindingSize = reportedMaxBufferSize;
        Limits limits = defaults;
        limits.maxUniformBufferBindingSize = reportedMaxUniformBufferBindingSize;
        limits.maxBufferSize = reportedMaxBufferSize;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxBufferSize, reportedMaxBufferSize);
        EXPECT_EQ(limits.maxUniformBufferBindingSize, reportedMaxUniformBufferBindingSize);
    }
    // maxUniformBufferBindingSize is larger than maxBufferSize, expect clamping to maxBufferSize
    {
        constexpr uint64_t reportedMaxBufferSize = 2147483648;
        constexpr uint64_t reportedMaxUniformBufferBindingSize = reportedMaxBufferSize + 1;
        Limits limits = defaults;
        limits.maxUniformBufferBindingSize = reportedMaxUniformBufferBindingSize;
        limits.maxBufferSize = reportedMaxBufferSize;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxBufferSize, reportedMaxBufferSize);
        EXPECT_EQ(limits.maxUniformBufferBindingSize, reportedMaxBufferSize);
    }
    // maxUniformBufferBindingSize is much larger than maxBufferSize, expect clamping to
    // maxBufferSize
    {
        constexpr uint64_t reportedMaxBufferSize = 2147483648;
        constexpr uint64_t reportedMaxUniformBufferBindingSize = 4294967295;
        Limits limits = defaults;
        limits.maxUniformBufferBindingSize = reportedMaxUniformBufferBindingSize;
        limits.maxBufferSize = reportedMaxBufferSize;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxBufferSize, reportedMaxBufferSize);
        EXPECT_EQ(limits.maxUniformBufferBindingSize, reportedMaxBufferSize);
    }
}

}  // namespace native
}  // namespace dawn
