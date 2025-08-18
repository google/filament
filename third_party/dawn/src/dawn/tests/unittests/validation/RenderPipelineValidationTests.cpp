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

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class RenderPipelineValidationTest : public ValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::ShaderF16};
    }

    void SetUp() override {
        ValidationTest::SetUp();

        vsModule = utils::CreateShaderModule(device, R"(
            @vertex fn main() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 1.0);
            })");

        fsModule = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(0.0, 1.0, 0.0, 1.0);
            })");

        fsModuleUint = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4u {
                return vec4u(0u, 255u, 0u, 255u);
            })");
    }

    wgpu::ShaderModule vsModule;
    wgpu::ShaderModule fsModule;
    wgpu::ShaderModule fsModuleUint;
};

bool BlendFactorContainsSrcAlpha(const wgpu::BlendFactor& blendFactor) {
    return blendFactor == wgpu::BlendFactor::SrcAlpha ||
           blendFactor == wgpu::BlendFactor::OneMinusSrcAlpha ||
           blendFactor == wgpu::BlendFactor::SrcAlphaSaturated;
}

// Test cases where creation should succeed
TEST_F(RenderPipelineValidationTest, CreationSuccess) {
    {
        // New format
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;

        device.CreateRenderPipeline(&descriptor);
    }
}

// Tests that depth bias parameters must not be NaN.
TEST_F(RenderPipelineValidationTest, DepthBiasParameterNotBeNaN) {
    // Control case, depth bias parameters in ComboRenderPipeline default to 0 which is finite
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.EnableDepthStencil();
        device.CreateRenderPipeline(&descriptor);
    }

    // Infinite depth bias clamp is invalid
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil();
        depthStencil->depthBiasClamp = INFINITY;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
    // NAN depth bias clamp is invalid
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil();
        depthStencil->depthBiasClamp = NAN;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    // Infinite depth bias slope is invalid
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil();
        depthStencil->depthBiasSlopeScale = INFINITY;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
    // NAN depth bias slope is invalid
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil();
        depthStencil->depthBiasSlopeScale = NAN;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

/// Tests that depthBias can't be used with point or line topologies.
TEST_F(RenderPipelineValidationTest, DepthBiasNotWithPointLine) {
    constexpr std::array<wgpu::PrimitiveTopology, 2> kValidTopologyTypes = {
        {wgpu::PrimitiveTopology::TriangleList, wgpu::PrimitiveTopology::TriangleStrip}};

    constexpr std::array<wgpu::PrimitiveTopology, 3> kInvalidTopologyTypes = {
        {wgpu::PrimitiveTopology::PointList, wgpu::PrimitiveTopology::LineList,
         wgpu::PrimitiveTopology::LineStrip}};

    // Depth bias parameters can be used with triangle topologies
    for (wgpu::PrimitiveTopology primitiveTopology : kValidTopologyTypes) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;

        descriptor.primitive.topology = primitiveTopology;
        if (primitiveTopology == wgpu::PrimitiveTopology::TriangleStrip) {
            descriptor.primitive.stripIndexFormat = wgpu::IndexFormat::Uint32;
        }

        wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil();
        depthStencil->depthBias = 1;
        depthStencil->depthBiasSlopeScale = 1;
        depthStencil->depthBiasClamp = 1;

        device.CreateRenderPipeline(&descriptor);
    }

    for (wgpu::PrimitiveTopology primitiveTopology : kInvalidTopologyTypes) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;

        descriptor.primitive.topology = primitiveTopology;
        if (primitiveTopology == wgpu::PrimitiveTopology::LineStrip) {
            descriptor.primitive.stripIndexFormat = wgpu::IndexFormat::Uint32;
        }

        wgpu::DepthStencilState* depthStencil = descriptor.EnableDepthStencil();

        // Depth bias parameters of zero are valid with point or line topologies
        {
            depthStencil->depthBias = 0;
            depthStencil->depthBiasSlopeScale = 0;
            depthStencil->depthBiasClamp = 0;
            device.CreateRenderPipeline(&descriptor);
        }

        // Non-zero depthBias cannot be used with point or line topologies
        {
            depthStencil->depthBias = 1;
            depthStencil->depthBiasSlopeScale = 0;
            depthStencil->depthBiasClamp = 0;
            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
        }

        // Non-zero depthBiasSlopeScale cannot be used with point or line topologies
        {
            depthStencil->depthBias = 0;
            depthStencil->depthBiasSlopeScale = 1;
            depthStencil->depthBiasClamp = 0;
            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
        }

        // Non-zero depthBiasClamp cannot be used with point or line topologies
        {
            depthStencil->depthBias = 0;
            depthStencil->depthBiasSlopeScale = 0;
            depthStencil->depthBiasClamp = 1;
            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
        }
    }
}

// Tests that depth or stencil aspect is required if we enable depth or stencil test.
TEST_F(RenderPipelineValidationTest, DepthStencilAspectRequirement) {
    // Control case, stencil aspect is required if stencil test or stencil write is enabled
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil =
            descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth24PlusStencil8);
        depthStencil->stencilFront.compare = wgpu::CompareFunction::LessEqual;
        depthStencil->stencilBack.failOp = wgpu::StencilOperation::Replace;
        device.CreateRenderPipeline(&descriptor);
    }

    // Control case, stencil faces with Keep/Always are valid when the format doesn't have stencil
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil =
            descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth24Plus);
        depthStencil->stencilFront.compare = wgpu::CompareFunction::Always;
        depthStencil->stencilFront.failOp = wgpu::StencilOperation::Keep;
        depthStencil->stencilFront.depthFailOp = wgpu::StencilOperation::Keep;
        depthStencil->stencilFront.passOp = wgpu::StencilOperation::Keep;
        depthStencil->stencilBack.compare = wgpu::CompareFunction::Always;
        depthStencil->stencilBack.failOp = wgpu::StencilOperation::Keep;
        depthStencil->stencilBack.depthFailOp = wgpu::StencilOperation::Keep;
        depthStencil->stencilBack.passOp = wgpu::StencilOperation::Keep;
        device.CreateRenderPipeline(&descriptor);
    }

    // Control case, stencil faces with Undefined are valid when the format doesn't have stencil
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil =
            descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth24Plus);
        depthStencil->stencilFront.compare = wgpu::CompareFunction::Undefined;
        depthStencil->stencilFront.failOp = wgpu::StencilOperation::Undefined;
        depthStencil->stencilFront.depthFailOp = wgpu::StencilOperation::Undefined;
        depthStencil->stencilFront.passOp = wgpu::StencilOperation::Undefined;
        depthStencil->stencilBack.compare = wgpu::CompareFunction::Undefined;
        depthStencil->stencilBack.failOp = wgpu::StencilOperation::Undefined;
        depthStencil->stencilBack.depthFailOp = wgpu::StencilOperation::Undefined;
        depthStencil->stencilBack.passOp = wgpu::StencilOperation::Undefined;
        device.CreateRenderPipeline(&descriptor);
    }

    // It is invalid if the texture format doesn't have stencil aspect while stencil test is
    // enabled (depthStencilState.stencilFront are not default values).
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil =
            descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth24Plus);
        depthStencil->stencilFront.compare = wgpu::CompareFunction::LessEqual;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    // It is invalid if the texture format doesn't have stencil aspect while stencil write is
    // enabled (depthStencilState.stencilBack are not default values).
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil =
            descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth24Plus);
        depthStencil->stencilBack.failOp = wgpu::StencilOperation::Replace;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    // Control case, depth aspect is required if depth test or depth write is enabled
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil =
            descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth24PlusStencil8);
        depthStencil->depthCompare = wgpu::CompareFunction::LessEqual;
        depthStencil->depthWriteEnabled = wgpu::OptionalBool::True;
        device.CreateRenderPipeline(&descriptor);
    }

    // It is invalid if the texture format doesn't have depth aspect while depth test is
    // enabled.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil =
            descriptor.EnableDepthStencil(wgpu::TextureFormat::Stencil8);
        depthStencil->depthCompare = wgpu::CompareFunction::LessEqual;
        depthStencil->depthWriteEnabled = wgpu::OptionalBool::False;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    // It is invalid if the texture format doesn't have depth aspect while depth write is
    // enabled.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        wgpu::DepthStencilState* depthStencil =
            descriptor.EnableDepthStencil(wgpu::TextureFormat::Stencil8);
        depthStencil->depthCompare = wgpu::CompareFunction::Undefined;
        depthStencil->depthWriteEnabled = wgpu::OptionalBool::True;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Tests that depth attachment is required when frag_depth is written in fragment stage.
TEST_F(RenderPipelineValidationTest, DepthAttachmentRequiredWhenFragDepthIsWritten) {
    wgpu::ShaderModule fsModuleFragDepthOutput = utils::CreateShaderModule(device, R"(
        struct Output {
            @builtin(frag_depth) depth_out: f32,
            @location(0) color : vec4f,
        }
        @fragment fn main() -> Output {
            var o: Output;
            // We need to make sure this frag_depth isn't optimized out even its value equals "no op".
            o.depth_out = 0.5;
            o.color = vec4f(1.0, 1.0, 1.0, 1.0);
            return o;
        }
    )");

    {
        // Succeeds because there is depth stencil state.
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleFragDepthOutput;
        descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth24PlusStencil8);

        device.CreateRenderPipeline(&descriptor);
    }

    {
        // Fails because there is no depth stencil state.
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleFragDepthOutput;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    {
        // Fails because there is depth stencil state but no depth aspect.
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleFragDepthOutput;
        descriptor.EnableDepthStencil(wgpu::TextureFormat::Stencil8);

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Tests that at least one color target state is required.
TEST_F(RenderPipelineValidationTest, ColorTargetStateRequired) {
    {
        // This one succeeds because attachment 0 is the color attachment
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cFragment.targetCount = 1;

        device.CreateRenderPipeline(&descriptor);
    }

    {  // Fail because lack of color target states (and depth/stencil state)
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cFragment.targetCount = 0;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Tests that target blend must not be set if the format is undefined.
TEST_F(RenderPipelineValidationTest, UndefinedColorStateFormatWithBlend) {
    {
        // Control case: Valid undefined format target.
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cFragment.targetCount = 1;
        descriptor.cTargets[0].format = wgpu::TextureFormat::Undefined;

        device.CreateRenderPipeline(&descriptor);
    }
    {
        // Error case: undefined format target with blend state set.
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cFragment.targetCount = 1;
        descriptor.cTargets[0].format = wgpu::TextureFormat::Undefined;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

        ASSERT_DEVICE_ERROR(
            device.CreateRenderPipeline(&descriptor),
            testing::HasSubstr("Color target[0] blend state is set when the format is undefined."));
    }
}

// Tests that a color target that's present in the pipeline descriptor but not in the shader must
// have its writeMask set to 0.
TEST_F(RenderPipelineValidationTest, WriteMaskMustBeZeroForColorTargetWithNoShaderOutput) {
    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.cFragment.targetCount = 2;
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.cTargets[1].format = wgpu::TextureFormat::RGBA8Unorm;

    // Control case: Target 1 not output by the shader but has writeMask = 0
    descriptor.cTargets[1].writeMask = wgpu::ColorWriteMask::None;
    device.CreateRenderPipeline(&descriptor);

    // Error case: the writeMask is not 0.
    descriptor.cTargets[1].writeMask = wgpu::ColorWriteMask::Red;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Tests that the color formats must be renderable.
TEST_F(RenderPipelineValidationTest, NonRenderableFormat) {
    {
        // Succeeds because RGBA8Unorm is renderable
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        device.CreateRenderPipeline(&descriptor);
    }

    {
        // Fails because RG11B10Ufloat is non-renderable
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RG11B10Ufloat;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Tests that the color formats must be blendable when blending is enabled.
// Those are renderable color formats with "float" capabilities in
// https://gpuweb.github.io/gpuweb/#plain-color-formats
TEST_F(RenderPipelineValidationTest, NonBlendableFormat) {
    {
        // Succeeds because RGBA8Unorm is blendable
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        device.CreateRenderPipeline(&descriptor);
    }

    {
        // Fails because RGBA8Uint is not blendable
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleUint;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Uint;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor),
                            testing::HasSubstr("Blending is enabled but color format"));
    }

    {
        // Succeeds because RGBA8Uint is not blendable but blending is disabled
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleUint;
        descriptor.cTargets[0].blend = nullptr;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Uint;

        device.CreateRenderPipeline(&descriptor);
    }
}

// Tests that the float32 color formats are not blendable without the float32-blendable feature.
TEST_F(RenderPipelineValidationTest, Float32NonBlendableFormats) {
    for (const auto f32Format : {wgpu::TextureFormat::R32Float, wgpu::TextureFormat::RG32Float,
                                 wgpu::TextureFormat::RGBA32Float}) {
        {
            // Control case: Succeeds because blending is disabled
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = vsModule;
            descriptor.cFragment.module = fsModule;
            descriptor.cTargets[0].blend = nullptr;
            descriptor.cTargets[0].format = f32Format;

            device.CreateRenderPipeline(&descriptor);
        }
        {
            // Fails because float32 color format blending requires the float32-blendable feature.
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = vsModule;
            descriptor.cFragment.module = fsModule;
            descriptor.cTargets[0].blend = &descriptor.cBlends[0];
            descriptor.cTargets[0].format = f32Format;

            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor),
                                testing::HasSubstr("Blending is enabled but color format"));
        }
    }
}

class Float32BlendableValidationTest : public RenderPipelineValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::Float32Blendable};
    }
};

// Tests that the float32 color formats are blendable only with the float32-blendable feature.
TEST_F(Float32BlendableValidationTest, Float32BlendableFormatsWithFeatureEnabled) {
    for (const auto f32Format : {wgpu::TextureFormat::R32Float, wgpu::TextureFormat::RG32Float,
                                 wgpu::TextureFormat::RGBA32Float}) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cTargets[0].format = f32Format;

        device.CreateRenderPipeline(&descriptor);
    }
}

// Tests that the format of the color state descriptor must match the output of the fragment shader.
TEST_F(RenderPipelineValidationTest, FragmentOutputFormatCompatibility) {
    std::vector<std::vector<std::string>> kScalarTypeLists = {// Float scalar types
                                                              {"f32", "f16"},
                                                              // Sint scalar type
                                                              {"i32"},
                                                              // Uint scalar type
                                                              {"u32"}};

    std::vector<std::vector<wgpu::TextureFormat>> kColorFormatLists = {
        // Float color formats
        {wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA16Float,
         wgpu::TextureFormat::RGBA32Float},
        // Sint color formats
        {wgpu::TextureFormat::RGBA8Sint, wgpu::TextureFormat::RGBA16Sint,
         wgpu::TextureFormat::RGBA32Sint},
        // Uint color formats
        {wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA16Uint,
         wgpu::TextureFormat::RGBA32Uint}};

    for (size_t i = 0; i < kScalarTypeLists.size(); ++i) {
        for (const std::string& scalarType : kScalarTypeLists[i]) {
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = vsModule;
            std::ostringstream stream;

            // Enable f16 extension if needed.
            if (scalarType == "f16") {
                stream << "enable f16;\n\n";
            }
            stream << R"(
            @fragment fn main() -> @location(0) vec4<)"
                   << scalarType << R"(> {
                var result : vec4<)"
                   << scalarType << R"(>;
                return result;
            })";

            descriptor.cFragment.module = utils::CreateShaderModule(device, stream.str().c_str());

            for (size_t j = 0; j < kColorFormatLists.size(); ++j) {
                for (wgpu::TextureFormat textureFormat : kColorFormatLists[j]) {
                    descriptor.cTargets[0].format = textureFormat;
                    if (i == j) {
                        device.CreateRenderPipeline(&descriptor);
                    } else {
                        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
                    }
                }
            }
        }
    }
}

