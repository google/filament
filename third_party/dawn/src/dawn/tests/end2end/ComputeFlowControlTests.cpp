// Copyright 2023 The Dawn & Tint Authors
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

#include <vector>

#include "dawn/tests/DawnTest.h"

#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

// Tests flow control in WGSL shaders. This helps to identify bugs either in Tint's WGSL
// compilation, or driver shader compilation.
class ComputeFlowControlTests : public DawnTest {
  public:
    void RunTest(const char* shader,
                 const std::vector<uint32_t>& inputs,
                 const std::vector<uint32_t>& expected);
};

void ComputeFlowControlTests::RunTest(const char* shader,
                                      const std::vector<uint32_t>& inputs,
                                      const std::vector<uint32_t>& expected) {
    // Set up shader and pipeline
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;

    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Set up src storage buffer
    wgpu::Buffer src = utils::CreateBufferFromData(
        device, inputs.data(), inputs.size() * sizeof(uint32_t),
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

    // Set up dst storage buffer
    std::vector<uint32_t> dst_init_values(expected.size(), 0xDEADBEEF);
    dst_init_values[0] = 0;  // initial count

    wgpu::Buffer dst = utils::CreateBufferFromData(
        device, dst_init_values.data(), dst_init_values.size() * sizeof(uint32_t),
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

    // Set up bind group and issue dispatch
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, src},
                                                         {1, dst},
                                                     });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), dst, 0, expected.size());
}

// Test no branching with one call to push_output
TEST_P(ComputeFlowControlTests, One) {
    const char* shader = R"(
struct Outputs {
  count : u32,
  data  : array<u32>,
};
@group(0) @binding(0) var<storage, read>       inputs  : array<u32>;
@group(0) @binding(1) var<storage, read_write> outputs : Outputs;

fn push_output(value : u32) {
  let i = outputs.count;
  outputs.data[i] = value;
  outputs.count++;
}

@compute @workgroup_size(1)
fn main() {
  _ = &inputs;
  _ = &outputs;

  push_output(0xA0);
})";

    auto inputs = std::vector<uint32_t>{
        0  // ignored
    };
    auto expected = std::vector<uint32_t>{1,            // count
                                          0xA0,         // first
                                          0xDEADBEEF};  // unwritten
    RunTest(shader, inputs, expected);
}

// Test no branching with two calls to push_output
TEST_P(ComputeFlowControlTests, Two) {
    const char* shader = R"(
struct Outputs {
  count : u32,
  data  : array<u32>,
};
@group(0) @binding(0) var<storage, read>       inputs  : array<u32>;
@group(0) @binding(1) var<storage, read_write> outputs : Outputs;

fn push_output(value : u32) {
  let i = outputs.count;
  outputs.data[i] = value;
  outputs.count++;
}

@compute @workgroup_size(1)
fn main() {
  _ = &inputs;
  _ = &outputs;

  push_output(0xA0);
  push_output(0xA1);
})";

    auto inputs = std::vector<uint32_t>{
        0  // ignored
    };
    auto expected = std::vector<uint32_t>{2,            // count
                                          0xA0,         // first
                                          0xA1,         // second
                                          0xDEADBEEF};  // unwritten
    RunTest(shader, inputs, expected);
}

// Test no branching with three calls to push_output
TEST_P(ComputeFlowControlTests, Three) {
    const char* shader = R"(
struct Outputs {
  count : u32,
  data  : array<u32>,
};
@group(0) @binding(0) var<storage, read>       inputs  : array<u32>;
@group(0) @binding(1) var<storage, read_write> outputs : Outputs;

fn push_output(value : u32) {
  let i = outputs.count;
  outputs.data[i] = value;
  outputs.count++;
}

@compute @workgroup_size(1)
fn main() {
  _ = &inputs;
  _ = &outputs;

  push_output(0xA0);
  push_output(0xA1);
  push_output(0xA2);
})";

    auto inputs = std::vector<uint32_t>{
        0  // ignored
    };
    auto expected = std::vector<uint32_t>{3,            // count
                                          0xA0,         // first
                                          0xA1,         // second
                                          0xA2,         // third
                                          0xDEADBEEF};  // unwritten
    RunTest(shader, inputs, expected);
}

