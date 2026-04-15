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

#include <utility>
#include <vector>

#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/ScopedIgnoreValidationErrors.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class ResourceTableValidationTest : public ValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable};
    }

    wgpu::ResourceTable MakeResourceTable(uint32_t size) {
        wgpu::ResourceTableDescriptor desc;
        desc.size = size;
        return device.CreateResourceTable(&desc);
    }

    wgpu::ResourceTable MakeErrorResourceTable(uint32_t size = 1) {
        wgpu::RenderPassMaxDrawCount maxDraw;
        maxDraw.maxDrawCount = 1000;
        wgpu::ResourceTableDescriptor desc{
            .nextInChain = &maxDraw,
            .size = size,
        };

        wgpu::ResourceTable table;
        ASSERT_DEVICE_ERROR(table = device.CreateResourceTable(&desc));
        return table;
    }

    wgpu::ComputePipeline MakeComputePipeline(bool defaulted, bool usesResourceTable) {
        auto shaderModuleUsesTable = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;
            @compute @workgroup_size(1) fn main() {
                _ = hasResource<texture_2d<f32, filterable>>(0);
            }
        )");
        auto shaderModuleNoTable = utils::CreateShaderModule(device, R"(
            @compute @workgroup_size(1) fn main() {}
        )");

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = usesResourceTable ? shaderModuleUsesTable : shaderModuleNoTable;

        if (defaulted) {
            csDesc.layout = nullptr;
            return device.CreateComputePipeline(&csDesc);
        }

        wgpu::PipelineLayoutResourceTable plResourceTable;
        plResourceTable.usesResourceTable = true;

        wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
        pipelineLayoutDescriptor.bindGroupLayoutCount = 0;
        if (usesResourceTable) {
            pipelineLayoutDescriptor.nextInChain = &plResourceTable;
        }

        csDesc.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);
        return device.CreateComputePipeline(&csDesc);
    }

    wgpu::RenderPipeline MakeRenderPipeline(bool defaulted, bool usesResourceTable) {
        auto shaderModuleUsesTable = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;
            @vertex fn vs() -> @builtin(position) vec4f {
                return vec4f();
            }
            @fragment fn fs() -> @location(0) vec4f {
                _ = hasResource<texture_2d<f32, filterable>>(0);
                return vec4f();
            }
        )");
        auto shaderModuleNoTable = utils::CreateShaderModule(device, R"(
            @vertex fn vs() -> @builtin(position) vec4f {
                return vec4f();
            }
            @fragment fn fs() -> @location(0) vec4f {
                return vec4f();
            }
        )");

        utils::ComboRenderPipelineDescriptor pDesc;
        pDesc.vertex.module = usesResourceTable ? shaderModuleUsesTable : shaderModuleNoTable;
        pDesc.cFragment.module = usesResourceTable ? shaderModuleUsesTable : shaderModuleNoTable;

        if (defaulted) {
            pDesc.layout = nullptr;
            return device.CreateRenderPipeline(&pDesc);
        }

        wgpu::PipelineLayoutResourceTable plResourceTable;
        plResourceTable.usesResourceTable = true;

        wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
        pipelineLayoutDescriptor.bindGroupLayoutCount = 0;
        if (usesResourceTable) {
            pipelineLayoutDescriptor.nextInChain = &plResourceTable;
        }

        pDesc.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);
        return device.CreateRenderPipeline(&pDesc);
    }

    enum class Mutator : uint8_t {
        Update,
        InsertBinding,
    };
    void TestMutator(Mutator mutator, const wgpu::BindingResource* resource, bool success) {
        wgpu::ResourceTable table = MakeResourceTable(1);

        switch (mutator) {
            case Mutator::Update: {
                wgpu::Status status;
                if (success) {
                    status = table.Update(0, resource);
                } else {
                    ASSERT_DEVICE_ERROR(status = table.Update(0, resource));
                }
                EXPECT_EQ(status, wgpu::Status::Success);
                break;
            }

            case Mutator::InsertBinding: {
                uint32_t slot = wgpu::kInvalidBinding;
                if (success) {
                    slot = table.InsertBinding(resource);
                } else {
                    ASSERT_DEVICE_ERROR(slot = table.InsertBinding(resource));
                }
                EXPECT_EQ(slot, 0u);
                break;
            }
        }
    }

    // Helper to make sure that the resource table is marked as used. Even if internally Dawn
    // doesn't track this, it makes tests more clearly correct.
    void UseResourceTableInSubmit(wgpu::ResourceTable table) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetResourceTable(table);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);
    }
};

class ResourceTableValidationTestDisabled : public ValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override { return {}; }
};

// Test that validates that the feature must be enabled
TEST_F(ResourceTableValidationTestDisabled, FeatureNotEnabled) {
    wgpu::ResourceTableDescriptor descriptor;
    descriptor.size = 1u;
    ASSERT_DEVICE_ERROR(device.CreateResourceTable(&descriptor));
}

// Test that setting invalid size returns nullptr
TEST_F(ResourceTableValidationTest, InvalidSize) {
    wgpu::ResourceTableDescriptor descriptor;

    // Size 0 is valid
    descriptor.size = 0u;
    device.CreateResourceTable(&descriptor);

    // Size of 1 is valid
    descriptor.size = 1u;
    device.CreateResourceTable(&descriptor);

    // Size of kMaxResourceTableSize is valid
    descriptor.size = kMaxResourceTableSize;
    device.CreateResourceTable(&descriptor);

    // Size > limits is invalid
    descriptor.size = kMaxResourceTableSize + 1u;
    wgpu::ResourceTable table = device.CreateResourceTable(&descriptor);
    ASSERT_EQ(table.Get(), nullptr);
}

// Test that setting nextInChain to anything is an error
TEST_F(ResourceTableValidationTest, NextInChain) {
    // Control case, nextInChain = nullptr is valid.
    {
        wgpu::ResourceTableDescriptor descriptor{
            .nextInChain = nullptr,
            .size = 3,
        };
        device.CreateResourceTable(&descriptor);
    }

    // Control case, nextInChain = non null is invalid.
    {
        wgpu::RenderPassMaxDrawCount maxDraw;
        maxDraw.maxDrawCount = 1000;
        wgpu::ResourceTableDescriptor descriptor{
            .nextInChain = &maxDraw,
            .size = 3,
        };
        ASSERT_DEVICE_ERROR(device.CreateResourceTable(&descriptor));
    }
}

// Test the Destroy call on a ResourceTable
TEST_F(ResourceTableValidationTest, Destroy) {
    wgpu::ResourceTableDescriptor descriptor;
    descriptor.size = 1u;
    wgpu::ResourceTable resourceTable = device.CreateResourceTable(&descriptor);

    // Calling destroy is valid
    resourceTable.Destroy();

    // Calling it multiple times is valid
    resourceTable.Destroy();
}

// Control case where enabling use of a resource table with the feature enabled is valid.
TEST_F(ResourceTableValidationTest, PipelineLayoutCreation_SuccessWithFeatureEnabled) {
    wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
    pipelineLayoutDescriptor.bindGroupLayoutCount = 0;
    wgpu::PipelineLayoutResourceTable resourceTable;
    resourceTable.usesResourceTable = true;
    pipelineLayoutDescriptor.nextInChain = &resourceTable;
    device.CreatePipelineLayout(&pipelineLayoutDescriptor);
}

// Error case where enabling use of a resource table with the feature disabled is an error.
TEST_F(ResourceTableValidationTestDisabled, PipelineLayoutCreation_FailureWithFeatureDisabled) {
    wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
    pipelineLayoutDescriptor.bindGroupLayoutCount = 0;
    wgpu::PipelineLayoutResourceTable resourceTable;
    pipelineLayoutDescriptor.nextInChain = &resourceTable;

    // Failure case
    resourceTable.usesResourceTable = true;
    ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&pipelineLayoutDescriptor));

    // Success case
    resourceTable.usesResourceTable = false;
    device.CreatePipelineLayout(&pipelineLayoutDescriptor);
}

// Error case where compiling a shader using the resource table with the extension disabled is an
// error.
TEST_F(ResourceTableValidationTestDisabled, WGSLEnableNotAllowed) {
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @compute @workgroup_size(1) fn main() {
            _ = hasResource<texture_2d<f32, filterable>>(0);
        }
    )"));
}

// Test that a shader using a resource table requires a layout with one.
TEST_F(ResourceTableValidationTest, PipelineCreation_ShaderRequiresLayoutWithResourceTable) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @compute @workgroup_size(1) fn main() {
            _ = hasResource<texture_2d<f32, filterable>>(0);
        }
    )");

    wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
    pipelineLayoutDescriptor.bindGroupLayoutCount = 0;
    wgpu::PipelineLayoutResourceTable resourceTable;
    pipelineLayoutDescriptor.nextInChain = &resourceTable;

    // Success case, the layout uses a resource table
    resourceTable.usesResourceTable = true;
    csDesc.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);
    device.CreateComputePipeline(&csDesc);

    // Failure case, the layout does not use a resource table
    resourceTable.usesResourceTable = false;
    csDesc.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);
    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));
}