// Tests that the component count of the color state target format must be fewer than that of the
// fragment shader output.
TEST_F(RenderPipelineValidationTest, FragmentOutputComponentCountCompatibility) {
    std::array<wgpu::TextureFormat, 3> kColorFormats = {wgpu::TextureFormat::R8Unorm,
                                                        wgpu::TextureFormat::RG8Unorm,
                                                        wgpu::TextureFormat::RGBA8Unorm};

    std::array<wgpu::BlendFactor, 8> kBlendFactors = {wgpu::BlendFactor::Zero,
                                                      wgpu::BlendFactor::One,
                                                      wgpu::BlendFactor::SrcAlpha,
                                                      wgpu::BlendFactor::OneMinusSrcAlpha,
                                                      wgpu::BlendFactor::Src,
                                                      wgpu::BlendFactor::DstAlpha,
                                                      wgpu::BlendFactor::OneMinusDstAlpha,
                                                      wgpu::BlendFactor::Dst};

    for (size_t componentCount = 1; componentCount <= 4; ++componentCount) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        std::ostringstream stream;
        stream << R"(
            @fragment fn main() -> @location(0) )";
        switch (componentCount) {
            case 1:
                stream << R"(f32 {
                return 1.0;
                })";
                break;
            case 2:
                stream << R"(vec2f {
                return vec2f(1.0, 1.0);
                })";
                break;
            case 3:
                stream << R"(vec3f {
                return vec3f(1.0, 1.0, 1.0);
                })";
                break;
            case 4:
                stream << R"(vec4f {
                return vec4f(1.0, 1.0, 1.0, 1.0);
                })";
                break;
            default:
                DAWN_UNREACHABLE();
        }
        descriptor.cFragment.module = utils::CreateShaderModule(device, stream.str().c_str());

        for (auto colorFormat : kColorFormats) {
            descriptor.cTargets[0].format = colorFormat;

            descriptor.cTargets[0].blend = nullptr;
            if (componentCount >= utils::GetTextureComponentCount(colorFormat)) {
                device.CreateRenderPipeline(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
            }

            descriptor.cTargets[0].blend = &descriptor.cBlends[0];

            for (auto colorSrcFactor : kBlendFactors) {
                descriptor.cBlends[0].color.srcFactor = colorSrcFactor;
                for (auto colorDstFactor : kBlendFactors) {
                    descriptor.cBlends[0].color.dstFactor = colorDstFactor;
                    for (auto alphaSrcFactor : kBlendFactors) {
                        descriptor.cBlends[0].alpha.srcFactor = alphaSrcFactor;
                        for (auto alphaDstFactor : kBlendFactors) {
                            descriptor.cBlends[0].alpha.dstFactor = alphaDstFactor;

                            bool valid = true;
                            if (componentCount >= utils::GetTextureComponentCount(colorFormat)) {
                                if (BlendFactorContainsSrcAlpha(
                                        descriptor.cTargets[0].blend->color.srcFactor) ||
                                    BlendFactorContainsSrcAlpha(
                                        descriptor.cTargets[0].blend->color.dstFactor)) {
                                    valid = componentCount == 4;
                                }
                            } else {
                                valid = false;
                            }

                            if (valid) {
                                device.CreateRenderPipeline(&descriptor);
                            } else {
                                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
                            }
                        }
                    }
                }
            }
        }
    }
}

// Tests that when blendOperationMinOrMax is "min" or "max", both srcBlendFactor and dstBlendFactor
// must be "one".
TEST_F(RenderPipelineValidationTest, BlendOperationAndBlendFactors) {
    constexpr std::array<wgpu::BlendFactor, 9> kBlendFactors = {wgpu::BlendFactor::Undefined,
                                                                wgpu::BlendFactor::Zero,
                                                                wgpu::BlendFactor::One,
                                                                wgpu::BlendFactor::SrcAlpha,
                                                                wgpu::BlendFactor::OneMinusSrcAlpha,
                                                                wgpu::BlendFactor::Src,
                                                                wgpu::BlendFactor::DstAlpha,
                                                                wgpu::BlendFactor::OneMinusDstAlpha,
                                                                wgpu::BlendFactor::Dst};

    constexpr std::array<wgpu::BlendOperation, 2> kBlendOperationsForTest = {
        wgpu::BlendOperation::Max, wgpu::BlendOperation::Min};

    for (wgpu::BlendOperation blendOperationMinOrMax : kBlendOperationsForTest) {
        for (wgpu::BlendFactor srcFactor : kBlendFactors) {
            for (wgpu::BlendFactor dstFactor : kBlendFactors) {
                utils::ComboRenderPipelineDescriptor descriptor;
                descriptor.vertex.module = vsModule;
                descriptor.cFragment.module = fsModule;
                descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
                descriptor.cTargets[0].blend = &descriptor.cBlends[0];
                descriptor.cBlends[0].color.srcFactor = srcFactor;
                descriptor.cBlends[0].color.dstFactor = dstFactor;
                descriptor.cBlends[0].alpha.srcFactor = srcFactor;
                descriptor.cBlends[0].alpha.dstFactor = dstFactor;

                descriptor.cBlends[0].color.operation = blendOperationMinOrMax;
                descriptor.cBlends[0].alpha.operation = wgpu::BlendOperation::Add;

                bool srcIsOne = srcFactor == wgpu::BlendFactor::One ||
                                srcFactor == wgpu::BlendFactor::Undefined;
                bool dstIsOne = dstFactor == wgpu::BlendFactor::One ||
                                dstFactor == wgpu::BlendFactor::Undefined;

                if (srcIsOne && dstIsOne) {
                    device.CreateRenderPipeline(&descriptor);
                } else {
                    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
                }

                descriptor.cBlends[0].color.operation = wgpu::BlendOperation::Add;
                descriptor.cBlends[0].alpha.operation = blendOperationMinOrMax;
                if (srcIsOne && dstIsOne) {
                    device.CreateRenderPipeline(&descriptor);
                } else {
                    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
                }
            }
        }
    }
}

// Tests that enums enabled by the DualSourceBlending feature are not valid when the feature is not
// enabled.
TEST_F(RenderPipelineValidationTest, DualSourceBlendingEnumsInvalid) {
    std::array<wgpu::BlendFactor, 4> kBlendFactors = {
        wgpu::BlendFactor::Src1, wgpu::BlendFactor::OneMinusSrc1, wgpu::BlendFactor::Src1Alpha,
        wgpu::BlendFactor::OneMinusSrc1Alpha};

    // Test color srcFactor
    for (wgpu::BlendFactor blendFactor : kBlendFactors) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cBlends[0].color.srcFactor = blendFactor;
        descriptor.cBlends[0].color.dstFactor = wgpu::BlendFactor::Src;
        descriptor.cBlends[0].color.operation = wgpu::BlendOperation::Add;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    // Test color dstFactor
    for (wgpu::BlendFactor blendFactor : kBlendFactors) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cBlends[0].color.srcFactor = wgpu::BlendFactor::Src;
        descriptor.cBlends[0].color.dstFactor = blendFactor;
        descriptor.cBlends[0].color.operation = wgpu::BlendOperation::Add;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    // Test alpha srcFactor
    for (wgpu::BlendFactor blendFactor : kBlendFactors) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cBlends[0].alpha.srcFactor = blendFactor;
        descriptor.cBlends[0].alpha.dstFactor = wgpu::BlendFactor::SrcAlpha;
        descriptor.cBlends[0].alpha.operation = wgpu::BlendOperation::Add;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    // Test alpha dstFactor
    for (wgpu::BlendFactor blendFactor : kBlendFactors) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cBlends[0].alpha.srcFactor = wgpu::BlendFactor::SrcAlpha;
        descriptor.cBlends[0].alpha.dstFactor = blendFactor;
        descriptor.cBlends[0].alpha.operation = wgpu::BlendOperation::Add;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

/// Tests that the sample count of the render pipeline must be valid.
TEST_F(RenderPipelineValidationTest, SampleCount) {
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.multisample.count = 4;

        device.CreateRenderPipeline(&descriptor);
    }

    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.multisample.count = 3;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Tests that the sample count of the render pipeline must be equal to the one of every attachments
// in the render pass.
TEST_F(RenderPipelineValidationTest, SampleCountCompatibilityWithRenderPass) {
    constexpr uint32_t kMultisampledCount = 4;
    constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr wgpu::TextureFormat kDepthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;

    wgpu::TextureDescriptor baseTextureDescriptor;
    baseTextureDescriptor.size.width = 4;
    baseTextureDescriptor.size.height = 4;
    baseTextureDescriptor.size.depthOrArrayLayers = 1;
    baseTextureDescriptor.mipLevelCount = 1;
    baseTextureDescriptor.dimension = wgpu::TextureDimension::e2D;
    baseTextureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;

    utils::ComboRenderPipelineDescriptor nonMultisampledPipelineDescriptor;
    nonMultisampledPipelineDescriptor.multisample.count = 1;
    nonMultisampledPipelineDescriptor.vertex.module = vsModule;
    nonMultisampledPipelineDescriptor.cFragment.module = fsModule;
    wgpu::RenderPipeline nonMultisampledPipeline =
        device.CreateRenderPipeline(&nonMultisampledPipelineDescriptor);

    nonMultisampledPipelineDescriptor.cFragment.targetCount = 0;
    nonMultisampledPipelineDescriptor.EnableDepthStencil();
    wgpu::RenderPipeline nonMultisampledPipelineWithDepthStencilOnly =
        device.CreateRenderPipeline(&nonMultisampledPipelineDescriptor);

    utils::ComboRenderPipelineDescriptor multisampledPipelineDescriptor;
    multisampledPipelineDescriptor.multisample.count = kMultisampledCount;
    multisampledPipelineDescriptor.vertex.module = vsModule;
    multisampledPipelineDescriptor.cFragment.module = fsModule;
    wgpu::RenderPipeline multisampledPipeline =
        device.CreateRenderPipeline(&multisampledPipelineDescriptor);

    multisampledPipelineDescriptor.cFragment.targetCount = 0;
    multisampledPipelineDescriptor.EnableDepthStencil();
    wgpu::RenderPipeline multisampledPipelineWithDepthStencilOnly =
        device.CreateRenderPipeline(&multisampledPipelineDescriptor);

    // It is not allowed to use multisampled render pass and non-multisampled render pipeline.
    {
        wgpu::TextureDescriptor textureDescriptor = baseTextureDescriptor;
        textureDescriptor.format = kColorFormat;
        textureDescriptor.sampleCount = kMultisampledCount;
        wgpu::Texture multisampledColorTexture = device.CreateTexture(&textureDescriptor);
        utils::ComboRenderPassDescriptor renderPassDescriptor(
            {multisampledColorTexture.CreateView()});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(nonMultisampledPipeline);
        renderPass.End();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        wgpu::TextureDescriptor textureDescriptor = baseTextureDescriptor;
        textureDescriptor.sampleCount = kMultisampledCount;
        textureDescriptor.format = kDepthStencilFormat;
        wgpu::Texture multisampledDepthStencilTexture = device.CreateTexture(&textureDescriptor);
        utils::ComboRenderPassDescriptor renderPassDescriptor(
            {}, multisampledDepthStencilTexture.CreateView());

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(nonMultisampledPipelineWithDepthStencilOnly);
        renderPass.End();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // It is allowed to use multisampled render pass and multisampled render pipeline.
    {
        wgpu::TextureDescriptor textureDescriptor = baseTextureDescriptor;
        textureDescriptor.format = kColorFormat;
        textureDescriptor.sampleCount = kMultisampledCount;
        wgpu::Texture multisampledColorTexture = device.CreateTexture(&textureDescriptor);
        utils::ComboRenderPassDescriptor renderPassDescriptor(
            {multisampledColorTexture.CreateView()});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(multisampledPipeline);
        renderPass.End();

        encoder.Finish();
    }

    {
        wgpu::TextureDescriptor textureDescriptor = baseTextureDescriptor;
        textureDescriptor.sampleCount = kMultisampledCount;
        textureDescriptor.format = kDepthStencilFormat;
        wgpu::Texture multisampledDepthStencilTexture = device.CreateTexture(&textureDescriptor);
        utils::ComboRenderPassDescriptor renderPassDescriptor(
            {}, multisampledDepthStencilTexture.CreateView());

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(multisampledPipelineWithDepthStencilOnly);
        renderPass.End();

        encoder.Finish();
    }

    // It is not allowed to use non-multisampled render pass and multisampled render pipeline.
    {
        wgpu::TextureDescriptor textureDescriptor = baseTextureDescriptor;
        textureDescriptor.format = kColorFormat;
        textureDescriptor.sampleCount = 1;
        wgpu::Texture nonMultisampledColorTexture = device.CreateTexture(&textureDescriptor);
        utils::ComboRenderPassDescriptor nonMultisampledRenderPassDescriptor(
            {nonMultisampledColorTexture.CreateView()});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass =
            encoder.BeginRenderPass(&nonMultisampledRenderPassDescriptor);
        renderPass.SetPipeline(multisampledPipeline);
        renderPass.End();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        wgpu::TextureDescriptor textureDescriptor = baseTextureDescriptor;
        textureDescriptor.sampleCount = 1;
        textureDescriptor.format = kDepthStencilFormat;
        wgpu::Texture nonMultisampledDepthStencilTexture = device.CreateTexture(&textureDescriptor);
        utils::ComboRenderPassDescriptor renderPassDescriptor(
            {}, nonMultisampledDepthStencilTexture.CreateView());

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(multisampledPipelineWithDepthStencilOnly);
        renderPass.End();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Tests that the vertex only pipeline must be used with a depth-stencil attachment only render pass
TEST_F(RenderPipelineValidationTest, VertexOnlyPipelineRequireDepthStencilAttachment) {
    constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr wgpu::TextureFormat kDepthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;

    wgpu::TextureDescriptor baseTextureDescriptor;
    baseTextureDescriptor.size = {4, 4};
    baseTextureDescriptor.mipLevelCount = 1;
    baseTextureDescriptor.dimension = wgpu::TextureDimension::e2D;
    baseTextureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;

    wgpu::TextureDescriptor colorTextureDescriptor = baseTextureDescriptor;
    colorTextureDescriptor.format = kColorFormat;
    colorTextureDescriptor.sampleCount = 1;
    wgpu::Texture colorTexture = device.CreateTexture(&colorTextureDescriptor);

    wgpu::TextureDescriptor depthStencilTextureDescriptor = baseTextureDescriptor;
    depthStencilTextureDescriptor.sampleCount = 1;
    depthStencilTextureDescriptor.format = kDepthStencilFormat;
    wgpu::Texture depthStencilTexture = device.CreateTexture(&depthStencilTextureDescriptor);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.multisample.count = 1;
    renderPipelineDescriptor.vertex.module = vsModule;

    renderPipelineDescriptor.fragment = nullptr;

    renderPipelineDescriptor.EnableDepthStencil(kDepthStencilFormat);

    wgpu::RenderPipeline vertexOnlyPipeline =
        device.CreateRenderPipeline(&renderPipelineDescriptor);

    // Vertex-only render pipeline can work with depth stencil attachment and no color target
    {
        utils::ComboRenderPassDescriptor renderPassDescriptor({}, depthStencilTexture.CreateView());

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(vertexOnlyPipeline);
        renderPass.End();

        encoder.Finish();
    }

    // Vertex-only render pipeline must have a depth stencil attachment
    {
        utils::ComboRenderPassDescriptor renderPassDescriptor;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(vertexOnlyPipeline);
        renderPass.End();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Vertex-only render pipeline can not work with color target
    {
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()},
                                                              depthStencilTexture.CreateView());

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(vertexOnlyPipeline);
        renderPass.End();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Vertex-only render pipeline can not work with color target, and must have a depth stencil
    // attachment
    {
        utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.SetPipeline(vertexOnlyPipeline);
        renderPass.End();

        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Tests that render pipeline without attachments are disallowed.
TEST_F(RenderPipelineValidationTest, NoAttachments) {
    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = vsModule;
    desc.fragment = nullptr;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&desc));

    // Setting a fragment state with no targets is also 0 attachments.
    wgpu::FragmentState fragment;
    fragment.targetCount = 0;
    fragment.module = utils::CreateShaderModule(device, R"(
        @fragment fn fs() {}
    )");
    desc.fragment = &fragment;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&desc));

    // Control case, with a DS attachment, creating the pipeline is allowed.
    desc.EnableDepthStencil(wgpu::TextureFormat::Depth32Float);
    device.CreateRenderPipeline(&desc);
}

// Tests that the sample count of the render pipeline must be valid
// when the alphaToCoverage mode is enabled.
TEST_F(RenderPipelineValidationTest, AlphaToCoverageAndSampleCount) {
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = true;

        device.CreateRenderPipeline(&descriptor);
    }

    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.multisample.count = 1;
        descriptor.multisample.alphaToCoverageEnabled = true;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Tests if the sample_mask builtin is a pipeline output of fragment shader,
// then alphaToCoverageEnabled must be false.
TEST_F(RenderPipelineValidationTest, AlphaToCoverageAndSampleMaskOutput) {
    wgpu::ShaderModule fsModuleSampleMaskOutput = utils::CreateShaderModule(device, R"(
        struct Output {
            @builtin(sample_mask) mask_out: u32,
            @location(0) color : vec4f,
        }
        @fragment fn main() -> Output {
            var o: Output;
            // We need to make sure this sample_mask isn't optimized out even its value equals "no op".
            o.mask_out = 0xFFFFFFFFu;
            o.color = vec4f(1.0, 1.0, 1.0, 1.0);
            return o;
        }
    )");

    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleSampleMaskOutput;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = false;

        device.CreateRenderPipeline(&descriptor);
    }

    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleSampleMaskOutput;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = true;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    {
        // Control cases: when fragment has no sample_mask output, it's good to have
        // alphaToCoverageEnabled enabled
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = true;

        device.CreateRenderPipeline(&descriptor);
    }
}

// Tests when alphaToCoverageEnabled is true, targets[0] must exist and have alpha channel.
TEST_F(RenderPipelineValidationTest, AlphaToCoverageAndColorTargetAlpha) {
    {
        // Control case
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = true;

        device.CreateRenderPipeline(&descriptor);
    }

    {
        // Fragment state must exist
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.fragment = nullptr;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = true;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    {
        // Fragment targets[0] must exist
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cFragment.targetCount = 0;
        descriptor.cFragment.targets = nullptr;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = true;
        descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth32Float);

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    {
        // Fragment targets[0].format must have alpha channel (only 1 target)
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::R8Unorm;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = true;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    wgpu::ShaderModule fsModule2 = utils::CreateShaderModule(device, R"(
        struct FragmentOut {
            @location(0) target0 : vec4f,
            @location(1) target1 : vec4f,
        }
        @fragment fn main() -> FragmentOut {
            var out: FragmentOut;
            out.target0 = vec4f(0, 0, 0, 1);
            out.target1 = vec4f(1, 0, 0, 0);
            return out;
        })");

    {
        // Fragment targets[0].format must have alpha channel (2 targets)
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule2;
        descriptor.cFragment.targetCount = 2;
        descriptor.cTargets[0].format = wgpu::TextureFormat::R8Unorm;
        descriptor.cTargets[1].format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.multisample.count = 4;
        descriptor.multisample.alphaToCoverageEnabled = true;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Tests that the texture component type in shader must match the bind group layout.
TEST_F(RenderPipelineValidationTest, TextureComponentTypeCompatibility) {
    constexpr uint32_t kNumTextureComponentType = 3u;
    std::array<const char*, kNumTextureComponentType> kScalarTypes = {{"f32", "i32", "u32"}};
    std::array<wgpu::TextureSampleType, kNumTextureComponentType> kTextureComponentTypes = {{
        wgpu::TextureSampleType::Float,
        wgpu::TextureSampleType::Sint,
        wgpu::TextureSampleType::Uint,
    }};

    for (size_t i = 0; i < kNumTextureComponentType; ++i) {
        for (size_t j = 0; j < kNumTextureComponentType; ++j) {
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = vsModule;

            std::ostringstream stream;
            stream << R"(
                @group(0) @binding(0) var myTexture : texture_2d<)"
                   << kScalarTypes[i] << R"(>;

                @fragment fn main() {
                    _ = textureDimensions(myTexture);
                })";
            descriptor.cFragment.module = utils::CreateShaderModule(device, stream.str().c_str());
            descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

            wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
                device, {{0, wgpu::ShaderStage::Fragment, kTextureComponentTypes[j]}});
            descriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);

            if (i == j) {
                device.CreateRenderPipeline(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
            }
        }
    }
}

// Tests that the texture view dimension in shader must match the bind group layout.
TEST_F(RenderPipelineValidationTest, TextureViewDimensionCompatibility) {
    constexpr uint32_t kNumTextureViewDimensions = 6u;
    std::array<const char*, kNumTextureViewDimensions> kTextureKeywords = {{
        "texture_1d",
        "texture_2d",
        "texture_2d_array",
        "texture_cube",
        "texture_cube_array",
        "texture_3d",
    }};

    std::array<wgpu::TextureViewDimension, kNumTextureViewDimensions> kTextureViewDimensions = {{
        wgpu::TextureViewDimension::e1D,
        wgpu::TextureViewDimension::e2D,
        wgpu::TextureViewDimension::e2DArray,
        wgpu::TextureViewDimension::Cube,
        wgpu::TextureViewDimension::CubeArray,
        wgpu::TextureViewDimension::e3D,
    }};

    for (size_t i = 0; i < kNumTextureViewDimensions; ++i) {
        for (size_t j = 0; j < kNumTextureViewDimensions; ++j) {
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = vsModule;

            std::ostringstream stream;
            stream << R"(
                @group(0) @binding(0) var myTexture : )"
                   << kTextureKeywords[i] << R"(<f32>;
                @fragment fn main() {
                    _ = textureDimensions(myTexture);
                })";
            descriptor.cFragment.module = utils::CreateShaderModule(device, stream.str().c_str());
            descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

            wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
                device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float,
                          kTextureViewDimensions[j]}});
            descriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);

            if (i == j) {
                device.CreateRenderPipeline(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
            }
        }
    }
}

