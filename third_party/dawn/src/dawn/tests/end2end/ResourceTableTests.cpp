// Copyright 2025 The Dawn & Tint Authors
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

#include <string>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "absl/strings/str_format.h"
#include "src/dawn/common/Enumerator.h"
#include "src/dawn/common/Range.h"
#include "src/dawn/tests/DawnTest.h"
#include "src/dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "src/dawn/utils/ComboRenderPipelineDescriptor.h"
#include "src/dawn/utils/ScopedIgnoreValidationErrors.h"
#include "src/dawn/utils/WGPUHelpers.h"
#include "src/utils/numeric.h"

namespace dawn {
namespace {

// The number of unique default samplers on D3D12, derived form ResourceTableDefaultResources.
// These necessarily take up a couple of slots in a resource table.
constexpr uint32_t kUniqueDefaultSamplers = 2;
// On D3D12, the max number of unique sampler descriptors available to the user in a ResourceTable.
constexpr uint32_t kD3D12MaxUniqueSamplers = 2048 - kUniqueDefaultSamplers;

class ResourceTableTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(
            !SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable}));

        // Swiftshader doesn't support variable count descriptor sets used in draw operations. In
        // vk::DescriptorSet::ParseDescriptors it iterates over all the descriptors to prep various
        // things but iterates over the whole size defined in the vkDescriptorSetLayout instead of
        // taking into account the variable count.
        DAWN_SUPPRESS_TEST_IF(IsSwiftshader());
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable})) {
            return {wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable};
        }
        return {};
    }

    wgpu::ResourceTable MakeResourceTable(
        uint32_t size,
        std::vector<std::pair<uint32_t, wgpu::BindingResource>> resources = {}) {
        wgpu::ResourceTableDescriptor desc;
        desc.size = size;
        wgpu::ResourceTable table = device.CreateResourceTable(&desc);

        for (auto& [slot, resource] : resources) {
            EXPECT_EQ(wgpu::Status::Success, table.Update(slot, &resource));
        }

        return table;
    }

    wgpu::TextureView CreateViewForTable(const wgpu::Texture& tex) {
        wgpu::TextureViewDescriptor desc;
        desc.usage = wgpu::TextureUsage::TextureBinding;
        return tex.CreateView(&desc);
    }

    std::vector<wgpu::BindGroupEntry> MakeBindGroupEntries(
        std::vector<std::pair<uint32_t, wgpu::BindingResource>> resources) {
        std::vector<wgpu::BindGroupEntry> entries;
        for (auto& r : resources) {
            wgpu::BindGroupEntry entry;
            entry.binding = r.first;
            DAWN_ASSERT(r.second.textureView);
            entry.textureView = r.second.textureView;
            entries.push_back(entry);
        }
        return entries;
    }

    wgpu::PipelineLayout MakePipelineLayoutWithTable(std::vector<wgpu::BindGroupLayout> bgls = {},
                                                     uint32_t immediateSize = 0) {
        wgpu::PipelineLayoutResourceTable plTable;
        plTable.usesResourceTable = true;

        wgpu::PipelineLayoutDescriptor desc{
            .nextInChain = &plTable,
            .bindGroupLayoutCount = bgls.size(),
            .bindGroupLayouts = bgls.data(),
            .immediateSize = immediateSize,
        };

        return device.CreatePipelineLayout(&desc);
    }

    // Test that the `table` has resources of `wgslType` in the `expected` slots.
    // If provided, creates a bind group 1 with 'bindGroupEntries' with resources of type
    // `bindGroupWgslType`.
    void TestHasResource(wgpu::ResourceTable table,
                         std::vector<bool> expected,
                         std::string wgslType = "texture_2d<f32>",
                         std::vector<wgpu::BindGroupEntry> bindGroupEntries = {},
                         std::string bindGroupWgslType = "texture_2d<f32>") {
        ASSERT_EQ(table.GetSize(), expected.size());

        std::string shader = R"(
            enable chromium_experimental_resource_table;

            @group(0) @binding(0) var<storage, read_write> results : array<u32>;
            var<immediate> resourceCount : u32;
            @compute @workgroup_size(1) fn main() {
                for (var i = 0u; i < resourceCount; i++) {
                    results[i] = u32(hasResource<)" +
                             wgslType + R"(>(i));
                }

                referenceTextures();
            }
        )";

        // If provided, add bindings in group(1)
        if (!bindGroupEntries.empty()) {
            for (auto entry : bindGroupEntries) {
                shader += absl::StrFormat("@group(1) @binding(%1$u) var tex%1$u : %2$s;\n",
                                          entry.binding, bindGroupWgslType);
            }
            shader += "fn referenceTextures() {\n";
            for (auto entry : bindGroupEntries) {
                shader +=
                    absl::StrFormat("let d%1$u = textureDimensions(tex%1$u);\n", entry.binding);
            }
            shader += "}\n";
        } else {
            shader += "fn referenceTextures() {}\n";
        }

        // Create the test pipeline.
        wgpu::ShaderModule module = utils::CreateShaderModule(device, shader);
        wgpu::ComputePipelineDescriptor csDesc = {.compute = {
                                                      .module = module,
                                                  }};
        wgpu::ComputePipeline testPipeline = device.CreateComputePipeline(&csDesc);

        // Create the result buffer.
        wgpu::BufferDescriptor bDesc = {
            .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
            .size = sizeof(uint32_t) * expected.size(),
        };
        wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);
        wgpu::BindGroup resultBG =
            utils::MakeBindGroup(device, testPipeline.GetBindGroupLayout(0), {{0, resultBuffer}});
        uint32_t resourceCount = table.GetSize();

        wgpu::BindGroup bindGroupToHideSlots;
        if (!bindGroupEntries.empty()) {
            wgpu::BindGroupDescriptor descriptor;
            descriptor.layout = testPipeline.GetBindGroupLayout(1);
            descriptor.entryCount = checked_cast<uint32_t>(bindGroupEntries.size());
            descriptor.entries = bindGroupEntries.data();
            bindGroupToHideSlots = device.CreateBindGroup(&descriptor);
        }

        // Run the test.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetResourceTable(table);
        pass.SetImmediates(0, &resourceCount, sizeof(resourceCount));
        pass.SetBindGroup(0, resultBG);
        if (bindGroupToHideSlots) {
            pass.SetBindGroup(1, bindGroupToHideSlots);
        }
        pass.SetPipeline(testPipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);

        // Check we have the expected results.
        std::vector<uint32_t> expectedU32;
        for (bool b : expected) {
            expectedU32.push_back(b ? 1u : 0u);
        }

        EXPECT_BUFFER_U32_RANGE_EQ(expectedU32.data(), resultBuffer, 0, expectedU32.size())
            << " for WGSL type " << wgslType;
    }

    void DoSomeWorkInSubmit() {
        wgpu::BufferDescriptor bufDesc = {
            .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc,
            .size = 4,
        };
        wgpu::Buffer src = device.CreateBuffer(&bufDesc);
        wgpu::Buffer dst = device.CreateBuffer(&bufDesc);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(src, 0, dst, 0, 4);

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    wgpu::TextureView MakeU8View(uint8_t value) {
        // Create the texture.
        wgpu::TextureDescriptor tDesc{
            .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst,
            .size = {1, 1},
            .format = wgpu::TextureFormat::R8Uint,
        };
        wgpu::Texture tex = device.CreateTexture(&tDesc);

        // Write the u8
        wgpu::TexelCopyTextureInfo srcInfo = utils::CreateTexelCopyTextureInfo(tex);
        wgpu::TexelCopyBufferLayout dstInfo = {};
        wgpu::Extent3D copySize = {1, 1, 1};
        queue.WriteTexture(&srcInfo, &value, 1, &dstInfo, &copySize);

        // Return a view to the texture.
        return tex.CreateView();
    }

    // For each table in `cases`, sets the `table` and dipatches on a compute pass encoder,
    // and validates that each `table` has a texture_2d<u32> iff the `expected` has a value, and
    // that the textures have the expected value, if any.
    struct TableAndExpected {
        wgpu::ResourceTable table;
        std::vector<std::optional<uint8_t>> expected;
    };
    void TestHasU8BindingsCompute(std::vector<TableAndExpected> cases) {
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;

            @group(0) @binding(0) var<storage, read_write> results : array<u32>;
            struct Immediates {
                resourceCount : u32,
                offset : u32,
            }
            var<immediate> immediates : Immediates;
            @compute @workgroup_size(1) fn main() {
                for (var i = 0u; i < immediates.resourceCount; i++) {
                    if !hasResource<texture_2d<u32>>(i) {
                        results[immediates.offset + i] = 0xBEEF;
                    } else {
                        let tex = getResource<texture_2d<u32>>(i);
                        results[immediates.offset + i] = textureLoad(tex, vec2(0), 0).x;
                    }
                }
            }
        )");

        // Make the result buffer large enough for all cases
        size_t resultSize = 0;
        for (auto& [table, expected] : cases) {
            ASSERT_EQ(table.GetSize(), expected.size());
            resultSize += expected.size();
        }

        wgpu::ComputePipelineDescriptor csDesc = {.compute = {
                                                      .module = module,
                                                  }};
        wgpu::ComputePipeline testPipeline = device.CreateComputePipeline(&csDesc);

        // Create the result buffer.
        wgpu::BufferDescriptor bDesc = {
            .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
            .size = sizeof(uint32_t) * resultSize,
        };
        wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);
        wgpu::BindGroup resultBG =
            utils::MakeBindGroup(device, testPipeline.GetBindGroupLayout(0), {{0, resultBuffer}});

        // Run the test.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        uint32_t offset = 0;
        for (auto& [table, expected] : cases) {
            pass.SetResourceTable(table);

            uint32_t immediates[] = {table.GetSize(), offset};
            pass.SetImmediates(0, &immediates, sizeof(immediates));
            offset += expected.size();

            pass.SetBindGroup(0, resultBG);

            pass.SetPipeline(testPipeline);
            pass.DispatchWorkgroups(1);
        }
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);

        // Check we have the expected results.
        std::vector<uint32_t> expectedU32;
        for (auto& [_, expected] : cases) {
            for (auto optValue : expected) {
                expectedU32.push_back(optValue ? *optValue : 0xBEEFu);
            }
        }

        EXPECT_BUFFER_U32_RANGE_EQ(expectedU32.data(), resultBuffer, 0, expectedU32.size());
    }

    // Sets the `table` and dipatches on a render pass/bundle encoder, and validates that `table`
    // has a texture_2d<u32> iff `expected` has a value, and that the textures have the expected
    // value, if any.
    void TestHasU8BindingsRender(wgpu::ResourceTable table,
                                 std::vector<std::optional<uint8_t>> expected,
                                 bool useRenderBundles = false) {
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;

            @vertex fn vs() -> @builtin(position) vec4f {
                return vec4f(0, 0, 0.5, 0.5);
            }

            @group(0) @binding(0) var<storage, read_write> results : array<u32>;
            var<immediate> resourceCount : u32;

            @fragment fn main() -> @location(0) vec4f {
                for (var i = 0u; i < resourceCount; i++) {
                    if !hasResource<texture_2d<u32>>(i) {
                        results[i] = 0xBEEF;
                    } else {
                        let tex = getResource<texture_2d<u32>>(i);
                        results[i] = textureLoad(tex, vec2(0), 0).x;
                    }
                }
                return vec4();
            }
        )");

        wgpu::BindGroupLayout resultBGL = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});

        wgpu::RenderPipeline testPipeline;
        {
            utils::ComboRenderPipelineDescriptor desc;
            desc.layout = MakePipelineLayoutWithTable({resultBGL}, 8);
            desc.vertex.module = module;
            desc.cFragment.module = module;
            desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
            testPipeline = device.CreateRenderPipeline(&desc);
        }

        // Create the result buffer.
        wgpu::BufferDescriptor bDesc = {
            .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
            .size = sizeof(uint32_t) * expected.size(),
        };
        wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);
        wgpu::BindGroup resultBG = utils::MakeBindGroup(device, resultBGL, {{0, resultBuffer}});

        // Run the test.
        auto rp = utils::CreateBasicRenderPass(device, 1, 1);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.SetResourceTable(table);

        if (useRenderBundles) {
            utils::ComboRenderBundleEncoderDescriptor desc = {};
            desc.SetUsesResourceTable();
            desc.colorFormatCount = 1;
            desc.cColorFormats[0] = rp.colorFormat;
            wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);

            uint32_t immediates = table.GetSize();
            rbe.SetImmediates(0, &immediates, sizeof(immediates));
            rbe.SetBindGroup(0, resultBG);
            rbe.SetPipeline(testPipeline);
            rbe.Draw(1);

            wgpu::RenderBundle bundle = rbe.Finish();
            pass.ExecuteBundles(1, &bundle);
        } else {
            uint32_t immediates = table.GetSize();
            pass.SetImmediates(0, &immediates, sizeof(immediates));
            pass.SetBindGroup(0, resultBG);
            pass.SetPipeline(testPipeline);
            pass.Draw(1);
        }
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);

        // Check we have the expected results.
        std::vector<uint32_t> expectedU32;
        for (auto optValue : expected) {
            expectedU32.push_back(optValue ? *optValue : 0xBEEFu);
        }

        EXPECT_BUFFER_U32_RANGE_EQ(expectedU32.data(), resultBuffer, 0, expectedU32.size());
    }

    // Convenience that tests cases using compute, render, and render bundle encoders
    void TestHasU8BindingsAll(wgpu::ResourceTable table,
                              std::vector<std::optional<uint8_t>> expected) {
        TestHasU8BindingsCompute({{table, expected}});
        TestHasU8BindingsRender(table, expected, true);
        TestHasU8BindingsRender(table, expected, false);
    }

    // Creates a sampler by address mode
    wgpu::Sampler CreateSampler(wgpu::AddressMode mode,
                                wgpu::CompareFunction compare = wgpu::CompareFunction::Undefined) {
        wgpu::SamplerDescriptor descriptor = {};
        descriptor.addressModeU = mode;
        descriptor.addressModeV = mode;
        descriptor.addressModeW = mode;
        descriptor.compare = compare;
        return device.CreateSampler(&descriptor);
    }

    // Creates a 2x2 checkerboard texture, with red in the top left and bottom right corners, green
    // in the other two.
    wgpu::Texture CreateCheckerboardTexture() {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size = {2, 2};
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
        auto texture = device.CreateTexture(&descriptor);

        const uint32_t rowPixels = kTextureBytesPerRowAlignment / sizeof(utils::RGBA8);
        std::array<utils::RGBA8, static_cast<size_t>(rowPixels) * 2> pixels;
        pixels[0] = pixels[rowPixels + 1] = utils::RGBA8::kRed;
        pixels[1] = pixels[rowPixels] = utils::RGBA8::kGreen;

        wgpu::TexelCopyTextureInfo srcInfo =
            utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
        wgpu::TexelCopyBufferLayout dstInfo = {};
        dstInfo.bytesPerRow = kTextureBytesPerRowAlignment;
        wgpu::Extent3D copySize = {2, 2, 1};
        queue.WriteTexture(&srcInfo, pixels.data(), pixels.size() * sizeof(utils::RGBA8), &dstInfo,
                           &copySize);

        return texture;
    }

    wgpu::Texture CreateColorTexture(utils::RGBA8 color) {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size = {1, 1};
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
        auto texture = device.CreateTexture(&descriptor);

        const uint32_t rowPixels = kTextureBytesPerRowAlignment / sizeof(utils::RGBA8);
        std::array<utils::RGBA8, rowPixels> pixels;
        pixels[0] = color;

        wgpu::TexelCopyTextureInfo srcInfo =
            utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
        wgpu::TexelCopyBufferLayout dstInfo = {};
        dstInfo.bytesPerRow = kTextureBytesPerRowAlignment;
        wgpu::Extent3D copySize = {1, 1, 1};
        queue.WriteTexture(&srcInfo, pixels.data(), pixels.size() * sizeof(utils::RGBA8), &dstInfo,
                           &copySize);

        return texture;
    }

    wgpu::Sampler CreateUniqueSampler(uint32_t i) {
        // Make samplers unique by making one of the values different for each
        wgpu::SamplerDescriptor descriptor;
        descriptor.lodMinClamp = (i + 1) / 1000.0f;
        return device.CreateSampler(&descriptor);
    }
};

