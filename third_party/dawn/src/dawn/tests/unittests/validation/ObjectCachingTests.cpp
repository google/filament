// Copyright 2019 The Dawn & Tint Authors
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

#include <vector>

#include "dawn/tests/unittests/validation/ValidationTest.h"

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using testing::Not;

// These tests works assuming Dawn Native's object deduplication. Comparing the pointer is
// exploiting an implementation detail of Dawn Native.
class ObjectCachingTest : public ValidationTest {
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::StaticSamplers, wgpu::FeatureName::YCbCrVulkanSamplers};
    }

    void SetUp() override {
        ValidationTest::SetUp();
        DAWN_SKIP_TEST_IF(UsesWire());
    }
};

// Test that BindGroupLayouts are correctly deduplicated.
TEST_F(ObjectCachingTest, BindGroupLayoutDeduplication) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});
    wgpu::BindGroupLayout sameBgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});
    wgpu::BindGroupLayout otherBgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform}});

    EXPECT_THAT(bgl, Not(BindGroupLayoutEq(otherBgl)));
    EXPECT_THAT(bgl, BindGroupLayoutEq(sameBgl));
}

// Test that two similar bind group layouts won't refer to the same one if they differ by dynamic.
TEST_F(ObjectCachingTest, BindGroupLayoutDynamic) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform, true}});
    wgpu::BindGroupLayout sameBgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform, true}});
    wgpu::BindGroupLayout otherBgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform, false}});

    EXPECT_THAT(bgl, Not(BindGroupLayoutEq(otherBgl)));
    EXPECT_THAT(bgl, BindGroupLayoutEq(sameBgl));
}

// Test that two similar bind group layouts won't refer to the same one if they differ by min size.
TEST_F(ObjectCachingTest, BindGroupLayoutMinBufferSize) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform, false, 0}});
    wgpu::BindGroupLayout sameBgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform, false, 0}});
    wgpu::BindGroupLayout otherBgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform, false, 4}});

    EXPECT_THAT(bgl, Not(BindGroupLayoutEq(otherBgl)));
    EXPECT_THAT(bgl, BindGroupLayoutEq(sameBgl));
}

// Test that two similar bind group layouts won't refer to the same one if they differ by
// textureComponentType
TEST_F(ObjectCachingTest, BindGroupLayoutTextureComponentType) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});
    wgpu::BindGroupLayout sameBgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});
    wgpu::BindGroupLayout otherBgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Uint}});

    EXPECT_THAT(bgl, Not(BindGroupLayoutEq(otherBgl)));
    EXPECT_THAT(bgl, BindGroupLayoutEq(sameBgl));
}

// Test that two similar bind group layouts won't refer to the same one if they differ by
// viewDimension
TEST_F(ObjectCachingTest, BindGroupLayoutViewDimension) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});
    wgpu::BindGroupLayout sameBgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});
    wgpu::BindGroupLayout otherBgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float,
                  wgpu::TextureViewDimension::e2DArray}});

    EXPECT_THAT(bgl, Not(BindGroupLayoutEq(otherBgl)));
    EXPECT_THAT(bgl, BindGroupLayoutEq(sameBgl));
}

