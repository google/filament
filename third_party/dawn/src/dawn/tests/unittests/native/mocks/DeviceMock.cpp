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

#include "dawn/tests/unittests/native/mocks/DeviceMock.h"

#include <utility>

#include "dawn/tests/unittests/native/mocks/BindGroupLayoutMock.h"
#include "dawn/tests/unittests/native/mocks/BindGroupMock.h"
#include "dawn/tests/unittests/native/mocks/BufferMock.h"
#include "dawn/tests/unittests/native/mocks/CommandBufferMock.h"
#include "dawn/tests/unittests/native/mocks/ComputePipelineMock.h"
#include "dawn/tests/unittests/native/mocks/ExternalTextureMock.h"
#include "dawn/tests/unittests/native/mocks/PipelineLayoutMock.h"
#include "dawn/tests/unittests/native/mocks/QuerySetMock.h"
#include "dawn/tests/unittests/native/mocks/QueueMock.h"
#include "dawn/tests/unittests/native/mocks/RenderPipelineMock.h"
#include "dawn/tests/unittests/native/mocks/SamplerMock.h"
#include "dawn/tests/unittests/native/mocks/ShaderModuleMock.h"
#include "dawn/tests/unittests/native/mocks/TextureMock.h"

namespace dawn::native {

using ::testing::NiceMock;
using ::testing::WithArgs;

DeviceMock::DeviceMock(AdapterBase* adapter,
                       const UnpackedPtr<DeviceDescriptor>& descriptor,
                       const TogglesState& deviceToggles,
                       Ref<DeviceLostEvent>&& lostEvent)
    : DeviceBase(adapter, descriptor, deviceToggles, std::move(lostEvent)) {
    // Set all default creation functions to return nice mock objects.
    ON_CALL(*this, CreateBindGroupImpl)
        .WillByDefault(WithArgs<0>(
            [this](const BindGroupDescriptor* descriptor) -> ResultOrError<Ref<BindGroupBase>> {
                return AcquireRef(new NiceMock<BindGroupMock>(this, descriptor));
            }));
    ON_CALL(*this, CreateBindGroupLayoutImpl)
        .WillByDefault(WithArgs<0>([this](const BindGroupLayoutDescriptor* descriptor)
                                       -> ResultOrError<Ref<BindGroupLayoutInternalBase>> {
            return AcquireRef(new NiceMock<BindGroupLayoutMock>(this, descriptor));
        }));
    ON_CALL(*this, CreateBufferImpl)
        .WillByDefault(WithArgs<0>([this](const UnpackedPtr<BufferDescriptor>& descriptor)
                                       -> ResultOrError<Ref<BufferBase>> {
            return AcquireRef(new NiceMock<BufferMock>(this, descriptor));
        }));
    ON_CALL(*this, CreateCommandBuffer)
        .WillByDefault(WithArgs<0, 1>(
            [this](CommandEncoder* encoder, const CommandBufferDescriptor* descriptor)
                -> ResultOrError<Ref<CommandBufferBase>> {
                return AcquireRef(new NiceMock<CommandBufferMock>(this, encoder, descriptor));
            }));
    ON_CALL(*this, CreateExternalTextureImpl)
        .WillByDefault(WithArgs<0>([this](const ExternalTextureDescriptor* descriptor)
                                       -> ResultOrError<Ref<ExternalTextureBase>> {
            return ExternalTextureMock::Create(this, descriptor);
        }));
    ON_CALL(*this, CreatePipelineLayoutImpl)
        .WillByDefault(WithArgs<0>([this](const UnpackedPtr<PipelineLayoutDescriptor>& descriptor)
                                       -> ResultOrError<Ref<PipelineLayoutBase>> {
            return AcquireRef(new NiceMock<PipelineLayoutMock>(this, descriptor));
        }));
    ON_CALL(*this, CreateQuerySetImpl)
        .WillByDefault(WithArgs<0>(
            [this](const QuerySetDescriptor* descriptor) -> ResultOrError<Ref<QuerySetBase>> {
                return AcquireRef(new NiceMock<QuerySetMock>(this, descriptor));
            }));
    ON_CALL(*this, CreateSamplerImpl)
        .WillByDefault(WithArgs<0>(
            [this](const SamplerDescriptor* descriptor) -> ResultOrError<Ref<SamplerBase>> {
                return AcquireRef(new NiceMock<SamplerMock>(this, descriptor));
            }));
    ON_CALL(*this, CreateShaderModuleImpl)
        .WillByDefault(WithArgs<0>([this](const UnpackedPtr<ShaderModuleDescriptor>& descriptor)
                                       -> ResultOrError<Ref<ShaderModuleBase>> {
            return ShaderModuleMock::Create(this, descriptor);
        }));
    ON_CALL(*this, CreateTextureImpl)
        .WillByDefault(WithArgs<0>([this](const UnpackedPtr<TextureDescriptor>& descriptor)
                                       -> ResultOrError<Ref<TextureBase>> {
            return AcquireRef(new NiceMock<TextureMock>(this, descriptor));
        }));
    ON_CALL(*this, CreateTextureViewImpl)
        .WillByDefault(WithArgs<0, 1>(
            [](TextureBase* texture, const UnpackedPtr<TextureViewDescriptor>& descriptor)
                -> ResultOrError<Ref<TextureViewBase>> {
                return AcquireRef(new NiceMock<TextureViewMock>(texture, descriptor));
            }));
    ON_CALL(*this, CreateUninitializedComputePipelineImpl)
        .WillByDefault(WithArgs<0>([this](const UnpackedPtr<ComputePipelineDescriptor>& descriptor)
                                       -> Ref<ComputePipelineBase> {
            return ComputePipelineMock::Create(this, descriptor);
        }));
    ON_CALL(*this, CreateUninitializedRenderPipelineImpl)
        .WillByDefault(WithArgs<0>([this](const UnpackedPtr<RenderPipelineDescriptor>& descriptor)
                                       -> Ref<RenderPipelineBase> {
            return RenderPipelineMock::Create(this, descriptor);
        }));

    // By default, the mock's TickImpl will succeed.
    ON_CALL(*this, TickImpl).WillByDefault([]() -> MaybeError { return {}; });

    // Initialize the device.
    GetInstance()->GetEventManager()->TrackEvent(mLostEvent);
    QueueDescriptor desc = {};
    EXPECT_FALSE(
        Initialize(descriptor, AcquireRef(new NiceMock<QueueMock>(this, &desc))).IsError());
}

DeviceMock::~DeviceMock() = default;

QueueMock* DeviceMock::GetQueueMock() {
    return reinterpret_cast<QueueMock*>(GetQueue());
}

}  // namespace dawn::native