// Test that it is valid to have a layout specifying a resource table with a shader that
// doesn't have one.
TEST_F(ResourceTableValidationTest, PipelineCreation_ShaderNoResourceTableWithLayoutThatHasOne) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1) fn main() {
        }
    )");

    wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
    pipelineLayoutDescriptor.bindGroupLayoutCount = 0;
    wgpu::PipelineLayoutResourceTable resourceTable;
    pipelineLayoutDescriptor.nextInChain = &resourceTable;

    resourceTable.usesResourceTable = true;
    csDesc.layout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);
    device.CreateComputePipeline(&csDesc);
}

// Test that an defaulted pipeline layout with a shader that uses a resource table has a
// PipelineLayoutResourceTable with usesResourceTable == true.
TEST_F(ResourceTableValidationTest, PipelineCreation_DefaultedLayoutWithResourceTable) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @compute @workgroup_size(1) fn main() {
            _ = hasResource<texture_2d<f32, filterable>>(0);
        }
    )");

    csDesc.layout = nullptr;  // Auto
    device.CreateComputePipeline(&csDesc);
}

// Test that an defaulted pipeline layout with a multi-stage shader where only one stage uses a
// resource table has a PipelineLayoutResourceTable with usesResourceTable == true.
TEST_F(ResourceTableValidationTest, PipelineCreation_OneShaderDefaultedLayoutWithResourceTable) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0.5, 0.5);
        }
        @compute @workgroup_size(1) fn compute_main() {
            _ = hasResource<texture_2d<f32, filterable>>(0);
        }
        @fragment fn fs() -> @location(0) vec4f {
            return vec4f(1.0, 0.0, 0.0, 1.0);
        }
    )");

    csDesc.layout = nullptr;  // Auto
    device.CreateComputePipeline(&csDesc);
}

// Test that a resource table uses up a BindGroupLayout slot
TEST_F(ResourceTableValidationTest, PipelineLayoutCreation_ResourceTableUsesBindGroupLayoutSlot) {
    // Control case: max bgls, no resource table
    {
        std::vector bgLayout(kMaxBindGroups, utils::MakeBindGroupLayout(device, {}));
        wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
        pipelineLayoutDescriptor.bindGroupLayoutCount = bgLayout.size();
        pipelineLayoutDescriptor.bindGroupLayouts = bgLayout.data();
        device.CreatePipelineLayout(&pipelineLayoutDescriptor);
    }

    // Failure case: not enough room for bgls and a resource table
    {
        std::vector bgLayout(kMaxBindGroups, utils::MakeBindGroupLayout(device, {}));
        wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
        pipelineLayoutDescriptor.bindGroupLayoutCount = bgLayout.size();
        pipelineLayoutDescriptor.bindGroupLayouts = bgLayout.data();
        wgpu::PipelineLayoutResourceTable resourceTable;
        resourceTable.usesResourceTable = true;
        pipelineLayoutDescriptor.nextInChain = &resourceTable;
        ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&pipelineLayoutDescriptor));
    }

    // Success case: enough room for bgls and a resource table
    {
        std::vector bgLayout(kMaxBindGroups - 1, utils::MakeBindGroupLayout(device, {}));
        wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
        pipelineLayoutDescriptor.bindGroupLayoutCount = bgLayout.size();
        pipelineLayoutDescriptor.bindGroupLayouts = bgLayout.data();
        wgpu::PipelineLayoutResourceTable resourceTable;
        resourceTable.usesResourceTable = true;
        pipelineLayoutDescriptor.nextInChain = &resourceTable;
        device.CreatePipelineLayout(&pipelineLayoutDescriptor);
    }
}

// Test that a resource table uses up a storage buffer binding
TEST_F(ResourceTableValidationTest, PipelineLayoutCreation_ResourceTableUsesOneStorageBuffer) {
    const uint32_t maxStorageBuffers = deviceLimits.maxStorageBuffersPerShaderStage;
    std::vector<wgpu::BindGroupLayoutEntry> storageBufferEntries(maxStorageBuffers);
    for (size_t i = 0; i < storageBufferEntries.size(); i++) {
        storageBufferEntries[i].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
        storageBufferEntries[i].visibility =
            wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Compute;
        storageBufferEntries[i].binding = i;
    }

    // Success case: exactly maxStorageBuffers are used (1 for the resource table, max - 1 for BGL
    // entries).
    {
        wgpu::BindGroupLayoutDescriptor bglDesc = {
            .entryCount = maxStorageBuffers - 1,
            .entries = storageBufferEntries.data(),
        };
        wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDesc);

        wgpu::PipelineLayoutResourceTable resourceTable;
        resourceTable.usesResourceTable = true;
        wgpu::PipelineLayoutDescriptor plDesc = {
            .nextInChain = &resourceTable,
            .bindGroupLayoutCount = 1,
            .bindGroupLayouts = &bgl,
        };
        device.CreatePipelineLayout(&plDesc);
    }

    // Error case: the resource table additional storage buffer make the layout go over the limit.
    {
        wgpu::BindGroupLayoutDescriptor bglDesc = {
            .entryCount = maxStorageBuffers,
            .entries = storageBufferEntries.data(),
        };
        wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDesc);

        wgpu::PipelineLayoutResourceTable resourceTable;
        resourceTable.usesResourceTable = true;
        wgpu::PipelineLayoutDescriptor plDesc = {
            .nextInChain = &resourceTable,
            .bindGroupLayoutCount = 1,
            .bindGroupLayouts = &bgl,
        };
        ASSERT_DEVICE_ERROR(device.CreatePipelineLayout(&plDesc));
    }
}

// Test that an defaulted pipeline layout with a resource table uses up a BindGroupLayout slot
TEST_F(ResourceTableValidationTest,
       PipelineCreation_DefaultedLayoutWithResourceTableUsesBindGroupLayoutSlot) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = nullptr;  // Auto

    // Control case: max bgls, no resource table
    {
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;
            @group(0) @binding(0) var<uniform> a : u32;
            @group(1) @binding(0) var<uniform> b : u32;
            @group(2) @binding(0) var<uniform> c : u32;
            @group(3) @binding(0) var<uniform> d : u32;
            @compute @workgroup_size(1) fn main() {
                // _ = hasResource<texture_2d<f32, filterable>>(0);
                _ = a;
                _ = b;
                _ = c;
                _ = d;
            }
        )");
        device.CreateComputePipeline(&csDesc);
    }

    // Failure case: not enough room for bgls and a resource table
    {
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;
            @group(0) @binding(0) var<uniform> a : u32;
            @group(1) @binding(0) var<uniform> b : u32;
            @group(2) @binding(0) var<uniform> c : u32;
            @group(3) @binding(0) var<uniform> d : u32;
            @compute @workgroup_size(1) fn main() {
                _ = hasResource<texture_2d<f32, filterable>>(0);
                _ = a;
                _ = b;
                _ = c;
                _ = d;
            }
        )");
        ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&csDesc));
    }

    // Success case: enough room for bgls and a resource table
    {
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;
            @group(0) @binding(0) var<uniform> a : u32;
            @group(1) @binding(0) var<uniform> b : u32;
            @group(2) @binding(0) var<uniform> c : u32;
            @compute @workgroup_size(1) fn main() {
                _ = hasResource<texture_2d<f32, filterable>>(0);
                _ = a;
                _ = b;
                _ = c;
            }
        )");
        device.CreateComputePipeline(&csDesc);
    }
}

// Test that GetBindGroupLayout is valid for one less BGL if resource tables are used.
TEST_F(ResourceTableValidationTest, GetBindGroupLayoutValidForOneLessIndex) {
    // Default behavior case: GetBGL is valid until kMaxBindGroups - 1 when no resource table is
    // used.
    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.layout = nullptr;
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;
            @compute @workgroup_size(1) fn main() {
            }
        )");
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

        pipeline.GetBindGroupLayout(kMaxBindGroups - 1);
        ASSERT_DEVICE_ERROR(pipeline.GetBindGroupLayout(kMaxBindGroups));
    }

    // Resource table case: GetBGL is valid until kMaxBindGroups - 2.
    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.layout = nullptr;
        csDesc.compute.module = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_resource_table;
            @compute @workgroup_size(1) fn main() {
                _ = hasResource<texture_2d<f32, filterable>>(0);
            }
        )");
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

        pipeline.GetBindGroupLayout(kMaxBindGroups - 2);
        ASSERT_DEVICE_ERROR(pipeline.GetBindGroupLayout(kMaxBindGroups - 1));
    }
}

