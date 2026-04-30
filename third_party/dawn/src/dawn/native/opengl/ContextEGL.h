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

#ifndef SRC_DAWN_NATIVE_OPENGL_CONTEXTEGL_H_
#define SRC_DAWN_NATIVE_OPENGL_CONTEXTEGL_H_

#include <memory>
#include <mutex>
#include <thread>
#include <utility>

#include "dawn/common/NonMovable.h"
#include "dawn/common/Ref.h"
#include "dawn/common/egl_platform.h"
#include "dawn/native/opengl/EGLFunctions.h"
#include "dawn/native/opengl/opengl_platform.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::opengl {

class DisplayEGL;

class ContextEGL : NonMovable {
  public:
    static ResultOrError<std::unique_ptr<ContextEGL>> Create(Ref<DisplayEGL> display,
                                                             wgpu::BackendType backend,
                                                             bool useRobustness,
                                                             bool disableEGL15Robustness,
                                                             bool useANGLETextureSharing,
                                                             bool forceES31AndMinExtensions,
                                                             bool bindContextOnlyDuringUse,
                                                             EGLint angleVirtualizationGroup);

    ~ContextEGL();

    void RequestRequiredExtensionsExplicitly();
    bool IsInScopedMakeCurrent() const;

    // Make the surface used by all MakeCurrent until the scoper gets out of scope.
    class ScopedSetCurrentSurface : NonMovable {
      public:
        ScopedSetCurrentSurface(ContextEGL* context, EGLSurface surface);
        ~ScopedSetCurrentSurface();

      private:
        raw_ptr<ContextEGL> mContext;
    };
    [[nodiscard]] ScopedSetCurrentSurface SetCurrentSurfaceScope(EGLSurface surface);

    // TODO(451928481): remove this once all call sites use MakeCurrent
    void DeprecatedMakeCurrent();

    struct ContextState {
        EGLContext context = EGL_NO_CONTEXT;
        EGLSurface drawSurface = EGL_NO_SURFACE;
        EGLSurface readSurface = EGL_NO_SURFACE;

        bool operator==(const ContextState& other) const;
        bool operator!=(const ContextState& other) const;
    };

    class ScopedMakeCurrent {
      public:
        ScopedMakeCurrent() = default;
        ScopedMakeCurrent(ScopedMakeCurrent&& src);
        ScopedMakeCurrent(const ScopedMakeCurrent&) = delete;
        ~ScopedMakeCurrent();

        ScopedMakeCurrent& operator=(ScopedMakeCurrent&& src);
        ScopedMakeCurrent& operator=(const ScopedMakeCurrent&) = delete;

        MaybeError End();

      private:
        friend class ContextEGL;

        explicit ScopedMakeCurrent(ContextEGL* context);
        MaybeError Initialize();

        raw_ptr<ContextEGL> mContext = nullptr;
        ContextState mPrevState;
        std::unique_lock<std::mutex> mLock;
    };

    ResultOrError<ScopedMakeCurrent> MakeCurrent();

  private:
    explicit ContextEGL(Ref<DisplayEGL> display, bool bindContextOnlyDuringUse);

    MaybeError Initialize(wgpu::BackendType backend,
                          bool useRobustness,
                          bool useANGLETextureSharing,
                          bool disableEGL15Robustness,
                          bool forceES31AndMinExtensions,
                          EGLint angleVirtualizationGroup);

    bool IsNotCurrentOnAnotherThread() const;

    // This mutex is used to make sure only one thread can enter ScopedMakeCurrent at a time.
    std::mutex mExclusiveMakeCurrentMutex;

    Ref<DisplayEGL> mDisplay;
    ContextState mState;
    EGLSurface mOffscreenSurface = EGL_NO_SURFACE;
    bool mForceES31AndMinExtensions = false;

    // Flag to allow this context to be used on multiple threads. A GL context cannot be used on
    // multiple threads at the same time. But if one thread locks the context, then unbinds it, that
    // will allow the context to be used on another thread.
    const bool mBindContextOnlyDuringUse = false;

    // For debugging purpose, storing the id of the thread that has the context current.
#if defined(DAWN_ENABLE_ASSERTS)
    std::thread::id mCurrentThread;
#endif
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_CONTEXTEGL_H_
