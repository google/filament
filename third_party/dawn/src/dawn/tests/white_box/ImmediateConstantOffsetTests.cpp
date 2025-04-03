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

#include "dawn/common/Assert.h"
#include "dawn/native/ComputePipeline.h"
#include "dawn/native/ImmediateConstantsLayout.h"
#include "dawn/native/ImmediateConstantsTracker.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn::native {
namespace {
class ImmediateConstantOffsetTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }

    wgpu::RenderPipeline MakeTestRenderPipelineWithClampingFragDepth() {
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = utils::CreateShaderModule(device, R"(
            @vertex fn main() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 0.0);
            }
        )");
        desc.vertex.entryPoint = "main";
        desc.cFragment.module = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @builtin(frag_depth) f32 {
                return 1.0;
            }
        )");
        desc.cFragment.entryPoint = "main";
        desc.cFragment.targetCount = 0;
        wgpu::DepthStencilState* dsState = desc.EnableDepthStencil();
        dsState->depthWriteEnabled = wgpu::OptionalBool::True;
        dsState->depthCompare = wgpu::CompareFunction::Always;

        return device.CreateRenderPipeline(&desc);
    }
};

// Test pipeline change reset dirty bits and update tracked pipeline constants mask.
// Use Compute pipeline to cover this common path.
TEST_P(ImmediateConstantOffsetTest, ClampFragDepth) {
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() || IsOpenGL() || IsMetal() || IsD3D11() || IsD3D12());
    ImmediateConstantMask expectedImmediateConstantMask = ImmediateConstantMask(0);
    // Hard coded bits and index.
    expectedImmediateConstantMask |= (1u << 4u);
    expectedImmediateConstantMask |= (1u << 5u);

    // Check dirty bits are set correctly.
    EXPECT_TRUE(FromAPI(MakeTestRenderPipelineWithClampingFragDepth().Get())->GetImmediateMask() ==
                expectedImmediateConstantMask);
}

DAWN_INSTANTIATE_TEST(ImmediateConstantOffsetTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn::native
