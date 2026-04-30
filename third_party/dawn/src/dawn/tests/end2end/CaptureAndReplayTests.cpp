// Copyright 2025 The Dawn & Tint Authors
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

#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "dawn/native/WebGPUBackend.h"
#include "dawn/replay/Replay.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class CaptureAndReplayTests : public DawnTest {
  public:
    void SetUp() override {
        DawnTest::SetUp();

        // This test is already testing capture and replay, we don't need to wrap it and test again.
        DAWN_TEST_UNSUPPORTED_IF(IsCaptureReplayCheckingEnabled());

        // StartCapture/EndCapture are dawn::native APIs that require a native
        // WGPUDevice. When the wire is enabled, device.Get() returns a wire
        // client device which cannot be cast to a native device, causing a crash.
        // TODO(crbug.com/452924800): Remove once these tests work properly with
        // the WebGPU on WebGPU backend with wire.
        DAWN_SUPPRESS_TEST_IF(UsesWire());
    }

    class Capture {
      public:
        Capture(const std::string& commandData, const std::string& contentData)
            : mCommandData(commandData), mContentData(contentData) {}

        std::unique_ptr<replay::Replay> Replay(wgpu::Device device) {
            std::istringstream commandIStream(mCommandData);
            std::istringstream contentIStream(mContentData);

            auto capture = replay::Capture::Create(commandIStream, mCommandData.size(),
                                                   contentIStream, mContentData.size());
            std::unique_ptr<replay::Replay> replay =
                replay::Replay::Create(device, std::move(capture));

            bool result = replay->Play();
            EXPECT_TRUE(result);
            return replay;
        }

      private:
        std::string mCommandData;
        std::string mContentData;
    };

    class Recorder {
      public:
        static Recorder CreateAndStart(wgpu::Device device) { return Recorder(device); }

        Capture Finish() {
            native::webgpu::EndCapture(mDevice.Get());
            return Capture(mCommandStream.str(), mContentStream.str());
        }

      private:
        explicit Recorder(wgpu::Device device) : mDevice(device) {
            native::webgpu::StartCapture(device.Get(), mCommandStream, mContentStream);
        }

        wgpu::Device mDevice;
        std::ostringstream mCommandStream;
        std::ostringstream mContentStream;
    };

    wgpu::Buffer CreateBuffer(const char* label,
                              uint64_t size,
                              wgpu::BufferUsage usage,
                              bool mappedAtCreation = false) {
        wgpu::BufferDescriptor descriptor;
        descriptor.label = label;
        descriptor.size = size;
        descriptor.usage = usage;
        descriptor.mappedAtCreation = mappedAtCreation;
        return device.CreateBuffer(&descriptor);
    }

    wgpu::Texture CreateTexture(const char* label,
                                const wgpu::Extent3D& size,
                                wgpu::TextureFormat format,
                                wgpu::TextureUsage usage) {
        wgpu::TextureDescriptor textureDesc;
        textureDesc.label = label;
        textureDesc.size = size;
        textureDesc.format = format;
        textureDesc.usage = usage;
        return device.CreateTexture(&textureDesc);
    }

    template <typename T>
    void WriteFullTexture(wgpu::Texture texture,
                          wgpu::TextureFormat format,
                          const wgpu::Extent3D& size,
                          const T& data) {
        ASSERT_TRUE(sizeof(data) > 0);
        uint32_t bytesPerBlock = utils::GetTexelBlockSizeInBytes(format);
        wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
            utils::CreateTexelCopyBufferLayout(0, bytesPerBlock * size.width);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        queue.WriteTexture(&texelCopyTextureInfo, data, sizeof(data), &texelCopyBufferLayout,
                           &size);
    }

    template <typename T>
    void ExpectBufferEQ(replay::Replay* replay, const char* label, const T& expected) {
        ASSERT_TRUE(sizeof(expected) > 0);
        wgpu::Buffer buffer = replay->GetObjectByLabel<wgpu::Buffer>(label);
        ASSERT_NE(buffer, nullptr);

        EXPECT_BUFFER_U8_RANGE_EQ(expected, buffer, 0, sizeof(expected));
    }

    template <typename T>
    void ExpectTextureEQ(replay::Replay* replay,
                         const char* label,
                         const wgpu::Extent3D& size,
                         const T& expected) {
        wgpu::Texture texture = replay->GetObjectByLabel<wgpu::Texture>(label);
        ASSERT_NE(texture, nullptr);
        ASSERT_TRUE(size.width > 0 && size.height > 0 && size.depthOrArrayLayers > 0);
        EXPECT_TEXTURE_EQ(&expected[0], texture, {0, 0}, size, 0, wgpu::TextureAspect::All);
    }
};

// During capture, makes a buffer, puts data in it.
// Then, replays and checks the data is correct.
TEST_P(CaptureAndReplayTests, Basic) {
    const char* label = "MyBuffer";
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};

    auto recorder = Recorder::CreateAndStart(device);

    wgpu::Buffer buffer = CreateBuffer(label, 4, wgpu::BufferUsage::CopyDst);
    queue.WriteBuffer(buffer, 0, &myData, sizeof(myData));

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    ExpectBufferEQ(replay.get(), label, myData);
}

// During capture, makes a buffer, puts data in it.
// Then, replays and checks the data is correct.
// It uses on label with 5 characters which may skew alignment.
TEST_P(CaptureAndReplayTests, NonMultipleOf4LabelLength) {
    const char* label = "MyBuf";
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    wgpu::Buffer buffer = CreateBuffer(label, 4, wgpu::BufferUsage::CopyDst);
    queue.WriteBuffer(buffer, 0, &myData, sizeof(myData));

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    ExpectBufferEQ(replay.get(), label, myData);
}

// Before capture, creates a buffer and sets half of it with WriteBuffer.
// It then starts a capture and writes the other half with WriteBuffer.
// On replay both halves should have the correct data..
TEST_P(CaptureAndReplayTests, StartCaptureAfterBufferCreationWriteBuffer) {
    const char* label = "MyBuffer";
    const uint8_t myData0[] = {0x11, 0x22, 0x33, 0x44};
    const uint8_t myData1[] = {0x55, 0x66, 0x77, 0x88};

    wgpu::Buffer buffer = CreateBuffer(label, 8, wgpu::BufferUsage::CopyDst);
    queue.WriteBuffer(buffer, 0, &myData0, sizeof(myData0));

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.WriteBuffer(buffer, 4, &myData1, sizeof(myData1));

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    ExpectBufferEQ(replay.get(), label, expected);
}

// Before capture, creates a buffer and sets half of it with mappedAtCreation.
// It then starts a capture and writes the other half with WriteBuffer.
// On replay both halves should have the correct data..
TEST_P(CaptureAndReplayTests, StartCaptureAfterBufferCreationMappedAtCreation) {
    const char* label = "MyBuffer";
    const uint8_t myData0[] = {0x11, 0x22, 0x33, 0x44};
    const uint8_t myData1[] = {0x55, 0x66, 0x77, 0x88};

    wgpu::Buffer buffer = CreateBuffer(label, 8, wgpu::BufferUsage::CopyDst, true);
    std::memcpy(buffer.GetMappedRange(), myData0, sizeof(myData0));
    buffer.Unmap();

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.WriteBuffer(buffer, 4, &myData1, sizeof(myData1));

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    ExpectBufferEQ(replay.get(), label, expected);
}

// Before capture, creates a buffer and sets half of it with a compute shader.
// It then starts a capture and writes the other half with WriteBuffer.
// On replay both halves should have the correct data..
TEST_P(CaptureAndReplayTests, StartCaptureAfterBufferCreationComputeShader) {
    const char* label = "MyBuffer";

    wgpu::Buffer buffer =
        CreateBuffer(label, 8, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst);

    const char* shader = R"(
        @group(0) @binding(0) var<storage, read_write> result : u32;

        @compute @workgroup_size(1) fn main() {
            result = 0x44332211;
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, buffer},
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

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    const uint8_t myData[] = {0x55, 0x66, 0x77, 0x88};
    queue.WriteBuffer(buffer, 4, &myData, sizeof(myData));

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    ExpectBufferEQ(replay.get(), label, expected);
}

// Before capture, creates a buffer and sets half of it with copyBufferToBuffer.
// It then starts a capture and writes the other half with WriteBuffer.
// On replay both halves should have the correct data..
TEST_P(CaptureAndReplayTests, StartCaptureAfterBufferCreationCopyB2B) {
    const char* srcLabel = "SrcBuffer";
    const char* dstLabel = "DstBuffer";

    wgpu::Buffer dstBuffer = CreateBuffer(dstLabel, 8, wgpu::BufferUsage::CopyDst);
    wgpu::Buffer srcBuffer =
        CreateBuffer(srcLabel, 8, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);
    const uint8_t myData1[] = {0x11, 0x22, 0x33, 0x44};
    queue.WriteBuffer(srcBuffer, 0, &myData1, sizeof(myData1));

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(srcBuffer, 0, dstBuffer, 0, 4);
        commands = encoder.Finish();
    }
    queue.Submit(1, &commands);

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    const uint8_t myData2[] = {0x55, 0x66, 0x77, 0x88};
    queue.WriteBuffer(dstBuffer, 4, &myData2, sizeof(myData2));

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    ExpectBufferEQ(replay.get(), dstLabel, expected);
}

