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

#include <gtest/gtest.h>
#include <webgpu/webgpu_cpp.h>

#include <string_view>
#include <utility>

#include "dawn/native/ChainUtils.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/tests/StringViewMatchers.h"
#include "mocks/BufferMock.h"
#include "mocks/ComputePipelineMock.h"
#include "mocks/DawnMockTest.h"
#include "mocks/DeviceMock.h"
#include "mocks/ExternalTextureMock.h"
#include "mocks/PipelineLayoutMock.h"
#include "mocks/RenderPipelineMock.h"
#include "mocks/ShaderModuleMock.h"
#include "mocks/TextureMock.h"

namespace dawn::native {
namespace {

using ::testing::_;
using ::testing::ByMove;
using ::testing::HasSubstr;
using ::testing::MockCallback;
using ::testing::MockCppCallback;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SizedStringMatches;
using ::testing::StrictMock;
using ::testing::Test;

using MockComputePipelineAsyncCallback =
    MockCppCallback<wgpu::CreateComputePipelineAsyncCallback<void>*>;
using MockRenderPipelineAsyncCallback =
    MockCppCallback<wgpu::CreateRenderPipelineAsyncCallback<void>*>;

static constexpr char kOomErrorMessage[] = "Out of memory error";
static constexpr char kInternalErrorMessage[] = "Internal error";

static constexpr std::string_view kComputeShader = R"(
        @compute @workgroup_size(1) fn main() {}
    )";

static constexpr std::string_view kVertexShader = R"(
        @vertex fn main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 0.0);
        }
    )";

class AllowedErrorTests : public DawnMockTest {};

//
// Exercise APIs where OOM errors cause a device lost.
//

TEST_F(AllowedErrorTests, QueueSubmit) {
    EXPECT_CALL(*(mDeviceMock->GetQueueMock()), SubmitImpl)
        .WillOnce(Return(ByMove(DAWN_OUT_OF_MEMORY_ERROR(kOomErrorMessage))));

    // Expect the device lost because of the error.
    EXPECT_CALL(mDeviceLostCallback, Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown,
                                          SizedStringMatches(HasSubstr(kOomErrorMessage))))
        .Times(1);

    device.GetQueue().Submit(0, nullptr);
}

TEST_F(AllowedErrorTests, QueueWriteBuffer) {
    BufferDescriptor desc = {};
    desc.size = 1;
    desc.usage = wgpu::BufferUsage::CopyDst;
    Ref<BufferMock> bufferMock = AcquireRef(new NiceMock<BufferMock>(mDeviceMock, &desc));
    wgpu::Buffer buffer = wgpu::Buffer::Acquire(ToAPI(ReturnToAPI(std::move(bufferMock))));

    EXPECT_CALL(*(mDeviceMock->GetQueueMock()), WriteBufferImpl)
        .WillOnce(Return(ByMove(DAWN_OUT_OF_MEMORY_ERROR(kOomErrorMessage))));

    // Expect the device lost because of the error.
    EXPECT_CALL(mDeviceLostCallback, Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown,
                                          SizedStringMatches(HasSubstr(kOomErrorMessage))))
        .Times(1);

    constexpr uint8_t data = 8;
    device.GetQueue().WriteBuffer(buffer, 0, &data, 0);
}

TEST_F(AllowedErrorTests, QueueWriteTexture) {
    TextureDescriptor desc = {};
    desc.size.width = 1;
    desc.size.height = 1;
    desc.usage = wgpu::TextureUsage::CopyDst;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    Ref<TextureMock> textureMock = AcquireRef(new NiceMock<TextureMock>(mDeviceMock, &desc));
    wgpu::Texture texture = wgpu::Texture::Acquire(ToAPI(ReturnToAPI(std::move(textureMock))));

    EXPECT_CALL(*(mDeviceMock->GetQueueMock()), WriteTextureImpl)
        .WillOnce(Return(ByMove(DAWN_OUT_OF_MEMORY_ERROR(kOomErrorMessage))));

    // Expect the device lost because of the error.
    EXPECT_CALL(mDeviceLostCallback, Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown,
                                          SizedStringMatches(HasSubstr(kOomErrorMessage))))
        .Times(1);

    constexpr uint8_t data[] = {1, 2, 4, 8};
    wgpu::TexelCopyTextureInfo dest = {};
    dest.texture = texture;
    wgpu::TexelCopyBufferLayout layout = {};
    wgpu::Extent3D size = {1, 1};
    device.GetQueue().WriteTexture(&dest, &data, 4, &layout, &size);
}

