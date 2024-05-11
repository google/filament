/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <filament/BufferObject.h>

#include <backend/BufferDescriptor.h>

#include "common/CallbackUtils.h"
#include "common/NioUtils.h"

using namespace filament;
using namespace backend;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_BufferObject_nCreateBuilder(JNIEnv *env, jclass type) {
    return (jlong) new BufferObject::Builder();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_BufferObject_nDestroyBuilder(JNIEnv *env, jclass type,
        jlong nativeBuilder) {
    BufferObject::Builder* builder = (BufferObject::Builder *) nativeBuilder;
    delete builder;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_BufferObject_nBuilderSize(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint byteCount) {
    BufferObject::Builder* builder = (BufferObject::Builder *) nativeBuilder;
    builder->size((uint32_t) byteCount);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_BufferObject_nBuilderBindingType(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint bindingType) {
    using BindingType = BufferObject::BindingType;
    BufferObject::Builder* builder = (BufferObject::Builder *) nativeBuilder;
    BindingType types[] = {BindingType::VERTEX};
    builder->bindingType(types[bindingType]);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_BufferObject_nBuilderBuild(JNIEnv *env, jclass type,
        jlong nativeBuilder, jlong nativeEngine) {
    BufferObject::Builder* builder = (BufferObject::Builder *) nativeBuilder;
    Engine *engine = (Engine *) nativeEngine;
    return (jlong) builder->build(*engine);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_BufferObject_nGetByteCount(JNIEnv *env, jclass type,
        jlong nativeBufferObject) {
    BufferObject *bufferObject = (BufferObject *) nativeBufferObject;
    return (jint) bufferObject->getByteCount();
}

extern "C" JNIEXPORT int JNICALL
Java_com_google_android_filament_BufferObject_nSetBuffer(JNIEnv *env, jclass type,
        jlong nativeBufferObject, jlong nativeEngine, jobject buffer, int remaining,
        jint destOffsetInBytes, jint count,
        jobject handler, jobject runnable) {
    BufferObject *bufferObject = (BufferObject *) nativeBufferObject;
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

    bufferObject->setBuffer(*engine, std::move(desc), (uint32_t) destOffsetInBytes);

    return 0;
}