// Test that creating resource tables doesn't crash in backends.
TEST_P(ResourceTableTests, ResourceTableCreation) {
    // Creating an empty resource table.
    MakeResourceTable(0);

    // Creating a resource table with a few entries.
    MakeResourceTable(36);

    // Creating a resource table with the maximum number of entries.
    MakeResourceTable(kMaxResourceTableSize);
}

// Test that creating pipeline layouts with resources tables doesn't crash in backends.
TEST_P(ResourceTableTests, PipelineLayoutWithResourceTableCreation) {
    // Make layouts with no BGLs with / without immediates.
    MakePipelineLayoutWithTable({}, 0);
    MakePipelineLayoutWithTable({}, 4);

    // Make layouts with one BGL, with / without immediates.
    wgpu::BindGroupLayout testBgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});
    MakePipelineLayoutWithTable({testBgl}, 0);
    MakePipelineLayoutWithTable({testBgl}, 4);

    // Make layouts with max BGLs (3 because the resource tables "consumes" one bind group), with /
    // without immediates.
    MakePipelineLayoutWithTable({testBgl, testBgl, testBgl}, 0);
    MakePipelineLayoutWithTable({testBgl, testBgl, testBgl}, 4);
}

// Test that creating pipelines that use resource tables doesn't crash in backends.
TEST_P(ResourceTableTests, ShaderWithResourceTableCreation) {
    wgpu::ComputePipelineDescriptor csDesc;

    // Test compiling a pipeline using only the resource table.
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @compute @workgroup_size(1) fn main() {
            _ = hasResource<texture_2d<f32>>(0);
        }
    )");
    device.CreateComputePipeline(&csDesc);

    // Test compiling a pipeline using the resource table and a bindgroup.
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @group(0) @binding(0) var t0 : texture_2d<f32>;
        @compute @workgroup_size(1) fn main() {
            _ = hasResource<texture_2d<f32>>(0);
            _ = t0;
        }
    )");
    device.CreateComputePipeline(&csDesc);

    // Test compiling a pipeline using the resource table and many bindgroup.
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @group(0) @binding(0) var t0 : texture_2d<f32>;
        @group(1) @binding(0) var t1 : texture_2d<f32>;
        @group(2) @binding(0) var t2 : texture_2d<f32>;
        @compute @workgroup_size(1) fn main() {
            _ = hasResource<texture_2d<f32>>(0);
            _ = t0;
            _ = t1;
            _ = t2;
        }
    )");
    device.CreateComputePipeline(&csDesc);
}

// Test that creating resource tables of different sizes doesn't end up reusing incorrectly sized
// allocations.
TEST_P(ResourceTableTests, RecyclingDoesntReuseTooSmallAllocation) {
    for (uint32_t i = 0; i < 10; i++) {
        MakeResourceTable(i);

        // Wait to ensure some deallocation happens and has a chance to cause incorrect recycling.
        WaitForAllOperations();
    }
}

// Test WGSL `hasResource` reflects the state of the resource table.
TEST_P(ResourceTableTests, HasResourceOneTexture) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::ResourceTable table =
        MakeResourceTable(3, {{1, {.textureView = CreateViewForTable(tex)}}});
    auto bge = MakeBindGroupEntries({{0, {.textureView = tex.CreateView()}}});

    // Table bound textures are visible by default
    TestHasResource(table, {false, true, false}, "texture_2d<f32>");

    // If the texture is also in a bind group as writable, it hides the same texture in the table
    TestHasResource(table, {false, false, false}, "texture_2d<f32>", bge,
                    "texture_storage_2d<rgba8unorm, write>");

    // But if it's bound as readonly, it doesn't hide it
    TestHasResource(table, {false, true, false}, "texture_2d<f32>", bge, "texture_2d<f32>");
}

// Test WGSL `hasResource` reflects the state of the resource table.
TEST_P(ResourceTableTests, HasResourceFilterableToUnfilterable) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R8Unorm,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::ResourceTable table = MakeResourceTable(3, {{1, {.textureView = tex.CreateView()}}});

    TestHasResource(table, {false, true, false}, "texture_2d<f32>");
}

// Test that calling texture.Destroy() implicitly hides it.
// Note: Similar tests for EndAccess are implemented in SharedTextureMemoryResourceTableTests.
TEST_P(ResourceTableTests, HasResourceOneTextureDestroy) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R8Unorm,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::ResourceTable table = MakeResourceTable(3, {{1, {.textureView = tex.CreateView()}}});

    // Has one valid entry
    TestHasResource(table, {false, true, false});

    // After texture destruction it has the no more valid entries.
    tex.Destroy();
    TestHasResource(table, {false, false, false});
}

// Test that a texture used multiple times in the same table has its availability correctly updated.
TEST_P(ResourceTableTests, HasResourceSameTextureMultipleTimes) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::ResourceTable table =
        MakeResourceTable(4, {
                                 {1, {.textureView = CreateViewForTable(tex)}},
                                 {3, {.textureView = CreateViewForTable(tex)}},
                             });

    auto bge = MakeBindGroupEntries({{0, {.textureView = tex.CreateView()}}});

    // Table bound textures are visible by default
    TestHasResource(table, {false, true, false, true});

    // If the texture is also in a bind group as writable, it hides the same texture in the table
    TestHasResource(table, {false, false, false, false}, "texture_2d<f32>", bge,
                    "texture_storage_2d<rgba8unorm, write>");

    // But if it's bound as readonly, it doesn't hide it
    TestHasResource(table, {false, true, false, true}, "texture_2d<f32>", bge, "texture_2d<f32>");
}

// Test that updating a table with an already destroyed texture works, but doesn't show that entry
// as available.
// Note: Similar tests for EndAccess are implemented in SharedTextureMemoryResourceTableTests.
TEST_P(ResourceTableTests, HasResourceUpdateWithTextureAlreadyDestroyed) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R8Unorm,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);
    tex.Destroy();

    wgpu::ResourceTable table = MakeResourceTable(1, {{0, {.textureView = tex.CreateView()}}});

    TestHasResource(table, {false});
}

// Test that a texture used in multiple resource tables has its availability correctly updated.
// Note: Similar tests for EndAccess are implemented in SharedTextureMemoryResourceTableTests.
TEST_P(ResourceTableTests, HasResourceSameTextureMultipleTables) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R8Unorm,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::ResourceTable table1 = MakeResourceTable(3, {{1, {.textureView = tex.CreateView()}}});
    wgpu::ResourceTable table2 = MakeResourceTable(1, {{0, {.textureView = tex.CreateView()}}});

    // Table bound textures are visible by default
    TestHasResource(table1, {false, true, false});
    TestHasResource(table2, {true});

    // After destroying one table, the other still has the texture available.
    table1.Destroy();
    TestHasResource(table2, {true});
}

// Test that texture availabililty is controlled per-texture.
TEST_P(ResourceTableTests, HasResourceMultipleTexturesTable) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::Texture tex0 = device.CreateTexture(&tDesc);
    wgpu::Texture tex1 = device.CreateTexture(&tDesc);

    wgpu::ResourceTable table =
        MakeResourceTable(2, {
                                 {0, {.textureView = CreateViewForTable(tex0)}},
                                 {1, {.textureView = CreateViewForTable(tex1)}},
                             });
    auto bge0 = MakeBindGroupEntries({{5, {.textureView = tex0.CreateView()}}});
    auto bge1 = MakeBindGroupEntries({{6, {.textureView = tex1.CreateView()}}});
    auto bgeBoth = MakeBindGroupEntries(
        {{5, {.textureView = tex0.CreateView()}}, {6, {.textureView = tex1.CreateView()}}});

    // Both visible by default
    TestHasResource(table, {true, true}, "texture_2d<f32>");

    // Both hidden if both are writable in bindgroup
    TestHasResource(table, {false, false}, "texture_2d<f32>", bgeBoth,
                    "texture_storage_2d<rgba8unorm, write>");

    // First hidden if first is writable in bindgroup
    TestHasResource(table, {false, true}, "texture_2d<f32>", bge0,
                    "texture_storage_2d<rgba8unorm, write>");

    // Second hidden if second is writable in bindgroup
    TestHasResource(table, {true, false}, "texture_2d<f32>", bge1,
                    "texture_storage_2d<rgba8unorm, write>");

    // Both visible if both are readonly in bindgroup
    TestHasResource(table, {true, true}, "texture_2d<f32>", bgeBoth, "texture_2d<f32>");
}

// Test that writing to a texture in a render pass implicitly hides it in the resource table.
// TODO(crbug.com/522749739): TestHasResource is used to test this with compute shaders. Consider
// making TestHasResource test both for compute and render passes.
TEST_P(ResourceTableTests, ImplicitHidingRender) {
    // TODO(https://issues.chromium.org/issues/530631417): Fails on PowerVR DXT-48-1536
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsImgTec() && IsVulkan());

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var pos = array<vec2f, 3>(
                vec2f(-1.0, -1.0),
                vec2f(3.0, -1.0),
                vec2f(-1.0, 3.0)
            );
            return vec4f(pos[VertexIndex], 0.0, 1.0);
        }
    )");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;

        @group(0) @binding(0) var<storage, read_write> results : array<u32>;

        @fragment fn main() -> @location(0) u32 {
            let hasRead = hasResource<texture_2d<u32>>(0);
            results[0] = u32(hasRead);
            return 0u;
        }
    )");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.cFragment.targetCount = 1;
    descriptor.cTargets[0].format = wgpu::TextureFormat::R32Uint;
    descriptor.layout = nullptr;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // Create textures.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment |
                 wgpu::TextureUsage::CopyDst,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Uint,
    };
    wgpu::Texture texA = device.CreateTexture(&tDesc);
    wgpu::Texture texB = device.CreateTexture(&tDesc);

    // Create result buffer.
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = 1 * sizeof(uint32_t),
    };
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);

    // Create resource table containing texA.
    wgpu::ResourceTable table =
        MakeResourceTable(1, {{0, {.textureView = CreateViewForTable(texA)}}});

    auto draw = [&](wgpu::Texture writeToTexture, bool expectHasResource) {
        wgpu::BindGroup bg = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                  {
                                                      {0, resultBuffer},
                                                  });

        wgpu::RenderPassColorAttachment attachment = {
            .view = writeToTexture.CreateView(),
            .loadOp = wgpu::LoadOp::Clear,
            .storeOp = wgpu::StoreOp::Store,
            .clearValue = {0, 0, 0, 0},
        };
        wgpu::RenderPassDescriptor rpDesc = {
            .colorAttachmentCount = 1,
            .colorAttachments = &attachment,
        };

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rpDesc);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bg);
        pass.SetResourceTable(table);
        pass.Draw(3);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        uint32_t expected[] = {expectHasResource ? 1u : 0u};
        EXPECT_BUFFER_U32_RANGE_EQ(expected, resultBuffer, 0, 1);
    };

    // Read texA, write texB (different textures), hasResource(texA) should return true
    draw(texB, true);

    // Read texA, write texA (same texture), hasResource(texA) should return false (hidden)
    draw(texA, false);
}