// During first capture, makes a buffer, puts data in it.
// During 2nd capture, puts a little data in the same buffer.
// Then, replays the 2nd capture and checks the data is correct.
TEST_P(CaptureAndReplayTests, TwoCaptures) {
    const char* label = "MyBuffer";
    const uint8_t myData1[] = {0x11, 0x22, 0x33, 0x44};
    const uint8_t myData2[] = {0x55, 0x66, 0x77, 0x88};

    wgpu::Buffer buffer;

    {
        auto recorder = Recorder::CreateAndStart(device);

        buffer = CreateBuffer(label, 8, wgpu::BufferUsage::CopyDst);
        queue.WriteBuffer(buffer, 0, &myData1, sizeof(myData1));

        recorder.Finish();
    }

    {
        auto recorder = Recorder::CreateAndStart(device);

        queue.WriteBuffer(buffer, 4, &myData2, sizeof(myData2));

        auto capture = recorder.Finish();
        auto replay = capture.Replay(device);

        uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
        ExpectBufferEQ(replay.get(), label, expected);
    }
}

// We make a buffer before capture. During capture
// write map it, put data in it, copyB2B from it,
// write map it again, put different data in it, copyB2B from it,
// then check the data is correct on replay.
TEST_P(CaptureAndReplayTests, MapWrite) {
    const uint8_t myData1[] = {0x11, 0x22, 0x33, 0x44};
    const uint8_t myData2[] = {0x55, 0x66, 0x77, 0x88};

    wgpu::Buffer srcBuffer =
        CreateBuffer("srcBuffer", 4, wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc);
    wgpu::Buffer dstBuffer = CreateBuffer("dstBuffer", 4, wgpu::BufferUsage::CopyDst);

    auto recorder = Recorder::CreateAndStart(device);

    MapAsyncAndWait(srcBuffer, wgpu::MapMode::Write, 0, 4);
    srcBuffer.WriteMappedRange(0, &myData1, sizeof(myData1));
    srcBuffer.Unmap();

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(srcBuffer, 0, dstBuffer, 0, 4);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    MapAsyncAndWait(srcBuffer, wgpu::MapMode::Write, 0, 4);
    srcBuffer.WriteMappedRange(0, &myData2, sizeof(myData2));
    srcBuffer.Unmap();

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(srcBuffer, 0, dstBuffer, 0, 4);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    ExpectBufferEQ(replay.get(), "srcBuffer", myData2);
    ExpectBufferEQ(replay.get(), "dstBuffer", myData2);
}

// We make 2 buffers before capture. During capture we map one buffer
// put some data it in via map/unmap. We then copy from that buffer to the other buffer.
// On replay check the data is correct.
TEST_P(CaptureAndReplayTests, CaptureWithMapWriteDuringCapture) {
    const char* srcLabel = "srcBuffer";
    const char* dstLabel = "dstBuffer";
    const uint8_t myData1[] = {0x11, 0x22, 0x33, 0x44};
    const uint8_t myData2[] = {0x55, 0x66, 0x77, 0x88};

    wgpu::Buffer dstBuffer =
        CreateBuffer(dstLabel, 8, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);
    queue.WriteBuffer(dstBuffer, 0, &myData1, sizeof(myData1));

    wgpu::Buffer srcBuffer =
        CreateBuffer(srcLabel, 4, wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc);

    auto recorder = Recorder::CreateAndStart(device);

    MapAsyncAndWait(srcBuffer, wgpu::MapMode::Write, 0, 4);
    srcBuffer.WriteMappedRange(0, &myData2, sizeof(myData2));
    srcBuffer.Unmap();

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(srcBuffer, 0, dstBuffer, 4, 4);
        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    uint8_t expected[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    ExpectBufferEQ(replay.get(), dstLabel, expected);
}

// Capture a buffer with MapRead.
TEST_P(CaptureAndReplayTests, CaptureWithMapRead) {
    const char* srcLabel = "srcBuffer";
    const char* dstLabel = "dstBuffer";
    const uint8_t myData[] = {0x55, 0x66, 0x77, 0x88};

    wgpu::Buffer srcBuffer =
        CreateBuffer(srcLabel, 4, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);
    queue.WriteBuffer(srcBuffer, 0, &myData, sizeof(myData));

    wgpu::Buffer dstBuffer =
        CreateBuffer(dstLabel, 4, wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst);

    auto recorder = Recorder::CreateAndStart(device);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(srcBuffer, 0, dstBuffer, 0, sizeof(myData));
        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    // We can't use ExpectBufferEQ here because it uses a copy and `dstBuffer`
    // does not have CopySrc. So, let's do it ourselves.
    MapAsyncAndWait(dstBuffer, wgpu::MapMode::Read, 0, sizeof(myData));
    auto actual = static_cast<const uint8_t*>(dstBuffer.GetConstMappedRange(0, sizeof(myData)));
    std::span<const uint8_t> actual_span(actual, std::size(myData));
    ASSERT_THAT(actual_span, ::testing::ElementsAreArray(myData));
}

TEST_P(CaptureAndReplayTests, CaptureCopyBufferToBuffer) {
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};

    wgpu::Buffer srcBuffer =
        CreateBuffer("srcBuffer", 4, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);
    queue.WriteBuffer(srcBuffer, 0, &myData, sizeof(myData));

    wgpu::Buffer dstBuffer = CreateBuffer("dstBuffer", 4, wgpu::BufferUsage::CopyDst);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(srcBuffer, 0, dstBuffer, 0, 4);
        commands = encoder.Finish();
    }

    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    ExpectBufferEQ(replay.get(), "dstBuffer", myData);
}

TEST_P(CaptureAndReplayTests, WriteTexture) {
    wgpu::Texture texture =
        CreateTexture("myTexture", {4}, wgpu::TextureFormat::R8Unorm,
                      wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc);

    auto recorder = Recorder::CreateAndStart(device);

    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};
    WriteFullTexture(texture, wgpu::TextureFormat::R8Unorm, {4}, myData);

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    ExpectTextureEQ(replay.get(), "myTexture", {4}, myData);
}

TEST_P(CaptureAndReplayTests, CaptureCopyBufferToTexture) {
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};

    wgpu::Buffer srcBuffer =
        CreateBuffer("srcBuffer", 4, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);
    queue.WriteBuffer(srcBuffer, 0, &myData, sizeof(myData));

    wgpu::Texture dstTexture =
        CreateTexture("dstTexture", {4}, wgpu::TextureFormat::R8Unorm,
                      wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
            utils::CreateTexelCopyBufferInfo(srcBuffer, 0, 256, 1);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::Extent3D extent = {4, 1, 1};

        encoder.CopyBufferToTexture(&texelCopyBufferInfo, &texelCopyTextureInfo, &extent);
        commands = encoder.Finish();
    }

    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    ExpectTextureEQ(replay.get(), "dstTexture", {4}, myData);
}

TEST_P(CaptureAndReplayTests, CaptureCopyTextureToBuffer) {
    wgpu::Texture srcTexture =
        CreateTexture("srcTexture", {4}, wgpu::TextureFormat::R8Unorm,
                      wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc);

    // Put data in source texture
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};
    WriteFullTexture(srcTexture, wgpu::TextureFormat::R8Unorm, {4}, myData);

    wgpu::Buffer dstBuffer =
        CreateBuffer("dstBuffer", 4, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);

    wgpu::CommandBuffer commands;
    {
        // Copy srcTexture to dstBuffer
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
            utils::CreateTexelCopyBufferInfo(dstBuffer, 0, 256, 1);
        wgpu::Extent3D extent = {4, 1, 1};

        encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &extent);
        commands = encoder.Finish();
    }

    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    ExpectBufferEQ(replay.get(), "dstBuffer", myData);
}

TEST_P(CaptureAndReplayTests, CaptureCopyTextureToTexture) {
    wgpu::Texture srcTexture =
        CreateTexture("srcTexture", {4}, wgpu::TextureFormat::R8Unorm,
                      wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc);

    // Put data in source texture
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};
    WriteFullTexture(srcTexture, wgpu::TextureFormat::R8Unorm, {4}, myData);

    wgpu::Texture dstTexture =
        CreateTexture("dstTexture", {4}, wgpu::TextureFormat::R8Unorm,
                      wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc);

    wgpu::CommandBuffer commands;
    {
        // Copy srcTexture to dstBuffer
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::Extent3D extent = {4, 1, 1};

        encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo, &extent);
        commands = encoder.Finish();
    }

    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    ExpectTextureEQ(replay.get(), "dstTexture", {4}, myData);
}

