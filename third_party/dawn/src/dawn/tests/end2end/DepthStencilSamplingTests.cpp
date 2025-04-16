// Copyright 2020 The Dawn & Tint Authors
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

#include <utility>
#include <vector>

#include "absl/strings/str_format.h"
#include "dawn/common/Assert.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using TextureFormat = wgpu::TextureFormat;
DAWN_TEST_PARAM_STRUCT(DepthStencilSamplingTestParams, TextureFormat);

constexpr wgpu::CompareFunction kCompareFunctions[] = {
    wgpu::CompareFunction::Never,        wgpu::CompareFunction::Less,
    wgpu::CompareFunction::LessEqual,    wgpu::CompareFunction::Greater,
    wgpu::CompareFunction::GreaterEqual, wgpu::CompareFunction::Equal,
    wgpu::CompareFunction::NotEqual,     wgpu::CompareFunction::Always,
};

// Test a "normal" ref value between 0 and 1; as well as negative and > 1 refs.
constexpr float kCompareRefs[] = {-0.1, 0.4, 1.2};

// Test 0, below the ref, equal to, above the ref, and 1.
const std::vector<float> kNormalizedTextureValues = {0.0, 0.3, 0.4, 0.5, 1.0};

// Test the limits, and some values in between.
const std::vector<uint32_t> kStencilValues = {0, 1, 38, 255};

class DepthStencilSamplingTest : public DawnTestWithParams<DepthStencilSamplingTestParams> {
  protected:
    wgpu::Limits GetRequiredLimits(const wgpu::Limits& supported) override {
        // Just copy all the limits, though all we really care about is
        // maxStorageBuffersInFragmentStage
        return supported;
    }

    enum class TestAspectAndSamplerType {
        DepthAsDepth,
        DepthAsFloat,
        StencilAsUint,
    };

    void SetUp() override {
        DawnTestWithParams<DepthStencilSamplingTestParams>::SetUp();

        DAWN_TEST_UNSUPPORTED_IF(!mIsFormatSupported);

        // Skip formats other than Depth24PlusStencil8 if we're specifically testing with the packed
        // depth24_unorm_stencil8 toggle.
        DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("use_packed_depth24_unorm_stencil8_format") &&
                                 GetParam().mTextureFormat !=
                                     wgpu::TextureFormat::Depth24PlusStencil8);

