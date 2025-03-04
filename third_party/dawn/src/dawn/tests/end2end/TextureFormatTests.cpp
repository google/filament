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

#include <cmath>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

enum class TextureComponentType {
    Float,
    Sint,
    Uint,
};

// An expectation for float buffer content that can correctly compare different NaN values and
// supports a basic tolerance for comparison of finite values.
class ExpectFloatWithTolerance : public detail::Expectation {
  public:
    ExpectFloatWithTolerance(std::vector<float> expected, float tolerance)
        : mExpected(std::move(expected)), mTolerance(tolerance) {}

    testing::AssertionResult Check(const void* data, size_t size) override {
        DAWN_ASSERT(size == sizeof(float) * mExpected.size());

        const float* actual = static_cast<const float*>(data);

        for (size_t i = 0; i < mExpected.size(); ++i) {
            float expectedValue = mExpected[i];
            float actualValue = actual[i];

            if (!FloatsMatch(expectedValue, actualValue)) {
                testing::AssertionResult result = testing::AssertionFailure()
                                                  << "Expected data[" << i << "] to be close to "
                                                  << expectedValue << ", actual " << actualValue
                                                  << "\n";
                return result;
            }
        }
        return testing::AssertionSuccess();
    }

  private:
    bool FloatsMatch(float expected, float actual) {
        if (std::isnan(expected)) {
            return std::isnan(actual);
        }

        if (std::isinf(expected)) {
            return std::isinf(actual) && std::signbit(expected) == std::signbit(actual);
        }

        if (mTolerance == 0.0f) {
            return expected == actual;
        }

        float error = std::abs(expected - actual);
        return error < mTolerance;
    }

    std::vector<float> mExpected;
    float mTolerance;
};

// An expectation for float16 buffers that can correctly compare NaNs (all NaNs are equivalent).
class ExpectFloat16 : public detail::Expectation {
  public:
    explicit ExpectFloat16(std::vector<uint16_t> expected) : mExpected(std::move(expected)) {}

    testing::AssertionResult Check(const void* data, size_t size) override {
        DAWN_ASSERT(size == sizeof(uint16_t) * mExpected.size());

        const uint16_t* actual = static_cast<const uint16_t*>(data);

        for (size_t i = 0; i < mExpected.size(); ++i) {
            uint16_t expectedValue = mExpected[i];
            uint16_t actualValue = actual[i];

            if (!Floats16Match(expectedValue, actualValue)) {
                testing::AssertionResult result = testing::AssertionFailure()
                                                  << "Expected data[" << i << "] to be "
                                                  << expectedValue << ", actual " << actualValue
                                                  << "\n";
                return result;
            }
        }
        return testing::AssertionSuccess();
    }

  private:
    bool Floats16Match(float expected, float actual) {
        if (IsFloat16NaN(expected)) {
            return IsFloat16NaN(actual);
        }

        return expected == actual;
    }

    std::vector<uint16_t> mExpected;
};

// An expectation for RG11B10Ufloat buffer content that can correctly compare different NaN values
class ExpectRG11B10Ufloat : public detail::Expectation {
  public:
    explicit ExpectRG11B10Ufloat(std::vector<uint32_t> expected) : mExpected(std::move(expected)) {}

    testing::AssertionResult Check(const void* data, size_t size) override {
        DAWN_ASSERT(size == sizeof(uint32_t) * mExpected.size());

        const uint32_t* actual = static_cast<const uint32_t*>(data);

        for (size_t i = 0; i < mExpected.size(); ++i) {
            uint32_t expectedValue = mExpected[i];
            uint32_t actualValue = actual[i];

            if (!RG11B10UfloatMatch(expectedValue, actualValue)) {
                testing::AssertionResult result = testing::AssertionFailure()
                                                  << "Expected data[" << i << "] to be "
                                                  << expectedValue << ", actual " << actualValue
                                                  << "\n";
                return result;
            }
        }
        return testing::AssertionSuccess();
    }

  private:
    bool RG11B10UfloatMatch(uint32_t expected, uint32_t actual) {
        const uint32_t expectedR = expected & 0x7FF;
        const uint32_t expectedG = (expected >> 11) & 0x7FF;
        const uint32_t expectedB = (expected >> 22) & 0x3FF;

        const uint32_t actualR = actual & 0x7FF;
        const uint32_t actualG = (actual >> 11) & 0x7FF;
        const uint32_t actualB = (actual >> 22) & 0x3FF;

        return Float11Match(expectedR, actualR) && Float11Match(expectedG, actualG) &&
               Float10Match(expectedB, actualB);
    }

    bool Float11Match(uint32_t expected, uint32_t actual) {
        DAWN_ASSERT((expected & ~0x7FF) == 0);
        DAWN_ASSERT((actual & ~0x7FF) == 0);

        if (IsFloat11NaN(expected)) {
            return IsFloat11NaN(actual);
        }

        return expected == actual;
    }

    bool Float10Match(uint32_t expected, uint32_t actual) {
        DAWN_ASSERT((expected & ~0x3FF) == 0);
        DAWN_ASSERT((actual & ~0x3FF) == 0);

        if (IsFloat10NaN(expected)) {
            return IsFloat10NaN(actual);
        }

        return expected == actual;
    }

    // The number is NaN if exponent bits are all 1 and mantissa is non-zero
    bool IsFloat11NaN(uint32_t value) {
        DAWN_ASSERT((value & ~0x7FF) == 0);

        return ((value & 0x7C0) == 0x7C0) && ((value & 0x3F) != 0);
    }

