// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_DEVICEMOCK_H_
#define SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_DEVICEMOCK_H_

#include <memory>
#include <vector>

#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/tests/unittests/native/mocks/QueueMock.h"
#include "gmock/gmock.h"

namespace dawn::native {

class BindGroupLayoutMock;

class DeviceMock : public DeviceBase {
  public:
    // Exposes some protected functions for testing purposes.
    using DeviceBase::DestroyObjects;
    using DeviceBase::ForceEnableFeatureForTesting;

    // TODO(chromium:42240655): Implement AdapterMock and use it in the constructor of DeviceMock
    DeviceMock(AdapterBase* adapter,
               const UnpackedPtr<DeviceDescriptor>& descriptor,
               const TogglesState& deviceToggles,
               Ref<DeviceLostEvent>&& lostEvent);
    ~DeviceMock() override;

    // Mock specific functionality.
    QueueMock* GetQueueMock();

    MOCK_METHOD(ResultOrError<Ref<CommandBufferBase>>,
                CreateCommandBuffer,
                (CommandEncoder*, const CommandBufferDescriptor*),
                (override));

    MOCK_METHOD(MaybeError,
                CopyFromStagingToBuffer,
                (BufferBase*, uint64_t, BufferBase*, uint64_t, uint64_t),
                (override));
    MOCK_METHOD(
        MaybeError,
        CopyFromStagingToTextureImpl,
        (const BufferBase*, const TexelCopyBufferLayout&, const TextureCopy&, const Extent3D&),
        (override));

    MOCK_METHOD(uint32_t, GetOptimalBytesPerRowAlignment, (), (const, override));
    MOCK_METHOD(uint64_t, GetOptimalBufferToTextureCopyOffsetAlignment, (), (const, override));

    MOCK_METHOD(float, GetTimestampPeriodInNS, (), (const, override));

    MOCK_METHOD(ResultOrError<Ref<BindGroupBase>>,
                CreateBindGroupImpl,
                (const BindGroupDescriptor*),
                (override));
    MOCK_METHOD(ResultOrError<Ref<BindGroupLayoutInternalBase>>,
                CreateBindGroupLayoutImpl,
                (const BindGroupLayoutDescriptor*),
                (override));
    MOCK_METHOD(ResultOrError<Ref<BufferBase>>,
                CreateBufferImpl,
                (const UnpackedPtr<BufferDescriptor>&),
                (override));
    MOCK_METHOD(Ref<ComputePipelineBase>,
                CreateUninitializedComputePipelineImpl,
                (const UnpackedPtr<ComputePipelineDescriptor>&),
                (override));
    MOCK_METHOD(ResultOrError<Ref<ExternalTextureBase>>,
                CreateExternalTextureImpl,
                (const ExternalTextureDescriptor*),
                (override));
    MOCK_METHOD(ResultOrError<Ref<PipelineLayoutBase>>,
                CreatePipelineLayoutImpl,
                (const UnpackedPtr<PipelineLayoutDescriptor>&),
                (override));
    MOCK_METHOD(ResultOrError<Ref<QuerySetBase>>,
                CreateQuerySetImpl,
                (const QuerySetDescriptor*),
                (override));
    MOCK_METHOD(Ref<RenderPipelineBase>,
                CreateUninitializedRenderPipelineImpl,
                (const UnpackedPtr<RenderPipelineDescriptor>&),
                (override));
    MOCK_METHOD(ResultOrError<Ref<SamplerBase>>,
                CreateSamplerImpl,
                (const SamplerDescriptor*),
                (override));
    MOCK_METHOD(ResultOrError<Ref<ShaderModuleBase>>,
                CreateShaderModuleImpl,
                (const UnpackedPtr<ShaderModuleDescriptor>&,
                 const std::vector<tint::wgsl::Extension>&,
                 ShaderModuleParseResult*),
                (override));
    MOCK_METHOD(ResultOrError<Ref<SwapChainBase>>,
                CreateSwapChainImpl,
                (Surface*, SwapChainBase*, const SurfaceConfiguration*),
                (override));
    MOCK_METHOD(ResultOrError<Ref<TextureBase>>,
                CreateTextureImpl,
                (const UnpackedPtr<TextureDescriptor>&),
                (override));
    MOCK_METHOD(ResultOrError<Ref<TextureViewBase>>,
                CreateTextureViewImpl,
                (TextureBase*, const UnpackedPtr<TextureViewDescriptor>&),
                (override));

    MOCK_METHOD(MaybeError, TickImpl, (), (override));

    MOCK_METHOD(void, DestroyImpl, (), (override));
};

}  // namespace dawn::native

#endif  // SRC_DAWN_TESTS_UNITTESTS_NATIVE_MOCKS_DEVICEMOCK_H_
