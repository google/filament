// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/tests/unittests/validation/ValidationTest.h"

#include "dawn/native/BindGroupLayout.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using testing::Not;

class GetBindGroupLayoutTests : public ValidationTest {
  protected:
    wgpu::RenderPipeline RenderPipelineFromFragmentShader(const char* shader) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
                @vertex fn main() -> @builtin(position) vec4f {
                    return vec4f();
                })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, shader);

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.layout = nullptr;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

        return device.CreateRenderPipeline(&descriptor);
    }
};

// Test that GetBindGroupLayout returns the same object for the same index
// and for matching layouts.
TEST_F(GetBindGroupLayoutTests, EquivalentBGLs) {
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        struct S {
            pos : vec4f
        }
        @group(0) @binding(0) var<uniform> uniform0 : S;
        @group(1) @binding(0) var<uniform> uniform1 : S;

        @vertex fn main() -> @builtin(position) vec4f {
            var pos : vec4f = uniform0.pos;
            pos = uniform1.pos;
            return vec4f();
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        struct S2 {
            pos : vec4f
        }
        @group(2) @binding(0) var<uniform> uniform2 : S2;

        struct S3 {
            pos : mat4x4<f32>
        }
        @group(3) @binding(0) var<storage, read_write> storage3 : S3;

        @fragment fn main() {
            var pos_u : vec4f = uniform2.pos;
            var pos_s : mat4x4<f32> = storage3.pos;
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // A fully equivalent layout is returned for the same index.
    EXPECT_THAT(pipeline.GetBindGroupLayout(0), BindGroupLayoutEq(pipeline.GetBindGroupLayout(0)));

    // Matching bind group layouts at different indices are fully equivalent.
    EXPECT_THAT(pipeline.GetBindGroupLayout(0), BindGroupLayoutEq(pipeline.GetBindGroupLayout(1)));

    // BGLs with different bindings types are different.
    EXPECT_THAT(pipeline.GetBindGroupLayout(2),
                Not(BindGroupLayoutEq(pipeline.GetBindGroupLayout(3))));

    // BGLs with different visibilities are different.
    EXPECT_THAT(pipeline.GetBindGroupLayout(0),
                Not(BindGroupLayoutEq(pipeline.GetBindGroupLayout(2))));
}

// Test that default BindGroupLayouts cannot be used in the creation of a new PipelineLayout
TEST_F(GetBindGroupLayoutTests, DefaultBindGroupLayoutPipelineCompatibility) {
    wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
        struct S {
            pos : vec4f
        }
        @group(0) @binding(0) var<uniform> uniforms : S;

        @fragment fn main() {
            var pos : vec4f = uniforms.pos;
        })");

    ASSERT_DEVICE_ERROR(utils::MakePipelineLayout(device, {pipeline.GetBindGroupLayout(0)}));
}

// Bind groups created from different default pipelines' GetBindGroupLayout aren't compatible, even
// if they appear to be the same.
TEST_F(GetBindGroupLayoutTests, DefaultBindGroupLayoutDifferentPipelines) {
    wgpu::RenderPipeline pipeline1 = RenderPipelineFromFragmentShader(R"(
        struct S {
            pos : vec4f
        }
        @group(0) @binding(0) var<uniform> uniforms : S;

        @fragment fn main() {
            var pos : vec4f = uniforms.pos + 1;
        })");
    wgpu::RenderPipeline pipeline2 = RenderPipelineFromFragmentShader(R"(
        struct S {
            pos : vec4f
        }
        @group(0) @binding(0) var<uniform> uniforms : S;

        @fragment fn main() {
            var pos : vec4f = uniforms.pos + 2;
        })");

    constexpr uint64_t kBufferSize = 16u;
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = kBufferSize;
    bufferDescriptor.usage = wgpu::BufferUsage::Uniform;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::BindGroup bg = utils::MakeBindGroup(device, pipeline2.GetBindGroupLayout(0),
                                              {{0, buffer, 0, kBufferSize}});

    constexpr uint32_t kTextureSize = 4u;
    utils::BasicRenderPass renderPass =
        utils::CreateBasicRenderPass(device, kTextureSize, kTextureSize);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder rp = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    rp.SetPipeline(pipeline1);
    rp.SetBindGroup(0, bg);
    rp.Draw(1);
    rp.End();
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Bind groups created from the exact same default pipelines' GetBindGroupLayout aren't compatible.
TEST_F(GetBindGroupLayoutTests, DefaultBindGroupLayoutSamePipelines) {
    wgpu::RenderPipeline pipeline1 = RenderPipelineFromFragmentShader(R"(
        struct S {
            pos : vec4f
        }
        @group(0) @binding(0) var<uniform> uniforms : S;

        @fragment fn main() {
            var pos : vec4f = uniforms.pos + 1;
        })");
    wgpu::RenderPipeline pipeline2 = RenderPipelineFromFragmentShader(R"(
        struct S {
            pos : vec4f
        }
        @group(0) @binding(0) var<uniform> uniforms : S;

        @fragment fn main() {
            var pos : vec4f = uniforms.pos + 1;
        })");

    constexpr uint64_t kBufferSize = 16u;
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = kBufferSize;
    bufferDescriptor.usage = wgpu::BufferUsage::Uniform;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::BindGroup bg = utils::MakeBindGroup(device, pipeline2.GetBindGroupLayout(0),
                                              {{0, buffer, 0, kBufferSize}});

    constexpr uint32_t kTextureSize = 4u;
    utils::BasicRenderPass renderPass =
        utils::CreateBasicRenderPass(device, kTextureSize, kTextureSize);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder rp = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    rp.SetPipeline(pipeline1);
    rp.SetBindGroup(0, bg);
    rp.Draw(1);
    rp.End();
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Test that getBindGroupLayout defaults are correct
// - shader stage visibility is the stage that adds the binding.
// - dynamic offsets is false
TEST_F(GetBindGroupLayoutTests, DefaultShaderStageAndDynamicOffsets) {
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
        struct S {
            pos : vec4f
        }
        @group(0) @binding(0) var<uniform> uniforms : S;

        @fragment fn main() {
            var pos : vec4f = uniforms.pos;
        })");

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.buffer.type = wgpu::BufferBindingType::Uniform;
    binding.buffer.minBindingSize = 4 * sizeof(float);

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    // Check that an otherwise compatible bind group layout is not fully equivalent to  one created
    // as part of a default pipeline layout, but cache equivalent.
    {
        binding.buffer.hasDynamicOffset = false;
        binding.visibility = wgpu::ShaderStage::Fragment;
        wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&desc);
        EXPECT_THAT(bgl, BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
        EXPECT_THAT(bgl, Not(BindGroupLayoutEq(pipeline.GetBindGroupLayout(0))));
    }

    // Check that any change in visibility doesn't match.
    binding.visibility = wgpu::ShaderStage::Vertex;
    EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                Not(BindGroupLayoutEq(pipeline.GetBindGroupLayout(0))));

    binding.visibility = wgpu::ShaderStage::Compute;
    EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                Not(BindGroupLayoutEq(pipeline.GetBindGroupLayout(0))));

    // Check that any change in hasDynamicOffsets doesn't match.
    binding.buffer.hasDynamicOffset = true;
    binding.visibility = wgpu::ShaderStage::Fragment;
    EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                Not(BindGroupLayoutEq(pipeline.GetBindGroupLayout(0))));
}