// Test that BindGroupLayouts with a static sampler entry are correctly
// deduplicated.
TEST_F(ObjectCachingTest, BindGroupLayoutStaticSamplerDeduplication) {
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    wgpu::SamplerDescriptor samplerDesc;
    samplerDesc.addressModeU = wgpu::AddressMode::ClampToEdge;
    staticSamplerBinding.sampler = device.CreateSampler(&samplerDesc);
    binding.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    wgpu::SamplerDescriptor otherSamplerDesc;
    otherSamplerDesc.addressModeU = wgpu::AddressMode::Repeat;
    wgpu::Sampler otherSampler = device.CreateSampler(&otherSamplerDesc);

    EXPECT_NE(staticSamplerBinding.sampler.Get(), otherSampler.Get());

    wgpu::BindGroupLayoutEntry otherBinding = {};
    otherBinding.binding = 0;
    wgpu::StaticSamplerBindingLayout otherStaticSamplerBinding = {};
    otherStaticSamplerBinding.sampler = otherSampler;
    otherBinding.nextInChain = &otherStaticSamplerBinding;

    EXPECT_NE(staticSamplerBinding.sampler.Get(), otherStaticSamplerBinding.sampler.Get());

    wgpu::BindGroupLayoutDescriptor otherDesc = {};
    otherDesc.entryCount = 1;
    otherDesc.entries = &otherBinding;

    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&desc);
    wgpu::BindGroupLayout sameBgl = device.CreateBindGroupLayout(&desc);
    wgpu::BindGroupLayout otherStaticSamplerBgl = device.CreateBindGroupLayout(&otherDesc);
    wgpu::BindGroupLayout otherBgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering}});

    EXPECT_THAT(bgl, BindGroupLayoutEq(sameBgl));
    EXPECT_THAT(bgl, Not(BindGroupLayoutEq(otherStaticSamplerBgl)));
    EXPECT_THAT(bgl, Not(BindGroupLayoutEq(otherBgl)));
}

// Test that BindGroupLayouts with a static sampler entry keep a reference
// to the static sampler, such that if a sampler is created from the same
// params the same object will be returned.
TEST_F(ObjectCachingTest, BindGroupLayoutKeepsRefToStaticSampler) {
    auto sampler1 = device.CreateSampler();
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    wgpu::StaticSamplerBindingLayout staticSamplerBinding = {};
    staticSamplerBinding.sampler = sampler1;
    binding.nextInChain = &staticSamplerBinding;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&desc);

    auto samplerRawPtr = sampler1.Get();
    sampler1 = nullptr;
    auto sampler2 = device.CreateSampler();
    EXPECT_EQ(samplerRawPtr, sampler2.Get());
}

// Test that PipelineLayouts are correctly deduplicated.
TEST_F(ObjectCachingTest, PipelineLayoutDeduplication) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});
    wgpu::BindGroupLayout otherBgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform}});

    wgpu::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, &bgl);
    wgpu::PipelineLayout samePl = utils::MakeBasicPipelineLayout(device, &bgl);
    wgpu::PipelineLayout otherPl1 = utils::MakeBasicPipelineLayout(device, nullptr);
    wgpu::PipelineLayout otherPl2 = utils::MakeBasicPipelineLayout(device, &otherBgl);

    EXPECT_NE(pl.Get(), otherPl1.Get());
    EXPECT_NE(pl.Get(), otherPl2.Get());
    EXPECT_EQ(pl.Get(), samePl.Get());
}

// Test that ShaderModules are correctly deduplicated.
TEST_F(ObjectCachingTest, ShaderModuleDeduplication) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })");
    wgpu::ShaderModule sameModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })");
    wgpu::ShaderModule otherModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        })");

    EXPECT_NE(module.Get(), otherModule.Get());
    EXPECT_EQ(module.Get(), sameModule.Get());
}

// Test that ComputePipeline are correctly deduplicated wrt. their ShaderModule
TEST_F(ObjectCachingTest, ComputePipelineDeduplicationOnShaderModule) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        var<workgroup> i : u32;
        @compute @workgroup_size(1) fn main() {
            i = 0u;
        })");
    wgpu::ShaderModule sameModule = utils::CreateShaderModule(device, R"(
        var<workgroup> i : u32;
        @compute @workgroup_size(1) fn main() {
            i = 0u;
        })");
    wgpu::ShaderModule otherModule = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1) fn main() {
        })");

    EXPECT_NE(module.Get(), otherModule.Get());
    EXPECT_EQ(module.Get(), sameModule.Get());

    wgpu::PipelineLayout layout = utils::MakeBasicPipelineLayout(device, nullptr);

    wgpu::ComputePipelineDescriptor desc;
    desc.layout = layout;

    desc.compute.module = module;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&desc);

    desc.compute.module = sameModule;
    wgpu::ComputePipeline samePipeline = device.CreateComputePipeline(&desc);

    desc.compute.module = otherModule;
    wgpu::ComputePipeline otherPipeline = device.CreateComputePipeline(&desc);

    EXPECT_NE(pipeline.Get(), otherPipeline.Get());
    EXPECT_EQ(pipeline.Get(), samePipeline.Get());
}

