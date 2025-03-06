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

#include <array>
#include <cstring>
#include <random>
#include <vector>

#include "dawn/samples/SampleUtils.h"

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

static const uint32_t kNumParticles = 1024;

struct Particle {
    std::array<float, 2> pos;
    std::array<float, 2> vel;
};

struct SimParams {
    float deltaT;
    float rule1Distance;
    float rule2Distance;
    float rule3Distance;
    float rule1Scale;
    float rule2Scale;
    float rule3Scale;
    int particleCount;
};

class ComputeBoidsSample : public SampleBase {
  public:
    using SampleBase::SampleBase;

  private:
    void initBuffers() {
        std::array<std::array<float, 2>, 3> model = {{
            {-0.01, -0.02},
            {0.01, -0.02},
            {0.00, 0.02},
        }};
        modelBuffer = dawn::utils::CreateBufferFromData(device, &model, sizeof(model),
                                                        wgpu::BufferUsage::Vertex);

        SimParams params = {0.04f, 0.1f, 0.025f, 0.025f, 0.02f, 0.05f, 0.005f, kNumParticles};
        updateParams = dawn::utils::CreateBufferFromData(device, &params, sizeof(params),
                                                         wgpu::BufferUsage::Uniform);

        std::vector<Particle> initialParticles(kNumParticles);
        {
            std::mt19937 generator;
            std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
            for (auto& p : initialParticles) {
                p.pos = {dist(generator), dist(generator)};
                p.vel = {dist(generator) * 0.1f, dist(generator) * 0.1f};
            }
        }

        for (size_t i = 0; i < 2; i++) {
            wgpu::BufferDescriptor descriptor;
            descriptor.size = sizeof(Particle) * kNumParticles;
            descriptor.usage =
                wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Storage;
            particleBuffers[i] = device.CreateBuffer(&descriptor);

            queue.WriteBuffer(particleBuffers[i], 0,
                              reinterpret_cast<uint8_t*>(initialParticles.data()),
                              sizeof(Particle) * kNumParticles);
        }
    }

    void initRender() {
        wgpu::ShaderModule vsModule = dawn::utils::CreateShaderModule(device, R"(
        struct VertexIn {
            @location(0) a_particlePos : vec2f,
            @location(1) a_particleVel : vec2f,
            @location(2) a_pos : vec2f,
        };

        @vertex
        fn main(input : VertexIn) -> @builtin(position) vec4f {
            var angle : f32 = -atan2(input.a_particleVel.x, input.a_particleVel.y);
            var pos : vec2f = vec2f(
                (input.a_pos.x * cos(angle)) - (input.a_pos.y * sin(angle)),
                (input.a_pos.x * sin(angle)) + (input.a_pos.y * cos(angle)));
            return vec4f(pos + input.a_particlePos, 0.0, 1.0);
        }
    )");

        wgpu::ShaderModule fsModule = dawn::utils::CreateShaderModule(device, R"(
        @fragment
        fn main() -> @location(0) vec4f {
            return vec4f(1.0, 1.0, 1.0, 1.0);
        }
    )");

        dawn::utils::ComboRenderPipelineDescriptor descriptor;

        descriptor.vertex.module = vsModule;
        descriptor.vertex.bufferCount = 2;
        descriptor.cBuffers[0].arrayStride = sizeof(Particle);
        descriptor.cBuffers[0].stepMode = wgpu::VertexStepMode::Instance;
        descriptor.cBuffers[0].attributeCount = 2;
        descriptor.cAttributes[0].offset = offsetof(Particle, pos);
        descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x2;
        descriptor.cAttributes[1].shaderLocation = 1;
        descriptor.cAttributes[1].offset = offsetof(Particle, vel);
        descriptor.cAttributes[1].format = wgpu::VertexFormat::Float32x2;
        descriptor.cBuffers[1].arrayStride = 2 * sizeof(float);
        descriptor.cBuffers[1].attributeCount = 1;
        descriptor.cBuffers[1].attributes = &descriptor.cAttributes[2];
        descriptor.cAttributes[2].shaderLocation = 2;
        descriptor.cAttributes[2].format = wgpu::VertexFormat::Float32x2;

        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = GetPreferredSurfaceTextureFormat();

        renderPipeline = device.CreateRenderPipeline(&descriptor);
    }