// Test if statement with branch taken
TEST_P(ComputeFlowControlTests, IfTrue) {
    const char* shader = R"(
struct Outputs {
  count : u32,
  data  : array<u32>,
};
@group(0) @binding(0) var<storage, read>       inputs  : array<u32>;
@group(0) @binding(1) var<storage, read_write> outputs : Outputs;

fn push_output(value : u32) {
  let i = outputs.count;
  outputs.data[i] = value;
  outputs.count++;
}

@compute @workgroup_size(1)
fn main() {
  _ = &inputs;
  _ = &outputs;

  push_output(0xA0);
  if (inputs[0] != 0) {
    push_output(0xA1);
  }
  push_output(0xA3);
})";

    auto inputs = std::vector<uint32_t>{
        1  // take branch
    };
    auto expected = std::vector<uint32_t>{3,            // count
                                          0xA0,         // before if-else
                                          0xA1,         // branch
                                          0xA3,         // after if-else
                                          0xDEADBEEF};  // unwritten

    RunTest(shader, inputs, expected);
}

// Test if statement with branch not taken
TEST_P(ComputeFlowControlTests, IfFalse) {
    const char* shader = R"(
struct Outputs {
  count : u32,
  data  : array<u32>,
};
@group(0) @binding(0) var<storage, read>       inputs  : array<u32>;
@group(0) @binding(1) var<storage, read_write> outputs : Outputs;

fn push_output(value : u32) {
  let i = outputs.count;
  outputs.data[i] = value;
  outputs.count++;
}

@compute @workgroup_size(1)
fn main() {
  _ = &inputs;
  _ = &outputs;

  push_output(0xA0);
  if (inputs[0] != 0) {
    push_output(0xA1);
  }
  push_output(0xA3);
})";

    auto inputs = std::vector<uint32_t>{
        0  // don't take branch
    };
    auto expected = std::vector<uint32_t>{2,            // count
                                          0xA0,         // before if-else
                                          0xA3,         // after if-else
                                          0xDEADBEEF};  // unwritten

    RunTest(shader, inputs, expected);
}

// Same as IfFalse test, but with push_output calls inlined
TEST_P(ComputeFlowControlTests, IfFalseInlined) {
    const char* shader = R"(
struct Outputs {
  count : u32,
  data  : array<u32>,
};
@group(0) @binding(0) var<storage, read>       inputs  : array<u32>;
@group(0) @binding(1) var<storage, read_write> outputs : Outputs;

@compute @workgroup_size(1)
fn main() {
  _ = &inputs;
  _ = &outputs;

  {
    let i = outputs.count;
    outputs.data[i] = 0xA0u;
    outputs.count++;
  }

  if (inputs[0] != 0) {
      let i = outputs.count;
      outputs.data[i] = 0xA1u;
      outputs.count++;
  }

  {
    var i = outputs.count;
    outputs.data[i] = 0xA3u;
    outputs.count++;
  }
})";

    auto inputs = std::vector<uint32_t>{
        0  // don't take branch
    };
    auto expected = std::vector<uint32_t>{2,            // count
                                          0xA0,         // before if-else
                                          0xA3,         // after if-else
                                          0xDEADBEEF};  // unwritten
    RunTest(shader, inputs, expected);
}

// Same as IfFalse test, but with fixed-size storage arrays
TEST_P(ComputeFlowControlTests, IfFalseFixedSizeArrays) {
    const char* shader = R"(
struct Outputs {
  count : u32,
  data  : array<u32, 2>,
};
@group(0) @binding(0) var<storage, read>       inputs  : array<u32, 1>;
@group(0) @binding(1) var<storage, read_write> outputs : Outputs;

fn push_output(value : u32) {
  let i = outputs.count;
  outputs.data[i] = value;
  outputs.count++;
}

@compute @workgroup_size(1)
fn main() {
  _ = &inputs;
  _ = &outputs;

  push_output(0xA0);
  if (inputs[0] != 0) {
    push_output(0xA1);
  }
  push_output(0xA3);
})";

    auto inputs = std::vector<uint32_t>{
        0  // don't take branch
    };
    auto expected = std::vector<uint32_t>{2,            // count
                                          0xA0,         // before if-else
                                          0xA3,         // after if-else
                                          0xDEADBEEF};  // unwritten
    RunTest(shader, inputs, expected);
}