// Test that ComputePipeline are correctly deduplicated wrt. their constants override values
TEST_F(ObjectCachingTest, ComputePipelineDeduplicationOnOverrides) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        override x: u32 = 1u;
        var<workgroup> i : u32;
        @compute @workgroup_size(x) fn main() {
            i = 0u;
        })");

    wgpu::PipelineLayout layout = utils::MakeBasicPipelineLayout(device, nullptr);

    wgpu::ComputePipelineDescriptor desc;
    desc.layout = layout;
    desc.compute.module = module;

    std::vector<wgpu::ConstantEntry> constants{{nullptr, "x", 16}};
    desc.compute.constantCount = constants.size();
    desc.compute.constants = constants.data();
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&desc);

    std::vector<wgpu::ConstantEntry> sameConstants{{nullptr, "x", 16}};
    desc.compute.constantCount = sameConstants.size();
    desc.compute.constants = sameConstants.data();
    wgpu::ComputePipeline samePipeline = device.CreateComputePipeline(&desc);

    desc.compute.constantCount = 0;
    desc.compute.constants = nullptr;
    wgpu::ComputePipeline otherPipeline1 = device.CreateComputePipeline(&desc);

    std::vector<wgpu::ConstantEntry> otherConstants{{nullptr, "x", 4}};
    desc.compute.constantCount = otherConstants.size();
    desc.compute.constants = otherConstants.data();
    wgpu::ComputePipeline otherPipeline2 = device.CreateComputePipeline(&desc);

    EXPECT_NE(pipeline.Get(), otherPipeline1.Get());
    EXPECT_NE(pipeline.Get(), otherPipeline2.Get());
    EXPECT_EQ(pipeline.Get(), samePipeline.Get());
}

// Test that ComputePipeline are correctly deduplicated wrt. their layout
TEST_F(ObjectCachingTest, ComputePipelineDeduplicationOnLayout) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});
    wgpu::BindGroupLayout otherBgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform}});

    wgpu::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, &bgl);
    wgpu::PipelineLayout samePl = utils::MakeBasicPipelineLayout(device, &bgl);
    wgpu::PipelineLayout otherPl = utils::MakeBasicPipelineLayout(device, nullptr);

    EXPECT_NE(pl.Get(), otherPl.Get());
    EXPECT_EQ(pl.Get(), samePl.Get());

    wgpu::ComputePipelineDescriptor desc;
    desc.compute.module = utils::CreateShaderModule(device, R"(
            var<workgroup> i : u32;
            @compute @workgroup_size(1) fn main() {
                i = 0u;
            })");

    desc.layout = pl;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&desc);

    desc.layout = samePl;
    wgpu::ComputePipeline samePipeline = device.CreateComputePipeline(&desc);

    desc.layout = otherPl;
    wgpu::ComputePipeline otherPipeline = device.CreateComputePipeline(&desc);

    EXPECT_NE(pipeline.Get(), otherPipeline.Get());
    EXPECT_EQ(pipeline.Get(), samePipeline.Get());
}

// Test that RenderPipelines are correctly deduplicated wrt. their layout
TEST_F(ObjectCachingTest, RenderPipelineDeduplicationOnLayout) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});
    wgpu::BindGroupLayout otherBgl = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform}});

    wgpu::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, &bgl);
    wgpu::PipelineLayout samePl = utils::MakeBasicPipelineLayout(device, &bgl);
    wgpu::PipelineLayout otherPl = utils::MakeBasicPipelineLayout(device, nullptr);

    EXPECT_NE(pl.Get(), otherPl.Get());
    EXPECT_EQ(pl.Get(), samePl.Get());

    utils::ComboRenderPipelineDescriptor desc;
    desc.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
    desc.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        })");
    desc.cFragment.module = utils::CreateShaderModule(device, R"(
        @fragment fn main() {
        })");

    desc.layout = pl;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    desc.layout = samePl;
    wgpu::RenderPipeline samePipeline = device.CreateRenderPipeline(&desc);

    desc.layout = otherPl;
    wgpu::RenderPipeline otherPipeline = device.CreateRenderPipeline(&desc);

    EXPECT_NE(pipeline.Get(), otherPipeline.Get());
    EXPECT_EQ(pipeline.Get(), samePipeline.Get());
}