    void initSim() {
        wgpu::ShaderModule module = dawn::utils::CreateShaderModule(device, R"(
        struct Particle {
            pos : vec2f,
            vel : vec2f,
        };
        struct SimParams {
            deltaT : f32,
            rule1Distance : f32,
            rule2Distance : f32,
            rule3Distance : f32,
            rule1Scale : f32,
            rule2Scale : f32,
            rule3Scale : f32,
            particleCount : u32,
        };
        struct Particles {
            particles : array<Particle>,
        };
        @binding(0) @group(0) var<uniform> params : SimParams;
        @binding(1) @group(0) var<storage, read_write> particlesA : Particles;
        @binding(2) @group(0) var<storage, read_write> particlesB : Particles;

        // https://github.com/austinEng/Project6-Vulkan-Flocking/blob/master/data/shaders/computeparticles/particle.comp
        @compute @workgroup_size(64)
        fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
            var index : u32 = GlobalInvocationID.x;
            if (index >= params.particleCount) {
                return;
            }
            var vPos : vec2f = particlesA.particles[index].pos;
            var vVel : vec2f = particlesA.particles[index].vel;
            var cMass : vec2f = vec2f(0.0, 0.0);
            var cVel : vec2f = vec2f(0.0, 0.0);
            var colVel : vec2f = vec2f(0.0, 0.0);
            var cMassCount : u32 = 0u;
            var cVelCount : u32 = 0u;
            var pos : vec2f;
            var vel : vec2f;

            for (var i : u32 = 0u; i < params.particleCount; i = i + 1u) {
                if (i == index) {
                    continue;
                }

                pos = particlesA.particles[i].pos.xy;
                vel = particlesA.particles[i].vel.xy;
                if (distance(pos, vPos) < params.rule1Distance) {
                    cMass = cMass + pos;
                    cMassCount = cMassCount + 1u;
                }
                if (distance(pos, vPos) < params.rule2Distance) {
                    colVel = colVel - (pos - vPos);
                }
                if (distance(pos, vPos) < params.rule3Distance) {
                    cVel = cVel + vel;
                    cVelCount = cVelCount + 1u;
                }
            }

            if (cMassCount > 0u) {
                cMass = (cMass / vec2f(f32(cMassCount), f32(cMassCount))) - vPos;
            }

            if (cVelCount > 0u) {
                cVel = cVel / vec2f(f32(cVelCount), f32(cVelCount));
            }
            vVel = vVel + (cMass * params.rule1Scale) + (colVel * params.rule2Scale) +
                (cVel * params.rule3Scale);

            // clamp velocity for a more pleasing simulation
            vVel = normalize(vVel) * clamp(length(vVel), 0.0, 0.1);
            // kinematic update
            vPos = vPos + (vVel * params.deltaT);

            // Wrap around boundary
            if (vPos.x < -1.0) {
                vPos.x = 1.0;
            }
            if (vPos.x > 1.0) {
                vPos.x = -1.0;
            }
            if (vPos.y < -1.0) {
                vPos.y = 1.0;
            }
            if (vPos.y > 1.0) {
                vPos.y = -1.0;
            }

            // Write back
            particlesB.particles[index].pos = vPos;
            particlesB.particles[index].vel = vVel;
            return;
        }
    )");

        auto bgl = dawn::utils::MakeBindGroupLayout(
            device, {
                        {0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform},
                        {1, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage},
                        {2, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage},
                    });

        wgpu::PipelineLayout pl = dawn::utils::MakeBasicPipelineLayout(device, &bgl);

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.layout = pl;
        csDesc.compute.module = module;
        csDesc.compute.entryPoint = "main";
        updatePipeline = device.CreateComputePipeline(&csDesc);

        for (uint32_t i = 0; i < 2; ++i) {
            updateBGs[i] = dawn::utils::MakeBindGroup(
                device, bgl,
                {
                    {0, updateParams, 0, sizeof(SimParams)},
                    {1, particleBuffers[i], 0, kNumParticles * sizeof(Particle)},
                    {2, particleBuffers[(i + 1) % 2], 0, kNumParticles * sizeof(Particle)},
                });
        }
    }

    wgpu::CommandBuffer createCommandBuffer(const wgpu::TextureView backbufferView, size_t i) {
        auto& bufferDst = particleBuffers[(i + 1) % 2];
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        {
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(updatePipeline);
            pass.SetBindGroup(0, updateBGs[i]);
            pass.DispatchWorkgroups(kNumParticles / 64);
            pass.End();
        }

        {
            dawn::utils::ComboRenderPassDescriptor renderPass({backbufferView});
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.SetPipeline(renderPipeline);
            pass.SetVertexBuffer(0, bufferDst);
            pass.SetVertexBuffer(1, modelBuffer);
            pass.Draw(3, kNumParticles);
            pass.End();
        }

        return encoder.Finish();
    }

    bool SetupImpl() override {
        initBuffers();
        initRender();
        initSim();
        return true;
    }

    void FrameImpl() override {
        wgpu::SurfaceTexture surfaceTexture;
        surface.GetCurrentTexture(&surfaceTexture);
        wgpu::CommandBuffer commandBuffer =
            createCommandBuffer(surfaceTexture.texture.CreateView(), pingpong);
        queue.Submit(1, &commandBuffer);
        pingpong = (pingpong + 1) % 2;
    }

    wgpu::Buffer modelBuffer;
    std::array<wgpu::Buffer, 2> particleBuffers;

    wgpu::RenderPipeline renderPipeline;

    wgpu::Buffer updateParams;
    wgpu::ComputePipeline updatePipeline;
    std::array<wgpu::BindGroup, 2> updateBGs;

    size_t pingpong = 0;
};

int main(int argc, const char* argv[]) {
    if (!InitSample(argc, argv)) {
        return 1;
    }

    ComputeBoidsSample* sample = new ComputeBoidsSample();
    sample->Run(16000);
}