// Tests calling ComputePassEncoder::SetResourceTable
TEST_F(ResourceTableValidationTest, ComputePassEncoder_SetResourceTable) {
    // Failure case: invalid encoder state
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.End();
        pass.SetResourceTable(nullptr);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Failure case: invalid resource table
    {
        ASSERT_DEVICE_ERROR(wgpu::ResourceTable resourceTable = MakeErrorResourceTable());

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetResourceTable(resourceTable);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Success case: valid resource table
    {
        wgpu::ResourceTable resourceTable = MakeResourceTable(1);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetResourceTable(resourceTable);
        pass.End();
        encoder.Finish();
    }

    // Success case: null resource table
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetResourceTable(nullptr);
        pass.End();
        encoder.Finish();
    }
}

// Tests calling RenderPassEncoder::SetResourceTable
TEST_F(ResourceTableValidationTest, RenderPassEncoder_SetResourceTable) {
    auto rp = utils::CreateBasicRenderPass(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);

    // Failure case: invalid encoder state
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.End();
        pass.SetResourceTable(nullptr);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Failure case: invalid resource table
    {
        ASSERT_DEVICE_ERROR(wgpu::ResourceTable resourceTable = MakeErrorResourceTable());

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.SetResourceTable(resourceTable);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Success case: valid resource table
    {
        wgpu::ResourceTable resourceTable = MakeResourceTable(1);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.SetResourceTable(resourceTable);
        pass.End();
        encoder.Finish();
    }

    // Success case: null resource table
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.SetResourceTable(nullptr);
        pass.End();
        encoder.Finish();
    }
}

// Tests calling RenderBundleEncoder::SetResourceTable
TEST_F(ResourceTableValidationTest, RenderBundleEncoder_SetResourceTable) {
    auto rp = utils::CreateBasicRenderPass(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = rp.colorFormat;

    // Failure case: invalid encoder state
    {
        wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
        wgpu::RenderBundle renderBundle = rbe.Finish();
        ASSERT_DEVICE_ERROR(rbe.SetResourceTable(nullptr));
    }

    // Failure case: invalid resource table
    {
        ASSERT_DEVICE_ERROR(wgpu::ResourceTable resourceTable = MakeErrorResourceTable());

        wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
        rbe.SetResourceTable(resourceTable);
        ASSERT_DEVICE_ERROR(wgpu::RenderBundle renderBundle = rbe.Finish());
    }

    // Success case: valid resource table
    {
        wgpu::ResourceTable resourceTable = MakeResourceTable(1);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
        rbe.SetResourceTable(resourceTable);
        wgpu::RenderBundle renderBundle = rbe.Finish();
        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        encoder.Finish();
    }

    // Success case: null resource table
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
        rbe.SetResourceTable(nullptr);
        wgpu::RenderBundle renderBundle = rbe.Finish();
        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        encoder.Finish();
    }
}

// Tests calling ComputePassEncoder::SetResourceTable when the feature is disabled
TEST_F(ResourceTableValidationTestDisabled, ComputePassEncoder_SetResourceTable) {
    // Failure case: feature is disabled
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetResourceTable(nullptr);
    pass.End();
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Tests calling RenderPassEncoder::SetResourceTable when the feature is disabled
TEST_F(ResourceTableValidationTestDisabled, RenderPassEncoder_SetResourceTable) {
    // Failure case: feature is disabled
    auto rp = utils::CreateBasicRenderPass(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
    pass.SetResourceTable(nullptr);
    pass.End();
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Tests calling RenderBundleEncoder::SetResourceTable when the feature is disabled
TEST_F(ResourceTableValidationTestDisabled, RenderBundleEncoder_SetResourceTable) {
    // Failure case: feature is disabled
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
    wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
    rbe.SetResourceTable(nullptr);
    ASSERT_DEVICE_ERROR(rbe.Finish());
}

// Tests that resource tables set on a compute pass can be used in submit
TEST_F(ResourceTableValidationTest, ComputePassEncoder_CanUseInSubmit) {
    wgpu::ResourceTable resourceTable = MakeResourceTable(1);
    wgpu::ResourceTable resourceTable2 = MakeResourceTable(1);
    wgpu::ResourceTable resourceTable3 = MakeResourceTable(1);

    // Success case: resource table can be used in submit
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetResourceTable(resourceTable);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);
    }

    // Success case: resource table can be used in multiple passes for submit
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetResourceTable(resourceTable);
        pass.End();
        wgpu::ComputePassEncoder pass2 = encoder.BeginComputePass();
        pass2.SetResourceTable(resourceTable);
        pass2.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);
    }

    // Failure case: resource table has been destroyed
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetResourceTable(resourceTable);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        resourceTable.Destroy();  // Destroy it
        ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
    }

    // Failure case: one of multiple resource tables has been destroyed
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetResourceTable(resourceTable);
        pass.SetResourceTable(resourceTable2);
        pass.SetResourceTable(resourceTable3);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        resourceTable2.Destroy();  // Destroy one
        ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
    }

    // Failure case: one of multiple resource tables in another pass has been destroyed
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetResourceTable(resourceTable);
        pass.SetResourceTable(resourceTable2);
        pass.End();
        wgpu::ComputePassEncoder pass2 = encoder.BeginComputePass();
        pass2.SetResourceTable(resourceTable);
        pass2.SetResourceTable(resourceTable3);
        pass2.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        resourceTable3.Destroy();  // Destroy one
        ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
    }

    // Failure case: resource table must still be valid if set, then nullptr is set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetResourceTable(resourceTable);
        pass.SetResourceTable(nullptr);  // Clear it
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        resourceTable.Destroy();  // Destroy it
        ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
    }
}

// Tests that resource tables set on a render pass can be used in submit
TEST_F(ResourceTableValidationTest, RenderPassEncoder_CanUseInSubmit) {
    wgpu::ResourceTable resourceTable = MakeResourceTable(1);
    wgpu::ResourceTable resourceTable2 = MakeResourceTable(1);
    wgpu::ResourceTable resourceTable3 = MakeResourceTable(1);
    auto rp = utils::CreateBasicRenderPass(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);

    // Success case: resource table can be used in submit
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.SetResourceTable(resourceTable);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);
    }

    // Success case: resource table can be used in multiple passes for submit
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.SetResourceTable(resourceTable);
        pass.End();
        wgpu::RenderPassEncoder pass2 = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass2.SetResourceTable(resourceTable);
        pass2.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);
    }

    // Failure case: resource table has been destroyed
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.SetResourceTable(resourceTable);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        resourceTable.Destroy();  // Destroy it
        ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
    }

    // Failure case: one of multiple resource tables has been destroyed
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.SetResourceTable(resourceTable);
        pass.SetResourceTable(resourceTable2);
        pass.SetResourceTable(resourceTable3);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        resourceTable2.Destroy();  // Destroy one
        ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
    }

    // Failure case: one of multiple resource tables in another pass has been destroyed
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.SetResourceTable(resourceTable);
        pass.SetResourceTable(resourceTable2);
        pass.End();
        wgpu::RenderPassEncoder pass2 = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass2.SetResourceTable(resourceTable);
        pass2.SetResourceTable(resourceTable3);
        pass2.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        resourceTable3.Destroy();  // Destroy one
        ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
    }

    // Failure case: resource table must still be valid if set, then nullptr is set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.SetResourceTable(resourceTable);
        pass.SetResourceTable(nullptr);  // Clear it
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        resourceTable.Destroy();  // Destroy it
        ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
    }
}

