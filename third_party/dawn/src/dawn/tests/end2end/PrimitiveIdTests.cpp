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

#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr uint32_t kRTSize = 16;
constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA8Unorm;

constexpr wgpu::FeatureName kPrimitiveIdFeature =
    wgpu::FeatureName::ChromiumExperimentalPrimitiveId;

using RequirePrimitiveIdFeature = bool;
DAWN_TEST_PARAM_STRUCT(PrimitiveIdTestsParams, RequirePrimitiveIdFeature);

class PrimitiveIdTests : public DawnTestWithParams<PrimitiveIdTestsParams> {
  public:
    wgpu::Texture CreateDefault2DTexture() {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = kRTSize;
        descriptor.size.height = kRTSize;
        descriptor.size.depthOrArrayLayers = 1;
        descriptor.sampleCount = 1;
        descriptor.format = kFormat;
        descriptor.mipLevelCount = 1;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        return device.CreateTexture(&descriptor);
    }

  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        mIsPrimitiveIdSupportedOnAdapter = SupportsFeatures({kPrimitiveIdFeature});
        if (!mIsPrimitiveIdSupportedOnAdapter) {
            return {};
        }

        if (GetParam().mRequirePrimitiveIdFeature) {
            return {kPrimitiveIdFeature};
        }

        return {};
    }

    bool IsPrimitiveIdSupportedOnAdapter() const { return mIsPrimitiveIdSupportedOnAdapter; }

  private:
    bool mIsPrimitiveIdSupportedOnAdapter = false;
};

// Test simple primitive ID within shader with enable directive. The result should be as expected if
// the device enables the extension, otherwise a shader creation error should be caught.
TEST_P(PrimitiveIdTests, BasicPrimitiveIdFeaturesTest) {
    // Skip if device doesn't support the extension.
    DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(kPrimitiveIdFeature));

    const char* shader = R"(
enable chromium_experimental_primitive_id;

@vertex
fn VSMain(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
    var pos = array(
        vec2f(-1, -1), vec2f(1, -1), vec2f(-1, 1),
        vec2f(1, 1), vec2f(1, -1), vec2f(-1, 1));

    return vec4f(pos[VertexIndex % 6], 0.0, 1.0);
}

var<private> colorId : array<vec3f, 5> = array<vec3f, 5>(
    vec3f(1, 0, 0),
    vec3f(0, 1, 0),
    vec3f(0, 0, 1),
    vec3f(1, 1, 0),
    vec3f(1, 1, 1)
);

@fragment
fn FSMain(@builtin(primitive_id) pid : u32) -> @location(0) vec4f {
    // Select a color based on the primitive ID
    return vec4f(colorId[pid%5], 1.0);
})";

    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader);

    // Create render pipeline.
    wgpu::RenderPipeline pipeline;
    {
        utils::ComboRenderPipelineDescriptor descriptor;

        descriptor.vertex.module = shaderModule;

        descriptor.cFragment.module = shaderModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        descriptor.cTargets[0].format = kFormat;

        pipeline = device.CreateRenderPipeline(&descriptor);
    }

    wgpu::Texture renderTarget = CreateDefault2DTexture();

    auto drawPrimitives = [&](uint32_t count) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        {
            // In the render pass we clear renderTarget to black, then draw a colored triangles
            // based on the primitive ID to the top left and bottom right of the rnederTarget.
            utils::ComboRenderPassDescriptor renderPass({renderTarget.CreateView()});
            renderPass.cColorAttachments[0].clearValue = {0.0f, 0.0f, 0.0f, 1.0f};

            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.SetPipeline(pipeline);
            pass.Draw(count * 3);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    };

    // Draw N primitives and ensure that the color written corresponds to the given primitive ID.
    drawPrimitives(2);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderTarget, 1, kRTSize - 1);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderTarget, kRTSize - 1, 1);

    drawPrimitives(4);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlue, renderTarget, 1, kRTSize - 1);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kYellow, renderTarget, kRTSize - 1, 1);

    drawPrimitives(6);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kWhite, renderTarget, 1, kRTSize - 1);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderTarget, kRTSize - 1, 1);
}

// DawnTestBase::CreateDeviceImpl always enables allow_unsafe_apis toggle.
DAWN_INSTANTIATE_TEST_P(PrimitiveIdTests,
                        {
                            D3D11Backend(),
                            D3D12Backend(),
                            VulkanBackend(),
                            MetalBackend(),
                            OpenGLBackend(),
                            OpenGLESBackend(),
                        },
                        {true, false});

}  // anonymous namespace
}  // namespace dawn