// Test that declaring a storage buffer in the vertex shader without setting pipeline layout won't
// cause crash.
TEST_F(RenderPipelineValidationTest, StorageBufferInVertexShaderNoLayout) {
    wgpu::ShaderModule vsModuleWithStorageBuffer = utils::CreateShaderModule(device, R"(
        struct Dst {
            data : array<u32, 100>
        }
        @group(0) @binding(0) var<storage, read> dst : Dst;
        @vertex fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var val = dst.data[VertexIndex];
            return vec4f();
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = nullptr;
    descriptor.vertex.module = vsModuleWithStorageBuffer;
    descriptor.cFragment.module = fsModule;
    device.CreateRenderPipeline(&descriptor);
}

// Tests that only strip primitive topologies allow an index format
TEST_F(RenderPipelineValidationTest, StripIndexFormatAllowed) {
    constexpr uint32_t kNumStripType = 2u;
    constexpr uint32_t kNumListType = 3u;
    constexpr uint32_t kNumIndexFormat = 3u;

    std::array<wgpu::PrimitiveTopology, kNumStripType> kStripTopologyTypes = {
        {wgpu::PrimitiveTopology::LineStrip, wgpu::PrimitiveTopology::TriangleStrip}};

    std::array<wgpu::PrimitiveTopology, kNumListType> kListTopologyTypes = {
        {wgpu::PrimitiveTopology::PointList, wgpu::PrimitiveTopology::LineList,
         wgpu::PrimitiveTopology::TriangleList}};

    std::array<wgpu::IndexFormat, kNumIndexFormat> kIndexFormatTypes = {
        {wgpu::IndexFormat::Undefined, wgpu::IndexFormat::Uint16, wgpu::IndexFormat::Uint32}};

    for (wgpu::PrimitiveTopology primitiveTopology : kStripTopologyTypes) {
        for (wgpu::IndexFormat indexFormat : kIndexFormatTypes) {
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = vsModule;
            descriptor.cFragment.module = fsModule;
            descriptor.primitive.topology = primitiveTopology;
            descriptor.primitive.stripIndexFormat = indexFormat;

            // Always succeeds, regardless of if an index format is given.
            device.CreateRenderPipeline(&descriptor);
        }
    }

    for (wgpu::PrimitiveTopology primitiveTopology : kListTopologyTypes) {
        for (wgpu::IndexFormat indexFormat : kIndexFormatTypes) {
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = vsModule;
            descriptor.cFragment.module = fsModule;
            descriptor.primitive.topology = primitiveTopology;
            descriptor.primitive.stripIndexFormat = indexFormat;

            if (indexFormat == wgpu::IndexFormat::Undefined) {
                // Succeeds even when the index format is undefined because the
                // primitive topology isn't a strip type.
                device.CreateRenderPipeline(&descriptor);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
            }
        }
    }
}

// Test that setting unclippedDepth to true is an error if the feature is not enabled.
TEST_F(RenderPipelineValidationTest, UnclippedDepthWithoutFeature) {
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.unclippedDepth = false;
        device.CreateRenderPipeline(&descriptor);
    }
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.unclippedDepth = true;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor),
                            testing::HasSubstr("not supported"));
    }
}

// Test that depthStencil.depthCompare is required only for formats with depth.
TEST_F(RenderPipelineValidationTest, DepthCompareRequiredForFormatsWithDepth) {
    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;

    descriptor.cDepthStencil.depthWriteEnabled = wgpu::OptionalBool::True;
    descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth32Float);

    // Control case: Always is valid for format with depth.
    descriptor.cDepthStencil.depthCompare = wgpu::CompareFunction::Always;
    device.CreateRenderPipeline(&descriptor);

    // Error case: Undefined is invalid for format with depth.
    descriptor.cDepthStencil.depthCompare = wgpu::CompareFunction::Undefined;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));

    // Undefined is valid though if depthCompare is not used by anything.
    descriptor.cDepthStencil.depthWriteEnabled = wgpu::OptionalBool::False;
    descriptor.cDepthStencil.stencilFront.depthFailOp = wgpu::StencilOperation::Keep;
    descriptor.cDepthStencil.stencilBack.depthFailOp = wgpu::StencilOperation::Keep;
    device.CreateRenderPipeline(&descriptor);

    // Undefined is invalid if depthCompare is used by depthWriteEnabled.
    descriptor.cDepthStencil.depthWriteEnabled = wgpu::OptionalBool::True;
    descriptor.cDepthStencil.stencilFront.depthFailOp = wgpu::StencilOperation::Keep;
    descriptor.cDepthStencil.stencilBack.depthFailOp = wgpu::StencilOperation::Keep;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));

    // Undefined is invalid if depthCompare is used by stencilFront.depthFailOp.
    descriptor.cDepthStencil.depthWriteEnabled = wgpu::OptionalBool::False;
    descriptor.cDepthStencil.stencilFront.depthFailOp = wgpu::StencilOperation::Zero;
    descriptor.cDepthStencil.stencilBack.depthFailOp = wgpu::StencilOperation::Keep;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));

    // Undefined is invalid if depthCompare is used by stencilBack.depthFailOp.
    descriptor.cDepthStencil.depthWriteEnabled = wgpu::OptionalBool::False;
    descriptor.cDepthStencil.stencilFront.depthFailOp = wgpu::StencilOperation::Keep;
    descriptor.cDepthStencil.stencilBack.depthFailOp = wgpu::StencilOperation::Zero;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));

    descriptor.cDepthStencil.depthWriteEnabled = wgpu::OptionalBool::False;
    descriptor.cDepthStencil.stencilFront.depthFailOp = wgpu::StencilOperation::Keep;
    descriptor.cDepthStencil.stencilBack.depthFailOp = wgpu::StencilOperation::Keep;
    descriptor.EnableDepthStencil(wgpu::TextureFormat::Stencil8);

    // Always is valid for format with no depth.
    descriptor.cDepthStencil.depthCompare = wgpu::CompareFunction::Always;
    device.CreateRenderPipeline(&descriptor);

    // Undefined is also valid for format with no depth.
    descriptor.cDepthStencil.depthCompare = wgpu::CompareFunction::Undefined;
    device.CreateRenderPipeline(&descriptor);
}

// Test that depthStencil.depthWriteEnabled is required only for formats with depth.
TEST_F(RenderPipelineValidationTest, DepthWriteEnabledRequiredForFormatsWithDepth) {
    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.cDepthStencil.depthCompare = wgpu::CompareFunction::Always;

    wgpu::DepthStencilState* depthStencil =
        descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth32Float);

    // Control case: Set depthWriteEnabled to false for format with depth.
    depthStencil->depthWriteEnabled = wgpu::OptionalBool::False;
    device.CreateRenderPipeline(&descriptor);

    depthStencil = descriptor.EnableDepthStencil(wgpu::TextureFormat::Stencil8);

    // depthWriteEnabled set to undefined is valid for format with no depth.
    depthStencil->depthWriteEnabled = wgpu::OptionalBool::Undefined;
    device.CreateRenderPipeline(&descriptor);

    // depthWriteEnabled set to false is valid for format with no depth.
    depthStencil->depthWriteEnabled = wgpu::OptionalBool::False;
    device.CreateRenderPipeline(&descriptor);

    // Error case: depthWriteEnabled set to true is invalid for format with no depth.
    depthStencil->depthWriteEnabled = wgpu::OptionalBool::True;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));

    depthStencil = descriptor.EnableDepthStencil(wgpu::TextureFormat::Depth32Float);

    // depthWriteEnabled set to false is valid for format with depth.
    depthStencil->depthWriteEnabled = wgpu::OptionalBool::False;
    device.CreateRenderPipeline(&descriptor);

    // depthWriteEnabled set to true is valid for format with depth.
    depthStencil->depthWriteEnabled = wgpu::OptionalBool::True;
    device.CreateRenderPipeline(&descriptor);

    // Error case: depthWriteEnabled set to undefined is invalid for format with depth.
    depthStencil->depthWriteEnabled = wgpu::OptionalBool::Undefined;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test that the entryPoint names must be present for the correct stage in the shader module.
