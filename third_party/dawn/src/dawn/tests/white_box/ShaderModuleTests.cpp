// Copyright 2024 The Dawn & Tint Authors
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

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "dawn/native/ShaderModule.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn::native {
namespace {

constexpr wgpu::TextureFormat kRenderAttachmentFormat = wgpu::TextureFormat::RGBA8Unorm;

const char* kVertexShader = R"(
    @vertex fn main(
        @builtin(vertex_index) VertexIndex : u32
    ) -> @builtin(position) vec4f {
        var pos = array(
            vec2f( 0.0,  0.5),
            vec2f(-0.5, -0.5),
            vec2f( 0.5, -0.5)
        );
        return vec4f(pos[VertexIndex], 0.0, 1.0);
    })";

const char* kFragmentShader = R"(
    @fragment fn main() -> @location(0) vec4f {
        return vec4f(0.0, 1.0, 0.0, 1.0);
    })";

const char* kComputeShader = R"(
    struct SSBO {
        value : u32
    }
    @group(0) @binding(0) var<storage, read_write> ssbo : SSBO;

    @compute @workgroup_size(1) fn main() {
        ssbo.value = 1u;
    })";

struct CreatePipelineAsyncTask {
    wgpu::ComputePipeline computePipeline = nullptr;
    wgpu::RenderPipeline renderPipeline = nullptr;
    bool isCompleted = false;
    std::string message;
};

class ShaderModuleTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }

    bool SupportsCreatePipelineAsync() const {
        // Async pipeline creation is disabled with Metal AMD and Validation.
        // See crbug.com/dawn/1200.
        if (IsAMD() && IsMetal() && IsBackendValidationEnabled()) {
            return false;
        }

        return true;
    }

    wgpu::RenderPipeline DoCreateRenderPipeline(const wgpu::ShaderModule& vsModule,
                                                const wgpu::ShaderModule& fsModule) {
        utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
        renderPipelineDescriptor.layout = utils::MakeBasicPipelineLayout(device, nullptr);
        renderPipelineDescriptor.vertex.module = vsModule;
        renderPipelineDescriptor.cFragment.module = fsModule;
        renderPipelineDescriptor.cTargets[0].format = kRenderAttachmentFormat;
        renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

        return device.CreateRenderPipeline(&renderPipelineDescriptor);
    }

    void DoCreateRenderPipelineAsync(const wgpu::ShaderModule& vsModule,
                                     const wgpu::ShaderModule& fsModule) {
        utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
        renderPipelineDescriptor.layout = utils::MakeBasicPipelineLayout(device, nullptr);
        renderPipelineDescriptor.vertex.module = vsModule;
        renderPipelineDescriptor.cFragment.module = fsModule;
        renderPipelineDescriptor.cTargets[0].format = kRenderAttachmentFormat;
        renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

        device.CreateRenderPipelineAsync(
            &renderPipelineDescriptor, wgpu::CallbackMode::AllowProcessEvents,
            [this](wgpu::CreatePipelineAsyncStatus status, wgpu::RenderPipeline pipeline,
                   wgpu::StringView message) {
                EXPECT_EQ(wgpu::CreatePipelineAsyncStatus::Success, status);

                task.renderPipeline = std::move(pipeline);
                task.isCompleted = true;
                task.message = message;
            });
    }

    wgpu::ComputePipeline DoCreateComputePipeline(const wgpu::ShaderModule& module) {
        wgpu::ComputePipelineDescriptor csDesc;
        auto bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});
        csDesc.layout = utils::MakeBasicPipelineLayout(device, &bgl);
        csDesc.compute.module = module;
        return device.CreateComputePipeline(&csDesc);
    }

    void DoCreateComputePipelineAsync(const wgpu::ShaderModule& module) {
        wgpu::ComputePipelineDescriptor csDesc;
        auto bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});
        csDesc.layout = utils::MakeBasicPipelineLayout(device, &bgl);
        csDesc.compute.module = module;

        device.CreateComputePipelineAsync(
            &csDesc, wgpu::CallbackMode::AllowProcessEvents,
            [this](wgpu::CreatePipelineAsyncStatus status, wgpu::ComputePipeline pipeline,
                   wgpu::StringView message) {
                EXPECT_EQ(wgpu::CreatePipelineAsyncStatus::Success, status);
                task.computePipeline = std::move(pipeline);
                task.isCompleted = true;
                task.message = message;
            });
    }

    CreatePipelineAsyncTask task;
};

