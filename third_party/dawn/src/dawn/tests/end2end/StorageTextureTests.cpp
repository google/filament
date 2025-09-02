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

#include <string>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class StorageTextureTests : public DawnTest {
  protected:
    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        // Just copy all the limits, though all we really care about is
        // maxStorageBuffersInFragmentStage
        // maxStorageTexturesInFragmentStage
        // maxStorageBuffersInVertexStage
        // maxStorageTexturesInVertexStage
        supported.UnlinkedCopyTo(&required);
    }

  public:
    static void FillExpectedData(void* pixelValuePtr,
                                 wgpu::TextureFormat format,
                                 uint32_t x,
                                 uint32_t y,
                                 uint32_t depthOrArrayLayer) {
        const uint32_t pixelValue = 1 + x + kWidth * (y + kHeight * depthOrArrayLayer);
        DAWN_ASSERT(pixelValue <= 255u / 4);

        switch (format) {
            // 32-bit unsigned integer formats
            case wgpu::TextureFormat::R32Uint: {
                uint32_t* valuePtr = static_cast<uint32_t*>(pixelValuePtr);
                *valuePtr = pixelValue;
                break;
            }

            case wgpu::TextureFormat::RG32Uint: {
                uint32_t* valuePtr = static_cast<uint32_t*>(pixelValuePtr);
                valuePtr[0] = pixelValue;
                valuePtr[1] = pixelValue * 2;
                break;
            }

            case wgpu::TextureFormat::RGBA32Uint: {
                uint32_t* valuePtr = static_cast<uint32_t*>(pixelValuePtr);
                valuePtr[0] = pixelValue;
                valuePtr[1] = pixelValue * 2;
                valuePtr[2] = pixelValue * 3;
                valuePtr[3] = pixelValue * 4;
                break;
            }

            // 32-bit signed integer formats
            case wgpu::TextureFormat::R32Sint: {
                int32_t* valuePtr = static_cast<int32_t*>(pixelValuePtr);
                *valuePtr = static_cast<int32_t>(pixelValue);
                break;
            }

            case wgpu::TextureFormat::RG32Sint: {
                int32_t* valuePtr = static_cast<int32_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<int32_t>(pixelValue);
                valuePtr[1] = -static_cast<int32_t>(pixelValue);
                break;
            }

            case wgpu::TextureFormat::RGBA32Sint: {
                int32_t* valuePtr = static_cast<int32_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<int32_t>(pixelValue);
                valuePtr[1] = -static_cast<int32_t>(pixelValue);
                valuePtr[2] = static_cast<int32_t>(pixelValue * 2);
                valuePtr[3] = -static_cast<int32_t>(pixelValue * 2);
                break;
            }

            // 32-bit float formats
            case wgpu::TextureFormat::R32Float: {
                float_t* valuePtr = static_cast<float_t*>(pixelValuePtr);
                *valuePtr = static_cast<float_t>(pixelValue * 1.1f);
                break;
            }

            case wgpu::TextureFormat::RG32Float: {
                float_t* valuePtr = static_cast<float_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<float_t>(pixelValue * 1.1f);
                valuePtr[1] = -static_cast<float_t>(pixelValue * 2.2f);
                break;
            }

            case wgpu::TextureFormat::RGBA32Float: {
                float_t* valuePtr = static_cast<float_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<float_t>(pixelValue * 1.1f);
                valuePtr[1] = -static_cast<float_t>(pixelValue * 1.1f);
                valuePtr[2] = static_cast<float_t>(pixelValue * 2.2f);
                valuePtr[3] = -static_cast<float_t>(pixelValue * 2.2f);
                break;
            }

            // 16-bit float formats
            case wgpu::TextureFormat::R16Float: {
                uint16_t* valuePtr = static_cast<uint16_t*>(pixelValuePtr);
                *valuePtr = Float32ToFloat16(static_cast<float_t>(pixelValue));
                break;
            }

            case wgpu::TextureFormat::RG16Float: {
                uint16_t* valuePtr = static_cast<uint16_t*>(pixelValuePtr);
                valuePtr[0] = Float32ToFloat16(static_cast<float_t>(pixelValue));
                valuePtr[1] = Float32ToFloat16(-static_cast<float_t>(pixelValue));
                break;
            }

            case wgpu::TextureFormat::RGBA16Float: {
                uint16_t* valuePtr = static_cast<uint16_t*>(pixelValuePtr);
                valuePtr[0] = Float32ToFloat16(static_cast<float_t>(pixelValue));
                valuePtr[1] = Float32ToFloat16(-static_cast<float_t>(pixelValue));
                valuePtr[2] = Float32ToFloat16(static_cast<float_t>(pixelValue * 2));
                valuePtr[3] = Float32ToFloat16(-static_cast<float_t>(pixelValue * 2));
                break;
            }

            // 8-bit (normalized/non-normalized signed/unsigned integer) 4-component formats
            case wgpu::TextureFormat::RGBA8Unorm:
            case wgpu::TextureFormat::RGBA8Uint: {
                utils::RGBA8* valuePtr = static_cast<utils::RGBA8*>(pixelValuePtr);
                *valuePtr =
                    utils::RGBA8(pixelValue, pixelValue * 2, pixelValue * 3, pixelValue * 4);
                break;
            }

            case wgpu::TextureFormat::BGRA8Unorm: {
                utils::RGBA8* valuePtr = static_cast<utils::RGBA8*>(pixelValuePtr);
                *valuePtr =
                    utils::RGBA8(pixelValue * 3, pixelValue * 2, pixelValue, pixelValue * 4);
                break;
            }

            case wgpu::TextureFormat::RGBA8Snorm:
            case wgpu::TextureFormat::RGBA8Sint: {
                int8_t* valuePtr = static_cast<int8_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<int8_t>(pixelValue);
                valuePtr[1] = -static_cast<int8_t>(pixelValue);
                valuePtr[2] = static_cast<int8_t>(pixelValue) * 2;
                valuePtr[3] = -static_cast<int8_t>(pixelValue) * 2;
                break;
            }

            // 16-bit normalized/non-normalized unsigned/signed integer formats
            case wgpu::TextureFormat::R16Unorm:
            case wgpu::TextureFormat::R16Uint: {
                uint16_t* valuePtr = static_cast<uint16_t*>(pixelValuePtr);
                *valuePtr = static_cast<uint16_t>(pixelValue);
                break;
            }

            case wgpu::TextureFormat::RG16Unorm:
            case wgpu::TextureFormat::RG16Uint: {
                uint16_t* valuePtr = static_cast<uint16_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<uint16_t>(pixelValue);
                valuePtr[1] = static_cast<uint16_t>(pixelValue * 2);
                break;
            }

            case wgpu::TextureFormat::R16Snorm:
            case wgpu::TextureFormat::R16Sint: {
                int16_t* valuePtr = static_cast<int16_t*>(pixelValuePtr);
                *valuePtr = static_cast<int16_t>(pixelValue);
                break;
            }

            case wgpu::TextureFormat::RG16Snorm:
            case wgpu::TextureFormat::RG16Sint: {
                int16_t* valuePtr = static_cast<int16_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<int16_t>(pixelValue);
                valuePtr[1] = -static_cast<int16_t>(pixelValue);
                break;
            }

            case wgpu::TextureFormat::RGBA16Unorm:
            case wgpu::TextureFormat::RGBA16Uint: {
                uint16_t* valuePtr = static_cast<uint16_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<uint16_t>(pixelValue);
                valuePtr[1] = static_cast<uint16_t>(pixelValue * 2);
                valuePtr[2] = static_cast<uint16_t>(pixelValue * 3);
                valuePtr[3] = static_cast<uint16_t>(pixelValue * 4);
                break;
            }

            case wgpu::TextureFormat::RGBA16Snorm:
            case wgpu::TextureFormat::RGBA16Sint: {
                int16_t* valuePtr = static_cast<int16_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<int16_t>(pixelValue);
                valuePtr[1] = -static_cast<int16_t>(pixelValue);
                valuePtr[2] = static_cast<int16_t>(pixelValue * 2);
                valuePtr[3] = -static_cast<int16_t>(pixelValue * 2);
                break;
            }

            // 8-bit normalized/non-normalized unsigned/signed integer formats
            case wgpu::TextureFormat::R8Unorm:
            case wgpu::TextureFormat::R8Uint: {
                uint8_t* valuePtr = static_cast<uint8_t*>(pixelValuePtr);
                *valuePtr = pixelValue;
                break;
            }

            case wgpu::TextureFormat::RG8Unorm:
            case wgpu::TextureFormat::RG8Uint: {
                uint8_t* valuePtr = static_cast<uint8_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<uint8_t>(pixelValue);
                valuePtr[1] = static_cast<uint8_t>(pixelValue * 2);
                break;
            }

            case wgpu::TextureFormat::R8Snorm:
            case wgpu::TextureFormat::R8Sint: {
                int8_t* valuePtr = static_cast<int8_t*>(pixelValuePtr);
                *valuePtr = static_cast<int8_t>(pixelValue);
                break;
            }

            case wgpu::TextureFormat::RG8Snorm:
            case wgpu::TextureFormat::RG8Sint: {
                int8_t* valuePtr = static_cast<int8_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<int8_t>(pixelValue);
                valuePtr[1] = -static_cast<int8_t>(pixelValue);
                break;
            }

            case wgpu::TextureFormat::RGB10A2Uint: {
                uint32_t* valuePtr = static_cast<uint32_t*>(pixelValuePtr);
                uint32_t r = static_cast<uint32_t>(pixelValue) % 1024;
                uint32_t g = static_cast<uint32_t>(pixelValue * 2) % 1024;
                uint32_t b = static_cast<uint32_t>(pixelValue * 3) % 1024;
                uint32_t a = static_cast<uint32_t>(3) % 4;
                *valuePtr = (a << 30) | (b << 20) | (g << 10) | r;
                break;
            }

            case wgpu::TextureFormat::RGB10A2Unorm: {
                uint32_t* valuePtr = static_cast<uint32_t*>(pixelValuePtr);
                uint32_t r = static_cast<uint32_t>(pixelValue) % 1024;
                uint32_t g = static_cast<uint32_t>(pixelValue * 2) % 1024;
                uint32_t b = static_cast<uint32_t>(pixelValue * 3) % 1024;
                uint32_t a = static_cast<uint32_t>(3);
                *valuePtr = (a << 30) | (b << 20) | (g << 10) | r;
                break;
            }

            case wgpu::TextureFormat::RG11B10Ufloat: {
                uint32_t* valuePtr = static_cast<uint32_t*>(pixelValuePtr);

                auto MakeRG11B10 = [](uint32_t r, uint32_t g, uint32_t b) {
                    DAWN_ASSERT((r & 0x7FF) == r);
                    DAWN_ASSERT((g & 0x7FF) == g);
                    DAWN_ASSERT((b & 0x3FF) == b);
                    return r | g << 11 | b << 22;
                };

                constexpr uint32_t kFloat11One = 0x3C0;
                constexpr uint32_t kFloat10Zero = 0;

                *valuePtr = MakeRG11B10(kFloat11One, kFloat11One, kFloat10Zero);
                break;
            }

            default:
                DAWN_UNREACHABLE();
                break;
        }
    }

    std::string GetImageDeclaration(wgpu::TextureFormat format,
                                    std::string accessQualifier,
                                    wgpu::TextureViewDimension dimension,
                                    uint32_t binding) {
        std::ostringstream ostream;
        ostream << "@group(0) @binding(" << binding << ") " << "var storageImage" << binding
                << " : ";
        switch (dimension) {
            case wgpu::TextureViewDimension::e1D:
                ostream << "texture_storage_1d";
                break;
            case wgpu::TextureViewDimension::e2D:
                ostream << "texture_storage_2d";
                break;
            case wgpu::TextureViewDimension::e2DArray:
                ostream << "texture_storage_2d_array";
                break;
            case wgpu::TextureViewDimension::e3D:
                ostream << "texture_storage_3d";
                break;
            default:
                DAWN_UNREACHABLE();
                break;
        }
        ostream << "<" << utils::GetWGSLImageFormatQualifier(format) << ", ";
        ostream << accessQualifier << ">;";
        return ostream.str();
    }

    const char* GetExpectedPixelValue(wgpu::TextureFormat format) {
        switch (format) {
            // non-normalized unsigned integer formats
            case wgpu::TextureFormat::R8Uint:
            case wgpu::TextureFormat::R16Uint:
            case wgpu::TextureFormat::R32Uint:
                return "vec4u(u32(value), 0u, 0u, 1u)";

            case wgpu::TextureFormat::RG8Uint:
            case wgpu::TextureFormat::RG16Uint:
            case wgpu::TextureFormat::RG32Uint:
                return "vec4u(u32(value), u32(value) * 2u, 0u, 1u)";

            case wgpu::TextureFormat::RGB10A2Uint:
                return "vec4u(u32(value), u32(value) * 2u, u32(value) * 3u, 3u)";

            case wgpu::TextureFormat::RGBA8Uint:
            case wgpu::TextureFormat::RGBA16Uint:
            case wgpu::TextureFormat::RGBA32Uint:
                return "vec4u(u32(value), u32(value) * 2u, "
                       "u32(value) * 3u, u32(value) * 4u)";

            // non-normalized signed integer formats
            case wgpu::TextureFormat::R8Sint:
            case wgpu::TextureFormat::R16Sint:
            case wgpu::TextureFormat::R32Sint:
                return "vec4i(i32(value), 0, 0, 1)";

            case wgpu::TextureFormat::RG8Sint:
            case wgpu::TextureFormat::RG16Sint:
            case wgpu::TextureFormat::RG32Sint:
                return "vec4i(i32(value), -i32(value), 0, 1)";

            case wgpu::TextureFormat::RGBA8Sint:
            case wgpu::TextureFormat::RGBA16Sint:
            case wgpu::TextureFormat::RGBA32Sint:
                return "vec4i(i32(value), -i32(value), i32(value) * 2, -i32(value) * 2)";

            // float formats
            case wgpu::TextureFormat::R16Float:
                return "vec4f(f32(value), 0.0, 0.0, 1.0)";

            case wgpu::TextureFormat::RG16Float:
                return "vec4f(f32(value), -f32(value), 0.0, 1.0)";

            case wgpu::TextureFormat::R32Float:
                return "vec4f(f32(value) * 1.1, 0.0, 0.0, 1.0)";

            case wgpu::TextureFormat::RG32Float:
                return "vec4f(f32(value) * 1.1, -f32(value) * 2.2, 0.0, 1.0)";

            case wgpu::TextureFormat::RGBA16Float:
                return "vec4f(f32(value), -f32(value), "
                       "f32(value) * 2.0, -f32(value) * 2.0)";

            case wgpu::TextureFormat::RGBA32Float:
                return "vec4f(f32(value) * 1.1, -f32(value) * 1.1, "
                       "f32(value) * 2.2, -f32(value) * 2.2)";

            // normalized signed/unsigned integer formats
            case wgpu::TextureFormat::RGBA8Unorm:
            case wgpu::TextureFormat::BGRA8Unorm:
                return "vec4f(f32(value) / 255.0, f32(value) / 255.0 * 2.0, "
                       "f32(value) / 255.0 * 3.0, f32(value) / 255.0 * 4.0)";

            case wgpu::TextureFormat::RGBA8Snorm:
                return "vec4f(f32(value) / 127.0, -f32(value) / 127.0, "
                       "f32(value) * 2.0 / 127.0, -f32(value) * 2.0 / 127.0)";

            case wgpu::TextureFormat::R8Unorm:
                return "vec4f(f32(value) / 255.0, 0.0, 0.0, 1.0)";

            case wgpu::TextureFormat::R8Snorm:
                return "vec4f(f32(value) / 127.0, 0.0, 0.0, 1.0)";

            case wgpu::TextureFormat::RG8Unorm:
                return "vec4f(f32(value) / 255.0, f32(value) * 2.0 / 255.0, 0.0, 1.0)";

            case wgpu::TextureFormat::RG8Snorm:
                return "vec4f(f32(value) / 127.0, -f32(value) / 127.0, 0.0, 1.0)";

            case wgpu::TextureFormat::R16Unorm:
                return "vec4f(f32(value) / 65535.0, 0.0, 0.0, 1.0)";

            case wgpu::TextureFormat::R16Snorm:
                return "vec4f(f32(value) / 32767.0, 0.0, 0.0, 1.0)";

            case wgpu::TextureFormat::RG16Unorm:
                return "vec4f(f32(value) / 65535.0, f32(value) * 2.0 / 65535.0, 0.0, 1.0)";

            case wgpu::TextureFormat::RG16Snorm:
                return "vec4f(f32(value) / 32767.0, -f32(value) / 32767.0, 0.0, 1.0)";

            case wgpu::TextureFormat::RGBA16Unorm:
                return "vec4f(f32(value) / 65535.0, f32(value) * 2.0 / 65535.0, "
                       "f32(value) * 3.0 / 65535.0, f32(value) * 4.0 / 65535.0)";

            case wgpu::TextureFormat::RGBA16Snorm:
                return "vec4f(f32(value) / 32767.0, -f32(value) / 32767.0, "
                       "f32(value) * 2.0 / 32767.0, -f32(value) * 2.0 / 32767.0)";

            case wgpu::TextureFormat::RGB10A2Unorm:
                return "vec4f(f32(value) / 1023.0, f32(value) * 2.0 / 1023.0, "
                       "f32(value) * 3.0 / 1023.0, 1.0)";

            case wgpu::TextureFormat::RG11B10Ufloat:
                return "vec4f(1.0, 1.0, 0.0, 1.0)";

            default:
                DAWN_UNREACHABLE();
                break;
        }
    }

    const char* GetComparisonFunction(wgpu::TextureFormat format) {
        switch (format) {
            // non-normalized unsigned integer formats
            case wgpu::TextureFormat::R32Uint:
            case wgpu::TextureFormat::RG32Uint:
            case wgpu::TextureFormat::RGBA8Uint:
            case wgpu::TextureFormat::RGBA16Uint:
            case wgpu::TextureFormat::RGBA32Uint:
                return R"(
fn IsEqualTo(pixel : vec4u, expected : vec4u) -> bool {
  return all(pixel == expected);
})";

            // non-normalized signed integer formats
            case wgpu::TextureFormat::R32Sint:
            case wgpu::TextureFormat::RG32Sint:
            case wgpu::TextureFormat::RGBA8Sint:
            case wgpu::TextureFormat::RGBA16Sint:
            case wgpu::TextureFormat::RGBA32Sint:
                return R"(
fn IsEqualTo(pixel : vec4i, expected : vec4i) -> bool {
  return all(pixel == expected);
})";

            // float formats
            case wgpu::TextureFormat::R32Float:
            case wgpu::TextureFormat::RG32Float:
            case wgpu::TextureFormat::RGBA16Float:
            case wgpu::TextureFormat::RGBA32Float:
                return R"(