// Tests that resource tables set on a render bundle can be used in submit
TEST_F(ResourceTableValidationTest, RenderBundleEncoder_CanUseInSubmit) {
    wgpu::ResourceTable resourceTable = MakeResourceTable(1);
    wgpu::ResourceTable resourceTable2 = MakeResourceTable(1);
    wgpu::ResourceTable resourceTable3 = MakeResourceTable(1);
    auto rp = utils::CreateBasicRenderPass(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = rp.colorFormat;

    // Success case: resource table can be used in submit
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
        rbe.SetResourceTable(resourceTable);
        wgpu::RenderBundle renderBundle = rbe.Finish();
        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);
    }

    // Success case: resource table can be used in multiple passes for submit
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
        rbe.SetResourceTable(resourceTable);
        wgpu::RenderBundle renderBundle = rbe.Finish();
        pass.ExecuteBundles(1, &renderBundle);
        pass.End();

        wgpu::RenderPassEncoder pass2 = encoder.BeginRenderPass(&rp.renderPassInfo);
        wgpu::RenderBundleEncoder rbe2 = device.CreateRenderBundleEncoder(&desc);
        rbe2.SetResourceTable(resourceTable);
        wgpu::RenderBundle renderBundle2 = rbe2.Finish();
        pass2.ExecuteBundles(1, &renderBundle2);
        pass2.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);
    }

    // Success case: resource table can be used in multiple rbes in one pass for submit
    {
        std::vector<wgpu::RenderBundle> renderBundles;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);

        wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
        rbe.SetResourceTable(resourceTable);
        renderBundles.push_back(rbe.Finish());

        wgpu::RenderBundleEncoder rbe2 = device.CreateRenderBundleEncoder(&desc);
        rbe2.SetResourceTable(resourceTable);
        renderBundles.push_back(rbe2.Finish());

        pass.ExecuteBundles(renderBundles.size(), renderBundles.data());
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        device.GetQueue().Submit(1, &commands);
    }

    // Failure case: resource table has been destroyed
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
        rbe.SetResourceTable(resourceTable);
        wgpu::RenderBundle renderBundle = rbe.Finish();
        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        resourceTable.Destroy();  // Destroy it
        ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
    }

    // Failure case: one of multiple resource tables has been destroyed
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
        rbe.SetResourceTable(resourceTable);
        rbe.SetResourceTable(resourceTable2);
        rbe.SetResourceTable(resourceTable3);
        wgpu::RenderBundle renderBundle = rbe.Finish();
        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        resourceTable2.Destroy();  // Destroy one
        ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
    }

    // Failure case: one of multiple resource tables in another pass has been destroyed
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
        rbe.SetResourceTable(resourceTable);
        rbe.SetResourceTable(resourceTable2);
        wgpu::RenderBundle renderBundle = rbe.Finish();
        pass.ExecuteBundles(1, &renderBundle);
        pass.End();

        wgpu::RenderPassEncoder pass2 = encoder.BeginRenderPass(&rp.renderPassInfo);
        wgpu::RenderBundleEncoder rbe2 = device.CreateRenderBundleEncoder(&desc);
        rbe2.SetResourceTable(resourceTable);
        rbe2.SetResourceTable(resourceTable3);
        wgpu::RenderBundle renderBundle2 = rbe2.Finish();
        pass2.ExecuteBundles(1, &renderBundle2);
        pass2.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        resourceTable3.Destroy();  // Destroy one
        ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
    }

    // Failure case: resource table must still be valid if set, then nullptr is set
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
        rbe.SetResourceTable(resourceTable);
        rbe.SetResourceTable(nullptr);  // Clear it
        wgpu::RenderBundle renderBundle = rbe.Finish();
        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        resourceTable.Destroy();  // Destroy it
        ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
    }
}

// Tests that the resource table can be used in dispatch
TEST_F(ResourceTableValidationTest, Submit_DispatchRequiresResourceTable) {
    for (bool defaulted : {true, false}) {
        wgpu::ComputePipeline pipelineUsesTable = MakeComputePipeline(defaulted, true);
        wgpu::ComputePipeline pipelineNoTable = MakeComputePipeline(defaulted, false);
        wgpu::ResourceTable resourceTable = MakeResourceTable(1);
        wgpu::ResourceTable resourceTable2 = MakeResourceTable(1);

        // Success case: `usesResourceTable` is enabled, and one has been set on the pass
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetResourceTable(resourceTable);
            pass.SetPipeline(pipelineUsesTable);
            pass.DispatchWorkgroups(1);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();
            device.GetQueue().Submit(1, &commands);
        }

        // Failure case: `usesResourceTable` is enabled, but none has been set on the pass
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipelineUsesTable);
            pass.DispatchWorkgroups(1);
            pass.End();
            ASSERT_DEVICE_ERROR(wgpu::CommandBuffer commands = encoder.Finish());
        }

        // Failure case: `usesResourceTable` is enabled, one then nullptr set on the pass
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetResourceTable(resourceTable);  // Set a valid one
            pass.SetResourceTable(nullptr);        // Then clear it
            pass.SetPipeline(pipelineUsesTable);
            pass.DispatchWorkgroups(1);
            pass.End();
            ASSERT_DEVICE_ERROR(wgpu::CommandBuffer commands = encoder.Finish());
        }

        // Success case: `usesResourceTable` is enabled, one then nullptr then another set on the
        // pass
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetResourceTable(resourceTable);   // Set a valid one
            pass.SetResourceTable(nullptr);         // Then clear it
            pass.SetResourceTable(resourceTable2);  // Then set another valid one
            pass.SetPipeline(pipelineUsesTable);
            pass.DispatchWorkgroups(1);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();
            device.GetQueue().Submit(1, &commands);
        }

        // Success case: `usesResourceTable` enabled only on pass 2 with table set
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipelineNoTable);
            pass.DispatchWorkgroups(1);
            pass.End();
            wgpu::ComputePassEncoder pass2 = encoder.BeginComputePass();
            pass2.SetResourceTable(resourceTable);
            pass2.SetPipeline(pipelineUsesTable);
            pass2.DispatchWorkgroups(1);
            pass2.End();
            wgpu::CommandBuffer commands = encoder.Finish();
        }

        // Failure case: `usesResourceTable` enabled only on pass 2 but no table set
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipelineNoTable);
            pass.DispatchWorkgroups(1);
            pass.End();
            wgpu::ComputePassEncoder pass2 = encoder.BeginComputePass();
            pass2.SetPipeline(pipelineUsesTable);
            pass2.DispatchWorkgroups(1);
            pass2.End();
            ASSERT_DEVICE_ERROR(wgpu::CommandBuffer commands = encoder.Finish());
        }
    }
}

// Tests that the resource table can be used in draw
TEST_F(ResourceTableValidationTest, Submit_DrawRequiresResourceTable) {
    for (bool defaulted : {true, false}) {
        wgpu::RenderPipeline pipelineUsesTable = MakeRenderPipeline(defaulted, true);
        wgpu::RenderPipeline pipelineNoTable = MakeRenderPipeline(defaulted, false);
        wgpu::ResourceTable resourceTable = MakeResourceTable(1);
        wgpu::ResourceTable resourceTable2 = MakeResourceTable(1);
        auto rp = utils::CreateBasicRenderPass(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);

        // Success case: `usesResourceTable` is enabled, and one has been set on the pass
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetResourceTable(resourceTable);
            pass.SetPipeline(pipelineUsesTable);
            pass.Draw(1);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();
            device.GetQueue().Submit(1, &commands);
        }

        // Failure case: `usesResourceTable` is enabled, but none has been set on the pass
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetPipeline(pipelineUsesTable);
            pass.Draw(1);
            pass.End();
            ASSERT_DEVICE_ERROR(wgpu::CommandBuffer commands = encoder.Finish());
        }

        // Failure case: `usesResourceTable` is enabled, one then nullptr set on the pass
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetResourceTable(resourceTable);  // Set a valid one
            pass.SetResourceTable(nullptr);        // Then clear it
            pass.SetPipeline(pipelineUsesTable);
            pass.Draw(1);
            pass.End();
            ASSERT_DEVICE_ERROR(wgpu::CommandBuffer commands = encoder.Finish());
        }

        // Success case: `usesResourceTable` is enabled, one then nullptr then another set on the
        // pass
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetResourceTable(resourceTable);
            pass.SetResourceTable(nullptr);         // Then clear it
            pass.SetResourceTable(resourceTable2);  // Then set another valid one
            pass.SetPipeline(pipelineUsesTable);
            pass.Draw(1);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();
            device.GetQueue().Submit(1, &commands);
        }

        // Success case: single pass toggles between pipelines that do not use and use a table
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetPipeline(pipelineNoTable);
            pass.Draw(1);
            pass.SetPipeline(pipelineUsesTable);
            pass.SetResourceTable(resourceTable);
            pass.Draw(1);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();
            device.GetQueue().Submit(1, &commands);
        }

        // Failure case: single pass toggles between pipelines that do not use and use a table, but
        // does not set a table when required
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetPipeline(pipelineNoTable);
            pass.Draw(1);
            pass.SetPipeline(pipelineUsesTable);
            pass.Draw(1);
            pass.End();
            ASSERT_DEVICE_ERROR(wgpu::CommandBuffer commands = encoder.Finish());
        }

        // Success case: `usesResourceTable` enabled only on pass 2 with table set
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetPipeline(pipelineNoTable);
            pass.Draw(1);
            pass.End();
            wgpu::RenderPassEncoder pass2 = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass2.SetResourceTable(resourceTable);
            pass2.SetPipeline(pipelineUsesTable);
            pass2.Draw(1);
            pass2.End();
            wgpu::CommandBuffer commands = encoder.Finish();
            device.GetQueue().Submit(1, &commands);
        }

        // Failure case: `usesResourceTable` enabled only on pass 2 but no table set
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetPipeline(pipelineNoTable);
            pass.Draw(1);
            pass.End();
            wgpu::RenderPassEncoder pass2 = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass2.SetPipeline(pipelineUsesTable);
            pass2.Draw(1);
            pass2.End();
            ASSERT_DEVICE_ERROR(wgpu::CommandBuffer commands = encoder.Finish());
        }
    }
}

