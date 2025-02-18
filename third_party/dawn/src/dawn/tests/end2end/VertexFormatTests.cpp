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

#include <limits>
#include <string>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

// Vertex format tests all work the same way: the test will render a triangle.
// Each test will set up a vertex buffer, and the vertex shader will check that
// the vertex content is the same as what we expected. On success it outputs green,
// otherwise red.

constexpr uint32_t kRTSize = 1;
constexpr uint32_t kVertexNum = 3;

std::vector<uint16_t> Float32ToFloat16(std::vector<float> data) {
    std::vector<uint16_t> expectedData;
    for (auto& element : data) {
        expectedData.push_back(dawn::Float32ToFloat16(element));
    }
    return expectedData;
}

template <typename destType, typename srcType>
std::vector<destType> BitCast(std::vector<srcType> data) {
    std::vector<destType> expectedData;
    for (auto& element : data) {
        expectedData.push_back(BitCast(element));
    }
    return expectedData;
}

class VertexFormatTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
    }

    utils::BasicRenderPass renderPass;

    bool IsNormalizedFormat(wgpu::VertexFormat format) {
        switch (format) {
            case wgpu::VertexFormat::Unorm8:
            case wgpu::VertexFormat::Unorm8x2:
            case wgpu::VertexFormat::Unorm8x4:
            case wgpu::VertexFormat::Unorm8x4BGRA:
            case wgpu::VertexFormat::Snorm8:
            case wgpu::VertexFormat::Snorm8x2:
            case wgpu::VertexFormat::Snorm8x4:
            case wgpu::VertexFormat::Unorm16:
            case wgpu::VertexFormat::Unorm16x2:
            case wgpu::VertexFormat::Unorm16x4:
            case wgpu::VertexFormat::Snorm16:
            case wgpu::VertexFormat::Snorm16x2:
            case wgpu::VertexFormat::Snorm16x4:
            case wgpu::VertexFormat::Unorm10_10_10_2:
                return true;
            default:
                return false;
        }
    }

    uint32_t NormalizationFactor(wgpu::VertexFormat format, uint32_t component) {
        switch (format) {
            case wgpu::VertexFormat::Unorm8:
            case wgpu::VertexFormat::Unorm8x2:
            case wgpu::VertexFormat::Unorm8x4:
            case wgpu::VertexFormat::Unorm8x4BGRA:
                return 255;
            case wgpu::VertexFormat::Snorm8:
            case wgpu::VertexFormat::Snorm8x2:
            case wgpu::VertexFormat::Snorm8x4:
                return 127;
            case wgpu::VertexFormat::Unorm16:
            case wgpu::VertexFormat::Unorm16x2:
            case wgpu::VertexFormat::Unorm16x4:
                return 65535;
            case wgpu::VertexFormat::Snorm16:
            case wgpu::VertexFormat::Snorm16x2:
            case wgpu::VertexFormat::Snorm16x4:
                return 32767;
            case wgpu::VertexFormat::Unorm10_10_10_2:
                return (component == 3) ? 3 : 1023;
            default:
                DAWN_UNREACHABLE();
        }
    }

    bool IsUnsignedFormat(wgpu::VertexFormat format) {
        switch (format) {
            case wgpu::VertexFormat::Uint8:
            case wgpu::VertexFormat::Uint8x2:
            case wgpu::VertexFormat::Uint8x4:
            case wgpu::VertexFormat::Uint16:
            case wgpu::VertexFormat::Uint16x2:
            case wgpu::VertexFormat::Uint16x4:
            case wgpu::VertexFormat::Uint32:
            case wgpu::VertexFormat::Uint32x2:
            case wgpu::VertexFormat::Uint32x3:
            case wgpu::VertexFormat::Uint32x4:
            case wgpu::VertexFormat::Unorm8:
            case wgpu::VertexFormat::Unorm8x2:
            case wgpu::VertexFormat::Unorm8x4:
            case wgpu::VertexFormat::Unorm8x4BGRA:
            case wgpu::VertexFormat::Unorm16:
            case wgpu::VertexFormat::Unorm16x2:
            case wgpu::VertexFormat::Unorm16x4:
            case wgpu::VertexFormat::Unorm10_10_10_2:
                return true;
            default:
                return false;
        }
    }

    bool IsFloatFormat(wgpu::VertexFormat format) {
        switch (format) {
            case wgpu::VertexFormat::Float16:
            case wgpu::VertexFormat::Float16x2:
            case wgpu::VertexFormat::Float16x4:
            case wgpu::VertexFormat::Float32:
            case wgpu::VertexFormat::Float32x2:
            case wgpu::VertexFormat::Float32x3:
            case wgpu::VertexFormat::Float32x4:
                return true;
            default:
                return false;
        }
    }

    bool IsHalfFormat(wgpu::VertexFormat format) {
        switch (format) {
            case wgpu::VertexFormat::Float16:
            case wgpu::VertexFormat::Float16x2:
            case wgpu::VertexFormat::Float16x4:
                return true;
            default:
                return false;
        }
    }

    uint32_t ByteSize(wgpu::VertexFormat format) {
        switch (format) {
            case wgpu::VertexFormat::Sint8:
            case wgpu::VertexFormat::Snorm8:
            case wgpu::VertexFormat::Uint8:
            case wgpu::VertexFormat::Unorm8:
                return 1;
            case wgpu::VertexFormat::Sint8x2:
            case wgpu::VertexFormat::Snorm8x2:
            case wgpu::VertexFormat::Uint8x2:
            case wgpu::VertexFormat::Unorm8x2:
            case wgpu::VertexFormat::Sint16:
            case wgpu::VertexFormat::Snorm16:
            case wgpu::VertexFormat::Uint16:
            case wgpu::VertexFormat::Unorm16:
            case wgpu::VertexFormat::Float16:
                return 2;
            case wgpu::VertexFormat::Float16x2:
            case wgpu::VertexFormat::Float32:
            case wgpu::VertexFormat::Unorm10_10_10_2:
            case wgpu::VertexFormat::Sint16x2:
            case wgpu::VertexFormat::Sint32:
            case wgpu::VertexFormat::Sint8x4:
            case wgpu::VertexFormat::Snorm16x2:
            case wgpu::VertexFormat::Snorm8x4:
            case wgpu::VertexFormat::Uint16x2:
            case wgpu::VertexFormat::Uint32:
            case wgpu::VertexFormat::Uint8x4:
            case wgpu::VertexFormat::Unorm16x2:
            case wgpu::VertexFormat::Unorm8x4:
            case wgpu::VertexFormat::Unorm8x4BGRA:
                return 4;
            case wgpu::VertexFormat::Float16x4:
            case wgpu::VertexFormat::Float32x2:
            case wgpu::VertexFormat::Sint16x4:
            case wgpu::VertexFormat::Sint32x2:
            case wgpu::VertexFormat::Snorm16x4:
            case wgpu::VertexFormat::Uint16x4:
            case wgpu::VertexFormat::Uint32x2:
            case wgpu::VertexFormat::Unorm16x4:
                return 8;
            case wgpu::VertexFormat::Float32x3:
            case wgpu::VertexFormat::Sint32x3:
            case wgpu::VertexFormat::Uint32x3:
                return 12;
            case wgpu::VertexFormat::Float32x4:
            case wgpu::VertexFormat::Sint32x4:
            case wgpu::VertexFormat::Uint32x4:
                return 16;
            default:
                DAWN_UNREACHABLE();
        }
    }

    uint32_t ComponentCount(wgpu::VertexFormat format) {
        switch (format) {
            case wgpu::VertexFormat::Uint8:
            case wgpu::VertexFormat::Sint8:
            case wgpu::VertexFormat::Unorm8:
            case wgpu::VertexFormat::Snorm8:
            case wgpu::VertexFormat::Float16:
            case wgpu::VertexFormat::Uint16:
            case wgpu::VertexFormat::Sint16:
            case wgpu::VertexFormat::Unorm16:
            case wgpu::VertexFormat::Snorm16:
            case wgpu::VertexFormat::Float32:
            case wgpu::VertexFormat::Uint32:
            case wgpu::VertexFormat::Sint32:
                return 1;
            case wgpu::VertexFormat::Uint8x2:
            case wgpu::VertexFormat::Sint8x2:
            case wgpu::VertexFormat::Unorm8x2:
            case wgpu::VertexFormat::Snorm8x2:
            case wgpu::VertexFormat::Uint16x2:
            case wgpu::VertexFormat::Sint16x2:
            case wgpu::VertexFormat::Unorm16x2:
            case wgpu::VertexFormat::Snorm16x2:
            case wgpu::VertexFormat::Float16x2:
            case wgpu::VertexFormat::Float32x2:
            case wgpu::VertexFormat::Uint32x2:
            case wgpu::VertexFormat::Sint32x2:
                return 2;
            case wgpu::VertexFormat::Float32x3:
            case wgpu::VertexFormat::Uint32x3:
            case wgpu::VertexFormat::Sint32x3:
                return 3;
            case wgpu::VertexFormat::Uint8x4:
            case wgpu::VertexFormat::Sint8x4:
            case wgpu::VertexFormat::Unorm8x4:
            case wgpu::VertexFormat::Unorm8x4BGRA:
            case wgpu::VertexFormat::Snorm8x4:
            case wgpu::VertexFormat::Uint16x4:
            case wgpu::VertexFormat::Sint16x4:
            case wgpu::VertexFormat::Unorm16x4:
            case wgpu::VertexFormat::Snorm16x4:
            case wgpu::VertexFormat::Float16x4:
            case wgpu::VertexFormat::Float32x4:
            case wgpu::VertexFormat::Uint32x4:
            case wgpu::VertexFormat::Sint32x4:
            case wgpu::VertexFormat::Unorm10_10_10_2:
                return 4;
            default:
                DAWN_UNREACHABLE();
        }
    }

    std::string ShaderTypeGenerator(bool isFloat,
                                    bool isNormalized,
                                    bool isUnsigned,
                                    uint32_t componentCount) {
        std::string base;
        if (isFloat || isNormalized) {
            base = "f32";
        } else if (isUnsigned) {
            base = "u32";
        } else {
            base = "i32";
        }

        if (componentCount == 1) {
            return base;
        }

        return "vec" + std::to_string(componentCount) + "<" + base + ">";
    }

    // The length of vertexData is fixed to 3, it aligns to triangle vertex number
    template <typename T>
    wgpu::RenderPipeline MakeTestPipeline(wgpu::VertexFormat format, std::vector<T>& expectedData) {
        bool isFloat = IsFloatFormat(format);
        bool isNormalized = IsNormalizedFormat(format);
        bool isUnsigned = IsUnsignedFormat(format);
        bool isInputTypeFloat = isFloat || isNormalized;
        bool isHalf = IsHalfFormat(format);
        const uint16_t kNegativeZeroInHalf = 0x8000;

        uint32_t componentCount = ComponentCount(format);

        std::string variableType =
            ShaderTypeGenerator(isFloat, isNormalized, isUnsigned, componentCount);
        std::string expectedDataType = ShaderTypeGenerator(isFloat, isNormalized, isUnsigned, 1);

        std::ostringstream vs;
        vs << "struct VertexIn {\n";
        vs << "    @location(0) test : " << variableType << ",\n";
        vs << "    @builtin(vertex_index) VertexIndex : u32,\n";
        vs << "}\n";

        // These functions map a 32-bit scalar value to its bits in u32, except
        // that the negative zero floating point value is remapped to u32(0),
        // which is also the bit representation for +0.0.
        // This is necessary because of the simple way we compute ULP.
        // Negative zero *equals* zero, so treat it as having the same bit
        // representation.
        vs << R"(
            fn rectify_u32(a: u32) -> u32 { return a; }
            fn rectify_i32(a: i32) -> u32 { return bitcast<u32>(a); }
            fn rectify_f32(a: f32) -> u32 {
              const negative_zero_bits = 1u << 31u;
              let b = bitcast<u32>(a);
              return select(b, 0u, b == negative_zero_bits);
            }
        )";

        // Because x86 CPU using "extended
        // precision"(https://en.wikipedia.org/wiki/Extended_precision) during float
        // math(https://developer.nvidia.com/sites/default/files/akamai/cuda/files/NVIDIA-CUDA-Floating-Point.pdf),
        // move normalization and Float16ToFloat32 into shader to generate
        // expected value.
        vs << R"(
            fn Float16ToFloat32(fp16 : u32) -> f32 {
                let magic : u32 = (254u - 15u) << 23u;
                let was_inf_nan : u32 = (127u + 16u) << 23u;
                var fp32u : u32 = (fp16 & 0x7FFFu) << 13u;
                let fp32 : f32 = bitcast<f32>(fp32u) * bitcast<f32>(magic);
                fp32u = bitcast<u32>(fp32);
                if (fp32 >= bitcast<f32>(was_inf_nan)) {
                    fp32u = fp32u | (255u << 23u);
                }
                fp32u = fp32u | ((fp16 & 0x8000u) << 16u);
                return bitcast<f32>(fp32u);
            }

            // NaN definition in IEEE 754-1985 is :
            //   - sign = either 0 or 1.
            //   - biased exponent = all 1 bits.
            //   - fraction = anything except all 0 bits (since all 0 bits represents infinity).
            // https://en.wikipedia.org/wiki/IEEE_754-1985#Representation_of_non-numbers
            fn isNaNCustom(val: f32) -> bool {
               let floatToUint: u32 = bitcast<u32>(val);
               return (floatToUint & 0x7fffffffu) > 0x7f800000u;
            }

            struct VertexOut {
                @location(0) color : vec4f,
                @builtin(position) position : vec4f,
            }

            @vertex
            fn main(input : VertexIn) -> VertexOut {
                var pos = array(
                    vec2f(-1.0, -1.0),
                    vec2f( 2.0,  0.0),
                    vec2f( 0.0,  2.0));
                var output : VertexOut;
                output.position = vec4f(pos[input.VertexIndex], 0.0, 1.0);
        )";

        // Declare expected values.
        vs << "var expected : array<array<" << expectedDataType << ", "
           << std::to_string(componentCount) << ">, " << std::to_string(kVertexNum) << ">;\n";
        // Assign each elements in expected values
        // e.g. expected[0][0] = u32(1u);
        //      expected[0][1] = u32(2u);
        for (uint32_t i = 0; i < kVertexNum; ++i) {
            for (uint32_t j = 0; j < componentCount; ++j) {
                vs << "    expected[" + std::to_string(i) + "][" + std::to_string(j) + "] = "
                   << expectedDataType << "(";
                if (isInputTypeFloat &&
                    std::isnan(static_cast<float>(expectedData[i * componentCount + j]))) {
                    // Set NaN.
                    vs << "0.0 / 0.0);\n";
                } else if (isNormalized) {
                    // Move normalize operation into shader because of CPU and GPU precision
                    // different on float math.
                    vs << "max(f32(" << std::to_string(expectedData[i * componentCount + j])
                       << ") / " << std::to_string(NormalizationFactor(format, j)) << ", -1.0));\n";
                } else if (isHalf) {
                    // Because Vulkan and D3D12 handle -0.0f through bitcast have different
                    // result (Vulkan take -0.0f as -0.0 but D3D12 take -0.0f as 0), add workaround
                    // for -0.0f.
                    // TODO(dawn:1566) Since rectify_32 will be used, we might
                    // not need this.
                    if (static_cast<uint16_t>(expectedData[i * componentCount + j]) ==
                        kNegativeZeroInHalf) {
                        vs << "-0.0);\n";
                    } else {
                        vs << "Float16ToFloat32(u32("
                           << std::to_string(expectedData[i * componentCount + j]) << ")));\n";
                    }
                } else if (isUnsigned) {
                    vs << std::to_string(expectedData[i * componentCount + j]) << "u);\n";
                } else {
                    vs << std::to_string(expectedData[i * componentCount + j]) << ");\n";
                }
            }
        }

        vs << "    var success : bool = true;\n";
        // Perform the checks by successively ANDing a boolean
        for (uint32_t component = 0; component < componentCount; ++component) {
            std::string suffix = componentCount == 1 ? "" : "[" + std::to_string(component) + "]";
            std::string testVal = "testVal" + std::to_string(component);
            std::string expectedVal = "expectedVal" + std::to_string(component);
            vs << "    var " << testVal << " : " << expectedDataType << ";\n";
            vs << "    var " << expectedVal << " : " << expectedDataType << ";\n";
            vs << "    " << testVal << " = input.test" << suffix << ";\n";
            vs << "    " << expectedVal << " = expected[input.VertexIndex]" << "[" << component
               << "];\n";
            if (!isInputTypeFloat) {  // Integer / unsigned integer need to match exactly.
                vs << "    success = success && (" << testVal << " == " << expectedVal << ");\n";
            } else {
                vs << "    success = success && (isNaNCustom(" << expectedVal << ") == isNaNCustom("
                   << testVal << "));\n";
                vs << "    if (!isNaNCustom(" << expectedVal << ")) {\n";
                // TODO(shaobo.yan@intel.com) : a difference of 8 ULPs is allowed in this test
                // because it is required on MacbookPro 11.5,AMD Radeon HD 8870M(on macOS 10.13.6),
                // but that it might be possible to tighten.
                vs << "        let testValFloatToUint : u32 = rectify_" << expectedDataType << "("
                   << testVal << ");\n";
                vs << "        let expectedValFloatToUint : u32 = rectify_" << expectedDataType
                   << "(" << expectedVal << ");\n";
                vs << "        success = success && max(testValFloatToUint, "
                      "expectedValFloatToUint)";
                vs << "        - min(testValFloatToUint, expectedValFloatToUint) < 8u;\n";
                vs << "    }\n";
            }
        }
        vs << R"(
            if (success) {
                output.color = vec4f(0.0, 1.0, 0.0, 1.0);
            } else {
                output.color = vec4f(1.0, 0.0, 0.0, 1.0);
            }
            return output;
        })";

        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vs.str().c_str());
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
                @fragment
                fn main(@location(0) color : vec4f) -> @location(0) vec4f {
                    return color;
                })");

        uint32_t strideBytes = Align(ByteSize(format), 4);

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.vertex.bufferCount = 1;
        descriptor.cBuffers[0].arrayStride = strideBytes;
        descriptor.cBuffers[0].attributeCount = 1;
        descriptor.cAttributes[0].format = format;
        descriptor.cTargets[0].format = renderPass.colorFormat;

        return device.CreateRenderPipeline(&descriptor);
    }

    template <typename VertexType, typename ExpectedType>
    void DoVertexFormatTest(wgpu::VertexFormat format,
                            std::vector<VertexType> vertex,
                            std::vector<ExpectedType> expectedData) {
        wgpu::RenderPipeline pipeline = MakeTestPipeline(format, expectedData);
        wgpu::Buffer vertexBuffer = utils::CreateBufferFromData(
            device, vertex.data(), vertex.size() * sizeof(VertexType), wgpu::BufferUsage::Vertex);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetVertexBuffer(0, vertexBuffer);
            pass.Draw(3);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 0, 0);
    }
};