fn IsEqualTo(pixel : vec4f, expected : vec4f) -> bool {
  return all(pixel == expected);
})";

            // normalized signed/unsigned integer formats
            case wgpu::TextureFormat::RGBA8Unorm:
            case wgpu::TextureFormat::RGBA8Snorm:
            case wgpu::TextureFormat::R8Unorm:
                // On Windows Intel drivers the tests will fail if tolerance <= 0.00000001f.
                return R"(
fn IsEqualTo(pixel : vec4f, expected : vec4f) -> bool {
  let tolerance : f32 = 0.0000001;
  return all(abs(pixel - expected) < vec4f(tolerance, tolerance, tolerance, tolerance));
})";

            default:
                DAWN_UNREACHABLE();
                break;
        }

        return "";
    }

    const char* GetEnable(wgpu::TextureFormat format) {
        if (format == wgpu::TextureFormat::R8Unorm) {
            return "enable chromium_internal_graphite;";
        }
        return "";
    }

    std::string CommonWriteOnlyTestCode(
        const char* stage,
        wgpu::TextureFormat format,
        wgpu::TextureViewDimension dimension = wgpu::TextureViewDimension::e2D) {
        std::string componentFmt = utils::GetWGSLColorTextureComponentTypeStr(format);
        auto texelType = "vec4<" + componentFmt + ">";
        std::string sliceCount;
        std::string textureStore;
        std::string textureSize = "vec2i(textureDimensions(storageImage0).xy)";
        switch (dimension) {
            case wgpu::TextureViewDimension::e1D:
                sliceCount = "1";
                textureStore = "textureStore(storageImage0, x, expected)";
                textureSize = "vec2i(i32(textureDimensions(storageImage0)), 1)";
                break;
            case wgpu::TextureViewDimension::e2D:
                sliceCount = "1";
                textureStore = "textureStore(storageImage0, vec2i(x, y), expected)";
                break;
            case wgpu::TextureViewDimension::e2DArray:
                sliceCount = "i32(textureNumLayers(storageImage0))";
                textureStore = "textureStore(storageImage0, vec2i(x, y), slice, expected)";
                break;
            case wgpu::TextureViewDimension::e3D:
                sliceCount = "i32(textureDimensions(storageImage0).z)";
                textureStore = "textureStore(storageImage0, vec3i(x, y, slice), expected)";
                break;
            default:
                DAWN_UNREACHABLE();
                break;
        }
        const char* workgroupSize = !strcmp(stage, "compute") ? " @workgroup_size(1)" : "";
        const bool isFragment = strcmp(stage, "fragment") == 0;

        std::ostringstream ostream;
        ostream << GetEnable(format) << "\n";
        ostream << GetImageDeclaration(format, "write", dimension, 0) << "\n";
        ostream << "@" << stage << workgroupSize << "\n";
        ostream << "fn main() ";
        if (isFragment) {
            ostream << "-> @location(0) vec4f ";
        }
        ostream << "{\n";
        ostream << "  let size : vec2i = " << textureSize << ";\n";
        ostream << "  let sliceCount : i32 = " << sliceCount << ";\n";
        ostream << "  for (var slice : i32 = 0; slice < sliceCount; slice = slice + 1) {\n";
        ostream << "    for (var y : i32 = 0; y < size.y; y = y + 1) {\n";
        ostream << "      for (var x : i32 = 0; x < size.x; x = x + 1) {\n";
        ostream << "        var value : i32 = " << kComputeExpectedValue << ";\n";
        ostream << "        var expected : " << texelType << " = " << GetExpectedPixelValue(format)
                << ";\n";
        ostream << "        " << textureStore << ";\n";
        ostream << "      }\n";
        ostream << "    }\n";
        ostream << "  }\n";
        if (isFragment) {
            ostream << "return vec4f();\n";
        }
        ostream << "}\n";

        return ostream.str();
    }

    std::string CommonReadOnlyTestCode(const char* stage, wgpu::TextureFormat format) {
        wgpu::TextureViewDimension dimension = wgpu::TextureViewDimension::e2D;
        utils::WGSLComponentType componentType = utils::GetWGSLColorTextureComponentType(format);
        const bool isFragment = strcmp(stage, "fragment") == 0;
        std::string textureSize = "textureDimensions(storageImage0).xy";

        bool isIntegerComponent = (componentType == utils::WGSLComponentType::Int32 ||
                                   componentType == utils::WGSLComponentType::Uint32);
        std::string comparisonCode = isIntegerComponent
                                         ? "any(pixel != expected)"
                                         : "any(abs(pixel - expected) > vec4<f32>(0.001))";

        std::ostringstream ostream;
        ostream << GetEnable(format) << "\n";
        ostream << GetImageDeclaration(format, "read", dimension, 0) << "\n";
        ostream << "@" << stage << " fn main() ";
        if (isFragment) {
            ostream << "-> @location(0) vec4f ";
        }
        ostream << "{\n";
        ostream << "  let size = vec2i(" << textureSize << ");\n";
        ostream << "  for (var y = 0; y < size.y; y += 1) {\n";
        ostream << "    for (var x = 0; x < size.x; x += 1) {\n";
        ostream << "      let value = 1 + x + size.x * y;\n";
        ostream << "      let expected = " << GetExpectedPixelValue(format) << ";\n";
        ostream << "      let pixel = textureLoad(storageImage0, vec2i(x, y));\n";
        ostream << "      if (" << comparisonCode << ") {\n";
        ostream << "        return vec4f(1, 0, 0, 1);\n";
        ostream << "      }\n";
        ostream << "    }\n";
        ostream << "  }\n";
        ostream << "  return vec4f(0, 1, 0, 1);\n";
        ostream << "}\n";

        return ostream.str();
    }

    static std::vector<uint8_t> GetExpectedData(wgpu::TextureFormat format,
                                                uint32_t sliceCount = 1) {
        const uint32_t texelSizeInBytes = utils::GetTexelBlockSizeInBytes(format);

        std::vector<uint8_t> outputData(texelSizeInBytes * kWidth * kHeight * sliceCount);

        for (uint32_t i = 0; i < outputData.size() / texelSizeInBytes; ++i) {
            uint8_t* pixelValuePtr = &outputData[i * texelSizeInBytes];
            const uint32_t x = i % kWidth;
            const uint32_t y = (i % (kWidth * kHeight)) / kWidth;
            const uint32_t slice = i / (kWidth * kHeight);
            FillExpectedData(pixelValuePtr, format, x, y, slice);
        }

        return outputData;
    }

    wgpu::Texture CreateTexture(wgpu::TextureFormat format,
                                wgpu::TextureUsage usage,
                                const wgpu::Extent3D& size,
                                wgpu::TextureDimension dimension = wgpu::TextureDimension::e2D) {
        wgpu::TextureDescriptor descriptor;
        descriptor.size = size;
        descriptor.dimension = dimension;
        descriptor.format = format;
        descriptor.usage = usage;
        return device.CreateTexture(&descriptor);
    }

    wgpu::Texture CreateTextureWithTestData(
        const uint8_t* initialTextureData,
        size_t initialTextureDataSize,
        wgpu::TextureFormat format,
        wgpu::TextureViewDimension dimension = wgpu::TextureViewDimension::e2D) {
        uint32_t texelSize = utils::GetTexelBlockSizeInBytes(format);
        DAWN_ASSERT(kWidth * texelSize <= kTextureBytesPerRowAlignment);

        const uint32_t bytesPerTextureRow = texelSize * kWidth;
        const uint32_t sliceCount =
            static_cast<uint32_t>(initialTextureDataSize / texelSize / (kWidth * kHeight));
        const size_t uploadBufferSize =
            kTextureBytesPerRowAlignment * (kHeight * sliceCount - 1) + kWidth * bytesPerTextureRow;

        std::vector<uint8_t> uploadBufferData(uploadBufferSize);
        for (uint32_t slice = 0; slice < sliceCount; ++slice) {
            const size_t initialDataOffset = bytesPerTextureRow * kHeight * slice;
            for (size_t y = 0; y < kHeight; ++y) {
                for (size_t x = 0; x < bytesPerTextureRow; ++x) {
                    uint8_t data =
                        initialTextureData[initialDataOffset + bytesPerTextureRow * y + x];
                    size_t indexInUploadBuffer =
                        (kHeight * slice + y) * kTextureBytesPerRowAlignment + x;
                    uploadBufferData[indexInUploadBuffer] = data;
                }
            }
        }
        wgpu::Buffer uploadBuffer =
            utils::CreateBufferFromData(device, uploadBufferData.data(), uploadBufferSize,
                                        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

        wgpu::Texture outputTexture = CreateTexture(
            format,
            wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc |
                wgpu::TextureUsage::CopyDst,
            {kWidth, kHeight, sliceCount}, utils::ViewDimensionToTextureDimension(dimension));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        const wgpu::Extent3D copyExtent = {kWidth, kHeight, sliceCount};
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo = utils::CreateTexelCopyBufferInfo(
            uploadBuffer, 0, kTextureBytesPerRowAlignment, kHeight);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo;
        texelCopyTextureInfo.texture = outputTexture;
        encoder.CopyBufferToTexture(&texelCopyBufferInfo, &texelCopyTextureInfo, &copyExtent);

        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        return outputTexture;
    }

    wgpu::ComputePipeline CreateComputePipeline(const char* computeShader) {
        wgpu::ShaderModule csModule = utils::CreateShaderModule(device, computeShader);
        wgpu::ComputePipelineDescriptor computeDescriptor;
        computeDescriptor.layout = nullptr;
        computeDescriptor.compute.module = csModule;
        return device.CreateComputePipeline(&computeDescriptor);
    }

    wgpu::RenderPipeline CreateRenderPipeline(const char* vertexShader,
                                              const char* fragmentShader) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vertexShader);
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fragmentShader);

        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = vsModule;
        desc.cFragment.module = fsModule;
        desc.cTargets[0].format = kRenderAttachmentFormat;
        desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
        return device.CreateRenderPipeline(&desc);
    }

    void CheckDrawsGreen(const char* vertexShader,
                         const char* fragmentShader,
                         wgpu::Texture readonlyStorageTexture) {
        wgpu::RenderPipeline pipeline = CreateRenderPipeline(vertexShader, fragmentShader);
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, pipeline.GetBindGroupLayout(0), {{0, readonlyStorageTexture.CreateView()}});

        // Clear the render attachment to red at the beginning of the render pass.
        wgpu::Texture outputTexture = CreateTexture(
            kRenderAttachmentFormat,
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc, {1, 1});
        utils::ComboRenderPassDescriptor renderPassDescriptor({outputTexture.CreateView()});
        renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        renderPassDescriptor.cColorAttachments[0].clearValue = {1.f, 0.f, 0.f, 1.f};
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPassEncoder.SetBindGroup(0, bindGroup);
        renderPassEncoder.SetPipeline(pipeline);
        renderPassEncoder.Draw(1);
        renderPassEncoder.End();

        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        // Check if the contents in the output texture are all as expected (green).
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, outputTexture, 0, 0)
            << "\nVertex Shader:\n"
            << vertexShader << "\n\nFragment Shader:\n"
            << fragmentShader;
    }

    void CheckResultInStorageBuffer(
        wgpu::Texture readonlyStorageTexture,
        const std::string& computeShader,
        wgpu::TextureViewDimension dimension = wgpu::TextureViewDimension::e2D) {
        wgpu::ComputePipeline pipeline = CreateComputePipeline(computeShader.c_str());

        // Clear the content of the result buffer into 0.
        constexpr uint32_t kInitialValue = 0;
        wgpu::Buffer resultBuffer =
            utils::CreateBufferFromData(device, &kInitialValue, sizeof(kInitialValue),
                                        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);
        wgpu::TextureViewDescriptor descriptor;
        descriptor.dimension = dimension;
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, pipeline.GetBindGroupLayout(0),
            {{0, readonlyStorageTexture.CreateView(&descriptor)}, {1, resultBuffer}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computeEncoder = encoder.BeginComputePass();
        computeEncoder.SetBindGroup(0, bindGroup);
        computeEncoder.SetPipeline(pipeline);
        computeEncoder.DispatchWorkgroups(1);
        computeEncoder.End();

        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        // Check if the contents in the result buffer are what we expect.
        constexpr uint32_t kExpectedValue = 1u;
        EXPECT_BUFFER_U32_RANGE_EQ(&kExpectedValue, resultBuffer, 0, 1u);
    }

    void WriteIntoStorageTextureInRenderPass(wgpu::Texture writeonlyStorageTexture,
                                             const char* vertexShader,
                                             const char* fragmentShader) {
        // Create a render pipeline that writes the expected pixel values into the storage texture
        // without fragment shader outputs.
        wgpu::RenderPipeline pipeline = CreateRenderPipeline(vertexShader, fragmentShader);
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, pipeline.GetBindGroupLayout(0), {{0, writeonlyStorageTexture.CreateView()}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        wgpu::Texture placeholderOutputTexture = CreateTexture(
            kRenderAttachmentFormat,
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc, {1, 1});
        utils::ComboRenderPassDescriptor renderPassDescriptor(
            {placeholderOutputTexture.CreateView()});
        wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPassEncoder.SetBindGroup(0, bindGroup);
        renderPassEncoder.SetPipeline(pipeline);
        renderPassEncoder.Draw(1);
        renderPassEncoder.End();
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);
    }

    void WriteIntoStorageTextureInComputePass(
        wgpu::Texture writeonlyStorageTexture,
        const char* computeShader,
        wgpu::TextureViewDimension dimension = wgpu::TextureViewDimension::e2D) {
        // Create a compute pipeline that writes the expected pixel values into the storage texture.
        wgpu::TextureViewDescriptor descriptor;
        descriptor.dimension = dimension;
        wgpu::ComputePipeline pipeline = CreateComputePipeline(computeShader);
        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {{0, writeonlyStorageTexture.CreateView(&descriptor)}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = encoder.BeginComputePass();
        computePassEncoder.SetBindGroup(0, bindGroup);
        computePassEncoder.SetPipeline(pipeline);
        computePassEncoder.DispatchWorkgroups(1);
        computePassEncoder.End();
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);
    }

    void ReadWriteIntoStorageTextureInComputePass(
        wgpu::Texture readonlyStorageTexture,
        wgpu::Texture writeonlyStorageTexture,
        const char* computeShader,
        wgpu::TextureViewDimension dimension = wgpu::TextureViewDimension::e2D) {
        // Create a compute pipeline that writes the expected pixel values into the storage texture.
        wgpu::TextureViewDescriptor descriptor;
        descriptor.dimension = dimension;
        wgpu::ComputePipeline pipeline = CreateComputePipeline(computeShader);
        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {{0, writeonlyStorageTexture.CreateView(&descriptor)},
                                  {1, readonlyStorageTexture.CreateView(&descriptor)}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = encoder.BeginComputePass();
        computePassEncoder.SetBindGroup(0, bindGroup);
        computePassEncoder.SetPipeline(pipeline);
        computePassEncoder.DispatchWorkgroups(1);
        computePassEncoder.End();
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);
    }

    void CheckOutputStorageTexture(wgpu::Texture storageTexture,
                                   wgpu::TextureFormat format,
                                   const wgpu::Extent3D& size) {
        const std::vector<uint8_t>& expectedData = GetExpectedData(format, size.depthOrArrayLayers);
        CheckOutputStorageTexture(storageTexture, format, size, expectedData.data(),
                                  expectedData.size());
    }

    void CheckOutputStorageTexture(wgpu::Texture storageTexture,
                                   wgpu::TextureFormat format,
                                   const wgpu::Extent3D& size,
                                   const uint8_t* expectedData,
                                   size_t expectedDataSize) {
        CheckOutputStorageTexture(storageTexture, format, 0, size, expectedData, expectedDataSize);
    }

    void CheckOutputStorageTexture(wgpu::Texture storageTexture,
                                   wgpu::TextureFormat format,
                                   uint32_t mipLevel,
                                   const wgpu::Extent3D& size,
                                   const uint8_t* expectedData,
                                   size_t expectedDataSize) {
        // Copy the content from the write-only storage texture to the result buffer.
        wgpu::BufferDescriptor descriptor;
        descriptor.size =
            utils::RequiredBytesInCopy(kTextureBytesPerRowAlignment, size.height, size, format);
        descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer resultBuffer = device.CreateBuffer(&descriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
                utils::CreateTexelCopyTextureInfo(storageTexture, mipLevel, {0, 0, 0});
            wgpu::TexelCopyBufferInfo texelCopyBufferInfo = utils::CreateTexelCopyBufferInfo(
                resultBuffer, 0, kTextureBytesPerRowAlignment, size.height);
            encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &size);
        }
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        // Check if the contents in the result buffer are what we expect.
        uint32_t texelSize = utils::GetTexelBlockSizeInBytes(format);
        DAWN_ASSERT(size.width * texelSize <= kTextureBytesPerRowAlignment);

        for (size_t z = 0; z < size.depthOrArrayLayers; ++z) {
            for (size_t y = 0; y < size.height; ++y) {
                const size_t resultBufferOffset =
                    kTextureBytesPerRowAlignment * (size.height * z + y);
                const size_t expectedDataOffset = texelSize * size.width * (size.height * z + y);
                EXPECT_BUFFER_U32_RANGE_EQ(
                    reinterpret_cast<const uint32_t*>(expectedData + expectedDataOffset),
                    resultBuffer, resultBufferOffset, texelSize);
            }
        }
    }

    static constexpr size_t kWidth = 4u;
    static constexpr size_t kHeight = 4u;
    static constexpr wgpu::TextureFormat kRenderAttachmentFormat = wgpu::TextureFormat::RGBA8Unorm;

    const char* kSimpleVertexShader = R"(
