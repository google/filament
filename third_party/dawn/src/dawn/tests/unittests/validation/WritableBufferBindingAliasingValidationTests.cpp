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

// Helper for describing bindings throughout the tests
struct BindingDescriptor {
    utils::BindingInitializationHelper binding;
    wgpu::BufferBindingType type = wgpu::BufferBindingType::Storage;

    bool hasDynamicOffset = false;
    uint32_t dynamicOffset = 0;

    wgpu::ShaderStage visibility = wgpu::ShaderStage::Compute | wgpu::ShaderStage::Fragment;
};

using BindingDescriptorGroups = std::vector<std::vector<BindingDescriptor>>;
struct TestSet {
    bool valid;
    BindingDescriptorGroups bindingEntries;
};

// Creates a bind group with given bindings for shader text
std::string GenerateBindingString(const BindingDescriptorGroups& bindingsGroups) {
    std::ostringstream ostream;
    size_t index = 0;
    size_t groupIndex = 0;
    for (const auto& bindings : bindingsGroups) {
        for (const BindingDescriptor& b : bindings) {
            ostream << "struct S" << index << " { " << "buffer : array<f32>" << "}\n";
            ostream << "@group(" << groupIndex << ") @binding(" << b.binding.binding << ") ";
            switch (b.type) {
                case wgpu::BufferBindingType::Uniform:
                    ostream << "var<uniform> b" << index << " : S" << index << ";\n";
                    break;
                case wgpu::BufferBindingType::Storage:
                    ostream << "var<storage, read_write> b" << index << " : S" << index << ";\n";
                    break;
                case wgpu::BufferBindingType::ReadOnlyStorage:
                    ostream << "var<storage, read> b" << index << " : S" << index << ";\n";
                    break;
                default:
                    DAWN_UNREACHABLE();
            }
            index++;
        }
        groupIndex++;
    }
    return ostream.str();
}

std::string GenerateReferenceString(const BindingDescriptorGroups& bindingsGroups,
                                    wgpu::ShaderStage stage) {
    std::ostringstream ostream;
    size_t index = 0;
    for (const auto& bindings : bindingsGroups) {
        for (const BindingDescriptor& b : bindings) {
            if (b.visibility & stage) {
                ostream << "_ = b" << index << "." << "buffer[0]" << ";\n";
            }
            index++;
        }
    }
    return ostream.str();
}

// Creates a compute shader with given bindings
std::string CreateComputeShaderWithBindings(const BindingDescriptorGroups& bindingsGroups) {
    return GenerateBindingString(bindingsGroups) + "@compute @workgroup_size(1,1,1) fn main() {\n" +
           GenerateReferenceString(bindingsGroups, wgpu::ShaderStage::Compute) + "}";
}

// Creates a vertex shader with given bindings
std::string CreateVertexShaderWithBindings(const BindingDescriptorGroups& bindingsGroups) {
    return GenerateBindingString(bindingsGroups) +
           "@vertex fn main() -> @builtin(position) vec4<f32> {\n" +
           GenerateReferenceString(bindingsGroups, wgpu::ShaderStage::Vertex) +
           "\n   return vec4<f32>(); " + "}";
}

// Creates a fragment shader with given bindings
std::string CreateFragmentShaderWithBindings(const BindingDescriptorGroups& bindingsGroups) {
    return GenerateBindingString(bindingsGroups) + "@fragment fn main() {\n" +
           GenerateReferenceString(bindingsGroups, wgpu::ShaderStage::Fragment) + "}";
}

