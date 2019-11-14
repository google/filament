/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <assert.h>

#include <jni.h>

#include <filament/Stream.h>
#include <backend/PixelBufferDescriptor.h>

#include "common/NioUtils.h"
#include "common/CallbackUtils.h"

#ifdef ANDROID

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
using namespace backend;

class StreamBuilder {
public:
    StreamBuilder() noexcept {
        mBuilder = new Stream::Builder{};
    }

    ~StreamBuilder() {
        assert(mStreamSource == nullptr);
        delete mBuilder;
    }

    Stream::Builder* builder() const noexcept { return mBuilder; }

    void setStreamSource(JNIEnv* env, jobject streamSource) noexcept {
        mStreamSource = env->NewGlobalRef(streamSource);
        mBuilder->stream(mStreamSource);
    }

    void cleanup(JNIEnv* env) {
        // This will be invoked by the GC thread, which may not have
        // the same JNIEnv
        if (mStreamSource) {
            env->DeleteGlobalRef(mStreamSource);
            mStreamSource = nullptr;
        }
    }

private:
    Stream::Builder* mBuilder = nullptr;
    jobject mStreamSource = nullptr;
};

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Stream_nCreateBuilder(JNIEnv*, jclass) {
    return (jlong) new StreamBuilder{};
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Stream_nDestroyBuilder(JNIEnv* env, jclass,
        jlong nativeStreamBuilder) {
    StreamBuilder* builder = (StreamBuilder*) nativeStreamBuilder;
    builder->cleanup(env);
    delete builder;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Stream_nBuilderStreamSource(JNIEnv* env,
        jclass, jlong nativeStreamBuilder, jobject streamSource) {
    StreamBuilder* builder = (StreamBuilder*) nativeStreamBuilder;
    builder->setStreamSource(env, streamSource);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Stream_nBuilderStream(JNIEnv*, jclass,
        jlong nativeStreamBuilder, jlong externalTextureId) {
    StreamBuilder* builder = (StreamBuilder*) nativeStreamBuilder;
    builder->builder()->stream(externalTextureId);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Stream_nBuilderWidth(JNIEnv*, jclass,
        jlong nativeStreamBuilder, jint width) {
    StreamBuilder* builder = (StreamBuilder*) nativeStreamBuilder;
    builder->builder()->width((uint32_t) width);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Stream_nBuilderHeight(JNIEnv*, jclass,
        jlong nativeStreamBuilder, jint height) {
    StreamBuilder* builder = (StreamBuilder*) nativeStreamBuilder;
    builder->builder()->height((uint32_t) height);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Stream_nBuilderBuild(JNIEnv*, jclass,
        jlong nativeStreamBuilder, jlong nativeEngine) {
    StreamBuilder* builder = (StreamBuilder*) nativeStreamBuilder;
    Engine* engine = (Engine*) nativeEngine;
    return (jlong) builder->builder()->build(*engine);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Stream_nGetStreamType(JNIEnv*, jclass, jlong nativeStream) {
    Stream* stream = (Stream*) nativeStream;
    return (jint) stream->getStreamType();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Stream_nSetDimensions(JNIEnv*, jclass, jlong nativeStream,
        jint width, jint height) {
    Stream* stream = (Stream*) nativeStream;
    stream->setDimensions((uint32_t) width, (uint32_t) height);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Stream_nReadPixels(JNIEnv *env, jclass,
        jlong nativeStream, jlong nativeEngine,
        jint xoffset, jint yoffset, jint width, jint height,
        jobject storage, jint remaining,
        jint left, jint top, jint type, jint alignment, jint stride, jint format,
        jobject handler, jobject runnable) {
    Stream *stream = (Stream *) nativeStream;
    Engine *engine = (Engine *) nativeEngine;

    stride = stride ? stride : width;
    size_t sizeInBytes = PixelBufferDescriptor::computeDataSize(
            (PixelDataFormat) format, (PixelDataType) type,
            (size_t) stride, (size_t) (height + top), (size_t) alignment);

    AutoBuffer nioBuffer(env, storage, 0);
    if (sizeInBytes > (remaining << nioBuffer.getShift())) {
        // BufferOverflowException
        return -1;
    }

    void *buffer = nioBuffer.getData();
    auto *callback = JniBufferCallback::make(engine, env, handler, runnable, std::move(nioBuffer));

    PixelBufferDescriptor desc(buffer, sizeInBytes, (backend::PixelDataFormat) format,
            (backend::PixelDataType) type, (uint8_t) alignment, (uint32_t) left, (uint32_t) top,
            (uint32_t) stride, &JniBufferCallback::invoke, callback);

    stream->readPixels(uint32_t(xoffset), uint32_t(yoffset), uint32_t(width), uint32_t(height),
            std::move(desc));

    return 0;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Stream_nGetTimestamp(JNIEnv*, jclass, jlong nativeStream) {
    Stream *stream = (Stream *) nativeStream;
    return stream->getTimestamp();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Stream_nSetAcquiredImage(JNIEnv* env, jclass, jlong nativeStream,
        jlong nativeEngine, jobject hwbuffer, jobject handler, jobject runnable) {
    Engine* engine = (Engine*) nativeEngine;
    Stream* stream = (Stream*) nativeStream;

#ifdef ANDROID

    // This function is not available before NDK 15 or before Android 8.
    if (UTILS_UNLIKELY(!AHardwareBuffer_fromHardwareBuffer_fn)) {
        if (!sHardwareBufferSupported) {
            return;
        }
        AHardwareBuffer_fromHardwareBuffer_fn = (PFN_FROMHARDWAREBUFFER) dlsym(RTLD_DEFAULT, "AHardwareBuffer_fromHardwareBuffer");
         if (!AHardwareBuffer_fromHardwareBuffer_fn) {
            __android_log_print(ANDROID_LOG_WARN, "Filament", "AHardwareBuffer_fromHardwareBuffer is not available.");
            sHardwareBufferSupported = false;
        }
        return;
    }

    AHardwareBuffer* nativeBuffer = AHardwareBuffer_fromHardwareBuffer_fn(env, hwbuffer);
    if (!nativeBuffer) {
        __android_log_print(ANDROID_LOG_INFO, "Filament", "Unable to obtain native HardwareBuffer.");
        return;
    }

    auto* callback = JniImageCallback::make(engine, env, handler, runnable, (long) nativeBuffer);

#else

    // TODO: for non-Android platforms, it is unclear how to go from "jobject" to "void*"
    // For now this code is reserved for future use.
    auto* callback = JniImageCallback::make(engine, env, handler, runnable, 0);
    void* nativeBuffer = nullptr;

#endif

    stream->setAcquiredImage((void*) nativeBuffer, &JniImageCallback::invoke, callback);
}
