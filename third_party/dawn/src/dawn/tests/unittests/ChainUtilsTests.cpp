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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dawn/native/ChainUtils.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {
namespace {

using ::testing::HasSubstr;

// Empty chain on roots that have and don't have valid extensions should not fail validation and all
// values should be nullptr.
TEST(ChainUtilsTests, ValidateAndUnpackEmpty) {
    {
        // TextureViewDescriptor has at least 1 valid chain extension..
        TextureViewDescriptor desc;
        auto unpacked = ValidateAndUnpack(&desc).AcquireSuccess();
        EXPECT_TRUE(unpacked.Empty());
    }
    {
        // InstanceDescriptor has at least 1 valid chain extension.
        InstanceDescriptor desc;
        auto unpacked = ValidateAndUnpack(&desc).AcquireSuccess();
        EXPECT_TRUE(unpacked.Empty());
    }
    {
        // SharedTextureMemoryProperties has at least 1 valid chain extension.
        SharedTextureMemoryProperties properties;
        auto unpacked = ValidateAndUnpack(&properties).AcquireSuccess();
        EXPECT_TRUE(unpacked.Empty());
    }
    {
        // SharedFenceExportInfo has at least 1 valid chain extension.
        SharedFenceExportInfo properties;
        auto unpacked = ValidateAndUnpack(&properties).AcquireSuccess();
        EXPECT_TRUE(unpacked.Empty());
    }
}

// Invalid chain extensions cause an error.
TEST(ChainUtilsTests, ValidateAndUnpackUnexpected) {
    {
        // TextureViewDescriptor (as of when this test was written) does not have any valid chains
        // in the JSON nor via additional extensions.
        TextureViewDescriptor desc;
        ChainedStruct chain;
        desc.nextInChain = &chain;
        EXPECT_THAT(ValidateAndUnpack(&desc).AcquireError()->GetFormattedMessage(),
                    HasSubstr("Unexpected"));
    }
    {
        // InstanceDescriptor has at least 1 valid chain extension.
        InstanceDescriptor desc;
        ChainedStruct chain;
        desc.nextInChain = &chain;
        EXPECT_THAT(ValidateAndUnpack(&desc).AcquireError()->GetFormattedMessage(),
                    HasSubstr("Unexpected"));
    }
}

// Inject invalid chain extensions cause an error.
TEST(ChainUtilsTests, ValidateAndUnpackInjected) {
    {
        // TextureViewDescriptor (as of when this test was written) does not have any valid chains
        // in the JSON nor via additional extensions.
        TextureViewDescriptor desc;
        DawnInjectedInvalidSType chain;
        chain.invalidSType = wgpu::SType::ShaderSourceWGSL;
        desc.nextInChain = &chain;
        EXPECT_THAT(ValidateAndUnpack(&desc).AcquireError()->GetFormattedMessage(),
                    HasSubstr("ShaderSourceWGSL"));
    }
    {
        // InstanceDescriptor has at least 1 valid chain extension.
        InstanceDescriptor desc;
        DawnInjectedInvalidSType chain;
        chain.invalidSType = wgpu::SType::ShaderSourceWGSL;
        desc.nextInChain = &chain;
        EXPECT_THAT(ValidateAndUnpack(&desc).AcquireError()->GetFormattedMessage(),
                    HasSubstr("ShaderSourceWGSL"));
    }
}

// Nominal unpacking valid descriptors should return the expected descriptors in the unpacked type.
TEST(ChainUtilsTests, ValidateAndUnpack) {
    // DawnTogglesDescriptor is a valid extension for InstanceDescriptor.
    InstanceDescriptor desc;
    DawnTogglesDescriptor chain;
    desc.nextInChain = &chain;
    auto unpacked = ValidateAndUnpack(&desc).AcquireSuccess();
    auto ext = unpacked.Get<DawnTogglesDescriptor>();
    EXPECT_EQ(ext, &chain);

    // For ChainedStructs, the resulting pointer from Get should be a const type.
    static_assert(std::is_const_v<std::remove_reference_t<decltype(*ext)>>);
}

// Nominal unpacking valid descriptors should return the expected descriptors in the unpacked type.
TEST(ChainUtilsTests, ValidateAndUnpackOut) {
    // DawnAdapterPropertiesPowerPreference is a valid extension for AdapterInfo.
    AdapterInfo info;
    DawnAdapterPropertiesPowerPreference chain;
    info.nextInChain = &chain;
    auto unpacked = ValidateAndUnpack(&info).AcquireSuccess();
    auto ext = unpacked.Get<DawnAdapterPropertiesPowerPreference>();
    EXPECT_EQ(ext, &chain);

    // For ChainedStructOuts, the resulting pointer from Get should not be a const type.
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(*ext)>>);
}

// Duplicate valid extensions cause an error.
TEST(ChainUtilsTests, ValidateAndUnpackDuplicate) {
    // DawnTogglesDescriptor is a valid extension for InstanceDescriptor.
    InstanceDescriptor desc;
    DawnTogglesDescriptor chain1;
    DawnTogglesDescriptor chain2;
    desc.nextInChain = &chain1;
    chain1.nextInChain = &chain2;
    EXPECT_THAT(ValidateAndUnpack(&desc).AcquireError()->GetFormattedMessage(),
                HasSubstr("Duplicate"));
}

// Duplicate valid extensions cause an error.
TEST(ChainUtilsTests, ValidateAndUnpackOutDuplicate) {
    // DawnAdapterPropertiesPowerPreference is a valid extension for AdapterInfo.
    AdapterInfo info;
    DawnAdapterPropertiesPowerPreference chain1;
    DawnAdapterPropertiesPowerPreference chain2;
    info.nextInChain = &chain1;
    chain1.nextInChain = &chain2;
    EXPECT_THAT(ValidateAndUnpack(&info).AcquireError()->GetFormattedMessage(),
                HasSubstr("Duplicate"));
}

// Additional extensions added via template specialization and not specified in the JSON unpack
// properly.
TEST(ChainUtilsTests, ValidateAndUnpackAdditionalExtensions) {
    // DawnInstanceDescriptor is an extension on InstanceDescriptor added in ChainUtilsImpl.inl.
    InstanceDescriptor desc;
    DawnInstanceDescriptor chain;
    desc.nextInChain = &chain;
    auto unpacked = ValidateAndUnpack(&desc).AcquireSuccess();
    EXPECT_EQ(unpacked.Get<DawnInstanceDescriptor>(), &chain);
}

// Duplicate additional extensions added via template specialization should cause an error.
TEST(ChainUtilsTests, ValidateAndUnpackDuplicateAdditionalExtensions) {
    // DawnInstanceDescriptor is an extension on InstanceDescriptor added in ChainUtilsImpl.inl.
    InstanceDescriptor desc;
    DawnInstanceDescriptor chain1;
    DawnInstanceDescriptor chain2;
    desc.nextInChain = &chain1;
    chain1.nextInChain = &chain2;
    EXPECT_THAT(ValidateAndUnpack(&desc).AcquireError()->GetFormattedMessage(),
                HasSubstr("Duplicate"));
}

using B1 = Branch<ShaderSourceWGSL>;
using B2 = Branch<ShaderSourceSPIRV>;
using B2Ext = Branch<ShaderSourceSPIRV, DawnShaderModuleSPIRVOptionsDescriptor>;

// Validates exacly 1 branch and ensures that there are no other extensions.
TEST(ChainUtilsTests, ValidateBranchesOneValidBranch) {
    ShaderModuleDescriptor desc;
    // Either allowed branches should validate successfully and return the expected enum.
    {
        ShaderSourceWGSL chain;
        desc.nextInChain = &chain;
        auto unpacked = ValidateAndUnpack(&desc).AcquireSuccess();
        EXPECT_EQ((unpacked.ValidateBranches<B1, B2>().AcquireSuccess()),
                  wgpu::SType::ShaderSourceWGSL);
    }
    {
        ShaderSourceSPIRV chain;
        desc.nextInChain = &chain;
        auto unpacked = ValidateAndUnpack(&desc).AcquireSuccess();
        EXPECT_EQ((unpacked.ValidateBranches<B1, B2>().AcquireSuccess()),
                  wgpu::SType::ShaderSourceSPIRV);

        // Extensions are optional so validation should still pass when the extension is not
        // provided.
        EXPECT_EQ((unpacked.ValidateBranches<B1, B2Ext>().AcquireSuccess()),
                  wgpu::SType::ShaderSourceSPIRV);
    }
}

// An allowed chain that is not one of the branches causes an error.
TEST(ChainUtilsTests, ValidateBranchesInvalidBranch) {
    ShaderModuleDescriptor desc;
    DawnShaderModuleSPIRVOptionsDescriptor chain;
    desc.nextInChain = &chain;
    auto unpacked = ValidateAndUnpack(&desc).AcquireSuccess();
    EXPECT_NE((unpacked.ValidateBranches<B1, B2>().AcquireError()), nullptr);
    EXPECT_NE((unpacked.ValidateBranches<B1, B2Ext>().AcquireError()), nullptr);
}

// Additional chains should cause an error when branches don't allow extensions.
TEST(ChainUtilsTests, ValidateBranchesInvalidExtension) {
    ShaderModuleDescriptor desc;
    {
        ShaderSourceWGSL chain1;
        DawnShaderModuleSPIRVOptionsDescriptor chain2;
        desc.nextInChain = &chain1;
        chain1.nextInChain = &chain2;
        auto unpacked = ValidateAndUnpack(&desc).AcquireSuccess();
        EXPECT_NE((unpacked.ValidateBranches<B1, B2>().AcquireError()), nullptr);
        EXPECT_NE((unpacked.ValidateBranches<B1, B2Ext>().AcquireError()), nullptr);
    }
    {
        ShaderSourceSPIRV chain1;
        DawnShaderModuleSPIRVOptionsDescriptor chain2;
        desc.nextInChain = &chain1;
        chain1.nextInChain = &chain2;
        auto unpacked = ValidateAndUnpack(&desc).AcquireSuccess();
        EXPECT_NE((unpacked.ValidateBranches<B1, B2>().AcquireError()), nullptr);
    }
}

// Branches that allow extensions pass successfully.
TEST(ChainUtilsTests, ValidateBranchesAllowedExtensions) {
    ShaderModuleDescriptor desc;
    ShaderSourceSPIRV chain1;
    DawnShaderModuleSPIRVOptionsDescriptor chain2;
    desc.nextInChain = &chain1;
    chain1.nextInChain = &chain2;
    auto unpacked = ValidateAndUnpack(&desc).AcquireSuccess();
    EXPECT_EQ((unpacked.ValidateBranches<B1, B2Ext>().AcquireSuccess()),
              wgpu::SType::ShaderSourceSPIRV);
}

// Unrealistic branching for ChainedStructOut testing. Note that this setup does not make sense.
using BOut1 = Branch<SharedFenceVkSemaphoreOpaqueFDExportInfo>;
using BOut2 = Branch<SharedFenceSyncFDExportInfo>;
using BOut2Ext = Branch<SharedFenceSyncFDExportInfo, SharedFenceVkSemaphoreZirconHandleExportInfo>;

// Validates exacly 1 branch and ensures that there are no other extensions.
TEST(ChainUtilsTests, ValidateBranchesOneValidBranchOut) {
    SharedFenceExportInfo info;
    // Either allowed branches should validate successfully and return the expected enum.
    {
        SharedFenceVkSemaphoreOpaqueFDExportInfo chain;
        info.nextInChain = &chain;
        auto unpacked = ValidateAndUnpack(&info).AcquireSuccess();
        EXPECT_EQ((unpacked.ValidateBranches<BOut1, BOut2>().AcquireSuccess()),
                  wgpu::SType::SharedFenceVkSemaphoreOpaqueFDExportInfo);
    }
    {
        SharedFenceSyncFDExportInfo chain;
        info.nextInChain = &chain;
        auto unpacked = ValidateAndUnpack(&info).AcquireSuccess();
        EXPECT_EQ((unpacked.ValidateBranches<BOut1, BOut2>().AcquireSuccess()),
                  wgpu::SType::SharedFenceSyncFDExportInfo);

        // Extensions are optional so validation should still pass when the extension is not
        // provided.
        EXPECT_EQ((unpacked.ValidateBranches<BOut1, BOut2Ext>().AcquireSuccess()),
                  wgpu::SType::SharedFenceSyncFDExportInfo);
    }
}

// An allowed chain that is not one of the branches causes an error.
TEST(ChainUtilsTests, ValidateBranchesInvalidBranchOut) {
    SharedFenceExportInfo info;
    SharedFenceDXGISharedHandleExportInfo chain;
    info.nextInChain = &chain;
    auto unpacked = ValidateAndUnpack(&info).AcquireSuccess();
    EXPECT_NE((unpacked.ValidateBranches<BOut1, BOut2>().AcquireError()), nullptr);
    EXPECT_NE((unpacked.ValidateBranches<BOut1, BOut2Ext>().AcquireError()), nullptr);
}

// Additional chains should cause an error when branches don't allow extensions.
TEST(ChainUtilsTests, ValidateBranchesInvalidExtensionOut) {
    SharedFenceExportInfo info;
    {
        SharedFenceVkSemaphoreOpaqueFDExportInfo chain1;
        SharedFenceVkSemaphoreZirconHandleExportInfo chain2;
        info.nextInChain = &chain1;
        chain1.nextInChain = &chain2;
        auto unpacked = ValidateAndUnpack(&info).AcquireSuccess();
        EXPECT_NE((unpacked.ValidateBranches<BOut1, BOut2>().AcquireError()), nullptr);
        EXPECT_NE((unpacked.ValidateBranches<BOut1, BOut2Ext>().AcquireError()), nullptr);
    }
    {
        SharedFenceSyncFDExportInfo chain1;
        SharedFenceVkSemaphoreZirconHandleExportInfo chain2;
        info.nextInChain = &chain1;
        chain1.nextInChain = &chain2;
        auto unpacked = ValidateAndUnpack(&info).AcquireSuccess();
        EXPECT_NE((unpacked.ValidateBranches<BOut1, BOut2>().AcquireError()), nullptr);
    }
}

// Branches that allow extensions pass successfully.
TEST(ChainUtilsTests, ValidateBranchesAllowedExtensionsOut) {
    SharedFenceExportInfo info;
    SharedFenceSyncFDExportInfo chain1;
    SharedFenceVkSemaphoreZirconHandleExportInfo chain2;
    info.nextInChain = &chain1;
    chain1.nextInChain = &chain2;
    auto unpacked = ValidateAndUnpack(&info).AcquireSuccess();
    EXPECT_EQ((unpacked.ValidateBranches<BOut1, BOut2Ext>().AcquireSuccess()),
              wgpu::SType::SharedFenceSyncFDExportInfo);
}

// Valid subsets should pass successfully, while invalid ones should error.
TEST(ChainUtilsTests, ValidateSubset) {
    DeviceDescriptor desc;
    DawnTogglesDescriptor chain1;
    DawnCacheDeviceDescriptor chain2;

    // With none set, subset for anything should work.
    {
        auto unpacked = ValidateAndUnpack(&desc).AcquireSuccess();
        EXPECT_TRUE(unpacked.ValidateSubset<>().IsSuccess());
        EXPECT_TRUE(unpacked.ValidateSubset<DawnTogglesDescriptor>().IsSuccess());
        EXPECT_TRUE(unpacked.ValidateSubset<DawnCacheDeviceDescriptor>().IsSuccess());
        EXPECT_TRUE((unpacked.ValidateSubset<DawnTogglesDescriptor, DawnCacheDeviceDescriptor>()
                         .IsSuccess()));
    }
    // With one set, subset with that allow that one should pass. Otherwise it should fail.
    {
        desc.nextInChain = &chain1;
        auto unpacked = ValidateAndUnpack(&desc).AcquireSuccess();
        EXPECT_NE(unpacked.ValidateSubset<>().AcquireError(), nullptr);
        EXPECT_TRUE(unpacked.ValidateSubset<DawnTogglesDescriptor>().IsSuccess());
        EXPECT_NE(unpacked.ValidateSubset<DawnCacheDeviceDescriptor>().AcquireError(), nullptr);
        EXPECT_TRUE((unpacked.ValidateSubset<DawnTogglesDescriptor, DawnCacheDeviceDescriptor>()
                         .IsSuccess()));
    }
    // With both set, single subsets should all fail.
    {
        chain1.nextInChain = &chain2;
        auto unpacked = ValidateAndUnpack(&desc).AcquireSuccess();
        EXPECT_NE(unpacked.ValidateSubset<>().AcquireError(), nullptr);
        EXPECT_NE(unpacked.ValidateSubset<DawnTogglesDescriptor>().AcquireError(), nullptr);
        EXPECT_NE(unpacked.ValidateSubset<DawnCacheDeviceDescriptor>().AcquireError(), nullptr);
        EXPECT_TRUE((unpacked.ValidateSubset<DawnTogglesDescriptor, DawnCacheDeviceDescriptor>()
                         .IsSuccess()));
    }
}

// Valid subsets should pass successfully, while invalid ones should error.
TEST(ChainUtilsTests, ValidateSubsetOut) {
    SharedFenceExportInfo info;
    SharedFenceVkSemaphoreOpaqueFDExportInfo chain1;
    SharedFenceVkSemaphoreZirconHandleExportInfo chain2;

    // With none set, subset for anything should work.
    {
        auto unpacked = ValidateAndUnpack(&info).AcquireSuccess();
        EXPECT_TRUE(unpacked.ValidateSubset<>().IsSuccess());
        EXPECT_TRUE(
            unpacked.ValidateSubset<SharedFenceVkSemaphoreOpaqueFDExportInfo>().IsSuccess());
        EXPECT_TRUE(
            unpacked.ValidateSubset<SharedFenceVkSemaphoreZirconHandleExportInfo>().IsSuccess());
        EXPECT_TRUE((unpacked
                         .ValidateSubset<SharedFenceVkSemaphoreOpaqueFDExportInfo,
                                         SharedFenceVkSemaphoreZirconHandleExportInfo>()
                         .IsSuccess()));
    }
    // With one set, subset with that allow that one should pass. Otherwise it should fail.
    {
        info.nextInChain = &chain1;
        auto unpacked = ValidateAndUnpack(&info).AcquireSuccess();
        EXPECT_NE(unpacked.ValidateSubset<>().AcquireError(), nullptr);
        EXPECT_TRUE(
            unpacked.ValidateSubset<SharedFenceVkSemaphoreOpaqueFDExportInfo>().IsSuccess());
        EXPECT_NE(
            unpacked.ValidateSubset<SharedFenceVkSemaphoreZirconHandleExportInfo>().AcquireError(),
            nullptr);
        EXPECT_TRUE((unpacked
                         .ValidateSubset<SharedFenceVkSemaphoreOpaqueFDExportInfo,
                                         SharedFenceVkSemaphoreZirconHandleExportInfo>()
                         .IsSuccess()));
    }
    // With both set, single subsets should all fail.
    {
        chain1.nextInChain = &chain2;
        auto unpacked = ValidateAndUnpack(&info).AcquireSuccess();
        EXPECT_NE(unpacked.ValidateSubset<>().AcquireError(), nullptr);
        EXPECT_NE(
            unpacked.ValidateSubset<SharedFenceVkSemaphoreOpaqueFDExportInfo>().AcquireError(),
            nullptr);
        EXPECT_NE(
            unpacked.ValidateSubset<SharedFenceVkSemaphoreZirconHandleExportInfo>().AcquireError(),
            nullptr);
        EXPECT_TRUE((unpacked
                         .ValidateSubset<SharedFenceVkSemaphoreOpaqueFDExportInfo,
                                         SharedFenceVkSemaphoreZirconHandleExportInfo>()
                         .IsSuccess()));
    }
}

}  // anonymous namespace
}  // namespace dawn::native