class WritableBufferBindingAliasingValidationTests : public ValidationTest {
  public:
    wgpu::Buffer CreateBuffer(uint64_t bufferSize, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor bufferDescriptor;
        bufferDescriptor.size = bufferSize;
        bufferDescriptor.usage = usage;

        return device.CreateBuffer(&bufferDescriptor);
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
        if (!layouts.empty()) {
            wgpu::PipelineLayoutDescriptor descriptor;
            descriptor.bindGroupLayoutCount = layouts.size();
            descriptor.bindGroupLayouts = layouts.data();
            pipelineDescriptor.layout = device.CreatePipelineLayout(&descriptor);
        }

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    // Creates bind group layout with given minimum sizes for each binding
    wgpu::BindGroupLayout CreateBindGroupLayout(const std::vector<BindingDescriptor>& bindings) {
        std::vector<wgpu::BindGroupLayoutEntry> entries;

        for (size_t i = 0; i < bindings.size(); ++i) {
            const BindingDescriptor& b = bindings[i];
            wgpu::BindGroupLayoutEntry e = {};
            e.binding = b.binding.binding;
            e.visibility = b.visibility;
            e.buffer.type = b.type;
            e.buffer.minBindingSize = 0;
            e.buffer.hasDynamicOffset = b.hasDynamicOffset;

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
                entries.push_back(binding.binding.GetAsBinding());
            }

            wgpu::BindGroupDescriptor descriptor;
            descriptor.layout = layouts[groupIdx];
            descriptor.entryCount = checked_cast<uint32_t>(entries.size());
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
            // Assuming that in our test we
            // (1) only have buffer bindings and
            // (2) only have buffer bindings with same hasDynamicOffset across one bindGroup,
            // the dynamic buffer binding is always compact.
            if (test.bindingEntries[i][0].hasDynamicOffset) {
                // build the dynamicOffset vector
                const auto& b = test.bindingEntries[i];
                std::vector<uint32_t> dynamicOffsets(b.size());
                for (size_t j = 0; j < b.size(); ++j) {
                    dynamicOffsets[j] = b[j].dynamicOffset;
                }

                computePassEncoder.SetBindGroup(i, bindGroups[i], dynamicOffsets.size(),
                                                dynamicOffsets.data());
            } else {
                computePassEncoder.SetBindGroup(i, bindGroups[i]);
            }
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
            // Assuming that in our test we
            // (1) only have buffer bindings and
            // (2) only have buffer bindings with same hasDynamicOffset across one bindGroup,
            // the dynamic buffer binding is always compact.
            if (test.bindingEntries[i][0].hasDynamicOffset) {
                const auto& b = test.bindingEntries[i];
                std::vector<uint32_t> dynamicOffsets(b.size());
                for (size_t j = 0; j < b.size(); ++j) {
                    dynamicOffsets[j] = b[j].dynamicOffset;
                }

                renderPassEncoder.SetBindGroup(i, bindGroups[i], dynamicOffsets.size(),
                                               dynamicOffsets.data());
            } else {
                renderPassEncoder.SetBindGroup(i, bindGroups[i]);
            }
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

// Test various combinations of buffer ranges, buffer usages, bind groups, etc. validating aliasing
TEST_F(WritableBufferBindingAliasingValidationTests, BasicTest) {
    wgpu::Buffer bufferStorage =
        CreateBuffer(1024, wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage);
    wgpu::Buffer bufferStorage2 =
        CreateBuffer(1024, wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage);
    wgpu::Buffer bufferNoStorage = CreateBuffer(1024, wgpu::BufferUsage::Uniform);

    std::vector<TestSet> testSet = {
        // same buffer, ranges don't overlap
        {true,
         {{
             {{0, bufferStorage, 256, 16}, wgpu::BufferBindingType::Storage},
             {{1, bufferStorage, 0, 8}, wgpu::BufferBindingType::Storage},
         }}},
        // same buffer, ranges overlap, in same bind group, max0 >= min1
        {false,
         {{
             {{0, bufferStorage, 256, 16}, wgpu::BufferBindingType::Storage},
             {{1, bufferStorage, 0, 264}, wgpu::BufferBindingType::Storage},
         }}},
        // same buffer, ranges overlap, in same bind group, max1 >= min0
        {false,
         {{
             {{0, bufferStorage, 0, 264}, wgpu::BufferBindingType::Storage},
             {{1, bufferStorage, 256, 16}, wgpu::BufferBindingType::Storage},
         }}},
        // same buffer, ranges don't overlap, in different bind group
        {true,
         {{
              {{0, bufferStorage, 256, 16}, wgpu::BufferBindingType::Storage},
          },
          {
              {{0, bufferStorage, 0, 8}, wgpu::BufferBindingType::Storage},
          }}},
        // same buffer, ranges overlap, in different bind group
        {false,
         {{
              {{0, bufferStorage, 0, 16}, wgpu::BufferBindingType::Storage},
          },
          {
              {{0, bufferStorage, 0, 8}, wgpu::BufferBindingType::Storage},
          }}},
        // same buffer, ranges overlap, but with read-only storage buffer type
        {true,
         {{
             {{0, bufferStorage, 0, 16}, wgpu::BufferBindingType::ReadOnlyStorage},
             {{1, bufferStorage, 0, 8}, wgpu::BufferBindingType::ReadOnlyStorage},
         }}},
        // different buffer, ranges overlap, valid
        {true,
         {{
             {{0, bufferStorage, 0, 16}, wgpu::BufferBindingType::Storage},
             {{1, bufferStorage2, 0, 8}, wgpu::BufferBindingType::Storage},
         }}},
        // same buffer, ranges don't overlap, but dynamic offset creates overlap.
        {false,
         {{
             {{0, bufferStorage, 256, 16}, wgpu::BufferBindingType::Storage, true, 0},
             {{1, bufferStorage, 0, 8}, wgpu::BufferBindingType::Storage, true, 256},
         }}},
        // same buffer, ranges don't overlap, but one binding has dynamic offset and creates
        // overlap.
        {false,
         {{
              {{0, bufferStorage, 256, 16}, wgpu::BufferBindingType::Storage},
          },
          {
              {{0, bufferStorage, 0, 8}, wgpu::BufferBindingType::Storage, true, 256},
          }}},
    };

    for (const auto& test : testSet) {
        std::vector<wgpu::BindGroupLayout> layouts;
        for (const std::vector<BindingDescriptor>& bindings : test.bindingEntries) {
            layouts.push_back(CreateBindGroupLayout(bindings));
        }

        std::string computeShader = CreateComputeShaderWithBindings(test.bindingEntries);
        wgpu::ComputePipeline computePipeline = CreateComputePipeline(layouts, computeShader);

        std::string vertexShader = CreateVertexShaderWithBindings(test.bindingEntries);
        std::string fragmentShader = CreateFragmentShaderWithBindings(test.bindingEntries);
        wgpu::RenderPipeline renderPipeline =
            CreateRenderPipeline(layouts, vertexShader, fragmentShader);

        TestBindings(computePipeline, renderPipeline, layouts, test);
    }
}

// Test if validate bind group lazy aspect flag is set and checked properly
TEST_F(WritableBufferBindingAliasingValidationTests, SetBindGroupLazyAspect) {
    wgpu::Buffer bufferStorage =
        CreateBuffer(1024, wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage);

    // no overlap, create valid bindGroups
    std::vector<BindingDescriptor> bindingDescriptor0 = {
        {{0, bufferStorage, 256, 16}, wgpu::BufferBindingType::Storage},
        {{1, bufferStorage, 0, 8}, wgpu::BufferBindingType::Storage},
    };
    // overlap, create invalid bindGroups
    std::vector<BindingDescriptor> bindingDescriptor1 = {
        {{0, bufferStorage, 0, 16}, wgpu::BufferBindingType::Storage},
        {{1, bufferStorage, 0, 8}, wgpu::BufferBindingType::Storage},
    };

    // bindingDescriptor0 and 1 share the same bind group layout, shader and pipeline
    wgpu::BindGroupLayout layout = CreateBindGroupLayout(bindingDescriptor0);

    std::string computeShader = CreateComputeShaderWithBindings({bindingDescriptor0});
    wgpu::ComputePipeline computePipeline = CreateComputePipeline({layout}, computeShader);
    std::string vertexShader = CreateVertexShaderWithBindings({bindingDescriptor0});
    std::string fragmentShader = CreateFragmentShaderWithBindings({bindingDescriptor0});
    wgpu::RenderPipeline renderPipeline =
        CreateRenderPipeline({layout}, vertexShader, fragmentShader);

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
}

// Test if validate bind group lazy aspect flag is set and checked properly for bind group layout
// with dynamic offset
TEST_F(WritableBufferBindingAliasingValidationTests, SetBindGroupLazyAspectDynamicOffset) {
    wgpu::Buffer bufferStorage =
        CreateBuffer(1024, wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage);

    // no overlap, but has dynamic offset
    std::vector<BindingDescriptor> bindingDescriptor = {
        {{0, bufferStorage, 256, 16}, wgpu::BufferBindingType::Storage, true},
        {{1, bufferStorage, 0, 8}, wgpu::BufferBindingType::Storage, true},
    };

    wgpu::BindGroupLayout layout = CreateBindGroupLayout(bindingDescriptor);

    std::string computeShader = CreateComputeShaderWithBindings({bindingDescriptor});
    wgpu::ComputePipeline computePipeline = CreateComputePipeline({layout}, computeShader);
    std::string vertexShader = CreateVertexShaderWithBindings({bindingDescriptor});
    std::string fragmentShader = CreateFragmentShaderWithBindings({bindingDescriptor});
    wgpu::RenderPipeline renderPipeline =
        CreateRenderPipeline({layout}, vertexShader, fragmentShader);

    std::vector<wgpu::BindGroup> bindGroups = CreateBindGroups({layout}, {bindingDescriptor});

    // Test compute pass dispatch

    // bindGroups[0] is valid
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetPipeline(computePipeline);

        std::vector<uint32_t> dynamicOffsets = {0, 0};

        computePassEncoder.SetBindGroup(0, bindGroups[0], dynamicOffsets.size(),
                                        dynamicOffsets.data());
        computePassEncoder.DispatchWorkgroups(1);

        computePassEncoder.End();
        commandEncoder.Finish();
    }

    // bindGroups[0] is invalid with given dynamic offsets
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetPipeline(computePipeline);

        std::vector<uint32_t> dynamicOffsets = {0, 256};

        computePassEncoder.SetBindGroup(0, bindGroups[0], dynamicOffsets.size(),
                                        dynamicOffsets.data());
        computePassEncoder.DispatchWorkgroups(1);

        computePassEncoder.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // setting invalid dynamic offsets first and then resetting to valid dynamic offsets should be
    // valid
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetPipeline(computePipeline);

        std::vector<uint32_t> dynamicOffsetsValid = {0, 0};
        std::vector<uint32_t> dynamicOffsetsInvalid = {0, 256};

        computePassEncoder.SetBindGroup(0, bindGroups[0], dynamicOffsetsInvalid.size(),
                                        dynamicOffsetsInvalid.data());
        computePassEncoder.SetBindGroup(0, bindGroups[0], dynamicOffsetsValid.size(),
                                        dynamicOffsetsValid.data());
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

        std::vector<uint32_t> dynamicOffsets = {0, 0};

        renderPassEncoder.SetBindGroup(0, bindGroups[0], dynamicOffsets.size(),
                                       dynamicOffsets.data());
        renderPassEncoder.Draw(3);

        renderPassEncoder.End();
        commandEncoder.Finish();
    }

    // bindGroups[0] is invalid with given dynamic offsets
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.SetPipeline(renderPipeline);

        std::vector<uint32_t> dynamicOffsets = {0, 256};

        renderPassEncoder.SetBindGroup(0, bindGroups[0], dynamicOffsets.size(),
                                       dynamicOffsets.data());
        renderPassEncoder.Draw(3);

        renderPassEncoder.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // setting invalid dynamic offsets first and then resetting to valid dynamic offsets should be
    // valid
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.SetPipeline(renderPipeline);

        std::vector<uint32_t> dynamicOffsetsValid = {0, 0};
        std::vector<uint32_t> dynamicOffsetsInvalid = {0, 256};

        renderPassEncoder.SetBindGroup(0, bindGroups[0], dynamicOffsetsInvalid.size(),
                                       dynamicOffsetsInvalid.data());
        renderPassEncoder.SetBindGroup(0, bindGroups[0], dynamicOffsetsValid.size(),
                                       dynamicOffsetsValid.data());
        renderPassEncoder.Draw(3);

        renderPassEncoder.End();
        commandEncoder.Finish();
    }
}

}  // anonymous namespace
}  // namespace dawn