        wgpu::BufferDescriptor uniformBufferDesc;
        uniformBufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        uniformBufferDesc.size = sizeof(float);
        mUniformBuffer = device.CreateBuffer(&uniformBufferDesc);
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        switch (GetParam().mTextureFormat) {
            case wgpu::TextureFormat::Depth32FloatStencil8:
                if (SupportsFeatures({wgpu::FeatureName::Depth32FloatStencil8})) {
                    mIsFormatSupported = true;
                    return {wgpu::FeatureName::Depth32FloatStencil8};
                }
                return {};
            default:
                mIsFormatSupported = true;
                return {};
        }
    }

    void GenerateSamplingShader(const std::vector<TestAspectAndSamplerType>& aspectAndSamplerTypes,
                                const std::vector<uint32_t> components,
                                std::ostringstream& shaderSource,
                                std::ostringstream& shaderBody) {
        shaderSource << "alias StencilValues = array<u32, " << components.size() << ">;\n";
        shaderSource << R"(
            struct DepthResult {
                value : f32
            }
            struct StencilResult {
                values : StencilValues
            })";
        shaderSource << "\n";

        uint32_t index = 0;
        for (TestAspectAndSamplerType aspectAndSamplerType : aspectAndSamplerTypes) {
            switch (aspectAndSamplerType) {
                case TestAspectAndSamplerType::DepthAsDepth:
                    shaderSource << "@group(0) @binding(" << 2 * index << ") var tex" << index
                                 << " : texture_depth_2d;\n";

                    shaderSource << "@group(0) @binding(" << 2 * index + 1
                                 << ") var<storage, read_write> result" << index
                                 << " : DepthResult;\n";

                    DAWN_ASSERT(components.size() == 1 && components[0] == 0);
                    shaderBody << "\nresult" << index << ".value = textureLoad(tex" << index
                               << ", vec2i(0, 0), 0);";
                    break;
                case TestAspectAndSamplerType::DepthAsFloat:
                    shaderSource << "@group(0) @binding(" << 2 * index << ") var tex" << index
                                 << " : texture_2d<f32>;\n";

                    shaderSource << "@group(0) @binding(" << 2 * index + 1
                                 << ") var<storage, read_write> result" << index
                                 << " : DepthResult;\n";

                    DAWN_ASSERT(components.size() == 1 && components[0] == 0);
                    shaderBody << "\nresult" << index << ".value = textureLoad(tex" << index
                               << ", vec2i(0, 0), 0)[0];";
                    break;
                case TestAspectAndSamplerType::StencilAsUint:
                    shaderSource << "@group(0) @binding(" << 2 * index << ") var tex" << index
                                 << " : texture_2d<u32>;\n";

                    shaderSource << "@group(0) @binding(" << 2 * index + 1
                                 << ") var<storage, read_write> result" << index
                                 << " : StencilResult;\n";

                    shaderBody << "var texel = textureLoad(tex" << index << ", vec2i(0, 0), 0);";

                    for (uint32_t i = 0; i < components.size(); ++i) {
                        shaderBody << "\nresult" << index << ".values[" << i << "] = texel["
                                   << components[i] << "];";
                    }
                    break;
            }

            index++;
        }
    }

    wgpu::RenderPipeline CreateSamplingRenderPipeline(
        std::vector<TestAspectAndSamplerType> aspectAndSamplerTypes,
        std::vector<uint32_t> components) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex fn main() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor pipelineDescriptor;

        std::ostringstream shaderSource;
        std::ostringstream shaderOutputStruct;
        std::ostringstream shaderBody;

        GenerateSamplingShader(aspectAndSamplerTypes, components, shaderSource, shaderBody);

        shaderSource << "@fragment fn main() -> @location(0) vec4f {\n";
        shaderSource << shaderBody.str() << "return vec4f();\n }";

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, shaderSource.str().c_str());
        pipelineDescriptor.vertex.module = vsModule;
        pipelineDescriptor.cFragment.module = fsModule;
        pipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
        pipelineDescriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    wgpu::ComputePipeline CreateSamplingComputePipeline(
        std::vector<TestAspectAndSamplerType> aspectAndSamplerTypes,
        std::vector<uint32_t> components) {
        std::ostringstream shaderSource;
        std::ostringstream shaderBody;
        GenerateSamplingShader(aspectAndSamplerTypes, components, shaderSource, shaderBody);

        shaderSource << "@compute @workgroup_size(1) fn main() { " << shaderBody.str() << "\n}";

        wgpu::ShaderModule csModule = utils::CreateShaderModule(device, shaderSource.str().c_str());

        wgpu::ComputePipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.compute.module = csModule;

        return device.CreateComputePipeline(&pipelineDescriptor);
    }

    wgpu::RenderPipeline CreateSamplingRenderPipeline(
        std::vector<TestAspectAndSamplerType> aspectAndSamplerTypes,
        uint32_t componentIndex) {
        return CreateSamplingRenderPipeline(std::move(aspectAndSamplerTypes),
                                            std::vector<uint32_t>{componentIndex});
    }

    wgpu::ComputePipeline CreateSamplingComputePipeline(
        std::vector<TestAspectAndSamplerType> aspectAndSamplerTypes,
        uint32_t componentIndex) {
        return CreateSamplingComputePipeline(std::move(aspectAndSamplerTypes),
                                             std::vector<uint32_t>{componentIndex});
    }

    wgpu::RenderPipeline CreateComparisonRenderPipeline() {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex fn main() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 1.0);
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var samp : sampler_comparison;
            @group(0) @binding(1) var tex : texture_depth_2d;
            struct Uniforms {
                compareRef : f32
            }
            @group(0) @binding(2) var<uniform> uniforms : Uniforms;

            @fragment fn main() -> @location(0) f32 {
                return textureSampleCompare(tex, samp, vec2f(0.5, 0.5), uniforms.compareRef);
            })");

        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = vsModule;
        pipelineDescriptor.cFragment.module = fsModule;
        pipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
        pipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::R32Float;

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    wgpu::ComputePipeline CreateComparisonComputePipeline() {
        wgpu::ShaderModule csModule = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var samp : sampler_comparison;
            @group(0) @binding(1) var tex : texture_depth_2d;
            struct Uniforms {
                compareRef : f32
            }
            @group(0) @binding(2) var<uniform> uniforms : Uniforms;

            struct SamplerResult {
                value : f32
            }
            @group(0) @binding(3) var<storage, read_write> samplerResult : SamplerResult;

            @compute @workgroup_size(1) fn main() {
                samplerResult.value = textureSampleCompare(tex, samp, vec2f(0.5, 0.5), uniforms.compareRef);
            })");

        wgpu::ComputePipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.compute.module = csModule;

        return device.CreateComputePipeline(&pipelineDescriptor);
    }

    wgpu::Texture CreateInputTexture(wgpu::TextureFormat format) {
        wgpu::TextureDescriptor inputTextureDesc;
        inputTextureDesc.usage =
            wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;
        inputTextureDesc.size = {1, 1, 1};
        inputTextureDesc.format = format;
        return device.CreateTexture(&inputTextureDesc);
    }

    wgpu::Texture CreateOutputTexture(wgpu::TextureFormat format) {
        wgpu::TextureDescriptor outputTextureDesc;
        outputTextureDesc.usage =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        outputTextureDesc.size = {1, 1, 1};
        outputTextureDesc.format = format;
        return device.CreateTexture(&outputTextureDesc);
    }

    wgpu::Buffer CreateOutputBuffer(uint32_t componentCount = 1) {
        wgpu::BufferDescriptor outputBufferDesc;
        outputBufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
        outputBufferDesc.size = sizeof(uint32_t) * componentCount;
        return device.CreateBuffer(&outputBufferDesc);
    }

    void UpdateInputDepth(wgpu::CommandEncoder commandEncoder,
                          wgpu::Texture texture,
                          wgpu::TextureFormat format,
                          float depthValue) {
        utils::ComboRenderPassDescriptor passDescriptor({}, texture.CreateView());
        passDescriptor.UnsetDepthStencilLoadStoreOpsForFormat(format);
        passDescriptor.cDepthStencilAttachmentInfo.depthClearValue = depthValue;

        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
        pass.End();
    }

    void UpdateInputStencil(wgpu::CommandEncoder commandEncoder,
                            wgpu::Texture texture,
                            wgpu::TextureFormat format,
                            uint8_t stencilValue) {
        utils::ComboRenderPassDescriptor passDescriptor({}, texture.CreateView());
        passDescriptor.UnsetDepthStencilLoadStoreOpsForFormat(format);
        passDescriptor.cDepthStencilAttachmentInfo.stencilClearValue = stencilValue;

        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
        pass.End();
    }

    template <typename T, typename CheckBufferFn>
    void DoSamplingTestImpl(TestAspectAndSamplerType aspectAndSamplerType,
                            wgpu::RenderPipeline pipeline,
                            wgpu::TextureFormat format,
                            std::vector<T> textureValues,
                            uint32_t componentCount,
                            CheckBufferFn CheckBuffer) {
        wgpu::Texture inputTexture = CreateInputTexture(format);
        wgpu::TextureViewDescriptor inputViewDesc = {};
        switch (aspectAndSamplerType) {
            case TestAspectAndSamplerType::DepthAsDepth:
            case TestAspectAndSamplerType::DepthAsFloat:
                inputViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
                break;
            case TestAspectAndSamplerType::StencilAsUint:
                inputViewDesc.aspect = wgpu::TextureAspect::StencilOnly;
                break;
        }

        wgpu::Buffer outputBuffer = CreateOutputBuffer(componentCount);

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {{0, inputTexture.CreateView(&inputViewDesc)}, {1, outputBuffer}});

        for (size_t i = 0; i < textureValues.size(); ++i) {
            // Set the input depth texture to the provided texture value
            wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
            switch (aspectAndSamplerType) {
                case TestAspectAndSamplerType::DepthAsDepth:
                case TestAspectAndSamplerType::DepthAsFloat:
                    UpdateInputDepth(commandEncoder, inputTexture, format, textureValues[i]);
                    break;
                case TestAspectAndSamplerType::StencilAsUint:
                    UpdateInputStencil(commandEncoder, inputTexture, format, textureValues[i]);
                    break;
            }

            // Render into the output texture
            {
                utils::BasicRenderPass renderPass =
                    utils::CreateBasicRenderPass(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
                wgpu::RenderPassEncoder pass =
                    commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
                pass.SetPipeline(pipeline);
                pass.SetBindGroup(0, bindGroup);
                pass.Draw(1);
                pass.End();
            }

            wgpu::CommandBuffer commands = commandEncoder.Finish();
            queue.Submit(1, &commands);

            CheckBuffer(textureValues[i], outputBuffer);
        }
    }

    template <typename T, typename CheckBufferFn>
    void DoSamplingTestImpl(TestAspectAndSamplerType aspectAndSamplerType,
                            wgpu::ComputePipeline pipeline,
                            wgpu::TextureFormat format,
                            std::vector<T> textureValues,
                            uint32_t componentCount,
                            CheckBufferFn CheckBuffer) {
        wgpu::Texture inputTexture = CreateInputTexture(format);
        wgpu::TextureViewDescriptor inputViewDesc = {};
        switch (aspectAndSamplerType) {
            case TestAspectAndSamplerType::DepthAsDepth:
            case TestAspectAndSamplerType::DepthAsFloat:
                inputViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
                break;
            case TestAspectAndSamplerType::StencilAsUint:
                inputViewDesc.aspect = wgpu::TextureAspect::StencilOnly;
                break;
        }

        wgpu::Buffer outputBuffer = CreateOutputBuffer(componentCount);

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {{0, inputTexture.CreateView(&inputViewDesc)}, {1, outputBuffer}});

        for (size_t i = 0; i < textureValues.size(); ++i) {
            // Set the input depth texture to the provided texture value
            wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
            switch (aspectAndSamplerType) {
                case TestAspectAndSamplerType::DepthAsDepth:
                case TestAspectAndSamplerType::DepthAsFloat:
                    UpdateInputDepth(commandEncoder, inputTexture, format, textureValues[i]);
                    break;
                case TestAspectAndSamplerType::StencilAsUint:
                    UpdateInputStencil(commandEncoder, inputTexture, format, textureValues[i]);
                    break;
            }

            // Sample into the output buffer
            {
                wgpu::ComputePassEncoder pass = commandEncoder.BeginComputePass();
                pass.SetPipeline(pipeline);
                pass.SetBindGroup(0, bindGroup);
                pass.DispatchWorkgroups(1);
                pass.End();
            }

            wgpu::CommandBuffer commands = commandEncoder.Finish();
            queue.Submit(1, &commands);

            CheckBuffer(textureValues[i], outputBuffer);
        }
    }

    template <typename T>
    void DoSamplingTest(TestAspectAndSamplerType aspectAndSamplerType,
                        wgpu::RenderPipeline pipeline,
                        wgpu::TextureFormat format,
                        std::vector<T> textureValues,
                        T tolerance = {}) {
        DoSamplingTestImpl(aspectAndSamplerType, pipeline, format, textureValues, 1,
                           [this, tolerance](T expected, wgpu::Buffer buffer) {
                               EXPECT_BUFFER(buffer, 0, sizeof(T),
                                             new ::dawn::detail::ExpectEq<T>(expected, tolerance));
                           });
    }

    template <typename T>
    void DoSamplingTest(TestAspectAndSamplerType aspectAndSamplerType,
                        wgpu::ComputePipeline pipeline,
                        wgpu::TextureFormat format,
                        std::vector<T> textureValues,
                        T tolerance = {}) {
        DoSamplingTestImpl(aspectAndSamplerType, pipeline, format, textureValues, 1,
                           [this, tolerance](T expected, wgpu::Buffer buffer) {
                               EXPECT_BUFFER(buffer, 0, sizeof(T),
                                             new ::dawn::detail::ExpectEq<T>(expected, tolerance));
                           });
    }

    class ExtraStencilComponentsExpectation : public detail::Expectation {
        using StencilData = std::array<uint32_t, 4>;

      public:
        explicit ExtraStencilComponentsExpectation(uint32_t expected) : mExpected(expected) {}

        ~ExtraStencilComponentsExpectation() override = default;

        testing::AssertionResult Check(const void* rawData, size_t size) override {
            DAWN_ASSERT(size == sizeof(StencilData));
            const uint32_t* data = static_cast<const uint32_t*>(rawData);

            StencilData ssss = {mExpected, mExpected, mExpected, mExpected};
            StencilData s001 = {mExpected, 0, 0, 1};

            if (memcmp(data, ssss.data(), size) == 0 || memcmp(data, s001.data(), size) == 0) {
                return testing::AssertionSuccess();
            }

            return testing::AssertionFailure()
                   << "Expected stencil data to be " << "(" << ssss[0] << ", " << ssss[1] << ", "
                   << ssss[2] << ", " << ssss[3] << ") or " << "(" << s001[0] << ", " << s001[1]
                   << ", " << s001[2] << ", " << s001[3] << "). Got " << "(" << data[0] << ", "
                   << data[1] << ", " << data[2] << ", " << data[3] << ").";
        }

      private:
        uint32_t mExpected;
    };

    void DoSamplingExtraStencilComponentsRenderTest(TestAspectAndSamplerType aspectAndSamplerType,
                                                    wgpu::TextureFormat format,
                                                    std::vector<uint8_t> textureValues) {
        DoSamplingTestImpl(
            aspectAndSamplerType,
            CreateSamplingRenderPipeline({TestAspectAndSamplerType::StencilAsUint}, {0, 1, 2, 3}),
            format, textureValues, 4, [&](uint32_t expected, wgpu::Buffer buffer) {
                EXPECT_BUFFER(buffer, 0, 4 * sizeof(uint32_t),
                              new ExtraStencilComponentsExpectation(expected));
            });
    }

    void DoSamplingExtraStencilComponentsComputeTest(TestAspectAndSamplerType aspectAndSamplerType,
                                                     wgpu::TextureFormat format,
                                                     std::vector<uint8_t> textureValues) {
        DoSamplingTestImpl(
            aspectAndSamplerType,
            CreateSamplingComputePipeline({TestAspectAndSamplerType::StencilAsUint}, {0, 1, 2, 3}),
            format, textureValues, 4, [&](uint32_t expected, wgpu::Buffer buffer) {
                EXPECT_BUFFER(buffer, 0, 4 * sizeof(uint32_t),
                              new ExtraStencilComponentsExpectation(expected));
            });
    }

    static bool CompareFunctionPasses(float compareRef,
                                      wgpu::CompareFunction compare,
                                      float textureValue) {
        switch (compare) {
            case wgpu::CompareFunction::Never:
                return false;
            case wgpu::CompareFunction::Less:
                return compareRef < textureValue;
            case wgpu::CompareFunction::LessEqual:
                return compareRef <= textureValue;
            case wgpu::CompareFunction::Greater:
                return compareRef > textureValue;
            case wgpu::CompareFunction::GreaterEqual:
                return compareRef >= textureValue;
            case wgpu::CompareFunction::Equal:
                return compareRef == textureValue;
            case wgpu::CompareFunction::NotEqual:
                return compareRef != textureValue;
            case wgpu::CompareFunction::Always:
                return true;
            default:
                return false;
        }
    }

    void DoDepthCompareRefTest(wgpu::RenderPipeline pipeline,
                               wgpu::TextureFormat format,
                               float compareRef,
                               wgpu::CompareFunction compare,
                               std::vector<float> textureValues) {
        queue.WriteBuffer(mUniformBuffer, 0, &compareRef, sizeof(float));

        wgpu::SamplerDescriptor samplerDesc;
        samplerDesc.compare = compare;
        wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

        wgpu::Texture inputTexture = CreateInputTexture(format);
        wgpu::TextureViewDescriptor inputViewDesc = {};
        inputViewDesc.aspect = wgpu::TextureAspect::DepthOnly;

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {
                                     {0, sampler},
                                     {1, inputTexture.CreateView(&inputViewDesc)},
                                     {2, mUniformBuffer},
                                 });

        wgpu::Texture outputTexture = CreateOutputTexture(wgpu::TextureFormat::R32Float);
        for (float textureValue : textureValues) {
            // Set the input depth texture to the provided texture value
            wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
            UpdateInputDepth(commandEncoder, inputTexture, format, textureValue);

            // Render into the output texture
            {
                utils::ComboRenderPassDescriptor passDescriptor({outputTexture.CreateView()});
                wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
                pass.SetPipeline(pipeline);
                pass.SetBindGroup(0, bindGroup);
                pass.Draw(1);
                pass.End();
            }

            wgpu::CommandBuffer commands = commandEncoder.Finish();
            queue.Submit(1, &commands);

            EXPECT_TEXTURE_EQ(CompareFunctionPasses(compareRef, compare, textureValue) ? 1.f : 0.f,
                              outputTexture, {0, 0})
                << "compareRef=" << compareRef << " " << compare
                << " textureValue=" << textureValue;
        }
    }

    void DoDepthCompareRefTest(wgpu::ComputePipeline pipeline,
                               wgpu::TextureFormat format,
                               float compareRef,
                               wgpu::CompareFunction compare,
                               std::vector<float> textureValues) {
        queue.WriteBuffer(mUniformBuffer, 0, &compareRef, sizeof(float));

        wgpu::SamplerDescriptor samplerDesc;
        samplerDesc.compare = compare;
        wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

        wgpu::Texture inputTexture = CreateInputTexture(format);
        wgpu::TextureViewDescriptor inputViewDesc = {};
        inputViewDesc.aspect = wgpu::TextureAspect::DepthOnly;

        wgpu::Buffer outputBuffer = CreateOutputBuffer();

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {{0, sampler},
                                  {1, inputTexture.CreateView(&inputViewDesc)},
                                  {2, mUniformBuffer},
                                  {3, outputBuffer}});

        for (float textureValue : textureValues) {
            // Set the input depth texture to the provided texture value
            wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
            UpdateInputDepth(commandEncoder, inputTexture, format, textureValue);

            // Sample into the output buffer
            {
                wgpu::ComputePassEncoder pass = commandEncoder.BeginComputePass();
                pass.SetPipeline(pipeline);
                pass.SetBindGroup(0, bindGroup);
                pass.DispatchWorkgroups(1);
                pass.End();
            }

            wgpu::CommandBuffer commands = commandEncoder.Finish();
            queue.Submit(1, &commands);

            float float0 = 0.f;
            float float1 = 1.f;
            float* expected =
                CompareFunctionPasses(compareRef, compare, textureValue) ? &float1 : &float0;

            EXPECT_BUFFER_U32_EQ(*reinterpret_cast<uint32_t*>(expected), outputBuffer, 0)
                << "compareRef=" << compareRef << " " << compare
                << " textureValue=" << textureValue;
        }
    }

  private:
    wgpu::Buffer mUniformBuffer;
    bool mIsFormatSupported = false;
};