;
@vertex fn main() -> @builtin(position) vec4f {
  return vec4f(0.0, 0.0, 0.0, 1.0);
})";

    const char* kComputeExpectedValue = "1 + x + size.x * (y + size.y * slice)";
};

// Test that write-only storage textures are supported in compute shader.
TEST_P(StorageTextureTests, WriteonlyStorageTextureInComputeShader) {
    for (wgpu::TextureFormat format : utils::kAllTextureFormats) {
        if (!utils::TextureFormatSupportsStorageTexture(format, device, IsCompatibilityMode())) {
            continue;
        }

        // TODO(dawn:1877): Snorm copy failing ANGLE Swiftshader, need further investigation.
        if (format == wgpu::TextureFormat::RGBA8Snorm && IsANGLESwiftShader()) {
            continue;
        }

        // Prepare the write-only storage texture.
        wgpu::Texture writeonlyStorageTexture =
            CreateTexture(format, wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc,
                          {kWidth, kHeight});

        // Write the expected pixel values into the write-only storage texture.
        const std::string computeShader = CommonWriteOnlyTestCode("compute", format);
        WriteIntoStorageTextureInComputePass(writeonlyStorageTexture, computeShader.c_str());

        // Verify the pixel data in the write-only storage texture is expected.
        CheckOutputStorageTexture(writeonlyStorageTexture, format, {kWidth, kHeight});
    }
}

