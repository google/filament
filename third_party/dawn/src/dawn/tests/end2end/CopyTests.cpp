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

#include <algorithm>
#include <array>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

// For MinimumBufferSpec bytesPerRow and rowsPerImage, compute a default from the copy extent.
constexpr uint32_t kStrideComputeDefault = 0xFFFF'FFFEul;

constexpr wgpu::TextureFormat kDefaultFormat = wgpu::TextureFormat::RGBA8Unorm;

bool IsSnorm(wgpu::TextureFormat format) {
    return format == wgpu::TextureFormat::RGBA8Snorm || format == wgpu::TextureFormat::RG8Snorm ||
           format == wgpu::TextureFormat::R8Snorm;
}

template <typename T, size_t NumComponents>
struct Color {
    static constexpr size_t kNumComponents = NumComponents;
    static constexpr size_t kDataSize = sizeof(T) * NumComponents;
    using ComponentRepresentation = T;

    // Get representation of one component.
    T GetCompRep(size_t idx) const { return components[idx]; }

    T components[NumComponents] = {};
};

template <size_t NumComponents>
struct ColorF16 : public Color<uint16_t, NumComponents> {
    using ComponentRepresentation = float;

    // Get representation of one component.
    float GetCompRep(size_t idx) const { return Float16ToFloat32(this->components[idx]); }
};

struct ColorRGB10A2 {
    static constexpr size_t kNumComponents = 4;
    static constexpr size_t kDataSize = sizeof(uint32_t);
    using ComponentRepresentation = uint32_t;

    // Get representation of one component.
    uint32_t GetCompRep(size_t idx) const { return (mPackedColor >> (10 * idx)) & 0x3ff; }

    uint32_t mPackedColor = 0;
};

static_assert(sizeof(Color<uint8_t, 1>) == 1, "Unexpected padding");
static_assert(sizeof(Color<uint8_t, 2>) == 2, "Unexpected padding");
static_assert(sizeof(Color<uint16_t, 1>) == 2, "Unexpected padding");
static_assert(sizeof(ColorRGB10A2) == 4, "Unexpected padding");

template <typename ColorType>
class ColorExpectation : public detail::CustomTextureExpectation {
  public:
    static constexpr size_t kNumComponents = ColorType::kNumComponents;
    using CompRepType = ColorType::ComponentRepresentation;

    ColorExpectation(const ColorType* expected, size_t count, CompRepType tolerance)
        : mTolerance(tolerance) {
        mExpected.assign(expected, expected + count);
    }

    uint32_t DataSize() override { return ColorType::kDataSize; }

    testing::AssertionResult Check(const void* data, size_t size) override {
        DAWN_ASSERT(size == sizeof(ColorType) * mExpected.size());
        const ColorType* actual = static_cast<const ColorType*>(data);

        for (size_t i = 0; i < mExpected.size(); ++i) {
            if (!AreEqual(mExpected[i], actual[i])) {
                return testing::AssertionFailure()
                       << "Expected data[" << i << "] to be " << ToString(mExpected[i])
                       << ", actual " << ToString(actual[i]) << "\n";
            }
        }
        return testing::AssertionSuccess();
    }

  private:
    static CompRepType GetComponent(const ColorType& color, size_t idx) {
        DAWN_ASSERT(idx <= kNumComponents);
        return color.GetCompRep(idx);
    }

    static CompRepType Diff(CompRepType lhs, CompRepType rhs) {
        if constexpr (std::is_integral_v<CompRepType>) {
            return std::abs(static_cast<int64_t>(lhs) - static_cast<int64_t>(rhs));
        } else {
            return std::abs(lhs - rhs);
        }
    }

    static std::ostream& Print(std::ostream& stream, CompRepType component) {
        if constexpr (std::is_same_v<CompRepType, uint8_t>) {
            return stream << static_cast<int>(component);
        } else {
            return stream << component;
        }
    }

    static std::string ToString(const ColorType& color) {
        std::ostringstream ss;
        for (size_t i = 0; i < kNumComponents; ++i) {
            Print(ss, GetComponent(color, i));

            if (i < kNumComponents - 1) {
                ss << " ";
            }
        }

        return ss.str();
    }

    bool AreEqual(const ColorType& lhs, const ColorType& rhs) const {
        for (size_t i = 0; i < kNumComponents; ++i) {
            if (Diff(GetComponent(lhs, i), GetComponent(rhs, i)) > mTolerance) {
                return false;
            }
        }
        return true;
    }

    std::vector<ColorType> mExpected;
    const CompRepType mTolerance;
};

class CopyTests {
  protected:
    struct TextureSpec {
        wgpu::TextureFormat format = kDefaultFormat;
        wgpu::Origin3D copyOrigin = {0, 0, 0};
        wgpu::Extent3D textureSize;
        uint32_t copyLevel = 0;
        uint32_t levelCount = 1;
    };

    struct BufferSpec {
        uint64_t size;
        uint64_t offset;
        uint32_t bytesPerRow;
        uint32_t rowsPerImage;
    };

    static std::vector<uint8_t> GetExpectedTextureData(wgpu::TextureFormat format,
                                                       const utils::TextureDataCopyLayout& layout) {
        switch (format) {
            case wgpu::TextureFormat::R16Float:
            case wgpu::TextureFormat::RG16Float:
            case wgpu::TextureFormat::RGBA16Float:
                return GetExpectedTextureData16Float(layout);
            case wgpu::TextureFormat::RGB9E5Ufloat:
                return GetExpectedTextureDataRGB9E5Ufloat(layout);
            case wgpu::TextureFormat::RG11B10Ufloat:
                return GetExpectedTextureDataRG11B10Ufloat(layout);
            default:
                return GetExpectedTextureDataGeneral(layout);
        }
    }

    static std::vector<uint8_t> GetExpectedTextureDataGeneral(
        const utils::TextureDataCopyLayout& layout) {
        uint32_t bytesPerTexelBlock = layout.bytesPerRow / layout.texelBlocksPerRow;
        std::vector<uint8_t> textureData(layout.byteLength);
        for (uint32_t layer = 0; layer < layout.mipSize.depthOrArrayLayers; ++layer) {
            const uint32_t byteOffsetPerSlice = layout.bytesPerImage * layer;
            for (uint32_t y = 0; y < layout.mipSize.height; ++y) {
                for (uint32_t x = 0; x < layout.mipSize.width * bytesPerTexelBlock; ++x) {
                    uint32_t i = x + y * layout.bytesPerRow;
                    uint8_t v = static_cast<uint8_t>((x + 1 + (layer + 1) * y) % 256);
                    if (v == 0x80u) {
                        // Some texture copy is implemented via textureLoad from compute pass.
                        // For 8 bit Snorm texture when read in shader, 0x80 (-128) becomes 0x81
                        // (-127) As Snorm value range mapped to [-1, 1]. To avoid failure in buffer
                        // comparison stage, simply avoid writing 0x81 instead of 0x80 to the
                        // texture.
                        v = 0x81u;
                    }
                    textureData[byteOffsetPerSlice + i] = v;
                }
            }
        }
        return textureData;
    }

    // Special function to generate test data for *16Float to workaround nonunique encoding
    // issue.
    static std::vector<uint8_t> GetExpectedTextureData16Float(
        const utils::TextureDataCopyLayout& layout) {
        // These are some known 16 bit float values that always unpack and pack to the same bytes.
        // Pick test data from these values to provide some level of test coverage for *16Float.
        constexpr uint8_t goodBytes[] = {
            0x30, 0x00, 0x49, 0x00, 0x56, 0x40, 0x20, 0x00, 0x37, 0x4C, 0x42, 0x00, 0x3F, 0x6C,
        };
        constexpr uint32_t formatByteSize = 2;
        constexpr uint32_t numGoodValues = sizeof(goodBytes) / sizeof(uint8_t) / formatByteSize;

        uint32_t bytesPerTexelBlock = layout.bytesPerRow / layout.texelBlocksPerRow;
        std::vector<uint8_t> textureData(layout.byteLength);
        for (uint32_t layer = 0; layer < layout.mipSize.depthOrArrayLayers; ++layer) {
            const uint32_t byteOffsetPerSlice = layout.bytesPerImage * layer;
            for (uint32_t y = 0; y < layout.mipSize.height; ++y) {
                for (uint32_t x = 0; x < layout.mipSize.width * bytesPerTexelBlock; ++x) {
                    uint32_t i = x + y * layout.bytesPerRow;

                    uint32_t pixelId = (x + 1 + (layer + 1) * y);
                    uint8_t v = goodBytes[pixelId % numGoodValues + pixelId % formatByteSize];

                    textureData[byteOffsetPerSlice + i] = v;
                }
            }
        }
        return textureData;
    }

    // Special function to generate test data for RGB9E5Ufloat to workaround nonunique encoding
    // issue.
    static std::vector<uint8_t> GetExpectedTextureDataRGB9E5Ufloat(
        const utils::TextureDataCopyLayout& layout) {
        // These are some known 4-byte RGB9E5Ufloat values that always unpack and pack to the same
        // bytes. Pick test data from these values to provide some level of test coverage for
        // RGB9E5Ufloat.
        constexpr uint8_t goodBytes[] = {
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
            0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
            0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28};
        constexpr uint32_t formatByteSize = 4;
        constexpr uint32_t numGoodValues = sizeof(goodBytes) / sizeof(uint8_t) / formatByteSize;
        uint32_t bytesPerTexelBlock = layout.bytesPerRow / layout.texelBlocksPerRow;
        std::vector<uint8_t> textureData(layout.byteLength);
        for (uint32_t layer = 0; layer < layout.mipSize.depthOrArrayLayers; ++layer) {
            const uint32_t byteOffsetPerSlice = layout.bytesPerImage * layer;
            for (uint32_t y = 0; y < layout.mipSize.height; ++y) {
                for (uint32_t x = 0; x < layout.mipSize.width; ++x) {
                    uint32_t o =
                        x * bytesPerTexelBlock + y * layout.bytesPerRow + byteOffsetPerSlice;
                    uint32_t idx = formatByteSize * ((x + 1 + (layer + 1) * y) % numGoodValues);
                    textureData[o + 0] = goodBytes[idx + 0];
                    textureData[o + 1] = goodBytes[idx + 1];
                    textureData[o + 2] = goodBytes[idx + 2];
                    textureData[o + 3] = goodBytes[idx + 3];
                }
            }
        }
        return textureData;
    }

    // Special function to generate test data for RG11B10Ufloat to workaround nonunique encoding
    // issue.
    static std::vector<uint8_t> GetExpectedTextureDataRG11B10Ufloat(
        const utils::TextureDataCopyLayout& layout) {
        // These are some known 4-byte RG11B10Ufloat values that always unpack and pack to the same
        // bytes. Pick test data from these values to provide some level of test coverage for
        // RG11B10Ufloat.
        constexpr uint8_t goodBytes[] = {
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
            0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
            0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28};
        constexpr uint32_t formatByteSize = 4;
        constexpr uint32_t numGoodValues = sizeof(goodBytes) / sizeof(uint8_t) / formatByteSize;
        uint32_t bytesPerTexelBlock = layout.bytesPerRow / layout.texelBlocksPerRow;
        std::vector<uint8_t> textureData(layout.byteLength);
        for (uint32_t layer = 0; layer < layout.mipSize.depthOrArrayLayers; ++layer) {
            const uint32_t byteOffsetPerSlice = layout.bytesPerImage * layer;
            for (uint32_t y = 0; y < layout.mipSize.height; ++y) {
                for (uint32_t x = 0; x < layout.mipSize.width; ++x) {
                    uint32_t o =
                        x * bytesPerTexelBlock + y * layout.bytesPerRow + byteOffsetPerSlice;
                    uint32_t idx = formatByteSize * ((x + 1 + (layer + 1) * y) % numGoodValues);
                    textureData[o + 0] = goodBytes[idx + 0];
                    textureData[o + 1] = goodBytes[idx + 1];
                    textureData[o + 2] = goodBytes[idx + 2];
                    textureData[o + 3] = goodBytes[idx + 3];
                }
            }
        }
        return textureData;
    }

    static BufferSpec MinimumBufferSpec(
        uint32_t width,
        uint32_t height,
        uint32_t depth = 1,
        wgpu::TextureFormat format = kDefaultFormat,
        uint32_t textureBytesPerRowAlignment = kTextureBytesPerRowAlignment) {
        return MinimumBufferSpec({width, height, depth}, kStrideComputeDefault,
                                 depth == 1 ? wgpu::kCopyStrideUndefined : kStrideComputeDefault,
                                 format, textureBytesPerRowAlignment);
    }

    static BufferSpec MinimumBufferSpec(
        wgpu::Extent3D copyExtent,
        uint32_t overrideBytesPerRow = kStrideComputeDefault,
        uint32_t overrideRowsPerImage = kStrideComputeDefault,
        wgpu::TextureFormat format = kDefaultFormat,
        uint32_t textureBytesPerRowAlignment = kTextureBytesPerRowAlignment) {
        uint32_t bytesPerRow =
            utils::GetMinimumBytesPerRow(format, copyExtent.width, textureBytesPerRowAlignment);
        if (overrideBytesPerRow != kStrideComputeDefault) {
            bytesPerRow = overrideBytesPerRow;
        }
        uint32_t rowsPerImage = copyExtent.height;
        if (overrideRowsPerImage != kStrideComputeDefault) {
            rowsPerImage = overrideRowsPerImage;
        }

        // Align with 4 byte, not actually "minimum" but is needed when check buffer content.
        uint32_t totalDataSize =
            Align(utils::RequiredBytesInCopy(bytesPerRow, rowsPerImage, copyExtent, format), 4);
        return {totalDataSize, 0, bytesPerRow, rowsPerImage};
    }
    static void CopyTextureData(uint32_t bytesPerTexelBlock,
                                const void* srcData,
                                uint32_t widthInBlocks,
                                uint32_t heightInBlocks,
                                uint32_t depthInBlocks,
                                uint32_t srcBytesPerRow,
                                uint32_t srcRowsPerImage,
                                void* dstData,
                                uint32_t dstBytesPerRow,
                                uint32_t dstRowsPerImage) {
        for (unsigned int z = 0; z < depthInBlocks; ++z) {
            uint32_t srcDepthOffset = z * srcBytesPerRow * srcRowsPerImage;
            uint32_t dstDepthOffset = z * dstBytesPerRow * dstRowsPerImage;
            for (unsigned int y = 0; y < heightInBlocks; ++y) {
                memcpy(static_cast<uint8_t*>(dstData) + dstDepthOffset + y * dstBytesPerRow,
                       static_cast<const uint8_t*>(srcData) + srcDepthOffset + y * srcBytesPerRow,
                       widthInBlocks * bytesPerTexelBlock);
            }
        }
    }
};

namespace {
using TextureFormat = wgpu::TextureFormat;
DAWN_TEST_PARAM_STRUCT(CopyTextureFormatParams, TextureFormat);

class CopyTests_WithFormatParam : public CopyTests,
                                  public DawnTestWithParams<CopyTextureFormatParams> {
  protected:
    struct TextureSpec : CopyTests::TextureSpec {
        TextureSpec() { format = GetParam().mTextureFormat; }
    };

    void SetUp() override {
        DawnTestWithParams<CopyTextureFormatParams>::SetUp();
        switch (GetParam().mTextureFormat) {
            case wgpu::TextureFormat::R16Unorm:
            case wgpu::TextureFormat::RG16Unorm:
            case wgpu::TextureFormat::RGBA16Unorm:
                DAWN_TEST_UNSUPPORTED_IF(
                    !device.HasFeature(wgpu::FeatureName::Unorm16TextureFormats));
                break;
            default:
                break;
        }
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};

        switch (GetParam().mTextureFormat) {
            case wgpu::TextureFormat::R16Unorm:
            case wgpu::TextureFormat::RG16Unorm:
            case wgpu::TextureFormat::RGBA16Unorm:
                if (SupportsFeatures({wgpu::FeatureName::Unorm16TextureFormats})) {
                    requiredFeatures.push_back(wgpu::FeatureName::Unorm16TextureFormats);
                }
                break;
            default:
                break;
        }

        if (SupportsFeatures({wgpu::FeatureName::DawnTexelCopyBufferRowAlignment})) {
            requiredFeatures.push_back(wgpu::FeatureName::DawnTexelCopyBufferRowAlignment);
        }
        return requiredFeatures;
    }

    uint32_t GetTextureBytesPerRowAlignment() const {
        if (!device.HasFeature(wgpu::FeatureName::DawnTexelCopyBufferRowAlignment)) {
            return kTextureBytesPerRowAlignment;
        }
        wgpu::Limits limits{};
        wgpu::DawnTexelCopyBufferRowAlignmentLimits alignmentLimits{};
        limits.nextInChain = &alignmentLimits;
        device.GetLimits(&limits);
        return alignmentLimits.minTexelCopyBufferRowAlignment;
    }
    BufferSpec MinimumBufferSpec(uint32_t width, uint32_t height, uint32_t depth = 1) {
        return CopyTests::MinimumBufferSpec(width, height, depth, GetParam().mTextureFormat,
                                            GetTextureBytesPerRowAlignment());
    }
    BufferSpec MinimumBufferSpec(wgpu::Extent3D copyExtent,
                                 uint32_t overrideBytesPerRow = kStrideComputeDefault,
                                 uint32_t overrideRowsPerImage = kStrideComputeDefault) {
        return CopyTests::MinimumBufferSpec(copyExtent, overrideBytesPerRow, overrideRowsPerImage,
                                            GetParam().mTextureFormat,
                                            GetTextureBytesPerRowAlignment());
    }
};

}  // namespace