// Repro test for crbug.com/dawn/1187 where sampling a depth texture returns values not in [0, 1]
TEST_P(DepthStencilSamplingTest, CheckDepthTextureRange) {
    // TODO(crbug.com/dawn/1187): The test fails on ANGLE D3D11, investigate why.
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());

    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 6 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsARM());

    constexpr uint32_t kWidth = 16;
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        const kWidth = 16.0;

        // Write a point per texel of a height = 0 texture with depths varying between 0 and 1.
        @vertex fn vs(@builtin(vertex_index) index : u32) -> @builtin(position) vec4f {
            let x = (f32(index) + 0.5) / kWidth * 2.0 - 1.0;
            let z = f32(index) / (kWidth - 1.0);
            return vec4(x, 0.0, z, 1.0);
        }

        // Writes an unused color, we only care about the depth.
        @fragment fn fs1() -> @location(0) f32 {
            return -42.0;
        }

        @group(0) @binding(0) var t : texture_depth_2d;
        @group(0) @binding(1) var s : sampler;

        // Check each depth texture texel has the expected value and outputs a "bool".
        @fragment fn fs2(@builtin(position) pos : vec4f) -> @location(0) f32 {
            let x = pos.x / kWidth;
            let depth = textureSample(t, s, vec2(x, 0.5));

            let index = pos.x - 0.5;
            let expectedDepth = index / (kWidth - 1.0);

            if (abs(depth - expectedDepth) < 0.001) {
                return 1.0;
            }
            return 0.0;
        }

        // Check each depth texture texel has the expected value and outputs a "bool".
        // Compat can not use texture_depth_2d with a non-comparison sampler
        @group(0) @binding(0) var t2 : texture_2d<f32>;
        @fragment fn fs2Compat(@builtin(position) pos : vec4f) -> @location(0) f32 {
            let x = pos.x / kWidth;
            let depth = textureSample(t2, s, vec2(x, 0.5)).r;

            let index = pos.x - 0.5;
            let expectedDepth = index / (kWidth - 1.0);

            if (abs(depth - expectedDepth) < 0.001) {
                return 1.0;
            }
            return 0.0;
        }
    )");

    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Fragment,
                     IsCompatibilityMode() ? wgpu::TextureSampleType::UnfilterableFloat
                                           : wgpu::TextureSampleType::Depth},
                    {1, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::NonFiltering},
                });

    wgpu::PipelineLayoutDescriptor plDescriptor;
    plDescriptor.bindGroupLayoutCount = 1;
    plDescriptor.bindGroupLayouts = &bgl;

    // The first pipeline will write to the depth texture.
    utils::ComboRenderPipelineDescriptor pDesc1;
    pDesc1.vertex.module = module;
    pDesc1.cFragment.module = module;
    pDesc1.cFragment.entryPoint = "fs1";
    pDesc1.cTargets[0].format = wgpu::TextureFormat::R32Float;
    pDesc1.primitive.topology = wgpu::PrimitiveTopology::PointList;
    pDesc1.EnableDepthStencil(wgpu::TextureFormat::Depth24PlusStencil8);
    pDesc1.cDepthStencil.depthWriteEnabled = wgpu::OptionalBool::True;
    wgpu::RenderPipeline pipeline1 = device.CreateRenderPipeline(&pDesc1);

    // The second pipeline checks the depth texture and outputs 1 to a texel on success.
    utils::ComboRenderPipelineDescriptor pDesc2;
    pDesc2.layout = device.CreatePipelineLayout(&plDescriptor);
    pDesc2.vertex.module = module;
    pDesc2.cFragment.module = module;
    pDesc2.cFragment.entryPoint = IsCompatibilityMode() ? "fs2Compat" : "fs2";
    pDesc2.cTargets[0].format = wgpu::TextureFormat::R32Float;
    pDesc2.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline pipeline2 = device.CreateRenderPipeline(&pDesc2);

    // Initialize resources.
    wgpu::TextureDescriptor tDesc;
    tDesc.size = {kWidth};
    tDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment |
                  wgpu::TextureUsage::CopySrc;
    tDesc.format = wgpu::TextureFormat::R32Float;
    wgpu::Texture colorTexture = device.CreateTexture(&tDesc);

    tDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
    wgpu::Texture depthTexture = device.CreateTexture(&tDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // Render the depth texture with varied depth values.
    utils::ComboRenderPassDescriptor passDesc1({colorTexture.CreateView()},
                                               depthTexture.CreateView());
    wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&passDesc1);
    pass1.SetPipeline(pipeline1);
    pass1.Draw(kWidth);
    pass1.End();

    // Check the depth values and output the result in a "boolean" encoded in an f32
    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.aspect = wgpu::TextureAspect::DepthOnly;
    wgpu::BindGroup bg = utils::MakeBindGroup(device, pipeline2.GetBindGroupLayout(0),
                                              {
                                                  {0, depthTexture.CreateView(&viewDesc)},
                                                  {1, device.CreateSampler()},
                                              });

    utils::ComboRenderPassDescriptor passDesc2({colorTexture.CreateView()});
    wgpu::RenderPassEncoder pass2 = encoder.BeginRenderPass(&passDesc2);
    pass2.SetPipeline(pipeline2);
    pass2.SetBindGroup(0, bg);
    pass2.Draw(kWidth);
    pass2.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Check all booleans are true.
    for (uint32_t x = 0; x < kWidth; x++) {
        EXPECT_PIXEL_FLOAT_EQ(1.0f, colorTexture, x, 0);
    }
}