// Test that write-only storage textures are supported in fragment shader.
TEST_P(StorageTextureTests, WriteonlyStorageTextureInFragmentShader) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageTexturesInFragmentStage < 1);

    for (wgpu::TextureFormat format : utils::kAllTextureFormats) {
        if (!utils::TextureFormatSupportsStorageTexture(format, device, IsCompatibilityMode())) {
            continue;
        }

        // TODO(dawn:1877): Snorm copy failing ANGLE Swiftshader, need further investigation.
        if (format == wgpu::TextureFormat::RGBA8Snorm && IsANGLESwiftShader()) {
            continue;
        }

        // TODO(dawn:1503): ANGLE OpenGL fails blit emulation path when texture is not copied
        // explicitly via the mUseCopy = true workaround path.
        if (format == wgpu::TextureFormat::RGBA8Snorm && IsANGLE() && IsWindows()) {
            continue;
        }

        // Prepare the write-only storage texture.
        wgpu::Texture writeonlyStorageTexture =
            CreateTexture(format, wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc,
                          {kWidth, kHeight});

        // Write the expected pixel values into the write-only storage texture.
        const std::string fragmentShader = CommonWriteOnlyTestCode("fragment", format);
        WriteIntoStorageTextureInRenderPass(writeonlyStorageTexture, kSimpleVertexShader,
                                            fragmentShader.c_str());

        // Verify the pixel data in the write-only storage texture is expected.
        CheckOutputStorageTexture(writeonlyStorageTexture, format, {kWidth, kHeight});
    }
}

// Verify 2D array and 3D write-only storage textures work correctly.
TEST_P(StorageTextureTests, Writeonly2DArrayOr3DStorageTexture) {
    constexpr uint32_t kSliceCount = 3u;

    constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::R32Uint;

    wgpu::TextureViewDimension dimensions[] = {
        wgpu::TextureViewDimension::e2DArray,
        wgpu::TextureViewDimension::e3D,
    };

    // Prepare the write-only storage texture.
    for (wgpu::TextureViewDimension dimension : dimensions) {
        wgpu::Texture writeonlyStorageTexture = CreateTexture(
            kTextureFormat, wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc,
            {kWidth, kHeight, kSliceCount}, utils::ViewDimensionToTextureDimension(dimension));

        // Write the expected pixel values into the write-only storage texture.
        const std::string computeShader =
            CommonWriteOnlyTestCode("compute", kTextureFormat, dimension);
        WriteIntoStorageTextureInComputePass(writeonlyStorageTexture, computeShader.c_str(),
                                             dimension);

        // Verify the pixel data in the write-only storage texture is expected.
        CheckOutputStorageTexture(writeonlyStorageTexture, kTextureFormat,
                                  {kWidth, kHeight, kSliceCount});
    }
}

// Verify 1D write-only storage textures work correctly.
TEST_P(StorageTextureTests, Writeonly1DStorageTexture) {
    constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::R32Uint;

    // Prepare the write-only storage texture.
    wgpu::Texture writeonlyStorageTexture = CreateTexture(
        kTextureFormat, wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc,
        {kWidth, 1, 1}, wgpu::TextureDimension::e1D);

    // Write the expected pixel values into the write-only storage texture.
    const std::string computeShader =
        CommonWriteOnlyTestCode("compute", kTextureFormat, wgpu::TextureViewDimension::e1D);
    WriteIntoStorageTextureInComputePass(writeonlyStorageTexture, computeShader.c_str(),
                                         wgpu::TextureViewDimension::e1D);

    // Verify the pixel data in the write-only storage texture is expected.
    CheckOutputStorageTexture(writeonlyStorageTexture, kTextureFormat, {kWidth, 1, 1});
}

// Test that multiple dispatches to increment values by ping-ponging between a sampled texture and
// a write-only storage texture are synchronized in one pass.
TEST_P(StorageTextureTests, SampledAndWriteonlyStorageTexturePingPong) {
    constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::R32Uint;
    wgpu::Texture storageTexture1 =
        CreateTexture(kTextureFormat,
                      wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding |
                          wgpu::TextureUsage::CopySrc,
                      {1u, 1u});
    wgpu::Texture storageTexture2 = CreateTexture(
        kTextureFormat, wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding,
        {1u, 1u});
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
@group(0) @binding(0) var Src : texture_2d<u32>;
@group(0) @binding(1) var Dst : texture_storage_2d<r32uint, write>;
@compute @workgroup_size(1) fn main() {
  var srcValue : vec4u = textureLoad(Src, vec2i(0, 0), 0);
  srcValue.x = srcValue.x + 1u;
  textureStore(Dst, vec2i(0, 0), srcValue);
}
    )");

    wgpu::ComputePipelineDescriptor pipelineDesc = {};
    pipelineDesc.compute.module = module;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    // In bindGroupA storageTexture1 is bound as read-only storage texture and storageTexture2 is
    // bound as write-only storage texture.
    wgpu::BindGroup bindGroupA = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                      {
                                                          {0, storageTexture1.CreateView()},
                                                          {1, storageTexture2.CreateView()},
                                                      });

    // In bindGroupA storageTexture2 is bound as read-only storage texture and storageTexture1 is
    // bound as write-only storage texture.
    wgpu::BindGroup bindGroupB = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                      {
                                                          {0, storageTexture2.CreateView()},
                                                          {1, storageTexture1.CreateView()},
                                                      });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);

    // After the first dispatch the value in storageTexture2 should be 1u.
    pass.SetBindGroup(0, bindGroupA);
    pass.DispatchWorkgroups(1);

    // After the second dispatch the value in storageTexture1 should be 2u;
    pass.SetBindGroup(0, bindGroupB);
    pass.DispatchWorkgroups(1);

    pass.End();

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(uint32_t);
    bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::TexelCopyTextureInfo texelCopyTextureInfo;
    texelCopyTextureInfo.texture = storageTexture1;

    wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
        utils::CreateTexelCopyBufferInfo(resultBuffer, 0, 256, 1);
    wgpu::Extent3D extent3D = {1, 1, 1};
    encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &extent3D);

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    constexpr uint32_t kFinalPixelValueInTexture1 = 2u;
    EXPECT_BUFFER_U32_EQ(kFinalPixelValueInTexture1, resultBuffer, 0);
}

