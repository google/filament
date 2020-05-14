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

#include "ExternalStreamManagerAndroid.h"

#include <utils/api_level.h>
#include <utils/compiler.h>
#include <utils/Log.h>

#include <dlfcn.h>

#include <chrono>

using namespace utils;

namespace filament {

using namespace backend;
using Stream = Platform::Stream;


template <typename T>
static void loadSymbol(T*& pfn, const char *symbol) noexcept {
    pfn = (T*)dlsym(RTLD_DEFAULT, symbol);
}

ExternalStreamManagerAndroid& ExternalStreamManagerAndroid::get() noexcept {
    // declaring this thread local, will ensure it's destroyed with the calling thread
    static UTILS_DECLARE_TLS(ExternalStreamManagerAndroid) instance;
    return instance;
}

ExternalStreamManagerAndroid::ExternalStreamManagerAndroid() noexcept
        : mVm(VirtualMachineEnv::get()) {

    // the following dlsym() calls don't work on API 19
    if (api_level() >= 21) {
        loadSymbol(ASurfaceTexture_fromSurfaceTexture, "ASurfaceTexture_fromSurfaceTexture");
        loadSymbol(ASurfaceTexture_release, "ASurfaceTexture_release");
        loadSymbol(ASurfaceTexture_attachToGLContext, "ASurfaceTexture_attachToGLContext");
        loadSymbol(ASurfaceTexture_detachFromGLContext, "ASurfaceTexture_detachFromGLContext");
        loadSymbol(ASurfaceTexture_updateTexImage, "ASurfaceTexture_updateTexImage");
        loadSymbol(ASurfaceTexture_getTimestamp, "ASurfaceTexture_getTimestamp");
    }

    if (ASurfaceTexture_fromSurfaceTexture) {
        slog.d << "Using ASurfaceTexture" << io::endl;
    }
}

UTILS_NOINLINE
JNIEnv* ExternalStreamManagerAndroid::getEnvironmentSlow() noexcept {
    JNIEnv * env = mVm.getEnvironment();
    mJniEnv = env;

    // we try to call directly the native version of updateTexImage() -- it's not 100% portable
    // but we mitigate this by falling back on the correct method if we don't find it.
    jclass SurfaceTextureClass = env->FindClass("android/graphics/SurfaceTexture");
    mSurfaceTextureClass_updateTexImage = env->GetMethodID(
            SurfaceTextureClass, "nativeUpdateTexImage", "()V");
    if (!mSurfaceTextureClass_updateTexImage) {
        mSurfaceTextureClass_updateTexImage = env->GetMethodID(
                SurfaceTextureClass, "updateTexImage", "()V");
    }

    mSurfaceTextureClass_attachToGLContext = env->GetMethodID(
            SurfaceTextureClass, "attachToGLContext", "(I)V");

    mSurfaceTextureClass_detachFromGLContext = env->GetMethodID(
            SurfaceTextureClass, "detachFromGLContext", "()V");

    mSurfaceTextureClass_getTimestamp = env->GetMethodID(
            SurfaceTextureClass, "getTimestamp", "()J");

    return env;
}

Stream* ExternalStreamManagerAndroid::acquire(jobject surfaceTexture) noexcept {
    // note: This is called on the application thread (not the GL thread)
    JNIEnv* env = VirtualMachineEnv::getThreadEnvironment();
    if (!env) {
        return nullptr; // this should not happen
    }
    EGLStream* stream = new EGLStream();
    stream->jSurfaceTexture = env->NewGlobalRef(surfaceTexture);
    if (ASurfaceTexture_fromSurfaceTexture) {
        stream->nSurfaceTexture = ASurfaceTexture_fromSurfaceTexture(env, surfaceTexture);
    }
    return stream;
}

void ExternalStreamManagerAndroid::release(Stream* handle) noexcept {
    EGLStream* stream = static_cast<EGLStream*>(handle);
    if (ASurfaceTexture_fromSurfaceTexture) {
        ASurfaceTexture_release(stream->nSurfaceTexture);
    }
    JNIEnv* const env = getEnvironment();
    assert(env); // we should have called attach() by now
    env->DeleteGlobalRef(stream->jSurfaceTexture);
    delete stream;
}

void ExternalStreamManagerAndroid::attach(Stream* handle, intptr_t tname) noexcept {
    EGLStream* stream = static_cast<EGLStream*>(handle);
    if (ASurfaceTexture_fromSurfaceTexture) {
        // associate our GL texture to the SurfaceTexture
        ASurfaceTexture* const aSurfaceTexture = stream->nSurfaceTexture;
        if (UTILS_UNLIKELY(ASurfaceTexture_attachToGLContext(aSurfaceTexture, (uint32_t)tname))) {
            // Unfortunately, before API 26 SurfaceTexture was always created in attached mode,
            // so attachToGLContext can fail. We consider this the unlikely case, because
            // this is how it should work.
            // So now we have to detach the surfacetexture from its texture
            ASurfaceTexture_detachFromGLContext(aSurfaceTexture);
            // and finally, try attaching again
            ASurfaceTexture_attachToGLContext(aSurfaceTexture, (uint32_t)tname);
        }
    } else {
        JNIEnv* const env = getEnvironment();
        assert(env); // we should have called attach() by now

        // associate our GL texture to the SurfaceTexture
        jobject jSurfaceTexture = stream->jSurfaceTexture;
        env->CallVoidMethod(jSurfaceTexture, mSurfaceTextureClass_attachToGLContext, (jint)tname);
        if (UTILS_UNLIKELY(env->ExceptionCheck())) {
            // Unfortunately, before API 26 SurfaceTexture was always created in attached mode,
            // so attachToGLContext can fail. We consider this the unlikely case, because
            // this is how it should work.
            env->ExceptionClear();

            // so now we have to detach the surfacetexture from its texture
            env->CallVoidMethod(jSurfaceTexture, mSurfaceTextureClass_detachFromGLContext);
            VirtualMachineEnv::handleException(env);

            // and finally, try attaching again
            env->CallVoidMethod(jSurfaceTexture, mSurfaceTextureClass_attachToGLContext, (jint)tname);
            VirtualMachineEnv::handleException(env);
        }
    }
}

void ExternalStreamManagerAndroid::detach(Stream* handle) noexcept {
    EGLStream* stream = static_cast<EGLStream*>(handle);
    if (ASurfaceTexture_fromSurfaceTexture) {
        ASurfaceTexture_detachFromGLContext(stream->nSurfaceTexture);
    } else {
        JNIEnv* const env = mVm.getEnvironment();
        assert(env); // we should have called attach() by now
        env->CallVoidMethod(stream->jSurfaceTexture, mSurfaceTextureClass_detachFromGLContext);
        VirtualMachineEnv::handleException(env);
    }
}

void ExternalStreamManagerAndroid::updateTexImage(Stream* handle, int64_t* timestamp) noexcept {
    EGLStream* stream = static_cast<EGLStream*>(handle);
    if (ASurfaceTexture_fromSurfaceTexture) {
        ASurfaceTexture_updateTexImage(stream->nSurfaceTexture);
        if (ASurfaceTexture_getTimestamp) {
            *timestamp = ASurfaceTexture_getTimestamp(stream->nSurfaceTexture);
        } else {
            // if we're not at least on API 28, we may not have getTimestamp()
            *timestamp = (std::chrono::steady_clock::now().time_since_epoch().count());
        }
    } else {
        JNIEnv* const env = mVm.getEnvironment();
        assert(env); // we should have called attach() by now
        env->CallVoidMethod(stream->jSurfaceTexture, mSurfaceTextureClass_updateTexImage);
        VirtualMachineEnv::handleException(env);
        *timestamp = env->CallLongMethod(stream->jSurfaceTexture, mSurfaceTextureClass_getTimestamp);
        VirtualMachineEnv::handleException(env);
    }
}

} // namespace filament