// Even though OOM is allowed in buffer creation, when creating a buffer in internal workaround the
// OOM should be masked as a device loss.
TEST_F(AllowedErrorTests, QueueCopyTextureForBrowserOomBuffer) {
    wgpu::TextureDescriptor desc = {};
    desc.size = {4, 4};
    desc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                 wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::TexelCopyTextureInfo src = {};
    src.texture = device.CreateTexture(&desc);
    wgpu::TexelCopyTextureInfo dst = {};
    dst.texture = device.CreateTexture(&desc);
    wgpu::Extent3D size = {4, 4};
    wgpu::CopyTextureForBrowserOptions options = {};

    // Copying texture for browser internally allocates a buffer which we will cause to fail here.
    EXPECT_CALL(*mDeviceMock, CreateBufferImpl)
        .WillOnce(Return(ByMove(DAWN_OUT_OF_MEMORY_ERROR(kOomErrorMessage))));

    // Expect the device lost because of the error.
    EXPECT_CALL(mDeviceLostCallback, Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown,
                                          SizedStringMatches(HasSubstr(kOomErrorMessage))))
        .Times(1);
    device.GetQueue().CopyTextureForBrowser(&src, &dst, &size, &options);
}

// Even though OOM is allowed in buffer creation, when creating a buffer in internal workaround the
// OOM should be masked as a device loss.
TEST_F(AllowedErrorTests, QueueCopyExternalTextureForBrowserOomBuffer) {
    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.size = {4, 4};
    textureDesc.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst |
                        wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::TextureViewDescriptor textureViewDesc = {};
    textureViewDesc.format = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::ExternalTextureDescriptor externalTextureDesc = {};
    std::array<float, 12> placeholderConstantArray;
    externalTextureDesc.yuvToRgbConversionMatrix = placeholderConstantArray.data();
    externalTextureDesc.gamutConversionMatrix = placeholderConstantArray.data();
    externalTextureDesc.srcTransferFunctionParameters = placeholderConstantArray.data();
    externalTextureDesc.dstTransferFunctionParameters = placeholderConstantArray.data();
    externalTextureDesc.cropOrigin = {0, 0};
    externalTextureDesc.cropSize = {4, 4};
    externalTextureDesc.apparentSize = {4, 4};
    externalTextureDesc.plane0 = device.CreateTexture(&textureDesc).CreateView(&textureViewDesc);

    wgpu::ImageCopyExternalTexture src = {};
    src.externalTexture = device.CreateExternalTexture(&externalTextureDesc);
    src.naturalSize = {4, 4};
    wgpu::TexelCopyTextureInfo dst = {};
    dst.texture = device.CreateTexture(&textureDesc);
    wgpu::Extent3D size = {4, 4};
    wgpu::CopyTextureForBrowserOptions options = {};

    // Copying texture for browser internally allocates a buffer which we will cause to fail here.
    EXPECT_CALL(*mDeviceMock, CreateBufferImpl)
        .WillOnce(Return(ByMove(DAWN_OUT_OF_MEMORY_ERROR(kOomErrorMessage))));

    // Expect the device lost because of the error.
    EXPECT_CALL(mDeviceLostCallback, Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown,
                                          SizedStringMatches(HasSubstr(kOomErrorMessage))))
        .Times(1);
    device.GetQueue().CopyExternalTextureForBrowser(&src, &dst, &size, &options);
}

// OOM error from synchronously initializing a compute pipeline should result in a device loss.
TEST_F(AllowedErrorTests, CreateComputePipeline) {
    Ref<ShaderModuleMock> csModule = ShaderModuleMock::Create(mDeviceMock, kComputeShader.data());

    ComputePipelineDescriptor desc = {};
    desc.compute.module = csModule.Get();

    Ref<ComputePipelineMock> computePipelineMock = ComputePipelineMock::Create(mDeviceMock, &desc);
    EXPECT_CALL(*computePipelineMock.Get(), InitializeImpl)
        .WillOnce(Return(ByMove(DAWN_OUT_OF_MEMORY_ERROR(kOomErrorMessage))));
    EXPECT_CALL(*mDeviceMock, CreateUninitializedComputePipelineImpl)
        .WillOnce(Return(ByMove(std::move(computePipelineMock))));

    // Expect the device lost because of the error.
    EXPECT_CALL(mDeviceLostCallback, Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown,
                                          SizedStringMatches(HasSubstr(kOomErrorMessage))))
        .Times(1);
    device.CreateComputePipeline(ToCppAPI(&desc));
}