// We make 3 textures. Put data in the first one. Copy to the 2nd one.
// Copy tha the 3rd. Check the results. The reason for this test is that
// the first texture is marked as initialized by WriteTexture. The 2nd is
// not. So, if the texture is not marked as initialized by CopyT2T then
// capture will fail as it will not copy the contents of the 2nd texture.
TEST_P(CaptureAndReplayTests, CaptureCopyTextureToTextureFromCopyT2TTexture) {
    wgpu::Texture dataTexture =
        CreateTexture("dataTexture", {4}, wgpu::TextureFormat::R8Unorm,
                      wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc);

    // Put data in data texture
    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};
    WriteFullTexture(dataTexture, wgpu::TextureFormat::R8Unorm, {4}, myData);

    wgpu::Texture srcTexture =
        CreateTexture("srcTexture", {4}, wgpu::TextureFormat::R8Unorm,
                      wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc);

    // Copy the data texture ot the src texture
    {
        wgpu::CommandBuffer commands;
        {
            // Copy srcTexture to dstBuffer
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
                dataTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
            wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
                srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
            wgpu::Extent3D extent = {4, 1, 1};

            encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo,
                                         &extent);
            commands = encoder.Finish();
        }
        queue.Submit(1, &commands);
    }

    wgpu::Texture dstTexture =
        CreateTexture("dstTexture", {4}, wgpu::TextureFormat::R8Unorm,
                      wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc);

    wgpu::CommandBuffer commands;
    {
        // Copy srcTexture to dstBuffer
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
        wgpu::Extent3D extent = {4, 1, 1};

        encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo, &extent);
        commands = encoder.Finish();
    }

    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    ExpectTextureEQ(replay.get(), "dstTexture", {4}, myData);
}

// Before capture, creates a texture and sets it in a compute pass as a storage texture.
// Then, captures a copyT2T to a 2nd texture. Checks the 2nd texture has the correct data on replay.
TEST_P(CaptureAndReplayTests, CaptureCopyTextureToTextureFromComputeTexture) {
    wgpu::Texture srcTexture =
        CreateTexture("srcTexture", {1}, wgpu::TextureFormat::RGBA8Uint,
                      wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::StorageBinding);

    const char* shader = R"(
        @group(0) @binding(0) var tex: texture_storage_2d<rgba8uint, write>;

        @compute @workgroup_size(1) fn main() {
            textureStore(tex, vec2u(0), vec4<u32>(0x11, 0x22, 0x33, 0x44));
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, srcTexture.CreateView()},
                                                     });

    {
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
    }

    wgpu::Texture dstTexture = CreateTexture(
        "dstTexture", {1, 1, 1}, wgpu::TextureFormat::RGBA8Uint, wgpu::TextureUsage::CopyDst);

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    {
        wgpu::CommandBuffer commands;
        {
            // Copy srcTexture to dstBuffer
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
                srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
            wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
                dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
            wgpu::Extent3D extent = {1, 1, 1};

            encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo,
                                         &extent);
            commands = encoder.Finish();
        }
        queue.Submit(1, &commands);
    }

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    utils::RGBA8 expected[] = {{0x11, 0x22, 0x33, 0x44}};
    ExpectTextureEQ(replay.get(), "dstTexture", {1}, expected);
}

// Before capture, creates a texture and sets in a render pass as a render attachment
// Then, captures a copyT2T to a 2nd texture. Checks the 2nd texture has the correct data on replay.
TEST_P(CaptureAndReplayTests, CaptureCopyTextureToTextureFromRenderTexture) {
    wgpu::Texture srcTexture =
        CreateTexture("srcTexture", {1}, wgpu::TextureFormat::RGBA8Uint,
                      wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    {
        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            utils::ComboRenderPassDescriptor passDescriptor({srcTexture.CreateView()});
            passDescriptor.cColorAttachments[0].clearValue = {0x11, 0x22, 0x33, 0x44};
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
            pass.End();

            commands = encoder.Finish();
        }
        queue.Submit(1, &commands);
    }

    wgpu::Texture dstTexture = CreateTexture(
        "dstTexture", {1, 1, 1}, wgpu::TextureFormat::RGBA8Uint, wgpu::TextureUsage::CopyDst);

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    {
        wgpu::CommandBuffer commands;
        {
            // Copy srcTexture to dstBuffer
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
                srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
            wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
                dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);
            wgpu::Extent3D extent = {1, 1, 1};

            encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo,
                                         &extent);
            commands = encoder.Finish();
        }
        queue.Submit(1, &commands);
    }

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    utils::RGBA8 expected[] = {{0x11, 0x22, 0x33, 0x44}};
    ExpectTextureEQ(replay.get(), "dstTexture", {1}, expected);
}

// Capture and replay the simplest compute shader.
TEST_P(CaptureAndReplayTests, CaptureComputeShaderBasic) {
    const char* label = "MyBuffer";

    wgpu::Buffer buffer =
        CreateBuffer(label, 4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst);

    const char* shader = R"(
        @group(0) @binding(0) var<storage, read_write> result : u32;

        @compute @workgroup_size(1) fn main() {
            result = 0x44332211;
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, buffer},
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

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    uint8_t expected[] = {0x11, 0x22, 0x33, 0x44};
    ExpectBufferEQ(replay.get(), label, expected);
}

// Capture and replay the simplest compute shader but set the bindGroup
// before setting the pipeline.
TEST_P(CaptureAndReplayTests, CaptureComputeShaderBasicSetBindGroupFirst) {
    const char* label = "MyBuffer";

    wgpu::Buffer buffer =
        CreateBuffer(label, 4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst);

    const char* shader = R"(
        @group(0) @binding(0) var<storage, read_write> result : u32;

        @compute @workgroup_size(1) fn main() {
            result = 0x44332211;
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, buffer},
                                                     });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bindGroup);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    uint8_t expected[] = {0x11, 0x22, 0x33, 0x44};
    ExpectBufferEQ(replay.get(), label, expected);
}

// Capture and replay 2 auto-layout compute pipelines with the same layout
// This is to verify a bug fix. In Dawn, there is BindGroupLayout and
// BindGroupLayoutInternal. In this test, given 2 pipelines with the same
// layout, there will be 2 BindGroupLayout objects pointing to one
// BindGroupLayoutInternal. That means that when serializing, one of them
// will get the wrong Pipeline if the pipeline is incorrectly associated
// with the one BindGroupLayoutInternal instead of each of the 2 pipelines
// being separately associated with one of the 2 BindGroupLayout objects.
// This is a regression test for crbug.com/455605671
TEST_P(CaptureAndReplayTests, CaptureTwoMatchingAutoLayoutComputePipelines) {
    wgpu::Buffer buffer1 =
        CreateBuffer("buffer1", 4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst);
    wgpu::Buffer buffer2 =
        CreateBuffer("buffer2", 4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst);

    const char* shader = R"(
        @group(0) @binding(0) var<storage, read_write> result : u32;

        @compute @workgroup_size(1) fn cs1() {
            result = 0x44332211;
        }

        @compute @workgroup_size(1) fn cs2() {
            result = 0x88776655;
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.label = "pipeline1";
    csDesc.compute.module = module;
    csDesc.compute.entryPoint = "cs1";
    wgpu::ComputePipeline pipeline1 = device.CreateComputePipeline(&csDesc);
    csDesc.label = "pipeline2";
    csDesc.compute.entryPoint = "cs2";
    wgpu::ComputePipeline pipeline2 = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup1 = utils::MakeBindGroup(device, pipeline1.GetBindGroupLayout(0),
                                                      {
                                                          {0, buffer1},
                                                      });
    wgpu::BindGroup bindGroup2 = utils::MakeBindGroup(device, pipeline2.GetBindGroupLayout(0),
                                                      {
                                                          {0, buffer2},
                                                      });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline1);
        pass.SetBindGroup(0, bindGroup1);
        pass.DispatchWorkgroups(1);
        pass.SetPipeline(pipeline2);
        pass.SetBindGroup(0, bindGroup2);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    uint8_t expected1[] = {0x11, 0x22, 0x33, 0x44};
    ExpectBufferEQ(replay.get(), "buffer1", expected1);

    uint8_t expected2[] = {0x55, 0x66, 0x77, 0x88};
    ExpectBufferEQ(replay.get(), "buffer2", expected2);
}

// Capture and replay 2 bindGroups that use implicit bindGroupLayouts from
// different pipelines but for 1, never set the pipeline nor dispatch. This effectively
// makes it a no-op. The issue is, we can't easily serialize a bindGroup that uses an
// implicit bindGroupLayout unless the pipeline that created that bindGroupLayout is
// used in the command buffer. So, we just don't serialize those calls to setBindGroup
// since they are effectively no-ops. This test checks things don't crash as if the
// call was actually serialized it would reference a bindGroupLayout that does not
// exist.
TEST_P(CaptureAndReplayTests, CaptureTwoAutoLayoutComputePipelinesOneIsBoundButUnused) {
    wgpu::Buffer buffer =
        CreateBuffer("buffer", 4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst);

    const char* shader = R"(
        @group(0) @binding(0) var<storage, read_write> result : u32;

        @compute @workgroup_size(1) fn cs1() {
            result = 0x44332211;
        }

        @compute @workgroup_size(1) fn cs2() {
            result = 0x88776655;
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.label = "pipeline1";
    csDesc.compute.module = module;
    csDesc.compute.entryPoint = "cs1";
    wgpu::ComputePipeline pipeline1 = device.CreateComputePipeline(&csDesc);
    csDesc.label = "pipeline2";
    csDesc.compute.entryPoint = "cs2";
    wgpu::ComputePipeline pipeline2 = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup1 = utils::MakeBindGroup(device, pipeline1.GetBindGroupLayout(0),
                                                      {
                                                          {0, buffer},
                                                      });
    wgpu::BindGroup bindGroup2 = utils::MakeBindGroup(device, pipeline2.GetBindGroupLayout(0),
                                                      {
                                                          {0, buffer},
                                                      });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bindGroup1);
        pass.SetPipeline(pipeline2);
        pass.SetBindGroup(0, bindGroup2);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    uint8_t expected[] = {0x55, 0x66, 0x77, 0x88};
    ExpectBufferEQ(replay.get(), "buffer", expected);
}