class CopyTests_T2B : public CopyTests_WithFormatParam {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures =
            CopyTests_WithFormatParam::GetRequiredFeatures();
        if (SupportsFeatures({wgpu::FeatureName::FlexibleTextureViews})) {
            requiredFeatures.push_back(wgpu::FeatureName::FlexibleTextureViews);
        }
        return requiredFeatures;
    }

    void SetUp() override {
        CopyTests_WithFormatParam::SetUp();

        auto format = GetParam().mTextureFormat;

        // TODO(dawn:2129): Fail for Win ANGLE D3D11
        DAWN_SUPPRESS_TEST_IF((format == wgpu::TextureFormat::RGB9E5Ufloat) && IsANGLED3D11() &&
                              IsWindows());

        // TODO(crbug.com/dawn/2294): diagnose BGRA T2B failures on Pixel 4 OpenGLES
        DAWN_SUPPRESS_TEST_IF(format == wgpu::TextureFormat::BGRA8Unorm && IsOpenGLES() &&
                              IsAndroid() && IsQualcomm());

        // TODO(dawn:1913): Many float formats tests failing for Metal backend on Mac Intel.
        DAWN_SUPPRESS_TEST_IF((format == wgpu::TextureFormat::R32Float ||
                               format == wgpu::TextureFormat::RG32Float ||
                               format == wgpu::TextureFormat::RGBA32Float ||
                               format == wgpu::TextureFormat::RGBA16Float ||
                               format == wgpu::TextureFormat::RG11B10Ufloat) &&
                              IsMacOS() && IsIntel() && IsMetal());

        // TODO(dawn:1935): Many 16 float formats tests failing for D3D11 and OpenGLES backends on
        // Intel Gen12.
        DAWN_SUPPRESS_TEST_IF((format == wgpu::TextureFormat::R16Float ||
                               format == wgpu::TextureFormat::RGBA16Float ||
                               format == wgpu::TextureFormat::RG11B10Ufloat) &&
                              (IsD3D11() || IsOpenGLES()) && IsIntelGen12());
    }

    void DoTest(
        const TextureSpec& textureSpec,
        const BufferSpec& bufferSpec,
        const wgpu::Extent3D& copySize,
        wgpu::TextureDimension dimension = wgpu::TextureDimension::e2D,
        wgpu::TextureViewDimension bindingViewDimension = wgpu::TextureViewDimension::Undefined,
        bool useMappableBuffer = false) {
        const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(textureSpec.format);
        // Create a texture that is `width` x `height` with (`level` + 1) mip levels.
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = dimension;
        descriptor.size = textureSpec.textureSize;
        descriptor.sampleCount = 1;
        descriptor.format = textureSpec.format;
        descriptor.mipLevelCount = textureSpec.levelCount;
        descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;

        // Test cube texture copy for compat.
        wgpu::TextureBindingViewDimensionDescriptor textureBindingViewDimensionDesc;
        if (IsCompatibilityMode() &&
            bindingViewDimension != wgpu::TextureViewDimension::Undefined) {
            textureBindingViewDimensionDesc.textureBindingViewDimension = bindingViewDimension;
            descriptor.nextInChain = &textureBindingViewDimensionDesc;
        }

        wgpu::Texture texture = device.CreateTexture(&descriptor);

        // Layout for initial data upload to texture.
        // Some parts of this result are also reused later.
        const utils::TextureDataCopyLayout copyLayout =
            utils::GetTextureDataCopyLayoutForTextureAtLevel(
                textureSpec.format, textureSpec.textureSize, textureSpec.copyLevel, dimension);

        const std::vector<uint8_t> textureArrayData =
            GetExpectedTextureData(textureSpec.format, copyLayout);
        {
            wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
                utils::CreateTexelCopyTextureInfo(texture, textureSpec.copyLevel, {0, 0, 0});
            wgpu::TexelCopyBufferLayout texelCopyBufferLayout = utils::CreateTexelCopyBufferLayout(
                0, copyLayout.bytesPerRow, copyLayout.rowsPerImage);
            queue.WriteTexture(&texelCopyTextureInfo, textureArrayData.data(),
                               copyLayout.byteLength, &texelCopyBufferLayout, &copyLayout.mipSize);
        }

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        // Create a buffer of `size` and populate it with empty data (0,0,0,0) Note:
        // Prepopulating the buffer with empty data ensures that there is not random data in the
        // expectation and helps ensure that the padding due to the bytes per row is not modified
        // by the copy.
        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = bufferSpec.size;
        bufferDesc.usage = wgpu::BufferUsage::CopyDst;
        if (useMappableBuffer) {
            bufferDesc.usage |= wgpu::BufferUsage::MapRead;
        } else {
            bufferDesc.usage |= wgpu::BufferUsage::CopySrc;
        }
        wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

        {
            wgpu::TexelCopyTextureInfo texelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
                texture, textureSpec.copyLevel, textureSpec.copyOrigin);
            wgpu::TexelCopyBufferInfo texelCopyBufferInfo = utils::CreateTexelCopyBufferInfo(
                buffer, bufferSpec.offset, bufferSpec.bytesPerRow, bufferSpec.rowsPerImage);
            encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &copySize);
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        uint64_t bufferOffset = bufferSpec.offset;

        uint32_t copyLayer = copySize.depthOrArrayLayers;
        uint32_t copyDepth = 1;
        if (dimension == wgpu::TextureDimension::e3D) {
            copyLayer = 1;
            copyDepth = copySize.depthOrArrayLayers;
        }

        const wgpu::Extent3D copySizePerLayer = {copySize.width, copySize.height, copyDepth};
        const uint32_t maxArrayLayer = textureSpec.copyOrigin.z + copyLayer;
        std::vector<uint8_t> expected(utils::RequiredBytesInCopy(
            bufferSpec.bytesPerRow, bufferSpec.rowsPerImage, copySizePerLayer, textureSpec.format));

        if (useMappableBuffer) {
            bool done = false;
            buffer.MapAsync(wgpu::MapMode::Read, 0, buffer.GetSize(),
                            wgpu::CallbackMode::AllowProcessEvents,
                            [&](wgpu::MapAsyncStatus status, wgpu::StringView) {
                                ASSERT_EQ(wgpu::MapAsyncStatus::Success, status);
                                done = true;
                            });
            while (!done) {
                WaitABit();
            }
        }

        for (uint32_t layer = textureSpec.copyOrigin.z; layer < maxArrayLayer; ++layer) {
            // Copy the data used to create the upload buffer in the specified copy region to have
            // the same format as the expected buffer data.
            std::fill(expected.begin(), expected.end(), 0x00);

            const uint32_t texelIndexOffset = copyLayout.bytesPerImage * layer;
            const uint32_t expectedTexelArrayDataStartIndex =
                texelIndexOffset +
                bytesPerTexel * (textureSpec.copyOrigin.x +
                                 textureSpec.copyOrigin.y * copyLayout.texelBlocksPerRow);

            CopyTextureData(bytesPerTexel,
                            textureArrayData.data() + expectedTexelArrayDataStartIndex,
                            copySize.width, copySize.height, copyDepth, copyLayout.bytesPerRow,
                            copyLayout.rowsPerImage, expected.data(), bufferSpec.bytesPerRow,
                            bufferSpec.rowsPerImage);

            std::ostringstream errorMsgSs;
            errorMsgSs << "Texture to Buffer copy failed copying region [("
                       << textureSpec.copyOrigin.x << ", " << textureSpec.copyOrigin.y << ", "
                       << textureSpec.copyOrigin.z << "), ("
                       << textureSpec.copyOrigin.x + copySize.width << ", "
                       << textureSpec.copyOrigin.y + copySize.height << ", "
                       << textureSpec.copyOrigin.z + copySize.depthOrArrayLayers << ")) from "
                       << textureSpec.textureSize.width << " x " << textureSpec.textureSize.height
                       << " texture at mip level " << textureSpec.copyLevel << " layer " << layer
                       << " to " << bufferSpec.size << "-byte buffer with offset " << bufferOffset
                       << " and bytes per row " << bufferSpec.bytesPerRow << "\n";

            if (useMappableBuffer) {
                const auto* mappedPtr = static_cast<const uint8_t*>(buffer.GetConstMappedRange());
                for (size_t i = 0; i < expected.size(); ++i) {
                    if (mappedPtr[bufferOffset + i] != expected[i]) {
                        EXPECT_EQ(mappedPtr[bufferOffset + i], expected[i])
                            << "with i=" << i << "\n"
                            << errorMsgSs.str();
                        break;
                    }
                }
            } else {
                EXPECT_BUFFER_U8_RANGE_EQ(reinterpret_cast<const uint8_t*>(expected.data()), buffer,
                                          bufferOffset, expected.size())
                    << errorMsgSs.str();
            }

            bufferOffset += bufferSpec.bytesPerRow * bufferSpec.rowsPerImage;
        }

        if (useMappableBuffer) {
            buffer.Unmap();
        }
    }
};

class CopyTests_B2T : public CopyTests_WithFormatParam {
  protected:
    void DoTest(const TextureSpec& textureSpec,
                const BufferSpec& bufferSpec,
                const wgpu::Extent3D& copySize,
                wgpu::TextureDimension dimension = wgpu::TextureDimension::e2D) {
        switch (textureSpec.format) {
            case wgpu::TextureFormat::R8Unorm:
                DoTestImpl<Color<uint8_t, 1>>(textureSpec, bufferSpec, copySize, dimension,
                                              /*tolerance=*/1);
                break;
            case wgpu::TextureFormat::RG8Unorm:
                DoTestImpl<Color<uint8_t, 2>>(textureSpec, bufferSpec, copySize, dimension,
                                              /*tolerance=*/1);
                break;
            case wgpu::TextureFormat::RGBA8Unorm:
            case wgpu::TextureFormat::BGRA8Unorm:
                DoTestImpl<Color<uint8_t, 4>>(textureSpec, bufferSpec, copySize, dimension,
                                              /*tolerance=*/1);
                break;
            case wgpu::TextureFormat::RGB10A2Unorm:
                DoTestImpl<ColorRGB10A2>(textureSpec, bufferSpec, copySize, dimension,
                                         /*tolerance=*/1);
                break;
            case wgpu::TextureFormat::R16Float:
                DoTestImpl<ColorF16<1>>(textureSpec, bufferSpec, copySize, dimension,
                                        /*tolerance=*/0.001f);
                break;
            case wgpu::TextureFormat::R16Unorm:
                DoTestImpl<Color<uint16_t, 1>>(textureSpec, bufferSpec, copySize, dimension,
                                               /*tolerance=*/1);
                break;
            case wgpu::TextureFormat::RG16Float:
                DoTestImpl<ColorF16<2>>(textureSpec, bufferSpec, copySize, dimension,
                                        /*tolerance=*/0.001f);
                break;
            case wgpu::TextureFormat::RG16Unorm:
                DoTestImpl<Color<uint16_t, 2>>(textureSpec, bufferSpec, copySize, dimension,
                                               /*tolerance=*/1);
                break;
            case wgpu::TextureFormat::RGBA16Float:
                DoTestImpl<ColorF16<4>>(textureSpec, bufferSpec, copySize, dimension,
                                        /*tolerance=*/0.001f);
                break;
            case wgpu::TextureFormat::RGBA16Unorm:
                DoTestImpl<Color<uint16_t, 4>>(textureSpec, bufferSpec, copySize, dimension,
                                               /*tolerance=*/1);
                break;
            case wgpu::TextureFormat::R32Float:
                DoTestImpl<Color<float, 1>>(textureSpec, bufferSpec, copySize, dimension,
                                            /*tolerance=*/0.0001f);
                break;
            case wgpu::TextureFormat::RG32Float:
                DoTestImpl<Color<float, 2>>(textureSpec, bufferSpec, copySize, dimension,
                                            /*tolerance=*/0.0001f);
                break;
            case wgpu::TextureFormat::RGBA32Float:
                DoTestImpl<Color<float, 4>>(textureSpec, bufferSpec, copySize, dimension,
                                            /*tolerance=*/0.0001f);
                break;
            default:
                DAWN_UNREACHABLE();
        }
    }

    template <class PixelType>
    void DoTestImpl(const TextureSpec& textureSpec,
                    const BufferSpec& bufferSpec,
                    const wgpu::Extent3D& copySize,
                    wgpu::TextureDimension dimension,
                    PixelType::ComponentRepresentation tolerance = {}) {
        const uint32_t bytesPerTexel = PixelType::kDataSize;
        DAWN_ASSERT(bytesPerTexel == utils::GetTexelBlockSizeInBytes(textureSpec.format));
        const utils::TextureDataCopyLayout copyLayout =
            utils::GetTextureDataCopyLayoutForTextureAtLevel(
                textureSpec.format, textureSpec.textureSize, textureSpec.copyLevel, dimension,
                bufferSpec.rowsPerImage, GetTextureBytesPerRowAlignment());

        // Create a buffer and populate it with data
        wgpu::Buffer buffer;
        std::vector<uint8_t> bufferData(bufferSpec.offset, 0xff);
        {
            const std::vector<uint8_t> copyData =
                GetExpectedTextureData(textureSpec.format, copyLayout);

            bufferData.insert(bufferData.end(), copyData.begin(), copyData.end());
            bufferData.resize(Align(bufferSpec.size, 4));

            wgpu::BufferDescriptor descriptor;
            descriptor.size = bufferData.size();
            descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;
            descriptor.mappedAtCreation = true;
            buffer = device.CreateBuffer(&descriptor);
            memcpy(buffer.GetMappedRange(), bufferData.data(), bufferData.size());
            buffer.Unmap();
        }

        // Create a texture that is `width` x `height` with (`level` + 1) mip levels.
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = dimension;
        descriptor.size = textureSpec.textureSize;
        descriptor.sampleCount = 1;
        descriptor.format = textureSpec.format;
        descriptor.mipLevelCount = textureSpec.levelCount;
        descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        wgpu::TexelCopyBufferInfo texelCopyBufferInfo = utils::CreateTexelCopyBufferInfo(
            buffer, bufferSpec.offset, bufferSpec.bytesPerRow, bufferSpec.rowsPerImage);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
            texture, textureSpec.copyLevel, textureSpec.copyOrigin);
        encoder.CopyBufferToTexture(&texelCopyBufferInfo, &texelCopyTextureInfo, &copySize);

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        uint32_t copyLayer = copySize.depthOrArrayLayers;
        uint32_t copyDepth = 1;
        if (dimension == wgpu::TextureDimension::e3D) {
            copyLayer = 1;
            copyDepth = copySize.depthOrArrayLayers;
        }

        uint64_t bufferOffset = bufferSpec.offset;
        const uint32_t blockWidth = utils::GetTextureFormatBlockWidth(textureSpec.format);
        const uint32_t blockHeight = utils::GetTextureFormatBlockHeight(textureSpec.format);
        const uint32_t texelCountPerLayer = copyDepth * (copyLayout.mipSize.width / blockWidth) *
                                            (copyLayout.mipSize.height / blockHeight) *
                                            bytesPerTexel;
        for (uint32_t layer = 0; layer < copyLayer; ++layer) {
            // Copy and pack the data used to create the buffer in the specified copy region to have
            // the same format as the expected texture data.
            std::vector<PixelType> expected(texelCountPerLayer);
            CopyTextureData(bytesPerTexel, bufferData.data() + bufferOffset, copySize.width,
                            copySize.height, copyDepth, bufferSpec.bytesPerRow,
                            bufferSpec.rowsPerImage, expected.data(),
                            copySize.width * bytesPerTexel, copySize.height);

            EXPECT_TEXTURE_EQ(
                new ColorExpectation<PixelType>(
                    expected.data(), copySize.width * copySize.height * copyDepth, tolerance),
                texture,
                {textureSpec.copyOrigin.x, textureSpec.copyOrigin.y,
                 textureSpec.copyOrigin.z + layer},
                {copySize.width, copySize.height, copyDepth}, textureSpec.copyLevel)
                << "Buffer to Texture copy failed copying " << bufferSpec.size
                << "-byte buffer with offset " << bufferSpec.offset << " and bytes per row "
                << bufferSpec.bytesPerRow << " to [(" << textureSpec.copyOrigin.x << ", "
                << textureSpec.copyOrigin.y << "), (" << textureSpec.copyOrigin.x + copySize.width
                << ", " << textureSpec.copyOrigin.y + copySize.height << ")) region of "
                << textureSpec.textureSize.width << " x " << textureSpec.textureSize.height
                << " texture at mip level " << textureSpec.copyLevel << " layer " << layer << "\n";
            bufferOffset += copyLayout.bytesPerImage;
        }
    }
};