// Test that we can hide a resource for one dispatch (using a storage texture write)
// and dynamically unhide it for the next dispatch (by removing the storage write)
// all within a single compute pass.
TEST_P(ResourceTableTests, ImplicitHidingComputeMidPass) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;

        @group(0) @binding(0) var<storage, read_write> results : array<u32>;
        @group(0) @binding(1) var writeTex : texture_storage_2d<r32uint, write>;
        @group(0) @binding(2) var<uniform> dispatchIndex : u32;

        @compute @workgroup_size(1) fn main() {
            let hasRead = hasResource<texture_2d<u32>>(0);
            results[dispatchIndex] = u32(hasRead);
            if (dispatchIndex == 0u) {
                textureStore(writeTex, vec2u(0, 0), vec4u(123, 0, 0, 0));
            }
        }
    )");

    wgpu::ComputePipelineDescriptor csDesc = {.compute = {
                                                  .module = module,
                                              }};
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Create textures.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Uint,
    };
    wgpu::Texture texA = device.CreateTexture(&tDesc);
    wgpu::Texture texB = device.CreateTexture(&tDesc);

    // Create result buffer (4 elements: [hasRead0, val0, hasRead1, val1]).
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = 2 * sizeof(uint32_t),
    };
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);

    // Create uniform buffers for dispatch indices.
    wgpu::Buffer dispatchIndex0 =
        utils::CreateBufferFromData(device, wgpu::BufferUsage::Uniform, {0u});
    wgpu::Buffer dispatchIndex1 =
        utils::CreateBufferFromData(device, wgpu::BufferUsage::Uniform, {1u});

    // Create resource table containing texA.
    wgpu::ResourceTable table =
        MakeResourceTable(1, {{0, {.textureView = CreateViewForTable(texA)}}});

    // Create bind groups.
    // bg0 writes to texA (causes hiding)
    wgpu::BindGroup bg0 = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                               {
                                                   {0, resultBuffer},
                                                   {1, texA.CreateView()},
                                                   {2, dispatchIndex0},
                                               });
    // bg1 writes to texB instead of texA (causes unhiding of texA)
    wgpu::BindGroup bg1 = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                               {
                                                   {0, resultBuffer},
                                                   {1, texB.CreateView()},
                                                   {2, dispatchIndex1},
                                               });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetResourceTable(table);

    // Dispatch 1: texA is bound for writing, should be hidden.
    pass.SetBindGroup(0, bg0);
    pass.DispatchWorkgroups(1);

    // Dispatch 2: texA is no longer bound for writing, should be unhidden.
    pass.SetBindGroup(0, bg1);
    pass.DispatchWorkgroups(1);

    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    uint32_t expected[] = {
        0,  // hidden
        1   // visible
    };
    EXPECT_BUFFER_U32_RANGE_EQ(expected, resultBuffer, 0, 2);
}

// Test that writing to a texture as a render attachment hides it for the entire render pass,
// and changing bind groups mid-pass does NOT unhide it.
// TODO(crbug.com/522749739): Also test writing to a depth attachment, including depthReadOnly.
TEST_P(ResourceTableTests, ImplicitHidingRenderMidPass) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var pos = array<vec2f, 3>(
                vec2f(-1.0, -1.0),
                vec2f(3.0, -1.0),
                vec2f(-1.0, 3.0)
            );
            return vec4f(pos[VertexIndex], 0.0, 1.0);
        }
    )");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;

        @group(0) @binding(0) var<storage, read_write> results : array<u32>;
        @group(0) @binding(1) var<uniform> drawIndex : u32;
        @group(0) @binding(2) var writeTex : texture_storage_2d<r32uint, write>;

        @fragment fn main() -> @location(0) u32 {
            let hasRead = hasResource<texture_2d<u32>>(0);
            results[drawIndex] = u32(hasRead);
            if (drawIndex == 0u) {
                textureStore(writeTex, vec2u(0, 0), vec4u(123, 0, 0, 0));
            }
            return 0u;
        }
    )");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = vsModule;
    desc.cFragment.module = fsModule;
    desc.cFragment.targetCount = 1;
    desc.cTargets[0].format = wgpu::TextureFormat::R32Uint;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    // Create textures.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Uint,
    };
    wgpu::Texture texA = device.CreateTexture(&tDesc);
    wgpu::Texture texB = device.CreateTexture(&tDesc);

    // Create result buffer.
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = 2 * sizeof(uint32_t),
    };
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);

    // Create uniform buffers for draw indices.
    wgpu::Buffer drawIndex0 = utils::CreateBufferFromData(device, wgpu::BufferUsage::Uniform, {0u});
    wgpu::Buffer drawIndex1 = utils::CreateBufferFromData(device, wgpu::BufferUsage::Uniform, {1u});

    // Create resource table containing texA.
    wgpu::ResourceTable table =
        MakeResourceTable(1, {{0, {.textureView = CreateViewForTable(texA)}}});

    // Create bind groups.
    // bg0 writes to texA (causes hiding)
    wgpu::BindGroup bg0 = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                               {
                                                   {0, resultBuffer},
                                                   {1, drawIndex0},
                                                   {2, texA.CreateView()},
                                               });
    // bg1 writes to texB instead of texA
    wgpu::BindGroup bg1 = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                               {
                                                   {0, resultBuffer},
                                                   {1, drawIndex1},
                                                   {2, texB.CreateView()},
                                               });

    utils::BasicRenderPass rp =
        utils::CreateBasicRenderPass(device, 1, 1, wgpu::TextureFormat::R32Uint);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetResourceTable(table);

    // Draw 1: writes to texA. texA should be hidden.
    pass.SetBindGroup(0, bg0);
    pass.Draw(3);

    // Draw 2: does not write to texA. But because it's a render pass, texA must remain hidden.
    pass.SetBindGroup(0, bg1);
    pass.Draw(3);

    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    uint32_t expected[] = {
        0,  // hidden
        0   // still hidden
    };
    EXPECT_BUFFER_U32_RANGE_EQ(expected, resultBuffer, 0, 2);
}

constexpr auto kWgslSampledTextureTypes = std::array{
    "texture_1d<f32>",
    "texture_1d<i32>",
    "texture_1d<u32>",
    "texture_2d<f32>",
    "texture_2d<i32>",
    "texture_2d<u32>",
    "texture_2d_array<f32>",
    "texture_2d_array<i32>",
    "texture_2d_array<u32>",
    "texture_cube<f32>",
    "texture_cube<i32>",
    "texture_cube<u32>",
    "texture_cube_array<f32>",
    "texture_cube_array<i32>",
    "texture_cube_array<u32>",
    "texture_3d<f32>",
    "texture_3d<i32>",
    "texture_3d<u32>",

    "texture_multisampled_2d<f32>",
    "texture_multisampled_2d<i32>",
    "texture_multisampled_2d<u32>",

    "texture_depth_2d",
    "texture_depth_2d_array",
    "texture_depth_cube",
    "texture_depth_cube_array",
    "texture_depth_multisampled_2d",

    "sampler",
    "sampler_comparison",
};

struct ResourceDescForTypeIDCase {
    // Set of all WGSL types that can be validly bound to the underlying resource described by
    // `desc`.
    std::unordered_set<std::string_view> wgslTypes;

    struct TextureDesc {
        wgpu::TextureFormat format;
        wgpu::TextureDimension dimension;
        wgpu::TextureViewDimension viewDimension = wgpu::TextureViewDimension::Undefined;
        uint32_t sampleCount = 1;
        wgpu::TextureAspect viewAspect = wgpu::TextureAspect::All;
    };
    struct SamplerDesc {
        bool filtering = false;
        bool comparison = false;
    };
    // The descriptor used to create the resource
    std::variant<TextureDesc, SamplerDesc> desc;

    // Create a view for a texture for this case.
    wgpu::TextureView CreateTestView(const wgpu::Device& device) {
        auto& d = std::get<TextureDesc>(desc);

        wgpu::TextureDescriptor tDesc = {
            .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc,
            .dimension = d.dimension,
            .size = {1, 1, 1},
            .format = d.format,
            .sampleCount = d.sampleCount,
        };
        if (d.viewDimension == wgpu::TextureViewDimension::Cube ||
            d.viewDimension == wgpu::TextureViewDimension::CubeArray) {
            tDesc.size.depthOrArrayLayers = 6;
        }
        if (d.sampleCount != 1) {
            tDesc.usage |= wgpu::TextureUsage::RenderAttachment;
        }

        wgpu::TextureViewDescriptor vDesc{
            .dimension = d.viewDimension,
            .aspect = d.viewAspect,
            .usage = wgpu::TextureUsage::TextureBinding,
        };

        wgpu::Texture texture = device.CreateTexture(&tDesc);
        return texture.CreateView(&vDesc);
    }

    // Create a sampler for this case.
    wgpu::Sampler CreateTestSampler(const wgpu::Device& device) {
        auto& d = std::get<SamplerDesc>(desc);

        wgpu::SamplerDescriptor sDesc{};
        if (d.filtering) {
            sDesc.magFilter = wgpu::FilterMode::Linear;
            sDesc.minFilter = wgpu::FilterMode::Linear;
            sDesc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
        }
        if (d.comparison) {
            sDesc.compare = wgpu::CompareFunction::Less;
        }
        return device.CreateSampler(&sDesc);
    }
};

