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

#ifndef SRC_DAWN_NATIVE_METAL_SHAREDTEXTUREFENCEMTL_H_
#define SRC_DAWN_NATIVE_METAL_SHAREDTEXTUREFENCEMTL_H_

#include <os/availability.h>
#include <vector>

#include "dawn/common/NSRef.h"
#include "dawn/native/Error.h"
#include "dawn/native/SharedFence.h"

@protocol MTLSharedEvent;

namespace dawn::native::metal {

class Device;

class SharedFence final : public SharedFenceBase {
  public:
    static ResultOrError<Ref<SharedFence>>
    Create(Device* device, StringView label, const SharedFenceMTLSharedEventDescriptor* descriptor);

    id<MTLSharedEvent> GetMTLSharedEvent() const;

  private:
    SharedFence(Device* device, StringView label, id<MTLSharedEvent> sharedEvent);

    MaybeError ExportInfoImpl(UnpackedPtr<SharedFenceExportInfo>& info) const override;

    NSPRef<id> mSharedEvent;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_SHAREDTEXTUREFENCEMTL_H_