// Tests that the resource table in RenderBundle can be used in draw
TEST_F(ResourceTableValidationTest, Submit_RenderBundleDrawRequiresResourceTable) {
    for (bool defaulted : {true, false}) {
        wgpu::RenderPipeline pipelineUsesTable = MakeRenderPipeline(defaulted, true);
        wgpu::RenderPipeline pipelineNoTable = MakeRenderPipeline(defaulted, false);
        wgpu::ResourceTable resourceTable = MakeResourceTable(1);
        wgpu::ResourceTable resourceTable2 = MakeResourceTable(1);

        auto rp = utils::CreateBasicRenderPass(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);

        utils::ComboRenderBundleEncoderDescriptor desc = {};
        desc.colorFormatCount = 1;
        desc.cColorFormats[0] = rp.colorFormat;

        // Success case: `usesResourceTable` is enabled, and one has been set on the encoder
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
            rbe.SetResourceTable(resourceTable);
            rbe.SetPipeline(pipelineUsesTable);
            rbe.Draw(1);
            wgpu::RenderBundle renderBundle = rbe.Finish();
            pass.ExecuteBundles(1, &renderBundle);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();
            device.GetQueue().Submit(1, &commands);
        }

        // Failure case: `usesResourceTable` is enabled, but none has been set on the encoder
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
            rbe.SetPipeline(pipelineUsesTable);
            rbe.Draw(1);
            ASSERT_DEVICE_ERROR(wgpu::RenderBundle renderBundle = rbe.Finish());
        }

        // Failure case: `usesResourceTable` is enabled, one then nullptr set on the encoder
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
            rbe.SetResourceTable(resourceTable);  // Set a valid one
            rbe.SetResourceTable(nullptr);        // Then clear it
            rbe.SetPipeline(pipelineUsesTable);
            rbe.Draw(1);
            ASSERT_DEVICE_ERROR(wgpu::RenderBundle renderBundle = rbe.Finish());
        }

        // Success case: `usesResourceTable` is enabled, one then nullptr then another set on the
        // encoder
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
            rbe.SetResourceTable(resourceTable);   // Set a valid one
            rbe.SetResourceTable(nullptr);         // Then clear it
            rbe.SetResourceTable(resourceTable2);  // Then set another valid one
            rbe.SetPipeline(pipelineUsesTable);
            rbe.Draw(1);
            wgpu::RenderBundle renderBundle = rbe.Finish();
            pass.ExecuteBundles(1, &renderBundle);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();
            device.GetQueue().Submit(1, &commands);
        }

        // Success case: single rbe toggles between pipelines that do not use and use a table
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
            rbe.SetPipeline(pipelineUsesTable);
            rbe.SetResourceTable(resourceTable);
            rbe.Draw(1);
            rbe.SetPipeline(pipelineNoTable);
            rbe.Draw(1);
            wgpu::RenderBundle renderBundle = rbe.Finish();
            pass.ExecuteBundles(1, &renderBundle);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();
            device.GetQueue().Submit(1, &commands);
        }

        // Failure case: single rbe toggles between pipelines that do not use and use a table, but
        // does not set a table when required
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
            rbe.SetPipeline(pipelineUsesTable);
            rbe.Draw(1);
            rbe.SetPipeline(pipelineNoTable);
            rbe.Draw(1);
            ASSERT_DEVICE_ERROR(wgpu::RenderBundle renderBundle = rbe.Finish());
        }

        // Success case: mix pass and rbe
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetPipeline(pipelineUsesTable);
            pass.SetResourceTable(resourceTable);
            pass.SetPipeline(pipelineNoTable);
            pass.Draw(1);

            wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
            rbe.SetPipeline(pipelineUsesTable);
            rbe.SetResourceTable(resourceTable);
            rbe.Draw(1);
            rbe.SetPipeline(pipelineNoTable);
            rbe.Draw(1);
            wgpu::RenderBundle renderBundle = rbe.Finish();
            pass.ExecuteBundles(1, &renderBundle);

            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();
            device.GetQueue().Submit(1, &commands);
        }
    }
}

// Test that render bundles do not persist resource tables onto a render pass
TEST_F(ResourceTableValidationTest, RenderBundleDoesNotPersistResourceTable) {
    auto rp = utils::CreateBasicRenderPass(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = rp.colorFormat;

    // Success case: pipeline uses table, one is set on the bundle, and one on the pass
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);

        // Create render bundle with a resource table
        wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
        wgpu::ResourceTable resourceTable = MakeResourceTable(1);
        rbe.SetResourceTable(resourceTable);
        wgpu::RenderBundle renderBundle = rbe.Finish();

        auto pipelineUsesTable = MakeRenderPipeline(true, true);
        pass.ExecuteBundles(1, &renderBundle);
        pass.SetPipeline(pipelineUsesTable);
        pass.SetResourceTable(resourceTable);  // Set table on the pass
        pass.Draw(1);
        pass.End();

        encoder.Finish();
    }

    // Failure case: pipeline uses table, one is set on the bundle, but not on the pass
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);

        // Create render bundle with a resource table
        wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
        wgpu::ResourceTable resourceTable = MakeResourceTable(1);
        rbe.SetResourceTable(resourceTable);
        wgpu::RenderBundle renderBundle = rbe.Finish();

        auto pipelineUsesTable = MakeRenderPipeline(true, true);
        pass.ExecuteBundles(1, &renderBundle);
        pass.SetPipeline(pipelineUsesTable);
        pass.Draw(1);
        pass.End();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that render bundles do not inherit resource tables from a render pass
TEST_F(ResourceTableValidationTest, RenderBundleDoesNotInheritResourceTable) {
    auto rp = utils::CreateBasicRenderPass(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = rp.colorFormat;

    // Success case: pipeline uses table, one is set on the bundle, and one on the pass
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        wgpu::ResourceTable resourceTable = MakeResourceTable(1);

        // Create render bundle with a resource table
        wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
        rbe.SetResourceTable(resourceTable);  // Set table on the bundle
        wgpu::RenderBundle renderBundle = rbe.Finish();

        auto pipelineUsesTable = MakeRenderPipeline(true, true);
        pass.ExecuteBundles(1, &renderBundle);
        pass.SetPipeline(pipelineUsesTable);
        pass.SetResourceTable(resourceTable);  // Set table on the pass
        pass.Draw(1);
        pass.End();

        encoder.Finish();
    }

    // Failure case: pipeline uses table, one is set on the pass, but not on the bundle
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        wgpu::ResourceTable resourceTable = MakeResourceTable(1);

        // Create render bundle without a resource table
        wgpu::RenderBundleEncoder rbe = device.CreateRenderBundleEncoder(&desc);
        wgpu::RenderBundle renderBundle = rbe.Finish();

        auto pipelineUsesTable = MakeRenderPipeline(true, true);
        pass.SetPipeline(pipelineUsesTable);
        pass.SetResourceTable(resourceTable);  // Set table on the pass
        pass.ExecuteBundles(1, &renderBundle);
        pass.Draw(1);
        pass.End();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that pinning / unpinning is valid for a simple case. This is a control for the test that
// errors are produced when the feature is not enabled.
TEST_F(ResourceTableValidationTest, PinUnpinTextureSuccess) {
    wgpu::TextureDescriptor desc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&desc);

    tex.Pin(wgpu::TextureUsage::TextureBinding);
    tex.Unpin();
}

// Test that calling pin/unpin is an error when the feature is not enabled.
TEST_F(ResourceTableValidationTestDisabled, PinUnpinTextureSuccess) {
    wgpu::TextureDescriptor desc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&desc);

    ASSERT_DEVICE_ERROR(tex.Pin(wgpu::TextureUsage::TextureBinding));
    ASSERT_DEVICE_ERROR(tex.Unpin());
}

