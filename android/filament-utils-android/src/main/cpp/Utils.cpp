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

jlong nCreateHDRTexture(JNIEnv* env, jclass,
        jlong nativeEngine, jobject javaBuffer, jint remaining, jint internalFormat);

static jlong nCreateKTXTexture(JNIEnv* env, jclass,
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

static jboolean nGetSphericalHarmonics(JNIEnv* env, jclass, jobject javaBuffer, jint remaining,
        jfloatArray outSphericalHarmonics_) {
    AutoBuffer buffer(env, javaBuffer, remaining);
    KtxBundle bundle((const uint8_t*) buffer.getData(), buffer.getSize());

    jfloat* outSphericalHarmonics = env->GetFloatArrayElements(outSphericalHarmonics_, nullptr);
    const auto success = bundle.getSphericalHarmonics(
        reinterpret_cast<filament::math::float3*>(outSphericalHarmonics)
    );
    env->ReleaseFloatArrayElements(outSphericalHarmonics_, outSphericalHarmonics, JNI_ABORT);

    return success ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void*) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    int rc;

    // KTXLoader
    jclass ktxloaderClass = env->FindClass("com/google/android/filament/utils/KTXLoader");
    if (ktxloaderClass == nullptr) return JNI_ERR;
    static const JNINativeMethod ktxMethods[] = {
        {strdup("nCreateKTXTexture"), strdup("(JLjava/nio/Buffer;IZ)J"), reinterpret_cast<void*>(nCreateKTXTexture)},
        {strdup("nCreateIndirectLight"), strdup("(JLjava/nio/Buffer;IZ)J"), reinterpret_cast<void*>(nCreateIndirectLight)},
        {strdup("nCreateSkybox"), strdup("(JLjava/nio/Buffer;IZ)J"), reinterpret_cast<void*>(nCreateSkybox)},
        {strdup("nGetSphericalHarmonics"), strdup("(Ljava/nio/Buffer;I[F)Z"), reinterpret_cast<void*>(nGetSphericalHarmonics)},
    };
    rc = env->RegisterNatives(ktxloaderClass, ktxMethods, sizeof(ktxMethods) / sizeof(JNINativeMethod));
    free(ktxMethods[0].name); free(ktxMethods[0].signature);
    free(ktxMethods[1].name); free(ktxMethods[1].signature);
    free(ktxMethods[2].name); free(ktxMethods[2].signature);
    free(ktxMethods[3].name); free(ktxMethods[3].signature);
    if (rc != JNI_OK) return rc;

    // HDRLoader
    jclass hdrloaderClass = env->FindClass("com/google/android/filament/utils/HDRLoader");
    if (hdrloaderClass == nullptr) return JNI_ERR;
    static const JNINativeMethod hdrMethods[] = {
        {strdup("nCreateHDRTexture"), strdup("(JLjava/nio/Buffer;II)J"), reinterpret_cast<void*>(nCreateHDRTexture)},
    };
    free(hdrMethods[0].name); free(hdrMethods[0].signature);
    rc = env->RegisterNatives(hdrloaderClass, hdrMethods, sizeof(hdrMethods) / sizeof(JNINativeMethod));
    if (rc != JNI_OK) return rc;

    return JNI_VERSION_1_6;
}
