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

#include <stdlib.h>
#include <string.h>

#include <filament/VertexBuffer.h>

#include <math/vec3.h>
#include <math/vec4.h>

#include "common/CallbackUtils.h"
#include "common/NioUtils.h"

using namespace filament;
using namespace filament::math;
using namespace backend;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_VertexBuffer_nCreateBuilder(JNIEnv*, jclass) {
    return (jlong) new VertexBuffer::Builder();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_VertexBuffer_nDestroyBuilder(JNIEnv*, jclass,
        jlong nativeBuilder) {
    VertexBuffer::Builder* builder = (VertexBuffer::Builder *) nativeBuilder;
    delete builder;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_VertexBuffer_nBuilderVertexCount(JNIEnv*, jclass,
        jlong nativeBuilder, jint vertexCount) {
    VertexBuffer::Builder* builder = (VertexBuffer::Builder *) nativeBuilder;
    builder->vertexCount((uint32_t) vertexCount);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_VertexBuffer_nBuilderEnableBufferObjects(JNIEnv*, jclass,
        jlong nativeBuilder, jboolean enabled) {
    VertexBuffer::Builder* builder = (VertexBuffer::Builder *) nativeBuilder;
    builder->enableBufferObjects(enabled);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_VertexBuffer_nBuilderBufferCount(JNIEnv*, jclass,
        jlong nativeBuilder, jint bufferCount) {
    VertexBuffer::Builder* builder = (VertexBuffer::Builder *) nativeBuilder;
    builder->bufferCount((uint8_t) bufferCount);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_VertexBuffer_nBuilderAttribute(JNIEnv*, jclass,
        jlong nativeBuilder, jint attribute, jint bufferIndex, jint attributeType, jint byteOffset,
        jint byteStride) {
    VertexBuffer::Builder* builder = (VertexBuffer::Builder *) nativeBuilder;
    builder->attribute((VertexAttribute) attribute, (uint8_t) bufferIndex,
                       (VertexBuffer::AttributeType) attributeType,
                       (uint32_t) byteOffset, (uint8_t) byteStride);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_VertexBuffer_nBuilderNormalized(JNIEnv*, jclass,
        jlong nativeBuilder, jint attribute, jboolean normalized) {
    VertexBuffer::Builder* builder = (VertexBuffer::Builder *) nativeBuilder;
    builder->normalized((VertexAttribute) attribute, normalized);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_VertexBuffer_nBuilderBuild(JNIEnv*, jclass,
        jlong nativeBuilder, jlong nativeEngine) {
    VertexBuffer::Builder* builder = (VertexBuffer::Builder *) nativeBuilder;
    Engine *engine = (Engine *) nativeEngine;
    return (jlong) builder->build(*engine);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_VertexBuffer_nGetVertexCount(JNIEnv*, jclass,
        jlong nativeVertexBuffer) {
    VertexBuffer *vertexBuffer = (VertexBuffer *) nativeVertexBuffer;
    return (jint) vertexBuffer->getVertexCount();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_VertexBuffer_nSetBufferAt(JNIEnv *env, jclass,
        jlong nativeVertexBuffer, jlong nativeEngine, jint bufferIndex,
        jobject buffer, jint remaining,
        jint destOffsetInBytes, jint count, jobject handler, jobject runnable) {
    VertexBuffer *vertexBuffer = (VertexBuffer *) nativeVertexBuffer;
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

    vertexBuffer->setBufferAt(*engine, (uint8_t) bufferIndex, std::move(desc),
            (uint32_t) destOffsetInBytes);

    return 0;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_VertexBuffer_nSetBufferObjectAt(JNIEnv*, jclass,
        jlong nativeVertexBuffer, jlong nativeEngine, jint bufferIndex, jlong nativeBufferObject) {
    VertexBuffer *vertexBuffer = (VertexBuffer *) nativeVertexBuffer;
    Engine *engine = (Engine *) nativeEngine;
    BufferObject *bufferObject = (BufferObject *) nativeBufferObject;
    vertexBuffer->setBufferObjectAt(*engine, (uint8_t) bufferIndex, bufferObject);
}
