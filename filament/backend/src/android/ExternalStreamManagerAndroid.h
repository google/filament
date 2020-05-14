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

#ifndef TNT_FILAMENT_DRIVER_ANDROID_EXTERNAL_STREAM_MANAGER_ANDROID_H
#define TNT_FILAMENT_DRIVER_ANDROID_EXTERNAL_STREAM_MANAGER_ANDROID_H

#include "android/VirtualMachineEnv.h"

#include <backend/Platform.h>

#if __has_include(<android/surface_texture.h>)
#   include <android/surface_texture.h>
#   include <android/surface_texture_jni.h>
#else
struct ASurfaceTexture;
typedef struct ASurfaceTexture ASurfaceTexture;
#endif

namespace filament {

class ExternalStreamManagerAndroid {
public:
    using Stream = backend::Platform::Stream;

    ExternalStreamManagerAndroid() noexcept;
    static ExternalStreamManagerAndroid& get() noexcept;

    Stream* acquire(jobject surfaceTexture) noexcept;
    void release(Stream* stream) noexcept;
    void attach(Stream* stream, intptr_t tname) noexcept;
    void detach(Stream* stream) noexcept;
    void updateTexImage(Stream* stream, int64_t* timestamp) noexcept;

private:
    VirtualMachineEnv& mVm;
    JNIEnv* mJniEnv = nullptr;

    struct EGLStream : public Stream {
        jobject             jSurfaceTexture = nullptr;
        ASurfaceTexture*    nSurfaceTexture = nullptr;
    };

    inline JNIEnv* getEnvironment() noexcept {
        JNIEnv* env = mJniEnv;
        if (UTILS_UNLIKELY(!env)) {
            return getEnvironmentSlow();
        }
        return env;
    }

    JNIEnv* getEnvironmentSlow() noexcept;

    jmethodID mSurfaceTextureClass_updateTexImage;
    jmethodID mSurfaceTextureClass_getTimestamp;
    jmethodID mSurfaceTextureClass_attachToGLContext;
    jmethodID mSurfaceTextureClass_detachFromGLContext;

    ASurfaceTexture* (*ASurfaceTexture_fromSurfaceTexture)(JNIEnv*, jobject);
    void (*ASurfaceTexture_release)(ASurfaceTexture*);
    int  (*ASurfaceTexture_attachToGLContext)(ASurfaceTexture*, uint32_t);
    int  (*ASurfaceTexture_detachFromGLContext)(ASurfaceTexture*);
    int  (*ASurfaceTexture_updateTexImage)(ASurfaceTexture*);
    int64_t (*ASurfaceTexture_getTimestamp)(ASurfaceTexture*);   // available since api 28
};

} // namespace filament

#endif //TNT_FILAMENT_DRIVER_ANDROID_EXTERNAL_STREAM_MANAGER_ANDROID_H