// OOM error from synchronously initializing a render pipeline should result in a device loss.
TEST_F(AllowedErrorTests, CreateRenderPipeline) {
    Ref<ShaderModuleMock> vsModule = ShaderModuleMock::Create(mDeviceMock, kVertexShader.data());

    DepthStencilState ds = {};
    ds.format = wgpu::TextureFormat::Depth32Float;
    ds.depthWriteEnabled = wgpu::OptionalBool::True;
    ds.depthCompare = wgpu::CompareFunction::Always;

    RenderPipelineDescriptor desc = {};
    desc.vertex.module = vsModule.Get();
    desc.depthStencil = &ds;

    Ref<RenderPipelineMock> renderPipelineMock = RenderPipelineMock::Create(mDeviceMock, &desc);
    EXPECT_CALL(*renderPipelineMock.Get(), InitializeImpl)
        .WillOnce(Return(ByMove(DAWN_OUT_OF_MEMORY_ERROR(kOomErrorMessage))));
    EXPECT_CALL(*mDeviceMock, CreateUninitializedRenderPipelineImpl)
        .WillOnce(Return(ByMove(std::move(renderPipelineMock))));

    // Expect the device lost because of the error.
    EXPECT_CALL(mDeviceLostCallback, Call(CHandleIs(device.Get()), wgpu::DeviceLostReason::Unknown,
                                          SizedStringMatches(HasSubstr(kOomErrorMessage))))
        .Times(1);
    device.CreateRenderPipeline(ToCppAPI(&desc));
}

// Internal error from synchronously initializing a compute pipeline should not result in a device
// loss.
TEST_F(AllowedErrorTests, CreateComputePipelineInternalError) {
    Ref<ShaderModuleMock> csModule = ShaderModuleMock::Create(mDeviceMock, kComputeShader.data());

    ComputePipelineDescriptor desc = {};
    desc.compute.module = csModule.Get();

    Ref<ComputePipelineMock> computePipelineMock = ComputePipelineMock::Create(mDeviceMock, &desc);
    EXPECT_CALL(*computePipelineMock.Get(), InitializeImpl)
        .WillOnce(Return(ByMove(DAWN_INTERNAL_ERROR(kInternalErrorMessage))));
    EXPECT_CALL(*mDeviceMock, CreateUninitializedComputePipelineImpl)
        .WillOnce(Return(ByMove(std::move(computePipelineMock))));

    // Expect the internal error.
    EXPECT_CALL(mDeviceErrorCallback, Call(CHandleIs(device.Get()), wgpu::ErrorType::Internal,
                                           SizedStringMatches(HasSubstr(kInternalErrorMessage))))
        .Times(1);
    device.CreateComputePipeline(ToCppAPI(&desc));
}

// Internal error from synchronously initializing a render pipeline should not result in a device
// loss.
TEST_F(AllowedErrorTests, CreateRenderPipelineInternalError) {
    Ref<ShaderModuleMock> vsModule = ShaderModuleMock::Create(mDeviceMock, kVertexShader.data());

    DepthStencilState ds = {};
    ds.format = wgpu::TextureFormat::Depth32Float;
    ds.depthWriteEnabled = wgpu::OptionalBool::True;
    ds.depthCompare = wgpu::CompareFunction::Always;

    RenderPipelineDescriptor desc = {};
    desc.vertex.module = vsModule.Get();
    desc.depthStencil = &ds;

    Ref<RenderPipelineMock> renderPipelineMock = RenderPipelineMock::Create(mDeviceMock, &desc);
    EXPECT_CALL(*renderPipelineMock.Get(), InitializeImpl)
        .WillOnce(Return(ByMove(DAWN_INTERNAL_ERROR(kInternalErrorMessage))));
    EXPECT_CALL(*mDeviceMock, CreateUninitializedRenderPipelineImpl)
        .WillOnce(Return(ByMove(std::move(renderPipelineMock))));

    // Expect the internal error.
    EXPECT_CALL(mDeviceErrorCallback, Call(CHandleIs(device.Get()), wgpu::ErrorType::Internal,
                                           SizedStringMatches(HasSubstr(kInternalErrorMessage))))
        .Times(1);
    device.CreateRenderPipeline(ToCppAPI(&desc));
}

