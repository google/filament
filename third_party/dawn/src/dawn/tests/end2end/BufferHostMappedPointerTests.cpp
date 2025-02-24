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

#include "dawn/tests/end2end/BufferHostMappedPointerTests.h"

#include "dawn/utils/WGPUHelpers.h"

namespace dawn {

std::pair<wgpu::Buffer, void*> BufferHostMappedPointerTestBackend::CreateHostMappedBuffer(
    wgpu::Device device,
    wgpu::BufferUsage usage,
    size_t size) {
    return CreateHostMappedBuffer(device, usage, size, [](void*) {});
}

std::vector<wgpu::FeatureName> BufferHostMappedPointerTests::GetRequiredFeatures() {
    if (!SupportsFeatures({wgpu::FeatureName::HostMappedPointer})) {
        return {};
    }
    return {wgpu::FeatureName::HostMappedPointer};
}

void BufferHostMappedPointerTests::SetUp() {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    DawnTestWithParams<BufferHostMappedPointerTestParams>::SetUp();
    DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::HostMappedPointer}));

    // TODO(crbug.com/dawn/2018): Expose a proper limit for the alignment.
    if (IsD3D12()) {
        mRequiredAlignment = 65536;
    } else {
        mRequiredAlignment = 4096;
    }
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(BufferHostMappedPointerTests);

namespace {

class BufferHostMappedPointerNoFeatureTests : public DawnTest {
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }
};

// Test that the feature must be enabled to create buffers from host-mapped pointers.
TEST_P(BufferHostMappedPointerNoFeatureTests, Creation) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::BufferHostMappedPointer hostMappedDesc;
    hostMappedDesc.pointer = nullptr;
    hostMappedDesc.disposeCallback = [](void* userdata) {};
    hostMappedDesc.userdata = nullptr;

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.usage = wgpu::BufferUsage::CopySrc;
    bufferDesc.size = 1024;
    bufferDesc.nextInChain = &hostMappedDesc;

    ASSERT_DEVICE_ERROR_MSG(
        device.CreateBuffer(&bufferDesc),
        testing::HasSubstr(
            "SType::BufferHostMappedPointer requires FeatureName::HostMappedPointer"));
}

DAWN_INSTANTIATE_TEST(BufferHostMappedPointerNoFeatureTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      VulkanBackend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend());

// Test that memory allocations must be aligned to the required alignment.
TEST_P(BufferHostMappedPointerTests, Alignment) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    // Invalid: half required alignment
    ASSERT_DEVICE_ERROR(GetParam().mBackend->CreateHostMappedBuffer(
        device, wgpu::BufferUsage::CopySrc, mRequiredAlignment / 2u));

    GetParam().mBackend->CreateHostMappedBuffer(device, wgpu::BufferUsage::CopySrc,
                                                mRequiredAlignment);

    // Invalid: just below required alignment
    ASSERT_DEVICE_ERROR(GetParam().mBackend->CreateHostMappedBuffer(
        device, wgpu::BufferUsage::CopySrc, mRequiredAlignment - 1));

    // Invalid: just over required alignment
    ASSERT_DEVICE_ERROR(GetParam().mBackend->CreateHostMappedBuffer(
        device, wgpu::BufferUsage::CopySrc, mRequiredAlignment + 1));

    // Valid: multiple of required alignment
    GetParam().mBackend->CreateHostMappedBuffer(device, wgpu::BufferUsage::CopySrc,
                                                2 * mRequiredAlignment);
}

// Test creating a buffer with data initially in the host-mapped memory.
// It should be GPU-visible immediately after creation.
// Then, change the host pointer, and see changes reflected on the GPU.
TEST_P(BufferHostMappedPointerTests, InitialDataAndCopySrc) {
    // Set up expected data.
    uint32_t bufferSize = mRequiredAlignment;
    std::vector<uint32_t> expected(bufferSize / sizeof(uint32_t));
    for (size_t i = 0; i < expected.size(); ++i) {
        expected[i] = i;
    }

    // Create the buffer and pre-fill it with data.
    auto [buffer, ptr] = GetParam().mBackend->CreateHostMappedBuffer(
        device, wgpu::BufferUsage::CopySrc, bufferSize,
        [&](void* initialPtr) { memcpy(initialPtr, expected.data(), bufferSize); });

    // Check the buffer contents.
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), buffer, 0, expected.size());

    // Wait for the GPU to complete, then change the host buffer contents.
    WaitForAllOperations();
    for (size_t i = 0; i < bufferSize / sizeof(uint32_t); ++i) {
        reinterpret_cast<uint32_t*>(ptr)[i] += 42;
    }

    // Expect to see the new contents in the buffer.
    for (auto& e : expected) {
        e += 42;
    }
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), buffer, 0, expected.size());
}

