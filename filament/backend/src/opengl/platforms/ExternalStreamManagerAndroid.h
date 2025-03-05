/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_BACKEND_OPENGL_ANDROID_EXTERNAL_STREAM_MANAGER_ANDROID_H
#define TNT_FILAMENT_BACKEND_OPENGL_ANDROID_EXTERNAL_STREAM_MANAGER_ANDROID_H

#include "private/backend/VirtualMachineEnv.h"

#include <backend/Platform.h>

#include <utils/compiler.h>

#if __has_include(<android/surface_texture.h>)
#   include <android/surface_texture.h>
#else
struct ASurfaceTexture;
typedef struct ASurfaceTexture ASurfaceTexture;
#endif

#include <jni.h>

#include <stdint.h>
#include <math/mat3.h>

namespace filament::backend {

/*
 * ExternalStreamManagerAndroid::Stream is basically a wrapper for SurfaceTexture.
 *
 * This class DOES DEPEND on having a GLES context, because that's how SurfaceTexture works.
 */
class ExternalStreamManagerAndroid {
public:
    using Stream = Platform::Stream;

    // must be called on GLES thread
    static ExternalStreamManagerAndroid& create() noexcept;

    // must be called on GLES thread
    static void destroy(ExternalStreamManagerAndroid* pExternalStreamManagerAndroid) noexcept;

    Stream* acquire(jobject surfaceTexture) noexcept;
    void release(Stream* stream) noexcept;

    // attach Stream to current GLES context
    void attach(Stream* stream, intptr_t tname) noexcept;

    // detach Stream to current GLES context
    void detach(Stream* stream) noexcept;

    // must be called on GLES context thread, updates the stream content
    void updateTexImage(Stream* stream, int64_t* timestamp) noexcept;

    // must be called on GLES context thread, returns the transform matrix
    void getTransformMatrix(Stream* stream, math::mat3f* uvTransform) noexcept;

private:
    ExternalStreamManagerAndroid() noexcept;
    ~ExternalStreamManagerAndroid() noexcept;

    VirtualMachineEnv& mVm;
    JNIEnv* mJniEnv = nullptr;

    struct EGLStream : public Stream {
        jobject             jSurfaceTexture = nullptr;
        ASurfaceTexture*    nSurfaceTexture = nullptr;
    };

    // Must only be called from the backend thread
    JNIEnv* getEnvironment() noexcept {
        JNIEnv* env = mJniEnv;
        if (UTILS_UNLIKELY(!env)) {
            return getEnvironmentSlow();
        }
        return env;
    }

    JNIEnv* getEnvironmentSlow() noexcept;

    jmethodID mSurfaceTextureClass_updateTexImage{};
    jmethodID mSurfaceTextureClass_getTimestamp{};
    jmethodID mSurfaceTextureClass_getTransformMatrix{};
    jmethodID mSurfaceTextureClass_attachToGLContext{};
    jmethodID mSurfaceTextureClass_detachFromGLContext{};
};
} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_OPENGL_ANDROID_EXTERNAL_STREAM_MANAGER_ANDROID_H
