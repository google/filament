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

#ifndef SRC_DAWN_NATIVE_OPENGL_SHARED_FENCE_EGL_H_
#define SRC_DAWN_NATIVE_OPENGL_SHARED_FENCE_EGL_H_

#include "dawn/native/SystemHandle.h"
#include "dawn/native/opengl/SharedFenceGL.h"
#include "dawn/native/opengl/UtilsEGL.h"

namespace dawn::native::opengl {

class Device;

class SharedFenceEGL : public SharedFence {
  public:
    static ResultOrError<Ref<SharedFence>> Create(Device* device,
                                                  StringView label,
                                                  const SharedFenceSyncFDDescriptor* descriptor);
    static ResultOrError<Ref<SharedFence>> Create(Device* device,
                                                  StringView label,
                                                  const SharedFenceEGLSyncDescriptor* descriptor);

    SharedFenceEGL(Device* device,
                   StringView label,
                   wgpu::SharedFenceType type,
                   SystemHandle&& handle,
                   Ref<WrappedEGLSync> sync);

    MaybeError ServerWait(uint64_t signaledValue) override;

  private:
    MaybeError ExportInfoImpl(UnpackedPtr<SharedFenceExportInfo>& info) const override;

    wgpu::SharedFenceType mType;
    SystemHandle mHandle;
    Ref<WrappedEGLSync> mSync;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_SHARED_FENCE_EGL_H_
