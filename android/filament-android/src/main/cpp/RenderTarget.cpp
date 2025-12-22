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
#include "../../../../common/JniExceptionBridge.h"

using namespace filament;
using namespace backend;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_RenderTarget_nCreateBuilder(JNIEnv *env, jclass type) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_RenderTarget_nCreateBuilder", 0, [&]() -> jlong {
            return (jlong) new RenderTarget::Builder();
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderTarget_nDestroyBuilder(JNIEnv *env, jclass type,
        jlong nativeBuilder) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderTarget_nDestroyBuilder", [&]() {
            RenderTarget::Builder* builder = (RenderTarget::Builder*) nativeBuilder;
            delete builder;
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderTarget_nBuilderTexture(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint attachment, jlong nativeTexture) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderTarget_nBuilderTexture", [&]() {
            RenderTarget::Builder* builder = (RenderTarget::Builder*) nativeBuilder;
            Texture* texture = (Texture*) nativeTexture;
            builder->texture(RenderTarget::AttachmentPoint(attachment), texture);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderTarget_nBuilderMipLevel(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint attachment, jint level) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderTarget_nBuilderMipLevel", [&]() {
            RenderTarget::Builder* builder = (RenderTarget::Builder*) nativeBuilder;
            builder->mipLevel(RenderTarget::AttachmentPoint(attachment), level);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderTarget_nBuilderFace(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint attachment, jint face) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderTarget_nBuilderFace", [&]() {
            RenderTarget::Builder* builder = (RenderTarget::Builder*) nativeBuilder;
            RenderTarget::CubemapFace cubeface = (RenderTarget::CubemapFace) face;
            builder->face(RenderTarget::AttachmentPoint(attachment), cubeface);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_RenderTarget_nBuilderLayer(JNIEnv *env, jclass type,
        jlong nativeBuilder, jint attachment, jint layer) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_RenderTarget_nBuilderLayer", [&]() {
            RenderTarget::Builder* builder = (RenderTarget::Builder*) nativeBuilder;
            builder->layer(RenderTarget::AttachmentPoint(attachment), layer);
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_RenderTarget_nBuilderBuild(JNIEnv *env, jclass type,
        jlong nativeBuilder, jlong nativeEngine) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_RenderTarget_nBuilderBuild", 0, [&]() -> jlong {
            RenderTarget::Builder* builder = (RenderTarget::Builder*) nativeBuilder;
            Engine *engine = (Engine *) nativeEngine;
            return (jlong) builder->build(*engine);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderTarget_nGetMipLevel(JNIEnv *env, jclass type,
        jlong nativeTarget, jint attachment) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_RenderTarget_nGetMipLevel", 0, [&]() -> jint {
            RenderTarget* target = (RenderTarget*) nativeTarget;
            return (jint) target->getMipLevel(RenderTarget::AttachmentPoint(attachment));
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderTarget_nGetFace(JNIEnv *env, jclass type,
        long nativeTarget, int attachment) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_RenderTarget_nGetFace", 0, [&]() -> jint {
            RenderTarget* target = (RenderTarget*) nativeTarget;
            return (jint) target->getFace(RenderTarget::AttachmentPoint(attachment));
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_RenderTarget_nGetLayer(JNIEnv *env, jclass type,
        long nativeTarget, int attachment) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_RenderTarget_nGetLayer", 0, [&]() -> jint {
            RenderTarget* target = (RenderTarget*) nativeTarget;
            return (jint) target->getLayer(RenderTarget::AttachmentPoint(attachment));
    });
}