TEST_F(RenderPipelineValidationTest, EntryPointNameValidation) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vertex_main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }

        @fragment fn fragment_main() -> @location(0) vec4f {
            return vec4f(1.0, 0.0, 0.0, 1.0);
        }
    )");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = module;
    descriptor.vertex.entryPoint = "vertex_main";
    descriptor.cFragment.module = module;
    descriptor.cFragment.entryPoint = "fragment_main";

    // Success case.
    device.CreateRenderPipeline(&descriptor);

    // Test for the vertex stage entryPoint name.
    {
        // The entryPoint name doesn't exist in the module.
        descriptor.vertex.entryPoint = "main";
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));

        // The entryPoint name exists, but not for the correct stage.
        descriptor.vertex.entryPoint = "fragment_main";
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    descriptor.vertex.entryPoint = "vertex_main";

    // Test for the fragment stage entryPoint name.
    {
        // The entryPoint name doesn't exist in the module.
        descriptor.cFragment.entryPoint = "main";
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));

        // The entryPoint name exists, but not for the correct stage.
        descriptor.cFragment.entryPoint = "vertex_main";
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Check that entry points are optional.
TEST_F(RenderPipelineValidationTest, EntryPointNameOptional) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vertex_main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }

        @fragment fn fragment_main() -> @location(0) vec4f {
            return vec4f(1.0, 0.0, 0.0, 1.0);
        }
    )");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = module;
    descriptor.vertex.entryPoint = nullptr;
    descriptor.cFragment.module = module;
    descriptor.cFragment.entryPoint = nullptr;

    // Success case.
    device.CreateRenderPipeline(&descriptor);
}

// Check that entry points are required if module has multiple entry points.
TEST_F(RenderPipelineValidationTest, EntryPointNameRequiredIfMultipleEntryPoints) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vertex1() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }

        @vertex fn vertex2() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }

        @fragment fn fragment1() -> @location(0) vec4f {
            return vec4f(1.0, 0.0, 0.0, 1.0);
        }

        @fragment fn fragment2() -> @location(0) vec4f {
            return vec4f(1.0, 0.0, 0.0, 1.0);
        }
    )");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = module;
    descriptor.cFragment.module = module;

    {
        // The vertex stage has more than one entryPoint.
        descriptor.vertex.entryPoint = nullptr;
        descriptor.cFragment.entryPoint = "fragment1";
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    {
        // The fragment stage has more than one entryPoint.
        descriptor.vertex.entryPoint = "vertex1";
        descriptor.cFragment.entryPoint = nullptr;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Check that entry points are required if module has no compatible entry points.
TEST_F(RenderPipelineValidationTest, EntryPointNameRequiredIfNoCompatibleEntryPoints) {
    {
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            @fragment fn fragment_main() -> @location(0) vec4f {
                return vec4f(1.0, 0.0, 0.0, 1.0);
            }
        )");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = module;
        descriptor.cFragment.module = module;

        // The vertex stage has no entryPoint.
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    {
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            @vertex fn vertex_main() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 1.0);
            }
        )");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = module;
        descriptor.cFragment.module = module;

        // The fragment stage has no entryPoint.
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Test that vertex attrib validation is for the correct entryPoint
TEST_F(RenderPipelineValidationTest, VertexAttribCorrectEntryPoint) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vertex0(@location(0) attrib0 : vec4f)
                                    -> @builtin(position) vec4f {
            return attrib0;
        }
        @vertex fn vertex1(@location(1) attrib1 : vec4f)
                                    -> @builtin(position) vec4f {
            return attrib1;
        }
    )");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = module;
    descriptor.cFragment.module = fsModule;

    descriptor.vertex.bufferCount = 1;
    descriptor.cBuffers[0].attributeCount = 1;
    descriptor.cBuffers[0].arrayStride = 16;
    descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
    descriptor.cAttributes[0].offset = 0;

    // Success cases, the attribute used by the entryPoint is declared in the pipeline.
    descriptor.vertex.entryPoint = "vertex0";
    descriptor.cAttributes[0].shaderLocation = 0;
    device.CreateRenderPipeline(&descriptor);

    descriptor.vertex.entryPoint = "vertex1";
    descriptor.cAttributes[0].shaderLocation = 1;
    device.CreateRenderPipeline(&descriptor);

    // Error cases, the attribute used by the entryPoint isn't declared in the pipeline.
    descriptor.vertex.entryPoint = "vertex1";
    descriptor.cAttributes[0].shaderLocation = 0;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));

    descriptor.vertex.entryPoint = "vertex0";
    descriptor.cAttributes[0].shaderLocation = 1;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test that fragment output validation is for the correct entryPoint
TEST_F(RenderPipelineValidationTest, FragmentOutputCorrectEntryPoint) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @fragment fn fragmentFloat() -> @location(0) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        }
        @fragment fn fragmentUint() -> @location(0) vec4u {
            return vec4u(0u, 0u, 0u, 0u);
        }
    )");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = module;

    // Success case, the component type matches between the pipeline and the entryPoint
    descriptor.cFragment.entryPoint = "fragmentFloat";
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA32Float;
    device.CreateRenderPipeline(&descriptor);

    descriptor.cFragment.entryPoint = "fragmentUint";
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA32Uint;
    device.CreateRenderPipeline(&descriptor);

    // Error case, the component type doesn't match between the pipeline and the entryPoint
    descriptor.cFragment.entryPoint = "fragmentUint";
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA32Float;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));

    descriptor.cFragment.entryPoint = "fragmentFloat";
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA32Uint;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Test that unwritten fragment outputs must have a write mask of 0.
TEST_F(RenderPipelineValidationTest, UnwrittenFragmentOutputsMask0) {
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f();
        }
    )");

    wgpu::ShaderModule fsModuleWriteNone = utils::CreateShaderModule(device, R"(
        @fragment fn main() {}
    )");

    wgpu::ShaderModule fsModuleWrite0 = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f();
        }
    )");

    wgpu::ShaderModule fsModuleWrite1 = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(1) vec4f {
            return vec4f();
        }
    )");

    wgpu::ShaderModule fsModuleWriteBoth = utils::CreateShaderModule(device, R"(
        struct FragmentOut {
            @location(0) target0 : vec4f,
            @location(1) target1 : vec4f,
        }
        @fragment fn main() -> FragmentOut {
            var out : FragmentOut;
            return out;
        }
    )");

    // Control case: write to target 0
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        descriptor.cFragment.targetCount = 1;
        descriptor.cFragment.module = fsModuleWrite0;
        device.CreateRenderPipeline(&descriptor);
    }

    // Control case: write to target 0 and target 1
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        descriptor.cFragment.targetCount = 2;
        descriptor.cFragment.module = fsModuleWriteBoth;
        device.CreateRenderPipeline(&descriptor);
    }

    // Write only target 1 (not in pipeline fragment state).
    // Errors because target 0 does not have a write mask of 0.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        descriptor.cFragment.targetCount = 1;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::All;
        descriptor.cFragment.module = fsModuleWrite1;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    // Write only target 1 (not in pipeline fragment state).
    // OK because target 0 has a write mask of 0.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        descriptor.cFragment.targetCount = 1;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        descriptor.cFragment.module = fsModuleWrite1;
        device.CreateRenderPipeline(&descriptor);
    }

    // Write only target 0 with two color targets.
    // Errors because target 1 does not have a write mask of 0.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        descriptor.cFragment.targetCount = 2;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::Red;
        descriptor.cTargets[1].writeMask = wgpu::ColorWriteMask::Alpha;
        descriptor.cFragment.module = fsModuleWrite0;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    // Write only target 0 with two color targets.
    // OK because target 1 has a write mask of 0.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        descriptor.cFragment.targetCount = 2;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::All;
        descriptor.cTargets[1].writeMask = wgpu::ColorWriteMask::None;
        descriptor.cFragment.module = fsModuleWrite0;
        device.CreateRenderPipeline(&descriptor);
    }

    // Write nothing with two color targets.
    // Errors because both target 0 and 1 have nonzero write masks.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        descriptor.cFragment.targetCount = 2;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::Red;
        descriptor.cTargets[1].writeMask = wgpu::ColorWriteMask::Green;
        descriptor.cFragment.module = fsModuleWriteNone;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    // Write nothing with two color targets.
    // OK because target 0 and 1 have write masks of 0.
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;

        descriptor.cFragment.targetCount = 2;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        descriptor.cTargets[1].writeMask = wgpu::ColorWriteMask::None;
        descriptor.cFragment.module = fsModuleWriteNone;
        device.CreateRenderPipeline(&descriptor);
    }
}

// Test that fragment output validation is for the correct entryPoint
TEST_F(RenderPipelineValidationTest, BindingsFromCorrectEntryPoint) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct Uniforms {
            data : vec4f
        }
        @group(0) @binding(0) var<uniform> var0 : Uniforms;
        @group(0) @binding(1) var<uniform> var1 : Uniforms;

        @vertex fn vertex0() -> @builtin(position) vec4f {
            return var0.data;
        }
        @vertex fn vertex1() -> @builtin(position) vec4f {
            return var1.data;
        }
    )");

    wgpu::BindGroupLayout bgl0 = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform}});
    wgpu::PipelineLayout layout0 = utils::MakeBasicPipelineLayout(device, &bgl0);

    wgpu::BindGroupLayout bgl1 = utils::MakeBindGroupLayout(
        device, {{1, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform}});
    wgpu::PipelineLayout layout1 = utils::MakeBasicPipelineLayout(device, &bgl1);

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = module;
    descriptor.cFragment.module = fsModule;

    // Success case, the BGL matches the bindings used by the entryPoint
    descriptor.vertex.entryPoint = "vertex0";
    descriptor.layout = layout0;
    device.CreateRenderPipeline(&descriptor);

    descriptor.vertex.entryPoint = "vertex1";
    descriptor.layout = layout1;
    device.CreateRenderPipeline(&descriptor);

    // Error case, the BGL doesn't match the bindings used by the entryPoint
    descriptor.vertex.entryPoint = "vertex1";
    descriptor.layout = layout0;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));

    descriptor.vertex.entryPoint = "vertex0";
    descriptor.layout = layout1;
    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
}

// Tests validation for per-pixel accounting for render targets. The tests currently assume that the
// default maxColorAttachmentBytesPerSample limit of 32 is used.
TEST_F(RenderPipelineValidationTest, RenderPipelineColorAttachmentBytesPerSample) {
    // Creates a fragment shader with maximum number of color attachments to enable testing.
    auto CreateShader = [&](const std::vector<wgpu::TextureFormat>& formats) -> wgpu::ShaderModule {
        // Default type to use when formats.size() < kMaxColorAttachments.
        static constexpr std::string_view kDefaultWgslType = "vec4f";

        std::ostringstream bindings;
        std::ostringstream outputs;
        for (size_t i = 0; i < kMaxColorAttachments; i++) {
            if (i < formats.size()) {
                std::ostringstream type;
                type << "vec4<" << utils::GetWGSLColorTextureComponentTypeStr(formats.at(i)) << ">";
                bindings << "@location(" << i << ") o" << i << " : " << type.str() << ", ";
                outputs << type.str() << "(1), ";
            } else {
                bindings << "@location(" << i << ") o" << i << " : " << kDefaultWgslType << ", ";
                outputs << kDefaultWgslType << "(1), ";
            }
        }

        std::ostringstream fsShader;
        fsShader << "struct Outputs { " << bindings.str() << "}\n";
        fsShader << "@fragment fn main() -> Outputs {\n";
        fsShader << "    return Outputs(" << outputs.str() << ");\n";
        fsShader << "}";
        return utils::CreateShaderModule(device, fsShader.str().c_str());
    };

    struct TestCase {
        std::vector<wgpu::TextureFormat> formats;
        bool success;
    };
    static std::vector<TestCase> kTestCases = {
        // Simple 1 format cases.

        // R8Unorm take 1 byte and are aligned to 1 byte so we can have 8 (max).
        {{wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm,
          wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm,
          wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm},
         true},
        // RGBA8Uint takes 4 bytes and are aligned to 1 byte so we can have 8 (max).
        {{wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint,
          wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint,
          wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint,
          wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint},
         true},
        // RGBA8Unorm takes 8 bytes (special case) and are aligned to 1 byte so only 4 allowed.
        {{wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm},
         true},
        {{wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA8Unorm},
         false},
        // RGBA32Float takes 16 bytes and are aligned to 4 bytes so only 2 are allowed.
        {{wgpu::TextureFormat::RGBA32Float, wgpu::TextureFormat::RGBA32Float}, true},
        {{wgpu::TextureFormat::RGBA32Float, wgpu::TextureFormat::RGBA32Float,
          wgpu::TextureFormat::RGBA32Float},
         false},

        // Different format alignment cases.

        // Alignment causes the first 1 byte R8Unorm to become 4 bytes. So even though 1+4+8+16+1 <
        // 32, the 4 byte alignment requirement of R32Float makes the first R8Unorm become 4 and
        // 4+4+8+16+1 > 32. Re-ordering this so the R8Unorm's are at the end, however is allowed:
        // 4+8+16+1+1 < 32.
        {{wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R32Float,
          wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA32Float,
          wgpu::TextureFormat::R8Unorm},
         false},
        {{wgpu::TextureFormat::R32Float, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA32Float, wgpu::TextureFormat::R8Unorm,
          wgpu::TextureFormat::R8Unorm},
         true},
    };

    for (const TestCase& testCase : kTestCases) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = CreateShader(testCase.formats);
        descriptor.cFragment.targetCount = testCase.formats.size();
        for (size_t i = 0; i < testCase.formats.size(); i++) {
            descriptor.cTargets[i].format = testCase.formats.at(i);
        }
        if (testCase.success) {
            device.CreateRenderPipeline(&descriptor);
        } else {
            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
        }
    }
}

// Creating render pipeline with ColorTargetStateExpandResolveTextureDawn without enabling
// LoadResolveTexture feature should result in error.
TEST_F(RenderPipelineValidationTest, LoadResolveTextureOnUnsupportedDevice) {
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsModule;
    pipelineDescriptor.multisample.count = 4;

    wgpu::ColorTargetStateExpandResolveTextureDawn pipelineMSAAExpandResolveDesc;
    pipelineMSAAExpandResolveDesc.enabled = true;
    pipelineDescriptor.cTargets[0].nextInChain = &pipelineMSAAExpandResolveDesc;

    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&pipelineDescriptor),
                        testing::HasSubstr("feature is not enabled"));
}

/// Tests that depthBias can't be used with point or line topologies.
TEST_F(RenderPipelineValidationTest, PointLineDepthBias) {
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;

        device.CreateRenderPipeline(&descriptor);
    }

    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.multisample.count = 3;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Regression test for 377530684 where we try to access the current pipeline in
