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

#include <sstream>

#include "dawn/common/SystemUtils.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class ShaderPrintTest : public DawnTest {
  public:
    void SetUp() override {
        DawnTest::SetUp();

        // TODO(434213725): The current implementation of print for Metal uses MSL's os_log
        // function, which is only available on Apple Silicon and requires macOS 15.
#if DAWN_PLATFORM_IS(MACOS)
        int32_t majorVersion;
        GetMacOSVersion(&majorVersion);
        DAWN_SUPPRESS_TEST_IF(!IsApple() || majorVersion < 15);
#endif

        // Capture print output so that we can test it.
        device.SetLoggingCallback(
            [](wgpu::LoggingType type, wgpu::StringView message, void* userdata) {
                std::stringstream& output = *static_cast<std::stringstream*>(userdata);
                std::string_view view = {message.data, message.length};
                switch (type) {
                    case wgpu::LoggingType::Verbose:
                        DebugLog() << view;
                        break;
                    case wgpu::LoggingType::Warning:
                        WarningLog() << view;
                        break;
                    case wgpu::LoggingType::Error:
                        ErrorLog() << view;
                        break;
                    default:
                        output << view << "\n";
                        break;
                }
            },
            static_cast<void*>(&print_output));
    }

  protected:
    std::stringstream print_output;
};

TEST_P(ShaderPrintTest, RenderPipeline) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            print(42i);
            return vec4f(0, 0, 0.5, 0.5);
        }

        @fragment fn fs() -> @location(0) vec4f {
            print(vec4u(1, 2, 3, 4));
            return vec4(0, 0, 0, 0);
        }
    )");

    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.cFragment.module = module;
    pDesc.cFragment.targetCount = 1;
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline testPipeline = device.CreateRenderPipeline(&pDesc);

    // Run the test
    auto rp = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);

    pass.SetPipeline(testPipeline);
    pass.Draw(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    WaitForAllOperations();

    EXPECT_THAT(print_output.str(), testing::HasSubstr("[ vert vs:L3 instance=0, vertex=0 ] 42"));
    EXPECT_THAT(
        print_output.str(),
        testing::HasSubstr("[ frag fs:L8 position(0.500000, 0.500000, 1.000000) ] 1,2,3,4"));
}

TEST_P(ShaderPrintTest, ComputePipeline) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(2, 2)
        fn main(@builtin(workgroup_id) wgid: vec3u) {
            print(wgid);
        }
    )");

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.DispatchWorkgroups(2);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    WaitForAllOperations();

    EXPECT_THAT(print_output.str(),
                testing::HasSubstr("[ comp main:L4 global_invocation_id(0, 0, 0) ] 0,0,0"));
    EXPECT_THAT(print_output.str(),
                testing::HasSubstr("[ comp main:L4 global_invocation_id(1, 0, 0) ] 0,0,0"));
    EXPECT_THAT(print_output.str(),
                testing::HasSubstr("[ comp main:L4 global_invocation_id(0, 1, 0) ] 0,0,0"));
    EXPECT_THAT(print_output.str(),
                testing::HasSubstr("[ comp main:L4 global_invocation_id(1, 1, 0) ] 0,0,0"));

    EXPECT_THAT(print_output.str(),
                testing::HasSubstr("[ comp main:L4 global_invocation_id(2, 0, 0) ] 1,0,0"));
    EXPECT_THAT(print_output.str(),
                testing::HasSubstr("[ comp main:L4 global_invocation_id(3, 0, 0) ] 1,0,0"));
    EXPECT_THAT(print_output.str(),
                testing::HasSubstr("[ comp main:L4 global_invocation_id(2, 1, 0) ] 1,0,0"));
    EXPECT_THAT(print_output.str(),
                testing::HasSubstr("[ comp main:L4 global_invocation_id(3, 1, 0) ] 1,0,0"));
}

DAWN_INSTANTIATE_TEST(ShaderPrintTest, MetalBackend({"enable_shader_print"}));

}  // anonymous namespace
}  // namespace dawn