    bool IsFloat10NaN(uint32_t value) {
        DAWN_ASSERT((value & ~0x3FF) == 0);

        return ((value & 0x3E0) == 0x3E0) && ((value & 0x1F) != 0);
    }

    std::vector<uint32_t> mExpected;
};

class TextureFormatTest : public DawnTest {
  protected:
    // Structure containing all the information that tests need to know about the format.
    struct FormatTestInfo {
        wgpu::TextureFormat format;
        uint32_t texelByteSize;
        TextureComponentType type;
        uint32_t componentCount;
    };

    // Returns a reprensentation of a format that can be used to contain the "uncompressed" values
    // of the format. That the equivalent format with all channels 32bit-sized.
    FormatTestInfo GetUncompressedFormatInfo(FormatTestInfo formatInfo) {
        switch (formatInfo.type) {
            case TextureComponentType::Float:
                return {wgpu::TextureFormat::RGBA32Float, 16, formatInfo.type, 4};
            case TextureComponentType::Sint:
                return {wgpu::TextureFormat::RGBA32Sint, 16, formatInfo.type, 4};
            case TextureComponentType::Uint:
                return {wgpu::TextureFormat::RGBA32Uint, 16, formatInfo.type, 4};
            default:
                DAWN_UNREACHABLE();
        }
    }