// Capture and replay the simplest render pass.
// It just starts and ends a render pass and uses the clearValue to set
// a texture.
TEST_P(CaptureAndReplayTests, CaptureRenderPassBasic) {
    wgpu::Texture texture = CreateTexture("myTexture", {1}, wgpu::TextureFormat::RGBA8Uint,
                                          wgpu::TextureUsage::RenderAttachment);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        utils::ComboRenderPassDescriptor passDescriptor({texture.CreateView()});
        passDescriptor.cColorAttachments[0].clearValue = {0x11, 0x22, 0x33, 0x44};
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    utils::RGBA8 expected[] = {{0x11, 0x22, 0x33, 0x44}};
    ExpectTextureEQ(replay.get(), "myTexture", {1}, expected);
}

// Capture and replay the a render pass where a texture is rendered into another.
TEST_P(CaptureAndReplayTests, CaptureRenderPassBasicWithBindGroup) {
    wgpu::Texture srcTexture =
        CreateTexture("srcTexture", {1}, wgpu::TextureFormat::RGBA8Uint,
                      wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst);

    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};
    WriteFullTexture(srcTexture, wgpu::TextureFormat::RGBA8Uint, {1}, myData);

    wgpu::Texture dstTexture = CreateTexture("dstTexture", {1}, wgpu::TextureFormat::RGBA8Uint,
                                             wgpu::TextureUsage::RenderAttachment);

    const char* shader = R"(
        @group(0) @binding(0) var tex: texture_2d<u32>;

        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }

        @fragment fn fs() -> @location(0) vec4u {
            return textureLoad(tex, vec2u(0), 0);
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;
    desc.cFragment.targetCount = 1;
    desc.cTargets[0].format = wgpu::TextureFormat::RGBA8Uint;
    desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, srcTexture.CreateView()},
                                                     });
    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        utils::ComboRenderPassDescriptor passDescriptor({dstTexture.CreateView()});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(1);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    utils::RGBA8 expected[] = {{0x11, 0x22, 0x33, 0x44}};
    ExpectTextureEQ(replay.get(), "dstTexture", {1}, expected);
}

TEST_P(CaptureAndReplayTests, CaptureRenderPassBasicWithAttributes) {
    const float myVertices[] = {
        -1, -1, 3, -1, -1, 3,
    };

    wgpu::Buffer vertexBuffer = CreateBuffer(
        "vertexBuffer", sizeof(myVertices), wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex);
    queue.WriteBuffer(vertexBuffer, 0, &myVertices, sizeof(myVertices));

    wgpu::Texture dstTexture = CreateTexture("dstTexture", {1}, wgpu::TextureFormat::RGBA8Uint,
                                             wgpu::TextureUsage::RenderAttachment);

    const char* shader = R"(
        @vertex fn vs(@location(0) pos: vec4f) -> @builtin(position) vec4f {
            return pos;
        }

        @fragment fn fs() -> @location(0) vec4u {
            return vec4u(0x11, 0x22, 0x33, 0x44);
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;
    desc.cFragment.targetCount = 1;
    desc.cTargets[0].format = wgpu::TextureFormat::RGBA8Uint;
    desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    desc.cBuffers[0].arrayStride = 2 * sizeof(float);
    desc.cBuffers[0].attributeCount = 1;
    desc.cBuffers[0].attributes = &desc.cAttributes[0];
    desc.cAttributes[0].shaderLocation = 0;
    desc.cAttributes[0].format = wgpu::VertexFormat::Float32x2;
    desc.vertex.bufferCount = 1;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        utils::ComboRenderPassDescriptor passDescriptor({dstTexture.CreateView()});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.Draw(3);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    utils::RGBA8 expected[] = {{0x11, 0x22, 0x33, 0x44}};
    ExpectTextureEQ(replay.get(), "dstTexture", {1}, expected);
}

// Capture and replay a compute shader with an explicit bindGroupLayout
TEST_P(CaptureAndReplayTests, CaptureComputeShaderBasicExplicitBindGroup) {
    wgpu::BindGroupLayoutEntry entries[1];
    entries[0].binding = 0;
    entries[0].visibility = wgpu::ShaderStage::Compute;
    entries[0].buffer.type = wgpu::BufferBindingType::Storage;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = entries;
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&bglDesc);

    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.bindGroupLayoutCount = 1;
    plDesc.bindGroupLayouts = &layout;
    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&plDesc);

    const char* label = "MyBuffer";
    wgpu::Buffer buffer =
        CreateBuffer(label, 4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst);

    const char* shader = R"(
        @group(0) @binding(0) var<storage, read_write> result : u32;

        @compute @workgroup_size(1) fn main() {
            result = 0x44332211;
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = pipelineLayout;
    csDesc.compute.module = module;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, buffer},
                                                     });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bindGroup);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    uint8_t expected[] = {0x11, 0x22, 0x33, 0x44};
    ExpectBufferEQ(replay.get(), label, expected);
}

// Capture and replay a pass that uses a storage texture
TEST_P(CaptureAndReplayTests, CaptureStorageTextureUsageWithExplicitBindGroupLayout) {
    wgpu::BindGroupLayoutEntry entries[1];
    entries[0].binding = 0;
    entries[0].visibility = wgpu::ShaderStage::Compute;
    entries[0].storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
    entries[0].storageTexture.format = wgpu::TextureFormat::RGBA8Uint;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = entries;
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&bglDesc);

    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.bindGroupLayoutCount = 1;
    plDesc.bindGroupLayouts = &layout;
    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&plDesc);

    wgpu::Texture texture =
        CreateTexture("myTexture", {1}, wgpu::TextureFormat::RGBA8Uint,
                      wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc);

    const char* shader = R"(
        @group(0) @binding(0) var tex: texture_storage_2d<rgba8uint, write>;

        @compute @workgroup_size(1) fn main() {
            textureStore(tex, vec2u(0), vec4u(0x11, 0x22, 0x33, 0x44));
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = pipelineLayout;
    csDesc.compute.module = module;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, texture.CreateView()},
                                                     });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bindGroup);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    utils::RGBA8 expected[] = {{0x11, 0x22, 0x33, 0x44}};
    ExpectTextureEQ(replay.get(), "myTexture", {1}, expected);
}

// Capture and replay a pass that uses a texture binding with explicit bind group layout.
TEST_P(CaptureAndReplayTests, CaptureTextureUsageWithExplicitBindGroupLayout) {
    wgpu::BindGroupLayoutEntry entries[2];
    entries[0].binding = 2;
    entries[0].visibility = wgpu::ShaderStage::Compute;
    entries[0].texture.sampleType = wgpu::TextureSampleType::Uint;
    entries[0].texture.viewDimension = wgpu::TextureViewDimension::e2D;
    entries[1].binding = 4;
    entries[1].visibility = wgpu::ShaderStage::Compute;
    entries[1].buffer.type = wgpu::BufferBindingType::Storage;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 2;
    bglDesc.entries = entries;
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&bglDesc);

    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.bindGroupLayoutCount = 1;
    plDesc.bindGroupLayouts = &layout;
    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&plDesc);

    wgpu::Texture texture =
        CreateTexture("myTexture", {1}, wgpu::TextureFormat::RGBA8Uint,
                      wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst);

    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};
    WriteFullTexture(texture, wgpu::TextureFormat::RGBA8Uint, {1}, myData);

    wgpu::Buffer buffer = CreateBuffer(
        "myBuffer", 4,
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);

    const char* shader = R"(
        @group(0) @binding(2) var tex: texture_2d<u32>;
        @group(0) @binding(4) var<storage, read_write> result: u32;

        @compute @workgroup_size(1) fn main() {
            let c = textureLoad(tex, vec2u(0), 0);
            result = pack4xU8Clamp(c);
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.layout = pipelineLayout;
    csDesc.compute.module = module;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {2, texture.CreateView()},
                                                         {4, buffer},
                                                     });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bindGroup);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    uint8_t expected[] = {0x11, 0x22, 0x33, 0x44};
    ExpectBufferEQ(replay.get(), "myBuffer", expected);
}