template <typename Parent>
class CopyTests_T2TBase : public CopyTests, public Parent {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::DawnInternalUsages};
    }

    void DoTest(const TextureSpec& srcSpec,
                const TextureSpec& dstSpec,
                const wgpu::Extent3D& copySize,
                wgpu::TextureDimension srcDimension,
                wgpu::TextureDimension dstDimension,
                bool copyWithinSameTexture = false) {
        const wgpu::TextureFormat format = srcSpec.format;

        wgpu::TextureDescriptor srcDescriptor;
        srcDescriptor.dimension = srcDimension;
        srcDescriptor.size = srcSpec.textureSize;
        srcDescriptor.sampleCount = 1;
        srcDescriptor.format = format;
        srcDescriptor.mipLevelCount = srcSpec.levelCount;
        srcDescriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
        wgpu::Texture srcTexture = this->device.CreateTexture(&srcDescriptor);

        wgpu::Texture dstTexture;
        if (copyWithinSameTexture) {
            dstTexture = srcTexture;
        } else {
            wgpu::TextureDescriptor dstDescriptor;
            dstDescriptor.dimension = dstDimension;
            dstDescriptor.size = dstSpec.textureSize;
            dstDescriptor.sampleCount = 1;
            dstDescriptor.format = dstSpec.format;
            dstDescriptor.mipLevelCount = dstSpec.levelCount;
            dstDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
            dstTexture = this->device.CreateTexture(&dstDescriptor);
        }

        // Create an upload buffer and use it to completely populate the subresources of the src
        // texture that will be copied from at the given mip level.
        const utils::TextureDataCopyLayout srcDataCopyLayout =
            utils::GetTextureDataCopyLayoutForTextureAtLevel(
                format,
                {srcSpec.textureSize.width, srcSpec.textureSize.height,
                 srcDimension == wgpu::TextureDimension::e3D
                     ? srcSpec.textureSize.depthOrArrayLayers
                     : copySize.depthOrArrayLayers},
                srcSpec.copyLevel, srcDimension);

        // Initialize the source texture
        const std::vector<uint8_t> srcTextureCopyData =
            GetExpectedTextureData(format, srcDataCopyLayout);
        {
            wgpu::TexelCopyTextureInfo texelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
                srcTexture, srcSpec.copyLevel, {0, 0, srcSpec.copyOrigin.z});
            wgpu::TexelCopyBufferLayout texelCopyBufferLayout = utils::CreateTexelCopyBufferLayout(
                0, srcDataCopyLayout.bytesPerRow, srcDataCopyLayout.rowsPerImage);
            this->queue.WriteTexture(&texelCopyTextureInfo, srcTextureCopyData.data(),
                                     srcDataCopyLayout.byteLength, &texelCopyBufferLayout,
                                     &srcDataCopyLayout.mipSize);
        }

        wgpu::CommandEncoder encoder = this->device.CreateCommandEncoder();

        // Perform the texture to texture copy
        wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(srcTexture, srcSpec.copyLevel, srcSpec.copyOrigin);
        wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dstTexture, dstSpec.copyLevel, dstSpec.copyOrigin);
        // (Off-topic) spot-test for defaulting of .aspect.
        srcTexelCopyTextureInfo.aspect = wgpu::TextureAspect::Undefined;
        dstTexelCopyTextureInfo.aspect = wgpu::TextureAspect::Undefined;
        encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo, &copySize);

        // Create an output buffer and use it to completely populate the subresources of the dst
        // texture that will be copied to at the given mip level.
        const utils::TextureDataCopyLayout dstDataCopyLayout =
            utils::GetTextureDataCopyLayoutForTextureAtLevel(
                format,
                {dstSpec.textureSize.width, dstSpec.textureSize.height,
                 dstDimension == wgpu::TextureDimension::e3D
                     ? dstSpec.textureSize.depthOrArrayLayers
                     : copySize.depthOrArrayLayers},
                dstSpec.copyLevel, dstDimension);
        wgpu::BufferDescriptor outputBufferDescriptor;
        outputBufferDescriptor.size = dstDataCopyLayout.byteLength;
        outputBufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer outputBuffer = this->device.CreateBuffer(&outputBufferDescriptor);
        const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(format);
        const uint32_t expectedDstDataOffset = dstSpec.copyOrigin.x * bytesPerTexel +
                                               dstSpec.copyOrigin.y * dstDataCopyLayout.bytesPerRow;
        wgpu::TexelCopyBufferInfo outputTexelCopyBufferInfo = utils::CreateTexelCopyBufferInfo(
            outputBuffer, expectedDstDataOffset, dstDataCopyLayout.bytesPerRow,
            dstDataCopyLayout.rowsPerImage);
        encoder.CopyTextureToBuffer(&dstTexelCopyTextureInfo, &outputTexelCopyBufferInfo,
                                    &copySize);

        wgpu::CommandBuffer commands = encoder.Finish();
        this->queue.Submit(1, &commands);

        // Validate if the data in outputBuffer is what we expected, including the untouched data
        // outside of the copy.
        {
            // Validate the output buffer slice-by-slice regardless of whether the destination
            // texture is 3D or 2D. The dimension here doesn't matter - we're only populating the
            // CPU data to verify against.
            uint32_t copyLayer = copySize.depthOrArrayLayers;
            uint32_t copyDepth = 1;

            const uint64_t validDataSizePerDstTextureLayer = utils::RequiredBytesInCopy(
                dstDataCopyLayout.bytesPerRow, dstDataCopyLayout.mipSize.height,
                dstDataCopyLayout.mipSize.width, dstDataCopyLayout.mipSize.height, copyDepth,
                bytesPerTexel);

            // expectedDstDataPerSlice stores one layer of the destination texture.
            std::vector<uint8_t> expectedDstDataPerSlice(validDataSizePerDstTextureLayer);
            for (uint32_t slice = 0; slice < copyLayer; ++slice) {
                // For each source texture array slice involved in the copy, emulate the T2T copy
                // on the CPU side by "copying" the copy data from the "source texture"
                // (srcTextureCopyData) to the "destination texture" (expectedDstDataPerSlice).
                std::fill(expectedDstDataPerSlice.begin(), expectedDstDataPerSlice.end(), 0);

                const uint32_t srcBytesOffset = srcDataCopyLayout.bytesPerImage * slice;

                // Get the offset of the srcTextureCopyData that contains the copy data on the
                // slice-th texture array layer of the source texture.
                const uint32_t srcTexelDataOffset =
                    srcBytesOffset + (srcSpec.copyOrigin.x * bytesPerTexel +
                                      srcSpec.copyOrigin.y * srcDataCopyLayout.bytesPerRow);
                // Do the T2T "copy" on the CPU side to get the expected texel value at the
                CopyTextureData(bytesPerTexel, &srcTextureCopyData[srcTexelDataOffset],
                                copySize.width, copySize.height, copyDepth,
                                srcDataCopyLayout.bytesPerRow, srcDataCopyLayout.rowsPerImage,
                                &expectedDstDataPerSlice[expectedDstDataOffset],
                                dstDataCopyLayout.bytesPerRow, dstDataCopyLayout.rowsPerImage);

                // Compare the content of the destination texture at the (dstSpec.copyOrigin.z +
                // slice)-th layer to its expected data after the copy (the outputBuffer contains
                // the data of the destination texture since the dstSpec.copyOrigin.z-th layer).
                uint64_t outputBufferExpectationBytesOffset =
                    dstDataCopyLayout.bytesPerImage * slice;
                EXPECT_BUFFER_U32_RANGE_EQ(
                    reinterpret_cast<const uint32_t*>(expectedDstDataPerSlice.data()), outputBuffer,
                    outputBufferExpectationBytesOffset,
                    validDataSizePerDstTextureLayer / sizeof(uint32_t));
            }
        }
    }

    void DoTest(const TextureSpec& srcSpec,
                const TextureSpec& dstSpec,
                const wgpu::Extent3D& copySize,
                bool copyWithinSameTexture = false,
                wgpu::TextureDimension dimension = wgpu::TextureDimension::e2D) {
        DoTest(srcSpec, dstSpec, copySize, dimension, dimension, copyWithinSameTexture);
    }
};

class CopyTests_T2T : public CopyTests_T2TBase<DawnTestWithParams<CopyTextureFormatParams>> {
  protected:
    struct TextureSpec : CopyTests::TextureSpec {
        TextureSpec() { format = GetParam().mTextureFormat; }
    };

    void SetUp() override {
        DawnTestWithParams<CopyTextureFormatParams>::SetUp();

        // TODO(dawn:2129): Fail for Win ANGLE D3D11
        DAWN_SUPPRESS_TEST_IF((GetParam().mTextureFormat == wgpu::TextureFormat::RGB9E5Ufloat) &&
                              IsANGLED3D11() && IsWindows());
    }
};

class CopyTests_T2T_Srgb : public CopyTests_T2TBase<DawnTestWithParams<CopyTextureFormatParams>> {
  protected:
    void SetUp() override {
        DawnTestWithParams<CopyTextureFormatParams>::SetUp();

        const auto format = GetParam().mTextureFormat;
        // BGRA8UnormSrgb is unsupported in Compatibility mode.
        DAWN_SUPPRESS_TEST_IF((format == wgpu::TextureFormat::BGRA8UnormSrgb ||
                               format == wgpu::TextureFormat::BGRA8Unorm) &&
                              IsCompatibilityMode());
    }

    // Texture format is compatible and could be copied to each other if the only diff is srgb-ness.
    wgpu::TextureFormat GetCopyCompatibleFormat(wgpu::TextureFormat format) {
        switch (format) {
            case wgpu::TextureFormat::RGBA8Unorm:
                return wgpu::TextureFormat::RGBA8UnormSrgb;
            case wgpu::TextureFormat::RGBA8UnormSrgb:
                return wgpu::TextureFormat::RGBA8Unorm;
            case wgpu::TextureFormat::BGRA8Unorm:
                return wgpu::TextureFormat::BGRA8UnormSrgb;
            case wgpu::TextureFormat::BGRA8UnormSrgb:
                return wgpu::TextureFormat::BGRA8Unorm;
            default:
                DAWN_UNREACHABLE();
        }
    }

    void DoTest(TextureSpec srcSpec,
                TextureSpec dstSpec,
                const wgpu::Extent3D& copySize,
                wgpu::TextureDimension srcDimension = wgpu::TextureDimension::e2D,
                wgpu::TextureDimension dstDimension = wgpu::TextureDimension::e2D) {
        srcSpec.format = GetParam().mTextureFormat;
        dstSpec.format = GetCopyCompatibleFormat(srcSpec.format);

        CopyTests_T2TBase<DawnTestWithParams<CopyTextureFormatParams>>::DoTest(
            srcSpec, dstSpec, copySize, srcDimension, dstDimension);
    }
};

class CopyTests_B2B : public DawnTest {
  protected:
    // This is the same signature as CopyBufferToBuffer except that the buffers are replaced by
    // only their size.
    void DoTest(uint64_t sourceSize,
                uint64_t sourceOffset,
                uint64_t destinationSize,
                uint64_t destinationOffset,
                uint64_t copySize) {
        DAWN_ASSERT(sourceSize % 4 == 0);
        DAWN_ASSERT(destinationSize % 4 == 0);

        // Create our two test buffers, destination filled with zeros, source filled with non-zeroes
        std::vector<uint32_t> zeroes(static_cast<size_t>(destinationSize / sizeof(uint32_t)));
        wgpu::Buffer destination =
            utils::CreateBufferFromData(device, zeroes.data(), zeroes.size() * sizeof(uint32_t),
                                        wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);

        std::vector<uint32_t> sourceData(static_cast<size_t>(sourceSize / sizeof(uint32_t)));
        for (size_t i = 0; i < sourceData.size(); i++) {
            sourceData[i] = i + 1;
        }
        wgpu::Buffer source = utils::CreateBufferFromData(device, sourceData.data(),
                                                          sourceData.size() * sizeof(uint32_t),
                                                          wgpu::BufferUsage::CopySrc);

        // Submit the copy
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(source, sourceOffset, destination, destinationOffset, copySize);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Check destination is exactly the expected content.
        EXPECT_BUFFER_U32_RANGE_EQ(zeroes.data(), destination, 0,
                                   destinationOffset / sizeof(uint32_t));
        EXPECT_BUFFER_U32_RANGE_EQ(sourceData.data() + sourceOffset / sizeof(uint32_t), destination,
                                   destinationOffset, copySize / sizeof(uint32_t));
        uint64_t copyEnd = destinationOffset + copySize;
        EXPECT_BUFFER_U32_RANGE_EQ(zeroes.data(), destination, copyEnd,
                                   (destinationSize - copyEnd) / sizeof(uint32_t));
    }
};

class ClearBufferTests : public DawnTest {
  protected:
    // This is the same signature as ClearBuffer except that the buffers are replaced by
    // only their size.
    void DoTest(uint64_t bufferSize, uint64_t clearOffset, uint64_t clearSize) {
        DAWN_ASSERT(bufferSize % 4 == 0);
        DAWN_ASSERT(clearSize % 4 == 0);

        // Create our test buffer, filled with non-zeroes
        std::vector<uint32_t> bufferData(static_cast<size_t>(bufferSize / sizeof(uint32_t)));
        for (size_t i = 0; i < bufferData.size(); i++) {
            bufferData[i] = i + 1;
        }
        wgpu::Buffer buffer = utils::CreateBufferFromData(
            device, bufferData.data(), bufferData.size() * sizeof(uint32_t),
            wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);

        std::vector<uint8_t> fillData(static_cast<size_t>(clearSize), 0u);

        // Submit the fill
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ClearBuffer(buffer, clearOffset, clearSize);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Check destination is exactly the expected content.
        EXPECT_BUFFER_U32_RANGE_EQ(bufferData.data(), buffer, 0, clearOffset / sizeof(uint32_t));
        EXPECT_BUFFER_U8_RANGE_EQ(fillData.data(), buffer, clearOffset, clearSize);
        uint64_t clearEnd = clearOffset + clearSize;
        EXPECT_BUFFER_U32_RANGE_EQ(bufferData.data() + clearEnd / sizeof(uint32_t), buffer,
                                   clearEnd, (bufferSize - clearEnd) / sizeof(uint32_t));
    }
};

// Test that copying an entire texture with 256-byte aligned dimensions works
TEST_P(CopyTests_T2B, FullTextureAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight), {kWidth, kHeight, 1});
}

// Test noop copies
TEST_P(CopyTests_T2B, ZeroSizedCopy) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight), {0, kHeight, 1});
    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight), {kWidth, 0, 1});
    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight), {kWidth, kHeight, 0});
}

// Test that copying an entire texture without 256-byte aligned dimensions works
TEST_P(CopyTests_T2B, FullTextureUnaligned) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight), {kWidth, kHeight, 1});
}