// Test that RenderPipelines are correctly deduplicated wrt. their vertex module
TEST_F(ObjectCachingTest, RenderPipelineDeduplicationOnVertexModule) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        })");
    wgpu::ShaderModule sameModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        })");
    wgpu::ShaderModule otherModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(1.0, 1.0, 1.0, 1.0);
        })");

    EXPECT_NE(module.Get(), otherModule.Get());
    EXPECT_EQ(module.Get(), sameModule.Get());

    utils::ComboRenderPipelineDescriptor desc;
    desc.layout = utils::MakeBasicPipelineLayout(device, nullptr);
    desc.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
    desc.cFragment.module = utils::CreateShaderModule(device, R"(
            @fragment fn main() {
            })");

    desc.vertex.module = module;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    desc.vertex.module = sameModule;
    wgpu::RenderPipeline samePipeline = device.CreateRenderPipeline(&desc);

    desc.vertex.module = otherModule;
    wgpu::RenderPipeline otherPipeline = device.CreateRenderPipeline(&desc);

    EXPECT_NE(pipeline.Get(), otherPipeline.Get());
    EXPECT_EQ(pipeline.Get(), samePipeline.Get());
}

// Test that RenderPipelines are correctly deduplicated wrt. their fragment module
TEST_F(ObjectCachingTest, RenderPipelineDeduplicationOnFragmentModule) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @fragment fn main() {
        })");
    wgpu::ShaderModule sameModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() {
        })");
    wgpu::ShaderModule otherModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        })");

    EXPECT_NE(module.Get(), otherModule.Get());
    EXPECT_EQ(module.Get(), sameModule.Get());

    utils::ComboRenderPipelineDescriptor desc;
    desc.layout = utils::MakeBasicPipelineLayout(device, nullptr);
    desc.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        })");

    desc.cFragment.module = module;
    desc.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    desc.cFragment.module = sameModule;
    wgpu::RenderPipeline samePipeline = device.CreateRenderPipeline(&desc);

    desc.cFragment.module = otherModule;
    wgpu::RenderPipeline otherPipeline = device.CreateRenderPipeline(&desc);

    EXPECT_NE(pipeline.Get(), otherPipeline.Get());
    EXPECT_EQ(pipeline.Get(), samePipeline.Get());
}

// Test that Renderpipelines are correctly deduplicated wrt. their constants override values
TEST_F(ObjectCachingTest, RenderPipelineDeduplicationOnOverrides) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        override a: f32 = 1.0;
        @vertex fn vertexMain() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        }
        @fragment fn fragmentMain() -> @location(0) vec4f {
            return vec4f(0.0, 0.0, 0.0, a);
        })");

    utils::ComboRenderPipelineDescriptor desc;
    desc.layout = utils::MakeBasicPipelineLayout(device, nullptr);
    desc.vertex.module = module;
    desc.cFragment.module = module;
    desc.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

    std::vector<wgpu::ConstantEntry> constants{{nullptr, "a", 0.5}};
    desc.cFragment.constantCount = constants.size();
    desc.cFragment.constants = constants.data();
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    std::vector<wgpu::ConstantEntry> sameConstants{{nullptr, "a", 0.5}};
    desc.cFragment.constantCount = sameConstants.size();
    desc.cFragment.constants = sameConstants.data();
    wgpu::RenderPipeline samePipeline = device.CreateRenderPipeline(&desc);

    std::vector<wgpu::ConstantEntry> otherConstants{{nullptr, "a", 1.0}};
    desc.cFragment.constantCount = otherConstants.size();
    desc.cFragment.constants = otherConstants.data();
    wgpu::RenderPipeline otherPipeline1 = device.CreateRenderPipeline(&desc);

    desc.cFragment.constantCount = 0;
    desc.cFragment.constants = nullptr;
    wgpu::RenderPipeline otherPipeline2 = device.CreateRenderPipeline(&desc);

    EXPECT_NE(pipeline.Get(), otherPipeline1.Get());
    EXPECT_NE(pipeline.Get(), otherPipeline2.Get());
    EXPECT_EQ(pipeline.Get(), samePipeline.Get());
}

