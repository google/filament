// Copyright 2023 The Dawn & Tint Authors
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
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Numeric.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using BindingDescriptorGroups = std::vector<std::vector<utils::BindingInitializationHelper>>;

struct TestSet {
    bool valid;
    BindingDescriptorGroups bindingEntries;
};

constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::RGBA8Unorm;

wgpu::TextureViewDescriptor GetTextureViewDescriptor(
    uint32_t baseMipLevel,
    uint32_t mipLevelcount,
    uint32_t baseArrayLayer,
    uint32_t arrayLayerCount,
    wgpu::TextureAspect aspect = wgpu::TextureAspect::All) {
    wgpu::TextureViewDescriptor descriptor;
    descriptor.dimension = wgpu::TextureViewDimension::e2DArray;
    descriptor.baseMipLevel = baseMipLevel;
    descriptor.mipLevelCount = mipLevelcount;
    descriptor.baseArrayLayer = baseArrayLayer;
    descriptor.arrayLayerCount = arrayLayerCount;
    descriptor.aspect = aspect;
    return descriptor;
}

// Creates a bind group with given bindings for shader text.
std::string GenerateBindingString(const BindingDescriptorGroups& descriptors) {
    std::ostringstream ostream;
    size_t index = 0;
    uint32_t groupIndex = 0;
    for (const auto& entries : descriptors) {
        for (uint32_t bindingIndex = 0; bindingIndex < entries.size(); bindingIndex++) {
            // All texture view binding format uses RGBA8Unorm in this test.
            ostream << "@group(" << groupIndex << ") @binding(" << bindingIndex << ") " << "var b"
                    << index << " : texture_storage_2d_array<rgba8unorm, write>;\n";

            index++;
        }
        groupIndex++;
    }
    return ostream.str();
}

// Creates reference shader text to make sure variables don't get optimized out.
std::string GenerateReferenceString(const BindingDescriptorGroups& descriptors) {
    std::ostringstream ostream;
    size_t index = 0;
    for (const auto& entries : descriptors) {
        for (uint32_t bindingIndex = 0; bindingIndex < entries.size(); bindingIndex++) {
            ostream << "_ = b" << index << ";\n";
            index++;
        }
    }
    return ostream.str();
}

// Creates a compute shader with given bindings
std::string CreateComputeShaderWithBindings(const BindingDescriptorGroups& bindingsGroups) {
    return GenerateBindingString(bindingsGroups) + "@compute @workgroup_size(1,1,1) fn main() {\n" +
           GenerateReferenceString(bindingsGroups) + "}";
}

// Creates a fragment shader with given bindings
std::string CreateFragmentShaderWithBindings(const BindingDescriptorGroups& bindingsGroups) {
    return GenerateBindingString(bindingsGroups) + "@fragment fn main() {\n" +
           GenerateReferenceString(bindingsGroups) + "}";
}

const char* kVertexShader = R"(
@vertex fn main() -> @builtin(position) vec4<f32> {
    return vec4<f32>();
}
)";

class WritableTextureBindingAliasingValidationTests : public ValidationTest {
  public:
    wgpu::Texture CreateTexture(wgpu::TextureUsage usage,
                                wgpu::TextureFormat format,
                                uint32_t mipLevelCount,
                                uint32_t arrayLayerCount,
                                wgpu::TextureDimension dimension = wgpu::TextureDimension::e2D) {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = dimension;
        descriptor.size = {16, 16, arrayLayerCount};
        descriptor.sampleCount = 1;
        descriptor.format = format;
        descriptor.mipLevelCount = mipLevelCount;
        descriptor.usage = usage;
        return device.CreateTexture(&descriptor);
    }

    // Creates compute pipeline given a layout and shader
    wgpu::ComputePipeline CreateComputePipeline(const std::vector<wgpu::BindGroupLayout>& layouts,
                                                const std::string& shader) {
        wgpu::ShaderModule csModule = utils::CreateShaderModule(device, shader.c_str());

        wgpu::ComputePipelineDescriptor csDesc;
        wgpu::PipelineLayoutDescriptor descriptor;
        descriptor.bindGroupLayoutCount = layouts.size();
        descriptor.bindGroupLayouts = layouts.data();
        csDesc.layout = device.CreatePipelineLayout(&descriptor);
        csDesc.compute.module = csModule;

        return device.CreateComputePipeline(&csDesc);
    }