    // Return a pipeline that can be used in a full-texture draw to sample from the texture in the
    // bindgroup and output its decompressed values to the render target.
    wgpu::RenderPipeline CreateSamplePipeline(FormatTestInfo sampleFormatInfo,
                                              FormatTestInfo renderFormatInfo) {
        utils::ComboRenderPipelineDescriptor desc;

        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-3.0, -1.0),
                    vec2f( 3.0, -1.0),
                    vec2f( 0.0,  2.0));

                return vec4f(pos[VertexIndex], 0.0, 1.0);
            })");

        // Compute the WGSL type of the texture's data.
        const char* type = utils::GetWGSLColorTextureComponentType(sampleFormatInfo.format);

        std::ostringstream fsSource;
        fsSource << "@group(0) @binding(0) var myTexture : texture_2d<" << type << ">;\n";
        fsSource << "struct FragmentOut {\n";
        fsSource << "   @location(0) color : vec4<" << type << ">\n";
        fsSource << R"(}
            @fragment
            fn main(@builtin(position) FragCoord : vec4f) -> FragmentOut {
                var output : FragmentOut;
                output.color = textureLoad(myTexture, vec2i(FragCoord.xy), 0);
                return output;
            })";

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fsSource.str().c_str());

        desc.vertex.module = vsModule;
        desc.cFragment.module = fsModule;
        desc.cTargets[0].format = renderFormatInfo.format;

        return device.CreateRenderPipeline(&desc);
    }

    // The sampling test uploads the sample data in a texture with the sampleFormatInfo.format.
    // It then samples from it and renders the results in a texture with the
    // renderFormatInfo.format format. Finally it checks that the data rendered matches
    // expectedRenderData, using the cutom expectation if present.
    void DoSampleTest(FormatTestInfo sampleFormatInfo,
                      const void* sampleData,
                      size_t sampleDataSize,
                      FormatTestInfo renderFormatInfo,
                      const void* expectedRenderData,
                      size_t expectedRenderDataSize,
                      detail::Expectation* customExpectation) {
        // The input data should contain an exact number of texels
        DAWN_ASSERT(sampleDataSize % sampleFormatInfo.texelByteSize == 0);
        uint32_t width = sampleDataSize / sampleFormatInfo.texelByteSize;

        // The input data must be a multiple of 4 byte in length for WriteBuffer
        DAWN_ASSERT(sampleDataSize % 4 == 0);
        DAWN_ASSERT(expectedRenderDataSize % 4 == 0);

        // Create the texture we will sample from
        wgpu::TextureDescriptor sampleTextureDesc;
        sampleTextureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
        sampleTextureDesc.size = {width, 1, 1};
        sampleTextureDesc.format = sampleFormatInfo.format;
        wgpu::Texture sampleTexture = device.CreateTexture(&sampleTextureDesc);

        wgpu::Buffer uploadBuffer = utils::CreateBufferFromData(device, sampleData, sampleDataSize,
                                                                wgpu::BufferUsage::CopySrc);

        // Create the texture that we will render results to
        DAWN_ASSERT(expectedRenderDataSize == width * renderFormatInfo.texelByteSize);

        wgpu::TextureDescriptor renderTargetDesc;
        renderTargetDesc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
        renderTargetDesc.size = {width, 1, 1};
        renderTargetDesc.format = renderFormatInfo.format;

        wgpu::Texture renderTarget = device.CreateTexture(&renderTargetDesc);

        // Create the readback buffer for the data in renderTarget
        wgpu::BufferDescriptor readbackBufferDesc;
        readbackBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
        readbackBufferDesc.size = expectedRenderDataSize;
        wgpu::Buffer readbackBuffer = device.CreateBuffer(&readbackBufferDesc);

        // Prepare objects needed to sample from texture in the renderpass
        wgpu::RenderPipeline pipeline = CreateSamplePipeline(sampleFormatInfo, renderFormatInfo);
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampleTexture.CreateView()}});

        // Encode commands for the test that fill texture, sample it to render to renderTarget then
        // copy renderTarget in a buffer so we can read it easily.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        {
            wgpu::TexelCopyBufferInfo bufferView =
                utils::CreateTexelCopyBufferInfo(uploadBuffer, 0, 256);
            wgpu::TexelCopyTextureInfo textureView =
                utils::CreateTexelCopyTextureInfo(sampleTexture, 0, {0, 0, 0});
            wgpu::Extent3D extent{width, 1, 1};
            encoder.CopyBufferToTexture(&bufferView, &textureView, &extent);
        }

        utils::ComboRenderPassDescriptor renderPassDesc({renderTarget.CreateView()});
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDesc);
        renderPass.SetPipeline(pipeline);
        renderPass.SetBindGroup(0, bindGroup);
        renderPass.Draw(3);
        renderPass.End();

        {
            wgpu::TexelCopyBufferInfo bufferView =
                utils::CreateTexelCopyBufferInfo(readbackBuffer, 0, 256);
            wgpu::TexelCopyTextureInfo textureView =
                utils::CreateTexelCopyTextureInfo(renderTarget, 0, {0, 0, 0});
            wgpu::Extent3D extent{width, 1, 1};
            encoder.CopyTextureToBuffer(&textureView, &bufferView, &extent);
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // For floats use a special expectation that understands how to compare NaNs and support a
        // tolerance.
        if (customExpectation != nullptr) {
            AddBufferExpectation(__FILE__, __LINE__, readbackBuffer, 0, expectedRenderDataSize,
                                 customExpectation);
        } else {
            EXPECT_BUFFER_U32_RANGE_EQ(static_cast<const uint32_t*>(expectedRenderData),
                                       readbackBuffer, 0,
                                       expectedRenderDataSize / sizeof(uint32_t));
        }
    }

    template <typename Data>
    std::vector<Data> ExpandDataTo4Component(const std::vector<Data>& originalData,
                                             uint32_t originalComponentCount,
                                             const std::array<Data, 4>& defaultValues) {
        std::vector<Data> result;

        for (size_t i = 0; i < originalData.size() / originalComponentCount; i++) {
            for (size_t component = 0; component < 4; component++) {
                if (component < originalComponentCount) {
                    result.push_back(originalData[i * originalComponentCount + component]);
                } else {
                    result.push_back(defaultValues[component]);
                }
            }
        }

        return result;
    }

    // Helper functions used to run tests that convert the typeful test objects to typeless void*

    template <typename TextureData, typename RenderData>
    void DoFormatSamplingTest(FormatTestInfo formatInfo,
                              const std::vector<TextureData>& textureData,
                              const std::vector<RenderData>& expectedRenderData,
                              detail::Expectation* customExpectation = nullptr) {
        FormatTestInfo renderFormatInfo = GetUncompressedFormatInfo(formatInfo);

        // Expand the expected data to be 4 component wide with the default sampling values of
        // (0, 0, 0, 1)
        std::array<RenderData, 4> defaultValues = {RenderData(0), RenderData(0), RenderData(0),
                                                   RenderData(1)};
        std::vector<RenderData> expandedRenderData =
            ExpandDataTo4Component(expectedRenderData, formatInfo.componentCount, defaultValues);

        DoSampleTest(formatInfo, textureData.data(), textureData.size() * sizeof(TextureData),
                     renderFormatInfo, expandedRenderData.data(),
                     expandedRenderData.size() * sizeof(RenderData), customExpectation);
    }

    template <typename TextureData>
    void DoFloatFormatSamplingTest(FormatTestInfo formatInfo,
                                   const std::vector<TextureData>& textureData,
                                   const std::vector<float>& expectedRenderData,
                                   float floatTolerance = 0.0f) {
        // Expand the expected data to be 4 component wide with the default sampling values of
        // (0, 0, 0, 1)
        std::array<float, 4> defaultValues = {0.0f, 0.0f, 0.0f, 1.0f};
        std::vector<float> expandedRenderData =
            ExpandDataTo4Component(expectedRenderData, formatInfo.componentCount, defaultValues);

        // Use a special expectation that understands how to compare NaNs and supports a tolerance.
        DoFormatSamplingTest(formatInfo, textureData, expectedRenderData,
                             new ExpectFloatWithTolerance(expandedRenderData, floatTolerance));
    }

    template <typename TextureData, typename RenderData>
    void DoFormatRenderingTest(FormatTestInfo formatInfo,
                               const std::vector<TextureData>& textureData,
                               const std::vector<RenderData>& expectedRenderData,
                               detail::Expectation* customExpectation = nullptr) {
        FormatTestInfo sampleFormatInfo = GetUncompressedFormatInfo(formatInfo);

        // Expand the sampling texture data to contain garbage data for unused components to check
        // that they don't influence the rendering result.
        std::array<TextureData, 4> garbageValues;
        garbageValues.fill(13);
        std::vector<TextureData> expandedTextureData =
            ExpandDataTo4Component(textureData, formatInfo.componentCount, garbageValues);

        DoSampleTest(sampleFormatInfo, expandedTextureData.data(),
                     expandedTextureData.size() * sizeof(TextureData), formatInfo,
                     expectedRenderData.data(), expectedRenderData.size() * sizeof(RenderData),
                     customExpectation);
    }

    // Below are helper functions for types that are very similar to one another so the logic is
    // shared.

    template <typename T>
    void DoUnormTest(FormatTestInfo formatInfo) {
        static_assert(!std::is_signed<T>::value && std::is_integral<T>::value);
        DAWN_ASSERT(sizeof(T) * formatInfo.componentCount == formatInfo.texelByteSize);
        DAWN_ASSERT(formatInfo.type == TextureComponentType::Float);

        DAWN_TEST_UNSUPPORTED_IF((utils::IsUnorm16TextureFormat(formatInfo.format)) &&
                                 !IsUnorm16TextureFormatsSupported());

        T maxValue = std::numeric_limits<T>::max();
        std::vector<T> textureData = {0, 1, maxValue, maxValue};
        std::vector<float> uncompressedData = {0.0f, 1.0f / maxValue, 1.0f, 1.0f};

        DoFormatSamplingTest(formatInfo, textureData, uncompressedData);
        DoFormatRenderingTest(formatInfo, uncompressedData, textureData);
    }

    template <typename T>
    void DoSnormTest(FormatTestInfo formatInfo) {
        static_assert(std::is_signed<T>::value && std::is_integral<T>::value);
        DAWN_ASSERT(sizeof(T) * formatInfo.componentCount == formatInfo.texelByteSize);
        DAWN_ASSERT(formatInfo.type == TextureComponentType::Float);

        DAWN_TEST_UNSUPPORTED_IF((utils::IsSnorm16TextureFormat(formatInfo.format)) &&
                                 !IsSnorm16TextureFormatsSupported());

        T maxValue = std::numeric_limits<T>::max();
        T minValue = std::numeric_limits<T>::min();
        std::vector<T> textureData = {0, 1, -1, maxValue, minValue, T(minValue + 1), 0, 0};
        std::vector<float> uncompressedData = {
            0.0f, 1.0f / maxValue, -1.0f / maxValue, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f};

        DoFloatFormatSamplingTest(formatInfo, textureData, uncompressedData, 0.0001f / maxValue);
        // Snorm formats aren't renderable because they are not guaranteed renderable in Vulkan
    }

    template <typename T>
    void DoUintTest(FormatTestInfo formatInfo) {
        static_assert(!std::is_signed<T>::value && std::is_integral<T>::value);
        DAWN_ASSERT(sizeof(T) * formatInfo.componentCount == formatInfo.texelByteSize);
        DAWN_ASSERT(formatInfo.type == TextureComponentType::Uint);

        T maxValue = std::numeric_limits<T>::max();
        std::vector<T> textureData = {0, 1, maxValue, maxValue};
        std::vector<uint32_t> uncompressedData = {0, 1, maxValue, maxValue};

        DoFormatSamplingTest(formatInfo, textureData, uncompressedData);
        DoFormatRenderingTest(formatInfo, uncompressedData, textureData);
    }

    template <typename T>
    void DoSintTest(FormatTestInfo formatInfo) {
        static_assert(std::is_signed<T>::value && std::is_integral<T>::value);
        DAWN_ASSERT(sizeof(T) * formatInfo.componentCount == formatInfo.texelByteSize);
        DAWN_ASSERT(formatInfo.type == TextureComponentType::Sint);

        T maxValue = std::numeric_limits<T>::max();
        T minValue = std::numeric_limits<T>::min();
        std::vector<T> textureData = {0, 1, maxValue, minValue};
        std::vector<int32_t> uncompressedData = {0, 1, maxValue, minValue};

        DoFormatSamplingTest(formatInfo, textureData, uncompressedData);
        DoFormatRenderingTest(formatInfo, uncompressedData, textureData);
    }

    void DoFloat32Test(FormatTestInfo formatInfo) {
        DAWN_ASSERT(sizeof(float) * formatInfo.componentCount == formatInfo.texelByteSize);
        DAWN_ASSERT(formatInfo.type == TextureComponentType::Float);

        std::vector<float> textureData = {+0.0f,   -0.0f, 1.0f,     1.0e-29f,
                                          1.0e29f, NAN,   INFINITY, -INFINITY};

        DoFloatFormatSamplingTest(formatInfo, textureData, textureData);
        DoFormatRenderingTest(formatInfo, textureData, textureData,
                              new ExpectFloatWithTolerance(textureData, 0.0f));
    }

    void DoFloat16Test(FormatTestInfo formatInfo) {
        DAWN_ASSERT(sizeof(int16_t) * formatInfo.componentCount == formatInfo.texelByteSize);
        DAWN_ASSERT(formatInfo.type == TextureComponentType::Float);

        std::vector<float> uncompressedData = {+0.0f,  -0.0f, 1.0f,     1.01e-4f,
                                               1.0e4f, NAN,   INFINITY, -INFINITY};
        std::vector<uint16_t> textureData;
        for (float value : uncompressedData) {
            textureData.push_back(Float32ToFloat16(value));
        }

        DoFloatFormatSamplingTest(formatInfo, textureData, uncompressedData, 1.0e-5f);

        // Use a special expectation that knows that all Float16 NaNs are equivalent.
        DoFormatRenderingTest(formatInfo, uncompressedData, textureData,
                              new ExpectFloat16(textureData));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::RG11B10UfloatRenderable})) {
            mIsRG11B10UfloatRenderableSupported = true;
            requiredFeatures.push_back(wgpu::FeatureName::RG11B10UfloatRenderable);
        }
        if (SupportsFeatures({wgpu::FeatureName::Unorm16TextureFormats})) {
            mIsUnorm16TextureFormatsSupported = true;
            requiredFeatures.push_back(wgpu::FeatureName::Unorm16TextureFormats);
        }
        if (SupportsFeatures({wgpu::FeatureName::Snorm16TextureFormats})) {
            mIsSnorm16TextureFormatsSupported = true;
            requiredFeatures.push_back(wgpu::FeatureName::Snorm16TextureFormats);
        }
        return requiredFeatures;
    }

    bool IsRG11B10UfloatRenderableSupported() { return mIsRG11B10UfloatRenderableSupported; }
    bool IsUnorm16TextureFormatsSupported() { return mIsUnorm16TextureFormatsSupported; }
    bool IsSnorm16TextureFormatsSupported() { return mIsSnorm16TextureFormatsSupported; }

  private:
    bool mIsRG11B10UfloatRenderableSupported = false;
    bool mIsUnorm16TextureFormatsSupported = false;
    bool mIsSnorm16TextureFormatsSupported = false;
};