// Test that reading pixels from a 256-byte aligned texture works
TEST_P(CopyTests_T2B, PixelReadAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    BufferSpec pixelBuffer = MinimumBufferSpec(1, 1);

    constexpr wgpu::Extent3D kCopySize = {1, 1, 1};
    constexpr wgpu::Extent3D kTextureSize = {kWidth, kHeight, 1};
    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = kTextureSize;

    {
        TextureSpec textureSpec = defaultTextureSpec;
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {kWidth - 1, 0, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {0, kHeight - 1, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {kWidth - 1, kHeight - 1, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {kWidth / 3, kHeight / 7, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {kWidth / 7, kHeight / 3, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }
}

// Test that copying pixels from a texture that is not 256-byte aligned works
TEST_P(CopyTests_T2B, PixelReadUnaligned) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;
    BufferSpec pixelBuffer = MinimumBufferSpec(1, 1);

    constexpr wgpu::Extent3D kCopySize = {1, 1, 1};
    constexpr wgpu::Extent3D kTextureSize = {kWidth, kHeight, 1};
    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = kTextureSize;

    {
        TextureSpec textureSpec = defaultTextureSpec;
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {kWidth - 1, 0, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {0, kHeight - 1, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {kWidth - 1, kHeight - 1, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {kWidth / 3, kHeight / 7, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {kWidth / 7, kHeight / 3, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }
}

// Test that copying regions with 256-byte aligned sizes works
TEST_P(CopyTests_T2B, TextureRegionAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    for (unsigned int w : {64, 128, 256}) {
        for (unsigned int h : {16, 32, 48}) {
            TextureSpec textureSpec;
            textureSpec.textureSize = {kWidth, kHeight, 1};
            DoTest(textureSpec, MinimumBufferSpec(w, h), {w, h, 1});
        }
    }
}

// Test that copying regions without 256-byte aligned sizes works
TEST_P(CopyTests_T2B, TextureRegionUnaligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, 1};

    for (unsigned int w : {13, 63, 65}) {
        for (unsigned int h : {17, 19, 63}) {
            TextureSpec textureSpec = defaultTextureSpec;
            DoTest(textureSpec, MinimumBufferSpec(w, h), {w, h, 1});
        }
    }
}

// Test that copying mips with 256-byte aligned sizes works
TEST_P(CopyTests_T2B, TextureMipAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, 1};

    for (unsigned int i = 1; i < 4; ++i) {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyLevel = i;
        textureSpec.levelCount = i + 1;
        DoTest(textureSpec, MinimumBufferSpec(kWidth >> i, kHeight >> i),
               {kWidth >> i, kHeight >> i, 1});
    }
}

// Test that copying mips when one dimension is 256-byte aligned and another dimension reach one
// works
TEST_P(CopyTests_T2B, TextureMipDimensionReachOne) {
    constexpr uint32_t mipLevelCount = 4;
    constexpr uint32_t kWidth = 256 << mipLevelCount;
    constexpr uint32_t kHeight = 2;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, 1};

    TextureSpec textureSpec = defaultTextureSpec;
    textureSpec.levelCount = mipLevelCount;

    for (unsigned int i = 0; i < 4; ++i) {
        textureSpec.copyLevel = i;
        DoTest(textureSpec,
               MinimumBufferSpec(std::max(kWidth >> i, 1u), std::max(kHeight >> i, 1u)),
               {std::max(kWidth >> i, 1u), std::max(kHeight >> i, 1u), 1});
    }
}

// Test that copying mips without 256-byte aligned sizes works
TEST_P(CopyTests_T2B, TextureMipUnaligned) {
    // TODO(dawn:1880): suppress failing on Windows Intel Vulkan backend with
    // blit path toggles on. These toggles are only turned on for this
    // backend in the test so the defect won't impact the production code directly. But something is
    // wrong with the specific hardware.
    DAWN_SUPPRESS_TEST_IF(HasToggleEnabled("use_blit_for_snorm_texture_to_buffer_copy") &&
                          IsSnorm(GetParam().mTextureFormat) && IsVulkan() && IsIntel() &&
                          IsWindows());
    DAWN_SUPPRESS_TEST_IF(HasToggleEnabled("use_blit_for_bgra8unorm_texture_to_buffer_copy") &&
                          (GetParam().mTextureFormat == wgpu::TextureFormat::BGRA8Unorm) &&
                          IsVulkan() && IsIntel() && IsWindows());
    DAWN_SUPPRESS_TEST_IF(HasToggleEnabled("use_blit_for_rgb9e5ufloat_texture_copy") &&
                          (GetParam().mTextureFormat == wgpu::TextureFormat::RGB9E5Ufloat) &&
                          IsVulkan() && IsIntel() && IsWindows());

    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, 1};

    for (unsigned int i = 1; i < 4; ++i) {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyLevel = i;
        textureSpec.levelCount = i + 1;
        DoTest(textureSpec, MinimumBufferSpec(kWidth >> i, kHeight >> i),
               {kWidth >> i, kHeight >> i, 1});
    }
}

// Test that copying with a 512-byte aligned buffer offset works
TEST_P(CopyTests_T2B, OffsetBufferAligned) {
    // TODO(crbug.com/dawn/2294): diagnose T2B failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm() &&
                          GetParam().mTextureFormat == wgpu::TextureFormat::R16Float);

    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    for (unsigned int i = 0; i < 3; ++i) {
        BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
        uint64_t offset = 512 * i;
        bufferSpec.size += offset;
        bufferSpec.offset += offset;
        DoTest(textureSpec, bufferSpec, {kWidth, kHeight, 1});
    }
}

// Test that copying without a 512-byte aligned buffer offset works
TEST_P(CopyTests_T2B, OffsetBufferUnaligned) {
    // TODO(crbug.com/dawn/2294): diagnose T2B failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    constexpr uint32_t kWidth = 128;
    constexpr uint32_t kHeight = 128;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(textureSpec.format);
    for (uint32_t i = bytesPerTexel; i < 512; i += bytesPerTexel * 9) {
        BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
        bufferSpec.size += i;
        bufferSpec.offset += i;
        bufferSpec.size = Align(bufferSpec.size, 4);
        bufferSpec.offset = Align(bufferSpec.offset, 4);
        DoTest(textureSpec, bufferSpec, {kWidth, kHeight, 1});
    }
}

// Test that copying without a 512-byte aligned buffer offset works. Note: the buffer is mappable.
TEST_P(CopyTests_T2B, MappableBufferWithOffsetUnaligned) {
    // TODO(crbug.com/dawn/2294): diagnose T2B failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    constexpr wgpu::Extent2D kSizes[] = {
        {4, 4},
        {17, 17},
        {128, 128},
        {256, 256},
    };

    for (const auto size : kSizes) {
        TextureSpec textureSpec;
        textureSpec.textureSize = {size.width, size.height, 1};

        const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(textureSpec.format);
        for (uint32_t i = bytesPerTexel; i < 512; i += bytesPerTexel * 9) {
            BufferSpec bufferSpec = MinimumBufferSpec(size.width, size.height);
            bufferSpec.size += i;
            bufferSpec.offset += i;
            bufferSpec.size = Align(bufferSpec.size, 4);
            bufferSpec.offset = Align(bufferSpec.offset, 4);
            DoTest(textureSpec, bufferSpec, textureSpec.textureSize, wgpu::TextureDimension::e2D,
                   wgpu::TextureViewDimension::Undefined, /*useMappableBuffer=*/true);
        }
    }
}

// Test that copying from a texture to a mappable buffer won't overwrite the buffer's bytes
// before and after the copied region.
TEST_P(CopyTests_T2B, MappableBufferBeforeAndAfterBytesNotOverwritten) {
    // TODO(crbug.com/dawn/2294): diagnose T2B failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    constexpr uint32_t kWidth = 256;

    wgpu::TextureDescriptor texDesc;
    texDesc.dimension = wgpu::TextureDimension::e2D;
    texDesc.size = {kWidth, 1, 1};
    texDesc.sampleCount = 1;
    texDesc.format = GetParam().mTextureFormat;
    texDesc.mipLevelCount = 1;
    texDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
    wgpu::Texture texture = device.CreateTexture(&texDesc);

    // Layout for initial data upload to texture.
    // Some parts of this result are also reused later.
    const utils::TextureDataCopyLayout copyLayout =
        utils::GetTextureDataCopyLayoutForTextureAtLevel(texDesc.format, texDesc.size, 0,
                                                         wgpu::TextureDimension::e2D);

    const std::vector<uint8_t> textureArrayData =
        GetExpectedTextureData(texDesc.format, copyLayout);
    const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(texDesc.format);
    {
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
        wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
            utils::CreateTexelCopyBufferLayout(0, copyLayout.bytesPerRow, copyLayout.rowsPerImage);
        queue.WriteTexture(&texelCopyTextureInfo, textureArrayData.data(), copyLayout.byteLength,
                           &texelCopyBufferLayout, &copyLayout.mipSize);
    }

    // Create a destination buffer and fill its before & after bytes with random data
    const uint32_t kCopyOffset = bytesPerTexel;
    const auto kPastCopyOffset = kCopyOffset + textureArrayData.size();
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = Align(kPastCopyOffset + 16, 4);
    bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
    bufferDesc.mappedAtCreation = true;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    const auto kNumPastCopyBytes = bufferDesc.size - kPastCopyOffset;
    const std::vector<uint8_t> kExpectedFirstBytes(kCopyOffset, 97);
    const std::vector<uint8_t> kExpectedLastBytes(kNumPastCopyBytes, 99);
    {
        auto ptr = static_cast<uint8_t*>(buffer.GetMappedRange());
        memcpy(ptr, kExpectedFirstBytes.data(), kExpectedFirstBytes.size());
        memcpy(ptr + kPastCopyOffset, kExpectedLastBytes.data(), kExpectedLastBytes.size());
        buffer.Unmap();
    }

    // Copy the texture to buffer at offset=bytesPerTexel and with tightly packed rows.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo = utils::CreateTexelCopyBufferInfo(
            buffer, /*offset=*/kCopyOffset,
            /*bytesPerRow=*/256 * bytesPerTexel, /*rowsPerImage=*/1);
        encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &texDesc.size);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    {
        bool done = false;
        buffer.MapAsync(wgpu::MapMode::Read, 0, buffer.GetSize(),
                        wgpu::CallbackMode::AllowProcessEvents,
                        [&](wgpu::MapAsyncStatus status, wgpu::StringView) {
                            ASSERT_EQ(wgpu::MapAsyncStatus::Success, status);
                            done = true;
                        });
        while (!done) {
            WaitABit();
        }
    }

    // Check copied bytes
    const auto* bufferReadPtr = static_cast<const uint8_t*>(buffer.GetConstMappedRange());
    for (size_t i = 0; i < textureArrayData.size(); ++i) {
        EXPECT_EQ(bufferReadPtr[kCopyOffset + i], textureArrayData[i])
            << "failed at [" << kCopyOffset + i << "]";
    }

    // Check that the first & last bytes outside copied region remain intact after the copy.
    for (size_t i = 0; i < kCopyOffset; ++i) {
        EXPECT_EQ(bufferReadPtr[i], kExpectedFirstBytes[i]) << "failed at [" << i << "]";
    }

    for (size_t i = 0; i < kNumPastCopyBytes; ++i) {
        const size_t idx = kPastCopyOffset + i;
        EXPECT_EQ(bufferReadPtr[idx], kExpectedLastBytes[i]) << "failed at [" << idx << "]";
    }

    buffer.Unmap();
}

// Test that copying without a 512-byte aligned buffer offset that is greater than the bytes per row
// works
TEST_P(CopyTests_T2B, OffsetBufferUnalignedSmallBytesPerRow) {
    // TODO(crbug.com/dawn/2294): diagnose T2B failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    constexpr uint32_t kWidth = 32;
    constexpr uint32_t kHeight = 128;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(textureSpec.format);
    for (uint32_t i = 256 + bytesPerTexel; i < 512; i += bytesPerTexel * 9) {
        BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
        bufferSpec.size += i;
        bufferSpec.offset += i;
        bufferSpec.size = Align(bufferSpec.size, 4);
        bufferSpec.offset = Align(bufferSpec.offset, 4);
        DoTest(textureSpec, bufferSpec, {kWidth, kHeight, 1});
    }
}

// Test that copying with a greater bytes per row than needed on a 256-byte aligned texture works
TEST_P(CopyTests_T2B, BytesPerRowAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
    for (unsigned int i = 1; i < 4; ++i) {
        bufferSpec.bytesPerRow += 256;
        bufferSpec.size += 256 * kHeight;
        DoTest(textureSpec, bufferSpec, {kWidth, kHeight, 1});
    }
}

// Test that copying with a greater bytes per row than needed on a texture that is not 256-byte
// aligned works
TEST_P(CopyTests_T2B, BytesPerRowUnaligned) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
    for (unsigned int i = 1; i < 4; ++i) {
        bufferSpec.bytesPerRow += 256;
        bufferSpec.size += 256 * kHeight;
        DoTest(textureSpec, bufferSpec, {kWidth, kHeight, 1});
    }
}

// Test that copying with bytesPerRow = 0 and bytesPerRow < bytesInACompleteRow works
// when we're copying one row only
TEST_P(CopyTests_T2B, BytesPerRowWithOneRowCopy) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    {
        BufferSpec bufferSpec = MinimumBufferSpec(5, 1);

        // bytesPerRow undefined
        bufferSpec.bytesPerRow = wgpu::kCopyStrideUndefined;
        DoTest(textureSpec, bufferSpec, {5, 1, 1});
    }
}

TEST_P(CopyTests_T2B, StrideSpecialCases) {
    TextureSpec textureSpec;
    textureSpec.textureSize = {4, 4, 4};

    // bytesPerRow 0
    for (const wgpu::Extent3D copyExtent :
         {wgpu::Extent3D{0, 2, 2}, {0, 0, 2}, {0, 2, 0}, {0, 0, 0}}) {
        DoTest(textureSpec, MinimumBufferSpec(copyExtent, 0, 2), copyExtent);
    }

    // bytesPerRow undefined
    for (const wgpu::Extent3D copyExtent :
         {wgpu::Extent3D{2, 1, 1}, {2, 0, 1}, {2, 1, 0}, {2, 0, 0}}) {
        DoTest(textureSpec, MinimumBufferSpec(copyExtent, wgpu::kCopyStrideUndefined, 2),
               copyExtent);
    }

    // rowsPerImage 0
    for (const wgpu::Extent3D copyExtent :
         {wgpu::Extent3D{2, 0, 2}, {2, 0, 0}, {0, 0, 2}, {0, 0, 0}}) {
        DoTest(textureSpec, MinimumBufferSpec(copyExtent, 256, 0), copyExtent);
    }

    // rowsPerImage undefined
    for (const wgpu::Extent3D copyExtent : {wgpu::Extent3D{2, 2, 1}, {2, 2, 0}}) {
        DoTest(textureSpec, MinimumBufferSpec(copyExtent, 256, wgpu::kCopyStrideUndefined),
               copyExtent);
    }
}

// Test copying a single slice with rowsPerImage larger than copy height and rowsPerImage will not
// take effect. If rowsPerImage takes effect, it looks like the copy may go past the end of the
// buffer.
TEST_P(CopyTests_T2B, RowsPerImageShouldNotCauseBufferOOBIfDepthOrArrayLayersIsOne) {
    // TODO(crbug.com/dawn/2294): diagnose T2B failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    constexpr uint32_t kWidth = 250;
    constexpr uint32_t kHeight = 3;
    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};
    const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(textureSpec.format);

    // Check various offsets to cover each code path in the 2D split code in TextureCopySplitter.
    for (uint32_t offset : {0u, Align(bytesPerTexel, 4u), Align(bytesPerTexel * 8u, 4u)}) {
        BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
        bufferSpec.rowsPerImage = 2 * kHeight;
        bufferSpec.offset = offset;
        bufferSpec.size += offset;
        DoTest(textureSpec, bufferSpec, {kWidth, kHeight, 1});
        DoTest(textureSpec, bufferSpec, {kWidth, kHeight, 1}, wgpu::TextureDimension::e3D);
    }
}

// Test copying a single row with bytesPerRow larger than copy width and bytesPerRow will not
// take effect. If bytesPerRow takes effect, it looks like the copy may go past the end of the
// buffer.
TEST_P(CopyTests_T2B, BytesPerRowShouldNotCauseBufferOOBIfCopyHeightIsOne) {
    // TODO(crbug.com/dawn/2294): diagnose T2B failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    constexpr uint32_t kWidth = 250;
    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, 1, 1};
    const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(textureSpec.format);

    // Check various offsets to cover each code path in the 2D split code in TextureCopySplitter.
    for (uint32_t offset : {0u, Align(bytesPerTexel, 4u), Align(bytesPerTexel * 25u, 4u)}) {
        BufferSpec bufferSpec = MinimumBufferSpec(kWidth, 1);
        bufferSpec.bytesPerRow =
            Align(kWidth * bytesPerTexel + 99, 256);  // the default bytesPerRow is 1024.
        bufferSpec.offset = offset;
        bufferSpec.size += offset;
        DoTest(textureSpec, bufferSpec, {kWidth, 1, 1});
        DoTest(textureSpec, bufferSpec, {kWidth, 1, 1}, wgpu::TextureDimension::e3D);
    }
}

// Test that copying whole texture 2D array layers in one texture-to-buffer-copy works.
TEST_P(CopyTests_T2B, Texture2DArrayFull) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 6u;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kLayers};

    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight, kLayers), {kWidth, kHeight, kLayers});
}

// Test that copying a range of texture 2D array layers in one texture-to-buffer-copy works.
TEST_P(CopyTests_T2B, Texture2DArraySubRegion) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 6u;
    constexpr uint32_t kBaseLayer = 2u;
    constexpr uint32_t kCopyLayers = 3u;

    TextureSpec textureSpec;
    textureSpec.copyOrigin = {0, 0, kBaseLayer};
    textureSpec.textureSize = {kWidth, kHeight, kLayers};

    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight, kCopyLayers),
           {kWidth, kHeight, kCopyLayers});
}

// Test that copying texture 2D array mips with 256-byte aligned sizes works
TEST_P(CopyTests_T2B, Texture2DArrayMip) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 6u;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, kLayers};

    for (unsigned int i = 1; i < 4; ++i) {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyLevel = i;
        textureSpec.levelCount = i + 1;

        DoTest(textureSpec, MinimumBufferSpec(kWidth >> i, kHeight >> i, kLayers),
               {kWidth >> i, kHeight >> i, kLayers});
    }
}

// Test that copying from a range of texture 2D array layers in one texture-to-buffer-copy when
// RowsPerImage is not equal to the height of the texture works.
TEST_P(CopyTests_T2B, Texture2DArrayRegionNonzeroRowsPerImage) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 6u;
    constexpr uint32_t kBaseLayer = 2u;
    constexpr uint32_t kCopyLayers = 3u;

    constexpr uint32_t kRowsPerImage = kHeight * 2;

    TextureSpec textureSpec;
    textureSpec.copyOrigin = {0, 0, kBaseLayer};
    textureSpec.textureSize = {kWidth, kHeight, kLayers};

    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kRowsPerImage, kCopyLayers);
    bufferSpec.rowsPerImage = kRowsPerImage;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kCopyLayers});
}

// Test a special code path in the D3D12 backends when (BytesPerRow * RowsPerImage) is not a
// multiple of 512.
TEST_P(CopyTests_T2B, Texture2DArrayRegionWithOffsetOddRowsPerImage) {
    constexpr uint32_t kWidth = 64;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 8u;
    constexpr uint32_t kBaseLayer = 2u;
    constexpr uint32_t kCopyLayers = 5u;

    constexpr uint32_t kRowsPerImage = kHeight + 1;

    TextureSpec textureSpec;
    textureSpec.copyOrigin = {0, 0, kBaseLayer};
    textureSpec.textureSize = {kWidth, kHeight, kLayers};

    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kRowsPerImage, kCopyLayers);
    bufferSpec.offset += 128u;
    bufferSpec.size += 128u;
    bufferSpec.rowsPerImage = kRowsPerImage;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kCopyLayers});
}

// Test a special code path in the D3D12 backends when (BytesPerRow * RowsPerImage) is a multiple
// of 512.
TEST_P(CopyTests_T2B, Texture2DArrayRegionWithOffsetEvenRowsPerImage) {
    constexpr uint32_t kWidth = 64;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 8u;
    constexpr uint32_t kBaseLayer = 2u;
    constexpr uint32_t kCopyLayers = 4u;

    constexpr uint32_t kRowsPerImage = kHeight + 2;

    TextureSpec textureSpec;
    textureSpec.copyOrigin = {0, 0, kBaseLayer};
    textureSpec.textureSize = {kWidth, kHeight, kLayers};

    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kRowsPerImage, kCopyLayers);
    bufferSpec.offset += 128u;
    bufferSpec.size += 128u;
    bufferSpec.rowsPerImage = kRowsPerImage;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kCopyLayers});
}