    // Creates render pipeline given layouts and shaders
    wgpu::RenderPipeline CreateRenderPipeline(const std::vector<wgpu::BindGroupLayout>& layouts,
                                              const std::string& vertexShader,
                                              const std::string& fragShader) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vertexShader.c_str());

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fragShader.c_str());

        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = vsModule;
        pipelineDescriptor.cFragment.module = fsModule;
        pipelineDescriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        pipelineDescriptor.layout = nullptr;

        DAWN_ASSERT(!layouts.empty());
        wgpu::PipelineLayoutDescriptor descriptor;
        descriptor.bindGroupLayoutCount = layouts.size();
        descriptor.bindGroupLayouts = layouts.data();
        pipelineDescriptor.layout = device.CreatePipelineLayout(&descriptor);

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    // Creates bind group layout with given minimum sizes for each binding
    wgpu::BindGroupLayout CreateBindGroupLayout(
        const std::vector<utils::BindingInitializationHelper>& bindings) {
        std::vector<wgpu::BindGroupLayoutEntry> entries;

        for (size_t i = 0; i < bindings.size(); ++i) {
            const utils::BindingInitializationHelper& b = bindings[i];
            wgpu::BindGroupLayoutEntry e = {};
            e.binding = b.binding;
            e.visibility = wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment;
            e.storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;  // only enum supported
            e.storageTexture.format = kTextureFormat;
            e.storageTexture.viewDimension = wgpu::TextureViewDimension::e2DArray;

            entries.push_back(e);
        }

        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.entryCount = entries.size();
        descriptor.entries = entries.data();
        return device.CreateBindGroupLayout(&descriptor);
    }

    std::vector<wgpu::BindGroup> CreateBindGroups(const std::vector<wgpu::BindGroupLayout>& layouts,
                                                  const BindingDescriptorGroups& bindingsGroups) {
        std::vector<wgpu::BindGroup> bindGroups;

        DAWN_ASSERT(layouts.size() == bindingsGroups.size());
        for (size_t groupIdx = 0; groupIdx < layouts.size(); groupIdx++) {
            const auto& bindings = bindingsGroups[groupIdx];

            std::vector<wgpu::BindGroupEntry> entries;
            entries.reserve(bindings.size());
            for (const auto& binding : bindings) {
                entries.push_back(binding.GetAsBinding());
            }

            wgpu::BindGroupDescriptor descriptor;
            descriptor.layout = layouts[groupIdx];
            descriptor.entryCount = entries.size();
            descriptor.entries = entries.data();

            bindGroups.push_back(device.CreateBindGroup(&descriptor));
        }

        return bindGroups;
    }

    // Runs a single dispatch with given pipeline and bind group
    void TestDispatch(const wgpu::ComputePipeline& computePipeline,
                      const std::vector<wgpu::BindGroup>& bindGroups,
                      const TestSet& test) {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetPipeline(computePipeline);

        DAWN_ASSERT(bindGroups.size() == test.bindingEntries.size());
        DAWN_ASSERT(bindGroups.size() > 0);
        for (size_t i = 0; i < bindGroups.size(); ++i) {
            computePassEncoder.SetBindGroup(i, bindGroups[i]);
        }

        computePassEncoder.DispatchWorkgroups(1);
        computePassEncoder.End();
        if (!test.valid) {
            ASSERT_DEVICE_ERROR(commandEncoder.Finish());
        } else {
            commandEncoder.Finish();
        }
    }

    // Runs a single draw with given pipeline and bind group
    void TestDraw(const wgpu::RenderPipeline& renderPipeline,
                  const std::vector<wgpu::BindGroup>& bindGroups,
                  const TestSet& test) {
        PlaceholderRenderPass renderPass(device);

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.SetPipeline(renderPipeline);

        DAWN_ASSERT(bindGroups.size() == test.bindingEntries.size());
        DAWN_ASSERT(bindGroups.size() > 0);
        for (size_t i = 0; i < bindGroups.size(); ++i) {
            renderPassEncoder.SetBindGroup(i, bindGroups[i]);
        }

        renderPassEncoder.Draw(3);
        renderPassEncoder.End();
        if (!test.valid) {
            ASSERT_DEVICE_ERROR(commandEncoder.Finish());
        } else {
            commandEncoder.Finish();
        }
    }

    void TestBindings(const wgpu::ComputePipeline& computePipeline,
                      const wgpu::RenderPipeline& renderPipeline,
                      const std::vector<wgpu::BindGroupLayout>& layouts,
                      const TestSet& test) {
        std::vector<wgpu::BindGroup> bindGroups = CreateBindGroups(layouts, test.bindingEntries);

        TestDispatch(computePipeline, bindGroups, test);
        TestDraw(renderPipeline, bindGroups, test);
    }
};