// Capture and replay a pass that uses a sampler binding with explicit bind group layout.
TEST_P(CaptureAndReplayTests, CaptureSamplerUsageWithExplicitBindGroupLayout) {
    wgpu::BindGroupLayoutEntry entries[2];
    entries[0].binding = 2;
    entries[0].visibility = wgpu::ShaderStage::Fragment;
    entries[0].texture.sampleType = wgpu::TextureSampleType::Float;
    entries[0].texture.viewDimension = wgpu::TextureViewDimension::e2D;
    entries[1].binding = 4;
    entries[1].visibility = wgpu::ShaderStage::Fragment;
    entries[1].sampler.type = wgpu::SamplerBindingType::Filtering;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 2;
    bglDesc.entries = entries;
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&bglDesc);

    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.bindGroupLayoutCount = 1;
    plDesc.bindGroupLayouts = &layout;
    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&plDesc);

    wgpu::Texture srcTexture =
        CreateTexture("srcTexture", {1}, wgpu::TextureFormat::RGBA8Unorm,
                      wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst);

    const uint8_t myData[] = {0x11, 0x22, 0x33, 0x44};
    WriteFullTexture(srcTexture, wgpu::TextureFormat::RGBA8Unorm, {1}, myData);

    wgpu::Sampler sampler = device.CreateSampler();

    const char* shader = R"(
        @group(0) @binding(2) var tex: texture_2d<f32>;
        @group(0) @binding(4) var smp: sampler;

        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0, 1);
        }

        @fragment fn fs() -> @location(0) vec4f {
            return textureSample(tex, smp, vec2f(0));
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;
    desc.cFragment.targetCount = 1;
    desc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {2, srcTexture.CreateView()},
                                                         {4, sampler},
                                                     });

    wgpu::Texture dstTexture =
        CreateTexture("dstTexture", {1}, wgpu::TextureFormat::RGBA8Unorm,
                      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        utils::ComboRenderPassDescriptor passDescriptor({dstTexture.CreateView()});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
        pass.SetBindGroup(0, bindGroup);
        pass.SetPipeline(pipeline);
        pass.Draw(1);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    utils::RGBA8 expected[] = {{0x11, 0x22, 0x33, 0x44}};
    ExpectTextureEQ(replay.get(), "dstTexture", {1}, expected);
}

// Capture and replay a pass uses a depth attachment.
// This was failing because the front end does not save the user's
// stencilLoadOp and stencilStoreOp as the spec requires "undefined"
TEST_P(CaptureAndReplayTests, CaptureDepthRenderPass) {
    wgpu::Texture texture = CreateTexture("texture", {1}, wgpu::TextureFormat::RGBA8Unorm,
                                          wgpu::TextureUsage::RenderAttachment);

    wgpu::Texture depthTexture =
        CreateTexture("depthTexture", {1}, wgpu::TextureFormat::Depth16Unorm,
                      wgpu::TextureUsage::RenderAttachment);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        utils::ComboRenderPassDescriptor passDescriptor({texture.CreateView()},
                                                        depthTexture.CreateView());
        passDescriptor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        passDescriptor.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    // We just expect no errors.
}

constexpr static uint64_t kSentinelValue = ~uint64_t(0u);
class OcclusionExpectation : public detail::Expectation {
  public:
    enum class Result { Zero, NonZero };

    ~OcclusionExpectation() override = default;

    explicit OcclusionExpectation(const std::vector<Result> expected) { mExpected = expected; }

    testing::AssertionResult Check(const void* data, size_t size) override {
        DAWN_ASSERT(size % sizeof(uint64_t) == 0);
        DAWN_ASSERT(size / sizeof(uint64_t) == mExpected.size());
        const uint64_t* actual = static_cast<const uint64_t*>(data);
        for (size_t i = 0; i < size / sizeof(uint64_t); i++) {
            if (actual[i] == kSentinelValue) {
                return testing::AssertionFailure()
                       << "Data[" << i << "] was not written (it kept the sentinel value of "
                       << kSentinelValue << ").\n";
            }
            Result expected = mExpected[i];
            if (expected == Result::Zero && actual[i] != 0) {
                return testing::AssertionFailure()
                       << "Expected data[" << i << "] to be zero, actual: " << actual[i] << ".\n";
            }
            if (expected == Result::NonZero && actual[i] == 0) {
                return testing::AssertionFailure()
                       << "Expected data[" << i << "] to be non-zero.\n";
            }
        }

        return testing::AssertionSuccess();
    }

  private:
    std::vector<Result> mExpected;
};

// Capture and replay a pass that uses a QuerySet.
// We use a point-list vertex shader that we can set the z value by passing a different vertex_index
// via the firstVertex argument to draw.
TEST_P(CaptureAndReplayTests, CaptureQuerySetBasic) {
    const char* shader = R"(
        @vertex fn vs(@builtin(vertex_index) vNdx: u32) -> @builtin(position) vec4f {
            return vec4f(0, 0, f32(vNdx) / 10.0, 1);
        }

        @fragment fn fs() -> @location(0) vec4f {
            return vec4f(0);
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;
    desc.cFragment.targetCount = 1;
    desc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    desc.primitive.topology = wgpu::PrimitiveTopology::PointList;

    wgpu::DepthStencilState* depthStencil =
        desc.EnableDepthStencil(wgpu::TextureFormat::Depth16Unorm);
    depthStencil->depthWriteEnabled = wgpu::OptionalBool::True;
    depthStencil->depthCompare = wgpu::CompareFunction::Less;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    wgpu::Texture dstTexture =
        CreateTexture("dstTexture", {1}, wgpu::TextureFormat::RGBA8Unorm,
                      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc);
    wgpu::Texture depthTexture =
        CreateTexture("depthTexture", {1}, wgpu::TextureFormat::Depth16Unorm,
                      wgpu::TextureUsage::RenderAttachment);

    constexpr uint32_t kNumQueries = 4;
    wgpu::QuerySetDescriptor qsDesc;
    qsDesc.label = "myQuerySet";
    qsDesc.count = kNumQueries;
    qsDesc.type = wgpu::QueryType::Occlusion;
    wgpu::QuerySet querySet = device.CreateQuerySet(&qsDesc);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        utils::ComboRenderPassDescriptor passDescriptor({dstTexture.CreateView()},
                                                        depthTexture.CreateView());
        passDescriptor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        passDescriptor.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        passDescriptor.occlusionQuerySet = querySet;

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
        pass.SetPipeline(pipeline);

        uint32_t nextIndex = 0;
        auto DrawPixelAtDepthWithOcclusionTest = [&](uint32_t depth) {
            pass.BeginOcclusionQuery(nextIndex++);
            pass.Draw(1, 1, depth, 0);
            pass.EndOcclusionQuery();
        };

        DrawPixelAtDepthWithOcclusionTest(5);  // draws at 0.5 (not occluded)
        DrawPixelAtDepthWithOcclusionTest(7);  // draws at 0.7 (occluded)
        DrawPixelAtDepthWithOcclusionTest(2);  // draws at 0.2 (not-occluded)
        DrawPixelAtDepthWithOcclusionTest(5);  // draws at 0.5 (occluded)

        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    {
        auto qs = replay->GetObjectByLabel<wgpu::QuerySet>("myQuerySet");
        ASSERT_TRUE(qs);

        uint64_t size = sizeof(uint64_t) * kNumQueries;
        auto resolveBuffer =
            CreateBuffer("", size,
                         wgpu::BufferUsage::QueryResolve | wgpu::BufferUsage::CopySrc |
                             wgpu::BufferUsage::CopyDst);
        std::vector<uint64_t> sentinels(kNumQueries, kSentinelValue);
        queue.WriteBuffer(resolveBuffer, 0, sentinels.data(), size);

        {
            wgpu::CommandBuffer commands;
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.ResolveQuerySet(qs, 0, kNumQueries, resolveBuffer, 0);
            commands = encoder.Finish();
            queue.Submit(1, &commands);
        }

        EXPECT_BUFFER(
            resolveBuffer, 0, size,
            new OcclusionExpectation(
                {OcclusionExpectation::Result::NonZero, OcclusionExpectation::Result::Zero,
                 OcclusionExpectation::Result::NonZero, OcclusionExpectation::Result::Zero}));
    }
}

// Capture and replay a render bundle.
TEST_P(CaptureAndReplayTests, CaptureRenderBundleBasic) {
    const char* shader = R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0, 1);
        }

        @fragment fn fs() -> @location(0) vec4u {
            return vec4u(0x11, 0x22, 0x33, 0x44);
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    utils::ComboRenderPipelineDescriptor passDesc;
    passDesc.vertex.module = module;
    passDesc.cFragment.module = module;
    passDesc.cFragment.targetCount = 1;
    passDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Uint;
    passDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&passDesc);

    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Uint;

    wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

    renderBundleEncoder.SetPipeline(pipeline);
    renderBundleEncoder.Draw(1);

    wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

    wgpu::Texture dstTexture =
        CreateTexture("dstTexture", {1}, wgpu::TextureFormat::RGBA8Uint,
                      wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        utils::ComboRenderPassDescriptor passDescriptor({dstTexture.CreateView()});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
        pass.ExecuteBundles(1, &renderBundle);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    utils::RGBA8 expected[] = {{0x11, 0x22, 0x33, 0x44}};
    ExpectTextureEQ(replay.get(), "dstTexture", {1}, expected);
}

// Test debug commands don't fail.
TEST_P(CaptureAndReplayTests, PushPopInsertDebug) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 4, 4);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.PushDebugGroup("Event Start");
    encoder.InsertDebugMarker("Marker");
    encoder.PopDebugGroup();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.PushDebugGroup("Event Start");
        pass.InsertDebugMarker("Marker");
        pass.PopDebugGroup();
        pass.End();
    }
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.PushDebugGroup("Event Start");
        pass.InsertDebugMarker("Marker");
        pass.PopDebugGroup();
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    // just expect no errors.
}