std::vector<ResourceDescForTypeIDCase> MakeDescForTypeIDCases() {
    std::vector<ResourceDescForTypeIDCase> cases;

    using TextureDesc = ResourceDescForTypeIDCase::TextureDesc;
    using SamplerDesc = ResourceDescForTypeIDCase::SamplerDesc;

    // Regular 1D textures.
    cases.push_back({.wgslTypes = {{"texture_1d<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA8Unorm,
                         .dimension = wgpu::TextureDimension::e1D,
                     }});
    cases.push_back({.wgslTypes = {{"texture_1d<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Float,
                         .dimension = wgpu::TextureDimension::e1D,
                     }});
    cases.push_back({.wgslTypes = {{"texture_1d<i32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Sint,
                         .dimension = wgpu::TextureDimension::e1D,
                     }});
    cases.push_back({.wgslTypes = {{"texture_1d<u32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Uint,
                         .dimension = wgpu::TextureDimension::e1D,
                     }});

    // Regular 2D textures.
    cases.push_back({.wgslTypes = {{"texture_2d<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA8Unorm,
                         .dimension = wgpu::TextureDimension::e2D,
                     }});
    cases.push_back({.wgslTypes = {{"texture_2d<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Float,
                         .dimension = wgpu::TextureDimension::e2D,
                     }});
    cases.push_back({.wgslTypes = {{"texture_2d<i32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Sint,
                         .dimension = wgpu::TextureDimension::e2D,
                     }});
    cases.push_back({.wgslTypes = {{"texture_2d<u32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Uint,
                         .dimension = wgpu::TextureDimension::e2D,
                     }});

    // Regular 2D array textures.
    cases.push_back({.wgslTypes = {{"texture_2d_array<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA8Unorm,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::e2DArray,
                     }});
    cases.push_back({.wgslTypes = {{"texture_2d_array<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Float,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::e2DArray,
                     }});
    cases.push_back({.wgslTypes = {{"texture_2d_array<i32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Sint,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::e2DArray,
                     }});
    cases.push_back({.wgslTypes = {{"texture_2d_array<u32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Uint,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::e2DArray,
                     }});

    // Regular cube textures.
    cases.push_back({.wgslTypes = {{"texture_cube<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA8Unorm,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::Cube,
                     }});
    cases.push_back({.wgslTypes = {{"texture_cube<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Float,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::Cube,
                     }});
    cases.push_back({.wgslTypes = {{"texture_cube<i32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Sint,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::Cube,
                     }});
    cases.push_back({.wgslTypes = {{"texture_cube<u32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Uint,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::Cube,
                     }});

    // Regular cube array textures.
    cases.push_back({.wgslTypes = {{"texture_cube_array<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA8Unorm,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::CubeArray,
                     }});
    cases.push_back({.wgslTypes = {{"texture_cube_array<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Float,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::CubeArray,
                     }});
    cases.push_back({.wgslTypes = {{"texture_cube_array<i32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Sint,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::CubeArray,
                     }});
    cases.push_back({.wgslTypes = {{"texture_cube_array<u32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Uint,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::CubeArray,
                     }});

    // Regular 3d textures.
    cases.push_back({.wgslTypes = {{"texture_3d<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA8Unorm,
                         .dimension = wgpu::TextureDimension::e3D,
                     }});
    cases.push_back({.wgslTypes = {{"texture_3d<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Float,
                         .dimension = wgpu::TextureDimension::e3D,
                     }});
    cases.push_back({.wgslTypes = {{"texture_3d<i32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Sint,
                         .dimension = wgpu::TextureDimension::e3D,
                     }});
    cases.push_back({.wgslTypes = {{"texture_3d<u32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA32Uint,
                         .dimension = wgpu::TextureDimension::e3D,
                     }});

    // Color multisampled textures.
    cases.push_back({.wgslTypes = {{"texture_multisampled_2d<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA8Unorm,
                         .dimension = wgpu::TextureDimension::e2D,
                         .sampleCount = 4,
                     }});
    cases.push_back({.wgslTypes = {{"texture_multisampled_2d<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA16Float,
                         .dimension = wgpu::TextureDimension::e2D,
                         .sampleCount = 4,
                     }});
    cases.push_back({.wgslTypes = {{"texture_multisampled_2d<i32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA16Sint,
                         .dimension = wgpu::TextureDimension::e2D,
                         .sampleCount = 4,
                     }});
    cases.push_back({.wgslTypes = {{"texture_multisampled_2d<u32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::RGBA16Uint,
                         .dimension = wgpu::TextureDimension::e2D,
                         .sampleCount = 4,
                     }});

    // Depth textures (including multisampled).
    cases.push_back({.wgslTypes = {{"texture_depth_2d"}, {"texture_2d<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::Depth32Float,
                         .dimension = wgpu::TextureDimension::e2D,
                     }});
    cases.push_back({.wgslTypes = {{"texture_depth_2d_array"}, {"texture_2d_array<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::Depth32Float,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::e2DArray,
                     }});
    cases.push_back({.wgslTypes = {{"texture_depth_cube"}, {"texture_cube<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::Depth32Float,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::Cube,
                     }});
    cases.push_back({.wgslTypes = {{"texture_depth_cube_array"}, {"texture_cube_array<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::Depth32Float,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::CubeArray,
                     }});
    cases.push_back({.wgslTypes = {{"texture_depth_multisampled_2d"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::Depth32Float,
                         .dimension = wgpu::TextureDimension::e2D,
                         .sampleCount = 4,
                     }});

    // Stencil textures can be used as 2D.
    cases.push_back({.wgslTypes = {{"texture_2d<u32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::Stencil8,
                         .dimension = wgpu::TextureDimension::e2D,
                     }});
    cases.push_back({.wgslTypes = {{"texture_2d_array<u32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::Stencil8,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::e2DArray,
                     }});
    cases.push_back({.wgslTypes = {{"texture_cube<u32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::Stencil8,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::Cube,
                     }});
    cases.push_back({.wgslTypes = {{"texture_cube_array<u32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::Stencil8,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewDimension = wgpu::TextureViewDimension::CubeArray,
                     }});

    // Depth-stencil textures with only one aspect selected.
    cases.push_back({.wgslTypes = {{"texture_depth_2d"}, {"texture_2d<f32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::Depth24PlusStencil8,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewAspect = wgpu::TextureAspect::DepthOnly,
                     }});
    cases.push_back({.wgslTypes = {{"texture_2d<u32>"}},
                     .desc = TextureDesc{
                         .format = wgpu::TextureFormat::Depth24PlusStencil8,
                         .dimension = wgpu::TextureDimension::e2D,
                         .viewAspect = wgpu::TextureAspect::StencilOnly,
                     }});

    // Non-filtering sampler
    cases.push_back({
        .wgslTypes = {{"sampler"}},
        .desc = SamplerDesc{},
    });
    // Filtering sampler
    cases.push_back({
        .wgslTypes = {{"sampler"}},
        .desc =
            SamplerDesc{
                .filtering = true,
            },
    });
    // Comparison sampler
    cases.push_back({
        .wgslTypes = {{"sampler_comparison"}},
        .desc =
            SamplerDesc{
                .comparison = true,
            },
    });

    return cases;
}

// Test that hasResource() works as expected for all supported resources in WGSL.
TEST_P(ResourceTableTests, HasResourceCompatibilityAllTypes) {
    // We rely on RGBA32Float being unfilterable for this test so that we can test both filterable /
    // unfilterable float without needing any additional extensions.
    DAWN_ASSERT(!device.HasFeature(wgpu::FeatureName::Float32Filterable));

    auto cases = MakeDescForTypeIDCases();

    // Make a resource table with all of our test texture views.
    wgpu::ResourceTable table = MakeResourceTable(cases.size());
    for (auto [i, c] : Enumerate(cases)) {
        if (std::holds_alternative<ResourceDescForTypeIDCase::TextureDesc>(c.desc)) {
            wgpu::BindingResource resource = {.textureView = c.CreateTestView(device)};
            EXPECT_EQ(wgpu::Status::Success, table.Update(i, &resource));
        } else if (std::holds_alternative<ResourceDescForTypeIDCase::SamplerDesc>(c.desc)) {
            wgpu::BindingResource resource = {.sampler = c.CreateTestSampler(device)};
            EXPECT_EQ(wgpu::Status::Success, table.Update(i, &resource));
        }
    }

    // Test hasResource returning for each of the supported WGSL types, against each resource.
    for (auto wgslType : kWgslSampledTextureTypes) {
        std::vector<bool> expected;
        expected.reserve(cases.size());
        for (auto& c : cases) {
            // The reasons wgslTypes is a vector is because some textures can be
            // both filterable and non-filterable, so hasResource will return true for both types on
            // the one table entry with such a texture.
            expected.push_back(c.wgslTypes.contains(wgslType));
        }

        TestHasResource(table, expected, wgslType);
    }
}

// Test that calling hasResource() with values outside of the resource table size returns false.
TEST_P(ResourceTableTests, HasResourceOOBIsFalse) {
    // Create the test pipeline
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;

        @group(0) @binding(0) var<storage, read_write> result : array<u32, 4>;
        var<immediate> resourceCount : u32;
        @compute @workgroup_size(1) fn getArrayLengths() {
            result[0] = u32(hasResource<texture_2d<f32>>(resourceCount - 1));
            result[1] = u32(hasResource<texture_2d<f32>>(resourceCount));

            // Check against all the slots where the default resources are.
            var result2 = 0u;
            for (var i = 1u; i < 100; i++) {
                result2 += u32(hasResource<texture_2d<f32>>(resourceCount + i));
            }
            result[2] = result2;

            result[3] = u32(hasResource<texture_2d<f32>>(resourceCount + 10000000));
        }
    )");
    wgpu::ComputePipelineDescriptor csDesc = {.compute = {
                                                  .module = module,
                                              }};
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Create the test resource table.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R8Unorm,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::ResourceTable table = MakeResourceTable(3, {
                                                         {0, {.textureView = tex.CreateView()}},
                                                         {1, {.textureView = tex.CreateView()}},
                                                         {2, {.textureView = tex.CreateView()}},
                                                     });

    // Create the other test resources.
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = 4 * sizeof(uint32_t),
    };
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);
    wgpu::BindGroup resultBG =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, resultBuffer}});
    uint32_t resourceCount = table.GetSize();

    // Run the test and check results are the expected ones.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetResourceTable(table);
    pass.SetImmediates(0, &resourceCount, sizeof(resourceCount));
    pass.SetBindGroup(0, resultBG);
    pass.SetPipeline(pipeline);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    device.GetQueue().Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(1, resultBuffer, 0);
    EXPECT_BUFFER_U32_EQ(0, resultBuffer, 4);
    EXPECT_BUFFER_U32_EQ(0, resultBuffer, 8);
    EXPECT_BUFFER_U32_EQ(0, resultBuffer, 12);
}

// Check that the default bindings are of size 1 and filled with zeroes. This is not an exhaustive
// test (that's for the CTS) but tries to check a few different interesting cases (MS, DS, Cube, 2D
// array).
TEST_P(ResourceTableTests, DefaultBindingsAreZeroAndSizeOne) {
    // TODO(crbug.com/385158827): Fails on older WARP 10.0.19041.5794
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    // Create the test pipeline
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;

        @group(0) @binding(0) var<storage, read_write> error : u32;
        @group(0) @binding(1) var s : sampler;

        var<private> checkIndex = 0u;
        fn check(b : bool) {
            if (!b && error == 0) {
                error = 1 + checkIndex;
            }
            checkIndex++;
        }

        @compute @workgroup_size(1) fn checkDefault() {
            // Default texture_2d<f32>
            {
                check(!hasResource<texture_2d<f32>>(0));
                let t = getResource<texture_2d<f32>>(0);
                check(all(textureDimensions(t) == vec2(1)));
                check(textureNumLevels(t) == 1);
                check(all(textureLoad(t, vec2(0), 0) == vec4(0, 0, 0, 1)));
            }

            // Default texture_multisampled_2d
            {
                check(!hasResource<texture_multisampled_2d<u32>>(0));
                let t = getResource<texture_multisampled_2d<u32>>(0);
                check(all(textureDimensions(t) == vec2(1)));
                check(textureNumSamples(t) == 4);
                check(all(textureLoad(t, vec2(0), 0) == vec4(0, 0, 0, 1)));
            }

            // Default texture_depth_cube
            {
                check(!hasResource<texture_depth_cube>(0));
                let t = getResource<texture_depth_cube>(0);
                check(all(textureDimensions(t) == vec2(1)));
                check(textureNumLevels(t) == 1);
                check(textureSampleLevel(t, s, vec3(0), 0) == 0);
            }

            // Default texture_2d_array<i32>
            {
                check(!hasResource<texture_2d_array<i32>>(0));
                let t = getResource<texture_2d_array<i32>>(0);
                check(all(textureDimensions(t) == vec2(1)));
                check(textureNumLevels(t) == 1);
                check(textureNumLayers(t) == 1);
                check(all(textureLoad(t, vec2(0), 0, 0) == vec4(0, 0, 0, 1)));
            }
        }
    )");
    wgpu::ComputePipelineDescriptor csDesc = {.compute = {
                                                  .module = module,
                                              }};
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Create the test resources.
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = sizeof(uint32_t),
    };
    wgpu::Buffer errorBuffer = device.CreateBuffer(&bDesc);

    wgpu::BindGroup bg = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                              {
                                                  {0, errorBuffer},
                                                  {1, device.CreateSampler()},
                                              });
    wgpu::ResourceTable table = MakeResourceTable(0);

    // Run the test and check results are the expected ones.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetResourceTable(table);
    pass.SetBindGroup(0, bg);
    pass.SetPipeline(pipeline);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    device.GetQueue().Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(0, errorBuffer, 0);
}

// Test that a resource table texture can be sampled by a resource table sampler.
TEST_P(ResourceTableTests, Sampler) {
    // TODO(https://issues.chromium.org/issues/490066027): Fails on Mali G78
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsARM());

    // Create a 1x1 texture with a single red pixel.
    wgpu::TextureDescriptor texDesc;
    texDesc.size = {1, 1};
    texDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    texDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
    wgpu::Texture texture = device.CreateTexture(&texDesc);

    const utils::RGBA8 red = utils::RGBA8::kRed;
    wgpu::TexelCopyTextureInfo srcInfo = utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
    wgpu::TexelCopyBufferLayout dstInfo = {};
    wgpu::Extent3D copySize = {1, 1, 1};
    queue.WriteTexture(&srcInfo, &red, sizeof(red), &dstInfo, &copySize);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;

        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }

        @fragment fn fs() -> @location(0) vec4f {
            let s = getResource<sampler>(0);
            let t = getResource<texture_2d<f32>>(1);
            return textureSample(t, s, vec2f(0.5, 0.5));
        }
    )");

    // Create the pipeline.
    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.cFragment.module = module;
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    wgpu::ResourceTable table = MakeResourceTable(2, {
                                                         {0, {.sampler = sampler}},
                                                         {1, {.textureView = texture.CreateView()}},
                                                     });

    // Create a 1x1 render target to verify the result.
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    // Render.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetResourceTable(table);
    pass.Draw(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Verify the result is red.
    EXPECT_PIXEL_RGBA8_EQ(red, renderPass.color, 0, 0);
}

// Test that a resource table texture can be sampled by multiple resource table samplers.
TEST_P(ResourceTableTests, MultipleSamplers) {
    // TODO(https://issues.chromium.org/issues/490066027): Fails on Mali G78
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsARM());
    // TODO(https://crbug.com/510904606): Fails on LLVMPipe
    DAWN_SUPPRESS_TEST_IF(IsMesaSoftware());

    struct Case {
        size_t tableSize;
        uint32_t samplerIndex0;
        uint32_t samplerIndex1;
        uint32_t samplerIndex2;
        uint32_t textureIndex;
    };
    constexpr auto kMax = kMaxResourceTableSize;
    Case cases[] = {
        {4, 0, 1, 2, 3},
        {2048, 2048 - 1, 2048 - 2, 2048 - 3, 2048 - 4},
        {kMax, kMax - 1, kMax - 2, kMax - 3, kMax - 4},
        {2048, 2048 / 4 * 1 - 1, 2048 / 4 * 2 - 1, 2048 / 4 * 3 - 1, 2048 / 4 * 4 - 1},
        {kMax, kMax / 4 * 1 - 1, kMax / 4 * 2 - 1, kMax / 4 * 3 - 1, kMax / 4 * 4 - 1},
    };

    for (auto c : cases) {
        wgpu::ShaderModule module =
            utils::CreateShaderModule(device, absl::StrFormat(R"(
            enable chromium_experimental_resource_table;

            @group(0) @binding(0) var<storage, read_write> results : array<vec4f>;

            @vertex fn vs() -> @builtin(position) vec4f {
                return vec4f(0, 0, 0.5, 0.5);
            }

            @fragment fn fs() -> @location(0) vec4f {
                let samplerRepeat = getResource<sampler>(%u);
                let samplerMirror = getResource<sampler>(%u);
                let samplerClamp = getResource<sampler>(%u);
                let t = getResource<texture_2d<f32>>(%u);

                results[0] = textureSample(t, samplerRepeat, vec2f(1, 0));
                results[1] = textureSample(t, samplerRepeat, vec2f(1.5, 0));

                results[2] = textureSample(t, samplerMirror, vec2f(1, 0));
                results[3] = textureSample(t, samplerMirror, vec2f(1.5, 0));

                results[4] = textureSample(t, samplerClamp, vec2f(1, 0));
                results[5] = textureSample(t, samplerClamp, vec2f(1.5, 0));

                return vec4f(0);
            }
        )",
                                                              c.samplerIndex0, c.samplerIndex1,
                                                              c.samplerIndex2, c.textureIndex));

        // Create the pipeline.
        utils::ComboRenderPipelineDescriptor pDesc;
        pDesc.vertex.module = module;
        pDesc.cFragment.module = module;
        pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

        // Create the texture
        wgpu::Texture texture = CreateCheckerboardTexture();

        // Create 3 samplers
        wgpu::Sampler samplerRepeat = CreateSampler(wgpu::AddressMode::Repeat);
        wgpu::Sampler samplerMirror = CreateSampler(wgpu::AddressMode::MirrorRepeat);
        wgpu::Sampler samplerClamp = CreateSampler(wgpu::AddressMode::ClampToEdge);

        // Create the result buffer resource
        wgpu::BufferDescriptor bDesc = {
            .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
            .size = sizeof(float) * 4 * 6,  // 6 vec4fs
        };
        wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);
        wgpu::BindGroup resultBG =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, resultBuffer}});

        // Create resource table and add the samplers and texture view to it
        wgpu::ResourceTable table = MakeResourceTable(
            c.tableSize, {
                             {c.samplerIndex0, {.sampler = samplerRepeat}},
                             {c.samplerIndex1, {.sampler = samplerMirror}},
                             {c.samplerIndex2, {.sampler = samplerClamp}},
                             {c.textureIndex, {.textureView = texture.CreateView()}},
                         });

        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, resultBG);
        pass.SetResourceTable(table);
        pass.Draw(1);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        float expectedGreen[4] = {0.0, 1.0, 0.0, 1.0};
        float expectedRed[4] = {1.0, 0.0, 0.0, 1.0};

        // repeat: 1,0 -> red, 1.5,0 -> green
        EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedRed, resultBuffer, 0ULL * 4 * sizeof(float), 4);
        EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedGreen, resultBuffer, 1ULL * 4 * sizeof(float), 4);

        // mirror: 1,0 -> green, 1.5,0 -> red
        EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedGreen, resultBuffer, 2ULL * 4 * sizeof(float), 4);
        EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedRed, resultBuffer, 3ULL * 4 * sizeof(float), 4);

        // clamp: 1,0 -> green, 1.5,0 -> green
        EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedGreen, resultBuffer, 4ULL * 4 * sizeof(float), 4);
        EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedGreen, resultBuffer, 5ULL * 4 * sizeof(float), 4);
    }
}

// Test that default samplers are correctly created, and accessed when an invalid index it provided.
TEST_P(ResourceTableTests, UseDefaultSamplers) {
    // TODO(https://issues.chromium.org/issues/490066027): Fails on Mali G78
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsARM());

    auto CreateDepthTexture = [&]() {
        wgpu::TextureDescriptor descriptor;
        // descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size = {1, 1};
        descriptor.format = wgpu::TextureFormat::Depth24Plus;
        descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
        return device.CreateTexture(&descriptor);
    };

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;

        @group(0) @binding(0) var<storage, read_write> results : array<vec4f>;

        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }

        @fragment fn fs() -> @location(0) vec4f {
            let t = getResource<texture_2d<f32>>(0);
            let dt = getResource<texture_depth_2d>(1);

            let samplerNonFiltering = getResource<sampler>(2);
            let samplerFiltering = getResource<sampler>(3);
            let samplerComparison = getResource<sampler_comparison>(4);

            results[0] = textureSample(t, samplerNonFiltering, vec2f(0.5, 0.5));
            results[1] = textureSample(t, samplerNonFiltering, vec2f(0.6, 0.6));

            results[2] = textureSample(t, samplerFiltering, vec2f(0.5, 0.5));
            results[3] = textureSample(t, samplerFiltering, vec2f(0.6, 0.6));

            let c = textureSampleCompare(dt, samplerComparison, vec2f(0.5, 0.5), 0.5);
            results[4] = vec4f(c, 42.0f, c, 83.5f);

            return vec4f(0);
        }
    )");

    // Create the pipeline.
    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.cFragment.module = module;
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    // Create the textures
    wgpu::Texture colorTexture = CreateCheckerboardTexture();
    wgpu::Texture depthTexture = CreateDepthTexture();

    // Create the result buffer resource
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = sizeof(float) * 4 * 5,  // 5 vec4fs
    };
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);
    wgpu::BindGroup resultBG =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, resultBuffer}});

    // Create resource table and only textures to it, no samplers, so that the default
    // samplers get used
    wgpu::ResourceTable table =
        MakeResourceTable(10, {
                                  {0, {.textureView = colorTexture.CreateView()}},
                                  {1, {.textureView = depthTexture.CreateView()}},
                              });

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, resultBG);
    pass.SetResourceTable(table);
    pass.Draw(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    float expectedRed[4] = {1.0, 0.0, 0.0, 1.0};
    float expectedGreen[4] = {1.0, 0.0, 0.0, 1.0};

    // The default non-filtering sampler should return red at (0.5, 0.5), and green at (0.6, 0.6)
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedRed, resultBuffer, 0ULL * 4 * sizeof(float), 4);
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedGreen, resultBuffer, 1ULL * 4 * sizeof(float), 4);

    // The default filtering sampler is actually a non-filtering one, so should return the same
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedRed, resultBuffer, 2ULL * 4 * sizeof(float), 4);
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedGreen, resultBuffer, 3ULL * 4 * sizeof(float), 4);

    // The comparison sampler is an 'always' one, so it should return 1.0, which the shader returns
    // in two of the vector elements.
    float expectedCompare[4] = {1.0, 42.0, 1.0, 83.5};
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedCompare, resultBuffer, 4ULL * 4 * sizeof(float), 4);
}

// Test that removing then adding a new sampler in a slot that already has a sampler of the same
// type (e.g. filterable) works as expected. This ensures, for example, that the metadata buffer
// gets an updated sampler index on D3D12.
TEST_P(ResourceTableTests, RemoveThenAddSamplerInSameSlot) {
    // TODO(https://issues.chromium.org/issues/490066027): Fails on Mali G78
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsARM());
    // TODO(https://crbug.com/510904606): Fails on LLVMPipe
    DAWN_SUPPRESS_TEST_IF(IsMesaSoftware());

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;

        @group(0) @binding(0) var<storage, read_write> results : array<vec4f>;

        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }

        @fragment fn fs() -> @location(0) vec4f {
            let t = getResource<texture_2d<f32>>(0);
            let s = getResource<sampler>(1);

            results[0] = textureSample(t, s, vec2f(1, 0));
            results[1] = textureSample(t, s, vec2f(1.5, 0));
            return vec4f(0);
        }
    )");

    // Create the pipeline.
    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.cFragment.module = module;
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    // Create texture
    wgpu::Texture texture = CreateCheckerboardTexture();

    // Create samplers
    wgpu::Sampler samplerRepeat = CreateSampler(wgpu::AddressMode::Repeat);
    wgpu::Sampler samplerMirror = CreateSampler(wgpu::AddressMode::MirrorRepeat);

    // Create the result buffer resource
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = sizeof(float) * 4 * 2,  // 2 vec4fs
    };
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);
    wgpu::BindGroup resultBG =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, resultBuffer}});

    // Create resource table
    wgpu::ResourceTable table = MakeResourceTable(2, {
                                                         {0, {.textureView = texture.CreateView()}},
                                                     });

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    auto draw = [&]() {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, resultBG);
        pass.SetResourceTable(table);
        pass.Draw(1);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    };

    wgpu::BindingResource res;
    float expectedGreen[4] = {0.0, 1.0, 0.0, 1.0};
    float expectedRed[4] = {1.0, 0.0, 0.0, 1.0};

    // Add repeat sampler and draw
    res = {.sampler = samplerRepeat};
    EXPECT_EQ(wgpu::Status::Success, table.Update(1, &res));
    draw();

    // repeat: 1,0 -> red, 1.5,0 -> green
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedRed, resultBuffer, 0ULL * 4 * sizeof(float), 4);
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedGreen, resultBuffer, 1ULL * 4 * sizeof(float), 4);

    // Now test removing then adding mirror sampler
    EXPECT_EQ(wgpu::Status::Success, table.Remove(1));
    WaitForAllOperations();
    res = {.sampler = samplerMirror};
    EXPECT_EQ(wgpu::Status::Success, table.Update(1, &res));
    draw();

    // mirror: 1,0 -> green, 1.5,0 -> red
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedGreen, resultBuffer, 0ULL * 4 * sizeof(float), 4);
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedRed, resultBuffer, 1ULL * 4 * sizeof(float), 4);
}

