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

#include <chrono>
#include <string>
#include <thread>
#include <utility>

#include "dawn/native/DawnNative.h"
#include "dawn/native/SharedResourceMemory.h"
#include "dawn/native/dawn_platform_autogen.h"
#include "dawn/tests/unittests/native/mocks/BufferMock.h"
#include "dawn/tests/unittests/native/mocks/DawnMockTest.h"
#include "dawn/tests/unittests/native/mocks/TextureMock.h"
#include "gtest/gtest.h"

namespace dawn::native {
namespace {

using ::testing::ByMove;
using ::testing::DoDefault;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrEq;

class MemoryDumpMock : public MemoryDump {
  public:
    MemoryDumpMock() {
        ON_CALL(*this, AddScalar)
            .WillByDefault(
                [this](const char* name, const char* key, const char* units, uint64_t value) {
                    if (key == MemoryDump::kNameSize && units == MemoryDump::kUnitsBytes) {
                        mTotalSize += value;
                    }
                });
    }

    MOCK_METHOD(void,
                AddScalar,
                (const char* name, const char* key, const char* units, uint64_t value),
                (override));

    void AddString(const char* name, const char* key, const std::string& value) override {}

    uint64_t GetTotalSize() const { return mTotalSize; }

  private:
    uint64_t mTotalSize = 0;
};

using MemoryInstrumentationTest = DawnMockTest;

TEST_F(MemoryInstrumentationTest, DumpMemoryStatistics) {
    MemoryDumpMock memoryDumpMock;

    auto textureLabel = [this](const wgpu::Texture& texture) {
        return absl::StrFormat("device_%p/texture_%p", device.Get(), texture.Get());
    };

    auto bufferLabel = [this](const wgpu::Buffer& buffer) {
        return absl::StrFormat("device_%p/buffer_%p", device.Get(), buffer.Get());
    };

    // Create a buffer and destroy it and check that its size is not counted.
    constexpr uint64_t kBufferSize = 31;
    constexpr wgpu::BufferDescriptor kBufferDesc = {
        .usage = wgpu::BufferUsage::Uniform,
        .size = kBufferSize,
    };
    wgpu::Buffer destroyedBuffer = device.CreateBuffer(&kBufferDesc);
    EXPECT_TRUE(destroyedBuffer);
    destroyedBuffer.Destroy();

    // Create a buffer whose allocated size is larger than requested size and check that allocated
    // size is counted.
    constexpr uint64_t kBufferAllocatedSize = 32;
    Ref<BufferMock> bufferMock = AcquireRef(
        new NiceMock<BufferMock>(mDeviceMock, FromCppAPI(&kBufferDesc), kBufferAllocatedSize));
    EXPECT_CALL(*mDeviceMock, CreateBufferImpl).WillOnce(Return(ByMove(std::move(bufferMock))));
    wgpu::Buffer buffer = device.CreateBuffer(&kBufferDesc);

    EXPECT_CALL(memoryDumpMock, AddScalar(StrEq(bufferLabel(buffer)), MemoryDump::kNameSize,
                                          MemoryDump::kUnitsBytes, kBufferAllocatedSize));

    // Create a mip-mapped texture and check that all mip level sizes are counted.
    constexpr wgpu::TextureFormat kRG8UnormTextureFormat = wgpu::TextureFormat::RG8Unorm;
    const wgpu::TextureDescriptor kMipmappedTextureDesc = {
        .usage = wgpu::TextureUsage::RenderAttachment,
        .size = {.width = 30, .height = 20, .depthOrArrayLayers = 10},
        .format = kRG8UnormTextureFormat,
        .mipLevelCount = 5,
        .viewFormatCount = 1,
        .viewFormats = &kRG8UnormTextureFormat,
    };
    wgpu::Texture mipmappedTexture = device.CreateTexture(&kMipmappedTextureDesc);

    // Byte size of entire mip chain =
    // ((level0 width * level0 height) + ... + (levelN width * levelN height)) * bpp * array layers.
    constexpr uint64_t kMipmappedTextureSize =
        (((30 * 20) + (15 * 10) + (7 * 5) + (3 * 2) + (1 * 1)) * 2) * 10;  // 15840
    EXPECT_CALL(memoryDumpMock,
                AddScalar(StrEq(textureLabel(mipmappedTexture)), MemoryDump::kNameSize,
                          MemoryDump::kUnitsBytes, kMipmappedTextureSize));

    // Create a multi-sampled texture and check that sample count is taken into account.
    const wgpu::TextureDescriptor kMultisampleTextureDesc = {
        .usage = wgpu::TextureUsage::RenderAttachment,
        .size = {.width = 30, .height = 20},
        .format = kRG8UnormTextureFormat,
        .sampleCount = 4,
        .viewFormatCount = 1,
        .viewFormats = &kRG8UnormTextureFormat,
    };
    wgpu::Texture multisampleTexture = device.CreateTexture(&kMultisampleTextureDesc);
    // Expected size = width(30) * height(20) * bytes per pixel(2) * sample count(4).
    constexpr uint64_t kMultisampleTextureSize = 30 * 20 * 2 * 4;
    EXPECT_CALL(memoryDumpMock,
                AddScalar(StrEq(textureLabel(multisampleTexture)), MemoryDump::kNameSize,
                          MemoryDump::kUnitsBytes, kMultisampleTextureSize));

    // Create a compressed texture and check that counted size is correct.
    mDeviceMock->ForceEnableFeatureForTesting(Feature::TextureCompressionETC2);
    constexpr wgpu::TextureFormat kETC2TextureFormat = wgpu::TextureFormat::ETC2RGB8Unorm;
    const wgpu::TextureDescriptor kETC2TextureDesc = {
        .usage = wgpu::TextureUsage::CopySrc,
        .size = {.width = 32, .height = 32},
        .format = kETC2TextureFormat,
        .viewFormatCount = 1,
        .viewFormats = &kETC2TextureFormat,
    };
    wgpu::Texture etc2Texture = device.CreateTexture(&kETC2TextureDesc);
    // Expected size = (width / block width) * (height / block height) * bytes per block.
    constexpr uint64_t kETC2TextureSize = (32 / 4) * (32 / 4) * 8;
    EXPECT_CALL(memoryDumpMock, AddScalar(StrEq(textureLabel(etc2Texture)), MemoryDump::kNameSize,
                                          MemoryDump::kUnitsBytes, kETC2TextureSize));

    // Create a texture and destroy it and check that its info is not emitted.
    wgpu::Texture destroyedTexture = device.CreateTexture(&kMipmappedTextureDesc);
    EXPECT_TRUE(destroyedTexture);
    destroyedTexture.Destroy();

    // Create a shared resource memory texture and check that its size is not counted.
    constexpr wgpu::TextureFormat kRGBA8UnormTextureFormat = wgpu::TextureFormat::RGBA8Unorm;
    const wgpu::TextureDescriptor kSharedTextureDesc = {
        .usage = wgpu::TextureUsage::TextureBinding,
        .size = {.width = 30, .height = 20},
        .format = kRGBA8UnormTextureFormat,
        .viewFormatCount = 1,
        .viewFormats = &kRGBA8UnormTextureFormat,
    };
    Ref<TextureMock> sharedTextureMock =
        AcquireRef(new NiceMock<TextureMock>(mDeviceMock, FromCppAPI(&kSharedTextureDesc)));
    sharedTextureMock->SetSharedResourceMemoryContentsForTesting(
        AcquireRef(new SharedResourceMemoryContents(nullptr)));
    EXPECT_CALL(*mDeviceMock, CreateTextureImpl)
        .WillOnce(Return(ByMove(std::move(sharedTextureMock))))
        .WillRepeatedly(DoDefault());
    wgpu::Texture sharedTexture = device.CreateTexture(&kSharedTextureDesc);
    EXPECT_CALL(memoryDumpMock, AddScalar(StrEq(textureLabel(sharedTexture)), MemoryDump::kNameSize,
                                          MemoryDump::kUnitsBytes, /*size=*/0));

    // Create a transient attachment (memoryless) texture and check that its size is not counted.
    mDeviceMock->ForceEnableFeatureForTesting(Feature::TransientAttachments);
    const wgpu::TextureDescriptor kTransientAttachmentTextureDesc = {
        .usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TransientAttachment,
        .size = {.width = 30, .height = 20},
        .format = kRGBA8UnormTextureFormat,
        .viewFormatCount = 1,
        .viewFormats = &kRGBA8UnormTextureFormat,
    };
    wgpu::Texture transientAttachmentTexture =
        device.CreateTexture(&kTransientAttachmentTextureDesc);
    EXPECT_CALL(memoryDumpMock,
                AddScalar(StrEq(textureLabel(transientAttachmentTexture)), MemoryDump::kNameSize,
                          MemoryDump::kUnitsBytes, /*size=*/0));

    DumpMemoryStatistics(device.Get(), &memoryDumpMock);

    EXPECT_EQ(memoryDumpMock.GetTotalSize(), kBufferAllocatedSize + kMipmappedTextureSize +
                                                 kMultisampleTextureSize + kETC2TextureSize);

    // Check that ComputeEstimatedMemoryUsage() matches the memory dump total size.
    EXPECT_EQ(
        ComputeEstimatedMemoryUsage(device.Get()),
        kBufferAllocatedSize + kMipmappedTextureSize + kMultisampleTextureSize + kETC2TextureSize);
}

TEST_F(MemoryInstrumentationTest, ReduceMemoryUsage) {
    constexpr uint64_t kBufferSize = 32;
    constexpr wgpu::BufferDescriptor kBufferDesc = {
        .usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
        .size = kBufferSize,
    };
    wgpu::Buffer uniformBuffer = device.CreateBuffer(&kBufferDesc);
    EXPECT_TRUE(uniformBuffer);

    std::array<uint8_t, kBufferSize> zeroes = {};
    device.GetQueue().WriteBuffer(uniformBuffer, 0, zeroes.data(), zeroes.size());
    device.GetQueue().Submit(0, nullptr);

    uniformBuffer.Destroy();

    mDeviceMock->GetInstance()->APIProcessEvents();

    // DynamicUploader buffers will still be alive.
    EXPECT_GT(ComputeEstimatedMemoryUsage(device.Get()), uint64_t(0));
    ReduceMemoryUsage(device.Get());
    // But not any more.
    EXPECT_EQ(ComputeEstimatedMemoryUsage(device.Get()), uint64_t(0));

    // Check that DynamicUploader buffer is recreated again.
    uniformBuffer = device.CreateBuffer(&kBufferDesc);
    EXPECT_TRUE(uniformBuffer);

    device.GetQueue().WriteBuffer(uniformBuffer, 0, zeroes.data(), zeroes.size());
    device.GetQueue().Submit(0, nullptr);

    uniformBuffer.Destroy();

    mDeviceMock->GetInstance()->APIProcessEvents();

    EXPECT_GT(ComputeEstimatedMemoryUsage(device.Get()), uint64_t(0));
}

}  // namespace
}  // namespace dawn::native
