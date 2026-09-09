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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/dawn/native/Limits.h"
#include "src/dawn/native/ShaderModule.h"

namespace dawn::native {
namespace {

LimitsForCompilationRequest MakeWorkgroupLimits() {
    LimitsForCompilationRequest limits = {};
    limits.maxComputeWorkgroupSizeX = 256;
    limits.maxComputeWorkgroupSizeY = 256;
    limits.maxComputeWorkgroupSizeZ = 64;
    limits.maxComputeInvocationsPerWorkgroup = 256;
    limits.maxComputeWorkgroupStorageSize = 16384;
    return limits;
}

TEST(ShaderModuleTests, SubgroupMatrixUsesExplicitSubgroupSizeForWorkgroupValidation) {
    tint::WorkgroupInfo workgroupInfo;
    workgroupInfo.x = 32;
    workgroupInfo.y = 1;
    workgroupInfo.z = 1;
    workgroupInfo.subgroup_size = 32;

    auto limits = MakeWorkgroupLimits();
    auto result = ValidateComputeStageWorkgroupSize(workgroupInfo, true, 64, limits, limits);
    if (result.IsError()) {
        FAIL() << result.AcquireError()->GetMessage();
    }
    Extent3D workgroupSize = result.AcquireSuccess();
    EXPECT_EQ(workgroupSize.width, 32u);
    EXPECT_EQ(workgroupSize.height, 1u);
    EXPECT_EQ(workgroupSize.depthOrArrayLayers, 1u);
}

TEST(ShaderModuleTests, SubgroupMatrixUsesMaximumSubgroupSizeWithoutExplicitSize) {
    tint::WorkgroupInfo workgroupInfo;
    workgroupInfo.x = 32;
    workgroupInfo.y = 1;
    workgroupInfo.z = 1;

    auto limits = MakeWorkgroupLimits();
    auto result = ValidateComputeStageWorkgroupSize(workgroupInfo, true, 64, limits, limits);
    if (!result.IsError()) {
        result.AcquireSuccess();
        FAIL() << "Expected workgroup validation to fail";
    }
    EXPECT_THAT(result.AcquireError()->GetMessage(), testing::HasSubstr("maxSubgroupSize (64)"));
}

}  // anonymous namespace
}  // namespace dawn::native
