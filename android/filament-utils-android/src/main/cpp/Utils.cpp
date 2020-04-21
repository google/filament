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

#include <filament/Engine.h>
#include <filament/IndirectLight.h>
#include <filament/Skybox.h>

#include <image/KtxUtility.h>

#include "common/NioUtils.h"

using namespace filament;
using namespace filament::math;
using namespace image;

static jlong nCreateTexture(JNIEnv* env, jclass,
        jlong nativeEngine, jobject javaBuffer, jint remaining, jboolean srgb) {
    Engine* engine = (Engine*) nativeEngine;
    AutoBuffer buffer(env, javaBuffer, remaining);
    KtxBundle* bundle = new KtxBundle((const uint8_t*) buffer.getData(), buffer.getSize());
    return (jlong) ktx::createTexture(engine, *bundle, srgb, [](void* userdata) {
        KtxBundle* bundle = (KtxBundle*) userdata;
        delete bundle;
    }, bundle);
}

static jlong nCreateIndirectLight(JNIEnv* env, jclass,
        jlong nativeEngine, jobject javaBuffer, jint remaining, jboolean srgb) {
    Engine* engine = (Engine*) nativeEngine;
    AutoBuffer buffer(env, javaBuffer, remaining);
    KtxBundle* bundle = new KtxBundle((const uint8_t*) buffer.getData(), buffer.getSize());
    Texture* cubemap = ktx::createTexture(engine, *bundle, srgb,  [](void* userdata) {
        KtxBundle* bundle = (KtxBundle*) userdata;
        delete bundle;
    }, bundle);

    float3 harmonics[9];
    bundle->getSphericalHarmonics(harmonics);

    IndirectLight* indirectLight = IndirectLight::Builder()
        .reflections(cubemap)
        .irradiance(3, harmonics)
        .intensity(30000)
        .build(*engine);

    return (jlong) indirectLight;
}

static jlong nCreateSkybox(JNIEnv* env, jclass,
        jlong nativeEngine, jobject javaBuffer, jint remaining, jboolean srgb) {
    Engine* engine = (Engine*) nativeEngine;
    AutoBuffer buffer(env, javaBuffer, remaining);
    KtxBundle* bundle = new KtxBundle((const uint8_t*) buffer.getData(), buffer.getSize());
    Texture* cubemap = ktx::createTexture(engine, *bundle, srgb,  [](void* userdata) {
        KtxBundle* bundle = (KtxBundle*) userdata;
        delete bundle;
    }, bundle);
    return (jlong) Skybox::Builder().environment(cubemap).showSun(true).build(*engine);
}


JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void*) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    jclass c = env->FindClass("com/google/android/filament/utils/KtxLoader");
    if (c == nullptr) return JNI_ERR;

    static const JNINativeMethod methods[] = {
        {"nCreateTexture", "(JLjava/nio/Buffer;IZ)J", reinterpret_cast<void*>(nCreateTexture)},
        {"nCreateIndirectLight", "(JLjava/nio/Buffer;IZ)J", reinterpret_cast<void*>(nCreateIndirectLight)},
        {"nCreateSkybox", "(JLjava/nio/Buffer;IZ)J", reinterpret_cast<void*>(nCreateSkybox)},
    };
    int rc = env->RegisterNatives(c, methods, sizeof(methods) / sizeof(JNINativeMethod));
    if (rc != JNI_OK) return rc;

    return JNI_VERSION_1_6;
}
