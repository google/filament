// Copyright 2024 The Dawn & Tint Authors
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

#include "dawn/tests/white_box/SharedBufferMemoryTests.h"

#include <gtest/gtest.h>
#include <vector>
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {

void SharedBufferMemoryTests::SetUp() {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    DawnTestWithParams<SharedBufferMemoryTestParams>::SetUp();
}

std::vector<wgpu::FeatureName> SharedBufferMemoryTests::GetRequiredFeatures() {
    auto features = GetParam().mBackend->RequiredFeatures(GetAdapter().Get());
    if (!SupportsFeatures(features)) {
        return {};
    }

    return features;
}

wgpu::Texture Create2DTexture(wgpu::Device device,
                              uint32_t width,
                              uint32_t height,
                              wgpu::TextureFormat format,
                              wgpu::TextureUsage usage) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = width;
    descriptor.size.height = height;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = format;
    descriptor.mipLevelCount = 1;
    descriptor.usage = usage;
    return device.CreateTexture(&descriptor);
}

wgpu::SharedFence SharedBufferMemoryTestBackend::ImportFenceTo(const wgpu::Device& importingDevice,
                                                               const wgpu::SharedFence& fence) {
    wgpu::SharedFenceExportInfo exportInfo;
    fence.ExportInfo(&exportInfo);

    switch (exportInfo.type) {
        case wgpu::SharedFenceType::DXGISharedHandle: {
            wgpu::SharedFenceDXGISharedHandleExportInfo dxgiExportInfo;
            exportInfo.nextInChain = &dxgiExportInfo;
            fence.ExportInfo(&exportInfo);

            wgpu::SharedFenceDXGISharedHandleDescriptor dxgiDesc;
            dxgiDesc.handle = dxgiExportInfo.handle;

            wgpu::SharedFenceDescriptor fenceDesc;
            fenceDesc.nextInChain = &dxgiDesc;
            return importingDevice.ImportSharedFence(&fenceDesc);
        }
        default:
            DAWN_UNREACHABLE();
    }
}