DAWN_INSTANTIATE_TEST(StorageTextureTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class BGRA8UnormStorageTextureTests : public StorageTextureTests {
  public:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (SupportsFeatures({wgpu::FeatureName::BGRA8UnormStorage})) {
            mIsBGRA8UnormStorageSupported = true;
            return {wgpu::FeatureName::BGRA8UnormStorage};
        } else {
            mIsBGRA8UnormStorageSupported = false;
            return {};
        }
    }

    bool IsBGRA8UnormStorageSupported() { return mIsBGRA8UnormStorageSupported; }

  private:
    bool mIsBGRA8UnormStorageSupported = false;
};

// Test that BGRA8Unorm is supported to be used as storage texture in compute shaders when the
// optional feature 'bgra8unorm-storage' is supported.
TEST_P(BGRA8UnormStorageTextureTests, WriteonlyStorageTextureInComputeShader) {
    DAWN_TEST_UNSUPPORTED_IF(!IsBGRA8UnormStorageSupported());

    constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::BGRA8Unorm;
    wgpu::Texture writeonlyStorageTexture =
        CreateTexture(kFormat, wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc,
                      {kWidth, kHeight});

    // Write the expected pixel values into the write-only storage texture.
    const std::string computeShader = CommonWriteOnlyTestCode("compute", kFormat);
    WriteIntoStorageTextureInComputePass(writeonlyStorageTexture, computeShader.c_str());

    // Verify the pixel data in the write-only storage texture is expected.
    CheckOutputStorageTexture(writeonlyStorageTexture, kFormat, {kWidth, kHeight});
}

// Test that BGRA8Unorm is supported to be used as storage texture in fragment shaders when the
// optional feature 'bgra8unorm-storage' is supported.
TEST_P(BGRA8UnormStorageTextureTests, WriteonlyStorageTextureInFragmentShader) {
    DAWN_TEST_UNSUPPORTED_IF(!IsBGRA8UnormStorageSupported());

    constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::BGRA8Unorm;

    // Prepare the write-only storage texture.
    wgpu::Texture writeonlyStorageTexture =
        CreateTexture(kFormat, wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc,
                      {kWidth, kHeight});

    // Write the expected pixel values into the write-only storage texture.
    const std::string fragmentShader = CommonWriteOnlyTestCode("fragment", kFormat);
    WriteIntoStorageTextureInRenderPass(writeonlyStorageTexture, kSimpleVertexShader,
                                        fragmentShader.c_str());

    // Verify the pixel data in the write-only storage texture is expected.
    CheckOutputStorageTexture(writeonlyStorageTexture, kFormat, {kWidth, kHeight});
}

DAWN_INSTANTIATE_TEST(BGRA8UnormStorageTextureTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class R8UnormStorageTextureTests : public StorageTextureTests {
  public:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (SupportsFeatures({wgpu::FeatureName::R8UnormStorage})) {
            mIsR8UnormStorageSupported = true;
            return {wgpu::FeatureName::R8UnormStorage};
        } else {
            mIsR8UnormStorageSupported = false;
            return {};
        }
    }

    bool IsR8UnormStorageSupported() { return mIsR8UnormStorageSupported; }

  private:
    bool mIsR8UnormStorageSupported = false;
};

// Test that R8Unorm is supported to be used as storage texture in compute shaders when the
// optional feature 'r8unorm-storage' is supported.
TEST_P(R8UnormStorageTextureTests, WriteonlyStorageTextureInComputeShader) {
    DAWN_TEST_UNSUPPORTED_IF(!IsR8UnormStorageSupported());

    constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::R8Unorm;
    wgpu::Texture writeonlyStorageTexture =
        CreateTexture(kFormat, wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc,
                      {kWidth, kHeight});

    // Write the expected pixel values into the write-only storage texture.
    const std::string computeShader = CommonWriteOnlyTestCode("compute", kFormat);
    WriteIntoStorageTextureInComputePass(writeonlyStorageTexture, computeShader.c_str());

    // Verify the pixel data in the write-only storage texture is expected.
    CheckOutputStorageTexture(writeonlyStorageTexture, kFormat, {kWidth, kHeight});
}

// Test that R8Unorm is supported to be used as storage texture in fragment shaders when the
// optional feature 'r8unorm-storage' is supported.
TEST_P(R8UnormStorageTextureTests, WriteonlyStorageTextureInFragmentShader) {
    DAWN_TEST_UNSUPPORTED_IF(!IsR8UnormStorageSupported());

    constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::R8Unorm;

    // Prepare the write-only storage texture.
    wgpu::Texture writeonlyStorageTexture =
        CreateTexture(kFormat, wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc,
                      {kWidth, kHeight});

    // Write the expected pixel values into the write-only storage texture.
    const std::string fragmentShader = CommonWriteOnlyTestCode("fragment", kFormat);
    WriteIntoStorageTextureInRenderPass(writeonlyStorageTexture, kSimpleVertexShader,
                                        fragmentShader.c_str());

    // Verify the pixel data in the write-only storage texture is expected.
    CheckOutputStorageTexture(writeonlyStorageTexture, kFormat, {kWidth, kHeight});
}

DAWN_INSTANTIATE_TEST(R8UnormStorageTextureTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class StorageTextureZeroInitTests : public StorageTextureTests {
  public:
    static std::vector<uint8_t> GetExpectedData() {
        constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::R32Uint;

        const uint32_t texelSizeInBytes = utils::GetTexelBlockSizeInBytes(kTextureFormat);
        const size_t kDataCount = texelSizeInBytes * kWidth * kHeight;
        std::vector<uint8_t> outputData(kDataCount, 0);

        uint32_t* outputDataPtr = reinterpret_cast<uint32_t*>(&outputData[0]);
        *outputDataPtr = 1u;

        return outputData;
    }

    const char* kCommonReadOnlyZeroInitTestCode = R"(
fn doTest() -> bool {
  for (var y : i32 = 0; y < 4; y = y + 1) {
    for (var x : i32 = 0; x < 4; x = x + 1) {
      var pixel : vec4u = textureLoad(srcImage, vec2i(x, y));
      if (any(pixel != vec4u(0u, 0u, 0u, 1u))) {
        return false;
      }
    }
  }
  return true;
})";

    const char* kCommonWriteOnlyZeroInitTestCodeFragment = R"(
@group(0) @binding(0) var dstImage : texture_storage_2d<r32uint, write>;

@fragment fn main() -> @location(0) vec4f {
  textureStore(dstImage, vec2i(0, 0), vec4u(1u, 0u, 0u, 1u));
  return vec4f();
})";
    const char* kCommonWriteOnlyZeroInitTestCodeCompute = R"(
@group(0) @binding(0) var dstImage : texture_storage_2d<r32uint, write>;

@compute @workgroup_size(1) fn main() {
  textureStore(dstImage, vec2i(0, 0), vec4u(1u, 0u, 0u, 1u));
})";
};

// Verify that the texture is correctly cleared to 0 before its first usage as a write-only storage
// storage texture in a render pass.
TEST_P(StorageTextureZeroInitTests, WriteonlyStorageTextureClearsToZeroInRenderPass) {
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageTexturesInFragmentStage < 1);

    // Prepare the write-only storage texture.
    wgpu::Texture writeonlyStorageTexture = CreateTexture(
        wgpu::TextureFormat::R32Uint,
        wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc, {kWidth, kHeight});

    WriteIntoStorageTextureInRenderPass(writeonlyStorageTexture, kSimpleVertexShader,
                                        kCommonWriteOnlyZeroInitTestCodeFragment);
    std::vector<uint8_t> expectedData = GetExpectedData();
    CheckOutputStorageTexture(writeonlyStorageTexture, wgpu::TextureFormat::R32Uint,
                              {kWidth, kHeight}, expectedData.data(), expectedData.size());
}

// Verify that the texture is correctly cleared to 0 before its first usage as a write-only storage
// texture in a compute pass.
TEST_P(StorageTextureZeroInitTests, WriteonlyStorageTextureClearsToZeroInComputePass) {
    // Prepare the write-only storage texture.
    wgpu::Texture writeonlyStorageTexture = CreateTexture(
        wgpu::TextureFormat::R32Uint,
        wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc, {kWidth, kHeight});

    WriteIntoStorageTextureInComputePass(writeonlyStorageTexture,
                                         kCommonWriteOnlyZeroInitTestCodeCompute);
    std::vector<uint8_t> expectedData = GetExpectedData();
    CheckOutputStorageTexture(writeonlyStorageTexture, wgpu::TextureFormat::R32Uint,
                              {kWidth, kHeight}, expectedData.data(), expectedData.size());
}