TEST_P(VertexFormatTest, Uint8) {
    std::vector<uint8_t> vertexData = {
        std::numeric_limits<uint8_t>::max(),
        0,  // padding three bytes for stride
        0,
        0,
        std::numeric_limits<uint8_t>::min(),
        0,  // padding three bytes for stride
        0,
        0,
        200,
        0,  // padding three bytes for stride
        0,
        0,
    };

    std::vector<uint8_t> expectedData = {
        std::numeric_limits<uint8_t>::max(),
        std::numeric_limits<uint8_t>::min(),
        200,
    };

    DoVertexFormatTest(wgpu::VertexFormat::Uint8, vertexData, expectedData);
}

TEST_P(VertexFormatTest, Uint8x2) {
    std::vector<uint8_t> vertexData = {
        std::numeric_limits<uint8_t>::max(),
        0,
        0,  // padding two bytes for stride
        0,
        std::numeric_limits<uint8_t>::min(),
        2,
        0,
        0,  // padding two bytes for stride
        200,
        201,
        0,
        0  // padding two bytes for buffer copy
    };

    std::vector<uint8_t> expectedData = {
        std::numeric_limits<uint8_t>::max(), 0, std::numeric_limits<uint8_t>::min(), 2, 200, 201,
    };

    DoVertexFormatTest(wgpu::VertexFormat::Uint8x2, vertexData, expectedData);
}