namespace {

constexpr uint32_t kBufferData = 0x76543210;
constexpr uint32_t kBufferData2 = 0x01234567;
constexpr uint32_t kBufferSize = 4;
constexpr wgpu::BufferUsage kMapWriteUsages =
    wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
constexpr wgpu::BufferUsage kMapReadUsages =
    wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
constexpr wgpu::BufferUsage kStorageUsages =
    wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage;
using ::testing::HasSubstr;

// Test that it is an error to import shared buffer memory without a chained struct.
TEST_P(SharedBufferMemoryTests, ImportSharedBufferMemoryNoChain) {
    wgpu::SharedBufferMemoryDescriptor desc;
    ASSERT_DEVICE_ERROR_MSG(
        wgpu::SharedBufferMemory memory = device.ImportSharedBufferMemory(&desc),
        HasSubstr("chain"));
}

// Test that it is an error to import shared buffer memory when the device is destroyed.
TEST_P(SharedBufferMemoryTests, ImportSharedBufferMemoryDeviceDestroy) {
    device.Destroy();

    wgpu::SharedBufferMemoryDescriptor desc;
    wgpu::SharedBufferMemory memory = device.ImportSharedBufferMemory(&desc);
}

// Test that SharedBufferMemory::IsDeviceLost() returns the expected value before and
// after destroying the device.
TEST_P(SharedBufferMemoryTests, CheckIsDeviceLostBeforeAndAfterDestroyingDevice) {
    wgpu::SharedBufferMemory memory =
        GetParam().mBackend->CreateSharedBufferMemory(device, kMapWriteUsages, kBufferSize);

    EXPECT_FALSE(memory.IsDeviceLost());
    device.Destroy();
    EXPECT_TRUE(memory.IsDeviceLost());
}

// Test that SharedBufferMemory::IsDeviceLost() returns the expected value before and
// after losing the device.
TEST_P(SharedBufferMemoryTests, CheckIsDeviceLostBeforeAndAfterLosingDevice) {
    wgpu::SharedBufferMemory memory =
        GetParam().mBackend->CreateSharedBufferMemory(device, kMapWriteUsages, kBufferSize);

    EXPECT_FALSE(memory.IsDeviceLost());
    LoseDeviceForTesting(device);
    EXPECT_TRUE(memory.IsDeviceLost());
}

// Test calling GetProperties on SharedBufferMemory after an error.
TEST_P(SharedBufferMemoryTests, GetPropertiesErrorMemory) {
    wgpu::SharedBufferMemoryDescriptor desc;
    ASSERT_DEVICE_ERROR(wgpu::SharedBufferMemory memory = device.ImportSharedBufferMemory(&desc));

    wgpu::SharedBufferMemoryProperties properties;
    memory.GetProperties(&properties);

    EXPECT_EQ(properties.usage, wgpu::BufferUsage::None);
    EXPECT_EQ(properties.size, 0u);
}

// Tests that creating SharedBufferMemory validates buffer size.
TEST_P(SharedBufferMemoryTests, SizeValidation) {
    wgpu::SharedBufferMemory memory =
        GetParam().mBackend->CreateSharedBufferMemory(device, kMapWriteUsages, kBufferSize);
    wgpu::SharedBufferMemoryProperties properties;
    memory.GetProperties(&properties);

    wgpu::BufferDescriptor bufferDesc = {};
    bufferDesc.usage = properties.usage;
    bufferDesc.size = properties.size + 1;
    ASSERT_DEVICE_ERROR_MSG(memory.CreateBuffer(&bufferDesc),
                            HasSubstr("doesn't match descriptor size"));
}

// Tests that creating SharedBufferMemory validates buffer usages.
TEST_P(SharedBufferMemoryTests, UsageValidation) {
    wgpu::SharedBufferMemory memory =
        GetParam().mBackend->CreateSharedBufferMemory(device, kMapWriteUsages, kBufferSize);
    wgpu::SharedBufferMemoryProperties properties;
    memory.GetProperties(&properties);

    wgpu::BufferDescriptor bufferDesc = {};
    bufferDesc.size = properties.size;

    for (wgpu::BufferUsage usage :
         {wgpu::BufferUsage::MapRead, wgpu::BufferUsage::MapWrite, wgpu::BufferUsage::CopySrc,
          wgpu::BufferUsage::CopyDst, wgpu::BufferUsage::Index, wgpu::BufferUsage::Vertex,
          wgpu::BufferUsage::Uniform, wgpu::BufferUsage::Storage, wgpu::BufferUsage::Indirect,
          wgpu::BufferUsage::QueryResolve}) {
        bufferDesc.usage = usage;
        if (usage & properties.usage) {
            wgpu::Buffer b = memory.CreateBuffer(&bufferDesc);
            EXPECT_EQ(b.GetUsage(), usage);
        } else {
            ASSERT_DEVICE_ERROR(memory.CreateBuffer(&bufferDesc));
        }
    }
}

// Tests that creating SharedBufferMemory emits a specific error message if Uniform usage specified.
TEST_P(SharedBufferMemoryTests, UniformUsageValidation) {
    wgpu::SharedBufferMemory memory =
        GetParam().mBackend->CreateSharedBufferMemory(device, kMapWriteUsages, kBufferSize);
    wgpu::SharedBufferMemoryProperties properties;
    memory.GetProperties(&properties);

    wgpu::BufferDescriptor bufferDesc = {};
    bufferDesc.size = properties.size;
    bufferDesc.usage = properties.usage | wgpu::BufferUsage::Uniform;

    ASSERT_DEVICE_ERROR_MSG(memory.CreateBuffer(&bufferDesc), HasSubstr("Uniform"));
}

// Ensure that EndAccess cannot be called on a mapped or pending mapped buffer.
TEST_P(SharedBufferMemoryTests, CallEndAccessOnMappedBuffer) {
    wgpu::SharedBufferMemory memory =
        GetParam().mBackend->CreateSharedBufferMemory(device, kMapWriteUsages, kBufferSize);
    wgpu::Buffer buffer = memory.CreateBuffer();
    wgpu::SharedBufferMemoryBeginAccessDescriptor desc;
    memory.BeginAccess(buffer, &desc);

    bool done = false;
    buffer.MapAsync(wgpu::MapMode::Write, 0, sizeof(uint32_t),
                    wgpu::CallbackMode::AllowProcessEvents,
                    [&done](wgpu::MapAsyncStatus status, wgpu::StringView) {
                        ASSERT_EQ(status, wgpu::MapAsyncStatus::Success);
                        done = true;
                    });

    // Calling EndAccess should generate an error even if the buffer has not completed being mapped.
    wgpu::SharedBufferMemoryEndAccessState state;
    ASSERT_DEVICE_ERROR(memory.EndAccess(buffer, &state));

    while (!done) {
        WaitABit();
    }

    // Calling EndAccess should generate an error after being mapped.
    ASSERT_DEVICE_ERROR(memory.EndAccess(buffer, &state));
}

// Ensure no queue usage can occur before calling BeginAccess.
TEST_P(SharedBufferMemoryTests, EnsureNoQueueUsageBeforeBeginAccess) {
    // We can't test this invalid scenario without validation.
    DAWN_SUPPRESS_TEST_IF(HasToggleEnabled("skip_validation"));

    wgpu::SharedBufferMemory memory =
        GetParam().mBackend->CreateSharedBufferMemory(device, kMapWriteUsages, kBufferSize);
    wgpu::Buffer sharedBuffer = memory.CreateBuffer();

    wgpu::BufferDescriptor descriptor;
    descriptor.size = kBufferSize;
    descriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    // Using the buffer in a submit without calling BeginAccess should cause an error.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(sharedBuffer, 0, buffer, 0, kBufferSize);
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    ASSERT_DEVICE_ERROR(queue.Submit(1, &commandBuffer));
}

// Ensure mapping cannot occur before calling BeginAccess.
TEST_P(SharedBufferMemoryTests, EnsureNoMapUsageBeforeBeginAccess) {
    wgpu::SharedBufferMemory memory =
        GetParam().mBackend->CreateSharedBufferMemory(device, kMapWriteUsages, kBufferSize);
    wgpu::Buffer sharedBuffer = memory.CreateBuffer();

    // Mapping a buffer without calling BeginAccess should cause an error.
    ASSERT_DEVICE_ERROR(sharedBuffer.MapAsync(wgpu::MapMode::Write, 0, 4,
                                              wgpu::CallbackMode::AllowProcessEvents,
                                              [](wgpu::MapAsyncStatus status, wgpu::StringView) {
                                                  ASSERT_EQ(status, wgpu::MapAsyncStatus::Error);
                                              }));
}

// Ensure multiple buffers created from a SharedBufferMemory cannot be accessed simultaneously.
TEST_P(SharedBufferMemoryTests, EnsureNoSimultaneousAccess) {
    wgpu::SharedBufferMemory memory =
        GetParam().mBackend->CreateSharedBufferMemory(device, kMapWriteUsages, kBufferSize);
    wgpu::Buffer sharedBuffer = memory.CreateBuffer();

    wgpu::SharedBufferMemoryBeginAccessDescriptor desc;
    memory.BeginAccess(sharedBuffer, &desc);

    wgpu::Buffer sharedBuffer2 = memory.CreateBuffer();
    ASSERT_DEVICE_ERROR(memory.BeginAccess(sharedBuffer2, &desc));
}

// Validate that calling EndAccess before BeginAccess produces an error.
TEST_P(SharedBufferMemoryTests, EnsureNoEndAccessBeforeBeginAccess) {
    wgpu::SharedBufferMemory memory =
        GetParam().mBackend->CreateSharedBufferMemory(device, kMapWriteUsages, kBufferSize);
    wgpu::Buffer buffer = memory.CreateBuffer();

    wgpu::SharedBufferMemoryEndAccessState state;
    ASSERT_DEVICE_ERROR(memory.EndAccess(buffer, &state));
}

// Validate that calling EndAccess on a different buffer created from the same Shared is invalid.
TEST_P(SharedBufferMemoryTests, EndAccessOnDifferentBuffer) {
    wgpu::SharedBufferMemory memory =
        GetParam().mBackend->CreateSharedBufferMemory(device, kMapWriteUsages, kBufferSize);
    wgpu::Buffer buffer = memory.CreateBuffer();
    wgpu::Buffer buffer2 = memory.CreateBuffer();

    wgpu::SharedBufferMemoryBeginAccessDescriptor desc;
    memory.BeginAccess(buffer, &desc);

    wgpu::SharedBufferMemoryEndAccessState state;
    ASSERT_DEVICE_ERROR(memory.EndAccess(buffer2, &state));

    wgpu::BufferDescriptor descriptor;
    descriptor.size = kBufferSize;
    descriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dst = device.CreateBuffer(&descriptor);

    // Use the buffer in a submit.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(buffer, 0, dst, 0, kBufferSize);
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    // Ensure that calling EndAccess on the correct buffer still returns a fence.
    memory.EndAccess(buffer, &state);
    ASSERT_EQ(state.fenceCount, static_cast<size_t>(1));
    ASSERT_NE(state.fences[0], nullptr);
}

// Validate that calling BeginAccess twice produces an error.
TEST_P(SharedBufferMemoryTests, EnsureNoDuplicateBeginAccessCalls) {
    wgpu::SharedBufferMemory memory =
        GetParam().mBackend->CreateSharedBufferMemory(device, kMapWriteUsages, kBufferSize);
    wgpu::Buffer buffer = memory.CreateBuffer();

    wgpu::SharedBufferMemoryBeginAccessDescriptor desc;
    memory.BeginAccess(buffer, &desc);
    ASSERT_DEVICE_ERROR(memory.BeginAccess(buffer, &desc));
}

// Ensure the BeginAccessDescriptor initialized parameter preserves or clears the buffer as
// necessary.
TEST_P(SharedBufferMemoryTests, BeginAccessInitialization) {
    // TODO(dawn:2382): Investigate why this test fails on Windows Intel bots.
    DAWN_SUPPRESS_TEST_IF(IsIntel() && IsD3D12());

    // Create a buffer with initialized data.
    wgpu::SharedBufferMemory memory =
        GetParam().mBackend->CreateSharedBufferMemory(device, kMapWriteUsages, kBufferSize);
    wgpu::Buffer buffer = memory.CreateBuffer();

    // Write data into the shared buffer.
    wgpu::SharedBufferMemoryBeginAccessDescriptor beginAccessDesc;
    beginAccessDesc.initialized = false;
    memory.BeginAccess(buffer, &beginAccessDesc);

    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, kBufferSize);