// CommandBufferStateTracker even after the pass is ended because a validation failure in End() made
// the RenderPass only partially closed, and further commands could keep pass the TryEncode
// validation.
TEST_F(RenderPipelineValidationTest, DrawAfterEndShouldNotAccessPipeline) {
    // A pipeline and texture used for rendering.
    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = vsModule;
    pDesc.cFragment.module = fsModule;
    pDesc.cFragment.targetCount = 1;
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    wgpu::TextureDescriptor tDesc;
    tDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    tDesc.size = {1, 1};
    tDesc.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture texture = device.CreateTexture(&tDesc);

    // The issue was found with DrawIndirect that gets the currently bound pipeline to know if
    // vertex duplication is necessary.
    wgpu::BufferDescriptor bDesc;
    bDesc.size = 64;
    bDesc.usage = wgpu::BufferUsage::Indirect;
    wgpu::Buffer buffer = device.CreateBuffer(&bDesc);

    // Cause an End() validation failure with maxDrawCount
    utils::ComboRenderPassDescriptor rpDesc{{texture.CreateView()}};
    wgpu::RenderPassMaxDrawCount maxDrawCount;
    maxDrawCount.maxDrawCount = 1;
    rpDesc.nextInChain = &maxDrawCount;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rpDesc);
    pass.SetPipeline(pipeline);
    pass.DrawIndirect(buffer, 0);
    pass.DrawIndirect(buffer, 0);
    pass.End();

    // This call shouldn't access any CommandBufferStateTracker state but prior to the fix, it would
    // and cause an ASSERT() to trigger.
    pass.DrawIndirect(buffer, 0);
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

class DepthClipControlValidationTest : public RenderPipelineValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::DepthClipControl};
    }
};

// Tests that specifying a unclippedDepth value succeeds if the feature is enabled.
TEST_F(DepthClipControlValidationTest, Success) {
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.unclippedDepth = true;
        device.CreateRenderPipeline(&descriptor);
    }
    {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.unclippedDepth = false;
        device.CreateRenderPipeline(&descriptor);
    }
}

class InterStageVariableMatchingValidationTest : public RenderPipelineValidationTest {
  protected:
    void CheckCreatingRenderPipeline(wgpu::ShaderModule vertexModule,
                                     wgpu::ShaderModule fragmentModule,
                                     bool shouldSucceed) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vertexModule;
        descriptor.cFragment.module = fragmentModule;
        if (shouldSucceed) {
            device.CreateRenderPipeline(&descriptor);
        } else {
            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
        }
    }
};

// Tests that creating render pipeline should fail when there is a fragment input that
// doesn't have its corresponding vertex output at the same location.
TEST_F(InterStageVariableMatchingValidationTest, MissingDeclarationAtSameLocation) {
    wgpu::ShaderModule vertexModuleOutputAtLocation0 = utils::CreateShaderModule(device, R"(
            struct A {
                @location(0) vout: f32,
                @builtin(position) pos: vec4f,
            }
            @vertex fn main() -> A {
                var vertexOut: A;
                vertexOut.pos = vec4f(0.0, 0.0, 0.0, 1.0);
                return vertexOut;
            })");
    wgpu::ShaderModule fragmentModuleAtLocation0 = utils::CreateShaderModule(device, R"(
            struct B {
                @location(0) fin: f32
            }
            @fragment fn main(fragmentIn: B) -> @location(0) vec4f  {
                return vec4f(fragmentIn.fin, 0.0, 0.0, 1.0);
            })");
    wgpu::ShaderModule fragmentModuleInputAtLocation1 = utils::CreateShaderModule(device, R"(
            struct A {
                @location(1) vout: f32
            }
            @fragment fn main(vertexOut: A) -> @location(0) vec4f  {
                return vec4f(vertexOut.vout, 0.0, 0.0, 1.0);
            })");
    wgpu::ShaderModule vertexModuleOutputAtLocation1 = utils::CreateShaderModule(device, R"(
            struct B {
                @location(1) fin: f32,
                @builtin(position) pos: vec4f,
            }
            @vertex fn main() -> B {
                var fragmentIn: B;
                fragmentIn.pos = vec4f(0.0, 0.0, 0.0, 1.0);
                return fragmentIn;
            })");

    {
        // It is okay if the fragment output is a subset of the vertex input.
        CheckCreatingRenderPipeline(vertexModuleOutputAtLocation0, fsModule, true);
    }
    {
        CheckCreatingRenderPipeline(vsModule, fragmentModuleAtLocation0, false);
        CheckCreatingRenderPipeline(vertexModuleOutputAtLocation0, fragmentModuleInputAtLocation1,
                                    false);
        CheckCreatingRenderPipeline(vertexModuleOutputAtLocation1, fragmentModuleAtLocation0,
                                    false);
    }

    {
        CheckCreatingRenderPipeline(vertexModuleOutputAtLocation0, fragmentModuleAtLocation0, true);
        CheckCreatingRenderPipeline(vertexModuleOutputAtLocation1, fragmentModuleInputAtLocation1,
                                    true);
    }
}

// Tests that creating render pipeline should fail when the type of a vertex stage output variable
// doesn't match the type of the fragment stage input variable at the same location.
TEST_F(InterStageVariableMatchingValidationTest, DifferentTypeAtSameLocation) {
    constexpr std::array<const char*, 16> kTypes = {
        {"f32", "vec2f", "vec3f", "vec4f", "f16", "vec2<f16>", "vec3<f16>", "vec4<f16>", "i32",
         "vec2i", "vec3i", "vec4i", "u32", "vec2u", "vec3u", "vec4u"}};

    std::array<wgpu::ShaderModule, 16> vertexModules;
    std::array<wgpu::ShaderModule, 16> fragmentModules;
    for (uint32_t i = 0; i < kTypes.size(); ++i) {
        std::string interfaceDeclaration;
        {
            std::ostringstream sstream;
            sstream << "struct A { @location(0) @interpolate(flat, either) a: " << kTypes[i]
                    << ",\n";
            interfaceDeclaration = sstream.str();
        }

        std::string extensionDeclaration = "enable f16;\n\n";

        {
            std::ostringstream vertexStream;
            vertexStream << extensionDeclaration << interfaceDeclaration << R"(
                    @builtin(position) pos: vec4f,
                }
                @vertex fn main() -> A {
                    var vertexOut: A;
                    vertexOut.pos = vec4f(0.0, 0.0, 0.0, 1.0);
                    return vertexOut;
                })";
            vertexModules[i] = utils::CreateShaderModule(device, vertexStream.str().c_str());
        }
        {
            std::ostringstream fragmentStream;
            fragmentStream << extensionDeclaration << interfaceDeclaration << R"(
                }
                @fragment fn main(fragmentIn: A) -> @location(0) vec4f {
                    return vec4f(0.0, 0.0, 0.0, 1.0);
                })";
            fragmentModules[i] = utils::CreateShaderModule(device, fragmentStream.str().c_str());
        }
    }

    for (uint32_t vertexModuleIndex = 0; vertexModuleIndex < kTypes.size(); ++vertexModuleIndex) {
        wgpu::ShaderModule vertexModule = vertexModules[vertexModuleIndex];
        for (uint32_t fragmentModuleIndex = 0; fragmentModuleIndex < kTypes.size();
             ++fragmentModuleIndex) {
            wgpu::ShaderModule fragmentModule = fragmentModules[fragmentModuleIndex];
            bool shouldSuccess = vertexModuleIndex == fragmentModuleIndex;
            CheckCreatingRenderPipeline(vertexModule, fragmentModule, shouldSuccess);
        }
    }
}

// Tests that creating render pipeline should fail when the interpolation attribute of a vertex
// stage output variable doesn't match the type of the fragment stage input variable at the same
// location.
TEST_F(InterStageVariableMatchingValidationTest, DifferentInterpolationAttributeAtSameLocation) {
    enum class InterpolationType : uint8_t {
        None = 0,
        Perspective,
        Linear,
        Flat,
        Count,
    };
    enum class InterpolationSampling : uint8_t {
        None = 0,
        Center,
        Centroid,
        Sample,
        First,
        Either,
        Count,
    };
    constexpr std::array<const char*, static_cast<size_t>(InterpolationType::Count)>
        kInterpolationTypeString = {{"", "perspective", "linear", "flat"}};
    constexpr std::array<const char*, static_cast<size_t>(InterpolationSampling::Count)>
        kInterpolationSamplingString = {{"", "center", "centroid", "sample", "first", "either"}};

    struct InterpolationAttribute {
        InterpolationType interpolationType;
        InterpolationSampling interpolationSampling;
    };

    constexpr std::array<InterpolationAttribute, 12> validInterpolationAttributes = {{
        {InterpolationType::None, InterpolationSampling::None},
        {InterpolationType::Flat, InterpolationSampling::None},
        {InterpolationType::Flat, InterpolationSampling::First},
        {InterpolationType::Flat, InterpolationSampling::Either},
        {InterpolationType::Linear, InterpolationSampling::None},
        {InterpolationType::Linear, InterpolationSampling::Center},
        {InterpolationType::Linear, InterpolationSampling::Centroid},
        {InterpolationType::Linear, InterpolationSampling::Sample},
        {InterpolationType::Perspective, InterpolationSampling::None},
        {InterpolationType::Perspective, InterpolationSampling::Center},
        {InterpolationType::Perspective, InterpolationSampling::Centroid},
        {InterpolationType::Perspective, InterpolationSampling::Sample},
    }};

    std::vector<wgpu::ShaderModule> vertexModules(validInterpolationAttributes.size());
    std::vector<wgpu::ShaderModule> fragmentModules(validInterpolationAttributes.size());
    for (uint32_t i = 0; i < validInterpolationAttributes.size(); ++i) {
        std::string interfaceDeclaration;
        {
            const auto& interpolationAttribute = validInterpolationAttributes[i];
            std::ostringstream sstream;
            sstream << "struct A { @location(0)";
            if (interpolationAttribute.interpolationType != InterpolationType::None) {
                sstream << " @interpolate("
                        << kInterpolationTypeString[static_cast<uint8_t>(
                               interpolationAttribute.interpolationType)];
                if (interpolationAttribute.interpolationSampling != InterpolationSampling::None) {
                    sstream << ", "
                            << kInterpolationSamplingString[static_cast<uint8_t>(
                                   interpolationAttribute.interpolationSampling)];
                }
                sstream << ")";
            }
            sstream << " a : vec4f,\n";
            interfaceDeclaration = sstream.str();
        }
        {
            std::ostringstream vertexStream;
            vertexStream << interfaceDeclaration << R"(
                    @builtin(position) pos: vec4f,
                }
                @vertex fn main() -> A {
                    var vertexOut: A;
                    vertexOut.pos = vec4f(0.0, 0.0, 0.0, 1.0);
                    return vertexOut;
                })";
            vertexModules[i] = utils::CreateShaderModule(device, vertexStream.str().c_str());
        }
        {
            std::ostringstream fragmentStream;
            fragmentStream << interfaceDeclaration << R"(
                }
                @fragment fn main(fragmentIn: A) -> @location(0) vec4f {
                    return fragmentIn.a;
                })";
            fragmentModules[i] = utils::CreateShaderModule(device, fragmentStream.str().c_str());
        }
    }

    auto GetAppliedInterpolationAttribute = [](const InterpolationAttribute& attribute) {
        InterpolationAttribute appliedAttribute = {attribute.interpolationType,
                                                   attribute.interpolationSampling};
        switch (attribute.interpolationType) {
            // If the interpolation attribute is not specified, then
            // @interpolate(perspective, center) or @interpolate(perspective) is assumed.
            case InterpolationType::None:
                appliedAttribute.interpolationType = InterpolationType::Perspective;
                appliedAttribute.interpolationSampling = InterpolationSampling::Center;
                break;

            // If the interpolation type is perspective or linear, and the interpolation
            // sampling is not specified, then 'center' is assumed.
            case InterpolationType::Perspective:
            case InterpolationType::Linear:
                if (appliedAttribute.interpolationSampling == InterpolationSampling::None) {
                    appliedAttribute.interpolationSampling = InterpolationSampling::Center;
                }
                break;

            case InterpolationType::Flat:
                if (appliedAttribute.interpolationSampling == InterpolationSampling::None) {
                    appliedAttribute.interpolationSampling = InterpolationSampling::First;
                }
                break;
            default:
                DAWN_UNREACHABLE();
        }
        return appliedAttribute;
    };

    auto InterpolationAttributeMatch = [GetAppliedInterpolationAttribute](
                                           const InterpolationAttribute& attribute1,
                                           const InterpolationAttribute& attribute2) {
        InterpolationAttribute appliedAttribute1 = GetAppliedInterpolationAttribute(attribute1);
        InterpolationAttribute appliedAttribute2 = GetAppliedInterpolationAttribute(attribute2);

        return appliedAttribute1.interpolationType == appliedAttribute2.interpolationType &&
               appliedAttribute1.interpolationSampling == appliedAttribute2.interpolationSampling;
    };

    for (uint32_t vertexModuleIndex = 0; vertexModuleIndex < validInterpolationAttributes.size();
         ++vertexModuleIndex) {
        wgpu::ShaderModule vertexModule = vertexModules[vertexModuleIndex];
        for (uint32_t fragmentModuleIndex = 0;
             fragmentModuleIndex < validInterpolationAttributes.size(); ++fragmentModuleIndex) {
            wgpu::ShaderModule fragmentModule = fragmentModules[fragmentModuleIndex];
            bool shouldSuccess =
                InterpolationAttributeMatch(validInterpolationAttributes[vertexModuleIndex],
                                            validInterpolationAttributes[fragmentModuleIndex]);
            CheckCreatingRenderPipeline(vertexModule, fragmentModule, shouldSuccess);
        }
    }
}

class RenderPipelineTransientAttachmentValidationTest : public RenderPipelineValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::ShaderF16, wgpu::FeatureName::TransientAttachments};
    }
};

// Test case where creation should succeed.
TEST_F(RenderPipelineTransientAttachmentValidationTest, CreationSuccess) {
    constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.usage =
        wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TransientAttachment;
    textureDescriptor.format = kColorFormat;
    textureDescriptor.size.width = 4;
    textureDescriptor.size.height = 4;

    wgpu::Texture transientTexture = device.CreateTexture(&textureDescriptor);
    utils::ComboRenderPassDescriptor renderPassDescriptor({transientTexture.CreateView()});

    // Set load and store ops to supported values with transient attachments.
    renderPassDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
    renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsModule;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    renderPass.SetPipeline(pipeline);
    renderPass.End();

    encoder.Finish();
}