// Test that adding and removing samplers in the same slot between draws ensure that
// backends only see the effective diff (first one added to last one added).
TEST_P(ResourceTableTests, RemoveThenAddSamplerMultipleInSameSlot) {
    // TODO(https://issues.chromium.org/issues/490066027): Fails on Mali G78
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsARM());

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;

        @group(0) @binding(0) var<storage, read_write> results : array<vec4f>;

        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }

        @fragment fn fs() -> @location(0) vec4f {
            let t = getResource<texture_2d<f32>>(0);
            let s = getResource<sampler>(1);

            results[0] = textureSample(t, s, vec2f(1, 0));
            results[1] = textureSample(t, s, vec2f(1.5, 0));
            return vec4f(0);
        }
    )");

    // Create the pipeline.
    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.cFragment.module = module;
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    // Create texture
    wgpu::Texture texture = CreateCheckerboardTexture();

    // Create samplers
    wgpu::Sampler samplerRepeat = CreateSampler(wgpu::AddressMode::Repeat);
    wgpu::Sampler samplerMirror[] = {
        // Create different types to make sure they don't get de-duped
        CreateSampler(wgpu::AddressMode::MirrorRepeat, wgpu::CompareFunction::Always),
        CreateSampler(wgpu::AddressMode::MirrorRepeat, wgpu::CompareFunction::Equal),
        CreateSampler(wgpu::AddressMode::MirrorRepeat, wgpu::CompareFunction::GreaterEqual)};
    wgpu::Sampler samplerClamp = CreateSampler(wgpu::AddressMode::ClampToEdge);

    // Create the result buffer resource
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = sizeof(float) * 4 * 2,  // 2 vec4fs
    };
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);
    wgpu::BindGroup resultBG =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, resultBuffer}});

    // Create resource table and add the samplers and texture view to it
    wgpu::ResourceTable table = MakeResourceTable(2, {
                                                         {0, {.textureView = texture.CreateView()}},
                                                     });

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    auto draw = [&]() {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, resultBG);
        pass.SetResourceTable(table);
        pass.Draw(1);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    };

    wgpu::BindingResource res;
    float expectedGreen[4] = {0.0, 1.0, 0.0, 1.0};
    float expectedRed[4] = {1.0, 0.0, 0.0, 1.0};

    // Add repeat sampler and draw
    res = {.sampler = samplerRepeat};
    EXPECT_EQ(wgpu::Status::Success, table.Update(1, &res));
    draw();

    // repeat: 1,0 -> red, 1.5,0 -> green
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedRed, resultBuffer, 0ULL * 4 * sizeof(float), 4);
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedGreen, resultBuffer, 1ULL * 4 * sizeof(float), 4);

    // Remove then add mirror samplers
    for (auto sampler : samplerMirror) {
        EXPECT_EQ(wgpu::Status::Success, table.Remove(1));
        WaitForAllOperations();
        res = {.sampler = sampler};
        EXPECT_EQ(wgpu::Status::Success, table.Update(1, &res));
    }

    // Finally, remove and add clamp sampler
    EXPECT_EQ(wgpu::Status::Success, table.Remove(1));
    WaitForAllOperations();
    res = {.sampler = samplerClamp};
    EXPECT_EQ(wgpu::Status::Success, table.Update(1, &res));

    // Now draw. Backends should basically ignore the adding and removal of the mirror sampler.
    draw();

    // clamp: 1,0 -> green, 1.5,0 -> green
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedGreen, resultBuffer, 0ULL * 4 * sizeof(float), 4);
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedGreen, resultBuffer, 1ULL * 4 * sizeof(float), 4);
}