// Same as IfFalse test, but `outputs.count++` is replaced by `outputs.count = i + 1`
TEST_P(ComputeFlowControlTests, IfFalseNoCountPlusPlus) {
    const char* shader = R"(
struct Outputs {
  count : u32,
  data  : array<u32>,
};
@group(0) @binding(0) var<storage, read>       inputs  : array<u32>;
@group(0) @binding(1) var<storage, read_write> outputs : Outputs;

fn push_output(value : u32) {
  let i = outputs.count;
  outputs.data[i] = value;
  outputs.count = i + 1;
}

@compute @workgroup_size(1)
fn main() {
  _ = &inputs;
  _ = &outputs;

  push_output(0xA0);
  if (inputs[0] != 0) {
    push_output(0xA1);
  }
  push_output(0xA3);
})";

    auto inputs = std::vector<uint32_t>{
        0  // don't take branch
    };
    auto expected = std::vector<uint32_t>{2,            // count
                                          0xA0,         // before if-else
                                          0xA3,         // after if-else
                                          0xDEADBEEF};  // unwritten
    RunTest(shader, inputs, expected);
}

// Same as IfFalse test, but `outputs.count++` is replaced by `outputs.count += 4`
TEST_P(ComputeFlowControlTests, IfFalseIncCountByFour) {
    const char* shader = R"(
struct Outputs {
  count : u32,
  data  : array<u32>,
};
@group(0) @binding(0) var<storage, read>       inputs  : array<u32>;
@group(0) @binding(1) var<storage, read_write> outputs : Outputs;

fn push_output(value : u32) {
  let i = outputs.count;
  outputs.data[i] = value;
  outputs.count += 4;
}

@compute @workgroup_size(1)
fn main() {
  _ = &inputs;
  _ = &outputs;

  push_output(0xA0);
  if (inputs[0] != 0) {
    push_output(0xA1);
  }
  push_output(0xA3);
})";

    auto inputs = std::vector<uint32_t>{
        0  // don't take branch
    };
    const uint32_t D = 0xDEADBEEF;
    auto expected = std::vector<uint32_t>{8,               // count
                                          0xA0, D, D, D,   // before if-else
                                          0xA3, D, D, D};  // after if-else
    RunTest(shader, inputs, expected);
}

// Test if-else statement with true branch taken
TEST_P(ComputeFlowControlTests, IfElseTrue) {
    const char* shader = R"(
struct Outputs {
  count : u32,
  data  : array<u32>,
};
@group(0) @binding(0) var<storage, read>       inputs  : array<u32>;
@group(0) @binding(1) var<storage, read_write> outputs : Outputs;

fn push_output(value : u32) {
  let i = outputs.count;
  outputs.data[i] = value;
  outputs.count++;
}

@compute @workgroup_size(1)
fn main() {
  _ = &inputs;
  _ = &outputs;

  push_output(0xA0);
  if (inputs[0] != 0) {
    push_output(0xA1);
  } else {
    push_output(0xA2);
  }
  push_output(0xA3);
})";

    auto inputs = std::vector<uint32_t>{
        1  // take true branch
    };
    auto expected = std::vector<uint32_t>{3,            // count
                                          0xA0,         // before if-else
                                          0xA1,         // true branch
                                          0xA3,         // after if-else
                                          0xDEADBEEF};  // unwritten
    RunTest(shader, inputs, expected);
}

// Test if-else statement with false branch taken
TEST_P(ComputeFlowControlTests, IfElseFalse) {
    const char* shader = R"(
struct Outputs {
  count : u32,
  data  : array<u32>,
};
@group(0) @binding(0) var<storage, read>       inputs  : array<u32>;
@group(0) @binding(1) var<storage, read_write> outputs : Outputs;

fn push_output(value : u32) {
  let i = outputs.count;
  outputs.data[i] = value;
  outputs.count++;
}

@compute @workgroup_size(1)
fn main() {
  _ = &inputs;
  _ = &outputs;

  push_output(0xA0);
  if (inputs[0] != 0) {
    push_output(0xA1);
  } else {
    push_output(0xA2);
  }
  push_output(0xA3);
})";

    auto inputs = std::vector<uint32_t>{
        0  // take false branch
    };
    auto expected = std::vector<uint32_t>{3,            // count
                                          0xA0,         // before if-else
                                          0xA2,         // false branch
                                          0xA3,         // after if-else
                                          0xDEADBEEF};  // unwritten
    RunTest(shader, inputs, expected);
}

DAWN_INSTANTIATE_TEST(ComputeFlowControlTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