TEST_F(GetBindGroupLayoutTests, DefaultTextureSampleType) {
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::BindGroupLayout filteringBGL = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
                  wgpu::TextureSampleType::Float},
                 {1, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
                  wgpu::SamplerBindingType::Filtering}});

    wgpu::BindGroupLayout nonFilteringBGL = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
                  wgpu::TextureSampleType::UnfilterableFloat},
                 {1, wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
                  wgpu::SamplerBindingType::Filtering}});

    wgpu::ShaderModule emptyVertexModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var myTexture : texture_2d<f32>;
        @group(0) @binding(1) var mySampler : sampler;
        @vertex fn main() -> @builtin(position) vec4f {
            _ = myTexture;
            _ = mySampler;
            return vec4f();
        })");

    wgpu::ShaderModule textureLoadVertexModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var myTexture : texture_2d<f32>;
        @group(0) @binding(1) var mySampler : sampler;
        @vertex fn main() -> @builtin(position) vec4f {
            _ = textureLoad(myTexture, vec2i(), 0);
            _ = mySampler;
            return vec4f();
        })");

    wgpu::ShaderModule textureSampleVertexModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var myTexture : texture_2d<f32>;
        @group(0) @binding(1) var mySampler : sampler;
        @vertex fn main() -> @builtin(position) vec4f {
            _ = textureSampleLevel(myTexture, mySampler, vec2f(), 0.0);
            return vec4f();
        })");

    wgpu::ShaderModule unusedTextureFragmentModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var myTexture : texture_2d<f32>;
        @group(0) @binding(1) var mySampler : sampler;
        @fragment fn main() {
            _ = myTexture;
            _ = mySampler;
        })");

    wgpu::ShaderModule textureLoadFragmentModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var myTexture : texture_2d<f32>;
        @group(0) @binding(1) var mySampler : sampler;
        @fragment fn main() {
            _ = textureLoad(myTexture, vec2i(), 0);
            _ = mySampler;
        })");

    wgpu::ShaderModule textureSampleFragmentModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var myTexture : texture_2d<f32>;
        @group(0) @binding(1) var mySampler : sampler;
        @fragment fn main() {
            _ = textureSample(myTexture, mySampler, vec2f());
        })");

    auto BGLFromModules = [this](wgpu::ShaderModule vertexModule,
                                 wgpu::ShaderModule fragmentModule) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vertexModule;
        descriptor.cFragment.module = fragmentModule;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        return device.CreateRenderPipeline(&descriptor).GetBindGroupLayout(0);
    };

    // Textures not used default to non-filtering
    {
        wgpu::BindGroupLayout bgl = BGLFromModules(emptyVertexModule, unusedTextureFragmentModule);
        EXPECT_THAT(bgl, BindGroupLayoutCacheEq(nonFilteringBGL));
        EXPECT_THAT(bgl, Not(BindGroupLayoutCacheEq(filteringBGL)));
    }

    // Textures used with textureLoad default to non-filtering
    {
        wgpu::BindGroupLayout bgl = BGLFromModules(emptyVertexModule, textureLoadFragmentModule);
        EXPECT_THAT(bgl, BindGroupLayoutCacheEq(nonFilteringBGL));
        EXPECT_THAT(bgl, Not(BindGroupLayoutCacheEq(filteringBGL)));
    }

    // Textures used with textureLoad on both stages default to non-filtering
    {
        wgpu::BindGroupLayout bgl =
            BGLFromModules(textureLoadVertexModule, textureLoadFragmentModule);
        EXPECT_THAT(bgl, BindGroupLayoutCacheEq(nonFilteringBGL));
        EXPECT_THAT(bgl, Not(BindGroupLayoutCacheEq(filteringBGL)));
    }

    // Textures used with textureSample default to filtering
    {
        wgpu::BindGroupLayout bgl = BGLFromModules(emptyVertexModule, textureSampleFragmentModule);
        EXPECT_THAT(bgl, Not(BindGroupLayoutCacheEq(nonFilteringBGL)));
        EXPECT_THAT(bgl, BindGroupLayoutCacheEq(filteringBGL));
    }
    {
        wgpu::BindGroupLayout bgl =
            BGLFromModules(textureSampleVertexModule, unusedTextureFragmentModule);
        EXPECT_THAT(bgl, Not(BindGroupLayoutCacheEq(nonFilteringBGL)));
        EXPECT_THAT(bgl, BindGroupLayoutCacheEq(filteringBGL));
    }

    // Textures used with both textureLoad and textureSample default to filtering
    {
        wgpu::BindGroupLayout bgl =
            BGLFromModules(textureLoadVertexModule, textureSampleFragmentModule);
        EXPECT_THAT(bgl, Not(BindGroupLayoutCacheEq(nonFilteringBGL)));
        EXPECT_THAT(bgl, BindGroupLayoutCacheEq(filteringBGL));
    }
    {
        wgpu::BindGroupLayout bgl =
            BGLFromModules(textureSampleVertexModule, textureLoadFragmentModule);
        EXPECT_THAT(bgl, Not(BindGroupLayoutCacheEq(nonFilteringBGL)));
        EXPECT_THAT(bgl, BindGroupLayoutCacheEq(filteringBGL));
    }
}