// Testing a series of params for 1D texture texture-to-buffer-copy.
TEST_P(CopyTests_T2B, Texture1D) {
    // TODO(crbug.com/dawn/2294): diagnose T2B failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    // TODO(dawn:1705): support 1d texture.
    DAWN_SUPPRESS_TEST_IF(IsD3D11());

    // TODO(crbug.com/dawn/2408): Re-enable on this configuration.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsNvidia());

    struct Param {
        uint32_t textureWidth;
        uint32_t copyWidth;
        uint32_t copyOrigin;
    };
    constexpr Param params[] = {
        {256, 256, 0}, {256, 16, 128}, {256, 16, 0},  {8, 8, 0},
        {8, 1, 7},     {259, 259, 0},  {259, 255, 3},
    };

    constexpr uint32_t bufferOffsets[] = {0, 16, 256};

    for (const auto& p : params) {
        for (const uint32_t offset : bufferOffsets) {
            TextureSpec textureSpec;
            textureSpec.copyOrigin = {p.copyOrigin, 0, 0};
            textureSpec.textureSize = {p.textureWidth, 1, 1};

            BufferSpec bufferSpec = MinimumBufferSpec(p.copyWidth, 1, 1);
            bufferSpec.offset = offset;
            bufferSpec.size += offset;

            DoTest(textureSpec, bufferSpec, {p.copyWidth, 1, 1}, wgpu::TextureDimension::e1D);
        }
    }
}

// Test that copying whole 3D texture in one texture-to-buffer-copy works.
TEST_P(CopyTests_T2B, Texture3DFull) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kDepth = 6;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};

    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight, kDepth), {kWidth, kHeight, kDepth},
           wgpu::TextureDimension::e3D);
}

// Test that copying a range of texture 3D depths in one texture-to-buffer-copy works.
TEST_P(CopyTests_T2B, Texture3DSubRegion) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kDepth = 6;
    constexpr uint32_t kBaseDepth = 2u;
    constexpr uint32_t kCopyDepth = 3u;

    TextureSpec textureSpec;
    textureSpec.copyOrigin = {0, 0, kBaseDepth};
    textureSpec.textureSize = {kWidth, kHeight, kDepth};

    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight, kCopyDepth),
           {kWidth / 2, kHeight / 2, kCopyDepth}, wgpu::TextureDimension::e3D);
}

TEST_P(CopyTests_T2B, Texture3DNoSplitRowDataWithEmptyFirstRow) {
    // TODO(crbug.com/dawn/2294): diagnose T2B failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    constexpr uint32_t kWidth = 2;
    constexpr uint32_t kHeight = 4;
    constexpr uint32_t kDepth = 3;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};
    const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(textureSpec.format);
    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight, kDepth);

    // The tests below are designed to test TextureCopySplitter for 3D textures on D3D12.
    // Base: no split for a row + no empty first row
    bufferSpec.offset = Align(60u, bytesPerTexel);
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);

    // This test will cover: no split for a row + empty first row
    bufferSpec.offset = Align(260u, bytesPerTexel);
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);
}

TEST_P(CopyTests_T2B, Texture3DSplitRowDataWithoutEmptyFirstRow) {
    // TODO(crbug.com/dawn/2294): diagnose T2B failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;
    constexpr uint32_t kDepth = 3;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};
    const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(textureSpec.format);
    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight, kDepth);

    // The test below is designed to test TextureCopySplitter for 3D textures on D3D12.
    // This test will cover: split for a row + no empty first row for both split regions
    bufferSpec.offset = Align(260u, bytesPerTexel);
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);
}

TEST_P(CopyTests_T2B, Texture3DSplitRowDataWithEmptyFirstRow) {
    // TODO(crbug.com/dawn/2294): diagnose T2B failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    constexpr uint32_t kWidth = 39;
    constexpr uint32_t kHeight = 4;
    constexpr uint32_t kDepth = 3;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};
    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight, kDepth);

    // The tests below are designed to test TextureCopySplitter for 3D textures on D3D12.
    // This test will cover: split for a row + empty first row for the head block
    bufferSpec.offset = 400;
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);

    // This test will cover: split for a row + empty first row for the tail block
    bufferSpec.offset = 160;
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);
}

TEST_P(CopyTests_T2B, Texture3DCopyHeightIsOneCopyWidthIsTiny) {
    // TODO(crbug.com/dawn/2294): diagnose T2B failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    constexpr uint32_t kWidth = 2;
    constexpr uint32_t kHeight = 1;
    constexpr uint32_t kDepth = 3;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};
    const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(textureSpec.format);
    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight, kDepth);

    // The tests below are designed to test TextureCopySplitter for 3D textures on D3D12.
    // Base: no split for a row, no empty row, and copy height is 1
    bufferSpec.offset = Align(60u, bytesPerTexel);
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);

    // This test will cover: no split for a row + empty first row, and copy height is 1
    bufferSpec.offset = Align(260u, bytesPerTexel);
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);
}

TEST_P(CopyTests_T2B, Texture3DCopyHeightIsOneCopyWidthIsSmall) {
    // TODO(crbug.com/dawn/2294): diagnose T2B failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    constexpr uint32_t kWidth = 39;
    constexpr uint32_t kHeight = 1;
    constexpr uint32_t kDepth = 3;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};
    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight, kDepth);

    // The tests below are designed to test TextureCopySplitter for 3D textures on D3D12.
    // This test will cover: split for a row + empty first row for the head block, and copy height
    // is 1
    bufferSpec.offset = 400;
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);

    // This test will cover: split for a row + empty first row for the tail block, and copy height
    // is 1
    bufferSpec.offset = 160;
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);
}

// Test that copying texture 3D array mips with 256-byte aligned sizes works
TEST_P(CopyTests_T2B, Texture3DMipAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kDepth = 64u;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, kDepth};

    for (unsigned int i = 1; i < 6; ++i) {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyLevel = i;
        textureSpec.levelCount = i + 1;

        DoTest(textureSpec, MinimumBufferSpec(kWidth >> i, kHeight >> i, kDepth >> i),
               {kWidth >> i, kHeight >> i, kDepth >> i}, wgpu::TextureDimension::e3D);
    }
}

// Test that copying texture 3D array mips with 256-byte unaligned sizes works
TEST_P(CopyTests_T2B, Texture3DMipUnaligned) {
    constexpr uint32_t kWidth = 261;
    constexpr uint32_t kHeight = 123;
    constexpr uint32_t kDepth = 69u;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, kDepth};

    for (unsigned int i = 1; i < 6; ++i) {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyLevel = i;
        textureSpec.levelCount = i + 1;

        DoTest(textureSpec, MinimumBufferSpec(kWidth >> i, kHeight >> i, kDepth >> i),
               {kWidth >> i, kHeight >> i, kDepth >> i}, wgpu::TextureDimension::e3D);
    }
}

DAWN_INSTANTIATE_TEST_P(CopyTests_T2B,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), OpenGLBackend(),
                         OpenGLESBackend(), VulkanBackend(),
                         VulkanBackend({"use_blit_for_snorm_texture_to_buffer_copy",
                                        "use_blit_for_bgra8unorm_texture_to_buffer_copy"})},
                        {
                            wgpu::TextureFormat::R8Unorm,
                            wgpu::TextureFormat::RG8Unorm,
                            wgpu::TextureFormat::RGBA8Unorm,

                            wgpu::TextureFormat::R8Uint,
                            wgpu::TextureFormat::R8Sint,

                            wgpu::TextureFormat::R16Uint,
                            wgpu::TextureFormat::R16Sint,
                            wgpu::TextureFormat::R16Float,

                            wgpu::TextureFormat::RG16Uint,
                            wgpu::TextureFormat::RG16Sint,
                            wgpu::TextureFormat::RG16Float,

                            wgpu::TextureFormat::R32Uint,
                            wgpu::TextureFormat::R32Sint,
                            wgpu::TextureFormat::R32Float,

                            wgpu::TextureFormat::RG32Float,
                            wgpu::TextureFormat::RG32Uint,
                            wgpu::TextureFormat::RG32Sint,

                            wgpu::TextureFormat::RGBA16Uint,
                            wgpu::TextureFormat::RGBA16Sint,
                            wgpu::TextureFormat::RGBA16Float,

                            wgpu::TextureFormat::RGBA32Float,

                            wgpu::TextureFormat::RGB10A2Unorm,
                            wgpu::TextureFormat::RG11B10Ufloat,

                            // Testing OpenGL compat Toggle::UseBlitForRGB9E5UfloatTextureCopy
                            wgpu::TextureFormat::RGB9E5Ufloat,

                            // Testing OpenGL compat Toggle::UseBlitForSnormTextureToBufferCopy
                            wgpu::TextureFormat::R8Snorm,
                            wgpu::TextureFormat::RG8Snorm,
                            wgpu::TextureFormat::RGBA8Snorm,

                            // Testing OpenGL compat Toggle::UseBlitForBGRA8UnormTextureToBufferCopy
                            wgpu::TextureFormat::BGRA8Unorm,
                        });

class CopyTests_T2B_No_Format_Param : public CopyTests, public DawnTest {};

// A regression test for a bug on D3D12 backend that causes crash when doing texture-to-texture
// copy one row with the texture format Depth32Float.
TEST_P(CopyTests_T2B_No_Format_Param, CopyOneRowWithDepth32Float) {
    constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::Depth32Float;
    constexpr uint32_t kPixelsPerRow = 4u;

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.format = kFormat;
    textureDescriptor.size = {kPixelsPerRow, 1, 1};
    textureDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // Initialize the depth texture with 0.5f.
    constexpr float kClearDepthValue = 0.5f;
    utils::ComboRenderPassDescriptor renderPass({}, texture.CreateView());
    renderPass.UnsetDepthStencilLoadStoreOpsForFormat(kFormat);
    renderPass.cDepthStencilAttachmentInfo.depthClearValue = kClearDepthValue;
    renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Clear;
    renderPass.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Store;
    wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPass);
    renderPassEncoder.End();

    constexpr uint32_t kBufferCopyOffset = kTextureBytesPerRowAlignment;
    const uint32_t kBufferSize =
        kBufferCopyOffset + utils::GetTexelBlockSizeInBytes(kFormat) * kPixelsPerRow;
    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = kBufferSize;
    bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
        utils::CreateTexelCopyBufferInfo(buffer, kBufferCopyOffset, kTextureBytesPerRowAlignment);
    wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
        utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
    // (Off-topic) spot-test for defaulting of .aspect.
    texelCopyTextureInfo.aspect = wgpu::TextureAspect::Undefined;

    wgpu::Extent3D copySize = textureDescriptor.size;
    encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &copySize);
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    std::array<float, kPixelsPerRow> expectedValues;
    std::fill(expectedValues.begin(), expectedValues.end(), kClearDepthValue);
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedValues.data(), buffer, kBufferCopyOffset, kPixelsPerRow);
}

DAWN_INSTANTIATE_TEST(CopyTests_T2B_No_Format_Param,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      VulkanBackend({"use_blit_for_depth32float_texture_to_buffer_copy"}));

class CopyTests_T2B_Compat : public CopyTests_T2B {
  protected:
    void SetUp() override {
        CopyTests_T2B::SetUp();
        DAWN_SUPPRESS_TEST_IF(!IsCompatibilityMode());
        DAWN_SUPPRESS_TEST_IF(IsANGLESwiftShader());
    }
};

// Test that copying 2d texture array with binding view dimension set to cube.
TEST_P(CopyTests_T2B_Compat, TextureCubeFull) {
    constexpr uint32_t kWidth = 32;
    constexpr uint32_t kHeight = 32;
    constexpr uint32_t kLayers = 6;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kLayers};

    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight, kLayers), {kWidth, kHeight, kLayers},
           wgpu::TextureDimension::e2D, wgpu::TextureViewDimension::Cube);
}

// Test that copying a range of cube texture layers in one texture-to-buffer-copy works.
TEST_P(CopyTests_T2B_Compat, TextureCubeSubRegion) {
    constexpr uint32_t kWidth = 32;
    constexpr uint32_t kHeight = 32;
    constexpr uint32_t kLayers = 6;
    constexpr uint32_t kBaseLayer = 2;
    constexpr uint32_t kCopyLayers = 3;

    TextureSpec textureSpec;
    textureSpec.copyOrigin = {0, 0, kBaseLayer};
    textureSpec.textureSize = {kWidth, kHeight, kLayers};

    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight, kCopyLayers),
           {kWidth, kHeight, kCopyLayers}, wgpu::TextureDimension::e2D,
           wgpu::TextureViewDimension::Cube);
}

// Test that copying texture 2D array mips with 256-byte aligned sizes works
TEST_P(CopyTests_T2B_Compat, TextureCubeMip) {
    constexpr uint32_t kWidth = 32;
    constexpr uint32_t kHeight = 32;
    constexpr uint32_t kLayers = 6;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, kLayers};

    for (unsigned int i = 1; i < 4; ++i) {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyLevel = i;
        textureSpec.levelCount = i + 1;

        DoTest(textureSpec, MinimumBufferSpec(kWidth >> i, kHeight >> i, kLayers),
               {kWidth >> i, kHeight >> i, kLayers}, wgpu::TextureDimension::e2D,
               wgpu::TextureViewDimension::Cube);
    }
}

// Test that copying from a range of texture 2D array layers in one texture-to-buffer-copy when
// RowsPerImage is not equal to the height of the texture works.
TEST_P(CopyTests_T2B_Compat, TextureCubeRegionNonzeroRowsPerImage) {
    constexpr uint32_t kWidth = 32;
    constexpr uint32_t kHeight = 32;
    constexpr uint32_t kLayers = 6;
    constexpr uint32_t kBaseLayer = 2;
    constexpr uint32_t kCopyLayers = 3;

    constexpr uint32_t kRowsPerImage = kHeight * 2;

    TextureSpec textureSpec;
    textureSpec.copyOrigin = {0, 0, kBaseLayer};
    textureSpec.textureSize = {kWidth, kHeight, kLayers};

    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kRowsPerImage, kCopyLayers);
    bufferSpec.rowsPerImage = kRowsPerImage;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kCopyLayers}, wgpu::TextureDimension::e2D,
           wgpu::TextureViewDimension::Cube);
}

DAWN_INSTANTIATE_TEST_P(CopyTests_T2B_Compat,
                        {
                            D3D11Backend(),
                            OpenGLBackend(),
                            OpenGLESBackend(),
                        },
                        {
                            // Control case: format not using blit workaround
                            wgpu::TextureFormat::RGBA8Unorm,

                            // Testing OpenGL compat Toggle::UseBlitForSnormTextureToBufferCopy
                            wgpu::TextureFormat::R8Snorm,
                            wgpu::TextureFormat::RG8Snorm,
                            wgpu::TextureFormat::RGBA8Snorm,

                            // Testing OpenGL compat Toggle::UseBlitForBGRA8UnormTextureToBufferCopy
                            wgpu::TextureFormat::BGRA8Unorm,

                            // Testing OpenGL compat Toggle::UseBlitForRGB9E5UfloatTextureCopy
                            wgpu::TextureFormat::RGB9E5Ufloat,
                        });

// Test that copying an entire texture with 256-byte aligned dimensions works
TEST_P(CopyTests_B2T, FullTextureAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight), {kWidth, kHeight, 1});
}

// Test noop copies.
TEST_P(CopyTests_B2T, ZeroSizedCopy) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight), {0, kHeight, 1});
    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight), {kWidth, 0, 1});
    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight), {kWidth, kHeight, 0});
}

// Test that copying an entire texture without 256-byte aligned dimensions works
TEST_P(CopyTests_B2T, FullTextureUnaligned) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight), {kWidth, kHeight, 1});
}