// Create ShaderModules from the same shader source code, and verify mTintProgram is released and
// re-created expectedly.
TEST_P(ShaderModuleTests, CachedShader) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, kVertexShader);

    // Add a internal reference
    Ref<ShaderModuleBase> shaderModule(FromAPI(module.Get()));
    EXPECT_TRUE(shaderModule.Get());

    EXPECT_TRUE(shaderModule->GetTintProgram());

    auto scopedUseTintProgram = shaderModule->UseTintProgram();

    // UseTintProgram() should keep the shader source code in memory.
    EXPECT_TRUE(shaderModule->GetNullableTintProgramForTesting());
    EXPECT_EQ(shaderModule->GetTintProgramRecreateCountForTesting(), 0);

    // Drop the external reference
    module = {};

    // The mTintProgram should be alive from UseTintProgram().
    EXPECT_TRUE(shaderModule->GetNullableTintProgramForTesting());
    EXPECT_EQ(shaderModule->GetTintProgramRecreateCountForTesting(), 0);

    // Drop the scopedUseTintProgram.
    scopedUseTintProgram = nullptr;

    // The mTintProgram should be released.
    EXPECT_FALSE(shaderModule->GetNullableTintProgramForTesting());
    EXPECT_EQ(shaderModule->GetTintProgramRecreateCountForTesting(), 0);

    // Create a ShaderModule with the same source code.
    module = utils::CreateShaderModule(device, kVertexShader);

    // The module should be from the cache.
    EXPECT_EQ(shaderModule.Get(), FromAPI(module.Get()));

    // The mTintProgram should be null, as it should be lazily recreated when calling
    // GetTintProgram().
    EXPECT_FALSE(shaderModule->GetNullableTintProgramForTesting());
    EXPECT_EQ(shaderModule->GetTintProgramRecreateCountForTesting(), 0);

    // Calling UseTintProgram() should add the ref count but not recreate mTintProgram, as it should
    // be lazily recreated when calling GetTintProgram.
    scopedUseTintProgram = shaderModule->UseTintProgram();
    EXPECT_FALSE(shaderModule->GetNullableTintProgramForTesting());
    EXPECT_EQ(shaderModule->GetTintProgramRecreateCountForTesting(), 0);

    // Calling GetTintProgram() should recreate the mTintProgram.
    EXPECT_TRUE(shaderModule->GetTintProgram());
    EXPECT_TRUE(shaderModule->GetNullableTintProgramForTesting());
    EXPECT_EQ(shaderModule->GetTintProgramRecreateCountForTesting(), 1);
}

// Check mTintProgram in ShaderModule is released after creation of a RenderPipeline is done.
TEST_P(ShaderModuleTests, CreateRenderPipeline) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, kVertexShader);
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, kFragmentShader);
    wgpu::RenderPipeline pipeline1 = DoCreateRenderPipeline(vsModule, fsModule);
    EXPECT_TRUE(pipeline1);

    Ref<ShaderModuleBase> vsShaderModule(FromAPI(vsModule.Get()));
    EXPECT_TRUE(vsShaderModule.Get());
    EXPECT_TRUE(vsShaderModule->GetNullableTintProgramForTesting());

    Ref<ShaderModuleBase> fsShaderModule(FromAPI(fsModule.Get()));
    EXPECT_TRUE(fsShaderModule.Get());
    EXPECT_TRUE(fsShaderModule->GetNullableTintProgramForTesting());

    // Drop the external reference.
    vsModule = {};
    fsModule = {};

    // mTintProgram should be released now since we dropped the last external references.
    EXPECT_FALSE(vsShaderModule->GetNullableTintProgramForTesting());
    EXPECT_FALSE(fsShaderModule->GetNullableTintProgramForTesting());

    // Create a pipeline with the same shader source code
    vsModule = utils::CreateShaderModule(device, kVertexShader);
    fsModule = utils::CreateShaderModule(device, kFragmentShader);
    EXPECT_EQ(vsShaderModule, FromAPI(vsModule.Get()));
    EXPECT_EQ(fsShaderModule, FromAPI(fsModule.Get()));

    // Should return the same pipeline.
    wgpu::RenderPipeline pipeline2 = DoCreateRenderPipeline(vsModule, fsModule);
    EXPECT_TRUE(pipeline2);
    EXPECT_EQ(pipeline1.Get(), pipeline2.Get());

    // mTintProgram should be released already since we got a cached pipeline.
    EXPECT_FALSE(vsShaderModule->GetNullableTintProgramForTesting());
    EXPECT_FALSE(fsShaderModule->GetNullableTintProgramForTesting());
}

