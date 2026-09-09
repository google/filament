// Copyright 2026 The Dawn & Tint Authors
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
#include <initializer_list>
#include <string>
#include <vector>

#include "src/dawn/common/Math.h"
#include "src/dawn/tests/DawnTest.h"
#include "src/dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class BufferViewTest : public DawnTest {
  protected:
    void SetUp() override { DawnTest::SetUp(); }

    void BaseLengthTest(const std::string& wgsl,
                        uint32_t len,
                        uint32_t dynamicOffset,
                        const std::initializer_list<uint32_t>& uniforms,
                        const std::vector<uint32_t>& expected) {
        wgpu::ShaderModule module = utils::CreateShaderModule(device, wgsl);

        wgpu::BindGroupLayout bgLayout = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage,
                      dynamicOffset != 0, len},
                     {1, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage},
                     {2, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform}});

        wgpu::PipelineLayout pipelineLayout = utils::MakePipelineLayout(device, {bgLayout});

        std::vector<wgpu::ConstantEntry> constants{{nullptr, "len", static_cast<double>(len)}};
        wgpu::ComputePipelineDescriptor desc;
        desc.layout = pipelineLayout;
        desc.compute.module = module;
        desc.compute.constants = constants.data();
        desc.compute.constantCount = constants.size();
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&desc);

        wgpu::BufferDescriptor srcDesc;
        srcDesc.size = len + dynamicOffset;
        srcDesc.usage =
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer src = device.CreateBuffer(&srcDesc);

        wgpu::BufferDescriptor outDesc;
        outDesc.size = sizeof(uint32_t) * expected.size();
        outDesc.usage =
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer out = device.CreateBuffer(&outDesc);

        wgpu::Buffer uni = utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopySrc, uniforms);

        wgpu::BindGroup bg =
            utils::MakeBindGroup(device, bgLayout,
                                 {
                                     {0, src, 0, len},
                                     {1, out, 0, outDesc.size},
                                     {2, uni, 0, uniforms.size() * sizeof(uint32_t)},
                                 });

        std::vector<uint32_t> offsets;
        if (dynamicOffset != 0) {
            offsets.push_back(dynamicOffset);
        }
        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bg, offsets.size(), offsets.data());
            pass.DispatchWorkgroups(1, 1, 1);
            pass.End();
            commands = encoder.Finish();
        }
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U32_RANGE_EQ(&expected[0], out, 0, expected.size())
            << "Buffer length = " << len << "\n";
    }

    void TestBufferViewArrayLength(uint32_t len, uint32_t dynamicOffset) {
        const std::string src = R"(
          override len : u32;
          struct Offsets {
            a : u32,
            b : u32,
            c : u32,
          }

          @group(0) @binding(0) var<storage> in : buffer;
          @group(0) @binding(1) var<storage, read_write> out : array<u32>;
          @group(0) @binding(2) var<uniform> offsets : Offsets;

          struct S1 {
            a : vec4f,
            b : array<u32>,
          }
          struct S2 {
            a : vec4f,
            b : array<vec2u>,
          }
          struct S3 {
            a : vec4f,
            b : array<vec4u>,
          }

          @compute @workgroup_size(1)
          fn main() {
            out[0] = bufferLength(&in);

            out[1] = arrayLength(bufferView<array<u32>>(&in, offsets.a));
            out[2] = arrayLength(bufferView<array<u32>>(&in, offsets.b));
            out[3] = arrayLength(bufferView<array<u32>>(&in, offsets.c));

            out[4] = arrayLength(bufferView<array<vec2u>>(&in, offsets.a));
            out[5] = arrayLength(bufferView<array<vec2u>>(&in, offsets.b));
            out[6] = arrayLength(bufferView<array<vec2u>>(&in, offsets.c));

            out[7] = arrayLength(bufferView<array<vec3u>>(&in, offsets.a));
            out[8] = arrayLength(bufferView<array<vec3u>>(&in, offsets.b));
            out[9] = arrayLength(bufferView<array<vec3u>>(&in, offsets.c));

            out[10] = arrayLength(bufferView<array<vec4u>>(&in, offsets.a));
            out[11] = arrayLength(bufferView<array<vec4u>>(&in, offsets.b));
            out[12] = arrayLength(bufferView<array<vec4u>>(&in, offsets.c));

            out[13] = arrayLength(&bufferView<S1>(&in, offsets.a).b);
            out[14] = arrayLength(&bufferView<S1>(&in, offsets.b).b);
            out[15] = arrayLength(&bufferView<S1>(&in, offsets.c).b);

            out[16] = arrayLength(&bufferView<S2>(&in, offsets.a).b);
            out[17] = arrayLength(&bufferView<S2>(&in, offsets.b).b);
            out[18] = arrayLength(&bufferView<S2>(&in, offsets.c).b);

            out[19] = arrayLength(&bufferView<S3>(&in, offsets.a).b);
            out[20] = arrayLength(&bufferView<S3>(&in, offsets.b).b);
            out[21] = arrayLength(&bufferView<S3>(&in, offsets.c).b);
          }
        )";

        std::initializer_list<uint32_t> uniforms = {0u, 16u, 32u};

        std::vector<uint32_t> expected = {
            len,                   //
            len / 4,               //
            (len - 16) / 4,        //
            (len - 32) / 4,        //
            len / 8,               //
            (len - 16) / 8,        //
            (len - 32) / 8,        //
            len / 16,              //
            (len - 16) / 16,       //
            (len - 32) / 16,       //
            len / 16,              //
            (len - 16) / 16,       //
            (len - 32) / 16,       //
            (len - 16) / 4,        //
            (len - 16 - 16) / 4,   //
            (len - 16 - 32) / 4,   //
            (len - 16) / 8,        //
            (len - 16 - 16) / 8,   //
            (len - 16 - 32) / 8,   //
            (len - 16) / 16,       //
            (len - 16 - 16) / 16,  //
            (len - 16 - 32) / 16,  //
        };

        BaseLengthTest(src, len, dynamicOffset, uniforms, expected);
    }

    void TestBufferArrayViewArrayLength(uint32_t len, uint32_t dynamicOffset) {
        const std::string src = R"(
          override len : u32;
          @group(0) @binding(0) var<storage> in : buffer;
          @group(0) @binding(1) var<storage, read_write> out : array<u32>;
          @group(0) @binding(2) var<uniform> uniforms : array<vec4u, 2>;

          struct S1 {
            a : u32,
            b : array<u32>,
          }
          struct S2 {
            a : u32,
            b : array<vec2u>,
          }
          struct S3 {
            a : u32,
            b : array<vec4u>,
          }

          @compute @workgroup_size(1)
          fn main() {
            var i = 0u;
            out[i] = bufferLength(&in); i++;

            out[i] = arrayLength(bufferArrayView<array<u32>>(&in, uniforms[0][0], uniforms[1][0])); i++;
            out[i] = arrayLength(bufferArrayView<array<u32>>(&in, uniforms[0][1], uniforms[1][1])); i++;
            out[i] = arrayLength(bufferArrayView<array<u32>>(&in, uniforms[0][0], uniforms[1][2])); i++;

            out[i] = arrayLength(bufferArrayView<array<vec2u>>(&in, uniforms[0][0], uniforms[1][0])); i++;
            out[i] = arrayLength(bufferArrayView<array<vec2u>>(&in, uniforms[0][1], uniforms[1][1])); i++;
            out[i] = arrayLength(bufferArrayView<array<vec2u>>(&in, uniforms[0][0], uniforms[1][3])); i++;

            out[i] = arrayLength(bufferArrayView<array<vec3u>>(&in, uniforms[0][0], uniforms[1][0])); i++;
            out[i] = arrayLength(bufferArrayView<array<vec3u>>(&in, uniforms[0][1], uniforms[1][1])); i++;
            out[i] = arrayLength(bufferArrayView<array<vec3u>>(&in, uniforms[0][1], uniforms[1][3])); i++;

            out[i] = arrayLength(bufferArrayView<array<vec4u>>(&in, uniforms[0][0], uniforms[1][0])); i++;
            out[i] = arrayLength(bufferArrayView<array<vec4u>>(&in, uniforms[0][1], uniforms[1][1])); i++;
            out[i] = arrayLength(bufferArrayView<array<vec4u>>(&in, uniforms[0][0], uniforms[1][3])); i++;

            out[i] = arrayLength(&bufferArrayView<S1>(&in, uniforms[0][0], uniforms[1][0]).b); i++;
            out[i] = arrayLength(&bufferArrayView<S1>(&in, uniforms[0][1], uniforms[1][1]).b); i++;
            out[i] = arrayLength(&bufferArrayView<S1>(&in, uniforms[0][1], uniforms[1][3]).b); i++;

            out[i] = arrayLength(&bufferArrayView<S2>(&in, uniforms[0][0], uniforms[1][0]).b); i++;
            out[i] = arrayLength(&bufferArrayView<S2>(&in, uniforms[0][1], uniforms[1][1]).b); i++;

            out[i] = arrayLength(&bufferArrayView<S3>(&in, uniforms[0][0], uniforms[1][1]).b); i++;
            out[i] = arrayLength(&bufferArrayView<S3>(&in, uniforms[0][0], uniforms[1][3]).b); i++;
          }
        )";

        std::initializer_list<uint32_t> uniforms = {0u, 16u, 32u, 48u, 20u, 36u, 52u, len - 20u};
        std::vector<uint32_t> uniformData{uniforms};

        std::vector<uint32_t> expected = {
            len,                         //
            uniformData[4] / 4,          //
            uniformData[5] / 4,          //
            uniformData[6] / 4,          //
            uniformData[4] / 8,          //
            uniformData[5] / 8,          //
            uniformData[7] / 8,          //
            uniformData[4] / 16,         //
            uniformData[5] / 16,         //
            uniformData[7] / 16,         //
            uniformData[4] / 16,         //
            uniformData[5] / 16,         //
            uniformData[7] / 16,         //
            (uniformData[4] - 4) / 4,    //
            (uniformData[5] - 4) / 4,    //
            (uniformData[7] - 4) / 4,    //
            (uniformData[4] - 8) / 8,    //
            (uniformData[5] - 8) / 8,    //
            (uniformData[5] - 16) / 16,  //
            (uniformData[7] - 16) / 16,  //
        };

        BaseLengthTest(src, len, dynamicOffset, uniforms, expected);
    }

    void TestArrayLengthFunctions(uint32_t len) {
        const std::string wgsl = R"(
          override len : u32;
          @group(0) @binding(0) var<storage> in : buffer<)" +
                                 std::to_string(len) + R"(>;
          @group(0) @binding(1) var<storage, read_write> out : array<u32>;
          @group(0) @binding(2) var<uniform> uniforms : buffer<20>;

          var<workgroup> wg1 : buffer<64>;
          var<workgroup> wg2 : buffer<128>;
          var<workgroup> wg3 : buffer<len>;

          struct S {
            a : vec2u,
            b : array<u32>,
          }

          fn uniformLengthUnsized(p : ptr<uniform, buffer>, offset : u32) -> u32 {
            return arrayLength(bufferView<array<u32>>(p, offset));
          }

          fn uniformLengthSized(p : ptr<uniform, buffer<16>>, offset : u32) -> u32 {
            return uniformLengthUnsized(p, offset);
          }

          fn workgroupLengthUnsized(p : ptr<workgroup, buffer>, offset : u32, size : u32) -> u32 {
            return arrayLength(bufferArrayView<array<u32>>(p, offset, size));
          }

          fn workgroupLengthSmall(p : ptr<workgroup, buffer<64>>, offset : u32) -> u32 {
            return workgroupLengthUnsized(p, offset, 32);
          }

          fn workgroupLengthOverride(p : ptr<workgroup, buffer<len>>, offset : u32) -> u32 {
            return workgroupLengthUnsized(p, offset, 32);
          }

          fn storageLengthUnsized(p : ptr<storage, buffer>, offset : u32, size : u32) -> u32 {
            return arrayLength(&bufferArrayView<S>(p, offset, size).b);
          }

          fn storageLengthLarge(p : ptr<storage, buffer>, offset : u32) -> u32 {
            return arrayLength(&bufferView<S>(p, offset).b);
          }

          fn storageLengthSmall(p : ptr<storage, buffer<48>>, offset : u32) -> u32 {
            return storageLengthLarge(p, offset);
          }

          @compute @workgroup_size(1)
          fn main() {
            let offsets = bufferView<array<u32>>(&uniforms, 0u);
            var i = 0u;
            out[i] = uniformLengthUnsized(&uniforms, offsets[0]); i++;
            out[i] = uniformLengthUnsized(&uniforms, offsets[1]); i++;
            out[i] = uniformLengthUnsized(&uniforms, offsets[2]); i++;
            out[i] = uniformLengthSized(&uniforms, offsets[0]); i++;
            out[i] = uniformLengthSized(&uniforms, offsets[1]); i++;

            out[i] = workgroupLengthUnsized(&wg1, offsets[0], bufferLength(&wg1)); i++;
            out[i] = workgroupLengthUnsized(&wg1, offsets[1], offsets[2]); i++;
            out[i] = workgroupLengthUnsized(&wg1, offsets[3], offsets[3]); i++;
            out[i] = workgroupLengthUnsized(&wg2, offsets[0], bufferLength(&wg2)); i++;
            out[i] = workgroupLengthSmall(&wg2, offsets[1]); i++;
            out[i] = workgroupLengthSmall(&wg2, offsets[2]); i++;
            out[i] = workgroupLengthUnsized(&wg3, offsets[0], bufferLength(&wg3)); i++;
            out[i] = workgroupLengthOverride(&wg3, offsets[1]); i++;
            out[i] = workgroupLengthOverride(&wg3, offsets[2]); i++;

            out[i] = storageLengthUnsized(&in, offsets[0], bufferLength(&in)); i++;
            out[i] = storageLengthUnsized(&in, offsets[0], offsets[4]); i++;
            out[i] = storageLengthUnsized(&in, offsets[1], bufferLength(&in) - 8); i++;
            out[i] = storageLengthSmall(&in, 0); i++;
          }
        )";

        std::initializer_list<uint32_t> uniforms = {0, 8, 16, 32, 64};
        std::vector<uint32_t> uniformData{uniforms};

        uint32_t uniformSize = static_cast<uint32_t>(uniforms.size());
        std::vector<uint32_t> expected = {
            // Uniform sizes
            uniformSize,
            uniformSize - uniformData[1] / 4,
            uniformSize - uniformData[2] / 4,
            uniformSize - 1,
            uniformSize - 1 - uniformData[1] / 4,
            // Workgroup sizes
            64 / 4,
            uniformData[2] / 4,
            uniformData[3] / 4,
            128 / 4,
            32 / 4,
            32 / 4,
            len / 4,
            32 / 4,
            32 / 4,
            // Storage sizes
            (len - 8) / 4,
            (uniformData[4] - 8) / 4,
            (len - 8 - uniformData[1]) / 4,
            (48 - 8) / 4,
        };

        BaseLengthTest(wgsl, len, 0, uniforms, expected);
    }

    void BaseViewTest(const std::string& wgsl) {
        wgpu::ShaderModule module = utils::CreateShaderModule(device, wgsl);

        std::initializer_list<uint32_t> uniforms = {0, 1, 2,  3,  4,  5,  6,  7,
                                                    8, 9, 10, 11, 12, 13, 14, 15};
        std::initializer_list<uint32_t> inputs = {
            1, 2, 3, 4, 0x3f800000, 0x40000000, 0x40400000, 0x40800000, 7, 8, 9, 10, 11, 12, 13, 14,
        };
        const std::vector<uint32_t> expected = {
            1, 2,  3,  4,  0x3f800000, 0x40000000, 0x40400000, 0x40800000, 7, 8,
            9, 10, 11, 12, 13,         14,         0,          0,          0, 1234,
        };

        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = module;
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&desc);

        wgpu::Buffer src = utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc, inputs);

        wgpu::BufferDescriptor outDesc;
        outDesc.size = sizeof(uint32_t) * expected.size();
        outDesc.usage =
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer out = device.CreateBuffer(&outDesc);

        wgpu::Buffer uni = utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopySrc, uniforms);

        wgpu::BindGroup bg =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {
                                     {0, src, 0, inputs.size() * sizeof(uint32_t)},
                                     {1, out, 0, outDesc.size},
                                     {2, uni, 0, uniforms.size() * sizeof(uint32_t)},
                                 });

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bg);
            pass.DispatchWorkgroups(1, 1, 1);
            pass.End();
            commands = encoder.Finish();
        }
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U32_RANGE_EQ(&expected[0], out, 0, expected.size());
    }

    void TestBufferViewBasics() {
        const std::string wgsl = R"(
          @group(0) @binding(0) var<storage> in : buffer;
          @group(0) @binding(1) var<storage, read_write> out : buffer;
          @group(0) @binding(2) var<uniform> uniforms : buffer<64>;

          struct S {
            a : u32,
            b : vec4u,
          }

          struct T {
            a : vec3u,
            b : u32,
          }

          @compute @workgroup_size(1)
          fn main() {
            let uniform_v1 = bufferView<array<u32>>(&uniforms, 0);

            let in1 = bufferView<array<u32, 4>>(&in, uniform_v1[0]);
            let out1 = bufferView<vec4u>(&out, uniform_v1[0]);
            out1.x = in1[0];
            out1.y = in1[1];
            out1.z = in1[2];
            out1.w = in1[3];

            let in2x = bufferView<f32>(&in, uniform_v1[4] * 4);
            let in2y = bufferView<f32>(&in, uniform_v1[5] * 4);
            let in2z = bufferView<f32>(&in, uniform_v1[6] * 4);
            let in2w = bufferView<f32>(&in, uniform_v1[7] * 4);
            let out2 = bufferView<array<f32, 4>>(&out, 16u);
            out2[0] = *in2x;
            out2[1] = *in2y;
            out2[2] = *in2z;
            out2[3] = *in2w;

            let in3 = bufferView<S>(&in, uniform_v1[8] * 4);
            let in3_sub = bufferView<array<u32, 3>>(&in, uniform_v1[9] * 4);
            let out3 = bufferView<S>(&out, uniform_v1[8] * 4);
            let out3_sub = bufferView<array<u32, 3>>(&out, uniform_v1[9] * 4);
            let in3_ld = *in3;
            *out3_sub = *in3_sub;
            *out3 = in3_ld;

            let out4 = bufferView<array<T>>(&out, 0);
            let len = arrayLength(out4);
            out4[len - 1].b = 1234;
          }
        )";

        BaseViewTest(wgsl);
    }

    void TestBufferArrayViewBasics() {
        const std::string wgsl = R"(
          @group(0) @binding(0) var<storage> in : buffer;
          @group(0) @binding(1) var<storage, read_write> out : buffer;
          @group(0) @binding(2) var<uniform> uniforms : buffer<64>;

          struct S {
            a : u32,
            b : vec4u,
          }

          struct T {
            a : vec3u,
            b : u32,
          }

          @compute @workgroup_size(1)
          fn main() {
            let uniform_v1 = bufferArrayView<array<u32>>(&uniforms, 0, 16 * 4);

            let in1 = bufferArrayView<array<u32>>(&in, uniform_v1[0], uniform_v1[5] * 4);
            let out1 = bufferArrayView<array<vec4u>>(&out, uniform_v1[0], 16);
            out1[0].x = in1[0];
            out1[0].y = in1[1];
            out1[0].z = in1[2];
            out1[0].w = in1[3];

            let in2x = bufferArrayView<array<f32>>(&in, uniform_v1[4] * 4, 8);
            let in2y = bufferArrayView<array<f32>>(&in, uniform_v1[5] * 4, 8);
            let in2z = bufferArrayView<array<f32>>(&in, uniform_v1[6] * 4, 8);
            let in2w = bufferArrayView<array<f32>>(&in, uniform_v1[7] * 4, 8);
            let out2 = bufferArrayView<array<f32>>(&out, 16u, 16u);
            out2[0] = in2x[0];
            out2[1] = in2y[0];
            out2[2] = in2z[0];
            out2[3] = in2w[0];

            let in3 = bufferArrayView<array<S>>(&in, uniform_v1[8] * 4, 32);
            let in3_sub = bufferArrayView<array<array<u32, 3>>>(&in, uniform_v1[9] * 4, 12);
            let out3 = bufferArrayView<array<S>>(&out, uniform_v1[8] * 4, 32);
            let out3_sub = bufferArrayView<array<array<u32, 3>>>(&out, uniform_v1[9] * 4, 12);
            let in3_ld = in3[0];
            out3_sub[0] = in3_sub[0];
            out3[0] = in3_ld;

            let out4 = bufferArrayView<array<T>>(&out, 0, bufferLength(&out));
            let len = arrayLength(out4);
            out4[len - 1].b = 1234;
          }
        )";

        BaseViewTest(wgsl);
    }

    void TestSharedLengthCall(uint32_t len) {
        ASSERT_GT(len, 128u + 16u);

        const std::string wgsl = R"(
            struct S {
              a: array<vec4u, 8>,
              b: array<u32>,
            }

            struct T {
              a: array<vec2u, 4>,
              b: array<u32>,
            }

            @group(0) @binding(0) var<storage> buffer1: buffer;
            @group(0) @binding(1) var<storage> ssbo1 : array<u32>;
            @group(0) @binding(2) var<storage> ssbo2 : S;
            @group(0) @binding(3) var<storage> ssbo3 : T;

            @group(1) @binding(0) var<storage, read_write> out : array<u32>;

            fn length(p : ptr<storage, array<u32>>) -> u32 {
              return arrayLength(p);
            }

            fn indirectLength1(p : ptr<storage, array<u32>>) -> u32 {
              return length(p);
            }

            fn indirectLength2(p : ptr<storage, S>) -> u32 {
              return indirectLength1(&p.b);
            }

            @compute @workgroup_size(1) fn main() {
              var i = 0;
              out[i] = length(bufferView<array<u32>>(&buffer1, 0)); i++;
              out[i] = length(&bufferView<S>(&buffer1, 0).b); i++;
              out[i] = length(&bufferView<T>(&buffer1, 0).b); i++;

              out[i] = length(bufferView<array<u32>>(&buffer1, 16)); i++;
              out[i] = length(&bufferView<S>(&buffer1, 16).b); i++;
              out[i] = length(&bufferView<T>(&buffer1, 16).b); i++;

              out[i] = indirectLength1(&ssbo1); i++;
              out[i] = indirectLength2(&ssbo2); i++;
              out[i] = indirectLength1(&ssbo3.b); i++;
            }
        )";

        const std::vector<uint32_t> expected = {
            len / 4,               //
            (len - 128) / 4,       //
            (len - 32) / 4,        //
            (len - 16) / 4,        //
            (len - 16 - 128) / 4,  //
            (len - 16 - 32) / 4,   //
            len / 4,               //
            (len - 128) / 4,       //
            (len - 32) / 4,        //
        };

        wgpu::ShaderModule module = utils::CreateShaderModule(device, wgsl);

        wgpu::ComputePipelineDescriptor desc;
        desc.compute.module = module;
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&desc);

        wgpu::BufferDescriptor inDesc;
        inDesc.size = len;
        inDesc.usage =
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;

        wgpu::Buffer buffer1 = device.CreateBuffer(&inDesc);
        wgpu::Buffer ssbo1 = device.CreateBuffer(&inDesc);
        wgpu::Buffer ssbo2 = device.CreateBuffer(&inDesc);
        wgpu::Buffer ssbo3 = device.CreateBuffer(&inDesc);

        wgpu::BufferDescriptor outDesc;
        outDesc.size = 9 * sizeof(uint32_t);
        outDesc.usage =
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;

        wgpu::Buffer out = device.CreateBuffer(&outDesc);

        wgpu::BindGroup bg0 = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                   {
                                                       {0, buffer1, 0, inDesc.size},
                                                       {1, ssbo1, 0, inDesc.size},
                                                       {2, ssbo2, 0, inDesc.size},
                                                       {3, ssbo3, 0, inDesc.size},
                                                   });

        wgpu::BindGroup bg1 = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(1),
                                                   {
                                                       {0, out, 0, outDesc.size},
                                                   });

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bg0);
            pass.SetBindGroup(1, bg1);
            pass.DispatchWorkgroups(1, 1, 1);
            pass.End();
            commands = encoder.Finish();
        }
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U32_RANGE_EQ(&expected[0], out, 0, expected.size());
    }
};

