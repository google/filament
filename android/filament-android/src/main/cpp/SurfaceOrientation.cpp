/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include <geometry/SurfaceOrientation.h>

#include "common/NioUtils.h"

#include <algorithm>

using namespace filament;
using namespace filament::geometry;
using namespace filament::math;

namespace {
    struct JniWrapper {
        SurfaceOrientation::Builder* builder;
        AutoBuffer* normals;
        AutoBuffer* tangents;
        AutoBuffer* uvs;
        AutoBuffer* positions;
        AutoBuffer* triangles16;
        AutoBuffer* triangles32;
    };
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_SurfaceOrientation_nCreateBuilder(JNIEnv* env, jclass) {
    JniWrapper* wrapper = new JniWrapper();
    wrapper->builder = new SurfaceOrientation::Builder();
    return (jlong) wrapper;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_SurfaceOrientation_nDestroyBuilder(JNIEnv* env, jclass,
        jlong nativeBuilder) {
    auto wrapper = (JniWrapper*) nativeBuilder;
    delete wrapper->builder;
    delete wrapper->normals;
    delete wrapper->tangents;
    delete wrapper->uvs;
    delete wrapper->positions;
    delete wrapper->triangles16;
    delete wrapper->triangles32;
    delete wrapper;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_SurfaceOrientation_nBuilderVertexCount(JNIEnv* env, jclass,
        jlong nativeBuilder, int vertexCount) {
    auto wrapper = (JniWrapper *) nativeBuilder;
    wrapper->builder->vertexCount(vertexCount);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_SurfaceOrientation_mBuilderTriangleCount(JNIEnv* env, jclass,
        jlong nativeBuilder, int triangleCount) {
    auto wrapper = (JniWrapper *) nativeBuilder;
    wrapper->builder->triangleCount(triangleCount);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_SurfaceOrientation_mBuilderNormals(JNIEnv* env, jclass,
        jlong nativeBuilder, jobject javaBuffer, jint remaining, int stride) {
    auto wrapper = (JniWrapper *) nativeBuilder;
    AutoBuffer* buffer = wrapper->normals = new AutoBuffer(env, javaBuffer, remaining);
    wrapper->builder->normals((const float3 *) buffer->getData(), stride);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_SurfaceOrientation_mBuilderTangents(JNIEnv* env, jclass,
        jlong nativeBuilder, jobject javaBuffer, jint remaining, int stride) {
    auto wrapper = (JniWrapper *) nativeBuilder;
    AutoBuffer* buffer = wrapper->tangents = new AutoBuffer(env, javaBuffer, remaining);
    wrapper->builder->tangents((const float4 *) buffer->getData(), stride);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_SurfaceOrientation_mBuilderUVs(JNIEnv* env, jclass,
        jlong nativeBuilder, jobject javaBuffer, int remaining, int stride) {
    auto wrapper = (JniWrapper *) nativeBuilder;
    AutoBuffer* buffer = wrapper->uvs = new AutoBuffer(env, javaBuffer, remaining);
    wrapper->builder->uvs((const float2 *) buffer->getData(), stride);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_SurfaceOrientation_mBuilderPositions(JNIEnv* env, jclass,
        jlong nativeBuilder, jobject javaBuffer, int remaining, int stride) {
    auto wrapper = (JniWrapper *) nativeBuilder;
    AutoBuffer* buffer = wrapper->positions = new AutoBuffer(env, javaBuffer, remaining);
    wrapper->builder->positions((const float3 *) buffer->getData(), stride);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_SurfaceOrientation_mBuilderTriangles16(JNIEnv* env, jclass,
        jlong nativeBuilder, jobject javaBuffer, int remaining) {
    auto wrapper = (JniWrapper *) nativeBuilder;
    AutoBuffer* buffer = wrapper->triangles16 = new AutoBuffer(env, javaBuffer, remaining);
    wrapper->builder->triangles((const ushort3 *) buffer->getData());
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_SurfaceOrientation_mBuilderTriangles32(JNIEnv* env, jclass,
        jlong nativeBuilder, jobject javaBuffer, int remaining) {
    auto wrapper = (JniWrapper *) nativeBuilder;
    AutoBuffer* buffer = wrapper->triangles32 = new AutoBuffer(env, javaBuffer, remaining);
    wrapper->builder->triangles((const uint3 *) buffer->getData());
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_SurfaceOrientation_nBuilderBuild(JNIEnv* env, jclass,
        jlong nativeBuilder) {
    auto wrapper = (JniWrapper *) nativeBuilder;
    return (jlong) wrapper->builder->build();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_SurfaceOrientation_nGetVertexCount(JNIEnv* env, jclass,
        jlong nativeObject) {
    SurfaceOrientation* helper = (SurfaceOrientation*) nativeObject;
    return helper->getVertexCount();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_SurfaceOrientation_nGetQuatsAsFloat(JNIEnv* env, jclass,
        jlong nativeObject, jobject javaBuffer, int remaining) {
    SurfaceOrientation* helper = (SurfaceOrientation*) nativeObject;
    AutoBuffer buffer(env, javaBuffer, remaining);
    size_t requestedCount = std::min(buffer.getSize() / sizeof(float4), helper->getVertexCount());
    helper->getQuats((quatf*) buffer.getData(), requestedCount);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_SurfaceOrientation_nGetQuatsAsHalf(JNIEnv* env, jclass,
        jlong nativeObject, jobject javaBuffer, int remaining) {
    SurfaceOrientation* helper = (SurfaceOrientation*) nativeObject;
    AutoBuffer buffer(env, javaBuffer, remaining);
    size_t requestedCount = std::min(buffer.getSize() / sizeof(quath), helper->getVertexCount());
    helper->getQuats((quath*) buffer.getData(), requestedCount);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_SurfaceOrientation_nGetQuatsAsShort(JNIEnv* env, jclass,
        jlong nativeObject, jobject javaBuffer, int remaining) {
    SurfaceOrientation* helper = (SurfaceOrientation*) nativeObject;
    AutoBuffer buffer(env, javaBuffer, remaining);
    size_t requestedCount = std::min(buffer.getSize() / sizeof(short4), helper->getVertexCount());
    helper->getQuats((short4*) buffer.getData(), requestedCount);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_SurfaceOrientation_nDestroy(JNIEnv* env, jclass,
        jlong nativeSurfaceOrientation) {
    SurfaceOrientation* helper = (SurfaceOrientation*) nativeSurfaceOrientation;
    delete helper;
}
