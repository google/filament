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

#ifndef SRC_DAWN_NATIVE_SHAREDBUFFERMEMORY_H_
#define SRC_DAWN_NATIVE_SHAREDBUFFERMEMORY_H_

#include "dawn/common/WeakRef.h"
#include "dawn/common/WeakRefSupport.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/SharedFence.h"
#include "dawn/native/SharedResourceMemory.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

struct SharedBufferMemoryDescriptor;
struct SharedBufferMemoryBeginAccessDescriptor;
struct SharedBufferMemoryEndAccessState;
struct SharedBufferMemoryProperties;
struct BufferDescriptor;

class SharedBufferMemoryBase : public SharedResourceMemory {
  public:
    using BeginAccessDescriptor = SharedBufferMemoryBeginAccessDescriptor;
    using EndAccessState = SharedBufferMemoryEndAccessState;
    static SharedBufferMemoryBase* MakeError(DeviceBase* device,
                                             const SharedBufferMemoryDescriptor* descriptor);

    wgpu::Status APIGetProperties(SharedBufferMemoryProperties* properties) const;
    BufferBase* APICreateBuffer(const BufferDescriptor* descriptor);

    ObjectType GetType() const override;

  protected:
    SharedBufferMemoryBase(DeviceBase* device,
                           StringView label,
                           const SharedBufferMemoryProperties& properties);
    SharedBufferMemoryBase(DeviceBase* device,
                           const SharedBufferMemoryDescriptor* descriptor,
                           ObjectBase::ErrorTag tag);

  private:
    ResultOrError<Ref<BufferBase>> CreateBuffer(const BufferDescriptor* rawDescriptor);

    virtual ResultOrError<Ref<BufferBase>> CreateBufferImpl(
        const UnpackedPtr<BufferDescriptor>& descriptor) = 0;

    SharedBufferMemoryProperties mProperties;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SHAREDBUFFERMEMORY_H_
