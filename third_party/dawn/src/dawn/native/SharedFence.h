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

#ifndef SRC_DAWN_NATIVE_SHAREDFENCE_H_
#define SRC_DAWN_NATIVE_SHAREDFENCE_H_

#include "dawn/native/Error.h"
#include "dawn/native/ObjectBase.h"

namespace dawn::native {

struct SharedFenceDescriptor;
struct SharedFenceExportInfo;

class SharedFenceBase : public ApiObjectBase {
  public:
    static Ref<SharedFenceBase> MakeError(DeviceBase* device,
                                          const SharedFenceDescriptor* descriptor);

    ObjectType GetType() const override;

    void APIExportInfo(SharedFenceExportInfo* info) const;

    MaybeError ExportInfo(SharedFenceExportInfo* info) const;

  protected:
    SharedFenceBase(DeviceBase* device, StringView label);
    SharedFenceBase(DeviceBase* device,
                    const SharedFenceDescriptor* descriptor,
                    ObjectBase::ErrorTag tag);

  private:
    void DestroyImpl() override;
    virtual MaybeError ExportInfoImpl(UnpackedPtr<SharedFenceExportInfo>& info) const = 0;
};

struct FenceAndSignalValue {
    Ref<SharedFenceBase> object;
    uint64_t signaledValue;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SHAREDFENCE_H_
