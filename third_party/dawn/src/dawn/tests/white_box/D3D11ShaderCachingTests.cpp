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

#include "dawn/native/d3d11/ComputePipelineD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/RenderPipelineD3D11.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class D3D11ShaderCachingTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire() || !IsD3D11());
    }
};

// Test that creating render pipelines with the same shader module will reuse the same D3D11 shader
// object.
TEST_P(D3D11ShaderCachingTests, RenderPipelineShaderReuse) {
    wgpu::ShaderModule vsModule1 = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule1 = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(1.0, 0.0, 0.0, 1.0);
        })");

    // Create a second fragment shader module to make the pipeline descriptor unique.
    wgpu::ShaderModule fsModule2 = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0); // Different color
        })");

    utils::ComboRenderPipelineDescriptor desc1;
    desc1.vertex.module = vsModule1;
    desc1.cFragment.module = fsModule1;
    wgpu::RenderPipeline pipeline1 = device.CreateRenderPipeline(&desc1);

    utils::ComboRenderPipelineDescriptor desc2;
    desc2.vertex.module = vsModule1;     // Same VS
    desc2.cFragment.module = fsModule2;  // Different FS
    wgpu::RenderPipeline pipeline2 = device.CreateRenderPipeline(&desc2);

    native::d3d11::RenderPipeline* d3d11Pipeline1 =
        native::d3d11::ToBackend(native::FromAPI(pipeline1.Get()));
    native::d3d11::RenderPipeline* d3d11Pipeline2 =
        native::d3d11::ToBackend(native::FromAPI(pipeline2.Get()));

    EXPECT_NE(d3d11Pipeline1, d3d11Pipeline2);
    // Vertex shader should be reused.
    EXPECT_EQ(d3d11Pipeline1->GetD3D11VertexShaderForTesting(),
              d3d11Pipeline2->GetD3D11VertexShaderForTesting());
    // Fragment shader should be different.
    EXPECT_NE(d3d11Pipeline1->GetD3D11PixelShaderForTesting(),
              d3d11Pipeline2->GetD3D11PixelShaderForTesting());
}

// Test that creating compute pipelines with the same shader module will reuse the same D3D11 shader
// object.
TEST_P(D3D11ShaderCachingTests, ComputePipelineShaderReuse) {
    // Use different entry points from the same shader module to bypass the frontend pipeline cache
    // and test backend shader object reuse.
    wgpu::ShaderModule csModule1 = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1) fn main1() {
        }
        @compute @workgroup_size(1) fn main2() {
        })");

    wgpu::ComputePipelineDescriptor desc1;
    desc1.compute.module = csModule1;
    desc1.compute.entryPoint = "main1";
    wgpu::ComputePipeline pipeline1 = device.CreateComputePipeline(&desc1);

    wgpu::ComputePipelineDescriptor desc2;
    desc2.compute.module = csModule1;
    desc2.compute.entryPoint = "main2";
    wgpu::ComputePipeline pipeline2 = device.CreateComputePipeline(&desc2);

    native::d3d11::ComputePipeline* d3d11Pipeline1 =
        native::d3d11::ToBackend(native::FromAPI(pipeline1.Get()));
    native::d3d11::ComputePipeline* d3d11Pipeline2 =
        native::d3d11::ToBackend(native::FromAPI(pipeline2.Get()));

    EXPECT_NE(d3d11Pipeline1, d3d11Pipeline2);
    // The backend shader should be reused because the HLSL generated for main1 and main2 is
    // identical.
    EXPECT_EQ(d3d11Pipeline1->GetD3D11ComputeShaderForTesting(),
              d3d11Pipeline2->GetD3D11ComputeShaderForTesting());

    // Create a third pipeline with a different compute shader, and check that the D3D11 shader
    // object is not reused.
    wgpu::ShaderModule csModule2 = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(2) fn main() { // Different workgroup size
        })");
    wgpu::ComputePipelineDescriptor desc3;
    desc3.compute.module = csModule2;
    wgpu::ComputePipeline pipeline3 = device.CreateComputePipeline(&desc3);
    native::d3d11::ComputePipeline* d3d11Pipeline3 =
        native::d3d11::ToBackend(native::FromAPI(pipeline3.Get()));

    EXPECT_NE(d3d11Pipeline1->GetD3D11ComputeShaderForTesting(),
              d3d11Pipeline3->GetD3D11ComputeShaderForTesting());
}

DAWN_INSTANTIATE_TEST(D3D11ShaderCachingTests, D3D11Backend());

}  // anonymous namespace
}  // namespace dawn