// Test the R8Unorm format
TEST_P(TextureFormatTest, R8Unorm) {
    DoUnormTest<uint8_t>({wgpu::TextureFormat::R8Unorm, 1, TextureComponentType::Float, 1});
}

// Test the RG8Unorm format
TEST_P(TextureFormatTest, RG8Unorm) {
    DoUnormTest<uint8_t>({wgpu::TextureFormat::RG8Unorm, 2, TextureComponentType::Float, 2});
}

// Test the R16Unorm format
TEST_P(TextureFormatTest, R16Unorm) {
    DoUnormTest<uint16_t>({wgpu::TextureFormat::R16Unorm, 2, TextureComponentType::Float, 1});
}

// Test the RG16Unorm format
TEST_P(TextureFormatTest, RG16Unorm) {
    DoUnormTest<uint16_t>({wgpu::TextureFormat::RG16Unorm, 4, TextureComponentType::Float, 2});
}

// Test the RGBA16Unorm format
TEST_P(TextureFormatTest, RGBA16Unorm) {
    DoUnormTest<uint16_t>({wgpu::TextureFormat::RGBA16Unorm, 8, TextureComponentType::Float, 4});
}

// Test the RGBA8Unorm format
TEST_P(TextureFormatTest, RGBA8Unorm) {
    DoUnormTest<uint8_t>({wgpu::TextureFormat::RGBA8Unorm, 4, TextureComponentType::Float, 4});
}