DAWN_INSTANTIATE_TEST(StorageTextureZeroInitTests,
                      D3D11Backend({"nonzero_clear_resources_on_creation_for_testing"}),
                      D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"}),
                      OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"}),
                      OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"}),
                      MetalBackend({"nonzero_clear_resources_on_creation_for_testing"}),
                      VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"}));

class ReadWriteStorageTextureTests : public StorageTextureTests {
  protected:
    void RunReadWriteStorageTextureTest(wgpu::TextureFormat format) {
        SCOPED_TRACE(
            absl::StrFormat("Test format: %s", utils::GetWGSLImageFormatQualifier(format)));

        const std::vector<uint8_t> initialTextureData = GetExpectedData(format);
        utils::WGSLComponentType componentType = utils::GetWGSLColorTextureComponentType(format);

        wgpu::Texture readWriteStorageTexture =
            CreateTextureWithTestData(initialTextureData.data(), initialTextureData.size(), format);

        std::ostringstream sstream;
        std::string multiplyStatement;
        if (componentType == utils::WGSLComponentType::Float32) {
            multiplyStatement = "data1.x = data1.x + 2.0;\n";
        } else if (componentType == utils::WGSLComponentType::Int32) {
            multiplyStatement = "data1.x = data1.x + 2;\n";
        } else {
            multiplyStatement = "data1.x = data1.x + 2u;\n";
        }

        sstream << R"(
@group(0) @binding(0) var rwImage : texture_storage_2d<)"
                << utils::GetWGSLImageFormatQualifier(format) << R"(, read_write>;

@compute @workgroup_size()"
                << kWidth << ", " << kHeight << R"()
fn main(@builtin(local_invocation_id) local_id: vec3<u32>) {
  var data1 = textureLoad(rwImage, vec2i(local_id.xy));
)" << multiplyStatement
                << R"(  textureStore(rwImage, vec2i(local_id.xy), data1);
})";

        wgpu::ComputePipeline pipeline = CreateComputePipeline(sstream.str().c_str());
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, pipeline.GetBindGroupLayout(0), {{0, readWriteStorageTexture.CreateView()}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = encoder.BeginComputePass();
        computePassEncoder.SetBindGroup(0, bindGroup);
        computePassEncoder.SetPipeline(pipeline);
        computePassEncoder.DispatchWorkgroups(1);
        computePassEncoder.End();
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        std::vector<uint8_t> expectedModifiedData(initialTextureData.size());
        const uint32_t texelSizeInBytes = utils::GetTexelBlockSizeInBytes(format);

        for (uint32_t i = 0; i < kWidth * kHeight; ++i) {
            uint8_t* pixelValuePtr = &expectedModifiedData[i * texelSizeInBytes];
            const uint32_t x = i % kWidth;
            const uint32_t y = i / kWidth;
            FillExpectedData(pixelValuePtr, format, x, y, 0);

            if (componentType == utils::WGSLComponentType::Float32) {
                float* val = reinterpret_cast<float*>(pixelValuePtr);
                val[0] += 2.0f;
            } else if (componentType == utils::WGSLComponentType::Int32) {
                int32_t* val = reinterpret_cast<int32_t*>(pixelValuePtr);
                val[0] += 2;
            } else {  // u32
                uint32_t* val = reinterpret_cast<uint32_t*>(pixelValuePtr);
                val[0] += 2u;
            }
        }

        CheckOutputStorageTexture(readWriteStorageTexture, format, {kWidth, kHeight},
                                  reinterpret_cast<const uint8_t*>(expectedModifiedData.data()),
                                  expectedModifiedData.size() * sizeof(uint32_t));
    }
};

// Verify read-write storage texture can work correctly in compute shaders.
TEST_P(ReadWriteStorageTextureTests, ReadWriteStorageTextureInComputeShader) {
    RunReadWriteStorageTextureTest(wgpu::TextureFormat::R32Uint);
}

// Verify read-write storage texture can work correctly in fragment shaders.
TEST_P(ReadWriteStorageTextureTests, ReadWriteStorageTextureInFragmentShader) {
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageTexturesInFragmentStage < 1);

    std::array<uint32_t, kWidth * kHeight> inputData;
    std::array<uint32_t, kWidth * kHeight> expectedData;
    for (size_t i = 0; i < inputData.size(); ++i) {
        inputData[i] = i + 1;
        expectedData[i] = inputData[i] * 2;
    }

    wgpu::Texture readWriteStorageTexture = CreateTextureWithTestData(
        reinterpret_cast<const uint8_t*>(inputData.data()), inputData.size() * sizeof(uint32_t),
        wgpu::TextureFormat::R32Uint);

    wgpu::TextureDescriptor colorTextureDescriptor;
    colorTextureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    colorTextureDescriptor.size = {kWidth, kHeight, 1};
    colorTextureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture dummyColorTexture = device.CreateTexture(&colorTextureDescriptor);

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
 @vertex fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
    var pos = array(
        vec2f(-2.0, -2.0),
        vec2f(-2.0,  2.0),
        vec2f( 2.0, -2.0),
        vec2f(-2.0,  2.0),
        vec2f( 2.0, -2.0),
        vec2f( 2.0,  2.0));
    return vec4f(pos[VertexIndex], 0.0, 1.0);
})");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
@group(0) @binding(0) var rwImage : texture_storage_2d<r32uint, read_write>;
@fragment fn main(@builtin(position) fragcoord: vec4f) -> @location(0) vec4f {
    var data1 = textureLoad(rwImage, vec2i(fragcoord.xy));
    data1.x = data1.x * 2;
    textureStore(rwImage, vec2i(fragcoord.xy), data1);
    return vec4f(0.0, 1.0, 0.0, 1.0);
})");

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsModule;
    pipelineDescriptor.cTargets[0].format = colorTextureDescriptor.format;
    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                     {{0, readWriteStorageTexture.CreateView()}});

    utils::ComboRenderPassDescriptor renderPassDescriptor({dummyColorTexture.CreateView()});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPassDescriptor);
    renderPassEncoder.SetBindGroup(0, bindGroup);
    renderPassEncoder.SetPipeline(renderPipeline);
    renderPassEncoder.Draw(6);
    renderPassEncoder.End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    CheckOutputStorageTexture(readWriteStorageTexture, wgpu::TextureFormat::R32Uint,
                              {kWidth, kHeight},
                              reinterpret_cast<const uint8_t*>(expectedData.data()),
                              expectedData.size() * sizeof(uint32_t));
}

// Verify read-only storage texture can work correctly in compute shaders.
TEST_P(ReadWriteStorageTextureTests, ReadOnlyStorageTextureInComputeShader) {
    constexpr wgpu::TextureFormat kStorageTextureFormat = wgpu::TextureFormat::R32Uint;
    const std::vector<uint8_t> kInitialTextureData = GetExpectedData(kStorageTextureFormat);
    wgpu::Texture readonlyStorageTexture = CreateTextureWithTestData(
        kInitialTextureData.data(), kInitialTextureData.size(), kStorageTextureFormat);

    std::ostringstream sstream;
    sstream << R"(
@group(0) @binding(0) var srcImage : texture_storage_2d<r32uint, read>;
@group(0) @binding(1) var<storage, read_write> output : u32;

@compute @workgroup_size(1)
fn main() {
    for (var y = 0u; y < )"
            << kHeight << R"(; y++) {
        for (var x = 0u; x < )"
            << kWidth << R"(; x++) {
            var expected = vec4u(1u + x + y * 4u, 0, 0, 1u);
            var pixel = textureLoad(srcImage, vec2u(x, y));
            if (any(pixel != expected)) {
                output = 0u;
                return;
            }
        }
    }
    output = 1u;
})";
    uint32_t kInitialValue = 0xFF;
    wgpu::Buffer resultBuffer =
        utils::CreateBufferFromData(device, &kInitialValue, sizeof(kInitialValue),
                                    wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

    wgpu::ComputePipeline pipeline = CreateComputePipeline(sstream.str().c_str());
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                             {{0, readonlyStorageTexture.CreateView()}, {1, resultBuffer}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computePassEncoder = encoder.BeginComputePass();
    computePassEncoder.SetBindGroup(0, bindGroup);
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    // Check if the contents in the result buffer are what we expect.
    constexpr uint32_t kExpectedValue = 1u;
    EXPECT_BUFFER_U32_RANGE_EQ(&kExpectedValue, resultBuffer, 0, 1u);
}

// Verify read-only storage texture can work correctly in vertex shaders.
TEST_P(ReadWriteStorageTextureTests, ReadOnlyStorageTextureInVertexShader) {
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageTexturesInVertexStage < 1);

    constexpr wgpu::TextureFormat kStorageTextureFormat = wgpu::TextureFormat::R32Uint;
    const std::vector<uint8_t> kInitialTextureData = GetExpectedData(kStorageTextureFormat);
    wgpu::Texture readonlyStorageTexture = CreateTextureWithTestData(
        kInitialTextureData.data(), kInitialTextureData.size(), kStorageTextureFormat);

    std::ostringstream vsstream;
    vsstream << R"(
@group(0) @binding(0) var srcImage : texture_storage_2d<r32uint, read>;

struct VertexOutput {
    @builtin(position) pos: vec4f,
    @location(0) color: vec4f,
}

@vertex fn main() -> VertexOutput {
    var vertexOutput : VertexOutput;
    vertexOutput.pos = vec4f(0, 0, 0, 1);
    for (var y = 0u; y < )"
             << kHeight << R"(; y++) {
        for (var x = 0u; x < )"
             << kWidth << R"(; x++) {
            var expected = vec4u(1u + x + y * 4u, 0, 0, 1u);
            var pixel = textureLoad(srcImage, vec2u(x, y));
            if (any(pixel != expected)) {
                vertexOutput.color = vec4f(1, 0, 0, 1);
                return vertexOutput;
            }
        }
    }
    vertexOutput.color = vec4f(0, 1, 0, 1);
    return vertexOutput;
})";
    const char* kFragmentShader = R"(
struct FragmentInput {
    @location(0) color: vec4f,
}

@fragment fn main(fragmentInput : FragmentInput) -> @location(0) vec4f {
    return fragmentInput.color;
})";

    CheckDrawsGreen(vsstream.str().c_str(), kFragmentShader, readonlyStorageTexture);
}

// Verify read-only storage texture can work correctly in fragment shaders.
TEST_P(ReadWriteStorageTextureTests, ReadOnlyStorageTextureInFragmentShader) {
    DAWN_TEST_UNSUPPORTED_IF(GetSupportedLimits().maxStorageTexturesInFragmentStage < 1);

    constexpr wgpu::TextureFormat kStorageTextureFormat = wgpu::TextureFormat::R32Uint;
    const std::vector<uint8_t> kInitialTextureData = GetExpectedData(kStorageTextureFormat);
    wgpu::Texture readonlyStorageTexture = CreateTextureWithTestData(
        kInitialTextureData.data(), kInitialTextureData.size(), kStorageTextureFormat);

    std::ostringstream fsstream;
    fsstream << R"(
@group(0) @binding(0) var srcImage : texture_storage_2d<r32uint, read>;

@fragment fn main() -> @location(0) vec4f {
    for (var y = 0u; y < )"
             << kHeight << R"(; y++) {
        for (var x = 0u; x < )"
             << kWidth << R"(; x++) {
            var expected = vec4u(1u + x + y * 4u, 0, 0, 1u);
            var pixel = textureLoad(srcImage, vec2u(x, y));
            if (any(pixel != expected)) {
                return vec4f(1, 0, 0, 1);
            }
        }
    }
    return vec4f(0, 1, 0, 1);
})";

    CheckDrawsGreen(kSimpleVertexShader, fsstream.str().c_str(), readonlyStorageTexture);
}