    uint32_t* mappedData = static_cast<uint32_t*>(buffer.GetMappedRange(0, kBufferSize));
    memcpy(mappedData, &kBufferData, kBufferSize);
    buffer.Unmap();

    wgpu::SharedBufferMemoryEndAccessState endState;
    memory.EndAccess(buffer, &endState);

    EXPECT_EQ(endState.initialized, true);

    // Pass fences from the previous operation to the next BeginAccessDescriptor to ensure
    // operations are complete.
    std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
    for (size_t j = 0; j < endState.fenceCount; ++j) {
        sharedFences[j] = GetParam().mBackend->ImportFenceTo(device, endState.fences[j]);
    }
    beginAccessDesc.fenceCount = sharedFences.size();
    beginAccessDesc.fences = sharedFences.data();
    beginAccessDesc.signaledValues = endState.signaledValues;

    // Create a second buffer from the SharedBuffer memory, which will be marked as initialized in
    // the BeginAccessDescriptor. This buffer should preserve the data from the previous copy.
    wgpu::Buffer buffer2 = memory.CreateBuffer();
    beginAccessDesc.initialized = true;
    memory.BeginAccess(buffer2, &beginAccessDesc);
    // The buffer should contain the data from initialization.
    EXPECT_BUFFER_U32_EQ(kBufferData, buffer2, 0);
    memory.EndAccess(buffer2, &endState);

