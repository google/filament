/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include <mutex>
#include <unordered_map>

#include <jni.h>

#include <filament/Stream.h>

#include <math/mat3.h>

#include "common/CallbackUtils.h"
#include <common/JniUtils.h>

#ifdef __ANDROID__

#if __has_include(<android/hardware_buffer_jni.h>)
#include <android/hardware_buffer_jni.h>
#else
struct AHardwareBuffer;
typedef struct AHardwareBuffer AHardwareBuffer;
#endif

#include <android/log.h>

#include <dlfcn.h>

using PFN_FROMHARDWAREBUFFER = AHardwareBuffer* (*)(JNIEnv*, jobject);
static PFN_FROMHARDWAREBUFFER AHardwareBuffer_fromHardwareBuffer_fn = nullptr;
static bool sHardwareBufferSupported = true;

#endif

using namespace filament;
using namespace filament::math;

static std::mutex sStreamSourceLock;
static std::unordered_map<Stream::Builder*, jobject> sBuilderStreamSources;

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_android_StreamHelper_nSetStreamSource(JNIEnv* env, jclass,
        jobject builderObj, jobject surfaceTexture) {
    jclass builderClass = env->GetObjectClass(builderObj);
    jfieldID nativeBuilderField = env->GetFieldID(builderClass, "mNativeBuilder", "J");
    Stream::Builder* builder = (Stream::Builder*) env->GetLongField(builderObj, nativeBuilderField);

    std::lock_guard<std::mutex> guard(sStreamSourceLock);
    auto it = sBuilderStreamSources.find(builder);
    if (it != sBuilderStreamSources.end()) {
        env->DeleteGlobalRef(it->second);
        sBuilderStreamSources.erase(it);
    }
    jobject globalRef = env->NewGlobalRef(surfaceTexture);
    sBuilderStreamSources[builder] = globalRef;

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
    builder->stream(globalRef);
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_android_StreamHelper_nClearStreamSource(JNIEnv* env, jclass,
        jobject builderObj) {
    jclass builderClass = env->GetObjectClass(builderObj);
    jfieldID nativeBuilderField = env->GetFieldID(builderClass, "mNativeBuilder", "J");
    Stream::Builder* builder = (Stream::Builder*) env->GetLongField(builderObj, nativeBuilderField);

    std::lock_guard<std::mutex> guard(sStreamSourceLock);
    auto it = sBuilderStreamSources.find(builder);
    if (it != sBuilderStreamSources.end()) {
        env->DeleteGlobalRef(it->second);
        sBuilderStreamSources.erase(it);
    }
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
    builder->stream(nullptr);
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_android_StreamHelper_nSetAcquiredImage(JNIEnv* env, jclass,
        jlong nativeStream, jobject hwbuffer, jobject handler, jobject runnable, jfloatArray transform_) {
    Stream* stream = (Stream*) nativeStream;

#ifdef __ANDROID__
    if (UTILS_UNLIKELY(!AHardwareBuffer_fromHardwareBuffer_fn)) {
        if (!sHardwareBufferSupported) {
            return;
        }
        AHardwareBuffer_fromHardwareBuffer_fn = (PFN_FROMHARDWAREBUFFER) dlsym(RTLD_DEFAULT, "AHardwareBuffer_fromHardwareBuffer");
        if (!AHardwareBuffer_fromHardwareBuffer_fn) {
            __android_log_print(ANDROID_LOG_WARN, "Filament", "AHardwareBuffer_fromHardwareBuffer is not available.");
            sHardwareBufferSupported = false;
            return;
        }
    }

    AHardwareBuffer* nativeBuffer = AHardwareBuffer_fromHardwareBuffer_fn(env, hwbuffer);
    if (!nativeBuffer) {
        __android_log_print(ANDROID_LOG_INFO, "Filament", "Unable to obtain native HardwareBuffer.");
        return;
    }

    auto* callback = JniImageCallback::make(nullptr, env, handler, runnable, (long) nativeBuffer);
#else
    auto* callback = JniImageCallback::make(nullptr, env, handler, runnable, 0);
    void* nativeBuffer = nullptr;
#endif

    mat3f transform;
    if (transform_) {
        jfloat const* const transformElements = env->GetFloatArrayElements(transform_, nullptr);
        transform = *reinterpret_cast<mat3f const*>(transformElements);
        env->ReleaseFloatArrayElements(transform_, const_cast<jfloat*>(transformElements), JNI_ABORT);
    }

    stream->setAcquiredImage((void*) nativeBuffer,
            callback->getHandler(), &JniImageCallback::postToJavaAndDestroy, callback, transform);
}
