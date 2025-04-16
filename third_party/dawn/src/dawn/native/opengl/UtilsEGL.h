// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_OPENGL_UTILSEGL_H_
#define SRC_DAWN_NATIVE_OPENGL_UTILSEGL_H_

#include "dawn/common/NonMovable.h"
#include "dawn/common/RefCounted.h"
#include "dawn/common/egl_platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/opengl/DisplayEGL.h"

namespace dawn::native::opengl {

class EGLFunctions;

const char* EGLErrorAsString(EGLint error);
MaybeError CheckEGL(const EGLFunctions& egl, EGLBoolean result, const char* context);

class WrappedEGLSync : public RefCounted, NonMovable {
  public:
    static ResultOrError<Ref<WrappedEGLSync>> Create(DisplayEGL* display,
                                                     EGLenum type,
                                                     const EGLint* attribs);
    static ResultOrError<Ref<WrappedEGLSync>> AcquireExternal(DisplayEGL* display, EGLSync sync);

    EGLSync Get() const;

    // Call eglSignalSync with given mode. Requires sync type of EGL_SYNC_REUSABLE_KHR.
    MaybeError Signal(EGLenum mode);

    // Call eglClientWaitSync
    ResultOrError<EGLenum> ClientWait(EGLint flags, Nanoseconds timeout);

    // Call eglWaitSync (server wait)
    MaybeError Wait();

    // Call eglDupNativeFenceFDANDROID
    ResultOrError<EGLint> DupFD();

  protected:
    WrappedEGLSync(DisplayEGL* display, EGLSync sync, bool ownsSync);
    ~WrappedEGLSync() override;

  private:
    Ref<DisplayEGL> mDisplay;
    EGLSync mSync = nullptr;
    bool mOwnsSync = false;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_UTILSEGL_H_