// Test the validation of the usage parameter of Pin.
TEST_F(ResourceTableValidationTest, PinUnpinTextureUsageConstraint) {
    wgpu::TextureDescriptor desc{
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };

    desc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc |
                 wgpu::TextureUsage::StorageBinding;
    wgpu::Texture testTexture = device.CreateTexture(&desc);

    desc.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture renderOnlyTexture = device.CreateTexture(&desc);

    // Control case, pinning the sampled texture to TextureBinding is valid.
    testTexture.Pin(wgpu::TextureUsage::TextureBinding);

    // Error case, pinning to a usage not in the texture is invalid.
    ASSERT_DEVICE_ERROR(renderOnlyTexture.Pin(wgpu::TextureUsage::TextureBinding));

    // Error case, pinning to an invalid usage is invalid.
    ASSERT_DEVICE_ERROR(testTexture.Pin(static_cast<wgpu::TextureUsage>(0x8000'0000)));

    // Error case, pinning must be to a shader usage.
    ASSERT_DEVICE_ERROR(testTexture.Pin(wgpu::TextureUsage::CopySrc));

    // Error case, pinning must be to a shader usage.
    // TODO(https://issues.chromium.org/473459218): Lift this constraint and allow other shader
    // usages.
    ASSERT_DEVICE_ERROR(testTexture.Pin(wgpu::TextureUsage::StorageBinding));
}

// Test that pinning / unpinning don't need to be balanced.
TEST_F(ResourceTableValidationTest, PinUnpinUnbalancedIsValid) {
    wgpu::TextureDescriptor desc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&desc);

    // Pinning right after creation is valid.
    tex.Unpin();

    // Pinning twice is valid.
    tex.Pin(wgpu::TextureUsage::TextureBinding);
    // TODO(https://issues.chromium.org/473459218): Use a different usage here when another is
    // valid.
    tex.Pin(wgpu::TextureUsage::TextureBinding);

    // Unpinning twice (plus one more to make sure we are unbalanced) is valid.
    tex.Unpin();
    tex.Unpin();
    tex.Unpin();
}

// Test that pinning is not allowed on a destroyed texture.
TEST_F(ResourceTableValidationTest, PinDestroyedTextureInvalid) {
    wgpu::TextureDescriptor desc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&desc);

    // Success case, pinning before Destroy() is valid.
    tex.Pin(wgpu::TextureUsage::TextureBinding);
    tex.Unpin();

    // Error case, pinning a destroyed texture is not allowed.
    tex.Destroy();
    ASSERT_DEVICE_ERROR(tex.Pin(wgpu::TextureUsage::TextureBinding));
}

enum class TestPinState { Default, Pinned, Unpinned };
std::array<TestPinState, 3> kAllTestPinStates = {TestPinState::Default, TestPinState::Pinned,
                                                 TestPinState::Unpinned};
wgpu::Texture CreateTextureWithPinState(const wgpu::Device& device,
                                        TestPinState pin,
                                        wgpu::TextureUsage usage) {
    wgpu::TextureDescriptor desc{
        .usage = usage,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture tex = device.CreateTexture(&desc);

    switch (pin) {
        case TestPinState::Default:
            break;
        case TestPinState::Pinned:
            tex.Pin(wgpu::TextureUsage::TextureBinding);
            break;
        case TestPinState::Unpinned:
            tex.Pin(wgpu::TextureUsage::TextureBinding);
            tex.Unpin();
            break;
    }

    return tex;
}

// Test that pinning prevents usage in WriteTexture
TEST_F(ResourceTableValidationTest, PinValidationUsageWriteTexture) {
    for (auto pin : kAllTestPinStates) {
        wgpu::Texture tex = CreateTextureWithPinState(
            device, pin, wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst);

        wgpu::TexelCopyTextureInfo dst = {
            .texture = tex,
        };
        wgpu::TexelCopyBufferLayout dataLayout = {};
        wgpu::Extent3D copySize = {0, 0, 0};

        if (pin == TestPinState::Pinned) {
            ASSERT_DEVICE_ERROR(
                device.GetQueue().WriteTexture(&dst, nullptr, 0, &dataLayout, &copySize));
        } else {
            device.GetQueue().WriteTexture(&dst, nullptr, 0, &dataLayout, &copySize);
        }
    }
}

// Test that pinning prevents usage in an encoder copy command
TEST_F(ResourceTableValidationTest, PinValidationUsageEncoderCopy) {
    wgpu::TextureDescriptor desc{
        .usage = wgpu::TextureUsage::CopyDst,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Float,
    };
    wgpu::Texture texDst = device.CreateTexture(&desc);

    for (auto pin : kAllTestPinStates) {
        wgpu::Texture tex = CreateTextureWithPinState(
            device, pin, wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc);

        wgpu::TexelCopyTextureInfo src = {
            .texture = tex,
        };
        wgpu::TexelCopyTextureInfo dst = {
            .texture = texDst,
        };
        wgpu::Extent3D copySize = {0, 0, 0};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&src, &dst, &copySize);
        wgpu::CommandBuffer commands = encoder.Finish();

        if (pin == TestPinState::Pinned) {
            ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
        } else {
            device.GetQueue().Submit(1, &commands);
        }
    }
}

// Test that pinning prevents usage in a dispatch if it is not the pinned usage.
TEST_F(ResourceTableValidationTest, PinValidationUsageDispatch) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var t_sampled : texture_2d<f32>;
        @compute @workgroup_size(1) fn sample() {
            _ = t_sampled;
        }

        @group(0) @binding(0) var t_ro_storage : texture_storage_2d<r32float, read>;
        @compute @workgroup_size(1) fn ro_storage() {
            _ = t_ro_storage;
        }
    )");

    csDesc.compute.entryPoint = "sample";
    wgpu::ComputePipeline samplePipeline = device.CreateComputePipeline(&csDesc);
    csDesc.compute.entryPoint = "ro_storage";
    wgpu::ComputePipeline storagePipeline = device.CreateComputePipeline(&csDesc);

    for (auto pin : kAllTestPinStates) {
        wgpu::Texture tex = CreateTextureWithPinState(
            device, pin, wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding);

        for (bool sample : {false, true}) {
            wgpu::ComputePipeline pipeline = sample ? samplePipeline : storagePipeline;
            wgpu::BindGroup bg = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                      {
                                                          {0, tex.CreateView()},
                                                      });

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bg);
            pass.DispatchWorkgroups(1);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();

            if (pin == TestPinState::Pinned && !sample) {
                ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
            } else {
                device.GetQueue().Submit(1, &commands);
            }
        }
    }
}

// Test that pinning prevents usage in a render pass if it is not the pinned usage.
TEST_F(ResourceTableValidationTest, PinValidationUsageRenderPass) {
    wgpu::BindGroupLayout sampleLayout = utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::UnfilterableFloat},
                });
    wgpu::BindGroupLayout storageLayout = utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Fragment, wgpu::StorageTextureAccess::ReadOnly,
                     wgpu::TextureFormat::R32Float},
                });

    for (auto pin : kAllTestPinStates) {
        wgpu::Texture tex = CreateTextureWithPinState(
            device, pin, wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding);

        for (bool sample : {false, true}) {
            wgpu::BindGroupLayout bgl = sample ? sampleLayout : storageLayout;
            wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl,
                                                      {
                                                          {0, tex.CreateView()},
                                                      });

            utils::BasicRenderPass rp = utils::CreateBasicRenderPass(device, 1, 1);

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
            pass.SetBindGroup(0, bg);
            pass.End();
            wgpu::CommandBuffer commands = encoder.Finish();

            if (pin == TestPinState::Pinned && !sample) {
                ASSERT_DEVICE_ERROR(device.GetQueue().Submit(1, &commands));
            } else {
                device.GetQueue().Submit(1, &commands);
            }
        }
    }
}

// Checks that only texture views and samplers are allowed as resources in mutators for
// SamplingResourceTable.
TEST_F(ResourceTableValidationTest, MutatorBindingKindValidation) {
    // Create the texture to put in the table.
    wgpu::TextureDescriptor tDesc = {
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::Texture texture = device.CreateTexture(&tDesc);

    // Create the buffer to put in the table.
    wgpu::BufferDescriptor bDesc{
        .usage = wgpu::BufferUsage::Storage,
        .size = 4,
    };
    wgpu::Buffer buffer = device.CreateBuffer(&bDesc);

    // Create the sampler to put in the table.
    wgpu::Sampler sampler = device.CreateSampler();

    for (auto mutator : {Mutator::Update, Mutator::InsertBinding}) {
        // Success case: a single texture is valid.
        {
            wgpu::BindingResource resource = {.textureView = texture.CreateView()};
            TestMutator(mutator, &resource, true);
        }

        // Success case: a single sampler is valid
        {
            wgpu::BindingResource resource = {.sampler = sampler};
            TestMutator(mutator, &resource, true);
        }

        // Error case: a buffer is an error.
        {
            wgpu::BindingResource resource = {.buffer = buffer};
            TestMutator(mutator, &resource, false);
        }

        // Error case: both a sampler and a texture at the same time is an error.
        {
            wgpu::BindingResource resource = {.sampler = sampler,
                                              .textureView = texture.CreateView()};
            TestMutator(mutator, &resource, false);
        }
    }
}

// Tests that resources added/inserted on a table must be valid objects
TEST_F(ResourceTableValidationTest, MutatorResourceMustBeValid) {
    // Create the texture to put in the table.
    wgpu::TextureDescriptor tDesc = {
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::Texture textureGood = device.CreateTexture(&tDesc);

    tDesc.size = {0, 0};
    ASSERT_DEVICE_ERROR(wgpu::Texture textureBad = device.CreateTexture(&tDesc));

    // Create the sampler to put in the table.
    wgpu::SamplerDescriptor sDesc = {};
    wgpu::Sampler samplerGood = device.CreateSampler(&sDesc);

    sDesc.lodMinClamp = -1;
    ASSERT_DEVICE_ERROR(wgpu::Sampler samplerBad = device.CreateSampler(&sDesc));

    for (auto mutator : {Mutator::Update, Mutator::InsertBinding}) {
        // Success case: valid texture
        {
            wgpu::BindingResource resource = {.textureView = textureGood.CreateView()};
            TestMutator(mutator, &resource, true);
        }

        // Error case: invalid texture
        {
            ASSERT_DEVICE_ERROR(
                wgpu::BindingResource resource = {.textureView = textureBad.CreateView()});
            TestMutator(mutator, &resource, false);
        }

        // Success case: valid sampler
        {
            wgpu::BindingResource resource = {.sampler = samplerGood};
            TestMutator(mutator, &resource, true);
        }

        // Error case: invalid sampler
        {
            wgpu::BindingResource resource = {.sampler = samplerBad};
            TestMutator(mutator, &resource, false);
        }
    }
}

// Tests that adding and removing different types of resources works
TEST_F(ResourceTableValidationTest, MutatorsMultipleResources) {
    // Create the texture to put in the table.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::BindingResource textureResource = {.textureView =
                                                 device.CreateTexture(&tDesc).CreateView()};
    wgpu::BindingResource samplerResource = {.sampler = device.CreateSampler()};

    auto table = MakeResourceTable(42);

    EXPECT_EQ(wgpu::Status::Success, table.Update(0, &textureResource));
    EXPECT_EQ(wgpu::Status::Success, table.Update(1, &samplerResource));
    EXPECT_EQ(2u, table.InsertBinding(&textureResource));
    EXPECT_EQ(3u, table.InsertBinding(&samplerResource));

    EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));
    EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(1));
    EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(2));
    EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(3));
}

