/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <filament/RenderTarget.h>

using namespace filament;
using namespace backend;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_RenderTarget_nCreateBuilder(JNIEnv *env, jclass type) {
    return (jlong) new RenderTarget::Builder();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderTarget_nDestroyBuilder(JNIEnv *env, jclass type,
        jlong nativeBuilder) {
    RenderTarget::Builder* builder = (RenderTarget::Builder*) nativeBuilder;
    delete builder;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderTarget_nBuilderTexture(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint attachment, jlong nativeTexture) {
    RenderTarget::Builder* builder = (RenderTarget::Builder*) nativeBuilder;
    Texture* texture = (Texture*) nativeTexture;
    builder->texture(RenderTarget::AttachmentPoint(attachment), texture);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderTarget_nBuilderMipLevel(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint attachment, jint level) {
    RenderTarget::Builder* builder = (RenderTarget::Builder*) nativeBuilder;
    builder->mipLevel(RenderTarget::AttachmentPoint(attachment), level);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderTarget_nBuilderFace(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint attachment, jint face) {
    RenderTarget::Builder* builder = (RenderTarget::Builder*) nativeBuilder;
    RenderTarget::CubemapFace cubeface = (RenderTarget::CubemapFace) face;
    builder->face(RenderTarget::AttachmentPoint(attachment), cubeface);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderTarget_nBuilderLayer(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint attachment, jint layer) {
    RenderTarget::Builder* builder = (RenderTarget::Builder*) nativeBuilder;
    builder->layer(RenderTarget::AttachmentPoint(attachment), layer);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_RenderTarget_nBuilderBuild(JNIEnv *env, jclass type,
        jlong nativeBuilder, jlong nativeEngine) {
    RenderTarget::Builder* builder = (RenderTarget::Builder*) nativeBuilder;
    Engine *engine = (Engine *) nativeEngine;
    return (jlong) builder->build(*engine);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderTarget_nGetMipLevel(JNIEnv *env, jclass type,
        jlong nativeTarget, jint attachment) {
    RenderTarget* target = (RenderTarget*) nativeTarget;
    return (jint) target->getMipLevel(RenderTarget::AttachmentPoint(attachment));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderTarget_nGetFace(JNIEnv *env, jclass type,
        long nativeTarget, int attachment) {
    RenderTarget* target = (RenderTarget*) nativeTarget;
    return (jint) target->getFace(RenderTarget::AttachmentPoint(attachment));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderTarget_nGetLayer(JNIEnv *env, jclass type,
        long nativeTarget, int attachment) {
    RenderTarget* target = (RenderTarget*) nativeTarget;
    return (jint) target->getLayer(RenderTarget::AttachmentPoint(attachment));
}