//
// Exercise async APIs where OOM errors do NOT currently cause a device lost.
//

// OOM error from asynchronously initializing a compute pipeline should not result in a device loss.
TEST_F(AllowedErrorTests, CreateComputePipelineAsync) {
    Ref<ShaderModuleMock> csModule = ShaderModuleMock::Create(mDeviceMock, kComputeShader.data());

    ComputePipelineDescriptor desc = {};
    desc.compute.module = csModule.Get();

    Ref<ComputePipelineMock> computePipelineMock = ComputePipelineMock::Create(mDeviceMock, &desc);
    EXPECT_CALL(*computePipelineMock.Get(), InitializeImpl)
        .WillOnce(Return(ByMove(DAWN_OUT_OF_MEMORY_ERROR(kOomErrorMessage))));
    EXPECT_CALL(*mDeviceMock, CreateUninitializedComputePipelineImpl)
        .WillOnce(Return(ByMove(std::move(computePipelineMock))));

    MockComputePipelineAsyncCallback cb;
    EXPECT_CALL(cb, Call(wgpu::CreatePipelineAsyncStatus::InternalError, _,
                         SizedStringMatches(HasSubstr(kOomErrorMessage))))
        .Times(1);

    device.CreateComputePipelineAsync(ToCppAPI(&desc), wgpu::CallbackMode::AllowProcessEvents,
                                      cb.Callback());
    ProcessEvents();
}

// OOM error from asynchronously initializing a render pipeline should not result in a device loss.
TEST_F(AllowedErrorTests, CreateRenderPipelineAsync) {
    Ref<ShaderModuleMock> vsModule = ShaderModuleMock::Create(mDeviceMock, kVertexShader.data());

    DepthStencilState ds = {};
    ds.format = wgpu::TextureFormat::Depth32Float;
    ds.depthWriteEnabled = wgpu::OptionalBool::True;
    ds.depthCompare = wgpu::CompareFunction::Always;

    RenderPipelineDescriptor desc = {};
    desc.vertex.module = vsModule.Get();
    desc.depthStencil = &ds;

    Ref<RenderPipelineMock> renderPipelineMock = RenderPipelineMock::Create(mDeviceMock, &desc);
    EXPECT_CALL(*renderPipelineMock.Get(), InitializeImpl)
        .WillOnce(Return(ByMove(DAWN_OUT_OF_MEMORY_ERROR(kOomErrorMessage))));
    EXPECT_CALL(*mDeviceMock, CreateUninitializedRenderPipelineImpl)
        .WillOnce(Return(ByMove(std::move(renderPipelineMock))));

    MockRenderPipelineAsyncCallback cb;
    EXPECT_CALL(cb, Call(wgpu::CreatePipelineAsyncStatus::InternalError, _,
                         SizedStringMatches(HasSubstr(kOomErrorMessage))))
        .Times(1);

    device.CreateRenderPipelineAsync(ToCppAPI(&desc), wgpu::CallbackMode::AllowProcessEvents,
                                     cb.Callback());
    ProcessEvents();
}

// Internal error from asynchronously initializing a compute pipeline should not result in a device
// loss.
TEST_F(AllowedErrorTests, CreateComputePipelineAsyncInternalError) {
    Ref<ShaderModuleMock> csModule = ShaderModuleMock::Create(mDeviceMock, kComputeShader.data());

    ComputePipelineDescriptor desc = {};
    desc.compute.module = csModule.Get();

    Ref<ComputePipelineMock> computePipelineMock = ComputePipelineMock::Create(mDeviceMock, &desc);
    EXPECT_CALL(*computePipelineMock.Get(), InitializeImpl)
        .WillOnce(Return(ByMove(DAWN_INTERNAL_ERROR(kInternalErrorMessage))));
    EXPECT_CALL(*mDeviceMock, CreateUninitializedComputePipelineImpl)
        .WillOnce(Return(ByMove(std::move(computePipelineMock))));

    MockComputePipelineAsyncCallback cb;
    EXPECT_CALL(cb, Call(wgpu::CreatePipelineAsyncStatus::InternalError, _,
                         HasSubstr(kInternalErrorMessage)))
        .Times(1);

    device.CreateComputePipelineAsync(ToCppAPI(&desc), wgpu::CallbackMode::AllowProcessEvents,
                                      cb.Callback());
    ProcessEvents();
}