// Check that the view must have only the TextureBinding usage for SamplingResourceTable.
// TODO(https://issues.chromium.org/473444515): Support storage textures in FullResourceTable
// TODO(https://issues.chromium.org/382544164): Support texel buffers in FullResourceTable
TEST_F(ResourceTableValidationTest, MutatorTextureViewMustBeOnlyTextureBinding) {
    // Create the texture to put in the table.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding |
                 wgpu::TextureUsage::StorageBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::R32Uint,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    for (auto mutator : {Mutator::Update, Mutator::InsertBinding}) {
        // Control case: limiting the usage to TextureBinding is valid.
        {
            wgpu::TextureViewDescriptor vDesc{
                .usage = wgpu::TextureUsage::TextureBinding,
            };
            wgpu::BindingResource resource = {.textureView = tex.CreateView(&vDesc)};
            TestMutator(mutator, &resource, true);
        }

        // Error case: having unrelated usages in the view is not allowed. RenderAttachment case.
        {
            wgpu::TextureViewDescriptor vDesc{
                .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment,
            };
            wgpu::BindingResource resource = {.textureView = tex.CreateView(&vDesc)};
            TestMutator(mutator, &resource, false);
        }

        // Error case: having unrelated usages in the view is not allowed. StorageBinding case.
        {
            wgpu::TextureViewDescriptor vDesc{
                .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding,
            };
            wgpu::BindingResource resource = {.textureView = tex.CreateView(&vDesc)};
            TestMutator(mutator, &resource, false);
        }

        // Error case: the defaulted texture usages don't contain TextureBinding.
        {
            wgpu::TextureDescriptor tDesc2 = tDesc;
            tDesc2.usage = wgpu::TextureUsage::CopyDst;
            wgpu::BindingResource resource = {.textureView =
                                                  device.CreateTexture(&tDesc2).CreateView()};
            TestMutator(mutator, &resource, false);
        }
    }
}

// Check that the texture view must have a single aspect for mutators.
TEST_F(ResourceTableValidationTest, MutatorTextureViewMustBeSingleAspect) {
    // Create the texture to put in the table.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::Depth24PlusStencil8,
    };
    wgpu::Texture tex = device.CreateTexture(&tDesc);

    for (auto mutator : {Mutator::Update, Mutator::InsertBinding}) {
        // Success case, only the depth aspect is selected.
        {
            wgpu::TextureViewDescriptor vDesc{
                .aspect = wgpu::TextureAspect::DepthOnly,
            };
            wgpu::BindingResource resource = {.textureView = tex.CreateView(&vDesc)};
            TestMutator(mutator, &resource, true);
        }

        // Success case, only the stencil aspect is selected.
        {
            wgpu::TextureViewDescriptor vDesc{
                .aspect = wgpu::TextureAspect::StencilOnly,
            };
            wgpu::BindingResource resource = {.textureView = tex.CreateView(&vDesc)};
            TestMutator(mutator, &resource, true);
        }

        // Error case: both aspects are selected.
        {
            wgpu::BindingResource resource = {.textureView = tex.CreateView()};
            TestMutator(mutator, &resource, false);
        }
    }
}

class ResourceTableStaticSamplerValidationTest : public ResourceTableValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {
            wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable,
            wgpu::FeatureName::StaticSamplers,
            wgpu::FeatureName::YCbCrVulkanSamplers,
        };
    }
};

// Checks that YCbCr samplers cannot be added to a resource table
TEST_F(ResourceTableStaticSamplerValidationTest, MutatorSamplerMustNotBeYCbCr) {
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::Sampler samplerDefault = device.CreateSampler(&samplerDesc);

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.externalFormat = 1;
    samplerDesc.nextInChain = &yCbCrDesc;
    wgpu::Sampler samplerYCbCr = device.CreateSampler(&samplerDesc);

    for (auto mutator : {Mutator::Update, Mutator::InsertBinding}) {
        // Success case: default sampler
        {
            wgpu::BindingResource resource = {.sampler = samplerDefault};
            TestMutator(mutator, &resource, true);
        }

        // Error case: YCbCr sampler
        {
            wgpu::BindingResource resource = {.sampler = samplerYCbCr};
            TestMutator(mutator, &resource, false);
        }
    }
}

// Checks that YCbCr texture views cannot be added to a resource table
TEST_F(ResourceTableStaticSamplerValidationTest, MutatorViewMustNotBeYCbCr) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::Texture textureDefault = device.CreateTexture(&tDesc);

    tDesc.format = wgpu::TextureFormat::OpaqueYCbCrAndroid;
    wgpu::Texture textureYCbCr = device.CreateTexture(&tDesc);

    wgpu::YCbCrVkDescriptor yCbCrDesc = {};
    yCbCrDesc.externalFormat = 1;

    wgpu::TextureViewDescriptor vDesc{
        .nextInChain = &yCbCrDesc,
        .format = wgpu::TextureFormat::OpaqueYCbCrAndroid,
        .arrayLayerCount = 1,
        .usage = wgpu::TextureUsage::TextureBinding,
    };

    for (auto mutator : {Mutator::Update, Mutator::InsertBinding}) {
        // Success case: default texture view
        {
            wgpu::BindingResource resource = {.textureView = textureDefault.CreateView()};
            TestMutator(mutator, &resource, true);
        }

        // Error case: YCbCr texture view
        {
            wgpu::BindingResource resource = {.textureView = textureYCbCr.CreateView(&vDesc)};
            // TestMutator(mutator, &resource, false);
        }
    }
}

// Test that it is not allowed to call Update, RemoveBinding or InsertBinding after the table is
// destroyed.
TEST_F(ResourceTableValidationTest, MutatorsAfterDestroy) {
    // Create the texture to put in the table.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::BindingResource resource = {.textureView = device.CreateTexture(&tDesc).CreateView()};

    // This is "content timeline" validation so it works the same on error tables and valid tables,
    // and we ignore device-timeline validation errors, they are not what we are testing here.
    for (auto table : {MakeResourceTable(7), MakeErrorResourceTable(7)}) {
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);

        // Add a few bindings just to test RemoveBinding
        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
        EXPECT_EQ(wgpu::Status::Success, table.Update(1, &resource));

        // Success cases, calling mutators before destroying is valid.
        EXPECT_EQ(wgpu::Status::Success, table.Update(2, &resource));
        EXPECT_NE(wgpu::kInvalidBinding, table.InsertBinding(&resource));
        EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));

        // Error case, after destruction all mutators return errors.
        table.Destroy();
        EXPECT_EQ(wgpu::Status::Error, table.Update(6, &resource));
        EXPECT_EQ(wgpu::kInvalidBinding, table.InsertBinding(&resource));
        EXPECT_EQ(wgpu::Status::Error, table.RemoveBinding(1));
    }
}

// Test that it is not allowed to call Update, RemoveBinding with slots past the end.
TEST_F(ResourceTableValidationTest, MutatorsAfterTableEnd) {
    // Create the texture to put in the table.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::BindingResource resource = {.textureView = device.CreateTexture(&tDesc).CreateView()};

    // This is "content timeline" validation so it works the same on error tables and valid tables,
    // and we ignore device-timeline validation errors, they are not what we are testing here.
    for (auto table : {MakeResourceTable(42), MakeErrorResourceTable(42)}) {
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);

        // Success cases, calling mutators with slots in bounds.
        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
        EXPECT_EQ(wgpu::Status::Success, table.Update(41, &resource));
        EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));
        EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(41));

        // Error case, calling mutators with out of bounds slots.
        EXPECT_EQ(wgpu::Status::Error, table.Update(42, &resource));
        EXPECT_EQ(wgpu::Status::Error, table.RemoveBinding(42));
    }
}