// Test that reading pixels from a 256-byte aligned texture works
TEST_P(CopyTests_B2T, PixelReadAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    BufferSpec pixelBuffer = MinimumBufferSpec(1, 1);

    constexpr wgpu::Extent3D kCopySize = {1, 1, 1};
    constexpr wgpu::Extent3D kTextureSize = {kWidth, kHeight, 1};
    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = kTextureSize;

    {
        TextureSpec textureSpec = defaultTextureSpec;
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {kWidth - 1, 0, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {0, kHeight - 1, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {kWidth - 1, kHeight - 1, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {kWidth / 3, kHeight / 7, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {kWidth / 7, kHeight / 3, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }
}

// Test that copying pixels from a texture that is not 256-byte aligned works
TEST_P(CopyTests_B2T, PixelReadUnaligned) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;
    BufferSpec pixelBuffer = MinimumBufferSpec(1, 1);

    constexpr wgpu::Extent3D kCopySize = {1, 1, 1};
    constexpr wgpu::Extent3D kTextureSize = {kWidth, kHeight, 1};
    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = kTextureSize;

    {
        TextureSpec textureSpec = defaultTextureSpec;
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {kWidth - 1, 0, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {0, kHeight - 1, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {kWidth - 1, kHeight - 1, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {kWidth / 3, kHeight / 7, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }

    {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyOrigin = {kWidth / 7, kHeight / 3, 0};
        DoTest(textureSpec, pixelBuffer, kCopySize);
    }
}

// Test that copying regions with 256-byte aligned sizes works
TEST_P(CopyTests_B2T, TextureRegionAligned) {
    // TODO(crbug.com/dawn/2294): diagnose T2B failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    for (unsigned int w : {64, 128, 256}) {
        for (unsigned int h : {16, 32, 48}) {
            TextureSpec textureSpec;
            textureSpec.textureSize = {kWidth, kHeight, 1};
            DoTest(textureSpec, MinimumBufferSpec(w, h), {w, h, 1});
        }
    }
}

// Test that copying regions without 256-byte aligned sizes works
TEST_P(CopyTests_B2T, TextureRegionUnaligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, 1};

    for (unsigned int w : {13, 63, 65}) {
        for (unsigned int h : {17, 19, 63}) {
            TextureSpec textureSpec = defaultTextureSpec;
            DoTest(textureSpec, MinimumBufferSpec(w, h), {w, h, 1});
        }
    }
}

// Test that copying mips with 256-byte aligned sizes works
TEST_P(CopyTests_B2T, TextureMipAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, 1};

    for (unsigned int i = 1; i < 4; ++i) {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyLevel = i;
        textureSpec.levelCount = i + 1;
        DoTest(textureSpec, MinimumBufferSpec(kWidth >> i, kHeight >> i),
               {kWidth >> i, kHeight >> i, 1});
    }
}

// Test that copying mips without 256-byte aligned sizes works
TEST_P(CopyTests_B2T, TextureMipUnaligned) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, 1};

    for (unsigned int i = 1; i < 4; ++i) {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyLevel = i;
        textureSpec.levelCount = i + 1;
        DoTest(textureSpec, MinimumBufferSpec(kWidth >> i, kHeight >> i),
               {kWidth >> i, kHeight >> i, 1});
    }
}

// Test that copying with a 512-byte aligned buffer offset works
TEST_P(CopyTests_B2T, OffsetBufferAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    for (unsigned int i = 0; i < 3; ++i) {
        BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
        uint64_t offset = 512 * i;
        bufferSpec.size += offset;
        bufferSpec.offset += offset;
        DoTest(textureSpec, bufferSpec, {kWidth, kHeight, 1});
    }
}

// Test that copying without a 512-byte aligned buffer offset works
TEST_P(CopyTests_B2T, OffsetBufferUnaligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(textureSpec.format);
    for (uint32_t i = bytesPerTexel; i < 512; i += bytesPerTexel * 9) {
        BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
        bufferSpec.size += i;
        bufferSpec.offset += i;
        bufferSpec.size = Align(bufferSpec.size, bytesPerTexel);
        bufferSpec.offset = Align(bufferSpec.offset, bytesPerTexel);
        DoTest(textureSpec, bufferSpec, {kWidth, kHeight, 1});
    }
}

// Test that copying without a 512-byte aligned buffer offset that is greater than the bytes per row
// works
TEST_P(CopyTests_B2T, OffsetBufferUnalignedSmallBytesPerRow) {
    constexpr uint32_t kWidth = 32;
    constexpr uint32_t kHeight = 128;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(textureSpec.format);
    for (uint32_t i = 256 + bytesPerTexel; i < 512; i += bytesPerTexel * 9) {
        BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
        bufferSpec.size += i;
        bufferSpec.offset += i;
        DoTest(textureSpec, bufferSpec, {kWidth, kHeight, 1});
    }
}

// Test that copying with a greater bytes per row than needed on a 256-byte aligned texture works
TEST_P(CopyTests_B2T, BytesPerRowAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
    for (unsigned int i = 1; i < 4; ++i) {
        bufferSpec.bytesPerRow += 256;
        bufferSpec.size += 256 * kHeight;
        DoTest(textureSpec, bufferSpec, {kWidth, kHeight, 1});
    }
}

// Test that copying with a greater bytes per row than needed on a texture that is not 256-byte
// aligned works
TEST_P(CopyTests_B2T, BytesPerRowUnaligned) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
    for (unsigned int i = 1; i < 4; ++i) {
        bufferSpec.bytesPerRow += 256;
        bufferSpec.size += 256 * kHeight;
        DoTest(textureSpec, bufferSpec, {kWidth, kHeight, 1});
    }
}

// Test that copying with bytesPerRow = 0 and bytesPerRow < bytesInACompleteRow works
// when we're copying one row only
TEST_P(CopyTests_B2T, BytesPerRowWithOneRowCopy) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};

    {
        BufferSpec bufferSpec = MinimumBufferSpec(5, 1);

        // bytesPerRow undefined
        bufferSpec.bytesPerRow = wgpu::kCopyStrideUndefined;
        DoTest(textureSpec, bufferSpec, {5, 1, 1});
    }
}

TEST_P(CopyTests_B2T, StrideSpecialCases) {
    TextureSpec textureSpec;
    textureSpec.textureSize = {4, 4, 4};

    // bytesPerRow 0
    for (const wgpu::Extent3D copyExtent :
         {wgpu::Extent3D{0, 2, 2}, {0, 0, 2}, {0, 2, 0}, {0, 0, 0}}) {
        DoTest(textureSpec, MinimumBufferSpec(copyExtent, 0, 2), copyExtent);
    }

    // bytesPerRow undefined
    for (const wgpu::Extent3D copyExtent :
         {wgpu::Extent3D{2, 1, 1}, {2, 0, 1}, {2, 1, 0}, {2, 0, 0}}) {
        DoTest(textureSpec, MinimumBufferSpec(copyExtent, wgpu::kCopyStrideUndefined, 2),
               copyExtent);
    }

    // rowsPerImage 0
    for (const wgpu::Extent3D copyExtent :
         {wgpu::Extent3D{2, 0, 2}, {2, 0, 0}, {0, 0, 2}, {0, 0, 0}}) {
        DoTest(textureSpec, MinimumBufferSpec(copyExtent, 256, 0), copyExtent);
    }

    // rowsPerImage undefined
    for (const wgpu::Extent3D copyExtent : {wgpu::Extent3D{2, 2, 1}, {2, 2, 0}}) {
        DoTest(textureSpec, MinimumBufferSpec(copyExtent, 256, wgpu::kCopyStrideUndefined),
               copyExtent);
    }
}

// Test that copying whole texture 2D array layers in one texture-to-buffer-copy works.
TEST_P(CopyTests_B2T, Texture2DArrayFull) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 6u;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kLayers};

    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight, kLayers), {kWidth, kHeight, kLayers});
}

// Test that copying a range of texture 2D array layers in one texture-to-buffer-copy works.
TEST_P(CopyTests_B2T, Texture2DArraySubRegion) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 6u;
    constexpr uint32_t kBaseLayer = 2u;
    constexpr uint32_t kCopyLayers = 3u;

    TextureSpec textureSpec;
    textureSpec.copyOrigin = {0, 0, kBaseLayer};
    textureSpec.textureSize = {kWidth, kHeight, kLayers};

    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight, kCopyLayers),
           {kWidth, kHeight, kCopyLayers});
}

// Test that copying into a range of texture 2D array layers in one texture-to-buffer-copy when
// RowsPerImage is not equal to the height of the texture works.
TEST_P(CopyTests_B2T, Texture2DArrayRegionNonzeroRowsPerImage) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 6u;
    constexpr uint32_t kBaseLayer = 2u;
    constexpr uint32_t kCopyLayers = 3u;

    constexpr uint32_t kRowsPerImage = kHeight * 2;

    TextureSpec textureSpec;
    textureSpec.copyOrigin = {0, 0, kBaseLayer};
    textureSpec.textureSize = {kWidth, kHeight, kLayers};

    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kRowsPerImage, kCopyLayers);
    bufferSpec.rowsPerImage = kRowsPerImage;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kCopyLayers});
}

// Test a special code path in the D3D12 backends when (BytesPerRow * RowsPerImage) is not a
// multiple of 512.
TEST_P(CopyTests_B2T, Texture2DArrayRegionWithOffsetOddRowsPerImage) {
    constexpr uint32_t kWidth = 64;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 8u;
    constexpr uint32_t kBaseLayer = 2u;
    constexpr uint32_t kCopyLayers = 5u;

    constexpr uint32_t kRowsPerImage = kHeight + 1;

    TextureSpec textureSpec;
    textureSpec.copyOrigin = {0, 0, kBaseLayer};
    textureSpec.textureSize = {kWidth, kHeight, kLayers};

    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kRowsPerImage, kCopyLayers);
    bufferSpec.offset += 128u;
    bufferSpec.size += 128u;
    bufferSpec.rowsPerImage = kRowsPerImage;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kCopyLayers});
}

// Test a special code path in the D3D12 backends when (BytesPerRow * RowsPerImage) is a multiple
// of 512.
TEST_P(CopyTests_B2T, Texture2DArrayRegionWithOffsetEvenRowsPerImage) {
    constexpr uint32_t kWidth = 64;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 8u;
    constexpr uint32_t kBaseLayer = 2u;
    constexpr uint32_t kCopyLayers = 5u;

    constexpr uint32_t kRowsPerImage = kHeight + 2;

    TextureSpec textureSpec;
    textureSpec.copyOrigin = {0, 0, kBaseLayer};
    textureSpec.textureSize = {kWidth, kHeight, kLayers};

    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kRowsPerImage, kCopyLayers);
    bufferSpec.offset += 128u;
    bufferSpec.size += 128u;
    bufferSpec.rowsPerImage = kRowsPerImage;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kCopyLayers});
}

// Test that copying whole texture 3D in one buffer-to-texture-copy works.
TEST_P(CopyTests_B2T, Texture3DFull) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kDepth = 6;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};

    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight, kDepth), {kWidth, kHeight, kDepth},
           wgpu::TextureDimension::e3D);
}

// Test that copying a range of texture 3D Depths in one texture-to-buffer-copy works.
TEST_P(CopyTests_B2T, Texture3DSubRegion) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kDepth = 6;
    constexpr uint32_t kBaseDepth = 2u;
    constexpr uint32_t kCopyDepth = 3u;

    TextureSpec textureSpec;
    textureSpec.copyOrigin = {0, 0, kBaseDepth};
    textureSpec.textureSize = {kWidth, kHeight, kDepth};

    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight, kCopyDepth),
           {kWidth / 2, kHeight / 2, kCopyDepth}, wgpu::TextureDimension::e3D);
}

TEST_P(CopyTests_B2T, Texture3DNoSplitRowDataWithEmptyFirstRow) {
    constexpr uint32_t kWidth = 2;
    constexpr uint32_t kHeight = 4;
    constexpr uint32_t kDepth = 3;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};
    const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(textureSpec.format);
    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight, kDepth);

    // The tests below are designed to test TextureCopySplitter for 3D textures on D3D12.
    // Base: no split for a row + no empty first row
    bufferSpec.offset = Align(60u, bytesPerTexel);
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);

    // This test will cover: no split for a row + empty first row
    bufferSpec.offset = Align(260u, bytesPerTexel);
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);
}

TEST_P(CopyTests_B2T, Texture3DSplitRowDataWithoutEmptyFirstRow) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;
    constexpr uint32_t kDepth = 3;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};
    const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(textureSpec.format);
    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight, kDepth);

    // The test below is designed to test TextureCopySplitter for 3D textures on D3D12.
    // This test will cover: split for a row + no empty first row for both split regions
    bufferSpec.offset = Align(260u, bytesPerTexel);
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);
}

TEST_P(CopyTests_B2T, Texture3DSplitRowDataWithEmptyFirstRow) {
    constexpr uint32_t kWidth = 39;
    constexpr uint32_t kHeight = 4;
    constexpr uint32_t kDepth = 3;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};
    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight, kDepth);

    // The tests below are designed to test TextureCopySplitter for 3D textures on D3D12.
    // This test will cover: split for a row + empty first row for the head block
    bufferSpec.offset = 400;
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);

    // This test will cover: split for a row + empty first row for the tail block
    bufferSpec.offset = 160;
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);
}

TEST_P(CopyTests_B2T, Texture3DCopyHeightIsOneCopyWidthIsTiny) {
    // TODO(crbug.com/dawn/2294): diagnose T2B failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    constexpr uint32_t kWidth = 2;
    constexpr uint32_t kHeight = 1;
    constexpr uint32_t kDepth = 3;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};
    const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(textureSpec.format);
    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight, kDepth);

    // The tests below are designed to test TextureCopySplitter for 3D textures on D3D12.
    // Base: no split for a row, no empty row, and copy height is 1
    bufferSpec.offset = Align(60u, bytesPerTexel);
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);

    // This test will cover: no split for a row + empty first row, and copy height is 1
    bufferSpec.offset = Align(260u, bytesPerTexel);
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);
}

TEST_P(CopyTests_B2T, Texture3DCopyHeightIsOneCopyWidthIsSmall) {
    constexpr uint32_t kWidth = 39;
    constexpr uint32_t kHeight = 1;
    constexpr uint32_t kDepth = 3;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};
    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight, kDepth);

    // The tests below are designed to test TextureCopySplitter for 3D textures on D3D12.
    // This test will cover: split for a row + empty first row for the head block, and copy height
    // is 1
    bufferSpec.offset = 400;
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);

    // This test will cover: split for a row + empty first row for the tail block, and copy height
    // is 1
    bufferSpec.offset = 160;
    bufferSpec.size += bufferSpec.offset;
    DoTest(textureSpec, bufferSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);
}

// Test that copying texture 3D array mips with 256-byte aligned sizes works
TEST_P(CopyTests_B2T, Texture3DMipAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kDepth = 64u;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, kDepth};

    for (unsigned int i = 1; i < 6; ++i) {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyLevel = i;
        textureSpec.levelCount = i + 1;

        DoTest(textureSpec, MinimumBufferSpec(kWidth >> i, kHeight >> i, kDepth >> i),
               {kWidth >> i, kHeight >> i, kDepth >> i}, wgpu::TextureDimension::e3D);
    }
}

// Test that copying texture 3D array mips with 256-byte unaligned sizes works
TEST_P(CopyTests_B2T, Texture3DMipUnaligned) {
    constexpr uint32_t kWidth = 261;
    constexpr uint32_t kHeight = 123;
    constexpr uint32_t kDepth = 69u;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, kDepth};

    for (unsigned int i = 1; i < 6; ++i) {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyLevel = i;
        textureSpec.levelCount = i + 1;

        DoTest(textureSpec, MinimumBufferSpec(kWidth >> i, kHeight >> i, kDepth >> i),
               {kWidth >> i, kHeight >> i, kDepth >> i}, wgpu::TextureDimension::e3D);
    }
}

// Test that copying a texture 1D works.
TEST_P(CopyTests_B2T, Texture1DFull) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 1;
    constexpr uint32_t kDepth = 1;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};

    DoTest(textureSpec, MinimumBufferSpec(kWidth, kHeight, kDepth), {kWidth, kHeight, kDepth},
           wgpu::TextureDimension::e1D);
}

DAWN_INSTANTIATE_TEST_P(CopyTests_B2T,
                        {D3D11Backend(), D3D11Backend({"d3d11_disable_cpu_buffers"}),
                         D3D12Backend(), MetalBackend(), OpenGLBackend(), OpenGLESBackend(),
                         VulkanBackend()},
                        {
                            wgpu::TextureFormat::R8Unorm,
                            wgpu::TextureFormat::RG8Unorm,
                            wgpu::TextureFormat::RGBA8Unorm,

                            wgpu::TextureFormat::RGB10A2Unorm,

                            wgpu::TextureFormat::R16Float,
                            wgpu::TextureFormat::R16Unorm,

                            wgpu::TextureFormat::RG16Float,
                            wgpu::TextureFormat::RG16Unorm,

                            wgpu::TextureFormat::R32Float,

                            wgpu::TextureFormat::RG32Float,

                            wgpu::TextureFormat::RGBA16Float,
                            wgpu::TextureFormat::RGBA16Unorm,

                            wgpu::TextureFormat::RGBA32Float,

                            wgpu::TextureFormat::BGRA8Unorm,
                        });

TEST_P(CopyTests_T2T, Texture) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};
    DoTest(textureSpec, textureSpec, {kWidth, kHeight, 1});
}

// Test noop copies.
TEST_P(CopyTests_T2T, ZeroSizedCopy) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};
    DoTest(textureSpec, textureSpec, {0, kHeight, 1});
    DoTest(textureSpec, textureSpec, {kWidth, 0, 1});
    DoTest(textureSpec, textureSpec, {kWidth, kHeight, 0});
}

TEST_P(CopyTests_T2T, TextureRegion) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, 1};

    for (unsigned int w : {64, 128, 256}) {
        for (unsigned int h : {16, 32, 48}) {
            TextureSpec textureSpec = defaultTextureSpec;
            DoTest(textureSpec, textureSpec, {w, h, 1});
        }
    }
}

TEST_P(CopyTests_T2T, TextureMip) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, 1};

    for (unsigned int i = 1; i < 4; ++i) {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyLevel = i;
        textureSpec.levelCount = i + 1;

        DoTest(textureSpec, textureSpec, {kWidth >> i, kHeight >> i, 1});
    }
}

TEST_P(CopyTests_T2T, SingleMipSrcMultipleMipDst) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec defaultTextureSpec;

    for (unsigned int i = 1; i < 4; ++i) {
        TextureSpec srcTextureSpec = defaultTextureSpec;
        srcTextureSpec.textureSize = {kWidth >> i, kHeight >> i, 1};

        TextureSpec dstTextureSpec = defaultTextureSpec;
        dstTextureSpec.textureSize = {kWidth, kHeight, 1};
        dstTextureSpec.copyLevel = i;
        dstTextureSpec.levelCount = i + 1;

        DoTest(srcTextureSpec, dstTextureSpec, {kWidth >> i, kHeight >> i, 1});
    }
}

TEST_P(CopyTests_T2T, MultipleMipSrcSingleMipDst) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec defaultTextureSpec;

    for (unsigned int i = 1; i < 4; ++i) {
        TextureSpec srcTextureSpec = defaultTextureSpec;
        srcTextureSpec.textureSize = {kWidth, kHeight, 1};
        srcTextureSpec.copyLevel = i;
        srcTextureSpec.levelCount = i + 1;

        TextureSpec dstTextureSpec = defaultTextureSpec;
        dstTextureSpec.textureSize = {kWidth >> i, kHeight >> i, 1};

        DoTest(srcTextureSpec, dstTextureSpec, {kWidth >> i, kHeight >> i, 1});
    }
}

// Test that copying from one mip level to another mip level within the same 2D texture works.
TEST_P(CopyTests_T2T, Texture2DSameTextureDifferentMipLevels) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, 1};
    defaultTextureSpec.levelCount = 6;

    for (unsigned int i = 1; i < 6; ++i) {
        TextureSpec srcSpec = defaultTextureSpec;
        srcSpec.copyLevel = i - 1;
        TextureSpec dstSpec = defaultTextureSpec;
        dstSpec.copyLevel = i;

        DoTest(srcSpec, dstSpec, {kWidth >> i, kHeight >> i, 1}, true);
    }
}