TEST_P(VertexFormatTest, Uint8x4) {
    std::vector<uint8_t> vertexData = {
        std::numeric_limits<uint8_t>::max(),
        0,
        1,
        2,
        std::numeric_limits<uint8_t>::min(),
        2,
        3,
        4,
        200,
        201,
        202,
        203,
    };

    DoVertexFormatTest(wgpu::VertexFormat::Uint8x4, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Sint8) {
    std::vector<int8_t> vertexData = {
        std::numeric_limits<int8_t>::max(),
        0,  // padding three bytes for stride
        0,
        0,
        std::numeric_limits<int8_t>::min(),
        0,  // padding three bytes for stride
        0,
        0,
        -121,
        0,  // padding two bytes for buffer copy
        0,
        0,
    };

    std::vector<int8_t> expectedData = {
        std::numeric_limits<int8_t>::max(),
        std::numeric_limits<int8_t>::min(),
        -121,
    };

    DoVertexFormatTest(wgpu::VertexFormat::Sint8, vertexData, expectedData);
}

TEST_P(VertexFormatTest, Sint8x2) {
    std::vector<int8_t> vertexData = {
        std::numeric_limits<int8_t>::max(),
        0,
        0,  // padding two bytes for stride
        0,
        std::numeric_limits<int8_t>::min(),
        -2,
        0,  // padding two bytes for stride
        0,
        120,
        -121,
        0,
        0  // padding two bytes for buffer copy
    };

    std::vector<int8_t> expectedData = {
        std::numeric_limits<int8_t>::max(), 0, std::numeric_limits<int8_t>::min(), -2, 120, -121,
    };

    DoVertexFormatTest(wgpu::VertexFormat::Sint8x2, vertexData, expectedData);
}

TEST_P(VertexFormatTest, Sint8x4) {
    std::vector<int8_t> vertexData = {
        std::numeric_limits<int8_t>::max(),
        0,
        -1,
        2,
        std::numeric_limits<int8_t>::min(),
        -2,
        3,
        4,
        120,
        -121,
        122,
        -123,
    };

    DoVertexFormatTest(wgpu::VertexFormat::Sint8x4, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Unorm8) {
    std::vector<uint8_t> vertexData = {
        std::numeric_limits<uint8_t>::max(),
        0,  // padding three bytes for stride
        0,
        0,
        std::numeric_limits<uint8_t>::min(),
        0,  // padding three bytes for stride
        0,
        0,
        200,
        0,  // padding three bytes for stride
        0,
        0,
    };

    std::vector<uint8_t> expectedData = {
        std::numeric_limits<uint8_t>::max(),
        std::numeric_limits<uint8_t>::min(),
        200,
    };

    DoVertexFormatTest(wgpu::VertexFormat::Unorm8, vertexData, expectedData);
}

TEST_P(VertexFormatTest, Unorm8x2) {
    std::vector<uint8_t> vertexData = {
        std::numeric_limits<uint8_t>::max(),
        std::numeric_limits<uint8_t>::min(),
        0,  // padding two bytes for stride
        0,
        std::numeric_limits<uint8_t>::max() / 2u,
        std::numeric_limits<uint8_t>::min() / 2u,
        0,  // padding two bytes for stride
        0,
        200,
        201,
        0,
        0  // padding two bytes for buffer copy
    };

    std::vector<uint8_t> expectedData = {std::numeric_limits<uint8_t>::max(),
                                         std::numeric_limits<uint8_t>::min(),
                                         std::numeric_limits<uint8_t>::max() / 2u,
                                         std::numeric_limits<uint8_t>::min() / 2u,
                                         200,
                                         201};

    DoVertexFormatTest(wgpu::VertexFormat::Unorm8x2, vertexData, expectedData);
}

TEST_P(VertexFormatTest, Unorm8x4) {
    std::vector<uint8_t> vertexData = {std::numeric_limits<uint8_t>::max(),
                                       std::numeric_limits<uint8_t>::min(),
                                       0,
                                       0,
                                       std::numeric_limits<uint8_t>::max() / 2u,
                                       std::numeric_limits<uint8_t>::min() / 2u,
                                       0,
                                       0,
                                       200,
                                       201,
                                       202,
                                       203};

    DoVertexFormatTest(wgpu::VertexFormat::Unorm8x4, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Unorm8x4BGRA) {
    std::vector<uint8_t> vertexData = {std::numeric_limits<uint8_t>::max(),
                                       std::numeric_limits<uint8_t>::min(),
                                       0,
                                       0,
                                       std::numeric_limits<uint8_t>::max() / 2u,
                                       std::numeric_limits<uint8_t>::min() / 2u,
                                       0,
                                       0,
                                       200,
                                       201,
                                       202,
                                       203};

    std::vector<uint8_t> expectedData = {0,
                                         std::numeric_limits<uint8_t>::min(),
                                         std::numeric_limits<uint8_t>::max(),
                                         0,
                                         0,
                                         std::numeric_limits<uint8_t>::min() / 2u,
                                         std::numeric_limits<uint8_t>::max() / 2u,
                                         0,
                                         202,
                                         201,
                                         200,
                                         203};

    DoVertexFormatTest(wgpu::VertexFormat::Unorm8x4BGRA, vertexData, expectedData);
}

TEST_P(VertexFormatTest, Snorm8) {
    std::vector<int8_t> vertexData = {
        std::numeric_limits<int8_t>::max(),
        0,  // padding three bytes for stride
        0,
        0,
        std::numeric_limits<int8_t>::min(),
        0,  // padding two bytes for stride
        0,
        0,
        -121,
        0,  // padding two bytes for stride
        0,
        0,
    };

    std::vector<int8_t> expectedData = {
        std::numeric_limits<int8_t>::max(),
        std::numeric_limits<int8_t>::min(),
        -121,
    };

    DoVertexFormatTest(wgpu::VertexFormat::Snorm8, vertexData, expectedData);
}

TEST_P(VertexFormatTest, Snorm8x2) {
    std::vector<int8_t> vertexData = {
        std::numeric_limits<int8_t>::max(),
        std::numeric_limits<int8_t>::min(),
        0,  // padding two bytes for stride
        0,
        std::numeric_limits<int8_t>::max() / 2,
        std::numeric_limits<int8_t>::min() / 2,
        0,  // padding two bytes for stride
        0,
        120,
        -121,
        0,
        0  // padding two bytes for buffer copy
    };

    std::vector<int8_t> expectedData = {
        std::numeric_limits<int8_t>::max(),
        std::numeric_limits<int8_t>::min(),
        std::numeric_limits<int8_t>::max() / 2,
        std::numeric_limits<int8_t>::min() / 2,
        120,
        -121,
    };

    DoVertexFormatTest(wgpu::VertexFormat::Snorm8x2, vertexData, expectedData);
}

TEST_P(VertexFormatTest, Snorm8x4) {
    std::vector<int8_t> vertexData = {std::numeric_limits<int8_t>::max(),
                                      std::numeric_limits<int8_t>::min(),
                                      0,
                                      0,
                                      std::numeric_limits<int8_t>::max() / 2,
                                      std::numeric_limits<int8_t>::min() / 2,
                                      -2,
                                      2,
                                      120,
                                      -120,
                                      102,
                                      -123};

    DoVertexFormatTest(wgpu::VertexFormat::Snorm8x4, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Uint16) {
    std::vector<uint16_t> vertexData = {
        std::numeric_limits<uint16_t>::max(),
        0,  // Padding
        std::numeric_limits<uint16_t>::min(),
        0,  // Padding
        65432,
        0,  // Padding
    };

    std::vector<uint16_t> expectedData = {
        std::numeric_limits<uint16_t>::max(),
        std::numeric_limits<uint16_t>::min(),
        65432,
    };

    DoVertexFormatTest(wgpu::VertexFormat::Uint16, vertexData, expectedData);
}

TEST_P(VertexFormatTest, Uint16x2) {
    std::vector<uint16_t> vertexData = {std::numeric_limits<uint16_t>::max(),
                                        0,
                                        std::numeric_limits<uint16_t>::min(),
                                        2,
                                        65432,
                                        4890};

    DoVertexFormatTest(wgpu::VertexFormat::Uint16x2, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Uint16x4) {
    std::vector<uint16_t> vertexData = {
        std::numeric_limits<uint16_t>::max(),
        std::numeric_limits<uint8_t>::max(),
        1,
        2,
        std::numeric_limits<uint16_t>::min(),
        2,
        3,
        4,
        65520,
        65521,
        3435,
        3467,
    };

    DoVertexFormatTest(wgpu::VertexFormat::Uint16x4, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Sint16) {
    std::vector<int16_t> vertexData = {
        std::numeric_limits<int16_t>::max(),
        0,  // Padding
        std::numeric_limits<int16_t>::min(),
        0,  // Padding
        -3948,
        0,  // Padding
    };

    std::vector<int16_t> expectedData = {std::numeric_limits<int16_t>::max(),
                                         std::numeric_limits<int16_t>::min(), -3948};

    DoVertexFormatTest(wgpu::VertexFormat::Sint16, vertexData, expectedData);
}

TEST_P(VertexFormatTest, Sint16x2) {
    std::vector<int16_t> vertexData = {std::numeric_limits<int16_t>::max(),
                                       0,
                                       std::numeric_limits<int16_t>::min(),
                                       -2,
                                       3876,
                                       -3948};

    DoVertexFormatTest(wgpu::VertexFormat::Sint16x2, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Sint16x4) {
    std::vector<int16_t> vertexData = {
        std::numeric_limits<int16_t>::max(),
        0,
        -1,
        2,
        std::numeric_limits<int16_t>::min(),
        -2,
        3,
        4,
        24567,
        -23545,
        4350,
        -2987,
    };

    DoVertexFormatTest(wgpu::VertexFormat::Sint16x4, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Unorm16) {
    std::vector<uint16_t> vertexData = {
        std::numeric_limits<uint16_t>::max(),
        0,  // Padding
        std::numeric_limits<uint16_t>::min(),
        0,  // Padding
        3456,
        0,  // Padding
    };

    std::vector<uint16_t> expectedData = {
        std::numeric_limits<uint16_t>::max(),
        std::numeric_limits<uint16_t>::min(),
        3456,
    };

    DoVertexFormatTest(wgpu::VertexFormat::Unorm16, vertexData, expectedData);
}

TEST_P(VertexFormatTest, Unorm16x2) {
    std::vector<uint16_t> vertexData = {std::numeric_limits<uint16_t>::max(),
                                        std::numeric_limits<uint16_t>::min(),
                                        std::numeric_limits<uint16_t>::max() / 2u,
                                        std::numeric_limits<uint16_t>::min() / 2u,
                                        3456,
                                        6543};

    DoVertexFormatTest(wgpu::VertexFormat::Unorm16x2, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Unorm16x4) {
    std::vector<uint16_t> vertexData = {std::numeric_limits<uint16_t>::max(),
                                        std::numeric_limits<uint16_t>::min(),
                                        0,
                                        0,
                                        std::numeric_limits<uint16_t>::max() / 2u,
                                        std::numeric_limits<uint16_t>::min() / 2u,
                                        0,
                                        0,
                                        2987,
                                        3055,
                                        2987,
                                        2987};

    DoVertexFormatTest(wgpu::VertexFormat::Unorm16x4, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Snorm16) {
    std::vector<int16_t> vertexData = {
        std::numeric_limits<int16_t>::max(),
        0,  // Padding
        std::numeric_limits<int16_t>::min(),
        0,  // Padding
        -6789,
        0,  // Padding
    };

    std::vector<int16_t> expectedData = {
        std::numeric_limits<int16_t>::max(),
        std::numeric_limits<int16_t>::min(),
        -6789,
    };

    DoVertexFormatTest(wgpu::VertexFormat::Snorm16, vertexData, expectedData);
}

TEST_P(VertexFormatTest, Snorm16x2) {
    std::vector<int16_t> vertexData = {std::numeric_limits<int16_t>::max(),
                                       std::numeric_limits<int16_t>::min(),
                                       std::numeric_limits<int16_t>::max() / 2,
                                       std::numeric_limits<int16_t>::min() / 2,
                                       4987,
                                       -6789};

    DoVertexFormatTest(wgpu::VertexFormat::Snorm16x2, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Snorm16x4) {
    std::vector<int16_t> vertexData = {std::numeric_limits<int16_t>::max(),
                                       std::numeric_limits<int16_t>::min(),
                                       0,
                                       0,
                                       std::numeric_limits<int16_t>::max() / 2,
                                       std::numeric_limits<int16_t>::min() / 2,
                                       -2,
                                       2,
                                       2890,
                                       -29011,
                                       20432,
                                       -2083};

    DoVertexFormatTest(wgpu::VertexFormat::Snorm16x4, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Float16) {
    // Fails on NVIDIA's Vulkan drivers on CQ but passes locally.
    // TODO(dawn:1566) Might pass when using rectify_f32?
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsNvidia());

    std::vector<uint16_t> vertexData =
        Float32ToFloat16(std::vector<float>({14.8f, 0.0f, -0.0f, 0.0f, -24.8f, 0.0f}));
    std::vector<uint16_t> expectedData =
        Float32ToFloat16(std::vector<float>({14.8f, -0.0f, -24.8f}));

    DoVertexFormatTest(wgpu::VertexFormat::Float16, vertexData, expectedData);
}

TEST_P(VertexFormatTest, Float16x2) {
    // Fails on NVIDIA's Vulkan drivers on CQ but passes locally.
    // TODO(dawn:1566) Might pass when using rectify_f32?
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsNvidia());

    std::vector<uint16_t> vertexData =
        Float32ToFloat16(std::vector<float>({14.8f, -0.0f, 22.5f, 1.3f, +0.0f, -24.8f}));

    DoVertexFormatTest(wgpu::VertexFormat::Float16x2, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Float16x4) {
    // Fails on NVIDIA's Vulkan drivers on CQ but passes locally.
    // TODO(dawn:1566) Might pass when using rectify_f32?
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsNvidia());

    std::vector<uint16_t> vertexData = Float32ToFloat16(std::vector<float>(
        {+0.0f, -16.8f, 18.2f, -0.0f, 12.5f, 1.3f, 14.8f, -12.4f, 22.5f, -48.8f, 47.4f, -24.8f}));

    DoVertexFormatTest(wgpu::VertexFormat::Float16x4, vertexData, vertexData);
}
TEST_P(VertexFormatTest, Float32_Zeros) {
    std::vector<float> vertexData = {1.3f, +0.0f, -0.0f};

    DoVertexFormatTest(wgpu::VertexFormat::Float32, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Float32_Plain) {
    std::vector<float> vertexData = {+1.0f, -1.0f, 18.23f};

    DoVertexFormatTest(wgpu::VertexFormat::Float32, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Float32x2) {
    // Fails on NVIDIA's Vulkan drivers on CQ but passes locally.
    // TODO(dawn:1566) This might pass now that we use rectify_f32?
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsNvidia());

    std::vector<float> vertexData = {18.23f, -0.0f, +0.0f, +1.0f, 1.3f, -1.0f};

    DoVertexFormatTest(wgpu::VertexFormat::Float32x2, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Float32x3) {
    // Fails on NVIDIA's Vulkan drivers on CQ but passes locally.
    // TODO(dawn:1566) This might pass now that we use rectify_f32?
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsNvidia());

    std::vector<float> vertexData = {
        +0.0f, -1.0f, -0.0f, 1.0f, 1.3f, 99.45f, 23.6f, -81.2f, 55.0f,
    };

    DoVertexFormatTest(wgpu::VertexFormat::Float32x3, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Float32x4) {
    std::vector<float> vertexData = {
        19.2f, -19.3f, +0.0f, 1.0f, -0.0f, 1.0f, 1.3f, -1.0f, 13.078f, 21.1965f, -1.1f, -1.2f,
    };

    DoVertexFormatTest(wgpu::VertexFormat::Float32x4, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Uint32) {
    std::vector<uint32_t> vertexData = {std::numeric_limits<uint32_t>::max(),
                                        std::numeric_limits<uint16_t>::max(),
                                        std::numeric_limits<uint8_t>::max()};

    DoVertexFormatTest(wgpu::VertexFormat::Uint32, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Uint32x2) {
    std::vector<uint32_t> vertexData = {std::numeric_limits<uint32_t>::max(), 32,
                                        std::numeric_limits<uint16_t>::max(), 64,
                                        std::numeric_limits<uint8_t>::max(),  128};

    DoVertexFormatTest(wgpu::VertexFormat::Uint32x2, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Uint32x3) {
    std::vector<uint32_t> vertexData = {std::numeric_limits<uint32_t>::max(), 32,   64,
                                        std::numeric_limits<uint16_t>::max(), 164,  128,
                                        std::numeric_limits<uint8_t>::max(),  1283, 256};

    DoVertexFormatTest(wgpu::VertexFormat::Uint32x3, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Uint32x4) {
    std::vector<uint32_t> vertexData = {std::numeric_limits<uint32_t>::max(), 32,   64,  5460,
                                        std::numeric_limits<uint16_t>::max(), 164,  128, 0,
                                        std::numeric_limits<uint8_t>::max(),  1283, 256, 4567};

    DoVertexFormatTest(wgpu::VertexFormat::Uint32x4, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Sint32) {
    std::vector<int32_t> vertexData = {std::numeric_limits<int32_t>::max(),
                                       std::numeric_limits<int32_t>::min(),
                                       std::numeric_limits<int8_t>::max()};

    DoVertexFormatTest(wgpu::VertexFormat::Sint32, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Sint32x2) {
    std::vector<int32_t> vertexData = {
        std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min(),
        std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::min(),
        std::numeric_limits<int8_t>::max(),  std::numeric_limits<int8_t>::min()};

    DoVertexFormatTest(wgpu::VertexFormat::Sint32x2, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Sint32x3) {
    std::vector<int32_t> vertexData = {
        std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min(), 64,
        std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::min(), 128,
        std::numeric_limits<int8_t>::max(),  std::numeric_limits<int8_t>::min(),  256};

    DoVertexFormatTest(wgpu::VertexFormat::Sint32x3, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Sint32x4) {
    std::vector<int32_t> vertexData = {
        std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min(), 64,   -5460,
        std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::min(), -128, 0,
        std::numeric_limits<int8_t>::max(),  std::numeric_limits<int8_t>::min(),  256,  -4567};

    DoVertexFormatTest(wgpu::VertexFormat::Sint32x4, vertexData, vertexData);
}

TEST_P(VertexFormatTest, Unorm10_10_10_2) {
    auto MakeRGB10A2 = [](uint32_t r, uint32_t g, uint32_t b, uint32_t a) -> uint32_t {
        DAWN_ASSERT((r & 0x3FF) == r);
        DAWN_ASSERT((g & 0x3FF) == g);
        DAWN_ASSERT((b & 0x3FF) == b);
        DAWN_ASSERT((a & 0x3) == a);
        return r | g << 10 | b << 20 | a << 30;
    };

    std::vector<uint32_t> vertexData = {
        MakeRGB10A2(0, 0, 0, 0),
        MakeRGB10A2(1023, 1023, 1023, 3),
        MakeRGB10A2(243, 567, 765, 2),
    };

    std::vector<uint32_t> expectedData = {
        0,    0,    0,    0,  //
        1023, 1023, 1023, 3,  //
        243,  567,  765,  2,
    };

    DoVertexFormatTest(wgpu::VertexFormat::Unorm10_10_10_2, vertexData, expectedData);
}

DAWN_INSTANTIATE_TEST(VertexFormatTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