// Test that sampling a depth/stencil texture at components 1, 2, and 3 yield 0, 0, and 1
// respectively
TEST_P(DepthStencilSamplingTest, SampleExtraComponents) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 6 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsARM());
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 1);

    wgpu::TextureFormat format = GetParam().mTextureFormat;

    DoSamplingExtraStencilComponentsRenderTest(TestAspectAndSamplerType::StencilAsUint, format,
                                               {uint8_t(42), uint8_t(37)});

    DoSamplingExtraStencilComponentsComputeTest(TestAspectAndSamplerType::StencilAsUint, format,
                                                {uint8_t(42), uint8_t(37)});
}

// Test sampling both depth and stencil with a render/compute pipeline works.
TEST_P(DepthStencilSamplingTest, SampleDepthAndStencilRender) {
    // In compat, you can't have different views of the same texture in the same draw command.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    wgpu::TextureFormat format = GetParam().mTextureFormat;

    // Depth24PlusStencil8 could be mapped to a depth24unorm format and ULP for 24-bit unorm is
    // 1 / (2 ^ 24 - 1) whereas ULP for 32-bit float varies and can be smaller.
    const float tolerance = format == wgpu::TextureFormat::Depth24PlusStencil8 ? 1e-6f : 0.0f;

    wgpu::SamplerDescriptor samplerDesc;
    wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

    wgpu::Texture inputTexture = CreateInputTexture(format);

    wgpu::TextureViewDescriptor depthViewDesc = {};
    depthViewDesc.aspect = wgpu::TextureAspect::DepthOnly;

    wgpu::TextureViewDescriptor stencilViewDesc = {};
    stencilViewDesc.aspect = wgpu::TextureAspect::StencilOnly;

    // With render pipeline
    {
        wgpu::RenderPipeline pipeline = CreateSamplingRenderPipeline(
            {TestAspectAndSamplerType::DepthAsDepth, TestAspectAndSamplerType::StencilAsUint}, 0);

        wgpu::Buffer depthOutput = CreateOutputBuffer();
        wgpu::Buffer stencilOutput = CreateOutputBuffer();

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {
                                     {0, inputTexture.CreateView(&depthViewDesc)},
                                     {1, depthOutput},
                                     {2, inputTexture.CreateView(&stencilViewDesc)},
                                     {3, stencilOutput},
                                 });

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

        // Initialize both depth and stencil aspects.
        utils::ComboRenderPassDescriptor passDescriptor({}, inputTexture.CreateView());
        passDescriptor.cDepthStencilAttachmentInfo.depthClearValue = 0.43f;
        passDescriptor.cDepthStencilAttachmentInfo.stencilClearValue = 31;

        {
            wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
            pass.End();
        }

        // Render into the output textures
        {
            utils::BasicRenderPass renderPass =
                utils::CreateBasicRenderPass(device, 1, 1, wgpu::TextureFormat::RGBA8Unorm);
            wgpu::RenderPassEncoder pass =
                commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(1);
            pass.End();
        }

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        float expectedDepth = 0.0f;
        memcpy(&expectedDepth, &passDescriptor.cDepthStencilAttachmentInfo.depthClearValue,
               sizeof(float));
        EXPECT_BUFFER(depthOutput, 0, sizeof(float),
                      new ::dawn::detail::ExpectEq<float>(expectedDepth, tolerance));

        uint8_t expectedStencil = 0;
        memcpy(&expectedStencil, &passDescriptor.cDepthStencilAttachmentInfo.stencilClearValue,
               sizeof(uint8_t));
        EXPECT_BUFFER_U32_EQ(expectedStencil, stencilOutput, 0);
    }

    // With compute pipeline
    {
        wgpu::ComputePipeline pipeline = CreateSamplingComputePipeline(
            {TestAspectAndSamplerType::DepthAsDepth, TestAspectAndSamplerType::StencilAsUint}, 0);

        wgpu::Buffer depthOutput = CreateOutputBuffer();
        wgpu::Buffer stencilOutput = CreateOutputBuffer();

        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {{0, inputTexture.CreateView(&depthViewDesc)},
                                  {1, depthOutput},
                                  {2, inputTexture.CreateView(&stencilViewDesc)},
                                  {3, stencilOutput}});

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        // Initialize both depth and stencil aspects.
        utils::ComboRenderPassDescriptor passDescriptor({}, inputTexture.CreateView());
        passDescriptor.cDepthStencilAttachmentInfo.depthClearValue = 0.43f;
        passDescriptor.cDepthStencilAttachmentInfo.stencilClearValue = 31;

        {
            wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
            pass.End();
        }

        // Sample into the output buffers
        {
            wgpu::ComputePassEncoder pass = commandEncoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.DispatchWorkgroups(1);
            pass.End();
        }

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        float expectedDepth = 0.0f;
        memcpy(&expectedDepth, &passDescriptor.cDepthStencilAttachmentInfo.depthClearValue,
               sizeof(float));
        EXPECT_BUFFER(depthOutput, 0, sizeof(float),
                      new ::dawn::detail::ExpectEq<float>(expectedDepth, tolerance));

        uint8_t expectedStencil = 0;
        memcpy(&expectedStencil, &passDescriptor.cDepthStencilAttachmentInfo.stencilClearValue,
               sizeof(uint8_t));
        EXPECT_BUFFER_U32_EQ(expectedStencil, stencilOutput, 0);
    }
}

