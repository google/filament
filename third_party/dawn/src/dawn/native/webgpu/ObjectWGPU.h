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

#ifndef SRC_DAWN_NATIVE_WEBGPU_OBJECTWGPU_H_
#define SRC_DAWN_NATIVE_WEBGPU_OBJECTWGPU_H_

#include <webgpu/webgpu.h>
#include "dawn/common/NonMovable.h"

namespace dawn::native::webgpu {

// This is the templated abstract base class for most WebGPU-on-WebGPU backend objects that has a
// corresponding WebGPU C API object.
// TODO(crbug.com/413053623): Add members needed for record/playback (e.g. ObjectIds)
template <typename WGPUHandle>
class ObjectWGPU : NonMovable {
  public:
    typedef void (*WGPUProcHandleRelease)(WGPUHandle);
    explicit ObjectWGPU(WGPUProcHandleRelease releaseProc) : mReleaseProc(releaseProc) {}
    WGPUHandle GetInnerHandle() const { return mInnerHandle; }

    virtual ~ObjectWGPU();

  protected:
    // The WebGPU C API handle of the "lower layer" object.
    // The inherited class is responsible to assign it properly.
    WGPUHandle mInnerHandle = nullptr;

  private:
    // The WebGPU C API handle release function pointer.
    // It will be called to release the mInnerHandle at the destructor.
    WGPUProcHandleRelease mReleaseProc = nullptr;
};

template <typename WGPUHandle>
ObjectWGPU<WGPUHandle>::~ObjectWGPU() {
    if (mInnerHandle != nullptr) {
        mReleaseProc(mInnerHandle);
        mInnerHandle = nullptr;
    }
}

}  // namespace dawn::native::webgpu

#endif  // SRC_DAWN_NATIVE_WEBGPU_OBJECTWGPU_H_
