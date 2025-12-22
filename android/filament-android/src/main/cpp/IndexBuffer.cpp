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

#include <jni.h>

#include <functional>
#include <stdlib.h>
#include <string.h>

#include <filament/IndexBuffer.h>

#include <backend/BufferDescriptor.h>

#include "common/CallbackUtils.h"
#include "common/NioUtils.h"
#include "../../../../common/JniExceptionBridge.h"

using namespace filament;
using namespace backend;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_IndexBuffer_nCreateBuilder(JNIEnv *env, jclass type) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_IndexBuffer_nCreateBuilder", 0, [&]() -> jlong {
            return (jlong) new IndexBuffer::Builder();
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_IndexBuffer_nDestroyBuilder(JNIEnv *env, jclass type,
        jlong nativeBuilder) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_IndexBuffer_nDestroyBuilder", [&]() {
            IndexBuffer::Builder* builder = (IndexBuffer::Builder *) nativeBuilder;
            delete builder;
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_IndexBuffer_nBuilderIndexCount(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint indexCount) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_IndexBuffer_nBuilderIndexCount", [&]() {
            IndexBuffer::Builder* builder = (IndexBuffer::Builder *) nativeBuilder;
            builder->indexCount((uint32_t) indexCount);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_IndexBuffer_nBuilderBufferType(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint indexType) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_IndexBuffer_nBuilderBufferType", [&]() {
            using IndexType = IndexBuffer::IndexType;
            IndexBuffer::Builder* builder = (IndexBuffer::Builder *) nativeBuilder;
            IndexType types[] = {IndexType::USHORT, IndexType::UINT};
            builder->bufferType(types[indexType & 1]);
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_IndexBuffer_nBuilderBuild(JNIEnv *env, jclass type,
        jlong nativeBuilder, jlong nativeEngine) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_IndexBuffer_nBuilderBuild", 0, [&]() -> jlong {
            IndexBuffer::Builder* builder = (IndexBuffer::Builder *) nativeBuilder;
            Engine *engine = (Engine *) nativeEngine;
            return (jlong) builder->build(*engine);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_IndexBuffer_nGetIndexCount(JNIEnv *env, jclass type,
        jlong nativeIndexBuffer) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_IndexBuffer_nGetIndexCount", 0, [&]() -> jint {
            IndexBuffer *indexBuffer = (IndexBuffer *) nativeIndexBuffer;
            return (jint) indexBuffer->getIndexCount();
    });
}

extern "C" JNIEXPORT int JNICALL
Java_com_google_android_filament_IndexBuffer_nSetBuffer(JNIEnv *env, jclass type,
        jlong nativeIndexBuffer, jlong nativeEngine, jobject buffer, int remaining,
        jint destOffsetInBytes, jint count,
        jobject handler, jobject runnable) {
    return filament::android::jniGuard<int>(env, "Java_com_google_android_filament_IndexBuffer_nSetBuffer", 0, [&]() -> int {
            IndexBuffer *indexBuffer = (IndexBuffer *) nativeIndexBuffer;
            Engine *engine = (Engine *) nativeEngine;

            AutoBuffer nioBuffer(env, buffer, count);
            void* data = nioBuffer.getData();
            size_t sizeInBytes = nioBuffer.getSize();
            if (sizeInBytes > (remaining << nioBuffer.getShift())) {
                // BufferOverflowException
                return -1;
            }

            auto* callback = JniBufferCallback::make(engine, env, handler, runnable, std::move(nioBuffer));

            BufferDescriptor desc(data, sizeInBytes,
                    callback->getHandler(), &JniBufferCallback::postToJavaAndDestroy, callback);

            indexBuffer->setBuffer(*engine, std::move(desc), (uint32_t) destOffsetInBytes);

            return 0;
    });
}