// Test capturing setting a label
TEST_P(CaptureAndReplayTests, CaptureSetLabel) {
    wgpu::Buffer buffer = CreateBuffer("buf", 4, wgpu::BufferUsage::CopyDst);
    wgpu::Texture texture =
        CreateTexture("tex", {1}, wgpu::TextureFormat::RGBA8Uint,
                      wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc);
    wgpu::TextureView view = texture.CreateView();
    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::QuerySetDescriptor qsDesc;
    qsDesc.count = 1;
    qsDesc.type = wgpu::QueryType::Occlusion;
    wgpu::QuerySet querySet = device.CreateQuerySet(&qsDesc);

    const char* shader = R"(
        @group(0) @binding(0) var tex: texture_storage_2d<rgba8uint, write>;

        @compute @workgroup_size(1) fn main() {
            textureStore(tex, vec2u(0), vec4<u32>(0));
        }

        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0);
        }

        @fragment fn fs() -> @location(0) vec4u {
            return vec4u(0);
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::BindGroupLayoutEntry entries[1];
    entries[0].binding = 0;
    entries[0].visibility = wgpu::ShaderStage::Compute;
    entries[0].buffer.type = wgpu::BufferBindingType::Storage;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = entries;
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&bglDesc);

    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.bindGroupLayoutCount = 1;
    plDesc.bindGroupLayouts = &layout;
    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&plDesc);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    wgpu::ComputePipeline cPipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, cPipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, view},
                                                     });

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;
    desc.cFragment.targetCount = 1;
    desc.cTargets[0].format = wgpu::TextureFormat::RGBA8Uint;
    desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline rPipeline = device.CreateRenderPipeline(&desc);

    utils::ComboRenderBundleEncoderDescriptor rbDesc = {};
    rbDesc.colorFormatCount = 1;
    rbDesc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Uint;

    wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&rbDesc);
    wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();

    auto recorder = Recorder::CreateAndStart(device);

    // Note: the first time capture see's a resource here is
    // on SetLabel. The label is set before the resource is captured
    // so we won't see the original label. So, set them twice so we
    // can verify the label changed.
    bindGroup.SetLabel("bgA");
    bindGroup.SetLabel("bgB");
    buffer.SetLabel("bufA");
    buffer.SetLabel("bufB");
    commandBuffer.SetLabel("cbA");
    commandBuffer.SetLabel("cbB");
    cPipeline.SetLabel("cpA");
    cPipeline.SetLabel("cpB");
    device.SetLabel("devA");
    device.SetLabel("devB");
    layout.SetLabel("loA");
    layout.SetLabel("loB");
    pipelineLayout.SetLabel("plA");
    pipelineLayout.SetLabel("plB");
    querySet.SetLabel("qsA");
    querySet.SetLabel("qsB");
    rPipeline.SetLabel("rpA");
    rPipeline.SetLabel("rpB");
    renderBundle.SetLabel("rbA");
    renderBundle.SetLabel("rbB");
    sampler.SetLabel("smpA");
    sampler.SetLabel("smpB");
    module.SetLabel("modA");
    module.SetLabel("modB");
    texture.SetLabel("texA");
    texture.SetLabel("texB");
    view.SetLabel("viewA");
    view.SetLabel("viewB");

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::BindGroup>("bgA") == nullptr);
    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::BindGroup>("bgB") != nullptr);

    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::Buffer>("buf") == nullptr);
    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::Buffer>("bufA") == nullptr);
    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::Buffer>("bufB") != nullptr);

    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::CommandBuffer>("cbA") == nullptr);
    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::CommandBuffer>("cbB") != nullptr);

    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::ComputePipeline>("cpA") == nullptr);
    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::ComputePipeline>("cpB") != nullptr);

    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::Device>("devA") == nullptr);
    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::Device>("devB") != nullptr);

    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::BindGroupLayout>("loA") == nullptr);
    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::BindGroupLayout>("loB") != nullptr);

    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::PipelineLayout>("plA") == nullptr);
    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::PipelineLayout>("plB") != nullptr);

    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::RenderBundle>("rbA") == nullptr);
    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::RenderBundle>("rbB") != nullptr);

    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::RenderPipeline>("rpA") == nullptr);
    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::RenderPipeline>("rpB") != nullptr);

    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::QuerySet>("qsA") == nullptr);
    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::QuerySet>("qsB") != nullptr);

    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::Sampler>("smpA") == nullptr);
    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::Sampler>("smpB") != nullptr);

    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::ShaderModule>("modA") == nullptr);
    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::ShaderModule>("modB") != nullptr);

    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::Texture>("texA") == nullptr);
    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::Texture>("texB") != nullptr);

    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::TextureView>("viewA") == nullptr);
    EXPECT_TRUE(replay->GetObjectByLabel<wgpu::TextureView>("viewB") != nullptr);
}

// Capture SetBlendConstant.
TEST_P(CaptureAndReplayTests, CaptureSetBlendConstant) {
    wgpu::Texture dstTexture = CreateTexture("dstTexture", {1}, wgpu::TextureFormat::RGBA8Unorm,
                                             wgpu::TextureUsage::RenderAttachment);

    const char* shader = R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0, 1);
        }

        @fragment fn fs() -> @location(0) vec4f {
            return vec4f(1);
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;
    desc.cFragment.targetCount = 1;
    desc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    desc.cTargets[0].blend = &desc.cBlends[0];
    desc.cBlends[0].color.operation = wgpu::BlendOperation::Add;
    desc.cBlends[0].color.srcFactor = wgpu::BlendFactor::Constant;
    desc.cBlends[0].color.dstFactor = wgpu::BlendFactor::Zero;
    desc.cBlends[0].alpha.operation = wgpu::BlendOperation::Add;
    desc.cBlends[0].alpha.srcFactor = wgpu::BlendFactor::Constant;
    desc.cBlends[0].alpha.dstFactor = wgpu::BlendFactor::Zero;
    desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        utils::ComboRenderPassDescriptor passDescriptor({dstTexture.CreateView()});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
        pass.SetPipeline(pipeline);
        // the +0.1 is to try get the same result on all GPUs.
        wgpu::Color color = {11.1 / 255.0, 22.1 / 255.0, 33.1 / 255.0, 44.1 / 255.0};
        pass.SetBlendConstant(&color);
        pass.Draw(1);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    utils::RGBA8 expected[] = {{11, 22, 33, 44}};
    ExpectTextureEQ(replay.get(), "dstTexture", {1}, expected);
}

// Capture DispatchIndirect.
TEST_P(CaptureAndReplayTests, CaptureDispatchIndirect) {
    const char* shader = R"(
        @group(0) @binding(0) var<storage, read_write> result : u32;

        @compute @workgroup_size(1) fn main() {
            result = 0x44332211;
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    const uint32_t myData[] = {0x1, 0x1, 0x1};
    wgpu::Buffer indirectBuffer = CreateBuffer(
        "indirect", sizeof(myData), wgpu::BufferUsage::Indirect | wgpu::BufferUsage::CopyDst);
    queue.WriteBuffer(indirectBuffer, 0, &myData, sizeof(myData));

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    const char* label = "MyBuffer";
    wgpu::Buffer buffer = CreateBuffer(label, 4, wgpu::BufferUsage::Storage);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, buffer},
                                                     });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bindGroup);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroupsIndirect(indirectBuffer, 0);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    uint8_t expected[] = {0x11, 0x22, 0x33, 0x44};
    ExpectBufferEQ(replay.get(), label, expected);
}

// Capture ClearBuffer.
TEST_P(CaptureAndReplayTests, CaptureClearBuffer) {
    const uint32_t myData[] = {0x11111111, 0x22222222, 0x33333333};
    wgpu::Buffer buffer = CreateBuffer("buf", sizeof(myData),
                                       wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst);
    queue.WriteBuffer(buffer, 0, &myData, sizeof(myData));

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ClearBuffer(buffer, 4, 4);
        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    const uint8_t expected[] = {0x11, 0x11, 0x11, 0x11, 0, 0, 0, 0, 0x33, 0x33, 0x33, 0x33};
    ExpectBufferEQ(replay.get(), "buf", expected);
}

TEST_P(CaptureAndReplayTests, SetImmediateComputePass) {
    const char* shader = R"(
        var<immediate> value: u32;
        @group(0) @binding(0) var<storage, read_write> result : u32;

        @compute @workgroup_size(1) fn main() {
            result = value;
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::BindGroupLayoutEntry entries[1];
    entries[0].binding = 0;
    entries[0].visibility = wgpu::ShaderStage::Compute;
    entries[0].buffer.type = wgpu::BufferBindingType::Storage;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = entries;
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&bglDesc);

    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.label = "immediatePipelineLayout";
    plDesc.bindGroupLayoutCount = 1;
    plDesc.bindGroupLayouts = &layout;
    plDesc.immediateSize = 4;
    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&plDesc);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.label = "immediatePipeline";
    csDesc.layout = pipelineLayout;
    csDesc.compute.module = module;

    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    const char* label = "MyBuffer";
    wgpu::Buffer buffer = CreateBuffer(label, 4, wgpu::BufferUsage::Storage);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, buffer},
                                                     });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bindGroup);
        pass.SetPipeline(pipeline);
        const uint8_t data[] = {0x11, 0x22, 0x33, 0x44};
        pass.SetImmediates(0, data, sizeof(data));
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    uint8_t expected[] = {0x11, 0x22, 0x33, 0x44};
    ExpectBufferEQ(replay.get(), label, expected);
}