class DepthSamplingTest : public DepthStencilSamplingTest {};

// Test that sampling a depth texture with a render/compute pipeline works
TEST_P(DepthSamplingTest, SampleDepthOnly) {
    // textureLoad with texture_depth_xxx is not supported in compat mode.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    // TODO(crbug.com/dawn/2552): diagnose this flake on Pixel 6 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsARM());

    // TODO(crbug.com/341258943): Fails on Win/Intel UHD 770.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsIntelGen12() && IsANGLED3D11());

    wgpu::TextureFormat format = GetParam().mTextureFormat;

    // ULP for Depth16Unorm is 1 / (2 ^ 16 - 1). Similarly Depth24PlusStencil8 could be mapped to a
    // depth24unorm format and ULP for 24-bit unorm is 1 / (2 ^ 24 - 1). But ULP for 32-bit float
    // varies and can be smaller so set a tolerance (any value greater than unorm ULP is fine).
    const float tolerance = (format == wgpu::TextureFormat::Depth16Unorm ||
                             format == wgpu::TextureFormat::Depth24PlusStencil8)
                                ? 1e-4f
                                : 0.0f;

    // Test 0, between [0, 1], and 1.
    DoSamplingTest(TestAspectAndSamplerType::DepthAsDepth,
                   CreateSamplingRenderPipeline({TestAspectAndSamplerType::DepthAsDepth}, 0),
                   format, kNormalizedTextureValues, tolerance);
    DoSamplingTest(TestAspectAndSamplerType::DepthAsFloat,
                   CreateSamplingRenderPipeline({TestAspectAndSamplerType::DepthAsFloat}, 0),
                   format, kNormalizedTextureValues, tolerance);

    DoSamplingTest(TestAspectAndSamplerType::DepthAsDepth,
                   CreateSamplingComputePipeline({TestAspectAndSamplerType::DepthAsDepth}, 0),
                   format, kNormalizedTextureValues, tolerance);
    DoSamplingTest(TestAspectAndSamplerType::DepthAsFloat,
                   CreateSamplingComputePipeline({TestAspectAndSamplerType::DepthAsFloat}, 0),
                   format, kNormalizedTextureValues, tolerance);
}