// Test what happens when we add more than kD3D12MaxUniqueSamplers unique samplers
TEST_P(ResourceTableTests, AddUniqueSamplersOverLimit) {
    // TODO(https://issues.chromium.org/issues/490066027): Fails on Mali G78
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsARM());

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @compute @workgroup_size(1) fn main() {
            _ = hasResource<texture_2d<f32>>(0);
        }
    )");
    auto pipeline = device.CreateComputePipeline(&csDesc);

    auto dispatch = [&](wgpu::ResourceTable table, bool shouldSucceed = true) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetResourceTable(table);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        if (shouldSucceed) {
            queue.Submit(1, &commands);
        } else {
            ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        }
    };

    wgpu::ResourceTable table = MakeResourceTable(2049);

    // Initial draw so that default resources are processed
    dispatch(table);

    // Add kD3D12MaxUniqueSamplers, should all succeed
    for (auto i : Range(kD3D12MaxUniqueSamplers)) {
        wgpu::BindingResource br{.sampler = CreateUniqueSampler(i)};
        EXPECT_EQ(wgpu::Status::Success, table.Update(i, &br));
    }

    dispatch(table);

    // Now add one more, should fail
    {
        wgpu::BindingResource br{.sampler = CreateUniqueSampler(kD3D12MaxUniqueSamplers)};
        EXPECT_EQ(wgpu::Status::Success, table.Update(kD3D12MaxUniqueSamplers, &br));
    }

    bool shouldSucceed = !IsD3D12();
    dispatch(table, shouldSucceed);
}

// Test that adding a sampler, draw, then remove and add a duplicate sampler and draw works. This
// tests that deduplication logic handles when no more refs are left to a sampler, and then a
// duplicate sampler is added back. On D3D12, internally this may result in a new sampler index,
// which should be fine.
TEST_P(ResourceTableTests, RemoveAddDuplicateSampler) {
    // TODO(https://issues.chromium.org/issues/490066027): Fails on Mali G78
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsARM());

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @compute @workgroup_size(1) fn main() {
            _ = hasResource<texture_2d<f32>>(0);
        }
    )");
    auto pipeline = device.CreateComputePipeline(&csDesc);

    auto dispatch = [&](wgpu::ResourceTable table) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetResourceTable(table);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    };

    wgpu::ResourceTable table = MakeResourceTable(2);

    // Initial draw so that default resources are processed
    dispatch(table);

    {
        // Add a unique sampler in slot 1
        wgpu::BindingResource br{.sampler = CreateUniqueSampler(42)};
        EXPECT_EQ(wgpu::Status::Success, table.Update(1, &br));
    }
    dispatch(table);

    {
        // Remove the sampler in slot 1
        EXPECT_EQ(wgpu::Status::Success, table.Remove(1));

        WaitForAllOperations();

        // Add another sampler in slot 0
        // This is to coax the backend into potentially assigning a different sampler index for this
        // same sampler.
        wgpu::BindingResource br{.sampler = CreateUniqueSampler(123)};
        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &br));

        // Add back the origin sampler in slot 1
        wgpu::BindingResource br2{.sampler = CreateUniqueSampler(42)};
        EXPECT_EQ(wgpu::Status::Success, table.Update(1, &br2));
    }

    dispatch(table);
}

// On D3D12 the number of sampler descriptors in a GPU heap is limited to 2048. We deduplicate
// samplers, so test that we can add more than 2048 samplers to the resource table, as long as there
// are duplicates. We test this by creating a table of size 2*2048, add 2048 unique samplers in the
// first half of the table, then add the same set of sampers in the second half of the table.
TEST_P(ResourceTableTests, AddDuplicateSamplersTwice) {
    // TODO(https://issues.chromium.org/issues/490066027): Fails on Mali G78
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsARM());

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @compute @workgroup_size(1) fn main() {
            _ = hasResource<texture_2d<f32>>(0);
        }
    )");
    auto pipeline = device.CreateComputePipeline(&csDesc);

    auto dispatch = [&](wgpu::ResourceTable table) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetResourceTable(table);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    };

    // Make a table with double the max number of unique samplers on D3D12.
    wgpu::ResourceTable table = MakeResourceTable(kD3D12MaxUniqueSamplers * 2);

    // Add samplers to slots 0..kD3D12MaxUniqueSamplers-1
    for (uint32_t i : Range(kD3D12MaxUniqueSamplers)) {
        wgpu::BindingResource br{.sampler = CreateUniqueSampler(i)};
        EXPECT_EQ(wgpu::Status::Success, table.Update(i, &br));
    }

    // Draw so that the CPU to GPU sampler heap creation and copy occurs.
    dispatch(table);

    // Add the same samplers to slots kD3D12MaxUniqueSamplers..kD3D12MaxUniqueSamplers*2-1
    for (uint32_t i : Range(kD3D12MaxUniqueSamplers)) {
        wgpu::BindingResource br{.sampler = CreateUniqueSampler(i)};
        EXPECT_EQ(wgpu::Status::Success, table.Update(i + kD3D12MaxUniqueSamplers, &br));
    }

    // Draw again. If we didn't handle removal of samplers correctly, this will fail
    // for surpassing the 2048 sampler descriptor per heap limit.
    dispatch(table);
}

// On D3D12 the number of sampler descriptors in a GPU heap is limited to 2048. This test
// allocates a resource table larger than that, adds 2048 unique samplers, draws, removes them all,
// then adds 2048 unique samplers (different from the first set) again in different slots,
// and draws again, making sure this is supported. Effectively, this tests that the resource table
// correctly handles removal of samplers.
TEST_P(ResourceTableTests, AddAndRemoveMaxSamplersTwice) {
    // TODO(https://issues.chromium.org/issues/490066027): Fails on Mali G78
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsARM());
    // TODO(https://issues.chromium.org/issues/512829734): Fails on Vulkan if
    // VkPhysicalDeviceLimits::maxSamplerAllocationCount <= 4K
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsWindows() && IsIntel());

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @compute @workgroup_size(1) fn main() {
            _ = hasResource<texture_2d<f32>>(0);
        }
    )");
    auto pipeline = device.CreateComputePipeline(&csDesc);

    auto dispatch = [&](wgpu::ResourceTable table) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetResourceTable(table);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    };

    // Make a table with double the max number of samplers on D3D12.
    wgpu::ResourceTable table = MakeResourceTable(kD3D12MaxUniqueSamplers * 2);

    // Add samplers to slots 0..kD3D12MaxUniqueSamplers-1
    for (uint32_t i : Range(kD3D12MaxUniqueSamplers)) {
        // Make samplers unique by making one of the values different for each
        wgpu::BindingResource br{.sampler = CreateUniqueSampler(i)};
        EXPECT_EQ(wgpu::Status::Success, table.Update(i, &br));
    }

    // Draw so that the CPU to GPU sampler heap creation and copy occurs.
    dispatch(table);

    // Remove all samplers
    for (uint32_t i : Range(kD3D12MaxUniqueSamplers)) {
        EXPECT_EQ(wgpu::Status::Success, table.Remove(i));
    }

    // Add different samplers to slots kD3D12MaxUniqueSamplers..kD3D12MaxUniqueSamplers*2-1
    for (uint32_t i : Range(kD3D12MaxUniqueSamplers)) {
        wgpu::BindingResource br{.sampler = CreateUniqueSampler(kD3D12MaxUniqueSamplers + i)};
        EXPECT_EQ(wgpu::Status::Success, table.Update(i + kD3D12MaxUniqueSamplers, &br));
    }

    // Draw again. If we didn't handle removal of samplers correctly, this will fail
    // for surpassing the 2048 sampler descriptor per heap limit.
    dispatch(table);
}

// Test that removing then adding a texture in a slot works as expected.
TEST_P(ResourceTableTests, RemoveThenAddTextureInSameSlot) {
    // TODO(https://issues.chromium.org/issues/490066027): Fails on Mali G78
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsARM());

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;

        @group(0) @binding(0) var<storage, read_write> results : array<vec4f>;

        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }

        @fragment fn fs() -> @location(0) vec4f {
            let t = getResource<texture_2d<f32>>(0);
            let s = getResource<sampler>(1);
            results[0] = textureSample(t, s, vec2f(0.5, 0.5));
            return vec4f(0);
        }
    )");

    // Create the pipeline.
    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.cFragment.module = module;
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    // Create textures
    wgpu::Texture textureRed = CreateColorTexture(utils::RGBA8::kRed);
    wgpu::Texture textureGreen = CreateColorTexture(utils::RGBA8::kGreen);

    // Create the result buffer resource
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = sizeof(float) * 4,  // 1 vec4fs
    };
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);
    wgpu::BindGroup resultBG =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, resultBuffer}});

    // Create resource table
    wgpu::ResourceTable table = MakeResourceTable(1);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    auto draw = [&]() {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, resultBG);
        pass.SetResourceTable(table);
        pass.Draw(1);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    };

    wgpu::BindingResource res;
    float expectedRed[4] = {1.0, 0.0, 0.0, 1.0};
    float expectedGreen[4] = {0.0, 1.0, 0.0, 1.0};

    // Add red texture and draw
    res = {.textureView = textureRed.CreateView()};
    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &res));
    draw();
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedRed, resultBuffer, 0, 4);

    // Now test removing and adding the green texture in the same slot
    EXPECT_EQ(wgpu::Status::Success, table.Remove(0));
    WaitForAllOperations();
    res = {.textureView = textureGreen.CreateView()};
    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &res));
    draw();
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedGreen, resultBuffer, 0, 4);
}

// Test that adding and removing textures in the same slot between draws ensure that
// backends only see the effective diff (first one added to last one added).
TEST_P(ResourceTableTests, RemoveThenAddTextureMultipleInSameSlot) {
    // TODO(https://issues.chromium.org/issues/490066027): Fails on Mali G78
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsARM());

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;

        @group(0) @binding(0) var<storage, read_write> results : array<vec4f>;

        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }

        @fragment fn fs() -> @location(0) vec4f {
            let t = getResource<texture_2d<f32>>(0);
            let s = getResource<sampler>(1);
            results[0] = textureSample(t, s, vec2f(0.5, 0.5));
            return vec4f(0);
        }
    )");

    // Create the pipeline.
    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.cFragment.module = module;
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    // Create textures
    wgpu::Texture textureRed = CreateColorTexture(utils::RGBA8::kRed);
    wgpu::Texture textureGreen = CreateColorTexture(utils::RGBA8::kGreen);
    wgpu::Texture textureBlue = CreateColorTexture(utils::RGBA8::kBlue);

    // Create the result buffer resource
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = sizeof(float) * 4,  // 1 vec4fs
    };
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);
    wgpu::BindGroup resultBG =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, resultBuffer}});

    // Create resource table
    wgpu::ResourceTable table = MakeResourceTable(1);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    auto draw = [&]() {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, resultBG);
        pass.SetResourceTable(table);
        pass.Draw(1);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    };

    wgpu::BindingResource res;
    float expectedRed[4] = {1.0, 0.0, 0.0, 1.0};
    float expectedBlue[4] = {0.0, 0.0, 1.0, 1.0};

    // Add red texture and draw
    res = {.textureView = textureRed.CreateView()};
    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &res));
    draw();
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedRed, resultBuffer, 0, 4);

    // Remove then add green texture (this could be done in a loop with multiple textures)
    EXPECT_EQ(wgpu::Status::Success, table.Remove(0));
    WaitForAllOperations();
    res = {.textureView = textureGreen.CreateView()};
    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &res));

    // Now test removing and adding the blue texture in the same slot
    EXPECT_EQ(wgpu::Status::Success, table.Remove(0));
    WaitForAllOperations();
    res = {.textureView = textureBlue.CreateView()};
    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &res));
    draw();
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedBlue, resultBuffer, 0, 4);
}