// Test various combinations of texture mip levels, array layers, aspects, bind groups, etc.
// validating aliasing
TEST_F(WritableTextureBindingAliasingValidationTests, BasicTest) {
    wgpu::Texture textureStorage =
        CreateTexture(wgpu::TextureUsage::StorageBinding, kTextureFormat, 4, 4);
    wgpu::Texture textureStorage2 =
        CreateTexture(wgpu::TextureUsage::StorageBinding, kTextureFormat, 4, 4);

    // view0 and view1 don't intersect at all
    wgpu::TextureViewDescriptor viewDescriptor0 = GetTextureViewDescriptor(0, 1, 0, 1);
    wgpu::TextureView view0 = textureStorage.CreateView(&viewDescriptor0);
    wgpu::TextureViewDescriptor viewDescriptor1 = GetTextureViewDescriptor(1, 1, 1, 1);
    wgpu::TextureView view1 = textureStorage.CreateView(&viewDescriptor1);

    // view2 and view3 intersects in mip levels only
    wgpu::TextureViewDescriptor viewDescriptor2 = GetTextureViewDescriptor(0, 1, 0, 1);
    wgpu::TextureView view2 = textureStorage.CreateView(&viewDescriptor2);
    wgpu::TextureViewDescriptor viewDescriptor3 = GetTextureViewDescriptor(0, 1, 1, 1);
    wgpu::TextureView view3 = textureStorage.CreateView(&viewDescriptor3);

    // view4 and view5 intersects in array layers only
    wgpu::TextureViewDescriptor viewDescriptor4 = GetTextureViewDescriptor(0, 1, 0, 3);
    wgpu::TextureView view4 = textureStorage.CreateView(&viewDescriptor4);
    wgpu::TextureViewDescriptor viewDescriptor5 = GetTextureViewDescriptor(1, 1, 1, 3);
    wgpu::TextureView view5 = textureStorage.CreateView(&viewDescriptor5);

    // view6 and view7 intersects in both mip levels and array layers
    wgpu::TextureViewDescriptor viewDescriptor6 = GetTextureViewDescriptor(0, 1, 0, 3);
    wgpu::TextureView view6 = textureStorage.CreateView(&viewDescriptor6);
    wgpu::TextureViewDescriptor viewDescriptor7 = GetTextureViewDescriptor(0, 1, 1, 3);
    wgpu::TextureView view7 = textureStorage.CreateView(&viewDescriptor7);

    // view72 is created by another texture, so no aliasing at all.
    wgpu::TextureView view72 = textureStorage2.CreateView(&viewDescriptor7);

    std::vector<TestSet> testSet = {
        // same texture, subresources don't intersect
        {true,
         {{
             {0, view0},
             {1, view1},
         }}},
        // same texture, subresources don't intersect
        {true,
         {{
             {0, view2},
             {1, view3},
         }}},
        // same texture, subresources don't intersect, in different bind groups
        {true,
         {{
              {0, view0},
          },
          {
              {0, view1},
          }}},
        // same texture, subresources intersect in array layers
        {true,
         {{
             {0, view4},
             {1, view5},
         }}},

        // same texture, subresources intersect in both mip levels and array layers
        {false,
         {{
             {0, view6},
             {1, view7},
         }}},
        // reverse order to test range overlap logic
        {false,
         {{
             {0, view6},
             {1, view7},
         }}},
        // subreources intersect in different bind groups
        {false,
         {{
              {0, view6},
          },
          {
              {0, view7},
          }}},
        // different texture, no aliasing at all
        {true,
         {{
             {0, view6},
             {1, view72},
         }}},
        // Altough spec says texture aspect could also affect whether two texture view intersects,
        // It is not possible to create storage texture with depth stencil format, with different
        // aspect values (all, depth only, stencil only)
        // So we don't have tests for this case.
    };

    for (const auto& test : testSet) {
        std::vector<wgpu::BindGroupLayout> layouts;
        for (const std::vector<utils::BindingInitializationHelper>& bindings :
             test.bindingEntries) {
            layouts.push_back(CreateBindGroupLayout(bindings));
        }

        std::string computeShader = CreateComputeShaderWithBindings(test.bindingEntries);
        wgpu::ComputePipeline computePipeline = CreateComputePipeline(layouts, computeShader);
        std::string fragmentShader = CreateFragmentShaderWithBindings(test.bindingEntries);
        wgpu::RenderPipeline renderPipeline =
            CreateRenderPipeline(layouts, kVertexShader, fragmentShader);

        TestBindings(computePipeline, renderPipeline, layouts, test);
    }
}