TEST_P(BufferViewTest, BufferViewArrayLength) {
    // TODO(crbug.com/tint/512253562): Older WARP gets incorrect results, but newer WARP passes.
    DAWN_SUPPRESS_TEST_IF(IsWARP());
    // TODO(crbug.com/518635945): Produces incorrect result on Pixel 10.
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsImgTec() && IsVulkan());

    TestBufferViewArrayLength(128, 0);
    TestBufferViewArrayLength(256, 256);
    TestBufferViewArrayLength(512, 256);
    TestBufferViewArrayLength(1024, 256);
    TestBufferViewArrayLength(64, 0);
}

TEST_P(BufferViewTest, BufferArrayViewArrayLength) {
    // TODO(crbug.com/518635945): Produces incorrect result on Pixel 10.
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsImgTec() && IsVulkan());

    TestBufferArrayViewArrayLength(128, 0);
    TestBufferArrayViewArrayLength(256, 256);
    TestBufferArrayViewArrayLength(512, 256);
    TestBufferArrayViewArrayLength(1024, 256);
    TestBufferArrayViewArrayLength(64, 0);
}

TEST_P(BufferViewTest, ArrayLengthFunctions) {
    TestArrayLengthFunctions(128);
    TestArrayLengthFunctions(256);
    TestArrayLengthFunctions(512);
    TestArrayLengthFunctions(1024);
    TestArrayLengthFunctions(64);
}

TEST_P(BufferViewTest, BufferViewBasics) {
    TestBufferViewBasics();
}

TEST_P(BufferViewTest, BufferArrayViewBasics) {
    TestBufferArrayViewBasics();
}

TEST_P(BufferViewTest, SharedBufferLengths) {
    TestSharedLengthCall(256);
    TestSharedLengthCall(288);
    TestSharedLengthCall(512);
    TestSharedLengthCall(1024);
}

DAWN_INSTANTIATE_TEST(BufferViewTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      WebGPUBackend());
}  // namespace
}  // namespace dawn