// Test the BGRA8Unorm format
TEST_P(TextureFormatTest, BGRA8Unorm) {
    // Intel's implementation of BGRA on ES is broken: it claims to support
    // GL_EXT_texture_format_BGRA8888, but won't accept GL_BGRA or GL_BGRA8_EXT as internalFormat.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsOpenGLES() && IsLinux());

    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    uint8_t maxValue = std::numeric_limits<uint8_t>::max();
    std::vector<uint8_t> textureData = {maxValue, 1, 0, maxValue};
    std::vector<float> uncompressedData = {0.0f, 1.0f / maxValue, 1.0f, 1.0f};
    DoFormatSamplingTest({wgpu::TextureFormat::BGRA8Unorm, 4, TextureComponentType::Float, 4},
                         textureData, uncompressedData);
    DoFormatRenderingTest({wgpu::TextureFormat::BGRA8Unorm, 4, TextureComponentType::Float, 4},
                          uncompressedData, textureData);
}

// Test the R8Snorm format
TEST_P(TextureFormatTest, R8Snorm) {
    DoSnormTest<int8_t>({wgpu::TextureFormat::R8Snorm, 1, TextureComponentType::Float, 1});
}

// Test the RG8Snorm format
TEST_P(TextureFormatTest, RG8Snorm) {
    DoSnormTest<int8_t>({wgpu::TextureFormat::RG8Snorm, 2, TextureComponentType::Float, 2});
}

// Test the RGBA8Snorm format
TEST_P(TextureFormatTest, RGBA8Snorm) {
    DoSnormTest<int8_t>({wgpu::TextureFormat::RGBA8Snorm, 4, TextureComponentType::Float, 4});
}

// Test the R16Snorm format
TEST_P(TextureFormatTest, R16Snorm) {
    DoSnormTest<int16_t>({wgpu::TextureFormat::R16Snorm, 2, TextureComponentType::Float, 1});
}

// Test the RG16Snorm format
TEST_P(TextureFormatTest, RG16Snorm) {
    DoSnormTest<int16_t>({wgpu::TextureFormat::RG16Snorm, 4, TextureComponentType::Float, 2});
}

// Test the RGBA16Snorm format
TEST_P(TextureFormatTest, RGBA16Snorm) {
    DoSnormTest<int16_t>({wgpu::TextureFormat::RGBA16Snorm, 8, TextureComponentType::Float, 4});
}

// Test the R8Uint format
TEST_P(TextureFormatTest, R8Uint) {
    DoUintTest<uint8_t>({wgpu::TextureFormat::R8Uint, 1, TextureComponentType::Uint, 1});
}

// Test the RG8Uint format
TEST_P(TextureFormatTest, RG8Uint) {
    DoUintTest<uint8_t>({wgpu::TextureFormat::RG8Uint, 2, TextureComponentType::Uint, 2});
}

// Test the RGBA8Uint format
TEST_P(TextureFormatTest, RGBA8Uint) {
    DoUintTest<uint8_t>({wgpu::TextureFormat::RGBA8Uint, 4, TextureComponentType::Uint, 4});
}