// Test copying the whole 2D array texture.
TEST_P(CopyTests_T2T, Texture2DArrayFull) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 6u;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kLayers};

    DoTest(textureSpec, textureSpec, {kWidth, kHeight, kLayers});
}

// Test copying a subresource region of the 2D array texture.
TEST_P(CopyTests_T2T, Texture2DArrayRegion) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 6u;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, kLayers};

    for (unsigned int w : {64, 128, 256}) {
        for (unsigned int h : {16, 32, 48}) {
            TextureSpec textureSpec = defaultTextureSpec;
            DoTest(textureSpec, textureSpec, {w, h, kLayers});
        }
    }
}

// Test copying one slice of a 2D array texture.
TEST_P(CopyTests_T2T, Texture2DArrayCopyOneSlice) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 6u;
    constexpr uint32_t kSrcBaseLayer = 1u;
    constexpr uint32_t kDstBaseLayer = 3u;
    constexpr uint32_t kCopyArrayLayerCount = 1u;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, kLayers};

    TextureSpec srcTextureSpec = defaultTextureSpec;
    srcTextureSpec.copyOrigin = {0, 0, kSrcBaseLayer};

    TextureSpec dstTextureSpec = defaultTextureSpec;
    dstTextureSpec.copyOrigin = {0, 0, kDstBaseLayer};

    DoTest(srcTextureSpec, dstTextureSpec, {kWidth, kHeight, kCopyArrayLayerCount});
}

// Test copying multiple contiguous slices of a 2D array texture.
TEST_P(CopyTests_T2T, Texture2DArrayCopyMultipleSlices) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 6u;
    constexpr uint32_t kSrcBaseLayer = 0u;
    constexpr uint32_t kDstBaseLayer = 3u;
    constexpr uint32_t kCopyArrayLayerCount = 3u;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, kLayers};

    TextureSpec srcTextureSpec = defaultTextureSpec;
    srcTextureSpec.copyOrigin = {0, 0, kSrcBaseLayer};

    TextureSpec dstTextureSpec = defaultTextureSpec;
    dstTextureSpec.copyOrigin = {0, 0, kDstBaseLayer};

    DoTest(srcTextureSpec, dstTextureSpec, {kWidth, kHeight, kCopyArrayLayerCount});
}

// Test copying one texture slice within the same texture.
TEST_P(CopyTests_T2T, CopyWithinSameTextureOneSlice) {
    constexpr uint32_t kWidth = 256u;
    constexpr uint32_t kHeight = 128u;
    constexpr uint32_t kLayers = 6u;
    constexpr uint32_t kSrcBaseLayer = 0u;
    constexpr uint32_t kDstBaseLayer = 3u;
    constexpr uint32_t kCopyArrayLayerCount = 1u;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, kLayers};

    TextureSpec srcTextureSpec = defaultTextureSpec;
    srcTextureSpec.copyOrigin = {0, 0, kSrcBaseLayer};

    TextureSpec dstTextureSpec = defaultTextureSpec;
    dstTextureSpec.copyOrigin = {0, 0, kDstBaseLayer};

    DoTest(srcTextureSpec, dstTextureSpec, {kWidth, kHeight, kCopyArrayLayerCount}, true);
}

// Test copying multiple contiguous texture slices within the same texture with non-overlapped
// slices.
TEST_P(CopyTests_T2T, CopyWithinSameTextureNonOverlappedSlices) {
    constexpr uint32_t kWidth = 256u;
    constexpr uint32_t kHeight = 128u;
    constexpr uint32_t kLayers = 6u;
    constexpr uint32_t kSrcBaseLayer = 0u;
    constexpr uint32_t kDstBaseLayer = 3u;
    constexpr uint32_t kCopyArrayLayerCount = 3u;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, kLayers};

    TextureSpec srcTextureSpec = defaultTextureSpec;
    srcTextureSpec.copyOrigin = {0, 0, kSrcBaseLayer};

    TextureSpec dstTextureSpec = defaultTextureSpec;
    dstTextureSpec.copyOrigin = {0, 0, kDstBaseLayer};

    DoTest(srcTextureSpec, dstTextureSpec, {kWidth, kHeight, kCopyArrayLayerCount}, true);
}

// A regression test (from WebGPU CTS) for an Intel D3D12 driver bug about T2T copy with specific
// texture formats. See http://crbug.com/1161355 for more details.
TEST_P(CopyTests_T2T, CopyFromNonZeroMipLevelWithTexelBlockSizeLessThan4Bytes) {
    constexpr std::array<wgpu::TextureFormat, 11> kFormats = {
        {wgpu::TextureFormat::RG8Sint, wgpu::TextureFormat::RG8Uint, wgpu::TextureFormat::RG8Snorm,
         wgpu::TextureFormat::RG8Unorm, wgpu::TextureFormat::R16Float, wgpu::TextureFormat::R16Sint,
         wgpu::TextureFormat::R16Uint, wgpu::TextureFormat::R8Snorm, wgpu::TextureFormat::R8Unorm,
         wgpu::TextureFormat::R8Sint, wgpu::TextureFormat::R8Uint}};

    constexpr uint32_t kSrcLevelCount = 4;
    constexpr uint32_t kDstLevelCount = 5;
    constexpr uint32_t kSrcSize = 2 << kSrcLevelCount;
    constexpr uint32_t kDstSize = 2 << kDstLevelCount;
    ASSERT_LE(kSrcSize, kTextureBytesPerRowAlignment);
    ASSERT_LE(kDstSize, kTextureBytesPerRowAlignment);

    // The copyLayer to test:
    // 1u (non-array texture), 3u (copyLayer < copyWidth), 5u (copyLayer > copyWidth)
    constexpr std::array<uint32_t, 3> kTestTextureLayer = {1u, 3u, 5u};

    for (wgpu::TextureFormat format : kFormats) {
        // TODO(dawn:1877): Snorm copy failing ANGLE Swiftshader, need further investigation.
        if (IsANGLESwiftShader() &&
            (format == wgpu::TextureFormat::RG8Snorm || format == wgpu::TextureFormat::R8Snorm)) {
            continue;
        }

        for (uint32_t textureLayer : kTestTextureLayer) {
            const wgpu::Extent3D kUploadSize = {4u, 4u, textureLayer};

            for (uint32_t srcLevel = 0; srcLevel < kSrcLevelCount; ++srcLevel) {
                for (uint32_t dstLevel = 0; dstLevel < kDstLevelCount; ++dstLevel) {
                    TextureSpec srcSpec;
                    srcSpec.levelCount = kSrcLevelCount;
                    srcSpec.format = format;
                    srcSpec.copyLevel = srcLevel;
                    srcSpec.textureSize = {kSrcSize, kSrcSize, textureLayer};

                    TextureSpec dstSpec = srcSpec;
                    dstSpec.levelCount = kDstLevelCount;
                    dstSpec.copyLevel = dstLevel;
                    dstSpec.textureSize = {kDstSize, kDstSize, textureLayer};

                    DoTest(srcSpec, dstSpec, kUploadSize);
                }
            }

            // Resolve all the deferred expectations now to avoid allocating too much memory
            // in mDeferredExpectations.
            ResolveDeferredExpectationsNow();
        }
    }
}

// Test that copying from one mip level to another mip level within the same 2D array texture works.
TEST_P(CopyTests_T2T, Texture2DArraySameTextureDifferentMipLevels) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 8u;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, kLayers};
    defaultTextureSpec.levelCount = 6;

    for (unsigned int i = 1; i < 6; ++i) {
        TextureSpec srcSpec = defaultTextureSpec;
        srcSpec.copyLevel = i - 1;
        TextureSpec dstSpec = defaultTextureSpec;
        dstSpec.copyLevel = i;

        DoTest(srcSpec, dstSpec, {kWidth >> i, kHeight >> i, kLayers}, true);
    }
}

// Test that copying whole 1D texture in one texture-to-texture-copy works.
TEST_P(CopyTests_T2T, Texture1DFull) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 1;
    constexpr uint32_t kDepth = 1;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};

    DoTest(textureSpec, textureSpec, {kWidth, kHeight, kDepth}, false, wgpu::TextureDimension::e1D);
}

// Test that copying whole 3D texture in one texture-to-texture-copy works.
TEST_P(CopyTests_T2T, Texture3DFull) {
    // TODO(crbug.com/dawn/2294): diagnose T2B failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kDepth = 6u;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};

    DoTest(textureSpec, textureSpec, {kWidth, kHeight, kDepth}, false, wgpu::TextureDimension::e3D);
}

// Test that copying from one mip level to another mip level within the same 3D texture works.
TEST_P(CopyTests_T2T, Texture3DSameTextureDifferentMipLevels) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kDepth = 6u;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};
    textureSpec.levelCount = 2;

    TextureSpec dstSpec = textureSpec;
    dstSpec.copyLevel = 1;

    DoTest(textureSpec, dstSpec, {kWidth >> 1, kHeight >> 1, kDepth >> 1}, true,
           wgpu::TextureDimension::e3D);
}

// Test that copying whole 3D texture to a 2D array in one texture-to-texture-copy works.
TEST_P(CopyTests_T2T, Texture3DTo2DArrayFull) {
    // TODO(crbug.com/dawn/1425): Remove this suppression.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows() && IsIntel());

    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kDepth = 6u;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};

    DoTest(textureSpec, textureSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D,
           wgpu::TextureDimension::e2D);
}

// Test that copying between 3D texture and 2D array textures works. It includes partial copy
// for src and/or dst texture, non-zero offset (copy origin), non-zero mip level.
TEST_P(CopyTests_T2T, Texture3DAnd2DArraySubRegion) {
    // TODO(crbug.com/dawn/2294): diagnose T2B failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    // TODO(crbug.com/dawn/1426): Remove this suppression.
    DAWN_SUPPRESS_TEST_IF(IsANGLE() && IsWindows() && IsIntel());

    constexpr uint32_t kWidth = 8;
    constexpr uint32_t kHeight = 4;
    constexpr uint32_t kDepth = 2u;

    TextureSpec baseSpec;
    baseSpec.textureSize = {kWidth, kHeight, kDepth};

    TextureSpec srcSpec = baseSpec;
    TextureSpec dstSpec = baseSpec;

    // dst texture is a partial copy
    dstSpec.textureSize = {kWidth * 2, kHeight * 2, kDepth * 2};
    DoTest(srcSpec, dstSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D,
           wgpu::TextureDimension::e2D);
    DoTest(srcSpec, dstSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e2D,
           wgpu::TextureDimension::e3D);

    // src texture is a partial copy
    srcSpec.textureSize = {kWidth * 2, kHeight * 2, kDepth * 2};
    dstSpec = baseSpec;
    DoTest(srcSpec, dstSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D,
           wgpu::TextureDimension::e2D);
    DoTest(srcSpec, dstSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e2D,
           wgpu::TextureDimension::e3D);

    // Both src and dst texture is a partial copy
    srcSpec.textureSize = {kWidth * 2, kHeight * 2, kDepth * 2};
    dstSpec.textureSize = {kWidth * 2, kHeight * 2, kDepth * 2};
    DoTest(srcSpec, dstSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D,
           wgpu::TextureDimension::e2D);
    DoTest(srcSpec, dstSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e2D,
           wgpu::TextureDimension::e3D);

    // Non-zero offset (copy origin)
    srcSpec = baseSpec;
    dstSpec.textureSize = {kWidth * 2, kHeight * 2, kDepth * 2};
    dstSpec.copyOrigin = {kWidth, kHeight, kDepth};
    DoTest(srcSpec, dstSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D,
           wgpu::TextureDimension::e2D);
    DoTest(srcSpec, dstSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e2D,
           wgpu::TextureDimension::e3D);

    // Non-zero mip level
    srcSpec = baseSpec;
    dstSpec.textureSize = {kWidth * 2, kHeight * 2, kDepth * 2};
    dstSpec.copyOrigin = {0, 0, 0};
    dstSpec.copyLevel = 1;
    dstSpec.levelCount = 2;
    DoTest(srcSpec, dstSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D,
           wgpu::TextureDimension::e2D);
    DoTest(srcSpec, dstSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e2D,
           wgpu::TextureDimension::e3D);
}

// Test that copying whole 2D array to a 3D texture in one texture-to-texture-copy works.
TEST_P(CopyTests_T2T, Texture2DArrayTo3DFull) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kDepth = 6u;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};

    DoTest(textureSpec, textureSpec, {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e2D,
           wgpu::TextureDimension::e3D);
}

// Test that copying subregion of a 3D texture in one texture-to-texture-copy works.
TEST_P(CopyTests_T2T, Texture3DSubRegion) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kDepth = 6u;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};

    DoTest(textureSpec, textureSpec, {kWidth / 2, kHeight / 2, kDepth / 2}, false,
           wgpu::TextureDimension::e3D);
}

// Test that copying subregion of a 3D texture to a 2D array in one texture-to-texture-copy works.
TEST_P(CopyTests_T2T, Texture3DTo2DArraySubRegion) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kDepth = 6u;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};

    DoTest(textureSpec, textureSpec, {kWidth / 2, kHeight / 2, kDepth / 2},
           wgpu::TextureDimension::e3D, wgpu::TextureDimension::e2D);
}

// Test that copying subregion of a 2D array to a 3D texture to in one texture-to-texture-copy
// works.
TEST_P(CopyTests_T2T, Texture2DArrayTo3DSubRegion) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kDepth = 6u;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};

    DoTest(textureSpec, textureSpec, {kWidth / 2, kHeight / 2, kDepth / 2},
           wgpu::TextureDimension::e2D, wgpu::TextureDimension::e3D);
}

// Test that copying texture 3D array mips in one texture-to-texture-copy works
TEST_P(CopyTests_T2T, Texture3DMipAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kDepth = 64u;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, kDepth};

    for (unsigned int i = 1; i < 6; ++i) {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyLevel = i;
        textureSpec.levelCount = i + 1;

        DoTest(textureSpec, textureSpec, {kWidth >> i, kHeight >> i, kDepth >> i},
               wgpu::TextureDimension::e3D, wgpu::TextureDimension::e3D);
    }
}

// Test that copying texture 3D array mips in one texture-to-texture-copy works
TEST_P(CopyTests_T2T, Texture3DMipUnaligned) {
    constexpr uint32_t kWidth = 261;
    constexpr uint32_t kHeight = 123;
    constexpr uint32_t kDepth = 69u;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = {kWidth, kHeight, kDepth};

    for (unsigned int i = 1; i < 6; ++i) {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.copyLevel = i;
        textureSpec.levelCount = i + 1;

        DoTest(textureSpec, textureSpec, {kWidth >> i, kHeight >> i, kDepth >> i},
               wgpu::TextureDimension::e3D, wgpu::TextureDimension::e3D);
    }
}

// TODO(dawn:1705): enable this test for D3D11
DAWN_INSTANTIATE_TEST_P(
    CopyTests_T2T,
    {D3D12Backend(),
     D3D12Backend({"use_temp_buffer_in_small_format_texture_to_texture_copy_from_greater_to_less_"
                   "mip_level"}),
     D3D12Backend(
         {"d3d12_use_temp_buffer_in_texture_to_texture_copy_between_different_dimensions"}),
     MetalBackend(), OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
    {wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGB9E5Ufloat});

// Test copying between textures that have srgb compatible texture formats;
TEST_P(CopyTests_T2T_Srgb, FullCopy) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};
    DoTest(textureSpec, textureSpec, {kWidth, kHeight, 1});
}

DAWN_INSTANTIATE_TEST_P(CopyTests_T2T_Srgb,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), OpenGLBackend(),
                         OpenGLESBackend(), VulkanBackend()},
                        {wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8UnormSrgb,
                         wgpu::TextureFormat::BGRA8Unorm, wgpu::TextureFormat::BGRA8UnormSrgb});

static constexpr uint64_t kSmallBufferSize = 4;
static constexpr uint64_t kLargeBufferSize = 1 << 16;

// Test copying full buffers
TEST_P(CopyTests_B2B, FullCopy) {
    DoTest(kSmallBufferSize, 0, kSmallBufferSize, 0, kSmallBufferSize);
    DoTest(kLargeBufferSize, 0, kLargeBufferSize, 0, kLargeBufferSize);
}

// Test copying small pieces of a buffer at different corner case offsets
TEST_P(CopyTests_B2B, SmallCopyInBigBuffer) {
    constexpr uint64_t kEndOffset = kLargeBufferSize - kSmallBufferSize;
    DoTest(kLargeBufferSize, 0, kLargeBufferSize, 0, kSmallBufferSize);
    DoTest(kLargeBufferSize, kEndOffset, kLargeBufferSize, 0, kSmallBufferSize);
    DoTest(kLargeBufferSize, 0, kLargeBufferSize, kEndOffset, kSmallBufferSize);
    DoTest(kLargeBufferSize, kEndOffset, kLargeBufferSize, kEndOffset, kSmallBufferSize);
}

// Test zero-size copies
TEST_P(CopyTests_B2B, ZeroSizedCopy) {
    DoTest(kLargeBufferSize, 0, kLargeBufferSize, 0, 0);
    DoTest(kLargeBufferSize, 0, kLargeBufferSize, kLargeBufferSize, 0);
    DoTest(kLargeBufferSize, kLargeBufferSize, kLargeBufferSize, 0, 0);
    DoTest(kLargeBufferSize, kLargeBufferSize, kLargeBufferSize, kLargeBufferSize, 0);
}