// Test that sampling in a render pipeline with all of the compare functions works.
TEST_P(DepthSamplingTest, CompareFunctionsRender) {
    // Initialization via renderPass loadOp doesn't work on Mac Intel.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && IsIntel());

    // TODO(dawn:1549) Fails on Qualcomm-based Android devices.
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsQualcomm());

    wgpu::TextureFormat format = GetParam().mTextureFormat;
    wgpu::RenderPipeline pipeline = CreateComparisonRenderPipeline();

    bool isDepthUnorm = format == wgpu::TextureFormat::Depth16Unorm ||
                        (format == wgpu::TextureFormat::Depth24PlusStencil8 &&
                         HasToggleEnabled("use_packed_depth24_unorm_stencil8_format"));

    // Test a "normal" ref value between 0 and 1; as well as negative and > 1 refs.
    for (float compareRef : kCompareRefs) {
        if (isDepthUnorm) {
            compareRef = std::clamp(compareRef, 0.0f, 1.0f);
        }
        // Test 0, below the ref, equal to, above the ref, and 1.
        for (wgpu::CompareFunction f : kCompareFunctions) {
            DoDepthCompareRefTest(pipeline, format, compareRef, f, kNormalizedTextureValues);
        }
    }
}

class StencilSamplingTest : public DepthStencilSamplingTest {};