// Test the R16Uint format
TEST_P(TextureFormatTest, R16Uint) {
    DoUintTest<uint16_t>({wgpu::TextureFormat::R16Uint, 2, TextureComponentType::Uint, 1});
}

// Test the RG16Uint format
TEST_P(TextureFormatTest, RG16Uint) {
    DoUintTest<uint16_t>({wgpu::TextureFormat::RG16Uint, 4, TextureComponentType::Uint, 2});
}

// Test the RGBA16Uint format
TEST_P(TextureFormatTest, RGBA16Uint) {
    DoUintTest<uint16_t>({wgpu::TextureFormat::RGBA16Uint, 8, TextureComponentType::Uint, 4});
}

// Test the R32Uint format
TEST_P(TextureFormatTest, R32Uint) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    DoUintTest<uint32_t>({wgpu::TextureFormat::R32Uint, 4, TextureComponentType::Uint, 1});
}

// Test the RG32Uint format
TEST_P(TextureFormatTest, RG32Uint) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    DoUintTest<uint32_t>({wgpu::TextureFormat::RG32Uint, 8, TextureComponentType::Uint, 2});
}

// Test the RGBA32Uint format
TEST_P(TextureFormatTest, RGBA32Uint) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    DoUintTest<uint32_t>({wgpu::TextureFormat::RGBA32Uint, 16, TextureComponentType::Uint, 4});
}

// Test the R8Sint format
TEST_P(TextureFormatTest, R8Sint) {
    DoSintTest<int8_t>({wgpu::TextureFormat::R8Sint, 1, TextureComponentType::Sint, 1});
}

// Test the RG8Sint format
TEST_P(TextureFormatTest, RG8Sint) {
    DoSintTest<int8_t>({wgpu::TextureFormat::RG8Sint, 2, TextureComponentType::Sint, 2});
}

// Test the RGBA8Sint format
TEST_P(TextureFormatTest, RGBA8Sint) {
    DoSintTest<int8_t>({wgpu::TextureFormat::RGBA8Sint, 4, TextureComponentType::Sint, 4});
}

// Test the R16Sint format
TEST_P(TextureFormatTest, R16Sint) {
    DoSintTest<int16_t>({wgpu::TextureFormat::R16Sint, 2, TextureComponentType::Sint, 1});
}

// Test the RG16Sint format
TEST_P(TextureFormatTest, RG16Sint) {
    DoSintTest<int16_t>({wgpu::TextureFormat::RG16Sint, 4, TextureComponentType::Sint, 2});
}

// Test the RGBA16Sint format
TEST_P(TextureFormatTest, RGBA16Sint) {
    DoSintTest<int16_t>({wgpu::TextureFormat::RGBA16Sint, 8, TextureComponentType::Sint, 4});
}

// Test the R32Sint format
TEST_P(TextureFormatTest, R32Sint) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    DoSintTest<int32_t>({wgpu::TextureFormat::R32Sint, 4, TextureComponentType::Sint, 1});
}

// Test the RG32Sint format
TEST_P(TextureFormatTest, RG32Sint) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    DoSintTest<int32_t>({wgpu::TextureFormat::RG32Sint, 8, TextureComponentType::Sint, 2});
}

// Test the RGBA32Sint format
TEST_P(TextureFormatTest, RGBA32Sint) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    DoSintTest<int32_t>({wgpu::TextureFormat::RGBA32Sint, 16, TextureComponentType::Sint, 4});
}

// Test the R32Float format
TEST_P(TextureFormatTest, R32Float) {
    DoFloat32Test({wgpu::TextureFormat::R32Float, 4, TextureComponentType::Float, 1});
}

// Test the RG32Float format
TEST_P(TextureFormatTest, RG32Float) {
    DoFloat32Test({wgpu::TextureFormat::RG32Float, 8, TextureComponentType::Float, 2});
}

// Test the RGBA32Float format
TEST_P(TextureFormatTest, RGBA32Float) {
    DoFloat32Test({wgpu::TextureFormat::RGBA32Float, 16, TextureComponentType::Float, 4});
}

// Test the R16Float format
TEST_P(TextureFormatTest, R16Float) {
    // TODO(https://crbug.com/swiftshader/147) Rendering INFINITY isn't handled correctly by
    // swiftshader
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsSwiftshader() || IsANGLE());

    DoFloat16Test({wgpu::TextureFormat::R16Float, 2, TextureComponentType::Float, 1});
}

// Test the RG16Float format
TEST_P(TextureFormatTest, RG16Float) {
    // TODO(https://crbug.com/swiftshader/147) Rendering INFINITY isn't handled correctly by
    // swiftshader
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsSwiftshader() || IsANGLE());

    DoFloat16Test({wgpu::TextureFormat::RG16Float, 4, TextureComponentType::Float, 2});
}

// Test the RGBA16Float format
TEST_P(TextureFormatTest, RGBA16Float) {
    // TODO(https://crbug.com/swiftshader/147) Rendering INFINITY isn't handled correctly by
    // swiftshader
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsSwiftshader() || IsANGLE());

    DoFloat16Test({wgpu::TextureFormat::RGBA16Float, 8, TextureComponentType::Float, 4});
}