// Test if validate bind group lazy aspect flag is set and checked properly
TEST_F(WritableTextureBindingAliasingValidationTests, SetBindGroupLazyAspect) {
    wgpu::Texture textureStorage =
        CreateTexture(wgpu::TextureUsage::StorageBinding, kTextureFormat, 4, 4);

    // view0 and view1 don't intersect
    wgpu::TextureViewDescriptor viewDescriptor0 = GetTextureViewDescriptor(0, 1, 0, 1);
    wgpu::TextureView view0 = textureStorage.CreateView(&viewDescriptor0);
    wgpu::TextureViewDescriptor viewDescriptor1 = GetTextureViewDescriptor(1, 1, 1, 1);
    wgpu::TextureView view1 = textureStorage.CreateView(&viewDescriptor1);

    // view2 and view3 intersects
    wgpu::TextureViewDescriptor viewDescriptor2 = GetTextureViewDescriptor(0, 1, 0, 2);
    wgpu::TextureView view2 = textureStorage.CreateView(&viewDescriptor2);
    wgpu::TextureViewDescriptor viewDescriptor3 = GetTextureViewDescriptor(0, 1, 1, 2);
    wgpu::TextureView view3 = textureStorage.CreateView(&viewDescriptor3);

    // subresources don't intersect, create valid bindGroups
    std::vector<utils::BindingInitializationHelper> bindingDescriptor0 = {{
        {0, view0},
        {1, view1},
    }};
    // subresources intersect, create invalid bindGroups
    std::vector<utils::BindingInitializationHelper> bindingDescriptor1 = {{
        {0, view2},
        {1, view3},
    }};

    // bindingDescriptor0 and 1 share the same bind group layout, shader and pipeline
    wgpu::BindGroupLayout layout = CreateBindGroupLayout(bindingDescriptor0);

    std::string computeShader = CreateComputeShaderWithBindings({bindingDescriptor0});
    wgpu::ComputePipeline computePipeline = CreateComputePipeline({layout}, computeShader);
    std::string fragmentShader = CreateFragmentShaderWithBindings({bindingDescriptor0});
    wgpu::RenderPipeline renderPipeline =
        CreateRenderPipeline({layout}, kVertexShader, fragmentShader);

    std::vector<wgpu::BindGroup> bindGroups =
        CreateBindGroups({layout, layout}, {bindingDescriptor0, bindingDescriptor1});

    // Test compute pass dispatch

    // bindGroups[0] is valid
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetPipeline(computePipeline);

        computePassEncoder.SetBindGroup(0, bindGroups[0]);
        computePassEncoder.DispatchWorkgroups(1);

        computePassEncoder.End();
        commandEncoder.Finish();
    }

    // bindGroups[1] is invalid
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetPipeline(computePipeline);

        computePassEncoder.SetBindGroup(0, bindGroups[1]);
        computePassEncoder.DispatchWorkgroups(1);

        computePassEncoder.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // setting bindGroups[1] first and then resetting to bindGroups[0] is valid
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetPipeline(computePipeline);

        computePassEncoder.SetBindGroup(0, bindGroups[1]);
        computePassEncoder.SetBindGroup(0, bindGroups[0]);
        computePassEncoder.DispatchWorkgroups(1);

        computePassEncoder.End();
        commandEncoder.Finish();
    }

    // bindGroups[0] is valid, bindGroups[1] is invalid but set to an unused slot, should still be
    // valid
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetPipeline(computePipeline);

        computePassEncoder.SetBindGroup(0, bindGroups[0]);
        computePassEncoder.SetBindGroup(1, bindGroups[1]);
        computePassEncoder.DispatchWorkgroups(1);

        computePassEncoder.End();
        commandEncoder.Finish();
    }

    // Test render pass draw

    PlaceholderRenderPass renderPass(device);

    // bindGroups[0] is valid
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.SetPipeline(renderPipeline);

        renderPassEncoder.SetBindGroup(0, bindGroups[0]);
        renderPassEncoder.Draw(3);

        renderPassEncoder.End();
        commandEncoder.Finish();
    }

    // bindGroups[1] is invalid
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.SetPipeline(renderPipeline);

        renderPassEncoder.SetBindGroup(0, bindGroups[1]);
        renderPassEncoder.Draw(3);

        renderPassEncoder.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // setting bindGroups[1] first and then resetting to bindGroups[0] is valid
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.SetPipeline(renderPipeline);

        renderPassEncoder.SetBindGroup(0, bindGroups[1]);
        renderPassEncoder.SetBindGroup(0, bindGroups[0]);
        renderPassEncoder.Draw(3);

        renderPassEncoder.End();
        commandEncoder.Finish();
    }

    // bindGroups[0] is valid, bindGroups[1] is invalid but set to an unused slot, should still be
    // valid
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.SetPipeline(renderPipeline);

        renderPassEncoder.SetBindGroup(0, bindGroups[0]);
        renderPassEncoder.SetBindGroup(1, bindGroups[1]);
        renderPassEncoder.Draw(3);

        renderPassEncoder.End();
        commandEncoder.Finish();
    }
}

}  // anonymous namespace
}  // namespace dawn