// Test that Samplers are correctly deduplicated.
TEST_F(ObjectCachingTest, SamplerDeduplication) {
    wgpu::SamplerDescriptor samplerDesc;
    wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

    wgpu::SamplerDescriptor sameSamplerDesc;
    wgpu::Sampler sameSampler = device.CreateSampler(&sameSamplerDesc);

    wgpu::SamplerDescriptor otherSamplerDescAddressModeU;
    otherSamplerDescAddressModeU.addressModeU = wgpu::AddressMode::Repeat;
    wgpu::Sampler otherSamplerAddressModeU = device.CreateSampler(&otherSamplerDescAddressModeU);

    wgpu::SamplerDescriptor otherSamplerDescAddressModeV;
    otherSamplerDescAddressModeV.addressModeV = wgpu::AddressMode::Repeat;
    wgpu::Sampler otherSamplerAddressModeV = device.CreateSampler(&otherSamplerDescAddressModeV);

    wgpu::SamplerDescriptor otherSamplerDescAddressModeW;
    otherSamplerDescAddressModeW.addressModeW = wgpu::AddressMode::Repeat;
    wgpu::Sampler otherSamplerAddressModeW = device.CreateSampler(&otherSamplerDescAddressModeW);

    wgpu::SamplerDescriptor otherSamplerDescMagFilter;
    otherSamplerDescMagFilter.magFilter = wgpu::FilterMode::Linear;
    wgpu::Sampler otherSamplerMagFilter = device.CreateSampler(&otherSamplerDescMagFilter);

    wgpu::SamplerDescriptor otherSamplerDescMinFilter;
    otherSamplerDescMinFilter.minFilter = wgpu::FilterMode::Linear;
    wgpu::Sampler otherSamplerMinFilter = device.CreateSampler(&otherSamplerDescMinFilter);

    wgpu::SamplerDescriptor otherSamplerDescMipmapFilter;
    otherSamplerDescMipmapFilter.mipmapFilter = wgpu::MipmapFilterMode::Linear;
    wgpu::Sampler otherSamplerMipmapFilter = device.CreateSampler(&otherSamplerDescMipmapFilter);

    wgpu::SamplerDescriptor otherSamplerDescLodMinClamp;
    otherSamplerDescLodMinClamp.lodMinClamp += 1;
    wgpu::Sampler otherSamplerLodMinClamp = device.CreateSampler(&otherSamplerDescLodMinClamp);

    wgpu::SamplerDescriptor otherSamplerDescLodMaxClamp;
    otherSamplerDescLodMaxClamp.lodMaxClamp += 1;
    wgpu::Sampler otherSamplerLodMaxClamp = device.CreateSampler(&otherSamplerDescLodMaxClamp);

    wgpu::SamplerDescriptor otherSamplerDescCompareFunction;
    otherSamplerDescCompareFunction.compare = wgpu::CompareFunction::Always;
    wgpu::Sampler otherSamplerCompareFunction =
        device.CreateSampler(&otherSamplerDescCompareFunction);

    wgpu::SamplerDescriptor otherSamplerDescYCbCrSampling;
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    otherSamplerDescYCbCrSampling.nextInChain = &yCbCrDesc;
    wgpu::Sampler otherSamplerYCbCrSampling = device.CreateSampler(&otherSamplerDescYCbCrSampling);

    EXPECT_NE(sampler.Get(), otherSamplerAddressModeU.Get());
    EXPECT_NE(sampler.Get(), otherSamplerAddressModeV.Get());
    EXPECT_NE(sampler.Get(), otherSamplerAddressModeW.Get());
    EXPECT_NE(sampler.Get(), otherSamplerMagFilter.Get());
    EXPECT_NE(sampler.Get(), otherSamplerMinFilter.Get());
    EXPECT_NE(sampler.Get(), otherSamplerMipmapFilter.Get());
    EXPECT_NE(sampler.Get(), otherSamplerLodMinClamp.Get());
    EXPECT_NE(sampler.Get(), otherSamplerLodMaxClamp.Get());
    EXPECT_NE(sampler.Get(), otherSamplerCompareFunction.Get());
    EXPECT_NE(sampler.Get(), otherSamplerYCbCrSampling.Get());
    EXPECT_EQ(sampler.Get(), sameSampler.Get());
}