DAWN_INSTANTIATE_TEST(CopyTests_B2B,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

// Test clearing full buffers
TEST_P(ClearBufferTests, FullClear) {
    DoTest(kSmallBufferSize, 0, kSmallBufferSize);
    DoTest(kLargeBufferSize, 0, kLargeBufferSize);
}

// Test clearing small pieces of a buffer at different corner case offsets
TEST_P(ClearBufferTests, SmallClearInBigBuffer) {
    constexpr uint64_t kEndOffset = kLargeBufferSize - kSmallBufferSize;
    DoTest(kLargeBufferSize, 0, kSmallBufferSize);
    DoTest(kLargeBufferSize, kSmallBufferSize, kSmallBufferSize);
    DoTest(kLargeBufferSize, kEndOffset, kSmallBufferSize);
}

// Test zero-size clears
TEST_P(ClearBufferTests, ZeroSizedClear) {
    DoTest(kLargeBufferSize, 0, 0);
    DoTest(kLargeBufferSize, kSmallBufferSize, 0);
    DoTest(kLargeBufferSize, kLargeBufferSize, 0);
}

DAWN_INSTANTIATE_TEST(ClearBufferTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

// Regression tests to reproduce a flaky failure when running whole WebGPU CTS on Intel GPUs.
// See crbug.com/dawn/1487 for more details.
namespace {
using TextureFormat = wgpu::TextureFormat;

enum class InitializationMethod {
    CopyBufferToTexture,
    WriteTexture,
    CopyTextureToTexture,
};

std::ostream& operator<<(std::ostream& o, InitializationMethod method) {
    switch (method) {
        case InitializationMethod::CopyBufferToTexture:
            o << "CopyBufferToTexture";
            break;
        case InitializationMethod::WriteTexture:
            o << "WriteTexture";
            break;
        case InitializationMethod::CopyTextureToTexture:
            o << "CopyTextureToTexture";
            break;
        default:
            DAWN_UNREACHABLE();
            break;
    }

    return o;
}

using AddRenderAttachmentUsage = bool;

DAWN_TEST_PARAM_STRUCT(CopyToDepthStencilTextureAfterDestroyingBigBufferTestsParams,
                       TextureFormat,
                       InitializationMethod,
                       AddRenderAttachmentUsage);

}  // anonymous namespace

class CopyToDepthStencilTextureAfterDestroyingBigBufferTests
    : public DawnTestWithParams<CopyToDepthStencilTextureAfterDestroyingBigBufferTestsParams> {};

TEST_P(CopyToDepthStencilTextureAfterDestroyingBigBufferTests, DoTest) {
    // Copies to stencil textures are unsupported on the OpenGL backend.
    DAWN_TEST_UNSUPPORTED_IF(GetParam().mTextureFormat == wgpu::TextureFormat::Stencil8 &&
                             (IsOpenGL() || IsOpenGLES()));

    wgpu::TextureFormat format = GetParam().mTextureFormat;

    const uint32_t texelBlockSize = utils::GetTexelBlockSizeInBytes(format);
    const uint32_t expectedDataSize = Align(texelBlockSize, 4u);

    // First, create a big buffer and fill some garbage data on DEFAULT heap.
    constexpr size_t kBigBufferSize = 159740u;
    constexpr uint8_t kGarbageData = 255u;
    std::array<uint8_t, kBigBufferSize> garbageData;
    garbageData.fill(kGarbageData);

    wgpu::Buffer bigBuffer =
        utils::CreateBufferFromData(device, garbageData.data(), garbageData.size(),
                                    wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);
    // Next, destroy the buffer. Its heap is still alive and contains the garbage data.
    bigBuffer.Destroy();

    // Ensure the underlying ID3D12Resource of bigBuffer is deleted.
    bool submittedWorkDone = false;
    queue.OnSubmittedWorkDone(wgpu::CallbackMode::AllowProcessEvents,
                              [&submittedWorkDone](wgpu::QueueWorkDoneStatus status) {
                                  EXPECT_EQ(status, wgpu::QueueWorkDoneStatus::Success);
                                  submittedWorkDone = true;
                              });
    while (!submittedWorkDone) {
        WaitABit();
    }

    // Then, create a small texture, which should be allocated on the heap that contains the
    // garbage data.
    wgpu::TextureDescriptor textureDescriptor = {};
    textureDescriptor.format = format;
    textureDescriptor.size = {1, 1, 1};
    textureDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    if (GetParam().mAddRenderAttachmentUsage) {
        textureDescriptor.usage |= wgpu::TextureUsage::RenderAttachment;
    }
    wgpu::Texture texture = device.CreateTexture(&textureDescriptor);

    // Finally, upload valid data into the texture and validate its contents.
    std::vector<uint8_t> expectedData(expectedDataSize);
    constexpr uint8_t kBaseValue = 204u;
    for (uint32_t i = 0; i < texelBlockSize; ++i) {
        expectedData[i] = static_cast<uint8_t>(i + kBaseValue);
    }

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::Extent3D copySize = {1, 1, 1};

    auto EncodeUploadDataToTexture = [this, copySize](wgpu::CommandEncoder encoder,
                                                      wgpu::Texture destinationTexture,
                                                      const std::vector<uint8_t>& expectedData) {
        wgpu::BufferDescriptor bufferDescriptor = {};
        bufferDescriptor.size = expectedData.size();
        bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;
        wgpu::Buffer uploadBuffer = device.CreateBuffer(&bufferDescriptor);
        bool done = false;
        uploadBuffer.MapAsync(wgpu::MapMode::Write, 0, static_cast<uint32_t>(expectedData.size()),
                              wgpu::CallbackMode::AllowProcessEvents,
                              [&done](wgpu::MapAsyncStatus status, wgpu::StringView) {
                                  ASSERT_EQ(wgpu::MapAsyncStatus::Success, status);
                                  done = true;
                              });
        while (!done) {
            WaitABit();
        }

        uint8_t* uploadData = static_cast<uint8_t*>(uploadBuffer.GetMappedRange());
        memcpy(uploadData, expectedData.data(), expectedData.size());
        uploadBuffer.Unmap();

        wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
            utils::CreateTexelCopyBufferInfo(uploadBuffer);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(destinationTexture);
        // (Off-topic) spot-test for defaulting of .aspect.
        texelCopyTextureInfo.aspect = wgpu::TextureAspect::Undefined;
        encoder.CopyBufferToTexture(&texelCopyBufferInfo, &texelCopyTextureInfo, &copySize);
    };

    switch (GetParam().mInitializationMethod) {
        case InitializationMethod::CopyBufferToTexture: {
            EncodeUploadDataToTexture(encoder, texture, expectedData);
            break;
        }
        case InitializationMethod::WriteTexture: {
            wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
                utils::CreateTexelCopyTextureInfo(texture);
            wgpu::TexelCopyBufferLayout layout;
            queue.WriteTexture(&texelCopyTextureInfo, expectedData.data(), texelBlockSize, &layout,
                               &copySize);
            break;
        }
        case InitializationMethod::CopyTextureToTexture: {
            wgpu::Texture stagingTexture = device.CreateTexture(&textureDescriptor);
            EncodeUploadDataToTexture(encoder, stagingTexture, expectedData);

            wgpu::TexelCopyTextureInfo imageCopyStagingTexture =
                utils::CreateTexelCopyTextureInfo(stagingTexture);
            wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
                utils::CreateTexelCopyTextureInfo(texture);
            encoder.CopyTextureToTexture(&imageCopyStagingTexture, &texelCopyTextureInfo,
                                         &copySize);
            break;
        }
        default:
            DAWN_UNREACHABLE();
            break;
    }

    wgpu::BufferDescriptor destinationBufferDescriptor = {};
    destinationBufferDescriptor.size = expectedDataSize;
    destinationBufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer destinationBuffer = device.CreateBuffer(&destinationBufferDescriptor);

    wgpu::TexelCopyTextureInfo texelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(texture);
    wgpu::TexelCopyBufferInfo imageCopyDestinationBuffer =
        utils::CreateTexelCopyBufferInfo(destinationBuffer);
    encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &imageCopyDestinationBuffer, &copySize);

    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    EXPECT_BUFFER_U8_RANGE_EQ(expectedData.data(), destinationBuffer, 0, expectedDataSize);
}

DAWN_INSTANTIATE_TEST_P(
    CopyToDepthStencilTextureAfterDestroyingBigBufferTests,
    {D3D11Backend(), D3D12Backend(),
     D3D12Backend({"d3d12_force_clear_copyable_depth_stencil_texture_on_creation"}), MetalBackend(),
     OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
    {wgpu::TextureFormat::Depth16Unorm, wgpu::TextureFormat::Stencil8},
    {InitializationMethod::CopyBufferToTexture, InitializationMethod::WriteTexture,
     InitializationMethod::CopyTextureToTexture},
    {true, false});

// A series of regression tests for an Intel D3D12 driver issue about creating textures with
// CreatePlacedResource(). See crbug.com/1237175 for more details.
class T2TCopyFromDirtyHeapTests : public DawnTest {
  public:
    void DoTest(uint32_t layerCount, uint32_t levelCount) {
        std::vector<uint32_t> expectedData;
        wgpu::Buffer uploadBuffer = GetUploadBufferAndExpectedData(&expectedData);

        // First, create colorTexture1 and colorTexture2 and fill data into them.
        wgpu::Texture colorTexture1 = Create2DTexture(kTextureSize, layerCount, levelCount);
        Initialize2DTexture(colorTexture1, layerCount, levelCount, uploadBuffer);
        wgpu::Texture colorTexture2 = Create2DTexture(kTextureSize, layerCount, levelCount);
        Initialize2DTexture(colorTexture2, layerCount, levelCount, uploadBuffer);

        // Next, destroy colorTexture1.
        colorTexture1.Destroy();

        // Ensure colorTexture1 has been destroyed on the backend.
        EnsureSubmittedWorkDone();
        // Call an empty queue.Submit to workaround crbug.com/dawn/833 where resources are not
        // recycled until the next serial.
        queue.Submit(0, nullptr);
        EnsureSubmittedWorkDone();

        // Then, try to create destinationTextures which should be allocated on the memory used by
        // colorTexture1 previously, copy data from colorTexture2 into each destinationTexture and
        // verify the data in destinationTexture.
        std::vector<wgpu::Texture> destinationTextures;

        for (uint32_t layer = 0; layer < layerCount; ++layer) {
            for (uint32_t level = 0; level < levelCount; ++level) {
                uint32_t textureSizeAtLevel = kTextureSize >> level;
                wgpu::Texture destinationTexture = Create2DTexture(textureSizeAtLevel, 1, 1);
                // Save all destinationTextures so that they won't be deleted during the test.
                destinationTextures.push_back(destinationTexture);

                CopyIntoStagingTextureAndVerifyTexelData(colorTexture2, level, layer,
                                                         destinationTexture, expectedData);
            }
        }
    }

    wgpu::Buffer GetUploadBufferAndExpectedData(std::vector<uint32_t>* expectedData) {
        const size_t kBufferSize =
            kBytesPerRow * (kTextureSize - 1) + kTextureSize * kBytesPerBlock;

        expectedData->resize(kBufferSize / sizeof(uint32_t));
        for (uint32_t y = 0; y < kTextureSize; ++y) {
            for (uint32_t x = 0; x < kTextureSize * (kBytesPerBlock / sizeof(uint32_t)); ++x) {
                uint32_t index = (kBytesPerRow / sizeof(uint32_t)) * y + x;
                (*expectedData)[index] = x + y * 1000;
            }
        }

        wgpu::BufferDescriptor uploadBufferDesc = {};
        uploadBufferDesc.size = kBufferSize;
        uploadBufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;
        uploadBufferDesc.mappedAtCreation = true;
        wgpu::Buffer uploadBuffer = device.CreateBuffer(&uploadBufferDesc);

        memcpy(uploadBuffer.GetMappedRange(), expectedData->data(), kBufferSize);
        uploadBuffer.Unmap();

        return uploadBuffer;
    }

    wgpu::Texture Create2DTexture(uint32_t textureSize, uint32_t layerCount, uint32_t levelCount) {
        wgpu::TextureDescriptor colorTextureDesc = {};
        colorTextureDesc.format = kFormat;
        colorTextureDesc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                                 wgpu::TextureUsage::RenderAttachment;
        colorTextureDesc.mipLevelCount = levelCount;
        colorTextureDesc.size = {textureSize, textureSize, layerCount};
        return device.CreateTexture(&colorTextureDesc);
    }

    void Initialize2DTexture(wgpu::Texture texture,
                             uint32_t layerCount,
                             uint32_t levelCount,
                             wgpu::Buffer uploadBuffer) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        for (uint32_t layer = 0; layer < layerCount; ++layer) {
            for (uint32_t level = 0; level < levelCount; ++level) {
                wgpu::TexelCopyBufferInfo uploadCopyBuffer =
                    utils::CreateTexelCopyBufferInfo(uploadBuffer, 0, kBytesPerRow, kTextureSize);
                wgpu::TexelCopyTextureInfo colorCopyTexture =
                    utils::CreateTexelCopyTextureInfo(texture, level, {0, 0, layer});

                wgpu::Extent3D copySize = {kTextureSize >> level, kTextureSize >> level, 1};
                encoder.CopyBufferToTexture(&uploadCopyBuffer, &colorCopyTexture, &copySize);
            }
        }
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);
    }

    void CopyIntoStagingTextureAndVerifyTexelData(wgpu::Texture sourceTexture,
                                                  uint32_t copyLevel,
                                                  uint32_t copyLayer,
                                                  wgpu::Texture stagingTexture,
                                                  const std::vector<uint32_t>& expectedData) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        // Copy from miplevel = copyLevel and arrayLayer = copyLayer of sourceTexture into
        // stagingTexture
        wgpu::TexelCopyTextureInfo colorCopyTexture =
            utils::CreateTexelCopyTextureInfo(sourceTexture, copyLevel, {0, 0, copyLayer});
        uint32_t stagingTextureSize = kTextureSize >> copyLevel;
        wgpu::Extent3D copySize = {stagingTextureSize, stagingTextureSize, 1};
        wgpu::TexelCopyTextureInfo stagingCopyTexture =
            utils::CreateTexelCopyTextureInfo(stagingTexture);
        encoder.CopyTextureToTexture(&colorCopyTexture, &stagingCopyTexture, &copySize);

        // Copy from stagingTexture into readback buffer. Note that we don't use EXPECT_BUFFER_xxx()
        // because the buffers with CopySrc | CopyDst may also be allocated on the same memory of
        // colorTexture1, then stagingTexture will not be able to be allocated at that piece of
        // memory, which is not what we intended to do.
        const size_t kBufferSize =
            kBytesPerRow * (stagingTextureSize - 1) + stagingTextureSize * kBytesPerBlock;
        wgpu::BufferDescriptor readbackBufferDesc = {};
        readbackBufferDesc.size = kBufferSize;
        readbackBufferDesc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer readbackBuffer = device.CreateBuffer(&readbackBufferDesc);
        wgpu::TexelCopyBufferInfo readbackCopyBuffer =
            utils::CreateTexelCopyBufferInfo(readbackBuffer, 0, kBytesPerRow, kTextureSize);
        encoder.CopyTextureToBuffer(&stagingCopyTexture, &readbackCopyBuffer, &copySize);

        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        // Check the data in readback buffer
        bool done = false;
        readbackBuffer.MapAsync(wgpu::MapMode::Read, 0, kBufferSize,
                                wgpu::CallbackMode::AllowProcessEvents,
                                [&done](wgpu::MapAsyncStatus status, wgpu::StringView) {
                                    ASSERT_EQ(wgpu::MapAsyncStatus::Success, status);
                                    done = true;
                                });
        while (!done) {
            WaitABit();
        }

        const uint32_t* readbackData =
            static_cast<const uint32_t*>(readbackBuffer.GetConstMappedRange());
        for (uint32_t y = 0; y < stagingTextureSize; ++y) {
            for (uint32_t x = 0; x < stagingTextureSize * (kBytesPerBlock / sizeof(uint32_t));
                 ++x) {
                uint32_t index = y * (kBytesPerRow / sizeof(uint32_t)) + x;
                ASSERT_EQ(expectedData[index], readbackData[index]);
            }
        }

        readbackBuffer.Destroy();
    }

    void EnsureSubmittedWorkDone() {
        bool submittedWorkDone = false;
        queue.OnSubmittedWorkDone(wgpu::CallbackMode::AllowProcessEvents,
                                  [&submittedWorkDone](wgpu::QueueWorkDoneStatus status) {
                                      EXPECT_EQ(status, wgpu::QueueWorkDoneStatus::Success);
                                      submittedWorkDone = true;
                                  });
        while (!submittedWorkDone) {
            WaitABit();
        }
    }

  private:
    const uint32_t kTextureSize = 63;
    const wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA32Uint;
    const uint32_t kBytesPerBlock = 16;
    const uint32_t kBytesPerRow =
        Align(kBytesPerBlock * kTextureSize, kTextureBytesPerRowAlignment);
};

TEST_P(T2TCopyFromDirtyHeapTests, From2DArrayTexture) {
    constexpr uint32_t kLayerCount = 7;
    constexpr uint32_t kLevelCount = 1;
    DoTest(kLayerCount, kLevelCount);
}

TEST_P(T2TCopyFromDirtyHeapTests, From2DMultiMipmapLevelTexture) {
    constexpr uint32_t kLayerCount = 1;
    constexpr uint32_t kLevelCount = 5;
    DoTest(kLayerCount, kLevelCount);
}

DAWN_INSTANTIATE_TEST(T2TCopyFromDirtyHeapTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