// Test the RGBA8Unorm format
TEST_P(TextureFormatTest, RGBA8UnormSrgb) {
    uint8_t maxValue = std::numeric_limits<uint8_t>::max();
    std::vector<uint8_t> textureData = {0, 1, maxValue, 64, 35, 68, 152, 168};

    std::vector<float> uncompressedData;
    for (size_t i = 0; i < textureData.size(); i += 4) {
        uncompressedData.push_back(SRGBToLinear(textureData[i + 0] / static_cast<float>(maxValue)));
        uncompressedData.push_back(SRGBToLinear(textureData[i + 1] / static_cast<float>(maxValue)));
        uncompressedData.push_back(SRGBToLinear(textureData[i + 2] / static_cast<float>(maxValue)));
        // Alpha is linear for sRGB formats
        uncompressedData.push_back(textureData[i + 3] / static_cast<float>(maxValue));
    }

    DoFloatFormatSamplingTest(
        {wgpu::TextureFormat::RGBA8UnormSrgb, 4, TextureComponentType::Float, 4}, textureData,
        uncompressedData, 1.0e-3);
    DoFormatRenderingTest({wgpu::TextureFormat::RGBA8UnormSrgb, 4, TextureComponentType::Float, 4},
                          uncompressedData, textureData);
}

// Test the BGRA8UnormSrgb format
TEST_P(TextureFormatTest, BGRA8UnormSrgb) {
    // BGRA8UnormSrgb is unsupported in Compatibility mode
    DAWN_SUPPRESS_TEST_IF(IsCompatibilityMode());

    uint8_t maxValue = std::numeric_limits<uint8_t>::max();
    std::vector<uint8_t> textureData = {0, 1, maxValue, 64, 35, 68, 152, 168};

    std::vector<float> uncompressedData;
    for (size_t i = 0; i < textureData.size(); i += 4) {
        // Note that R and B are swapped
        uncompressedData.push_back(SRGBToLinear(textureData[i + 2] / static_cast<float>(maxValue)));
        uncompressedData.push_back(SRGBToLinear(textureData[i + 1] / static_cast<float>(maxValue)));
        uncompressedData.push_back(SRGBToLinear(textureData[i + 0] / static_cast<float>(maxValue)));
        // Alpha is linear for sRGB formats
        uncompressedData.push_back(textureData[i + 3] / static_cast<float>(maxValue));
    }

    DoFloatFormatSamplingTest(
        {wgpu::TextureFormat::BGRA8UnormSrgb, 4, TextureComponentType::Float, 4}, textureData,
        uncompressedData, 1.0e-3);
    DoFormatRenderingTest({wgpu::TextureFormat::BGRA8UnormSrgb, 4, TextureComponentType::Float, 4},
                          uncompressedData, textureData);
}

// Test the RGB10A2Unorm format
TEST_P(TextureFormatTest, RGB10A2Uint) {
    auto MakeRGB10A2 = [](uint32_t r, uint32_t g, uint32_t b, uint32_t a) -> uint32_t {
        DAWN_ASSERT((r & 0x3FF) == r);
        DAWN_ASSERT((g & 0x3FF) == g);
        DAWN_ASSERT((b & 0x3FF) == b);
        DAWN_ASSERT((a & 0x3) == a);
        return r | g << 10 | b << 20 | a << 30;
    };

    std::vector<uint32_t> textureData = {MakeRGB10A2(0, 0, 0, 0), MakeRGB10A2(1023, 1023, 1023, 1),
                                         MakeRGB10A2(243, 576, 765, 2), MakeRGB10A2(0, 0, 0, 3)};
    // clang-format off
    std::vector<uint32_t> uncompressedData = {
       0, 0, 0, 0,
       1023, 1023, 1023, 1,
       243, 576, 765, 2,
       0, 0, 0, 3
    };
    // clang-format on

    DoFormatSamplingTest({wgpu::TextureFormat::RGB10A2Uint, 4, TextureComponentType::Uint, 4},
                         textureData, uncompressedData);
    DoFormatRenderingTest({wgpu::TextureFormat::RGB10A2Uint, 4, TextureComponentType::Uint, 4},
                          uncompressedData, textureData);
}

// Test the RGB10A2Unorm format
TEST_P(TextureFormatTest, RGB10A2Unorm) {
    auto MakeRGB10A2 = [](uint32_t r, uint32_t g, uint32_t b, uint32_t a) -> uint32_t {
        DAWN_ASSERT((r & 0x3FF) == r);
        DAWN_ASSERT((g & 0x3FF) == g);
        DAWN_ASSERT((b & 0x3FF) == b);
        DAWN_ASSERT((a & 0x3) == a);
        return r | g << 10 | b << 20 | a << 30;
    };

    std::vector<uint32_t> textureData = {MakeRGB10A2(0, 0, 0, 0), MakeRGB10A2(1023, 1023, 1023, 1),
                                         MakeRGB10A2(243, 576, 765, 2), MakeRGB10A2(0, 0, 0, 3)};
    // clang-format off
    std::vector<float> uncompressedData = {
       0.0f, 0.0f, 0.0f, 0.0f,
       1.0f, 1.0f, 1.0f, 1 / 3.0f,
        243 / 1023.0f, 576 / 1023.0f, 765 / 1023.0f, 2 / 3.0f,
       0.0f, 0.0f, 0.0f, 1.0f
    };
    // clang-format on

    DoFloatFormatSamplingTest(
        {wgpu::TextureFormat::RGB10A2Unorm, 4, TextureComponentType::Float, 4}, textureData,
        uncompressedData, 1.0e-5);
    DoFormatRenderingTest({wgpu::TextureFormat::RGB10A2Unorm, 4, TextureComponentType::Float, 4},
                          uncompressedData, textureData);
}