// Creation of a pipeline that stores into a transient attachment should cause
// an error.
TEST_F(RenderPipelineTransientAttachmentValidationTest, StoreCausesError) {
    constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.usage =
        wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TransientAttachment;
    textureDescriptor.format = kColorFormat;
    textureDescriptor.size.width = 4;
    textureDescriptor.size.height = 4;

    wgpu::Texture transientTexture = device.CreateTexture(&textureDescriptor);
    utils::ComboRenderPassDescriptor renderPassDescriptor({transientTexture.CreateView()});

    renderPassDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
    renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsModule;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    renderPass.SetPipeline(pipeline);
    renderPass.End();

    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Creation of a pipeline that loads from a transient attachment should cause
// an error.
TEST_F(RenderPipelineTransientAttachmentValidationTest, LoadCausesError) {
    constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.usage =
        wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TransientAttachment;
    textureDescriptor.format = kColorFormat;
    textureDescriptor.size.width = 4;
    textureDescriptor.size.height = 4;

    wgpu::Texture transientTexture = device.CreateTexture(&textureDescriptor);
    utils::ComboRenderPassDescriptor renderPassDescriptor({transientTexture.CreateView()});

    renderPassDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
    renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Load;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsModule;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    renderPass.SetPipeline(pipeline);
    renderPass.End();

    ASSERT_DEVICE_ERROR(encoder.Finish());
}

class LoadResolveTexturePipelineDescriptorValidationTest : public RenderPipelineValidationTest {
  protected:
    void SetUp() override {
        RenderPipelineValidationTest::SetUp();

        fsWithTextureModule = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var src_tex : texture_2d<f32>;

            @fragment fn main() -> @location(0) vec4f {
                return textureLoad(src_tex, vec2u(0, 0), 0);
            })");

        fsWithTextureToTarget1Module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var src_tex : texture_2d<f32>;

            @fragment fn main() -> @location(1) vec4f {
                return textureLoad(src_tex, vec2u(0, 0), 0);
            })");

        fs2TargetsModule = utils::CreateShaderModule(device, R"(
            struct FragmentOut {
                @location(0) color0 : vec4f,
                @location(1) color1 : vec4f,
            }

            @fragment fn main() -> FragmentOut {
                var output : FragmentOut;
                output.color0 = vec4f(0.85);
                output.color1 = output.color0 * 0.5;
                return output;
            })");
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::DawnLoadResolveTexture};
    }

    wgpu::Texture CreateTexture(wgpu::TextureUsage textureUsage, uint32_t sampleCount) {
        wgpu::TextureDescriptor textureDescriptor;
        textureDescriptor.usage = textureUsage;
        textureDescriptor.format = kColorFormat;
        textureDescriptor.sampleCount = sampleCount;
        textureDescriptor.size.width = 4;
        textureDescriptor.size.height = 4;
        return device.CreateTexture(&textureDescriptor);
    }

    static constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::ShaderModule fsWithTextureModule;
    wgpu::ShaderModule fsWithTextureToTarget1Module;
    wgpu::ShaderModule fs2TargetsModule;
};

// Test that creating and using a render pipeline with ColorTargetStateExpandResolveTextureDawn
// chained struct should success.
TEST_F(LoadResolveTexturePipelineDescriptorValidationTest, ValidUse) {
    constexpr uint32_t kSampleCount = 4;

    auto msaaTexture = CreateTexture(wgpu::TextureUsage::RenderAttachment, kSampleCount);

    // Create single sampled texture.
    auto texture =
        CreateTexture(wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding, 1);

    // Create render pass (with ExpandResolveTexture load op).
    utils::ComboRenderPassDescriptor renderPassDescriptor({msaaTexture.CreateView()});
    renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
    renderPassDescriptor.cColorAttachments[0].resolveTarget = texture.CreateView();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);

    // Create render pipeline
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsWithTextureModule;
    pipelineDescriptor.multisample.count = kSampleCount;

    wgpu::ColorTargetStateExpandResolveTextureDawn pipelineMSAAExpandResolveDesc;
    pipelineMSAAExpandResolveDesc.enabled = true;
    pipelineDescriptor.cTargets[0].nextInChain = &pipelineMSAAExpandResolveDesc;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    // Input texture.
    auto sampledTexture = CreateTexture(wgpu::TextureUsage::TextureBinding, 1);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, sampledTexture.CreateView()}});

    renderPass.SetPipeline(pipeline);
    renderPass.SetBindGroup(0, bindGroup);
    renderPass.Draw(3);
    renderPass.End();

    encoder.Finish();
}

// Test that creating and using a render pipeline with ColorTargetStateExpandResolveTextureDawn
// chained struct in a non-zero indexed attachment should success.
TEST_F(LoadResolveTexturePipelineDescriptorValidationTest, UseInNonZeroIndexedAttachment) {
    constexpr uint32_t kSampleCount = 4;

    auto msaaTexture = CreateTexture(wgpu::TextureUsage::RenderAttachment, kSampleCount);

    // Create single sampled texture.
    auto texture =
        CreateTexture(wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding, 1);

    // Create render pass (with ExpandResolveTexture load op).
    utils::ComboRenderPassDescriptor renderPassDescriptor({nullptr, msaaTexture.CreateView()});
    renderPassDescriptor.cColorAttachments[1].loadOp = wgpu::LoadOp::ExpandResolveTexture;
    renderPassDescriptor.cColorAttachments[1].resolveTarget = texture.CreateView();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);

    // Create render pipeline
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsWithTextureToTarget1Module;
    pipelineDescriptor.multisample.count = kSampleCount;
    pipelineDescriptor.cFragment.targetCount = 2;
    pipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::Undefined;
    pipelineDescriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;
    pipelineDescriptor.cTargets[1].format = kColorFormat;
    pipelineDescriptor.cTargets[1].writeMask = wgpu::ColorWriteMask::All;

    wgpu::ColorTargetStateExpandResolveTextureDawn pipelineMSAAExpandResolveDesc;
    pipelineMSAAExpandResolveDesc.enabled = true;
    pipelineDescriptor.cTargets[1].nextInChain = &pipelineMSAAExpandResolveDesc;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    // Input texture.
    auto sampledTexture = CreateTexture(wgpu::TextureUsage::TextureBinding, 1);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, sampledTexture.CreateView()}});

    renderPass.SetPipeline(pipeline);
    renderPass.SetBindGroup(0, bindGroup);
    renderPass.Draw(3);
    renderPass.End();

    encoder.Finish();
}

// If a render pipeline's MultisampleState contains ColorTargetStateExpandResolveTextureDawn
// chained struct. Then its sampleCount must be > 1.
TEST_F(LoadResolveTexturePipelineDescriptorValidationTest,
       PipelineSampleCountMustBeGreaterThanOne) {
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsModule;
    pipelineDescriptor.multisample.count = 1;

    wgpu::ColorTargetStateExpandResolveTextureDawn pipelineMSAAExpandResolveDesc;
    pipelineMSAAExpandResolveDesc.enabled = true;
    pipelineDescriptor.cTargets[0].nextInChain = &pipelineMSAAExpandResolveDesc;

    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&pipelineDescriptor),
                        testing::HasSubstr("multisample count (1) is not > 1"));
}

// If a render pipeline is created with ColorTargetStateExpandResolveTextureDawn.enabled =
// true, then it cannot be used in a render pass that wasn't created with ExpandResolveTexture load
// op.
TEST_F(LoadResolveTexturePipelineDescriptorValidationTest,
       ExpandResolveTexturePipeline_UseIn_NormalRenderPass_Error) {
    constexpr uint32_t kSampleCount = 4;

    // Create MSAA texture.
    auto texture = CreateTexture(wgpu::TextureUsage::RenderAttachment, 4);

    // Create render pass (without ExpandResolveTexture load op).
    utils::ComboRenderPassDescriptor renderPassDescriptor({texture.CreateView()});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);

    // Create render pipeline
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsModule;
    pipelineDescriptor.multisample.count = kSampleCount;

    wgpu::ColorTargetStateExpandResolveTextureDawn pipelineMSAAExpandResolveDesc;
    pipelineMSAAExpandResolveDesc.enabled = true;
    pipelineDescriptor.cTargets[0].nextInChain = &pipelineMSAAExpandResolveDesc;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);
    renderPass.SetPipeline(pipeline);
    renderPass.End();

    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Using a normal render pipeline in a ExpandResolveTexture render pass should result in
// incompatible error.
TEST_F(LoadResolveTexturePipelineDescriptorValidationTest,
       NormalPipeline_Use_In_ExpandResolveTextureRenderPass_Error) {
    constexpr uint32_t kSampleCount = 4;

    // Create multi sampled texture.
    auto msaaTexture = CreateTexture(wgpu::TextureUsage::RenderAttachment, kSampleCount);

    // Create single sampled texture.
    auto texture =
        CreateTexture(wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding, 1);

    // Create render pass (with ExpandResolveTexture load op).
    utils::ComboRenderPassDescriptor renderPassDescriptor({msaaTexture.CreateView()});
    renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
    renderPassDescriptor.cColorAttachments[0].resolveTarget = texture.CreateView();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);

    // Create render pipeline (without ColorTargetStateExpandResolveTextureDawn)
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsModule;

    pipelineDescriptor.multisample.count = kSampleCount;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);
    renderPass.SetPipeline(pipeline);
    renderPass.End();

    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Test that the following scenario works:
// - Render pipeline and render pass both have two color outputs with resolve targets.
// - Only second output uses ExpandResolveTexture.
TEST_F(LoadResolveTexturePipelineDescriptorValidationTest,
       RenderPipelineAndRenderPassTwoOuputsLoadColor1) {
    constexpr uint32_t kSampleCount = 4;

    auto msaaTexture1 = CreateTexture(wgpu::TextureUsage::RenderAttachment, kSampleCount);
    auto msaaTexture2 = CreateTexture(wgpu::TextureUsage::RenderAttachment, kSampleCount);

    // Create single sampled textures.
    auto texture1 =
        CreateTexture(wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding, 1);
    auto texture2 =
        CreateTexture(wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding, 1);

    // Create render pass with two outputs. Second one uses ExpandResolveTexture.
    utils::ComboRenderPassDescriptor renderPassDescriptor(
        {msaaTexture1.CreateView(), msaaTexture2.CreateView()});
    renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Load;
    renderPassDescriptor.cColorAttachments[0].resolveTarget = texture1.CreateView();
    renderPassDescriptor.cColorAttachments[1].loadOp = wgpu::LoadOp::ExpandResolveTexture;
    renderPassDescriptor.cColorAttachments[1].resolveTarget = texture2.CreateView();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);

    // Create render pipeline with two chained ColorTargetStateExpandResolveTextureDawn.
    // The first one's enabled flag = true.
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fs2TargetsModule;
    pipelineDescriptor.multisample.count = kSampleCount;

    wgpu::ColorTargetStateExpandResolveTextureDawn pipelineMSAAExpandResolveDesc[2];
    pipelineMSAAExpandResolveDesc[0].enabled = false;
    pipelineDescriptor.cTargets[0].format = kColorFormat;
    pipelineDescriptor.cTargets[0].nextInChain = &pipelineMSAAExpandResolveDesc[0];
    pipelineMSAAExpandResolveDesc[1].enabled = true;
    pipelineDescriptor.cTargets[1].format = kColorFormat;
    pipelineDescriptor.cTargets[1].nextInChain = &pipelineMSAAExpandResolveDesc[1];
    pipelineDescriptor.cFragment.targetCount = 2;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);
    renderPass.SetPipeline(pipeline);
    renderPass.End();

    encoder.Finish();
}

// Test that the following render pipeline and render pass are incompatible.
// - Render pass:
//   - colorAttachments[0]
//     - has resolveTarget.
//     - loadOp = ExpandResolveTexture.
//   - colorAttachments[1]
//     - has resolveTarget.
//     - loadOp = Load.
// - Render pipeline:
//   - colorTargets[0].nextInChain contains ColorTargetStateExpandResolveTextureDawn.
//     - ColorTargetStateExpandResolveTextureDawn.enabled = true.
//   - colorTargets[1].nextInChain = null.
// They are incompatible by spec's requirements.
TEST_F(LoadResolveTexturePipelineDescriptorValidationTest,
       RenderPipelineAndRenderPassMismatchResolveTargetsError) {
    constexpr uint32_t kSampleCount = 4;

    auto msaaTexture1 = CreateTexture(wgpu::TextureUsage::RenderAttachment, kSampleCount);
    auto msaaTexture2 = CreateTexture(wgpu::TextureUsage::RenderAttachment, kSampleCount);

    // Create single sampled textures.
    auto texture1 =
        CreateTexture(wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding, 1);
    auto texture2 =
        CreateTexture(wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding, 1);

    // Create render pass with two resolve targets. First one uses ExpandResolveTexture.
    utils::ComboRenderPassDescriptor renderPassDescriptor(
        {msaaTexture1.CreateView(), msaaTexture2.CreateView()});
    renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
    renderPassDescriptor.cColorAttachments[0].resolveTarget = texture1.CreateView();
    renderPassDescriptor.cColorAttachments[1].loadOp = wgpu::LoadOp::Load;
    renderPassDescriptor.cColorAttachments[1].resolveTarget = texture2.CreateView();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);

    // Create render pipeline (without ColorTargetStateExpandResolveTextureDawn chained to
    // colorTargets[1])
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fs2TargetsModule;
    pipelineDescriptor.multisample.count = kSampleCount;

    wgpu::ColorTargetStateExpandResolveTextureDawn pipelineMSAAExpandResolveDesc;
    pipelineMSAAExpandResolveDesc.enabled = true;
    pipelineDescriptor.cTargets[0].format = kColorFormat;
    pipelineDescriptor.cTargets[0].nextInChain = &pipelineMSAAExpandResolveDesc;
    pipelineDescriptor.cTargets[1].format = kColorFormat;
    pipelineDescriptor.cFragment.targetCount = 2;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);
    renderPass.SetPipeline(pipeline);
    renderPass.End();

    ASSERT_DEVICE_ERROR(encoder.Finish(), testing::HasSubstr("not compatible"));
}

// Test that the following render pipeline and render pass are incompatible.
// - Render pass:
//   - colorAttachments[0]
//     - has resolveTarget.
//     - loadOp = ExpandResolveTexture.
//   - colorAttachments[1]
//     - has **no** resolveTarget.
// - Render pipeline:
//   - colorTargets[0].nextInChain = ColorTargetStateExpandResolveTextureDawn.
//     - ColorTargetStateExpandResolveTextureDawn.enabled = true.
//   - colorTargets[1].nextInChain = ColorTargetStateExpandResolveTextureDawn.
//     - ColorTargetStateExpandResolveTextureDawn.enabled = false.
// They are incompatible by spec's requirements.
TEST_F(LoadResolveTexturePipelineDescriptorValidationTest,
       RenderPipelineAndRenderPassMismatchResolveTargetsError2) {
    constexpr uint32_t kSampleCount = 4;

    auto msaaTexture1 = CreateTexture(wgpu::TextureUsage::RenderAttachment, kSampleCount);
    auto msaaTexture2 = CreateTexture(wgpu::TextureUsage::RenderAttachment, kSampleCount);

    // Create single sampled textures.
    auto texture1 =
        CreateTexture(wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding, 1);

    // Create render pass with two outputs but one resolve target. First one uses
    // ExpandResolveTexture.
    utils::ComboRenderPassDescriptor renderPassDescriptor(
        {msaaTexture1.CreateView(), msaaTexture2.CreateView()});
    renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
    renderPassDescriptor.cColorAttachments[0].resolveTarget = texture1.CreateView();
    renderPassDescriptor.cColorAttachments[1].loadOp = wgpu::LoadOp::Load;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);

    // Create render pipeline with two chained ColorTargetStateExpandResolveTextureDawn.
    // The first one's enabled flag = true.
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fs2TargetsModule;
    pipelineDescriptor.multisample.count = kSampleCount;

    wgpu::ColorTargetStateExpandResolveTextureDawn pipelineMSAAExpandResolveDesc[2];
    pipelineMSAAExpandResolveDesc[0].enabled = true;
    pipelineDescriptor.cTargets[0].format = kColorFormat;
    pipelineDescriptor.cTargets[0].nextInChain = &pipelineMSAAExpandResolveDesc[0];
    pipelineMSAAExpandResolveDesc[1].enabled = false;
    pipelineDescriptor.cTargets[1].format = kColorFormat;
    pipelineDescriptor.cTargets[1].nextInChain = &pipelineMSAAExpandResolveDesc[1];
    pipelineDescriptor.cFragment.targetCount = 2;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);
    renderPass.SetPipeline(pipeline);
    renderPass.End();

    ASSERT_DEVICE_ERROR(encoder.Finish(), testing::HasSubstr("not compatible"));
}