// Check mTintProgram in ShaderModule is released after async creation of a RenderPipeline is done.
TEST_P(ShaderModuleTests, CreateRenderPipelineAsync) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, kVertexShader);
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, kFragmentShader);
    DoCreateRenderPipelineAsync(vsModule, fsModule);

    Ref<ShaderModuleBase> vsShaderModule(FromAPI(vsModule.Get()));
    EXPECT_TRUE(vsShaderModule.Get());
    EXPECT_TRUE(vsShaderModule->GetNullableTintProgramForTesting());

    Ref<ShaderModuleBase> fsShaderModule(FromAPI(fsModule.Get()));
    EXPECT_TRUE(fsShaderModule.Get());
    EXPECT_TRUE(fsShaderModule->GetNullableTintProgramForTesting());

    // Drop the external reference.
    vsModule = {};
    fsModule = {};

    // Wait until pipeline creation is done.
    do {
        WaitABit();
    } while (!task.isCompleted);
    EXPECT_TRUE(task.renderPipeline);

    // mTintProgram should be released now since we dropped the last external references.
    EXPECT_FALSE(vsShaderModule->GetNullableTintProgramForTesting());
    EXPECT_FALSE(fsShaderModule->GetNullableTintProgramForTesting());
}

// Check mTintProgram in ShaderModule is released after creation of a ComputePipeline is done.
TEST_P(ShaderModuleTests, CreateComputePipeline) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, kComputeShader);
    wgpu::ComputePipeline pipeline1 = DoCreateComputePipeline(module);
    EXPECT_TRUE(pipeline1);

    Ref<ShaderModuleBase> shaderModule(FromAPI(module.Get()));
    EXPECT_TRUE(shaderModule.Get());
    EXPECT_TRUE(shaderModule->GetNullableTintProgramForTesting());

    // Drop the external reference.
    module = {};

    // mTintProgram should be released now since we dropped the last external references.
    EXPECT_FALSE(shaderModule->GetNullableTintProgramForTesting());

    // Create a pipeline with the same shader source code
    module = utils::CreateShaderModule(device, kComputeShader);
    EXPECT_EQ(shaderModule, FromAPI(module.Get()));

    // Should return the same pipeline.
    wgpu::ComputePipeline pipeline2 = DoCreateComputePipeline(module);
    EXPECT_TRUE(pipeline2);
    EXPECT_EQ(pipeline1.Get(), pipeline2.Get());

    // mTintProgram should be released already since we got a cached pipeline.
    EXPECT_FALSE(shaderModule->GetNullableTintProgramForTesting());
}

// Check mTintProgram in ShaderModule is released after async creation of a ComputePipeline is done.
TEST_P(ShaderModuleTests, CreateComputePipelineAsync) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, kComputeShader);
    DoCreateComputePipelineAsync(module);

    Ref<ShaderModuleBase> shaderModule(FromAPI(module.Get()));
    EXPECT_TRUE(shaderModule.Get());
    EXPECT_TRUE(shaderModule->GetNullableTintProgramForTesting());

    // Drop the external reference.
    module = {};

    // Wait until pipeline creation is done.
    do {
        WaitABit();
    } while (!task.isCompleted);
    EXPECT_TRUE(task.computePipeline);

    // mTintProgram should be released now since we dropped the last external references.
    EXPECT_FALSE(shaderModule->GetNullableTintProgramForTesting());
}

DAWN_INSTANTIATE_TEST(ShaderModuleTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend());

}  // anonymous namespace
}  // namespace dawn::native