// Test the RG11B10Ufloat format
TEST_P(TextureFormatTest, RG11B10Ufloat) {
    // TODO(crbug.com/388318201): sampling test also requires format to be color-renderable
    DAWN_SUPPRESS_TEST_IF(!IsRG11B10UfloatRenderableSupported());
    // TODO(crbug.com/388318201): expected: 0xf87e0000, actual: 0xfffff800
    DAWN_SUPPRESS_TEST_IF(IsD3D11());

    constexpr uint32_t kFloat11Zero = 0;
    constexpr uint32_t kFloat11Infinity = 0x7C0;
    constexpr uint32_t kFloat11Nan = 0x7C1;
    constexpr uint32_t kFloat11One = 0x3C0;

    constexpr uint32_t kFloat10Zero = 0;
    constexpr uint32_t kFloat10Infinity = 0x3E0;
    constexpr uint32_t kFloat10Nan = 0x3E1;
    constexpr uint32_t kFloat10One = 0x1E0;

    auto MakeRG11B10 = [](uint32_t r, uint32_t g, uint32_t b) {
        DAWN_ASSERT((r & 0x7FF) == r);
        DAWN_ASSERT((g & 0x7FF) == g);
        DAWN_ASSERT((b & 0x3FF) == b);
        return r | g << 11 | b << 22;
    };

    // Test each of (0, 1, INFINITY, NaN) for each component but never two with the same value at a
    // time.
    std::vector<uint32_t> textureData = {
        MakeRG11B10(kFloat11Zero, kFloat11Infinity, kFloat10Nan),
        MakeRG11B10(kFloat11Infinity, kFloat11Nan, kFloat10One),
        MakeRG11B10(kFloat11Nan, kFloat11One, kFloat10Zero),
        MakeRG11B10(kFloat11One, kFloat11Zero, kFloat10Infinity),
    };

    // This is one of the only 3-channel formats, so we don't have specific testing for them. Alpha
    // should always be sampled as 1
    // clang-format off
    std::vector<float> uncompressedData = {
        0.0f,     INFINITY, NAN,      1.0f,
        INFINITY, NAN,      1.0f,     1.0f,
        NAN,      1.0f,     0.0f,     1.0f,
        1.0f,     0.0f,     INFINITY, 1.0f
    };
    // clang-format on

    DoFloatFormatSamplingTest(
        {wgpu::TextureFormat::RG11B10Ufloat, 4, TextureComponentType::Float, 4}, textureData,
        uncompressedData);

    // TODO(https://crbug.com/swiftshader/147) Rendering INFINITY and NaN isn't handled
    // correctly by swiftshader
    if ((IsVulkan() && IsSwiftshader()) || IsANGLE()) {
        WarningLog() << "Skip Rendering test because Swiftshader doesn't render INFINITY "
                        "and NaN correctly for RG11B10Ufloat texture format.";
    } else {
        DoFormatRenderingTest(
            {wgpu::TextureFormat::RG11B10Ufloat, 4, TextureComponentType::Float, 4},
            uncompressedData, textureData, new ExpectRG11B10Ufloat(textureData));
    }
}

// Test the RGB9E5Ufloat format
TEST_P(TextureFormatTest, RGB9E5Ufloat) {
    // RGB9E5 is different from other floating point formats because the mantissa doesn't index in
    // the window defined by the exponent but is instead treated as a pure multiplier. There is
    // also no Infinity or NaN. The OpenGL 4.6 spec has the best explanation I've found in section
    // 8.25 "Shared Exponent Texture Color Conversion":
    //
    //   red = reduint * 2^(expuint - B - N) = reduint * 2^(expuint - 24)
    //
    // Where reduint and expuint are the integer values when considering the E5 as a 5bit uint, and
    // the r9 as a 9bit uint. B the number of bits of the mantissa (9), and N the offset for the
    // exponent (15).

    float smallestExponent = std::pow(2.0f, -24.0f);
    float largestExponent = std::pow(2.0f, float{31 - 24});

    auto MakeRGB9E5 = [](uint32_t r, uint32_t g, uint32_t b, uint32_t e) {
        DAWN_ASSERT((r & 0x1FF) == r);
        DAWN_ASSERT((g & 0x1FF) == g);
        DAWN_ASSERT((b & 0x1FF) == b);
        DAWN_ASSERT((e & 0x1F) == e);
        return r | g << 9 | b << 18 | e << 27;
    };

    // Test the smallest largest, and "1" exponents
    std::vector<uint32_t> textureData = {
        MakeRGB9E5(0, 1, 2, 0b00000),
        MakeRGB9E5(2, 1, 0, 0b11111),
        MakeRGB9E5(0, 1, 2, 0b11000),
    };

    // This is one of the only 3-channel formats, so we don't have specific testing for them. Alpha
    // should always be sampled as 1
    // clang-format off
    std::vector<float> uncompressedData = {
        0.0f, smallestExponent, 2.0f * smallestExponent, 1.0f,
        2.0f * largestExponent, largestExponent, 0.0f, 1.0f,
        0.0f, 1.0f, 2.0f, 1.0f,
    };
    // clang-format on

    DoFloatFormatSamplingTest(
        {wgpu::TextureFormat::RGB9E5Ufloat, 4, TextureComponentType::Float, 4}, textureData,
        uncompressedData);
    // This format is not renderable.
}

DAWN_INSTANTIATE_TEST(TextureFormatTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