// Test GetBindGroupLayout works with a compute pipeline
TEST_F(GetBindGroupLayoutTests, ComputePipeline) {
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::ShaderModule csModule = utils::CreateShaderModule(device, R"(
        struct S {
            pos : vec4f
        }
        @group(0) @binding(0) var<uniform> uniforms : S;

        @compute @workgroup_size(1) fn main() {
            var pos : vec4f = uniforms.pos;
        })");

    wgpu::ComputePipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.compute.module = csModule;

    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&descriptor);

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.buffer.type = wgpu::BufferBindingType::Uniform;
    binding.visibility = wgpu::ShaderStage::Compute;
    binding.buffer.hasDynamicOffset = false;
    binding.buffer.minBindingSize = 4 * sizeof(float);

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    // The pipeline bind group should be cache equivalent, but not fully equivalent since it was
    // default created by the pipeline.
    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&desc);
    EXPECT_THAT(bgl, Not(BindGroupLayoutEq(pipeline.GetBindGroupLayout(0))));
    EXPECT_THAT(bgl, BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
}

// Test that the binding type matches the shader.
TEST_F(GetBindGroupLayoutTests, BindingType) {
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.buffer.hasDynamicOffset = false;
    binding.buffer.minBindingSize = 4 * sizeof(float);
    binding.visibility = wgpu::ShaderStage::Fragment;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    {
        // Storage buffer binding is not supported in vertex shader.
        binding.visibility = wgpu::ShaderStage::Fragment;
        binding.buffer.type = wgpu::BufferBindingType::Storage;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            struct S {
                pos : vec4f
            }
            @group(0) @binding(0) var<storage, read_write> ssbo : S;

            @fragment fn main() {
                var pos : vec4f = ssbo.pos;
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }
    {
        binding.buffer.type = wgpu::BufferBindingType::Uniform;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            struct S {
                pos : vec4f
            }
            @group(0) @binding(0) var<uniform> uniforms : S;

            @fragment fn main() {
                var pos : vec4f = uniforms.pos;
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    {
        binding.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            struct S {
                pos : vec4f
            }
            @group(0) @binding(0) var<storage, read> ssbo : S;

            @fragment fn main() {
                var pos : vec4f = ssbo.pos;
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    binding.buffer.type = wgpu::BufferBindingType::BindingNotUsed;
    binding.buffer.minBindingSize = 0;
    {
        binding.texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            @group(0) @binding(0) var myTexture : texture_2d<f32>;

            @fragment fn main() {
                _ = textureDimensions(myTexture);
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    {
        binding.texture.multisampled = true;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            @group(0) @binding(0) var myTexture : texture_multisampled_2d<f32>;

            @fragment fn main() {
                _ = textureDimensions(myTexture);
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    binding.texture.sampleType = wgpu::TextureSampleType::BindingNotUsed;
    {
        binding.sampler.type = wgpu::SamplerBindingType::Filtering;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            @group(0) @binding(0) var mySampler: sampler;

            @fragment fn main() {
                _ = mySampler;
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }
}

// Tests that the external texture binding type matches with a texture_external declared in the
// shader.
TEST_F(GetBindGroupLayoutTests, ExternalTextureBindingType) {
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.visibility = wgpu::ShaderStage::Fragment;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    binding.nextInChain = &utils::kExternalTextureBindingLayout;
    wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            @group(0) @binding(0) var myExternalTexture: texture_external;

            @fragment fn main() {
               _ = myExternalTexture;
            })");
    EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
}

// Test that texture view dimension matches the shader.
TEST_F(GetBindGroupLayoutTests, TextureViewDimension) {
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.visibility = wgpu::ShaderStage::Fragment;
    binding.texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::e1D;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            @group(0) @binding(0) var myTexture : texture_1d<f32>;

            @fragment fn main() {
                _ = textureDimensions(myTexture);
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::e2D;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            @group(0) @binding(0) var myTexture : texture_2d<f32>;

            @fragment fn main() {
                _ = textureDimensions(myTexture);
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));

        // viewDimension defaults to 2D, so should be cached the same.
        binding.texture.viewDimension = wgpu::TextureViewDimension::Undefined;
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::e2DArray;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            @group(0) @binding(0) var myTexture : texture_2d_array<f32>;

            @fragment fn main() {
                _ = textureDimensions(myTexture);
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::e3D;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            @group(0) @binding(0) var myTexture : texture_3d<f32>;

            @fragment fn main() {
                _ = textureDimensions(myTexture);
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::Cube;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            @group(0) @binding(0) var myTexture : texture_cube<f32>;

            @fragment fn main() {
                _ = textureDimensions(myTexture);
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    {
        binding.texture.viewDimension = wgpu::TextureViewDimension::CubeArray;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            @group(0) @binding(0) var myTexture : texture_cube_array<f32>;

            @fragment fn main() {
                _ = textureDimensions(myTexture);
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }
}

// Test that storageTexture view dimension matches the shader.
TEST_F(GetBindGroupLayoutTests, StorageTextureViewDimension) {
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.visibility = wgpu::ShaderStage::Fragment;
    binding.storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
    binding.storageTexture.format = wgpu::TextureFormat::R32Float;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    {
        binding.storageTexture.viewDimension = wgpu::TextureViewDimension::e1D;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            @group(0) @binding(0) var myTexture : texture_storage_1d<r32float, write>;

            @fragment fn main() {
                _ = textureDimensions(myTexture);
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    {
        binding.storageTexture.viewDimension = wgpu::TextureViewDimension::e2D;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            @group(0) @binding(0) var myTexture : texture_storage_2d<r32float, write>;

            @fragment fn main() {
                _ = textureDimensions(myTexture);
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));

        // viewDimension defaults to 2D, so should be cached the same.
        binding.storageTexture.viewDimension = wgpu::TextureViewDimension::Undefined;
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    {
        binding.storageTexture.viewDimension = wgpu::TextureViewDimension::e2DArray;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            @group(0) @binding(0) var myTexture : texture_storage_2d_array<r32float, write>;

            @fragment fn main() {
                _ = textureDimensions(myTexture);
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    {
        binding.storageTexture.viewDimension = wgpu::TextureViewDimension::e3D;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            @group(0) @binding(0) var myTexture : texture_storage_3d<r32float, write>;

            @fragment fn main() {
                _ = textureDimensions(myTexture);
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }
}

// Test that texture component type matches the shader.
TEST_F(GetBindGroupLayoutTests, TextureComponentType) {
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.visibility = wgpu::ShaderStage::Fragment;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    {
        binding.texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            @group(0) @binding(0) var myTexture : texture_2d<f32>;

            @fragment fn main() {
                _ = textureDimensions(myTexture);
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    {
        binding.texture.sampleType = wgpu::TextureSampleType::Sint;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            @group(0) @binding(0) var myTexture : texture_2d<i32>;

            @fragment fn main() {
                _ = textureDimensions(myTexture);
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    {
        binding.texture.sampleType = wgpu::TextureSampleType::Uint;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            @group(0) @binding(0) var myTexture : texture_2d<u32>;

            @fragment fn main() {
                _ = textureDimensions(myTexture);
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }
}

// Test that binding= indices match.
TEST_F(GetBindGroupLayoutTests, BindingIndices) {
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::BindGroupLayoutEntry binding = {};
    binding.visibility = wgpu::ShaderStage::Fragment;
    binding.buffer.type = wgpu::BufferBindingType::Uniform;
    binding.buffer.hasDynamicOffset = false;
    binding.buffer.minBindingSize = 4 * sizeof(float);

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    {
        binding.binding = 0;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            struct S {
                pos : vec4f
            }
            @group(0) @binding(0) var<uniform> uniforms : S;

            @fragment fn main() {
                var pos : vec4f = uniforms.pos;
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    {
        binding.binding = 1;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            struct S {
                pos : vec4f
            }
            @group(0) @binding(1) var<uniform> uniforms : S;

            @fragment fn main() {
                var pos : vec4f = uniforms.pos;
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    {
        binding.binding = 2;
        wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
            struct S {
                pos : vec4f
            }
            @group(0) @binding(1) var<uniform> uniforms : S;

            @fragment fn main() {
                var pos : vec4f = uniforms.pos;
            })");
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    Not(BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0))));
    }
}

// Test it is valid to have duplicate bindings in the shaders.
TEST_F(GetBindGroupLayoutTests, DuplicateBinding) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        struct S {
            pos : vec4f
        }
        @group(0) @binding(0) var<uniform> uniform0 : S;
        @group(1) @binding(0) var<uniform> uniform1 : S;

        @vertex fn main() -> @builtin(position) vec4f {
            var pos : vec4f = uniform0.pos;
            pos = uniform1.pos;
            return vec4f();
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        struct S {
            pos : vec4f
        }
        @group(1) @binding(0) var<uniform> uniforms : S;

        @fragment fn main() {
            var pos : vec4f = uniforms.pos;
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

    device.CreateRenderPipeline(&descriptor);
}

// Test that minBufferSize is set on the BGL and that the max of the min buffer sizes is used.
TEST_F(GetBindGroupLayoutTests, MinBufferSize) {
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::ShaderModule vsModule4 = utils::CreateShaderModule(device, R"(
        struct S {
            pos : f32
        }
        @group(0) @binding(0) var<uniform> uniforms : S;

        @vertex fn main() -> @builtin(position) vec4f {
            var pos : f32 = uniforms.pos;
            return vec4f();
        })");

    wgpu::ShaderModule vsModule64 = utils::CreateShaderModule(device, R"(
        struct S {
            pos : mat4x4<f32>
        }
        @group(0) @binding(0) var<uniform> uniforms : S;

        @vertex fn main() -> @builtin(position) vec4f {
            var pos : mat4x4<f32> = uniforms.pos;
            return vec4f();
        })");

    wgpu::ShaderModule fsModule4 = utils::CreateShaderModule(device, R"(
        struct S {
            pos : f32
        }
        @group(0) @binding(0) var<uniform> uniforms : S;

        @fragment fn main() {
            var pos : f32 = uniforms.pos;
        })");

    wgpu::ShaderModule fsModule64 = utils::CreateShaderModule(device, R"(
        struct S {
            pos : mat4x4<f32>
        }
        @group(0) @binding(0) var<uniform> uniforms : S;

        @fragment fn main() {
            var pos : mat4x4<f32> = uniforms.pos;
        })");

    // Create BGLs with minBufferBindingSize 4 and 64.
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.buffer.type = wgpu::BufferBindingType::Uniform;
    binding.visibility = wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Vertex;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    binding.buffer.minBindingSize = 4;
    wgpu::BindGroupLayout bgl4 = device.CreateBindGroupLayout(&desc);
    binding.buffer.minBindingSize = 64;
    wgpu::BindGroupLayout bgl64 = device.CreateBindGroupLayout(&desc);

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

    // Check with both stages using 4 bytes.
    {
        descriptor.vertex.module = vsModule4;
        descriptor.cFragment.module = fsModule4;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        EXPECT_THAT(pipeline.GetBindGroupLayout(0), BindGroupLayoutCacheEq(bgl4));
    }

    // Check that the max is taken between 4 and 64.
    {
        descriptor.vertex.module = vsModule64;
        descriptor.cFragment.module = fsModule4;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        EXPECT_THAT(pipeline.GetBindGroupLayout(0), BindGroupLayoutCacheEq(bgl64));
    }

    // Check that the order doesn't change that the max is taken.
    {
        descriptor.vertex.module = vsModule4;
        descriptor.cFragment.module = fsModule64;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        EXPECT_THAT(pipeline.GetBindGroupLayout(0), BindGroupLayoutCacheEq(bgl64));
    }
}

// Test that the visibility is correctly aggregated if two stages have the exact same binding.
TEST_F(GetBindGroupLayoutTests, StageAggregation) {
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::ShaderModule vsModuleNoSampler = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f();
        })");

    wgpu::ShaderModule vsModuleSampler = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var mySampler: sampler;
        @vertex fn main() -> @builtin(position) vec4f {
            _ = mySampler;
            return vec4f();
        })");

    wgpu::ShaderModule fsModuleNoSampler = utils::CreateShaderModule(device, R"(
        @fragment fn main() {
        })");

    wgpu::ShaderModule fsModuleSampler = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var mySampler: sampler;
        @fragment fn main() {
            _ = mySampler;
        })");

    // Create BGLs with minBufferBindingSize 4 and 64.
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.sampler.type = wgpu::SamplerBindingType::Filtering;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

    // Check with only the vertex shader using the sampler
    {
        descriptor.vertex.module = vsModuleSampler;
        descriptor.cFragment.module = fsModuleNoSampler;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        binding.visibility = wgpu::ShaderStage::Vertex;
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    // Check with only the fragment shader using the sampler
    {
        descriptor.vertex.module = vsModuleNoSampler;
        descriptor.cFragment.module = fsModuleSampler;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        binding.visibility = wgpu::ShaderStage::Fragment;
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }

    // Check with both shaders using the sampler
    {
        descriptor.vertex.module = vsModuleSampler;
        descriptor.cFragment.module = fsModuleSampler;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        binding.visibility = wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Vertex;
        EXPECT_THAT(device.CreateBindGroupLayout(&desc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }
}

// Test that a binding_array is reflected into a BGLEntry with an arraySize.
TEST_F(GetBindGroupLayoutTests, ArraySizeReflected) {
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::BindGroupLayoutEntry entry;
    entry.binding = 0;
    entry.visibility = wgpu::ShaderStage::Fragment;
    entry.texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = &entry;

    // The pipeline using binding_array
    wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
        @group(0) @binding(0) var t: binding_array<texture_2d<f32>, 3>;
        @fragment fn main() {
            _ = t[0];
        })");

    entry.bindingArraySize = 3;
    EXPECT_THAT(device.CreateBindGroupLayout(&bglDesc),
                BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    entry.bindingArraySize = 2;
    EXPECT_THAT(device.CreateBindGroupLayout(&bglDesc),
                Not(BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0))));
    entry.bindingArraySize = 1;
    EXPECT_THAT(device.CreateBindGroupLayout(&bglDesc),
                Not(BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0))));
}

// Test that a binding_array is reflected into an entry with the max of both sizes
TEST_F(GetBindGroupLayoutTests, ArraySizeTwoStages) {
    DAWN_SKIP_TEST_IF(UsesWire());

    // A BGL with arraySize = 3
    wgpu::BindGroupLayoutEntry entry;
    entry.binding = 0;
    entry.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
    entry.bindingArraySize = 3;
    entry.texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = &entry;

    // The pipeline using binding_array, with differing binding_array sizes. VS = 3, FS = 2
    {
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var vs_t: binding_array<texture_2d<f32>, 3>;
            @vertex fn vs() -> @builtin(position) vec4f {
                _ = vs_t[0];
                return vec4(0);
            }

            @group(0) @binding(0) var fs_t: binding_array<texture_2d<f32>, 2>;
            @fragment fn fs() {
                _ = fs_t[0];
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.layout = nullptr;
        descriptor.vertex.module = module;
        descriptor.cFragment.module = module;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        EXPECT_THAT(device.CreateBindGroupLayout(&bglDesc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }
    // The pipeline using binding_array, with differing binding_array sizes. VS = 2, FS = 3
    {
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var vs_t: binding_array<texture_2d<f32>, 2>;
            @vertex fn vs() -> @builtin(position) vec4f {
                _ = vs_t[0];
                return vec4(0);
            }

            @group(0) @binding(0) var fs_t: binding_array<texture_2d<f32>, 3>;
            @fragment fn fs() {
                _ = fs_t[0];
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.layout = nullptr;
        descriptor.vertex.module = module;
        descriptor.cFragment.module = module;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        EXPECT_THAT(device.CreateBindGroupLayout(&bglDesc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }
    // The pipeline using binding_array, with differing binding_array sizes. VS = 3, FS = NotArrayed
    {
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var vs_t: binding_array<texture_2d<f32>, 3>;
            @vertex fn vs() -> @builtin(position) vec4f {
                _ = vs_t[0];
                return vec4(0);
            }

            @group(0) @binding(0) var fs_t: texture_2d<f32>;
            @fragment fn fs() {
                _ = fs_t;
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.layout = nullptr;
        descriptor.vertex.module = module;
        descriptor.cFragment.module = module;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
        EXPECT_THAT(device.CreateBindGroupLayout(&bglDesc),
                    BindGroupLayoutCacheEq(pipeline.GetBindGroupLayout(0)));
    }
}

// Test it is invalid to have conflicting binding types in the shaders.
TEST_F(GetBindGroupLayoutTests, ConflictingBindingType) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        struct S {
            pos : vec4f
        }
        @group(0) @binding(0) var<uniform> ubo : S;

        @vertex fn main() -> @builtin(position) vec4f {
            var pos : vec4f = ubo.pos;
            return vec4f();
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        struct S {
            pos : vec4f
        }
        @group(0) @binding(0) var<storage, read_write> ssbo : S;

        @fragment fn main() {
            var pos : vec4f = ssbo.pos;
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;

    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test it is invalid to have conflicting binding texture multisampling in the shaders.
TEST_F(GetBindGroupLayoutTests, ConflictingBindingTextureMultisampling) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var myTexture : texture_2d<f32>;

        @vertex fn main() -> @builtin(position) vec4f {
            _ = textureDimensions(myTexture);
            return vec4f();
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var myTexture : texture_multisampled_2d<f32>;

        @fragment fn main() {
            _ = textureDimensions(myTexture);
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;

    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test it is invalid to have conflicting binding texture dimension in the shaders.
TEST_F(GetBindGroupLayoutTests, ConflictingBindingViewDimension) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var myTexture : texture_2d<f32>;

        @vertex fn main() -> @builtin(position) vec4f {
            _ = textureDimensions(myTexture);
            return vec4f();
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var myTexture : texture_3d<f32>;

        @fragment fn main() {
            _ = textureDimensions(myTexture);
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;

    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test it is invalid to have conflicting binding texture component type in the shaders.
TEST_F(GetBindGroupLayoutTests, ConflictingBindingTextureComponentType) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var myTexture : texture_2d<f32>;

        @vertex fn main() -> @builtin(position) vec4f {
            _ = textureDimensions(myTexture);
            return vec4f();
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var myTexture : texture_2d<i32>;

        @fragment fn main() {
            _ = textureDimensions(myTexture);
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;

    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test it is an error to query an out of range bind group layout.
TEST_F(GetBindGroupLayoutTests, OutOfRangeIndex) {
    ASSERT_DEVICE_ERROR(RenderPipelineFromFragmentShader(R"(
        @fragment fn main() {
        })")
                            .GetBindGroupLayout(kMaxBindGroups));

    ASSERT_DEVICE_ERROR(RenderPipelineFromFragmentShader(R"(
        @fragment fn main() {
        })")
                            .GetBindGroupLayout(kMaxBindGroups + 1));
}

// Test that unused indices return the empty bind group layout if less than the maximum number of
// bind groups, an error otherwise.
TEST_F(GetBindGroupLayoutTests, UnusedIndex) {
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::RenderPipeline pipeline = RenderPipelineFromFragmentShader(R"(
        struct S {
            pos : vec4f
        }
        @group(0) @binding(0) var<uniform> uniforms0 : S;
        @group(2) @binding(0) var<uniform> uniforms2 : S;

        @fragment fn main() {
            var pos : vec4f = uniforms0.pos;
            pos = uniforms2.pos;
        })");

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 0;
    desc.entries = nullptr;

    wgpu::BindGroupLayout emptyBindGroupLayout = device.CreateBindGroupLayout(&desc);

    EXPECT_THAT(pipeline.GetBindGroupLayout(0),
                Not(BindGroupLayoutCacheEq(emptyBindGroupLayout)));  // Used
    EXPECT_THAT(pipeline.GetBindGroupLayout(1),
                BindGroupLayoutCacheEq(emptyBindGroupLayout));  // Not used
    EXPECT_THAT(pipeline.GetBindGroupLayout(2),
                Not(BindGroupLayoutCacheEq(emptyBindGroupLayout)));  // Used
    EXPECT_THAT(pipeline.GetBindGroupLayout(3),
                BindGroupLayoutCacheEq(emptyBindGroupLayout));  // Past last defined BGL

    // Equal to kMaxBindGroups, error!
    ASSERT_DEVICE_ERROR(pipeline.GetBindGroupLayout(kMaxBindGroups));
}

// Test that after explicitly creating a pipeline with a pipeline layout, calling
// GetBindGroupLayout reflects the same bind group layouts.
TEST_F(GetBindGroupLayoutTests, Reflection) {
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.buffer.type = wgpu::BufferBindingType::Uniform;
    binding.visibility = wgpu::ShaderStage::Vertex;

    wgpu::BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = 1;
    bglDesc.entries = &binding;

    wgpu::BindGroupLayout bindGroupLayout = device.CreateBindGroupLayout(&bglDesc);

    wgpu::PipelineLayoutDescriptor pipelineLayoutDesc = {};
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;

    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&pipelineLayoutDesc);

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        struct S {
            pos : vec4f
        }
        @group(0) @binding(0) var<uniform> uniforms : S;

        @vertex fn main() -> @builtin(position) vec4f {
            var pos : vec4f = uniforms.pos;
            return vec4f();
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() {
        })");

    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.vertex.module = vsModule;
    pipelineDesc.cFragment.module = fsModule;
    pipelineDesc.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

    EXPECT_THAT(pipeline.GetBindGroupLayout(0), BindGroupLayoutEq(bindGroupLayout));
}

// Test that fragment output validation is for the correct entryPoint
TEST_F(GetBindGroupLayoutTests, FromCorrectEntryPoint) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct Data {
            data : f32
        }
        @group(0) @binding(0) var<storage, read_write> data0 : Data;
        @group(0) @binding(1) var<storage, read_write> data1 : Data;

        @compute @workgroup_size(1) fn compute0() {
            data0.data = 0.0;
        }

        @compute @workgroup_size(1) fn compute1() {
            data1.data = 0.0;
        }
    )");

    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.compute.module = module;

    // Get each entryPoint's BGL.
    pipelineDesc.compute.entryPoint = "compute0";
    wgpu::ComputePipeline pipeline0 = device.CreateComputePipeline(&pipelineDesc);
    wgpu::BindGroupLayout bgl0 = pipeline0.GetBindGroupLayout(0);

    pipelineDesc.compute.entryPoint = "compute1";
    wgpu::ComputePipeline pipeline1 = device.CreateComputePipeline(&pipelineDesc);
    wgpu::BindGroupLayout bgl1 = pipeline1.GetBindGroupLayout(0);

    // Create the buffer used in the bindgroups.
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 4;
    bufferDesc.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    // Success case, the BGL matches the descriptor for the bindgroup.
    utils::MakeBindGroup(device, bgl0, {{0, buffer}});
    utils::MakeBindGroup(device, bgl1, {{1, buffer}});

    // Error case, the BGL doesn't match the descriptor for the bindgroup.
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl0, {{1, buffer}}));
    ASSERT_DEVICE_ERROR(utils::MakeBindGroup(device, bgl1, {{0, buffer}}));
}

// Test that a pipeline full of explicitly empty BGLs correctly reflects them.
TEST_F(GetBindGroupLayoutTests, FullOfEmptyBGLs) {
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::BindGroupLayout emptyBGL = utils::MakeBindGroupLayout(device, {});
    wgpu::PipelineLayout pl =
        utils::MakePipelineLayout(device, {emptyBGL, emptyBGL, emptyBGL, emptyBGL});

    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.layout = pl;
    pipelineDesc.compute.module = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1) fn main() {
        }
    )");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    EXPECT_THAT(pipeline.GetBindGroupLayout(0), BindGroupLayoutEq(emptyBGL));
    EXPECT_THAT(pipeline.GetBindGroupLayout(1), BindGroupLayoutEq(emptyBGL));
    EXPECT_THAT(pipeline.GetBindGroupLayout(2), BindGroupLayoutEq(emptyBGL));
    EXPECT_THAT(pipeline.GetBindGroupLayout(3), BindGroupLayoutEq(emptyBGL));
}

// Test that a pipeline full of explicitly null BGLs correctly reflects empty BGLs.
TEST_F(GetBindGroupLayoutTests, NullBGLs) {
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::PipelineLayout pl =
        utils::MakePipelineLayout(device, {nullptr, nullptr, nullptr, nullptr});

    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.layout = pl;
    pipelineDesc.compute.module = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(1) fn main() {
        }
    )");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    wgpu::BindGroupLayout emptyBGL = utils::MakeBindGroupLayout(device, {});
    EXPECT_THAT(pipeline.GetBindGroupLayout(0), BindGroupLayoutEq(emptyBGL));
    EXPECT_THAT(pipeline.GetBindGroupLayout(1), BindGroupLayoutEq(emptyBGL));
    EXPECT_THAT(pipeline.GetBindGroupLayout(2), BindGroupLayoutEq(emptyBGL));
    EXPECT_THAT(pipeline.GetBindGroupLayout(3), BindGroupLayoutEq(emptyBGL));
}

}  // anonymous namespace
}  // namespace dawn