// Test that YCbCr samplers are correctly deduplicated.
TEST_F(ObjectCachingTest, YCbCrSamplerDeduplication) {
    wgpu::SamplerDescriptor samplerDesc;
    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    samplerDesc.nextInChain = &yCbCrDesc;
    wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

    wgpu::SamplerDescriptor sameSamplerDesc;
    wgpu::YCbCrVkDescriptor sameYCbCrDesc = {};
    sameSamplerDesc.nextInChain = &sameYCbCrDesc;
    wgpu::Sampler sameSampler = device.CreateSampler(&sameSamplerDesc);

    wgpu::SamplerDescriptor otherSamplerDescVkFormat;
    wgpu::YCbCrVkDescriptor otherYCbCrDescVkFormat = {};
    otherYCbCrDescVkFormat.vkFormat = 42;
    otherSamplerDescVkFormat.nextInChain = &otherYCbCrDescVkFormat;
    wgpu::Sampler otherSamplerVkFormat = device.CreateSampler(&otherSamplerDescVkFormat);

    wgpu::SamplerDescriptor otherSamplerDescModel;
    wgpu::YCbCrVkDescriptor otherYCbCrDescModel = {};
    otherYCbCrDescModel.vkYCbCrModel = 3;
    otherSamplerDescModel.nextInChain = &otherYCbCrDescModel;
    wgpu::Sampler otherSamplerModel = device.CreateSampler(&otherSamplerDescModel);

    wgpu::SamplerDescriptor otherSamplerDescRange;
    wgpu::YCbCrVkDescriptor otherYCbCrDescRange = {};
    otherYCbCrDescRange.vkYCbCrRange = 3;
    otherSamplerDescRange.nextInChain = &otherYCbCrDescRange;
    wgpu::Sampler otherSamplerRange = device.CreateSampler(&otherSamplerDescRange);

    wgpu::SamplerDescriptor otherSamplerDescRed;
    wgpu::YCbCrVkDescriptor otherYCbCrDescRed = {};
    otherYCbCrDescRed.vkComponentSwizzleRed = 3;
    otherSamplerDescRed.nextInChain = &otherYCbCrDescRed;
    wgpu::Sampler otherSamplerRed = device.CreateSampler(&otherSamplerDescRed);

    wgpu::SamplerDescriptor otherSamplerDescGreen;
    wgpu::YCbCrVkDescriptor otherYCbCrDescGreen = {};
    otherYCbCrDescGreen.vkComponentSwizzleGreen = 3;
    otherSamplerDescGreen.nextInChain = &otherYCbCrDescGreen;
    wgpu::Sampler otherSamplerGreen = device.CreateSampler(&otherSamplerDescGreen);

    wgpu::SamplerDescriptor otherSamplerDescBlue;
    wgpu::YCbCrVkDescriptor otherYCbCrDescBlue = {};
    otherYCbCrDescBlue.vkComponentSwizzleBlue = 3;
    otherSamplerDescBlue.nextInChain = &otherYCbCrDescBlue;
    wgpu::Sampler otherSamplerBlue = device.CreateSampler(&otherSamplerDescBlue);

    wgpu::SamplerDescriptor otherSamplerDescAlpha;
    wgpu::YCbCrVkDescriptor otherYCbCrDescAlpha = {};
    otherYCbCrDescAlpha.vkComponentSwizzleAlpha = 3;
    otherSamplerDescAlpha.nextInChain = &otherYCbCrDescAlpha;
    wgpu::Sampler otherSamplerAlpha = device.CreateSampler(&otherSamplerDescAlpha);

    wgpu::SamplerDescriptor otherSamplerDescXChromaOffset;
    wgpu::YCbCrVkDescriptor otherYCbCrDescXChromaOffset = {};
    otherYCbCrDescXChromaOffset.vkXChromaOffset = 3;
    otherSamplerDescXChromaOffset.nextInChain = &otherYCbCrDescXChromaOffset;
    wgpu::Sampler otherSamplerXChromaOffset = device.CreateSampler(&otherSamplerDescXChromaOffset);

    wgpu::SamplerDescriptor otherSamplerDescYChromaOffset;
    wgpu::YCbCrVkDescriptor otherYCbCrDescYChromaOffset = {};
    otherYCbCrDescYChromaOffset.vkYChromaOffset = 3;
    otherSamplerDescYChromaOffset.nextInChain = &otherYCbCrDescYChromaOffset;
    wgpu::Sampler otherSamplerYChromaOffset = device.CreateSampler(&otherSamplerDescYChromaOffset);

    wgpu::SamplerDescriptor otherSamplerDescChromaFilter;
    wgpu::YCbCrVkDescriptor otherYCbCrDescChromaFilter = {};
    otherYCbCrDescChromaFilter.vkChromaFilter = wgpu::FilterMode::Linear;
    otherSamplerDescChromaFilter.nextInChain = &otherYCbCrDescChromaFilter;
    wgpu::Sampler otherSamplerChromaFilter = device.CreateSampler(&otherSamplerDescChromaFilter);

    wgpu::SamplerDescriptor otherSamplerDescReconstruction;
    wgpu::YCbCrVkDescriptor otherYCbCrDescReconstruction = {};
    otherYCbCrDescReconstruction.forceExplicitReconstruction = true;
    otherSamplerDescReconstruction.nextInChain = &otherYCbCrDescReconstruction;
    wgpu::Sampler otherSamplerReconstruction =
        device.CreateSampler(&otherSamplerDescReconstruction);

    wgpu::SamplerDescriptor otherSamplerDescExternalFormat;
    wgpu::YCbCrVkDescriptor otherYCbCrDescExternalFormat = {};
    otherYCbCrDescExternalFormat.externalFormat = 42;
    otherSamplerDescExternalFormat.nextInChain = &otherYCbCrDescExternalFormat;
    wgpu::Sampler otherSamplerExternalFormat =
        device.CreateSampler(&otherSamplerDescExternalFormat);

    EXPECT_NE(sampler.Get(), otherSamplerVkFormat.Get());
    EXPECT_NE(sampler.Get(), otherSamplerModel.Get());
    EXPECT_NE(sampler.Get(), otherSamplerRange.Get());
    EXPECT_NE(sampler.Get(), otherSamplerRed.Get());
    EXPECT_NE(sampler.Get(), otherSamplerGreen.Get());
    EXPECT_NE(sampler.Get(), otherSamplerBlue.Get());
    EXPECT_NE(sampler.Get(), otherSamplerAlpha.Get());
    EXPECT_NE(sampler.Get(), otherSamplerXChromaOffset.Get());
    EXPECT_NE(sampler.Get(), otherSamplerYChromaOffset.Get());
    EXPECT_NE(sampler.Get(), otherSamplerChromaFilter.Get());
    EXPECT_NE(sampler.Get(), otherSamplerReconstruction.Get());
    EXPECT_NE(sampler.Get(), otherSamplerExternalFormat.Get());
    EXPECT_EQ(sampler.Get(), sameSampler.Get());
}

}  // anonymous namespace
}  // namespace dawn