// Test that Update/RemoveBinding return success but generates a validation error when used on an
// invalid table.
TEST_F(ResourceTableValidationTest, MutatorsOnInvalidTable) {
    // Create the texture to put in the table.
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::BindingResource resource = {.textureView = device.CreateTexture(&tDesc).CreateView()};

    // Test on a valid table.
    {
        wgpu::ResourceTable table = MakeResourceTable(3);

        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
        EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));
        EXPECT_NE(wgpu::kInvalidBinding, table.InsertBinding(&resource));
    }

    // Test on an invalid table.
    {
        wgpu::ResourceTable table = MakeErrorResourceTable(3);

        ASSERT_DEVICE_ERROR(EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource)));
        ASSERT_DEVICE_ERROR(EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0)));
        ASSERT_DEVICE_ERROR(EXPECT_NE(wgpu::kInvalidBinding, table.InsertBinding(&resource)));
    }
}

// Test that Update() can be called on a table slot if it has never been used before.
TEST_F(ResourceTableValidationTest, UpdateBindingWhenNeverUsed) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::BindingResource resource = {.textureView = device.CreateTexture(&tDesc).CreateView()};

    // This is "content timeline" validation so it works the same on error tables and valid tables,
    // and we ignore device-timeline validation errors, they are not what we are testing here.
    for (auto table : {MakeResourceTable(3), MakeErrorResourceTable(3)}) {
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);

        // Updating slot 0 when it has never been used is valid, but a second time is an error.
        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
        EXPECT_EQ(wgpu::Status::Error, table.Update(0, &resource));

        // Even after using the table, a previously unused entry is valid to update.
        UseResourceTableInSubmit(table);
        EXPECT_EQ(wgpu::Status::Success, table.Update(1, &resource));
    }
}

// Test that Remove() can be called on a table slot even when it was never used.
TEST_F(ResourceTableValidationTest, RemoveBindingWhenNeverUsed) {
    // This is "content timeline" validation so it works the same on error tables and valid tables,
    // and we ignore device-timeline validation errors, they are not what we are testing here.
    for (auto table : {MakeResourceTable(3), MakeErrorResourceTable(3)}) {
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);
        EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));
    }
}

// Check that a table slot can be updated only after all commands submitted prior to RemoveBinding
// are completed.
TEST_F(ResourceTableValidationTest, UpdateAfterRemoveRequiresGPUIsFinished) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::BindingResource resource = {.textureView = device.CreateTexture(&tDesc).CreateView()};

    // This is "content timeline" validation so it works the same on error tables and valid tables,
    // and we ignore device-timeline validation errors, they are not what we are testing here.
    for (auto table : {MakeResourceTable(1), MakeErrorResourceTable(1)}) {
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);

        // Removing while the table is still potentially in used by the GPU is an error. But
        // immediately after we know that the GPU is finished, it is valid.
        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));

        bool updateValid = false;
        UseResourceTableInSubmit(table);
        device.GetQueue().OnSubmittedWorkDone(
            wgpu::CallbackMode::AllowSpontaneous,
            [&](wgpu::QueueWorkDoneStatus, wgpu::StringView) { updateValid = true; });
        EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));

        // The null backend happens to call OnSubmittedWorkDone immediately because commands take 0
        // time. This test is duplicated in the end2end tests where OnSubmittedWorkDone won't fire
        // immediately.
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

// Check that trying to insert bindings fail when no more are available.
TEST_F(ResourceTableValidationTest, InsertBindingFailWhenNoMoreSpace) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::BindingResource resource = {.textureView = device.CreateTexture(&tDesc).CreateView()};

    // This is "content timeline" validation so it works the same on error tables and valid tables,
    // and we ignore device-timeline validation errors, they are not what we are testing here.
    for (auto table : {MakeResourceTable(3), MakeErrorResourceTable(3)}) {
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);

        // There is space for only three resources.
        EXPECT_EQ(0u, table.InsertBinding(&resource));
        EXPECT_EQ(1u, table.InsertBinding(&resource));
        EXPECT_EQ(2u, table.InsertBinding(&resource));
        EXPECT_EQ(wgpu::kInvalidBinding, table.InsertBinding(&resource));

        // Remove one binding (and wait for it to be recycled), it will be available for
        // InsertBinding after which a new InsertBinding will still run out of space.
        EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(1));
        UseResourceTableInSubmit(table);
        WaitForAllOperations();

        EXPECT_EQ(1u, table.InsertBinding(&resource));
        EXPECT_EQ(wgpu::kInvalidBinding, table.InsertBinding(&resource));
    }
}

// Check that bindings that are inserted are unavailable for Update() until RemoveBinding.
TEST_F(ResourceTableValidationTest, InsertBindingPreventsUpdate) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::BindingResource resource = {.textureView = device.CreateTexture(&tDesc).CreateView()};

    // This is "content timeline" validation so it works the same on error tables and valid tables,
    // and we ignore device-timeline validation errors, they are not what we are testing here.
    for (auto table : {MakeResourceTable(1), MakeErrorResourceTable(1)}) {
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);

        EXPECT_EQ(0u, table.InsertBinding(&resource));
        EXPECT_EQ(wgpu::Status::Error, table.Update(0, &resource));

        // Remove one binding (and wait for it to be recycled), it will be available for Update.
        EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(0));
        UseResourceTableInSubmit(table);
        WaitForAllOperations();

        EXPECT_EQ(wgpu::Status::Success, table.Update(0, &resource));
    }
}

// Check that InsertBinding skips over used slots.
TEST_F(ResourceTableValidationTest, InsertBindingSkipsOverUsedSlots) {
    wgpu::TextureDescriptor tDesc{
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {1, 1},
        .format = wgpu::TextureFormat::RGBA8Unorm,
    };
    wgpu::BindingResource resource = {.textureView = device.CreateTexture(&tDesc).CreateView()};

    // This is "content timeline" validation so it works the same on error tables and valid tables,
    // and we ignore device-timeline validation errors, they are not what we are testing here.
    for (auto table : {MakeResourceTable(5), MakeErrorResourceTable(5)}) {
        utils::ScopedIgnoreValidationErrors ignoreErrors(device);
        EXPECT_EQ(wgpu::Status::Success, table.Update(1, &resource));
        EXPECT_EQ(wgpu::Status::Success, table.Update(3, &resource));

        // InsertBinding skips over entries used by Update()
        EXPECT_EQ(0u, table.InsertBinding(&resource));
        EXPECT_EQ(2u, table.InsertBinding(&resource));
        EXPECT_EQ(4u, table.InsertBinding(&resource));

        // Remove bindings in inverse order.
        for (uint32_t i : {4, 3, 2}) {
            EXPECT_EQ(wgpu::Status::Success, table.RemoveBinding(i));
        }
        UseResourceTableInSubmit(table);
        WaitForAllOperations();

        // InsertBinding should still return the min available slot.
        EXPECT_EQ(2u, table.InsertBinding(&resource));
        EXPECT_EQ(3u, table.InsertBinding(&resource));
        EXPECT_EQ(4u, table.InsertBinding(&resource));
    }
}

// Test the value returned by GetSize right after creating the table.
TEST_F(ResourceTableValidationTest, GetSizeAfterCreation) {
    // Valid resource tables of varying size.
    {
        EXPECT_EQ(0u, MakeResourceTable(0).GetSize());
        EXPECT_EQ(42u, MakeResourceTable(42).GetSize());
        EXPECT_EQ(kMaxResourceTableSize, MakeResourceTable(kMaxResourceTableSize).GetSize());
    }

    // Invalid resource tables of varying size under the limit.
    {
        EXPECT_EQ(0u, MakeErrorResourceTable(0).GetSize());
        EXPECT_EQ(42u, MakeErrorResourceTable(42).GetSize());
        EXPECT_EQ(kMaxResourceTableSize, MakeErrorResourceTable(kMaxResourceTableSize).GetSize());
    }
}

// Test the value returned by GetSize after calling Destroy() should return the same value.
TEST_F(ResourceTableValidationTest, GetSizeAfterDestroy) {
    // Valid resource table.
    {
        wgpu::ResourceTable table = MakeResourceTable(42);
        EXPECT_EQ(42u, table.GetSize());
        table.Destroy();
        EXPECT_EQ(42u, table.GetSize());
    }

    // Invalid resource table.
    {
        wgpu::ResourceTable table = MakeResourceTable(42);
        EXPECT_EQ(42u, table.GetSize());
        table.Destroy();
        EXPECT_EQ(42u, table.GetSize());
    }
}

}  // namespace
}  // namespace dawn