// Bind resolve attachment in a ExpandResolveTexture render pass as texture should result
// in error.
TEST_F(LoadResolveTexturePipelineDescriptorValidationTest, BindColorAttachmentAsTextureError) {
    constexpr uint32_t kSampleCount = 4;

    // Create multi sampled texture.
    auto msaaTexture = CreateTexture(wgpu::TextureUsage::RenderAttachment, kSampleCount);

    // Create single sampled texture.
    auto resolveTexture =
        CreateTexture(wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding, 1);

    // Create render pass (with ExpandResolveTexture load op).
    utils::ComboRenderPassDescriptor renderPassDescriptor({msaaTexture.CreateView()});
    renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
    renderPassDescriptor.cColorAttachments[0].resolveTarget = resolveTexture.CreateView();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);

    // Create render pipeline
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsWithTextureModule;
    pipelineDescriptor.multisample.count = kSampleCount;

    wgpu::ColorTargetStateExpandResolveTextureDawn pipelineMSAAExpandResolveDesc;
    pipelineMSAAExpandResolveDesc.enabled = true;
    pipelineDescriptor.cTargets[0].nextInChain = &pipelineMSAAExpandResolveDesc;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    // Use resolve attachment as input texture.
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, resolveTexture.CreateView()}});

    renderPass.SetPipeline(pipeline);
    renderPass.SetBindGroup(0, bindGroup);
    renderPass.Draw(3);
    renderPass.End();

    ASSERT_DEVICE_ERROR(
        encoder.Finish(),
        testing::HasSubstr(
            "includes writable usage and another usage in the same synchronization scope"));
}

class DualSourceBlendingFeatureTest : public RenderPipelineValidationTest {
  protected:
    void SetUp() override {
        RenderPipelineValidationTest::SetUp();

        fsModuleWithBlendSrc1 = utils::CreateShaderModule(device, R"(
        enable dual_source_blending;
        struct FragOut {
            @location(0) @blend_src(0) color : vec4f,
            @location(0) @blend_src(1) blend : vec4f,
        }
        @fragment fn main() -> FragOut {
            var output : FragOut;
            output.color = vec4f(0.0, 1.0, 0.0, 1.0);
            output.blend = vec4f(0.0, 1.0, 0.0, 1.0);
            return output;
        })");
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::DualSourceBlending};
    }

    wgpu::ShaderModule fsModuleWithBlendSrc1;
};

// Tests that enums associated with the DualSourceBlending feature are valid when the feature is
// enabled.
TEST_F(DualSourceBlendingFeatureTest, FeatureEnumsValidWithFeatureEnabled) {
    constexpr std::array kBlendFactors = {wgpu::BlendFactor::Src1, wgpu::BlendFactor::OneMinusSrc1,
                                          wgpu::BlendFactor::Src1Alpha,
                                          wgpu::BlendFactor::OneMinusSrc1Alpha};

    // Test color srcFactor
    for (wgpu::BlendFactor blendFactor : kBlendFactors) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleWithBlendSrc1;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cBlends[0].color.srcFactor = blendFactor;
        descriptor.cBlends[0].color.dstFactor = wgpu::BlendFactor::Src;
        descriptor.cBlends[0].color.operation = wgpu::BlendOperation::Add;
        device.CreateRenderPipeline(&descriptor);
    }

    // Test color dstFactor
    for (wgpu::BlendFactor blendFactor : kBlendFactors) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleWithBlendSrc1;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cBlends[0].color.srcFactor = wgpu::BlendFactor::Src;
        descriptor.cBlends[0].color.dstFactor = blendFactor;
        descriptor.cBlends[0].color.operation = wgpu::BlendOperation::Add;
        device.CreateRenderPipeline(&descriptor);
    }

    // Test alpha srcFactor
    for (wgpu::BlendFactor blendFactor : kBlendFactors) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleWithBlendSrc1;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cBlends[0].alpha.srcFactor = blendFactor;
        descriptor.cBlends[0].alpha.dstFactor = wgpu::BlendFactor::SrcAlpha;
        descriptor.cBlends[0].alpha.operation = wgpu::BlendOperation::Add;
        device.CreateRenderPipeline(&descriptor);
    }

    // Test alpha dstFactor
    for (wgpu::BlendFactor blendFactor : kBlendFactors) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleWithBlendSrc1;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cBlends[0].alpha.srcFactor = wgpu::BlendFactor::SrcAlpha;
        descriptor.cBlends[0].alpha.dstFactor = blendFactor;
        descriptor.cBlends[0].alpha.operation = wgpu::BlendOperation::Add;
        device.CreateRenderPipeline(&descriptor);
    }
}

// Test that rendering to multiple render targets while using dual source blending results in an
// error.
TEST_F(DualSourceBlendingFeatureTest, MultipleRenderTargetsNotAllowed) {
    wgpu::Limits limits;
    device.GetLimits(&limits);

    for (uint32_t location = 1; location < limits.maxColorAttachments; location++) {
        std::ostringstream sstream;
        sstream << R"(
                enable dual_source_blending;

                struct TestData {
                    color : vec4f,
                    blend : vec4f
                }

                @group(0) @binding(0) var<uniform> testData : TestData;

                struct FragOut {
                    @location(0) @blend_src(0) color : vec4<f32>,
                    @location(0) @blend_src(1) blend : vec4<f32>,
                    @location()"
                << location << R"("invalidOutput : vec4<f32>
                }

                @fragment fn main() -> FragOut {
                    var output : FragOut;
                    output.color = testData.color;
                    output.blend = testData.blend;
                    return output;)";

        ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, sstream.str().c_str()));
    }
}

// Test that when any blend factor uses `src1` `@blend_src(1)` must be declared in the fragment
// shader.
TEST_F(DualSourceBlendingFeatureTest, BlendFactorSrc1RequiresBlendSrc1InWGSL) {
    constexpr std::array kBlendFactors = {wgpu::BlendFactor::Src1, wgpu::BlendFactor::OneMinusSrc1,
                                          wgpu::BlendFactor::Src1Alpha,
                                          wgpu::BlendFactor::OneMinusSrc1Alpha};

    wgpu::ShaderModule fsModuleWithoutBlendSrc1 = fsModule;

    auto CheckBlendFactorSrc1 = [&](utils::ComboRenderPipelineDescriptor* descriptor) {
        descriptor->cFragment.module = fsModuleWithoutBlendSrc1;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(descriptor));

        descriptor->cFragment.module = fsModuleWithBlendSrc1;
        device.CreateRenderPipeline(descriptor);
    };

    // Test color srcFactor
    for (wgpu::BlendFactor blendFactor : kBlendFactors) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cBlends[0].color.srcFactor = blendFactor;
        descriptor.cBlends[0].color.dstFactor = wgpu::BlendFactor::Src;
        descriptor.cBlends[0].color.operation = wgpu::BlendOperation::Add;

        CheckBlendFactorSrc1(&descriptor);
    }

    // Test color dstFactor
    for (wgpu::BlendFactor blendFactor : kBlendFactors) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cBlends[0].color.srcFactor = wgpu::BlendFactor::Src;
        descriptor.cBlends[0].color.dstFactor = blendFactor;
        descriptor.cBlends[0].color.operation = wgpu::BlendOperation::Add;

        CheckBlendFactorSrc1(&descriptor);
    }

    // Test alpha srcFactor
    for (wgpu::BlendFactor blendFactor : kBlendFactors) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cBlends[0].alpha.srcFactor = blendFactor;
        descriptor.cBlends[0].alpha.dstFactor = wgpu::BlendFactor::SrcAlpha;
        descriptor.cBlends[0].alpha.operation = wgpu::BlendOperation::Add;

        CheckBlendFactorSrc1(&descriptor);
    }

    // Test alpha dstFactor
    for (wgpu::BlendFactor blendFactor : kBlendFactors) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cBlends[0].alpha.srcFactor = wgpu::BlendFactor::SrcAlpha;
        descriptor.cBlends[0].alpha.dstFactor = blendFactor;
        descriptor.cBlends[0].alpha.operation = wgpu::BlendOperation::Add;

        CheckBlendFactorSrc1(&descriptor);
    }
}

// Test that when any color blend factor uses the alpha channel of `src1` the fragment shader output
// with `@blend_src(1)` must have an alpha channel.
TEST_F(DualSourceBlendingFeatureTest, ColorBlendFactorSrc1AlphaRequiresBlendSrc1AlphaInWGSL) {
    constexpr std::array kBlendFactors = {wgpu::BlendFactor::Src1, wgpu::BlendFactor::OneMinusSrc1,
                                          wgpu::BlendFactor::Src1Alpha,
                                          wgpu::BlendFactor::OneMinusSrc1Alpha};

    wgpu::ShaderModule fsModuleWithBlendSrc1NoAlpha = utils::CreateShaderModule(device, R"(
        enable dual_source_blending;
        struct FragOut {
            @location(0) @blend_src(0) color : vec2f,
            @location(0) @blend_src(1) blend : vec2f,
        }
        @fragment fn main() -> FragOut {
            var output : FragOut;
            output.color = vec2f(0.0, 1.0);
            output.blend = vec2f(0.0, 1.0);
            return output;
        })");

    auto CheckBlendFactorSrc1Alpha = [&](const wgpu::RenderPipelineDescriptor* descriptor,
                                         wgpu::BlendFactor blendFactor, bool isColorBlendFactor) {
        if (!isColorBlendFactor) {
            device.CreateRenderPipeline(descriptor);
            return;
        }
        switch (blendFactor) {
            case wgpu::BlendFactor::Src1:
            case wgpu::BlendFactor::OneMinusSrc1:
                device.CreateRenderPipeline(descriptor);
                break;
            case wgpu::BlendFactor::Src1Alpha:
            case wgpu::BlendFactor::OneMinusSrc1Alpha:
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(descriptor));
                break;
            default:
                DAWN_UNREACHABLE();
        }
    };

    // Test color srcFactor
    for (wgpu::BlendFactor blendFactor : kBlendFactors) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleWithBlendSrc1NoAlpha;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RG8Unorm;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cBlends[0].color.srcFactor = blendFactor;
        descriptor.cBlends[0].color.dstFactor = wgpu::BlendFactor::Src;
        descriptor.cBlends[0].color.operation = wgpu::BlendOperation::Add;

        constexpr bool kIsColorBlendFactor = true;
        CheckBlendFactorSrc1Alpha(&descriptor, blendFactor, kIsColorBlendFactor);
    }

    // Test color dstFactor
    for (wgpu::BlendFactor blendFactor : kBlendFactors) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleWithBlendSrc1NoAlpha;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RG8Unorm;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cBlends[0].color.srcFactor = wgpu::BlendFactor::Src;
        descriptor.cBlends[0].color.dstFactor = blendFactor;
        descriptor.cBlends[0].color.operation = wgpu::BlendOperation::Add;

        constexpr bool kIsColorBlendFactor = true;
        CheckBlendFactorSrc1Alpha(&descriptor, blendFactor, kIsColorBlendFactor);
    }

    // Test alpha srcFactor
    for (wgpu::BlendFactor blendFactor : kBlendFactors) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleWithBlendSrc1NoAlpha;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RG8Unorm;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cBlends[0].alpha.srcFactor = blendFactor;
        descriptor.cBlends[0].alpha.dstFactor = wgpu::BlendFactor::SrcAlpha;
        descriptor.cBlends[0].alpha.operation = wgpu::BlendOperation::Add;

        constexpr bool kIsColorBlendFactor = false;
        CheckBlendFactorSrc1Alpha(&descriptor, blendFactor, kIsColorBlendFactor);
    }

    // Test alpha dstFactor
    for (wgpu::BlendFactor blendFactor : kBlendFactors) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleWithBlendSrc1NoAlpha;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RG8Unorm;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cBlends[0].alpha.srcFactor = wgpu::BlendFactor::SrcAlpha;
        descriptor.cBlends[0].alpha.dstFactor = blendFactor;
        descriptor.cBlends[0].alpha.operation = wgpu::BlendOperation::Add;

        constexpr bool kIsColorBlendFactor = false;
        CheckBlendFactorSrc1Alpha(&descriptor, blendFactor, kIsColorBlendFactor);
    }
}

// Test that it is valid to declare `@blend_src(1)` before `@blend_src(0)`.
TEST_F(DualSourceBlendingFeatureTest, BlendSrc1BeforeBlendSrc0) {
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        enable dual_source_blending;
        struct FragOut {
            @location(0) @blend_src(1) blend : vec4f,
            @location(0) @blend_src(0) color : vec4f,
        }
        @fragment fn main() -> FragOut {
            var output : FragOut;
            output.color = vec4f(0.0, 1.0, 0.0, 1.0);
            output.blend = vec4f(0.0, 1.0, 0.0, 1.0);
            return output;
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.cTargets[0].blend = &descriptor.cBlends[0];
    descriptor.cBlends[0].color.srcFactor = wgpu::BlendFactor::Src1;
    descriptor.cBlends[0].color.dstFactor = wgpu::BlendFactor::Src;
    descriptor.cBlends[0].color.operation = wgpu::BlendOperation::Add;
    device.CreateRenderPipeline(&descriptor);
}

// Test that it is valid to use a constant expression as `@blend_src` attribute.
TEST_F(DualSourceBlendingFeatureTest, ConstValueAsBlendSrc) {
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        enable dual_source_blending;
        const a = 1;
        const b = 2;
        struct FragOut {
            @location(0) @blend_src(0) color : vec4f,
            @location(0) @blend_src(b - a) blend : vec4f,
        }
        @fragment fn main() -> FragOut {
            var output : FragOut;
            output.color = vec4f(0.0, 1.0, 0.0, 1.0);
            output.blend = vec4f(0.0, 1.0, 0.0, 1.0);
            return output;
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.cTargets[0].blend = &descriptor.cBlends[0];
    descriptor.cBlends[0].color.srcFactor = wgpu::BlendFactor::Src1;
    descriptor.cBlends[0].color.dstFactor = wgpu::BlendFactor::Src;
    descriptor.cBlends[0].color.operation = wgpu::BlendOperation::Add;
    device.CreateRenderPipeline(&descriptor);
}

// Test that when `Src` is used in the blend factor, it is not allowed to have multiple color
// targets, even when all the color targets whose index is non-zero have a color mask equal to None.
TEST_F(DualSourceBlendingFeatureTest, MultipleColorTargets) {
    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        enable dual_source_blending;
        struct FragOut {
            @location(0) @blend_src(0) color : vec4f,
            @location(0) @blend_src(1) blend : vec4f,
        }
        @fragment fn main() -> FragOut {
            var output : FragOut;
            output.color = vec4f(0.0, 1.0, 0.0, 1.0);
            output.blend = vec4f(0.0, 1.0, 0.0, 1.0);
            return output;
        })");

    constexpr std::array<wgpu::ColorWriteMask, 2> kColorWriteMasks = {
        {wgpu::ColorWriteMask::All, wgpu::ColorWriteMask::None}};

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.cFragment.targetCount = 2;
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.cTargets[0].blend = &descriptor.cBlends[0];
    descriptor.cBlends[0].color.srcFactor = wgpu::BlendFactor::Src1;
    descriptor.cBlends[0].color.dstFactor = wgpu::BlendFactor::Src;
    descriptor.cBlends[0].color.operation = wgpu::BlendOperation::Add;
    descriptor.cTargets[1] = descriptor.cTargets[0];
    descriptor.cTargets[1].blend = nullptr;

    for (wgpu::ColorWriteMask writeMask : kColorWriteMasks) {
        descriptor.cTargets[1].writeMask = writeMask;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

class FramebufferFetchFeatureTest : public RenderPipelineValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::FramebufferFetch};
    }
};

