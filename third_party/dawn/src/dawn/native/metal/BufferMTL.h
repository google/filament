// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_METAL_BUFFERMTL_H_
#define SRC_DAWN_NATIVE_METAL_BUFFERMTL_H_

#include "dawn/common/NSRef.h"
#include "dawn/common/SerialQueue.h"
#include "dawn/native/Buffer.h"

#import <Metal/Metal.h>

namespace dawn::native::metal {

class CommandRecordingContext;
class Device;

class Buffer final : public BufferBase {
  public:
    static ResultOrError<Ref<Buffer>> Create(Device* device,
                                             const UnpackedPtr<BufferDescriptor>& descriptor);

    Buffer(DeviceBase* device, const UnpackedPtr<BufferDescriptor>& descriptor);

    id<MTLBuffer> GetMTLBuffer() const;

    void TrackUsage();
    bool EnsureDataInitialized(CommandRecordingContext* commandContext);
    bool EnsureDataInitializedAsDestination(CommandRecordingContext* commandContext,
                                            uint64_t offset,
                                            uint64_t size);
    bool EnsureDataInitializedAsDestination(CommandRecordingContext* commandContext,
                                            const CopyTextureToBufferCmd* copy);

    static uint64_t QueryMaxBufferLength(id<MTLDevice> mtlDevice);

  private:
    using BufferBase::BufferBase;
    MaybeError Initialize(bool mappedAtCreation);
    MaybeError InitializeHostMapped(const BufferHostMappedPointer* regionDesc);

    ~Buffer() override;

    MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) override;
    void UnmapImpl() override;
    void DestroyImpl() override;
    void SetLabelImpl() override;
    void* GetMappedPointerImpl() override;
    bool IsCPUWritableAtCreation() const override;
    MaybeError MapAtCreationImpl() override;

    void InitializeToZero(CommandRecordingContext* commandContext);
    void ClearBuffer(CommandRecordingContext* commandContext,
                     uint8_t clearValue,
                     uint64_t offset = 0,
                     uint64_t size = 0);

    NSPRef<id<MTLBuffer>> mMtlBuffer;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_BUFFERMTL_H_