    // Pass fences from the previous operation to the next BeginAccessDescriptor to ensure
    // operations are complete.
    std::vector<wgpu::SharedFence> sharedFences2(endState.fenceCount);
    for (size_t j = 0; j < endState.fenceCount; ++j) {
        sharedFences2[j] = GetParam().mBackend->ImportFenceTo(device, endState.fences[j]);
    }
    beginAccessDesc.fenceCount = sharedFences2.size();
    beginAccessDesc.fences = sharedFences2.data();
    beginAccessDesc.signaledValues = endState.signaledValues;

    // Create another buffer from the SharedBufferMemory, but mark it uninitialized in the
    // BeginAccessDescriptor.
    wgpu::Buffer buffer3 = memory.CreateBuffer();
    beginAccessDesc.initialized = false;
    memory.BeginAccess(buffer3, &beginAccessDesc);
    // The buffer should be zero'd out because the BeginAccessDescriptor stated it was
    // uninitialized.
    EXPECT_BUFFER_U32_EQ(0, buffer3, 0);
    memory.EndAccess(buffer3, &endState);
}

// Tests that an unininitialized buffer that is not read or writt
TEST_P(SharedBufferMemoryTests, UninitializedBufferRemainsUninitialized) {
    // Create a buffer with initialized data.
    wgpu::SharedBufferMemory memory =
        GetParam().mBackend->CreateSharedBufferMemory(device, kMapWriteUsages, kBufferSize);
    wgpu::Buffer buffer = memory.CreateBuffer();

    wgpu::SharedBufferMemoryBeginAccessDescriptor beginAccessDesc;
    beginAccessDesc.initialized = false;
    memory.BeginAccess(buffer, &beginAccessDesc);
    wgpu::SharedBufferMemoryEndAccessState state;
    memory.EndAccess(buffer, &state);
    ASSERT_EQ(state.initialized, false);
}