// Verify using read-write storage texture access in pipeline layout is compatible with write-only
// storage texture access in shader.
TEST_P(ReadWriteStorageTextureTests, ReadWriteInPipelineLayoutAndWriteOnlyInShader) {
    constexpr wgpu::TextureFormat kStorageTextureFormat = wgpu::TextureFormat::R32Uint;
    std::array<uint32_t, kWidth * kHeight> expectedData;
    for (size_t i = 0; i < expectedData.size(); ++i) {
        expectedData[i] = i + 1;
    }

    wgpu::Texture storageTexture = CreateTexture(
        wgpu::TextureFormat::R32Uint,
        wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc, {kWidth, kHeight, 1});

    std::ostringstream sstream;
    sstream << R"(
@group(0) @binding(0) var rwImage : texture_storage_2d<r32uint, write>;

@compute @workgroup_size()"
            << kWidth << ", " << kHeight << R"()
fn main(
  @builtin(local_invocation_id) local_id: vec3u,
  @builtin(local_invocation_index) local_index : u32) {
  let data1 = vec4u(local_index + 1u, 0, 0, 1);
  textureStore(rwImage, vec2i(local_id.xy), data1);
})";

    wgpu::BindGroupLayout bindGroupLayout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::StorageTextureAccess::ReadWrite,
                  kStorageTextureFormat, wgpu::TextureViewDimension::e2D}});
    wgpu::ComputePipelineDescriptor computeDescriptor;
    computeDescriptor.layout = utils::MakePipelineLayout(device, {bindGroupLayout});
    computeDescriptor.compute.module = utils::CreateShaderModule(device, sstream.str().c_str());
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&computeDescriptor);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, bindGroupLayout, {{0, storageTexture.CreateView()}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computePassEncoder = encoder.BeginComputePass();
    computePassEncoder.SetBindGroup(0, bindGroup);
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    CheckOutputStorageTexture(storageTexture, wgpu::TextureFormat::R32Uint, {kWidth, kHeight},
                              reinterpret_cast<const uint8_t*>(expectedData.data()),
                              expectedData.size() * sizeof(uint32_t));
}

// Tests reading from mip level 0 of a mipLevelCount = 3 texture using a TEXTURE_BINDING
// and then writing to mip level 1 as STORAGE_BINDING. This surfaced an issue in the GL
// backend where the first usage sets TEXTURE_BASE_LEVEL and TEXTURE_MAX_LEVEL on the texture
// and they end up affecting the 2nd usage.
TEST_P(ReadWriteStorageTextureTests, ReadMipLevel0WriteMipLevel1) {
    // https://crbug.com/392121637
    DAWN_SUPPRESS_TEST_IF(IsOpenGL() || IsOpenGLES());

    wgpu::ShaderModule csModule = utils::CreateShaderModule(device, R"(
        @binding(0) @group(0) var<storage, read_write> buf : array<f32>;
        @binding(1) @group(0) var t_in: texture_2d<f32>;
        @binding(2) @group(0) var t_out: texture_storage_2d<rgba8unorm, write>;

        @compute @workgroup_size(1) fn csLoad() {
          // just make sure we actually read (don't want this optimized out)
          buf[0] = textureLoad(t_in, vec2u(0), 0).w;
        }
        @compute @workgroup_size(1) fn csStore() {
         textureStore(t_out, vec2u(0), vec4f(64, 128, 192, 255) / 255);
        }
    )");

    wgpu::ComputePipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.layout = nullptr;
    pipelineDescriptor.compute.module = csModule;

    pipelineDescriptor.compute.entryPoint = "csLoad";
    wgpu::ComputePipeline loadPipeline = device.CreateComputePipeline(&pipelineDescriptor);

    pipelineDescriptor.compute.entryPoint = "csStore";
    wgpu::ComputePipeline storePipeline = device.CreateComputePipeline(&pipelineDescriptor);

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 16;
    bufferDesc.usage = wgpu::BufferUsage::Storage;
    wgpu::Buffer storageBuffer = device.CreateBuffer(&bufferDesc);

    bufferDesc.size = 4;
    bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer resultBuffer0 = device.CreateBuffer(&bufferDesc);
    wgpu::Buffer resultBuffer1 = device.CreateBuffer(&bufferDesc);

    wgpu::TextureDescriptor textureDesc;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.size = {2, 1};
    textureDesc.mipLevelCount = 2;
    textureDesc.usage = wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::TextureBinding |
                        wgpu::TextureUsage::CopySrc;
    wgpu::Texture texture = device.CreateTexture(&textureDesc);

    wgpu::TextureViewDescriptor textureViewDesc1;
    textureViewDesc1.baseMipLevel = 0;
    textureViewDesc1.mipLevelCount = 1;
    wgpu::BindGroup loadBindGroup =
        utils::MakeBindGroup(device, loadPipeline.GetBindGroupLayout(0),
                             {{1, texture.CreateView(&textureViewDesc1)}, {0, storageBuffer}});

    wgpu::TextureViewDescriptor textureViewDesc2;
    textureViewDesc2.baseMipLevel = 1;
    textureViewDesc2.mipLevelCount = 1;
    wgpu::BindGroup storeBindGroup = utils::MakeBindGroup(
        device, storePipeline.GetBindGroupLayout(0), {{2, texture.CreateView(&textureViewDesc2)}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computeEncoder = encoder.BeginComputePass();

    computeEncoder.SetBindGroup(0, loadBindGroup);
    computeEncoder.SetPipeline(loadPipeline);
    computeEncoder.DispatchWorkgroups(1);

    computeEncoder.SetBindGroup(0, storeBindGroup);
    computeEncoder.SetPipeline(storePipeline);
    computeEncoder.DispatchWorkgroups(1);

    computeEncoder.End();

    {
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
            utils::CreateTexelCopyBufferInfo(resultBuffer0, 0, 256, 1);
        wgpu::Extent3D size({1, 1, 1});
        encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &size);
    }

    {
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(texture, 1, {0, 0, 0});
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
            utils::CreateTexelCopyBufferInfo(resultBuffer1, 0, 256, 1);
        wgpu::Extent3D size({1, 1, 1});
        encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &size);
    }

    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    static uint8_t expectedData0[]{0, 0, 0, 0};
    EXPECT_BUFFER_U8_RANGE_EQ(expectedData0, resultBuffer0, 0, 4);

    static uint8_t expectedData1[]{64, 128, 192, 255};
    EXPECT_BUFFER_U8_RANGE_EQ(expectedData1, resultBuffer1, 0, 4);
}

// Tests reading from both a TEXTURE_BINDING and a STORAGE_BINDING from the same
// texture at the same time. This test is to double check on a workaround for
// fixing the previous test above where in the GL backend we try to reset the
// TEXTURE_BASE_LEVEL and TEXTURE_MAX_LEVEL. If we mistakenly apply the workaround
// to read only textures then this test will fail.
TEST_P(ReadWriteStorageTextureTests, ReadMipLevel2AsBothTextureBindingAndStorageBinding) {
    // This asserts in TextureVK.cpp, see https://crbug.com/392121643
    DAWN_SUPPRESS_TEST_IF(IsVulkan());
    // https://crbug.com/392121648
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());

    wgpu::ShaderModule csModule = utils::CreateShaderModule(device, R"(
        @binding(0) @group(0) var<storage, read_write> buf : array<vec4u>;
        @binding(1) @group(0) var t_in: texture_2d<f32>;
        @binding(2) @group(0) var s_in: texture_storage_2d<rgba8unorm, read>;

        @compute @workgroup_size(1) fn cs() {
          buf[0] = vec4u(
            u32(textureLoad(t_in, vec2u(0), 0).r * 255),
            u32(textureLoad(s_in, vec2u(0)).r * 255),
            123,
            456,
          );
        }
    )");

    wgpu::ComputePipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.layout = nullptr;
    pipelineDescriptor.compute.module = csModule;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDescriptor);

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 16;
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer storageBuffer = device.CreateBuffer(&bufferDesc);

    // make a 3 mip level texture
    wgpu::TextureDescriptor textureDesc;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.size = {4, 1};
    textureDesc.mipLevelCount = 3;
    textureDesc.usage = wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::TextureBinding |
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    wgpu::Texture texture = device.CreateTexture(&textureDesc);

    // put 1 in first mip, 2 in 2nd, 3 in 3rd.
    for (uint32_t mipLevel = 0; mipLevel < 3; ++mipLevel) {
        uint32_t width = 4 >> mipLevel;
        uint32_t bytesPerRow = width * 4;
        wgpu::Extent3D copySize({width, 1, 1});
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(texture, mipLevel, {0, 0, 0});
        wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
            utils::CreateTexelCopyBufferLayout(0, bytesPerRow);
        std::vector<uint8_t> data(bytesPerRow, mipLevel + 1);
        queue.WriteTexture(&texelCopyTextureInfo, data.data(), bytesPerRow, &texelCopyBufferLayout,
                           &copySize);
    }

    // View mip level 2
    wgpu::TextureViewDescriptor textureViewDesc;
    textureViewDesc.baseMipLevel = 2;
    textureViewDesc.mipLevelCount = 1;
    wgpu::TextureView view = texture.CreateView(&textureViewDesc);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, storageBuffer}, {1, view}, {2, view}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computeEncoder = encoder.BeginComputePass();

    computeEncoder.SetBindGroup(0, bindGroup);
    computeEncoder.SetPipeline(pipeline);
    computeEncoder.DispatchWorkgroups(1);

    computeEncoder.End();

    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    // expect 3 from reading through the texture binding and
    // also 3 from reading through the storage binding.
    static uint32_t expectedData[]{3, 3, 123, 456};
    EXPECT_BUFFER_U32_RANGE_EQ(expectedData, storageBuffer, 0, 4);
}