// Test that the framebuffer input must have a corresponding color target.
TEST_F(FramebufferFetchFeatureTest, FramebufferInputMustHaveColorTarget) {
    uint32_t colorIndices[] = {0, 1, 2, kMaxColorAttachments - 1, kMaxColorAttachments};
    for (uint32_t colorIndex : colorIndices) {
        std::ostringstream fsStream;
        fsStream << R"(
            enable chromium_experimental_framebuffer_fetch;
            @fragment fn main(@color()"
                 << colorIndex << R"() in : vec4f) -> @location(1) vec4f {
                return in;
            }
        )";

        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = vsModule;
        desc.cFragment.module = utils::CreateShaderModule(device, fsStream.str().c_str());
        desc.cFragment.targetCount = 2;
        desc.cTargets[0].format = wgpu::TextureFormat::Undefined;
        desc.cTargets[1].format = wgpu::TextureFormat::RGBA8Unorm;

        // Only colorIndex 1 should work because it is the only index with a color target.
        if (colorIndex == 1) {
            device.CreateRenderPipeline(&desc);
        } else {
            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&desc));
        }
    }
}

// Test that the framebuffer fetch type matches the texture format exactly.
TEST_F(FramebufferFetchFeatureTest, InputMatchesFormat) {
    struct ValidPair {
        const char* type;
        wgpu::TextureFormat format;
    };

    std::array<ValidPair, 9> validPairs = {{
        {"f32", wgpu::TextureFormat::R32Float},
        {"vec2f", wgpu::TextureFormat::RG16Float},
        {"vec4f", wgpu::TextureFormat::RGBA8Unorm},
        {"u32", wgpu::TextureFormat::R32Uint},
        {"vec2u", wgpu::TextureFormat::RG16Uint},
        {"vec4u", wgpu::TextureFormat::RGBA8Uint},
        {"i32", wgpu::TextureFormat::R32Sint},
        {"vec2i", wgpu::TextureFormat::RG16Sint},
        {"vec4i", wgpu::TextureFormat::RGBA8Sint},
    }};

    for (size_t i = 0; i < validPairs.size(); i++) {
        wgpu::TextureFormat format = validPairs[i].format;
        const char* outputType = validPairs[i].type;

        for (size_t j = 0; j < validPairs.size(); j++) {
            const char* inputType = validPairs[j].type;

            std::ostringstream fsStream;
            fsStream << R"(
                enable chromium_experimental_framebuffer_fetch;
                @fragment fn main(@color(0) in : )"
                     << inputType << R"() -> @location(0) )" << outputType << R"( {
                    var res : )"
                     << outputType << R"(;
                    return res;
                }
            )";

            utils::ComboRenderPipelineDescriptor desc;
            desc.vertex.module = vsModule;
            desc.cFragment.module = utils::CreateShaderModule(device, fsStream.str().c_str());
            desc.cTargets[0].format = format;

            if (i == j) {
                device.CreateRenderPipeline(&desc);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&desc));
            }
        }
    }
}

class ClipDistancesValidationTest : public RenderPipelineValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::ClipDistances};
    }
    std::string CreateShaderWithClipDistances(uint32_t clipDistancesArraySize,
                                              uint32_t userDefinedVertexOutputsCount,
                                              bool enableExtension = true) {
        std::ostringstream stream;
        if (enableExtension) {
            stream << "enable clip_distances;\n";
        }
        stream << R"(
            struct VertexOutputs {
                @builtin(position) pos : vec4f,
                @builtin(clip_distances) clipDistances : array<f32, )"
               << clipDistancesArraySize << ">,";
        for (uint32_t i = 0; i < userDefinedVertexOutputsCount; ++i) {
            stream << "@location(" << i << ") vertex_out" << i << " : vec4f,\n";
        }
        stream << R"(}
            struct FragmentInput {)";
        for (uint32_t i = 0; i < userDefinedVertexOutputsCount; ++i) {
            stream << "@location(" << i << ") fragment_input" << i << " : vec4f,\n";
        }
        stream << R"(
            }
            @vertex
            fn vsMain(@builtin(vertex_index) vertexIndex : u32) -> VertexOutputs {
                var vertexOutput : VertexOutputs;
                return vertexOutput;
            }
            @fragment
            fn fsMain(fragmentInput : FragmentInput) -> @location(0) vec4f {
                return vec4f(1.0, 0.0, 0.0, 1.0);
            })";
        return stream.str();
    }
};

// Test that using `clip_distances` in vertex shader succeeds if the feature is enabled.
TEST_F(ClipDistancesValidationTest, Success) {
    utils::ComboRenderPipelineDescriptor descriptor;
    constexpr uint32_t kClipDistancesSize = 1u;
    constexpr uint32_t kUserDefinedVertexOutputsCount = 1u;
    descriptor.vertex.module = utils::CreateShaderModule(
        device, CreateShaderWithClipDistances(kClipDistancesSize, kUserDefinedVertexOutputsCount));
    descriptor.cFragment.module = fsModule;
    device.CreateRenderPipeline(&descriptor);
}

// Test that using `clip_distances` in vertex shader fails if the feature is not enabled.
TEST_F(ClipDistancesValidationTest, FailWithoutExtensionEnabled) {
    constexpr uint32_t kClipDistancesSize = 1u;
    constexpr uint32_t kUserDefinedVertexOutputsCount = 1u;
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(
        device,
        CreateShaderWithClipDistances(kClipDistancesSize, kUserDefinedVertexOutputsCount, false)));
}

// Test that when the array size of `clip_distances` is no more than 4 it will always consume 1
// inter-stage shader variable, and when the array size of `clip_distances` is at least 4 (no more
// than 8) it will always consume 2 inter-stage shader variables.
TEST_F(ClipDistancesValidationTest, ClipDistancesAgainstMaxInterStageShaderVariables) {
    for (uint32_t clipDistancesSize = 1; clipDistancesSize <= 8; ++clipDistancesSize) {
        for (uint32_t userDefinedVertexOutputsCount = kMaxInterStageShaderVariables - 2u;
             userDefinedVertexOutputsCount <= kMaxInterStageShaderVariables;
             ++userDefinedVertexOutputsCount) {
            wgpu::ShaderModule shaderModule = utils::CreateShaderModule(
                device,
                CreateShaderWithClipDistances(clipDistancesSize, userDefinedVertexOutputsCount));
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = shaderModule;
            descriptor.cFragment.module = shaderModule;

            uint32_t slotsForClipDistances = Align(clipDistancesSize, 4u) / 4;
            uint32_t totalSlots = slotsForClipDistances + userDefinedVertexOutputsCount;
            if (totalSlots > kMaxInterStageShaderVariables) {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
            } else {
                device.CreateRenderPipeline(&descriptor);
            }
        }
    }
}

// Test that when the array size of `clip_distances` is no more than 4 it will always consume 1
// inter-stage shader variable, and when the array size of `clip_distances` is at least 4 (no more
// than 8) it will always consume 2 inter-stage shader variables.
TEST_F(ClipDistancesValidationTest, ClipDistancesAgainstMaxInterStageShaderVariables_PointList) {
    for (uint32_t clipDistancesSize = 1; clipDistancesSize <= 8; ++clipDistancesSize) {
        for (uint32_t userDefinedVertexOutputsCount = kMaxInterStageShaderVariables - 3u;
             userDefinedVertexOutputsCount <= kMaxInterStageShaderVariables - 1u;
             ++userDefinedVertexOutputsCount) {
            wgpu::ShaderModule shaderModule = utils::CreateShaderModule(
                device,
                CreateShaderWithClipDistances(clipDistancesSize, userDefinedVertexOutputsCount));
            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = shaderModule;
            descriptor.cFragment.module = shaderModule;
            descriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;

            uint32_t slotsForClipDistances = Align(clipDistancesSize, 4u) / 4;
            uint32_t totalSlots = slotsForClipDistances + userDefinedVertexOutputsCount + 1u;
            if (totalSlots > kMaxInterStageShaderVariables) {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
            } else {
                device.CreateRenderPipeline(&descriptor);
            }
        }
    }
}

// Tests that using @builtin(clip_distances) will decrease the maximum location of the inter-stage
// shader variable, while the PointList primitive topology doesn't affect the maximum location of
// the inter-stage shader variable.
TEST_F(ClipDistancesValidationTest, ClipDistancesAgainstMaxInterStageLocation) {
    constexpr std::array<wgpu::PrimitiveTopology, 2> kPrimitives = {
        {wgpu::PrimitiveTopology::TriangleList, wgpu::PrimitiveTopology::PointList}};
    for (wgpu::PrimitiveTopology primitive : kPrimitives) {
        for (uint32_t clipDistancesSize = 1; clipDistancesSize <= 8; ++clipDistancesSize) {
            for (uint32_t location = kMaxInterStageShaderVariables - 3u;
                 location < kMaxInterStageShaderVariables; ++location) {
                std::stringstream stream;
                stream << R"(
                    enable clip_distances;
                    struct VertexOut {
                        @location()"
                       << location << ") color : vec4f,\n"
                       << R"(
                        @builtin(clip_distances) clipDistances : array<f32, )"
                       << clipDistancesSize << ">,\n"
                       << R"(
                        @builtin(position) pos : vec4f,
                     }
                     struct FragmentIn {
                         @location()"
                       << location << ") color : vec4f,\n"
                       << R"(
                         @builtin(position) pos : vec4f,
                     }
                     @vertex
                     fn vsMain() -> VertexOut {
                         var vout : VertexOut;
                         return vout;
                     }
                     @fragment
                     fn fsMain(fragIn : FragmentIn) -> @location(0) vec4f {
                         return fragIn.pos;
                     })";

                wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, stream.str());
                utils::ComboRenderPipelineDescriptor descriptor;
                descriptor.vertex.module = shaderModule;
                descriptor.cFragment.module = shaderModule;
                descriptor.primitive.topology = primitive;

                uint32_t slotsForClipDistances = Align(clipDistancesSize, 4u) / 4;
                uint32_t maxLocation = kMaxInterStageShaderVariables - 1 - slotsForClipDistances;
                if (location > maxLocation) {
                    ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
                } else {
                    device.CreateRenderPipeline(&descriptor);
                }
            }
        }
    }
}

// Tests that kTier1AdditionalRenderableFormats are not renderable without feature.
TEST_F(RenderPipelineValidationTest, NotRenderableWithoutTier1) {
    for (const auto format : utils::kTier1AdditionalRenderableFormats) {
        SCOPED_TRACE(absl::StrFormat("Test format: %s", format));
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = format;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Tests that kTier1AdditionalRenderableFormats are not blendable without feature.
TEST_F(RenderPipelineValidationTest, NotBlendableWithoutTier1) {
    for (const auto format : utils::kTier1AdditionalRenderableFormats) {
        SCOPED_TRACE(absl::StrFormat("Test format: %s", format));
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModuleUint;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cTargets[0].format = format;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

// Tests that kTier1AdditionalRenderableFormats can not support multisampling without feature.
TEST_F(RenderPipelineValidationTest, NoMultisampleSupportWithoutTier1) {
    for (const auto format : utils::kTier1AdditionalRenderableFormats) {
        SCOPED_TRACE(absl::StrFormat("Test format: %s", format));
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = format;
        descriptor.multisample.count = 4;
        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

class TextureFormatsTier1PipelineTest : public RenderPipelineValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::TextureFormatsTier1};
    }
};

// Tests that kTier1AdditionalRenderableFormats must be renderable when the feature
// TextureFormatsTier1 is enabled.
TEST_F(TextureFormatsTier1PipelineTest, RenderableWithTier1) {
    for (const auto format : utils::kTier1AdditionalRenderableFormats) {
        SCOPED_TRACE(absl::StrFormat("Test format: %s", format));
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = format;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
    }
}

// Tests that kTier1AdditionalRenderableFormats must be blendable when the feature
// TextureFormatsTier1 is enabled.
TEST_F(TextureFormatsTier1PipelineTest, BlendableWithTier1) {
    for (const auto format : utils::kTier1AdditionalRenderableFormats) {
        SCOPED_TRACE(absl::StrFormat("Test format: %s", format));
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].blend = &descriptor.cBlends[0];
        descriptor.cTargets[0].format = format;
        device.CreateRenderPipeline(&descriptor);
    }
}

// Tests that kTier1AdditionalRenderableFormats must support multisampling when the
// TextureFormatsTier1 feature is enabled.
TEST_F(TextureFormatsTier1PipelineTest, MultisampleSupportWithTier1) {
    for (const auto format : utils::kTier1AdditionalRenderableFormats) {
        SCOPED_TRACE(absl::StrFormat("Test format: %s", format));
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = format;
        descriptor.multisample.count = 4;
        device.CreateRenderPipeline(&descriptor);
    }
}

class RG11B10UfloatRenderablePipelineTest : public RenderPipelineValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::RG11B10UfloatRenderable};
    }
};

// Tests that rg11b10ufloat must be renderable when the feature RG11B10UfloatRenderable is enabled.
TEST_F(RG11B10UfloatRenderablePipelineTest, RenderableWithFeatureEnabled) {
    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.cTargets[0].format = wgpu::TextureFormat::RG11B10Ufloat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);
}

// Tests that rg11b10ufloat must be blendable when the feature RG11B10UfloatRenderable is enabled.
TEST_F(RG11B10UfloatRenderablePipelineTest, BlendableWithFeatureEnabled) {
    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.cTargets[0].blend = &descriptor.cBlends[0];
    descriptor.cTargets[0].format = wgpu::TextureFormat::RG11B10Ufloat;
    device.CreateRenderPipeline(&descriptor);
}

// Tests that rg11b10ufloat must support multisampling when the RG11B10UfloatRenderable feature is
// enabled.
TEST_F(RG11B10UfloatRenderablePipelineTest, MultisampleSupportWithFeatureEnabled) {
    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.cTargets[0].format = wgpu::TextureFormat::RG11B10Ufloat;
    descriptor.multisample.count = 4;
    device.CreateRenderPipeline(&descriptor);
}

}  // anonymous namespace
}  // namespace dawn