// Check that zero-initialization of the resources happens implicitly
TEST_P(ResourceTableTests, ImplicitZeroInit) {
    // TODO(crbug.com/385158827): Fails on older WARP 10.0.19041.5794
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    // Create the pipeline reading back from the texture.
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;

        @group(0) @binding(0) var<storage, read_write> result : u32;

        @compute @workgroup_size(1) fn readbackPixel() {
            let errorIfNotPresent = u32(!hasResource<texture_2d<u32>>(0));
            let tex = getResource<texture_2d<u32>>(0);
            let texel = textureLoad(tex, vec2u(0), 0).r;
            result = errorIfNotPresent + texel;
        }
    )");

    wgpu::ComputePipelineDescriptor csDesc = {.compute = {
                                                  .module = module,
                                              }};
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Create the test resource table.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Uint,
    };
    wgpu::TextureViewDescriptor vDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::ResourceTable table =
        MakeResourceTable(1, {{0, {.textureView = tex.CreateView(&vDesc)}}});

    // Create the other test resources.
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = sizeof(uint32_t),
    };
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);

    wgpu::BindGroup bg = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                              {
                                                  {0, resultBuffer},
                                              });

    // Check that the initial zero init happens implicitly.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetResourceTable(table);
        pass.SetBindGroup(0, bg);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);

        EXPECT_BUFFER_U32_EQ(0, resultBuffer, 0);
    }

    // Use a render pass discard to mark the texture as uninitialized again. Use a LoadOp::Clear to
    // set some non-zero value in the texture which hopefully would tell us if the lazy clear didn't
    // happen.
    {
        wgpu::RenderPassColorAttachment attachment = {
            .view = tex.CreateView(),
            .loadOp = wgpu::LoadOp::Clear,
            .storeOp = wgpu::StoreOp::Discard,
            .clearValue = {.r = 1.0, .g = 0.0, .b = 0.0, .a = 0.0},
        };
        wgpu::RenderPassDescriptor rpDesc = {
            .colorAttachmentCount = 1,
            .colorAttachments = &attachment,
        };

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rpDesc);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);
    }

    // Check that the zero init happens implicitly after a discard.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetResourceTable(table);
        pass.SetBindGroup(0, bg);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);

        EXPECT_BUFFER_U32_EQ(0, resultBuffer, 0);
    }
}

// Check that a resource table slot can be updated only after all commands submitted prior to
// Remove are completed.
TEST_P(ResourceTableTests, UpdateAfterRemoveRequiresGPUIsFinished) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Uint,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);
    wgpu::BindingResource resource{.textureView = tex.CreateView()};

    wgpu::ResourceTable table = MakeResourceTable(1);
    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));

    // Removing while the table is still potentially in used by the GPU is an error. But immediately
    // after we know that the GPU is finished, it is valid.
    bool updateValid = false;
    DoSomeWorkInSubmit();
    queue.OnSubmittedWorkDone(
        wgpu::CallbackMode::AllowSpontaneous,
        [&](wgpu::QueueWorkDoneStatus, wgpu::StringView) { updateValid = true; });
    EXPECT_EQ(wgpu::Status::Success, table.Remove(0));

    if (updateValid) {
        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
        updateValid = false;
    } else {
        EXPECT_EQ(wgpu::Status::Error, table.Update(0, &resource));
    }

    WaitForAllOperations();

    if (updateValid) {
        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
    } else {
        EXPECT_EQ(wgpu::Status::Error, table.Update(0, &resource));
    }
}

// Check that a resource table slot can be updated only after all commands submitted prior to
// Remove are completed.
TEST_P(ResourceTableTests, UpdateAfterRemoveRequiresGPUIsFinished_ErrorBindGroup) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Uint,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);
    wgpu::BindingResource resource{.textureView = tex.CreateView()};

    // Make an error resource table.
    wgpu::RenderPassMaxDrawCount maxDraw;
    maxDraw.maxDrawCount = 1000;
    wgpu::ResourceTableDescriptor desc{
        .nextInChain = &maxDraw,
        .size = 1,
    };
    wgpu::ResourceTable table;
    ASSERT_DEVICE_ERROR(table = device.CreateResourceTable(&desc));

    {
        // Ignore all validation errors for this test as they are tested in other places, and we're
        // checking immediate validation returned as a wgpu::Status and supposed to be the same for
        // valid and invalid objects.
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);

        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));

        // Removing while the table is still potentially in used by the GPU is an error. But
        // immediately after we know that the GPU is finished, it is valid.
        bool updateValid = false;
        DoSomeWorkInSubmit();
        queue.OnSubmittedWorkDone(
            wgpu::CallbackMode::AllowSpontaneous,
            [&](wgpu::QueueWorkDoneStatus, wgpu::StringView) { updateValid = true; });
        EXPECT_EQ(wgpu::Status::Success, table.Remove(0));

        if (updateValid) {
            EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
            updateValid = false;
        } else {
            EXPECT_EQ(wgpu::Status::Error, table.Update(0, &resource));
        }

        WaitForAllOperations();

        if (updateValid) {
            EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
        } else {
            EXPECT_EQ(wgpu::Status::Error, table.Update(0, &resource));
        }
    }
}

// Check that Update and Insert make the new binding visible in the resource table.
TEST_P(ResourceTableTests, UpdateAndInsertMakeBindingVisible) {
    // TODO(crbug.com/385158827): Fails on older WARP 10.0.19041.5794
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    wgpu::ResourceTable table = MakeResourceTable(2);

    // Before we do anything, the table has no valid entries.
    TestHasU8BindingsAll(table, {{}, {}});

    // Update makes the entry visible.
    wgpu::BindingResource resource0 = {.textureView = MakeU8View(17)};
    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource0));
    TestHasU8BindingsAll(table, {{17}, {}});

    // Insert makes the entry visible.
    wgpu::BindingResource resource1 = {.textureView = MakeU8View(42)};
    EXPECT_EQ(1u, table.Insert(&resource1));
    TestHasU8BindingsAll(table, {{17}, {42}});
}

// Check that Remove instantly makes the binding not visible, both for entries added with
// Update and Insert.
TEST_P(ResourceTableTests, RemoveMakeBindingInvalid) {
    // TODO(crbug.com/385158827): Fails on older WARP 10.0.19041.5794
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    // Fill a resource table with both Update and Insert.
    wgpu::ResourceTable table = MakeResourceTable(2);

    wgpu::BindingResource resource0 = {.textureView = MakeU8View(100)};
    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource0));

    wgpu::BindingResource resource1 = {.textureView = MakeU8View(101)};
    EXPECT_EQ(1u, table.Insert(&resource1));

    // Before we remove bindings, they are all valid.
    TestHasU8BindingsAll(table, {{100}, {101}});

    // Remove immediately makes bindings invalid.
    EXPECT_EQ(wgpu::Status::Success, table.Remove(1));
    TestHasU8BindingsAll(table, {{100}, {}});
    EXPECT_EQ(wgpu::Status::Success, table.Remove(0));
    TestHasU8BindingsAll(table, {{}, {}});
}

// Check that removing a binding and adding a different one works.
TEST_P(ResourceTableTests, ReplaceBinding) {
    // TODO(crbug.com/385158827): Fails on older WARP 10.0.19041.5794
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    // Create the test resource table.
    wgpu::ResourceTable table = MakeResourceTable(1);
    wgpu::BindingResource resource = {.textureView = MakeU8View(19)};
    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));

    // Test removing a binding that was previously there.
    TestHasU8BindingsAll(table, {{19}});
    EXPECT_EQ(wgpu::Status::Success, table.Remove(0));
    TestHasU8BindingsAll(table, {{}});

    /// Add it back a new entry, the shader should be seeing the updated entry.
    WaitForAllOperations();

    wgpu::BindingResource newResource = {.textureView = MakeU8View(23)};
    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &newResource));
    TestHasU8BindingsAll(table, {{23}});
}

// Check that removing a binding and adding it back works.
TEST_P(ResourceTableTests, ReplaceWithSameBinding) {
    // TODO(crbug.com/385158827): Fails on older WARP 10.0.19041.5794
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    // Create the test resource table.
    wgpu::ResourceTable table = MakeResourceTable(1);
    wgpu::BindingResource resource = {.textureView = MakeU8View(19)};
    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));

    // Test removing a binding that was previously there.
    TestHasU8BindingsAll(table, {{19}});
    EXPECT_EQ(wgpu::Status::Success, table.Remove(0));
    TestHasU8BindingsAll(table, {{}});

    /// Add it back a new entry, the shader should be seeing the updated entry.
    WaitForAllOperations();

    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
    TestHasU8BindingsAll(table, {{19}});
}

// Check that setting multiple resource table, on per dispatch, on a single pass works.
TEST_P(ResourceTableTests, SingleComputePassMultipleResourceTables) {
    // TODO(crbug.com/385158827): Fails on older WARP 10.0.19041.5794
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    std::vector<wgpu::BindingResource> resources;

    wgpu::ResourceTable table0 = MakeResourceTable(2);
    resources.push_back(wgpu::BindingResource{.textureView = MakeU8View(17)});
    EXPECT_EQ(wgpu::Status::Success, table0.Update(0, &resources.back()));
    resources.push_back(wgpu::BindingResource{.textureView = MakeU8View(18)});
    EXPECT_EQ(wgpu::Status::Success, table0.Update(1, &resources.back()));

    wgpu::ResourceTable table1 = MakeResourceTable(3);
    resources.push_back(wgpu::BindingResource{.textureView = MakeU8View(27)});
    EXPECT_EQ(wgpu::Status::Success, table1.Update(0, &resources.back()));
    // Leave slot 1 empty
    resources.push_back(wgpu::BindingResource{.textureView = MakeU8View(29)});
    EXPECT_EQ(wgpu::Status::Success, table1.Update(2, &resources.back()));

    wgpu::ResourceTable table2 = MakeResourceTable(4);
    resources.push_back(wgpu::BindingResource{.textureView = MakeU8View(37)});
    EXPECT_EQ(wgpu::Status::Success, table2.Update(0, &resources.back()));
    resources.push_back(wgpu::BindingResource{.textureView = MakeU8View(38)});
    EXPECT_EQ(wgpu::Status::Success, table2.Update(1, &resources.back()));
    // Leave slot 2 empty
    resources.push_back(wgpu::BindingResource{.textureView = MakeU8View(40)});
    EXPECT_EQ(wgpu::Status::Success, table2.Update(3, &resources.back()));

    auto case0 = TableAndExpected(table0, {{17, 18}});
    auto case1 = TableAndExpected(table1, {{27, {}, 29}});
    auto case2 = TableAndExpected(table2, {{37, 38, {}, 40}});

    TestHasU8BindingsCompute({case0, case1, case2});
    TestHasU8BindingsCompute({case1, case0, case2});
    TestHasU8BindingsCompute({case2, case1, case0});
}

// Check that logic to dirty or reuse VkDescriptorSet takes into account the resource table in the
// Vulkan backend.
TEST_P(ResourceTableTests, SwitchUseResourceTableAndNot) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;

        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }

        @group(0) @binding(0) var<storage, read_write> results : array<u32>;
        var<immediate> resultIndex : u32;

        @fragment fn yes_resource_table() -> @location(0) vec4f {
            results[resultIndex] = 10 + u32(hasResource<texture_2d<f32>>(resultIndex));
            return vec4();
        }

        @fragment fn no_resource_table() -> @location(0) vec4f {
            results[resultIndex] = 42;
            return vec4();
        }
    )");

    wgpu::BindGroupLayout resultBGL = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});

    wgpu::RenderPipeline resourceTablePipeline;
    {
        utils::ComboRenderPipelineDescriptor desc;
        desc.layout = MakePipelineLayoutWithTable({resultBGL}, 4);
        desc.vertex.module = module;
        desc.cFragment.module = module;
        desc.cFragment.entryPoint = "yes_resource_table";
        desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
        resourceTablePipeline = device.CreateRenderPipeline(&desc);
    }

    wgpu::RenderPipeline noResourceTablePipeline;
    {
        utils::ComboRenderPipelineDescriptor desc;
        desc.layout = utils::MakeBasicPipelineLayout(device, &resultBGL, 4);
        desc.vertex.module = module;
        desc.cFragment.module = module;
        desc.cFragment.entryPoint = "no_resource_table";
        desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
        noResourceTablePipeline = device.CreateRenderPipeline(&desc);
    }

    // Create the result buffer resource.
    wgpu::BufferDescriptor bDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = sizeof(uint32_t) * 3,
    };
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bDesc);
    wgpu::BindGroup resultBG = utils::MakeBindGroup(device, resultBGL, {{0, resultBuffer}});

    // Create and populate the resource table.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Uint,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    wgpu::ResourceTable table = MakeResourceTable(0);

    // Encode render commands that switch between the two pipelines. The resultBGL index in the
    // Vulkan backend will be pushed by 1 if the pipeline uses the resource table, so we check that
    // the invalidation of VkDescriptorSet inheritance works correctly.
    uint32_t resultIndex = 0;
    auto rp = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
    pass.SetResourceTable(table);
    pass.SetBindGroup(0, resultBG);

    // Start by not using the resource table.
    pass.SetPipeline(noResourceTablePipeline);
    pass.SetImmediates(0, &resultIndex, sizeof(resultIndex));
    pass.Draw(1);
    resultIndex++;

    pass.SetPipeline(resourceTablePipeline);
    pass.SetImmediates(0, &resultIndex, sizeof(resultIndex));
    pass.Draw(1);
    resultIndex++;

    // And back to not using it.
    pass.SetPipeline(noResourceTablePipeline);
    pass.SetImmediates(0, &resultIndex, sizeof(resultIndex));
    pass.Draw(1);
    resultIndex++;

    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(42, resultBuffer, 0);
    EXPECT_BUFFER_U32_EQ(10, resultBuffer, 4);
    EXPECT_BUFFER_U32_EQ(42, resultBuffer, 8);
}