// Read and write a buffer with MapWrite and CopySrc usages.
TEST_P(SharedBufferMemoryTests, ReadWriteSharedMapWriteBuffer) {
    // Create buffer buffer with initialized data.
    wgpu::SharedBufferMemory memory = GetParam().mBackend->CreateSharedBufferMemory(
        device, kMapWriteUsages, kBufferSize, kBufferData);
    wgpu::Buffer buffer = memory.CreateBuffer();

    // Begin access and check the contents within Dawn.
    wgpu::SharedBufferMemoryBeginAccessDescriptor beginAccessDesc;
    beginAccessDesc.initialized = true;
    memory.BeginAccess(buffer, &beginAccessDesc);
    EXPECT_BUFFER_U32_EQ(kBufferData, buffer, 0);

    MapAsyncAndWait(buffer, wgpu::MapMode::Write, 0, kBufferSize);

    uint32_t* mappedData = static_cast<uint32_t*>(buffer.GetMappedRange(0, kBufferSize));
    memcpy(mappedData, &kBufferData2, kBufferSize);
    buffer.Unmap();

    EXPECT_BUFFER_U32_EQ(kBufferData2, buffer, 0);
}

// Read and write a buffer with MapRead and CopyDst usages.
TEST_P(SharedBufferMemoryTests, ReadWriteSharedMapReadBuffer) {
    // Create buffer buffer with initialized data.
    wgpu::SharedBufferMemory memory = GetParam().mBackend->CreateSharedBufferMemory(
        device, kMapReadUsages, kBufferSize, kBufferData);
    wgpu::Buffer buffer = memory.CreateBuffer();

    // Begin access and check the contents within Dawn.
    wgpu::SharedBufferMemoryBeginAccessDescriptor beginAccessDesc;
    beginAccessDesc.initialized = true;
    memory.BeginAccess(buffer, &beginAccessDesc);

    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, kBufferSize);

    const uint32_t* mappedData =
        static_cast<const uint32_t*>(buffer.GetConstMappedRange(0, kBufferSize));
    ASSERT_EQ(*mappedData, kBufferData);

    buffer.Unmap();

    // Copy new data into the buffer from within Dawn and check the contents.
    wgpu::Buffer dawnBuffer =
        utils::CreateBufferFromData(device, &kBufferData2, kBufferSize, wgpu::BufferUsage::CopySrc);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(dawnBuffer, 0, buffer, 0, 4);
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, kBufferSize);

    mappedData = static_cast<const uint32_t*>(buffer.GetConstMappedRange(0, kBufferSize));
    ASSERT_EQ(*mappedData, kBufferData2);
}