// Test that sampling a stencil texture with a render/compute pipeline works
TEST_P(StencilSamplingTest, SampleStencilOnly) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 6 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsARM());
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 1);

    wgpu::TextureFormat format = GetParam().mTextureFormat;

    DoSamplingTest(TestAspectAndSamplerType::StencilAsUint,
                   CreateSamplingRenderPipeline({TestAspectAndSamplerType::StencilAsUint}, 0),
                   format, kStencilValues);

    DoSamplingTest(TestAspectAndSamplerType::StencilAsUint,
                   CreateSamplingComputePipeline({TestAspectAndSamplerType::StencilAsUint}, 0),
                   format, kStencilValues);
}

DAWN_INSTANTIATE_TEST_P(DepthStencilSamplingTest,
                        {D3D11Backend(), D3D11Backend({"use_packed_depth24_unorm_stencil8_format"}),
                         D3D12Backend(), D3D12Backend({"use_packed_depth24_unorm_stencil8_format"}),
                         MetalBackend(), OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
                        std::vector<wgpu::TextureFormat>(utils::kDepthAndStencilFormats.begin(),
                                                         utils::kDepthAndStencilFormats.end()));

DAWN_INSTANTIATE_TEST_P(DepthSamplingTest,
                        {D3D11Backend(), D3D11Backend({"use_packed_depth24_unorm_stencil8_format"}),
                         MetalBackend(), OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
                        std::vector<wgpu::TextureFormat>(utils::kDepthFormats.begin(),
                                                         utils::kDepthFormats.end()));

DAWN_INSTANTIATE_TEST_P(StencilSamplingTest,
                        {D3D12Backend(), D3D12Backend({"use_packed_depth24_unorm_stencil8_format"}),
                         MetalBackend(),
                         MetalBackend({"metal_use_combined_depth_stencil_format_for_stencil8"}),
                         OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
                        std::vector<wgpu::TextureFormat>(utils::kStencilFormats.begin(),
                                                         utils::kStencilFormats.end()));

}  // anonymous namespace
}  // namespace dawn