// Tests reading from mip level 1 via TEXTURE_BINDING and write to mip level 2 via
// STORAGE_BINDING at the same time.
TEST_P(ReadWriteStorageTextureTests, ReadMipLevel1AndWriteLevel2AtTheSameTime) {
    // Compat mode doesn't support different views of the same texture
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    wgpu::ShaderModule csModule = utils::CreateShaderModule(device, R"(
        @binding(0) @group(0) var<storage, read_write> buf : array<vec4u>;
        @binding(1) @group(0) var t_in: texture_2d<f32>;
        @binding(2) @group(0) var s_out: texture_storage_2d<rgba8unorm, write>;

        @compute @workgroup_size(1) fn cs() {
          buf[0] = vec4u(textureLoad(t_in, vec2u(0), 0) * 255);
          textureStore(s_out, vec2u(0), vec4f(64, 128, 192, 255) / 255);
        }
    )");

    wgpu::ComputePipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.layout = nullptr;
    pipelineDescriptor.compute.module = csModule;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDescriptor);

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 16;
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer storageBuffer = device.CreateBuffer(&bufferDesc);

    bufferDesc.size = 16;
    bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bufferDesc);

    // make a 3 mip level texture
    wgpu::TextureDescriptor textureDesc;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.size = {4, 1};
    textureDesc.mipLevelCount = 3;
    textureDesc.usage = wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::TextureBinding |
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    wgpu::Texture texture = device.CreateTexture(&textureDesc);

    // put 1 in first mip, 2 in 2nd, 3 in 3rd.
    for (uint32_t mipLevel = 0; mipLevel < 3; ++mipLevel) {
        uint32_t width = 4 >> mipLevel;
        uint32_t bytesPerRow = width * 4;
        wgpu::Extent3D copySize({width, 1, 1});
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(texture, mipLevel, {0, 0, 0});
        wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
            utils::CreateTexelCopyBufferLayout(0, bytesPerRow);
        std::vector<uint8_t> data(bytesPerRow, mipLevel + 1);
        queue.WriteTexture(&texelCopyTextureInfo, data.data(), bytesPerRow, &texelCopyBufferLayout,
                           &copySize);
    }

    // View mip level 1
    wgpu::TextureViewDescriptor textureViewDesc;
    textureViewDesc.baseMipLevel = 1;
    textureViewDesc.mipLevelCount = 1;
    wgpu::TextureView viewL1 = texture.CreateView(&textureViewDesc);

    // View mip level 2
    textureViewDesc.baseMipLevel = 2;
    textureViewDesc.mipLevelCount = 1;
    wgpu::TextureView viewL2 = texture.CreateView(&textureViewDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(
        device, pipeline.GetBindGroupLayout(0), {{0, storageBuffer}, {1, viewL1}, {2, viewL2}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computeEncoder = encoder.BeginComputePass();

    computeEncoder.SetBindGroup(0, bindGroup);
    computeEncoder.SetPipeline(pipeline);
    computeEncoder.DispatchWorkgroups(1);

    computeEncoder.End();

    // copy a texel from mip level 2
    {
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(texture, 2, {0, 0, 0});
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
            utils::CreateTexelCopyBufferInfo(resultBuffer, 0, 256, 1);
        wgpu::Extent3D size({1, 1, 1});
        encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &size);
    }

    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    // expect 2 from reading mip level 1.
    {
        static uint32_t expectedData[]{2, 2, 2, 2};
        EXPECT_BUFFER_U32_RANGE_EQ(expectedData, storageBuffer, 0, 4);
    }

    // expect 0.25, 0.5, 0.75, 1 in mip level 2
    {
        static uint8_t expectedData[]{64, 128, 192, 255};
        EXPECT_BUFFER_U8_RANGE_EQ(expectedData, resultBuffer, 0, 4);
    }
}

// Test for crbug.com/417296309 which observed a failure on Apple Silicon.
// This ensures that we insert a memory fence in between reads and writes to a read-write storage
// texture to prevent reordering of memory operations within an invocation.
TEST_P(ReadWriteStorageTextureTests, ReadWriteStorageTexture_WriteAfterReadHazard) {
    // The texture dimensions need to be fairly large in order to reliably trigger a failure when
    // no fence is present.
    constexpr uint32_t kWidth = 1024;
    constexpr uint32_t kHeight = 1024;
    constexpr uint32_t kDepth = 64;

    wgpu::Texture readWriteStorageTexture =
        CreateTexture(wgpu::TextureFormat::R32Uint, wgpu::TextureUsage::StorageBinding,
                      {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(uint32_t);
    bufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    std::ostringstream sstream;
    sstream << R"(
@group(0) @binding(0) var rwImage : texture_storage_3d<r32uint, read_write>;
@group(0) @binding(1) var<storage, read_write> buffer : atomic<u32>;

// The reordering appears to be somewhat dependent on the value that is used in the condition.
const kSpecialValue = 42;

@compute @workgroup_size(4, 4, 4)
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
  // We read a value from the texture.
  // The texture is zero-initialized, so this value should be a zero.
  let value = textureLoad(rwImage, gid).x;

  // We then write a special value back to the texture.
  textureStore(rwImage, gid, vec4(kSpecialValue));

  // We then conditionally increment an atomic counter if the value that we read does not match the
  // special value that we just wrote.
  // This condition should be true for every single invocation, since they all read zero.
  if (value != kSpecialValue) {
    atomicAdd(&buffer, 1u);
  }
})";

    wgpu::ComputePipeline pipeline = CreateComputePipeline(sstream.str().c_str());
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, readWriteStorageTexture.CreateView()},
                                                         {1, buffer},
                                                     });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computePassEncoder = encoder.BeginComputePass();
    computePassEncoder.SetBindGroup(0, bindGroup);
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.DispatchWorkgroups(kWidth / 4, kHeight / 4, kDepth / 4);
    computePassEncoder.End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    // The counter should have be incremented once for every invocation.
    EXPECT_BUFFER_U32_EQ(kWidth * kHeight * kDepth, buffer, 0);
}

// Test related to crbug.com/417296309 which observed a failure on Apple Silicon.
// This ensures that we insert a memory fence in between writes and reads to a read-write storage
// texture to prevent reordering of memory operations within an invocation.
TEST_P(ReadWriteStorageTextureTests, ReadWriteStorageTexture_ReadAfterWriteHazard) {
    // The texture dimensions need to be fairly large in order to reliably trigger a failure when
    // no fence is present.
    constexpr uint32_t kWidth = 1024;
    constexpr uint32_t kHeight = 1024;
    constexpr uint32_t kDepth = 64;

    wgpu::Texture readWriteStorageTexture =
        CreateTexture(wgpu::TextureFormat::R32Uint, wgpu::TextureUsage::StorageBinding,
                      {kWidth, kHeight, kDepth}, wgpu::TextureDimension::e3D);

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(uint32_t);
    bufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    std::ostringstream sstream;
    sstream << R"(
@group(0) @binding(0) var rwImage : texture_storage_3d<r32uint, read_write>;
@group(0) @binding(1) var<storage, read_write> buffer : atomic<u32>;

// The reordering appears to be somewhat dependent on the value that is used in the condition.
const kSpecialValue = 42;

@compute @workgroup_size(4, 4, 4)
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
  // We write a special value back to the texture, which is zero-initialized.
  textureStore(rwImage, gid, vec4(kSpecialValue));

  // We then read a value from the texture.
  // This should be the special value that we just wrote.
  let value = textureLoad(rwImage, gid).x;

  // We then conditionally increment an atomic counter if the value that we read matches the special
  // value that we just wrote.
  // This condition should be true for every single invocation.
  if (value == kSpecialValue) {
    atomicAdd(&buffer, 1u);
  }
})";

    wgpu::ComputePipeline pipeline = CreateComputePipeline(sstream.str().c_str());
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, readWriteStorageTexture.CreateView()},
                                                         {1, buffer},
                                                     });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computePassEncoder = encoder.BeginComputePass();
    computePassEncoder.SetBindGroup(0, bindGroup);
    computePassEncoder.SetPipeline(pipeline);
    computePassEncoder.DispatchWorkgroups(kWidth / 4, kHeight / 4, kDepth / 4);
    computePassEncoder.End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    // The counter should have be incremented once for every invocation.
    EXPECT_BUFFER_U32_EQ(kWidth * kHeight * kDepth, buffer, 0);
}

DAWN_INSTANTIATE_TEST(ReadWriteStorageTextureTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      MetalBackend(),
                      VulkanBackend());

class Tier1StorageValidationTests : public StorageTextureTests {
  public:
    void SetUp() override {
        StorageTextureTests::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::TextureFormatsTier1}));
    }
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (SupportsFeatures({wgpu::FeatureName::TextureFormatsTier1})) {
            return {wgpu::FeatureName::TextureFormatsTier1};
        }
        return {};
    }
};

// Test that kTier1AdditionalStorageFormats formats have the "write-only" GPUStorageTextureAccess
//  capability if 'texture-formats-tier1' is enabled.
TEST_P(Tier1StorageValidationTests, WriteonlyStorageTextureInFragmentShader) {
    for (const auto format : utils::kTier1AdditionalStorageFormats) {
        SCOPED_TRACE(
            absl::StrFormat("Test format: %s", utils::GetWGSLImageFormatQualifier(format)));
        // Prepare the write-only storage texture.
        wgpu::Texture writeonlyStorageTexture =
            CreateTexture(format, wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc,
                          {kWidth, kHeight});

        // Write the expected pixel values into the write-only storage texture.
        const std::string fragmentShader = CommonWriteOnlyTestCode("fragment", format);
        WriteIntoStorageTextureInRenderPass(writeonlyStorageTexture, kSimpleVertexShader,
                                            fragmentShader.c_str());

        // Verify the pixel data in the write-only storage texture is expected.
        CheckOutputStorageTexture(writeonlyStorageTexture, format, {kWidth, kHeight});
    }
}

// Test that kTier1AdditionalStorageFormats formats have the "read-only" GPUStorageTextureAccess
//  capability if 'texture-formats-tier1' is enabled.
TEST_P(Tier1StorageValidationTests, ReadOnlyStorageTextureInFragmentShader) {
    for (const auto format : utils::kTier1AdditionalStorageFormats) {
        SCOPED_TRACE(
            absl::StrFormat("Test format: %s", utils::GetWGSLImageFormatQualifier(format)));
        wgpu::TextureFormat kStorageTextureFormat = format;
        const std::vector<uint8_t> kInitialTextureData = GetExpectedData(kStorageTextureFormat);
        wgpu::Texture readonlyStorageTexture = CreateTextureWithTestData(
            kInitialTextureData.data(), kInitialTextureData.size(), kStorageTextureFormat);

        const std::string fragmentShader = CommonReadOnlyTestCode("fragment", format);

        CheckDrawsGreen(kSimpleVertexShader, fragmentShader.c_str(), readonlyStorageTexture);
    }
}

DAWN_INSTANTIATE_TEST(Tier1StorageValidationTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      MetalBackend(),
                      VulkanBackend());

class Tier2StorageValidationTests : public ReadWriteStorageTextureTests {
  public:
    void SetUp() override {
        StorageTextureTests::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::TextureFormatsTier2}));
    }
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (SupportsFeatures({wgpu::FeatureName::TextureFormatsTier2})) {
            return {wgpu::FeatureName::TextureFormatsTier2};
        }
        return {};
    }
};

// Test that kTier2AdditionalIntStorageFormats support "read_write" GPUStorageTextureAccess
// in compute shaders when 'texture-formats-tier2' is enabled.
// TODO: Tests for r8unorm/rgba8unorm/r16float/rgba16float/rgba32float.
TEST_P(Tier2StorageValidationTests, ReadWriteStorageTextureInComputeShader) {
    for (const auto format : utils::kTier2AdditionalIntStorageFormats) {
        SCOPED_TRACE(
            absl::StrFormat("Test format: %s", utils::GetWGSLImageFormatQualifier(format)));
        RunReadWriteStorageTextureTest(format);
    }
}

DAWN_INSTANTIATE_TEST(Tier2StorageValidationTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
}  // anonymous namespace
}  // namespace dawn