// Test ensures that a shader can read and write from a shared storage buffer.
TEST_P(SharedBufferMemoryTests, ReadWriteSharedStorageBuffer) {
    wgpu::SharedBufferMemory memory = GetParam().mBackend->CreateSharedBufferMemory(
        device, kStorageUsages, kBufferSize, kBufferData);
    wgpu::Buffer buffer = memory.CreateBuffer();

    // Begin access and check the contents within Dawn.
    wgpu::SharedBufferMemoryBeginAccessDescriptor beginAccessDesc;
    beginAccessDesc.initialized = true;
    memory.BeginAccess(buffer, &beginAccessDesc);

    wgpu::ComputePipelineDescriptor pipelineDescriptor;

    // This compute shader reads from the shared storage buffer and increments it by one.
    pipelineDescriptor.compute.module = utils::CreateShaderModule(device, R"(
    struct OutputBuffer {
        value : u32
    }

    @group(0) @binding(0) var<storage, read_write> outputBuffer : OutputBuffer;

    @compute @workgroup_size(1) fn main() {
        outputBuffer.value = outputBuffer.value + 1u;
    })");

    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDescriptor);
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});

    wgpu::CommandBuffer commands;
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();
    commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The storage buffer should have been incremented by one in the compute shader.
    EXPECT_BUFFER_U32_EQ(kBufferData + 1, buffer, 0);
}

TEST_P(SharedBufferMemoryTests, ImportExportSharedFences) {
    wgpu::SharedBufferMemory memory = GetParam().mBackend->CreateSharedBufferMemory(
        device, kStorageUsages, kBufferSize, kBufferData);
    wgpu::Buffer buffer = memory.CreateBuffer();
    wgpu::SharedBufferMemoryEndAccessState endState;

    // Each loop checks the value of a storage buffer is correct and increments the value in a
    // compute shader. Every loop exports a shared fence, which will be imported in the next loop.
    for (int i = 0; i < 5; i++) {
        // Begin access and check the contents within Dawn.
        wgpu::SharedBufferMemoryBeginAccessDescriptor beginAccessDesc;
        beginAccessDesc.initialized = true;

        // Get any fences from the previous loop's SharedBufferMemoryEndAccessState.
        std::vector<wgpu::SharedFence> sharedFences(endState.fenceCount);
        for (size_t j = 0; j < endState.fenceCount; ++j) {
            sharedFences[j] = GetParam().mBackend->ImportFenceTo(device, endState.fences[j]);
        }
        beginAccessDesc.fenceCount = sharedFences.size();
        beginAccessDesc.fences = sharedFences.data();
        beginAccessDesc.signaledValues = endState.signaledValues;
        memory.BeginAccess(buffer, &beginAccessDesc);

        // The storage buffer should be incremented by one per loop
        EXPECT_BUFFER_U32_EQ(kBufferData + i, buffer, 0);

        wgpu::ComputePipelineDescriptor pipelineDescriptor;

        // This compute shader reads from the shared storage buffer and increments it by one.
        pipelineDescriptor.compute.module = utils::CreateShaderModule(device, R"(
        struct OutputBuffer {
            value : u32
        }

        @group(0) @binding(0) var<storage, read_write> outputBuffer : OutputBuffer;

        @compute @workgroup_size(1) fn main() {
            outputBuffer.value = outputBuffer.value + 1u;
        })");

        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDescriptor);
        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer}});

        wgpu::CommandBuffer commands;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();
        commands = encoder.Finish();
        queue.Submit(1, &commands);

        memory.EndAccess(buffer, &endState);
    }
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SharedBufferMemoryTests);

}  // anonymous namespace
}  // namespace dawn