DAWN_INSTANTIATE_TEST(ResourceTableTests, D3D12Backend(), MetalBackend(), VulkanBackend());

// Test that a resource used in a bindful way outside of a usage scope will get correctly barriered
// the next time it is used in a resource table.
enum class BindlessReadKind {
    Compute,
    Render,
};
std::ostream& operator<<(std::ostream& os, BindlessReadKind read) {
    switch (read) {
        case BindlessReadKind::Compute:
            return (os << "Compute");
        case BindlessReadKind::Render:
            return (os << "Render");
    }
}

enum class BindfulWriteKind {
    Render,
    Copy,
    RenderStorage,
    ComputeStorage,
};
std::ostream& operator<<(std::ostream& os, BindfulWriteKind write) {
    switch (write) {
        case BindfulWriteKind::Render:
            return (os << "Render");
        case BindfulWriteKind::Copy:
            return (os << "Copy");
        case BindfulWriteKind::RenderStorage:
            return (os << "RenderStorage");
        case BindfulWriteKind::ComputeStorage:
            return (os << "ComputeStorage");
    }
}

DAWN_TEST_PARAM_STRUCT(BindfulInteractionParams, BindlessReadKind, BindfulWriteKind);

class ResourceTableBindfulInteractionTests : public DawnTestWithParams<BindfulInteractionParams> {
  protected:
    void SetUp() override {
        DawnTestWithParams<BindfulInteractionParams>::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(
            !SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable}));

        // Swiftshader doesn't support variable count descriptor sets used in draw operations. In
        // vk::DescriptorSet::ParseDescriptors it iterates over all the descriptors to prep various
        // things but iterates over the whole size defined in the vkDescriptorSetLayout instead of
        // taking into account the variable count.
        DAWN_SUPPRESS_TEST_IF(IsSwiftshader());
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable})) {
            return {wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable};
        }
        return {};
    }

    void DoTest(BindlessReadKind read, BindfulWriteKind write) {
        switch (read) {
            case BindlessReadKind::Compute:
                DoComputeTest(write);
                break;
            case BindlessReadKind::Render:
                DoRenderTest(write);
                break;
        }
    }

  private:
    // Makes the texture that we'll use for the test and put it in a single-entry ResourceTable.
    std::pair<wgpu::Texture, wgpu::ResourceTable> CreateTestTextureAndTable(BindfulWriteKind write,
                                                                            uint32_t initialValue) {
        // Create and initialize the texture.
        wgpu::TextureDescriptor tDesc = {
            .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst |
                     TextureUsageForWriteKind(write),
            .size = {1, 1},
            .format = wgpu::TextureFormat::R32Uint,
        };
        wgpu::Texture texture = device.CreateTexture(&tDesc);

        wgpu::TexelCopyTextureInfo srcInfo = utils::CreateTexelCopyTextureInfo(texture);
        wgpu::TexelCopyBufferLayout dstInfo = {};
        wgpu::Extent3D copySize = {1, 1, 1};
        queue.WriteTexture(&srcInfo, &initialValue, sizeof(initialValue), &dstInfo, &copySize);

        // Create and initialize the resource table.
        wgpu::ResourceTableDescriptor tableDesc;
        tableDesc.size = 1;
        wgpu::ResourceTable table = device.CreateResourceTable(&tableDesc);

        wgpu::TextureViewDescriptor vDesc = {.usage = wgpu::TextureUsage::TextureBinding};
        wgpu::BindingResource resource = {.textureView = texture.CreateView(&vDesc)};
        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));

        return {std::move(texture), std::move(table)};
    }

    void DoComputeTest(BindfulWriteKind write) {
        auto [texture, table] = CreateTestTextureAndTable(write, 0xBEEFu);

        // Pipeline that writes the getResource<>(0)'s content in the result buffer.
        wgpu::ComputePipelineDescriptor pDesc;
        pDesc.compute.module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;

            @group(0) @binding(0) var<storage, read_write> result : u32;
            @compute @workgroup_size(1) fn main() {
                let tex = getResource<texture_2d<u32>>(0);
                result = textureLoad(tex, vec2(0), 0).x;
            }
        )");
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pDesc);

        // The result storage buffer and BindGroup.
        wgpu::BufferDescriptor bDesc = {
            .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
            .size = sizeof(uint32_t) * 2,
        };
        wgpu::Buffer beefBuffer = device.CreateBuffer(&bDesc);
        wgpu::Buffer cafeBuffer = device.CreateBuffer(&bDesc);

        // Sample the texture bindlessly, write to it bindfully, then sample bindlessly again.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        {
            wgpu::BindGroup resultBG =
                utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, beefBuffer}});

            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetBindGroup(0, resultBG);
            pass.SetResourceTable(table);
            pass.SetPipeline(pipeline);

            pass.DispatchWorkgroups(1);
            pass.End();
        }

        EncodeBindfulWrite(encoder, texture, write, 0xCAFEu);

        {
            wgpu::BindGroup resultBG =
                utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, cafeBuffer}});

            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetBindGroup(0, resultBG);
            pass.SetResourceTable(table);
            pass.SetPipeline(pipeline);

            pass.DispatchWorkgroups(1);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Check results
        EXPECT_BUFFER_U32_EQ(0xBEEFu, beefBuffer, 0);
        EXPECT_BUFFER_U32_EQ(0xCAFEu, cafeBuffer, 0);
    }

    void DoRenderTest(BindfulWriteKind write) {
        auto [texture, table] = CreateTestTextureAndTable(write, 0xBEEFu);

        // Pipeline that writes the getResource<>(0)'s content at an offset in a storage buffer.
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;

            @vertex fn vs() -> @builtin(position) vec4f {
                return vec4f(0, 0, 0.5, 0.5);
            }
            @fragment fn fs() -> @location(0) u32 {
                let tex = getResource<texture_2d<u32>>(0);
                return textureLoad(tex, vec2(0), 0).x;
            }
        )");

        utils::ComboRenderPipelineDescriptor pDesc;
        pDesc.vertex.module = module;
        pDesc.cFragment.module = module;
        pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
        pDesc.cTargets[0].format = wgpu::TextureFormat::R32Uint;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

        // Sample the texture bindlessly, write to it bindfully, then sample bindlessly again.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        wgpu::Texture beefTexture;
        {
            utils::BasicRenderPass rp =
                utils::CreateBasicRenderPass(device, 1, 1, wgpu::TextureFormat::R32Uint);
            beefTexture = rp.color;

            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetResourceTable(table);
            pass.SetPipeline(pipeline);
            pass.Draw(1);
            pass.End();
        }

        EncodeBindfulWrite(encoder, texture, write, 0xCAFEu);

        wgpu::Texture cafeTexture;
        {
            utils::BasicRenderPass rp =
                utils::CreateBasicRenderPass(device, 1, 1, wgpu::TextureFormat::R32Uint);
            cafeTexture = rp.color;

            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetResourceTable(table);
            pass.SetPipeline(pipeline);
            pass.Draw(1);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Check results
        EXPECT_PIXEL_U32_EQ(uint32_t{0xBEEF}, beefTexture, 0, 0);
        EXPECT_PIXEL_U32_EQ(uint32_t{0xCAFE}, cafeTexture, 0, 0);
    }

    wgpu::TextureUsage TextureUsageForWriteKind(BindfulWriteKind kind) {
        switch (kind) {
            case BindfulWriteKind::Render:
                return wgpu::TextureUsage::RenderAttachment;
            case BindfulWriteKind::Copy:
                return wgpu::TextureUsage::CopyDst;
            case BindfulWriteKind::ComputeStorage:
            case BindfulWriteKind::RenderStorage:
                return wgpu::TextureUsage::StorageBinding;
        }
    }

    void EncodeBindfulWrite(wgpu::CommandEncoder encoder,
                            wgpu::Texture texture,
                            BindfulWriteKind kind,
                            uint32_t value) {
        switch (kind) {
            case BindfulWriteKind::Render:
                EncodeBindfulRender(encoder, texture, value);
                break;
            case BindfulWriteKind::Copy:
                EncodeBindfulCopy(encoder, texture, value);
                break;
            case BindfulWriteKind::RenderStorage:
                EncodeBindfulRenderStorageWrite(encoder, texture, value);
                break;
            case BindfulWriteKind::ComputeStorage:
                EncodeBindfulComputeStorageWrite(encoder, texture, value);
                break;
        }
    }

    void EncodeBindfulRender(wgpu::CommandEncoder encoder, wgpu::Texture texture, uint32_t value) {
        wgpu::RenderPassColorAttachment attachment{
            .view = texture.CreateView(),
            .loadOp = wgpu::LoadOp::Clear,
            .storeOp = wgpu::StoreOp::Store,
            .clearValue = {.r = static_cast<double>(value)},
        };
        wgpu::RenderPassDescriptor passDesc{
            .colorAttachmentCount = 1,
            .colorAttachments = &attachment,
        };
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDesc);
        pass.End();
    }

    void EncodeBindfulCopy(wgpu::CommandEncoder encoder, wgpu::Texture texture, uint32_t value) {
        wgpu::Buffer srcBuffer =
            utils::CreateBufferFromData(device, wgpu::BufferUsage::CopySrc, {value});

        wgpu::TexelCopyTextureInfo dstInfo = {.texture = texture};
        wgpu::TexelCopyBufferInfo srcInfo = {.buffer = srcBuffer};
        wgpu::Extent3D copySize = {1, 1};
        encoder.CopyBufferToTexture(&srcInfo, &dstInfo, &copySize);
    }

    void EncodeBindfulRenderStorageWrite(wgpu::CommandEncoder encoder,
                                         wgpu::Texture texture,
                                         uint32_t value) {
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            @vertex fn vs() -> @builtin(position) vec4f {
                return vec4f(0, 0, 0.5, 0.5);
            }
            @group(0) @binding(0) var dst : texture_storage_2d<r32uint, write>;
            var<immediate> value : u32;
            @fragment fn fs() -> @location(0) vec4f {
                textureStore(dst, vec2i(0, 0), vec4u(value, 0, 0, 0));
                return vec4f();
            }
        )");

        utils::ComboRenderPipelineDescriptor pDesc;
        pDesc.vertex.module = module;
        pDesc.cFragment.module = module;
        pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

        wgpu::BindGroup bg = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                  {{0, texture.CreateView()}});

        auto rp = utils::CreateBasicRenderPass(device, 1, 1);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bg);
        pass.SetImmediates(0, &value, sizeof(value));
        pass.Draw(1);
        pass.End();
    }

    void EncodeBindfulComputeStorageWrite(wgpu::CommandEncoder encoder,
                                          wgpu::Texture texture,
                                          uint32_t value) {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var dst : texture_storage_2d<r32uint, write>;
            var<immediate> value : u32;
            @compute @workgroup_size(1) fn cs() {
                textureStore(dst, vec2i(0, 0), vec4u(value, 0, 0, 0));
            }
        )");
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

        wgpu::BindGroup bg = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                  {{0, texture.CreateView()}});

        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bg);
        pass.SetImmediates(0, &value, sizeof(value));
        pass.DispatchWorkgroups(1);
        pass.End();
    }
};

TEST_P(ResourceTableBindfulInteractionTests, Test) {
    DoTest(GetParam().mBindlessReadKind, GetParam().mBindfulWriteKind);
}

DAWN_INSTANTIATE_TEST_P(ResourceTableBindfulInteractionTests,
                        {D3D12Backend(), MetalBackend(), VulkanBackend()},
                        {BindlessReadKind::Render, BindlessReadKind::Compute},
                        {BindfulWriteKind::Render, BindfulWriteKind::Copy,
                         BindfulWriteKind::ComputeStorage, BindfulWriteKind::RenderStorage});

// TODO(479179409): Add tests for dynamic validation of filterability
//   - BGL has tex2d, shader has tex1d, swap in default tex1d
//   - BGL has sampler_comparison, shader has sampler, swap in sampler
//   - BGL has unfilterable texture and filtering sampler, swap sampler to non_filternig
//   - BGL has unfilterable texture, bind-less filtering sampler, swap texture
//   - bind-less unfilterable texture, BGL has filtering sampler, swap sampler

}  // anonymous namespace
}  // namespace dawn