// Internal error from asynchronously initializing a render pipeline should not result in a device
// loss.
TEST_F(AllowedErrorTests, CreateRenderPipelineAsyncInternalError) {
    Ref<ShaderModuleMock> vsModule = ShaderModuleMock::Create(mDeviceMock, kVertexShader.data());

    DepthStencilState ds = {};
    ds.format = wgpu::TextureFormat::Depth32Float;
    ds.depthWriteEnabled = wgpu::OptionalBool::True;
    ds.depthCompare = wgpu::CompareFunction::Always;

    RenderPipelineDescriptor desc = {};
    desc.vertex.module = vsModule.Get();
    desc.depthStencil = &ds;

    Ref<RenderPipelineMock> renderPipelineMock = RenderPipelineMock::Create(mDeviceMock, &desc);
    EXPECT_CALL(*renderPipelineMock.Get(), InitializeImpl)
        .WillOnce(Return(ByMove(DAWN_INTERNAL_ERROR(kInternalErrorMessage))));
    EXPECT_CALL(*mDeviceMock, CreateUninitializedRenderPipelineImpl)
        .WillOnce(Return(ByMove(std::move(renderPipelineMock))));

    MockRenderPipelineAsyncCallback cb;
    EXPECT_CALL(cb, Call(wgpu::CreatePipelineAsyncStatus::InternalError, _,
                         HasSubstr(kInternalErrorMessage)))
        .Times(1);

    device.CreateRenderPipelineAsync(ToCppAPI(&desc), wgpu::CallbackMode::AllowProcessEvents,
                                     cb.Callback());
    ProcessEvents();
}

//
// Exercise APIs where OOM error are allowed and surfaced.
//

// OOM error from buffer creation is allowed and surfaced directly.
TEST_F(AllowedErrorTests, CreateBuffer) {
    EXPECT_CALL(*mDeviceMock, CreateBufferImpl)
        .WillOnce(Return(ByMove(DAWN_OUT_OF_MEMORY_ERROR(kOomErrorMessage))));

    // Expect the OOM error.
    EXPECT_CALL(mDeviceErrorCallback, Call(CHandleIs(device.Get()), wgpu::ErrorType::OutOfMemory,
                                           SizedStringMatches(HasSubstr(kOomErrorMessage))))
        .Times(1);

    wgpu::BufferDescriptor desc = {};
    desc.usage = wgpu::BufferUsage::Uniform;
    desc.size = 16;
    device.CreateBuffer(&desc);
}

// OOM error from texture creation is allowed and surfaced directly.
TEST_F(AllowedErrorTests, CreateTexture) {
    EXPECT_CALL(*mDeviceMock, CreateTextureImpl)
        .WillOnce(Return(ByMove(DAWN_OUT_OF_MEMORY_ERROR(kOomErrorMessage))));

    // Expect the OOM error.
    EXPECT_CALL(mDeviceErrorCallback, Call(CHandleIs(device.Get()), wgpu::ErrorType::OutOfMemory,
                                           SizedStringMatches(HasSubstr(kOomErrorMessage))))
        .Times(1);

    wgpu::TextureDescriptor desc = {};
    desc.usage = wgpu::TextureUsage::CopySrc;
    desc.size = {4, 4};
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    device.CreateTexture(&desc);
}

// OOM error from query set creation is allowed and surfaced directly.
TEST_F(AllowedErrorTests, CreateQuerySet) {
    EXPECT_CALL(*mDeviceMock, CreateQuerySetImpl)
        .WillOnce(Return(ByMove(DAWN_OUT_OF_MEMORY_ERROR(kOomErrorMessage))));

    // Expect the OOM error.
    EXPECT_CALL(mDeviceErrorCallback, Call(CHandleIs(device.Get()), wgpu::ErrorType::OutOfMemory,
                                           SizedStringMatches(HasSubstr(kOomErrorMessage))))
        .Times(1);

    wgpu::QuerySetDescriptor desc = {};
    desc.type = wgpu::QueryType::Occlusion;
    desc.count = 1;
    device.CreateQuerySet(&desc);
}

TEST_F(AllowedErrorTests, InjectError) {
    // Expect the OOM error.
    EXPECT_CALL(mDeviceErrorCallback, Call(CHandleIs(device.Get()), wgpu::ErrorType::OutOfMemory,
                                           SizedStringMatches(HasSubstr(kOomErrorMessage))))
        .Times(1);

    device.InjectError(wgpu::ErrorType::OutOfMemory, kOomErrorMessage);
}

}  // anonymous namespace
}  // namespace dawn::native