TEST_P(CaptureAndReplayTests, SetImmediateRenderPass) {
    const char* shader = R"(
        var<immediate> value: u32;
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0, 1);
        }

        @fragment fn fs() -> @location(0) vec4u {
            return vec4u(value);
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::BindGroupLayoutEntry entries[1];
    entries[0].binding = 0;
    entries[0].visibility = wgpu::ShaderStage::Compute;
    entries[0].buffer.type = wgpu::BufferBindingType::Storage;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = entries;
    wgpu::BindGroupLayout layout = device.CreateBindGroupLayout(&bglDesc);

    wgpu::PipelineLayoutDescriptor plDesc;
    plDesc.label = "immediatePipelineLayout";
    plDesc.bindGroupLayoutCount = 0;
    plDesc.immediateSize = 4;
    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&plDesc);

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;
    desc.cFragment.targetCount = 1;
    desc.cTargets[0].format = wgpu::TextureFormat::R32Uint;
    desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    wgpu::Texture dstTexture = CreateTexture("dstTexture", {1}, wgpu::TextureFormat::R32Uint,
                                             wgpu::TextureUsage::RenderAttachment);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor passDescriptor({dstTexture.CreateView()});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
        pass.SetPipeline(pipeline);
        const uint8_t data[] = {0x11, 0x22, 0x33, 0x44};
        pass.SetImmediates(0, data, sizeof(data));
        pass.Draw(1);
        pass.End();

        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    const uint32_t expected[] = {0x44332211};
    ExpectTextureEQ(replay.get(), "dstTexture", {1}, expected);
}

// Make sure it does not capture the unmap of a buffer if that unmap is triggered by the buffer
// destroy.
TEST_P(CaptureAndReplayTests, MappedBufferDestroyed) {
    auto recorder = Recorder::CreateAndStart(device);

    {
        // Create a buffer mapped at creation and then release it.
        wgpu::Buffer buffer = CreateBuffer("MyBuffer", 4, wgpu::BufferUsage::CopyDst, true);
    }

    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    // just expect no errors
}

// Depth24Plus is not directly copyable as either src or dst so, we make a depth24plus.
// put values in it via render pass. Then capture it in an empty render pass.
// On replay we read the values via a compute shader.
TEST_P(CaptureAndReplayTests, CaptureDepth24Plus) {
    // TODO(477645283): This fails only on WARP and after it fails, all following tests
    // fail to create a device.
    DAWN_SUPPRESS_TEST_IF(IsWARP());

    constexpr uint32_t kNumLayers = 6;
    auto [cPipeline, commands] = [&]() {
        wgpu::Texture texture = CreateTexture(
            "myTexture", {1, 1, kNumLayers}, wgpu::TextureFormat::Depth24Plus,
            wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);

        const char* shader = R"(
            struct VOut {
              @builtin(position) pos: vec4f,
              @location(0) @interpolate(flat, either) instance_index: u32,
            };

            @vertex fn vs(@builtin(vertex_index) vNdx: u32,
                          @builtin(instance_index) iNdx: u32)
                 -> VOut {
                let pos = array(
                  vec2f(-1, -1),
                  vec2f(-1,  3),
                  vec2f( 3, -1),
                );
                return VOut(vec4f(pos[vNdx], 0, 1), iNdx);
            }

            @fragment fn fs(v: VOut) -> @builtin(frag_depth) f32 {
                return (f32(v.instance_index) + 0.5) / 6.0;
            }

            @group(0) @binding(0) var tex: texture_2d_array<f32>;
            @group(0) @binding(1) var<storage, read_write> result: array<f32>;

            @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) gid: vec3u) {
                result[gid.x] = textureLoad(tex, vec2u(0), gid.x, 0).x;
            }
        )";
        auto module = utils::CreateShaderModule(device, shader);

        // Put values in depth texture. This step is not captured.
        {
            utils::ComboRenderPipelineDescriptor desc;
            desc.vertex.module = module;
            desc.cFragment.module = module;
            desc.cFragment.targetCount = 0;
            desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

            wgpu::DepthStencilState* depthStencil =
                desc.EnableDepthStencil(wgpu::TextureFormat::Depth24Plus);
            depthStencil->depthWriteEnabled = wgpu::OptionalBool::True;
            depthStencil->depthCompare = wgpu::CompareFunction::Always;

            wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

            wgpu::CommandBuffer commands;
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            for (uint32_t layer = 0; layer < kNumLayers; ++layer) {
                wgpu::TextureViewDescriptor viewDesc;
                viewDesc.baseArrayLayer = layer;
                viewDesc.arrayLayerCount = 1;

                utils::ComboRenderPassDescriptor passDescriptor({}, texture.CreateView(&viewDesc));
                passDescriptor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;
                passDescriptor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
                passDescriptor.cDepthStencilAttachmentInfo.stencilStoreOp =
                    wgpu::StoreOp::Undefined;
                wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
                pass.SetPipeline(pipeline);
                pass.Draw(3, 1, 0, layer);
                pass.End();
            }
            commands = encoder.Finish();
            queue.Submit(1, &commands);
        }

        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = module;
        wgpu::ComputePipeline cPipeline = device.CreateComputePipeline(&csDesc);

        // Copy texture to temp buffer via compute shader during capture.
        // We don't care about the temp buffer. We just care that texture is
        // referenced so it will appear during replay.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::Buffer tempBuffer =
                CreateBuffer("temp", sizeof(float) * kNumLayers,
                             wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);
            wgpu::BindGroup bindGroup =
                utils::MakeBindGroup(device, cPipeline.GetBindGroupLayout(0),
                                     {
                                         {0, texture.CreateView()},
                                         {1, tempBuffer},
                                     });

            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.SetPipeline(cPipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.DispatchWorkgroups(kNumLayers);
            pass.End();
        }
        return std::make_pair(cPipeline, encoder.Finish());
    }();

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    // Read the replay version of myTexture in result via compute shader.
    wgpu::Buffer result = CreateBuffer("result", sizeof(float) * kNumLayers,
                                       wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        wgpu::Texture texture = replay->GetObjectByLabel<wgpu::Texture>("myTexture");
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, cPipeline.GetBindGroupLayout(0),
                                                         {
                                                             {0, texture.CreateView()},
                                                             {1, result},
                                                         });

        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(cPipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(kNumLayers);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    float expected[kNumLayers];
    for (uint32_t i = 0; i < kNumLayers; ++i) {
        expected[i] = (i + 0.5f) / 6.f;
    }
    EXPECT_BUFFER_FLOAT_RANGE_TOLERANCE_EQ(expected, result, 0, 6, 0.05);
}

// Test that you can not copy Depth24plus/Depth24PlusStencil8 depth aspect
// The spec requires this to be validated out but the WGPU backend enables copying
// on the inner device by the use_blit_for_depth24plus_texture_to_buffer_copy
// toggle. Make sure that even when that toggle is enabled copying these formats
// is validated out.
TEST_P(CaptureAndReplayTests, Depth24PlusNotCopyable) {
    // No validation errors if validation is off.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::Buffer buffer = CreateBuffer("buf", 4, wgpu::BufferUsage::CopyDst);
    for (wgpu::TextureFormat format :
         {wgpu::TextureFormat::Depth24Plus, wgpu::TextureFormat::Depth24PlusStencil8}) {
        wgpu::Texture texture = CreateTexture("tex", {1}, format, wgpu::TextureUsage::CopySrc);

        wgpu::CommandBuffer commands;
        {
            // Copy srcTexture to dstBuffer
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::TexelCopyTextureInfo texelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
                texture, 0, {0, 0, 0}, wgpu::TextureAspect::DepthOnly);
            wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
                utils::CreateTexelCopyBufferInfo(buffer, 0, 256, 1);
            wgpu::Extent3D extent = {1, 1, 1};

            encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &extent);
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }
}

// Test that capturing and replaying a BindGroup with an ExternalTexture works.
TEST_P(CaptureAndReplayTests, BindGroupWithExternalTexture) {
    wgpu::TextureDescriptor textureDesc;
    textureDesc.size = {1, 1, 1};
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture texture = device.CreateTexture(&textureDesc);

    wgpu::ExternalTexture externalTexture = utils::MakePassthroughExternalTexture(device, texture);

    const char* shader = R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0, 0, 0, 1);
        }

        @group(0) @binding(0) var s : sampler;
        @group(0) @binding(1) var t : texture_external;

        @fragment fn main(@builtin(position) FragCoord : vec4f)
                                    -> @location(0) vec4f {
            return textureSampleBaseClampToEdge(t, s, FragCoord.xy / vec2f(4.0, 4.0));
        }
    )";
    auto module = utils::CreateShaderModule(device, shader);

    // Create an explicit bind group layout.
    auto bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::Filtering},
                 {1, wgpu::ShaderStage::Fragment, &utils::kExternalTextureBindingLayout}});

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);
    descriptor.vertex.module = module;
    descriptor.cFragment.module = module;
    descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    auto pipeline = device.CreateRenderPipeline(&descriptor);

    auto sampler = device.CreateSampler();

    auto bg = utils::MakeBindGroup(device, bgl, {{0, sampler}, {1, externalTexture}});
    bg.SetLabel("MyBindGroup");

    auto renderTexture = CreateTexture("renderTexture", {1}, wgpu::TextureFormat::RGBA8Unorm,
                                       wgpu::TextureUsage::RenderAttachment);
    utils::ComboRenderPassDescriptor renderPass({renderTexture.CreateView()});
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    {
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bg);
        pass.Draw(3);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    // Expect no errors.
    // Verify that the bind group exists in the replayed device.
    auto replayedBg = replay->GetObjectByLabel<wgpu::BindGroup>("MyBindGroup");
    EXPECT_NE(replayedBg, nullptr);
}

