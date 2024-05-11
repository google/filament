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

#include <filament/SkinningBuffer.h>

#include "common/CallbackUtils.h"
#include "common/NioUtils.h"

using namespace filament;
using namespace backend;

extern "C"
JNIEXPORT jlong JNICALL
Java_com_google_android_filament_SkinningBuffer_nCreateBuilder(JNIEnv*, jclass) {
    return (jlong) new SkinningBuffer::Builder();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_SkinningBuffer_nDestroyBuilder(JNIEnv*, jclass,
        jlong nativeBuilder) {
    SkinningBuffer::Builder* builder = (SkinningBuffer::Builder *) nativeBuilder;
    delete builder;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_SkinningBuffer_nBuilderBoneCount(JNIEnv*, jclass,
        jlong nativeBuilder, jint boneCount) {
    SkinningBuffer::Builder* builder = (SkinningBuffer::Builder *) nativeBuilder;
    builder->boneCount((uint32_t)boneCount);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_SkinningBuffer_nBuilderInitialize(JNIEnv*, jclass,
        jlong nativeBuilder, jboolean initialize) {
    SkinningBuffer::Builder* builder = (SkinningBuffer::Builder *) nativeBuilder;
    builder->initialize((bool)initialize);
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_google_android_filament_SkinningBuffer_nBuilderBuild(JNIEnv*, jclass,
        jlong nativeBuilder, jlong nativeEngine) {
    SkinningBuffer::Builder* builder = (SkinningBuffer::Builder *) nativeBuilder;
    Engine *engine = (Engine *) nativeEngine;
    return (jlong) builder->build(*engine);
}

// ------------------------------------------------------------------------------------------------

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_SkinningBuffer_nSetBonesAsMatrices(JNIEnv* env, jclass,
        jlong nativeSkinningBuffer, jlong nativeEngine, jobject matrices, jint remaining, jint boneCount,
        jint offset) {
    SkinningBuffer *skinningBuffer = (SkinningBuffer *) nativeSkinningBuffer;
    Engine *engine = (Engine *) nativeEngine;
    AutoBuffer nioBuffer(env, matrices, boneCount * 16);
    void* data = nioBuffer.getData();
    size_t sizeInBytes = nioBuffer.getSize();
    if (sizeInBytes > (remaining << nioBuffer.getShift())) {
        // BufferOverflowException
        return -1;
    }
    skinningBuffer->setBones(*engine,
            static_cast<filament::math::mat4f const *>(data), (size_t)boneCount, (size_t)offset);
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_SkinningBuffer_nSetBonesAsQuaternions(JNIEnv* env, jclass,
        jlong nativeSkinningBuffer, jlong nativeEngine, jobject quaternions, jint remaining,
        jint boneCount, jint offset) {
    SkinningBuffer *skinningBuffer = (SkinningBuffer *) nativeSkinningBuffer;
    Engine *engine = (Engine *) nativeEngine;
    AutoBuffer nioBuffer(env, quaternions, boneCount * 8);
    void* data = nioBuffer.getData();
    size_t sizeInBytes = nioBuffer.getSize();
    if (sizeInBytes > (remaining << nioBuffer.getShift())) {
        // BufferOverflowException
        return -1;
    }
    skinningBuffer->setBones(*engine,
            static_cast<RenderableManager::Bone const *>(data), (size_t)boneCount, (size_t)offset);
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_SkinningBuffer_nGetBoneCount(JNIEnv*, jclass,
        jlong nativeSkinningBuffer) {
    SkinningBuffer *skinningBuffer = (SkinningBuffer *) nativeSkinningBuffer;
    return (jint)skinningBuffer->getBoneCount();
}
