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

#include <filament/MorphTargetBuffer.h>

#include "common/CallbackUtils.h"
#include "common/NioUtils.h"

using namespace filament;
using namespace backend;

extern "C"
JNIEXPORT jlong JNICALL
Java_com_google_android_filament_MorphTargetBuffer_nCreateBuilder(JNIEnv*, jclass) {
    return (jlong) new MorphTargetBuffer::Builder();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MorphTargetBuffer_nDestroyBuilder(JNIEnv*, jclass,
        jlong nativeBuilder) {
    MorphTargetBuffer::Builder* builder = (MorphTargetBuffer::Builder *) nativeBuilder;
    delete builder;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MorphTargetBuffer_nBuilderVertexCount(JNIEnv*, jclass,
        jlong nativeBuilder, jint vertexCount) {
    MorphTargetBuffer::Builder* builder = (MorphTargetBuffer::Builder *) nativeBuilder;
    builder->vertexCount((size_t) vertexCount);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_MorphTargetBuffer_nBuilderCount(JNIEnv*, jclass,
        jlong nativeBuilder, jint count) {
    MorphTargetBuffer::Builder* builder = (MorphTargetBuffer::Builder *) nativeBuilder;
    builder->count((size_t) count);
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_google_android_filament_MorphTargetBuffer_nBuilderBuild(JNIEnv*, jclass,
        jlong nativeBuilder, jlong nativeEngine) {
    MorphTargetBuffer::Builder* builder = (MorphTargetBuffer::Builder *) nativeBuilder;
    Engine *engine = (Engine *) nativeEngine;
    return (jlong) builder->build(*engine);
}

// ------------------------------------------------------------------------------------------------

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_MorphTargetBuffer_nSetPositionsAt(JNIEnv* env, jclass,
        jlong nativeObject, jlong nativeEngine,
        jint targetIndex, jfloatArray positions, jint count) {
    MorphTargetBuffer *morphTargetBuffer = (MorphTargetBuffer *) nativeObject;
    Engine *engine = (Engine *) nativeEngine;
    jfloat* data = env->GetFloatArrayElements(positions, NULL);
    morphTargetBuffer->setPositionsAt(*engine, targetIndex,
            (math::float4*) data, size_t(count));
    env->ReleaseFloatArrayElements(positions, data, JNI_ABORT);
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_MorphTargetBuffer_nSetTangentsAt(JNIEnv* env, jclass,
        jlong nativeObject, jlong nativeEngine,
        jint targetIndex, jshortArray tangents, jint count) {
    MorphTargetBuffer *morphTargetBuffer = (MorphTargetBuffer *) nativeObject;
    Engine *engine = (Engine *) nativeEngine;
    jshort* data = env->GetShortArrayElements(tangents, NULL);
    morphTargetBuffer->setTangentsAt(*engine, targetIndex,
            (math::short4*) data, size_t(count));
    env->ReleaseShortArrayElements(tangents, data, JNI_ABORT);
    return 0;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_MorphTargetBuffer_nGetVertexCount(JNIEnv*, jclass,
        jlong nativeObject) {
    MorphTargetBuffer *morphTargetBuffer = (MorphTargetBuffer *) nativeObject;
    return (jint)morphTargetBuffer->getVertexCount();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_MorphTargetBuffer_nGetCount(JNIEnv*, jclass,
        jlong nativeObject) {
    MorphTargetBuffer *morphTargetBuffer = (MorphTargetBuffer *) nativeObject;
    return (jint)morphTargetBuffer->getCount();
}
