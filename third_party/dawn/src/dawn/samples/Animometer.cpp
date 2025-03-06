// Copyright 2017 The Dawn & Tint Authors
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

#include <cstdio>
#include <cstdlib>
#include <vector>

#include "dawn/samples/SampleUtils.h"

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/Timer.h"
#include "dawn/utils/WGPUHelpers.h"

constexpr size_t kNumTriangles = 10000;

float RandomFloat(float min, float max) {
    // NOLINTNEXTLINE(runtime/threadsafe_fn)
    float zeroOne = rand() / static_cast<float>(RAND_MAX);
    return zeroOne * (max - min) + min;
}

// Aligned as minUniformBufferOffsetAlignment
struct alignas(256) ShaderData {
    float scale;
    float time;
    float offsetX;
    float offsetY;
    float scalar;
    float scalarOffset;
};

static std::vector<ShaderData> shaderData;

class AnimometerSample : public SampleBase {
  public:
    using SampleBase::SampleBase;

  private:
    bool SetupImpl() override {
        wgpu::ShaderModule vsModule = dawn::utils::CreateShaderModule(device, R"(
        struct Constants {
            scale : f32,
            time : f32,
            offsetX : f32,
            offsetY : f32,
            scalar : f32,
            scalarOffset : f32,
        };
        @group(0) @binding(0) var<uniform> c : Constants;

        struct VertexOut {
            @location(0) v_color : vec4f,
            @builtin(position) Position : vec4f,
        };

        @vertex fn main(@builtin(vertex_index) VertexIndex : u32) -> VertexOut {
            var positions : array<vec4f, 3> = array(
                vec4f( 0.0,  0.1, 0.0, 1.0),
                vec4f(-0.1, -0.1, 0.0, 1.0),
                vec4f( 0.1, -0.1, 0.0, 1.0)
            );

            var colors : array<vec4f, 3> = array(
                vec4f(1.0, 0.0, 0.0, 1.0),
                vec4f(0.0, 1.0, 0.0, 1.0),
                vec4f(0.0, 0.0, 1.0, 1.0)
            );

            var position : vec4f = positions[VertexIndex];
            var color : vec4f = colors[VertexIndex];

            // TODO(dawn:572): Revisit once modf has been reworked in WGSL.
            var fade : f32 = c.scalarOffset + c.time * c.scalar / 10.0;
            fade = fade - floor(fade);
            if (fade < 0.5) {
                fade = fade * 2.0;
            } else {
                fade = (1.0 - fade) * 2.0;
            }

            var xpos : f32 = position.x * c.scale;
            var ypos : f32 = position.y * c.scale;
            let angle : f32 = 3.14159 * 2.0 * fade;
            let xrot : f32 = xpos * cos(angle) - ypos * sin(angle);
            let yrot : f32 = xpos * sin(angle) + ypos * cos(angle);
            xpos = xrot + c.offsetX;
            ypos = yrot + c.offsetY;

            var output : VertexOut;
            output.v_color = vec4f(fade, 1.0 - fade, 0.0, 1.0) + color;
            output.Position = vec4f(xpos, ypos, 0.0, 1.0);
            return output;
        })");

        wgpu::ShaderModule fsModule = dawn::utils::CreateShaderModule(device, R"(
        @fragment fn main(@location(0) v_color : vec4f) -> @location(0) vec4f {
            return v_color;
        })");

        wgpu::BindGroupLayout bgl = dawn::utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform, true}});

        dawn::utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.layout = dawn::utils::MakeBasicPipelineLayout(device, &bgl);
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = GetPreferredSurfaceTextureFormat();

        pipeline = device.CreateRenderPipeline(&descriptor);

        shaderData.resize(kNumTriangles);
        for (auto& data : shaderData) {
            data.scale = RandomFloat(0.2f, 0.4f);
            data.time = 0.0;
            data.offsetX = RandomFloat(-0.9f, 0.9f);
            data.offsetY = RandomFloat(-0.9f, 0.9f);
            data.scalar = RandomFloat(0.5f, 2.0f);
            data.scalarOffset = RandomFloat(0.0f, 10.0f);
        }

        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = kNumTriangles * sizeof(ShaderData);
        bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
        ubo = device.CreateBuffer(&bufferDesc);

        bindGroup = dawn::utils::MakeBindGroup(device, bgl, {{0, ubo, 0, sizeof(ShaderData)}});

        timer->Start();
        return true;
    }

    void FrameImpl() override {
        frameCount++;
        for (auto& data : shaderData) {
            data.time = frameCount / 60.0f;
        }
        queue.WriteBuffer(ubo, 0, shaderData.data(), kNumTriangles * sizeof(ShaderData));

        wgpu::SurfaceTexture surfaceTexture;
        surface.GetCurrentTexture(&surfaceTexture);
        dawn::utils::ComboRenderPassDescriptor renderPass({surfaceTexture.texture.CreateView()});
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.SetPipeline(pipeline);

            for (size_t i = 0; i < kNumTriangles; i++) {
                uint32_t offset = i * sizeof(ShaderData);
                pass.SetBindGroup(0, bindGroup, 1, &offset);
                pass.Draw(3);
            }

            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        if (frameCount % 60 == 0) {
            printf("FPS: %lf\n", 60.0 / timer->GetElapsedTime());
            timer->Start();
        }
    }

    std::vector<ShaderData> shaderData;
    wgpu::RenderPipeline pipeline;
    wgpu::BindGroup bindGroup;
    wgpu::Buffer ubo;
    int frameCount = 0;
    dawn::utils::Timer* timer = dawn::utils::CreateTimer();
};

int main(int argc, const char* argv[]) {
    if (!InitSample(argc, argv)) {
        return 1;
    }

    AnimometerSample* sample = new AnimometerSample();
    sample->Run(0);
}