DAWN_INSTANTIATE_TEST(CaptureAndReplayTests, WebGPUBackend());

class CaptureAndReplayDrawTests : public CaptureAndReplayTests {
  public:
    // Sets up point-list render pipeline to a 1x1 rgba8uint texture
    // and expects texture to be 'expected'
    template <typename Func, typename T>
    void TestDrawCommand(Func fn, const T& expected) {
        wgpu::Texture dstTexture = CreateTexture("dstTexture", {1}, wgpu::TextureFormat::RGBA8Uint,
                                                 wgpu::TextureUsage::RenderAttachment);

        wgpu::Texture depthTexture =
            CreateTexture("depthTexture", {1}, wgpu::TextureFormat::Depth24PlusStencil8,
                          wgpu::TextureUsage::RenderAttachment);

        const char* shader = R"(
            struct VOut {
                @builtin(position) pos: vec4f,
                @location(0) @interpolate(flat, either) params: vec4u,
            };

            @vertex fn vs(
                @builtin(vertex_index) vNdx: u32,
                @builtin(instance_index) iNdx: u32) -> VOut
            {
                return VOut(
                    vec4f(0, 0, 0, 1),
                    vec4u(vNdx, iNdx, 0x33, 0x44));
            }

            @fragment fn fs(v: VOut) -> @location(0) vec4u {
                return v.params;
            }
        )";
        auto module = utils::CreateShaderModule(device, shader);

        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = module;
        desc.cFragment.module = module;
        desc.cFragment.targetCount = 1;
        desc.depthStencil = &desc.cDepthStencil;
        desc.cTargets[0].format = wgpu::TextureFormat::RGBA8Uint;
        desc.cDepthStencil.format = wgpu::TextureFormat::Depth24PlusStencil8;
        desc.cDepthStencil.depthCompare = wgpu::CompareFunction::Always;
        desc.cDepthStencil.stencilFront.compare = wgpu::CompareFunction::Equal;
        desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

        wgpu::CommandBuffer commands;
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

            utils::ComboRenderPassDescriptor passDescriptor({dstTexture.CreateView()},
                                                            depthTexture.CreateView());
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
            pass.SetPipeline(pipeline);
            fn(pass);
            pass.End();

            commands = encoder.Finish();
        }

        // --- capture ---
        auto recorder = Recorder::CreateAndStart(device);

        queue.Submit(1, &commands);

        // --- replay ---
        auto capture = recorder.Finish();
        auto replay = capture.Replay(device);

        ExpectTextureEQ(replay.get(), "dstTexture", {1}, expected);
    }
};

// Capture DrawIndexed
TEST_P(CaptureAndReplayDrawTests, CaptureDrawIndexed) {
    uint32_t indices[] = {0x10, 0x20, 0x30};
    wgpu::Buffer indexBuffer = CreateBuffer("index", sizeof(indices),
                                            wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index);
    queue.WriteBuffer(indexBuffer, 0, indices, sizeof(indices));

    utils::RGBA8 expected[] = {{0x32, 0x3, 0x33, 0x44}};
    TestDrawCommand(
        [&](wgpu::RenderPassEncoder pass) {
            pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
            pass.DrawIndexed(1,   // indexCount
                             1,   // instanceCount
                             2,   // firstIndex,
                             2,   // baseVertex,
                             3);  // firstInstance
        },
        expected);
}

// Capture DrawIndirect
TEST_P(CaptureAndReplayDrawTests, CaptureDrawIndirect) {
    uint32_t indirect[] = {
        0x11,  // vertexCount
        0x22,  // instanceCount
        0,     // firstVertex
        0,     // firstInstance (must be 0 without "indirect-first-instance")
    };
    wgpu::Buffer indirectBuffer = CreateBuffer(
        "indirect", sizeof(indirect), wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Indirect);
    queue.WriteBuffer(indirectBuffer, 0, indirect, sizeof(indirect));

    utils::RGBA8 expected[] = {{0x10, 0x21, 0x33, 0x44}};
    TestDrawCommand([&](wgpu::RenderPassEncoder pass) { pass.DrawIndirect(indirectBuffer, 0); },
                    expected);
}

// Capture DrawIndexedIndirect
TEST_P(CaptureAndReplayDrawTests, CaptureDrawIndexedIndirect) {
    uint32_t indices[] = {10, 20};
    wgpu::Buffer indexBuffer = CreateBuffer("index", sizeof(indices),
                                            wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index);

    uint32_t indirectIndexed[] = {
        1,   // indexCount
        10,  // instanceCount
        1,   // firstIndex
        3,   // baseVertex
        0,   // firstInstance (must be 0 without "indirect-first-instance")
    };
    wgpu::Buffer indirectIndexedBuffer =
        CreateBuffer("indirectIndexed", sizeof(indirectIndexed),
                     wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Indirect);
    queue.WriteBuffer(indirectIndexedBuffer, 0, indirectIndexed, sizeof(indirectIndexed));

    utils::RGBA8 expected[] = {{0x3, 9, 0x33, 0x44}};
    TestDrawCommand(
        [&](wgpu::RenderPassEncoder pass) {
            pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
            pass.DrawIndexedIndirect(indirectIndexedBuffer, 0);
        },
        expected);
}

// Capture SetViewport. Draws twice. The second draw should be ignored because
// its out of the viewport.
TEST_P(CaptureAndReplayDrawTests, CaptureSetViewport) {
    utils::RGBA8 expected[] = {{0x11, 0x22, 0x33, 0x44}};
    TestDrawCommand(
        [&](wgpu::RenderPassEncoder pass) {
            pass.Draw(1, 1, 0x11, 0x22);
            pass.SetViewport(1, 1, 1, 1, 0, 1);
            pass.Draw(1, 1, 0x1, 0x2);
        },
        expected);
}

// Capture SetScissorRect. Draws twice. The second draw should be ignored because
// its out of the scissor rect.
TEST_P(CaptureAndReplayDrawTests, CaptureSetScissorRect) {
    // TODO(464436694): Zero size scissor fails on Intel Mac for this case.
    DAWN_SUPPRESS_TEST_IF(IsWebGPUOn(wgpu::BackendType::Metal) && IsIntel());

    utils::RGBA8 expected[] = {{0x11, 0x22, 0x33, 0x44}};
    TestDrawCommand(
        [&](wgpu::RenderPassEncoder pass) {
            pass.Draw(1, 1, 0x11, 0x22);
            pass.SetScissorRect(0, 0, 0, 0);
            pass.Draw(1, 1, 0x1, 0x2);
        },
        expected);
}

// Capture SetStencilReference. Draws twice. The second draw should be ignored because
// it doesn't match the stencil reference.
TEST_P(CaptureAndReplayDrawTests, CaptureSetStencilReference) {
    utils::RGBA8 expected[] = {{0x11, 0x22, 0x33, 0x44}};
    TestDrawCommand(
        [&](wgpu::RenderPassEncoder pass) {
            pass.Draw(1, 1, 0x11, 0x22);
            pass.SetStencilReference(1);
            pass.Draw(1, 1, 0x1, 0x2);
        },
        expected);
}

DAWN_INSTANTIATE_TEST(CaptureAndReplayDrawTests, WebGPUBackend());

class CaptureAndReplayTimestampTests : public CaptureAndReplayTests {
  protected:
    void SetUp() override {
        CaptureAndReplayTests::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(
            !SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalTimestampQueryInsidePasses}));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalTimestampQueryInsidePasses})) {
            requiredFeatures.push_back(
                wgpu::FeatureName::ChromiumExperimentalTimestampQueryInsidePasses);
            requiredFeatures.push_back(wgpu::FeatureName::TimestampQuery);
        }
        return requiredFeatures;
    }
};

// Test WriteTimestamp in compute pass, render pass, and
// command buffer. We don't expect any results. We only
// expect it doesn't get any errors.
TEST_P(CaptureAndReplayTimestampTests, WriteTimestamp) {
    wgpu::QuerySetDescriptor qsDesc;
    qsDesc.label = "myQuerySet";
    qsDesc.count = 3;
    qsDesc.type = wgpu::QueryType::Timestamp;
    wgpu::QuerySet querySet = device.CreateQuerySet(&qsDesc);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.WriteTimestamp(querySet, 0);
        {
            utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.WriteTimestamp(querySet, 1);
            pass.End();
        }
        {
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
            pass.WriteTimestamp(querySet, 2);
            pass.End();
        }
        commands = encoder.Finish();
    }

    // --- capture ---
    auto recorder = Recorder::CreateAndStart(device);

    queue.Submit(1, &commands);

    // --- replay ---
    auto capture = recorder.Finish();
    auto replay = capture.Replay(device);

    // just expect no errors.
}

DAWN_INSTANTIATE_TEST(CaptureAndReplayTimestampTests, WebGPUBackend());

}  // anonymous namespace
}  // namespace dawn