// Create a host-mapped buffer with CopyDst usage. Test that changes on the GPU
// are visible to the host.
TEST_P(BufferHostMappedPointerTests, CopyDst) {
    // TODO(crbug.com/358296955): Re-enable when this no longer causes
    // subsequent tests to flakily crash.
    DAWN_SUPPRESS_TEST_IF(IsMacOS() && IsAMD() && IsMetal());

    // Set up expected data.
    uint32_t bufferSize = mRequiredAlignment;
    std::vector<uint32_t> expected(bufferSize / sizeof(uint32_t));
    for (size_t i = 0; i < expected.size(); ++i) {
        expected[i] = i;
    }

    // Create the buffer.
    auto [buffer, ptr] =
        GetParam().mBackend->CreateHostMappedBuffer(device, wgpu::BufferUsage::CopyDst, bufferSize);

    // Create another GPU buffer to use as the source.
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = bufferSize;
    bufferDesc.usage = wgpu::BufferUsage::CopySrc;
    bufferDesc.mappedAtCreation = true;
    wgpu::Buffer bufferSrc = device.CreateBuffer(&bufferDesc);

    // Fill the src buffer wth data.
    memcpy(bufferSrc.GetMappedRange(), expected.data(), bufferSize);
    bufferSrc.Unmap();

    // Do a GPU-GPU copy from the source buffer into the host-mapped buffer.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(bufferSrc, 0, buffer, 0, bufferSize);
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    device.GetQueue().Submit(1, &commandBuffer);

    // Wait for the GPU to complete.
    WaitForAllOperations();

    // Expect the changes to be reflected in the host pointer.
    EXPECT_EQ(memcmp(ptr, expected.data(), bufferSize), 0);
}

// Create a host-mapped buffer with Storage usage. Test that writes on the host
// are visible on the GPU, and writes on the GPU are visible on the host.
TEST_P(BufferHostMappedPointerTests, Storage) {
    // Set up expected data.
    uint32_t bufferSize = mRequiredAlignment;
    std::vector<uint32_t> contents(bufferSize / sizeof(uint32_t));
    for (size_t i = 0; i < contents.size(); ++i) {
        contents[i] = i;
    }

    // Create the buffer, but don't prefill it with data.
    auto [buffer, ptr] =
        GetParam().mBackend->CreateHostMappedBuffer(device, wgpu::BufferUsage::Storage, bufferSize);

    // Copy contents into the buffer after creation. We'll check that this
    // write is visible to the GPU.
    memcpy(ptr, contents.data(), bufferSize);

    // Test storage read/write by checking the contents in a shader.
    // When the contents are as expected, increment the value. We'll read back on the CPU
    // to verify the writes are visible.
    wgpu::ComputePipelineDescriptor pipelineDesc = {};
    pipelineDesc.compute.module = utils::CreateShaderModule(device, R"(
        struct Buf {
            values : array<u32>,
        };
        @group(0) @binding(0) var<storage, read_write> buf : Buf;

        @workgroup_size(64)
        @compute fn main(@builtin(global_invocation_id) gid : vec3<u32>) {
            if (buf.values[gid.x] == gid.x) {
                buf.values[gid.x] = gid.x + 1u;
            }
        }
    )");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, buffer},
                                                     });
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(contents.size() / 64);
    pass.End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    device.GetQueue().Submit(1, &commandBuffer);

    // Wait for the GPU to complete.
    WaitForAllOperations();
    for (uint32_t& v : contents) {
        v += 1;
    }
    // Expect the changes to be reflected in the host pointer.
    EXPECT_EQ(memcmp(ptr, contents.data(), bufferSize), 0);
}

// Test interaction with other buffer mapping APIs.
TEST_P(BufferHostMappedPointerTests, Mapping) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    auto [buffer, _] = GetParam().mBackend->CreateHostMappedBuffer(
        device, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite, mRequiredAlignment);

    // Can't get mapped range from buffer.
    ASSERT_EQ(buffer.GetMappedRange(), nullptr);

    // Invalid to unmap a persistently host mapped buffer.
    ASSERT_DEVICE_ERROR(buffer.Unmap());

    // Invalid to map a persistently host mapped buffer.
    ASSERT_DEVICE_ERROR_MSG(buffer.MapAsync(wgpu::MapMode::Write, 0, wgpu::kWholeMapSize,
                                            wgpu::CallbackMode::AllowSpontaneous,
                                            [](wgpu::MapAsyncStatus, wgpu::StringView) {}),
                            testing::HasSubstr("cannot be mapped"));

    // Still invalid to GetMappedRange() or Unmap.
    ASSERT_EQ(buffer.GetMappedRange(), nullptr);
    ASSERT_DEVICE_ERROR(buffer.Unmap());

    // TODO(crbug.com/dawn/2018):
    // Test it is invalid to pass mappedAtCreation = true
}

// Test creating a buffer with data initially in the host-mapped memory
// on multiple threads. The contents should be correct and  GPU-visible
// immediately after creation.
TEST_P(BufferHostMappedPointerTests, MultithreadedCreation) {
    std::vector<wgpu::Buffer> buffers(20);

    uint32_t bufferSize = mRequiredAlignment;
    uint32_t u32PerBuffer = bufferSize / sizeof(uint32_t);
    // Set up expected data.
    std::vector<uint32_t> expected(buffers.size() * bufferSize);
    for (size_t i = 0; i < expected.size(); ++i) {
        expected[i] = i;
    }

    // Create buffers on multiple threads.
    utils::RunInParallel(buffers.size(), [&, this](uint32_t i) {
        auto [buffer, _] = GetParam().mBackend->CreateHostMappedBuffer(
            device, wgpu::BufferUsage::CopySrc, bufferSize,
            [&](void* initialPtr) { memcpy(initialPtr, &expected[i * u32PerBuffer], bufferSize); });
        buffers[i] = std::move(buffer);
    });

    // Check the buffer contents.
    for (uint32_t i = 0; i < buffers.size(); ++i) {
        EXPECT_BUFFER_U32_RANGE_EQ(&expected[i * u32PerBuffer], buffers[i], 0, u32PerBuffer);
    }
}

// TODO(crbug.com/dawn/2018):
// - Figure out and test error handling. Is / when is the dispose callback
//   called when there is an error?

}  // anonymous namespace
}  // namespace dawn
