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

#include <filament/IndirectLight.h>
#include <filament/Texture.h>
#include <common/NioUtils.h>
#include <common/CallbackUtils.h>
#include <math/mat4.h>
#include "../../../../common/JniExceptionBridge.h"

using namespace filament;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_IndirectLight_nCreateBuilder(JNIEnv* env, jclass) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_IndirectLight_nCreateBuilder", 0, [&]() -> jlong {
            return (jlong) new IndirectLight::Builder();
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_IndirectLight_nDestroyBuilder(JNIEnv* env, jclass,
        jlong nativeBuilder) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_IndirectLight_nDestroyBuilder", [&]() {
            IndirectLight::Builder* builder = (IndirectLight::Builder*) nativeBuilder;
            delete builder;
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_IndirectLight_nBuilderBuild(JNIEnv* env, jclass,
        jlong nativeBuilder, jlong nativeEngine) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_IndirectLight_nBuilderBuild", 0, [&]() -> jlong {
            IndirectLight::Builder* builder = (IndirectLight::Builder*) nativeBuilder;
            Engine *engine = (Engine *) nativeEngine;
            return (jlong) builder->build(*engine);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_IndirectLight_nBuilderReflections(JNIEnv* env, jclass,
        jlong nativeBuilder, jlong nativeTexture) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_IndirectLight_nBuilderReflections", [&]() {
            IndirectLight::Builder* builder = (IndirectLight::Builder*) nativeBuilder;
            const Texture *texture = (const Texture *) nativeTexture;
            builder->reflections(texture);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_IndirectLight_nIrradiance(JNIEnv* env, jclass,
        jlong nativeBuilder, jint bands, jfloatArray sh_) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_IndirectLight_nIrradiance", [&]() {
            IndirectLight::Builder* builder = (IndirectLight::Builder*) nativeBuilder;
            jfloat* sh = env->GetFloatArrayElements(sh_, NULL);
            builder->irradiance((uint8_t) bands, (const filament::math::float3*) sh);
            env->ReleaseFloatArrayElements(sh_, sh, JNI_ABORT);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_IndirectLight_nRadiance(JNIEnv* env, jclass,
        jlong nativeBuilder, jint bands, jfloatArray sh_) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_IndirectLight_nRadiance", [&]() {
            IndirectLight::Builder* builder = (IndirectLight::Builder*) nativeBuilder;
            jfloat* sh = env->GetFloatArrayElements(sh_, NULL);
            builder->radiance((uint8_t) bands, (const filament::math::float3*) sh);
            env->ReleaseFloatArrayElements(sh_, sh, JNI_ABORT);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_IndirectLight_nIrradianceAsTexture(JNIEnv* env, jclass,
        jlong nativeBuilder, jlong nativeTexture) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_IndirectLight_nIrradianceAsTexture", [&]() {
            IndirectLight::Builder* builder = (IndirectLight::Builder*) nativeBuilder;
            const Texture* texture = (const Texture*) nativeTexture;
            builder->irradiance(texture);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_IndirectLight_nIntensity(JNIEnv* env, jclass,
        jlong nativeBuilder, jfloat envIntensity) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_IndirectLight_nIntensity", [&]() {
            IndirectLight::Builder* builder = (IndirectLight::Builder*) nativeBuilder;
            builder->intensity(envIntensity);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_IndirectLight_nRotation(JNIEnv * env, jclass, jlong nativeBuilder,
        jfloat v0, jfloat v1, jfloat v2, jfloat v3, jfloat v4, jfloat v5, jfloat v6, jfloat v7,
        jfloat v8) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_IndirectLight_nRotation", [&]() {
            IndirectLight::Builder *builder = (IndirectLight::Builder *) nativeBuilder;
            builder->rotation(filament::math::mat3f{v0, v1, v2, v3, v4, v5, v6, v7, v8});
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_IndirectLight_nSetIntensity(JNIEnv* env, jclass,
        jlong nativeIndirectLight, jfloat intensity) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_IndirectLight_nSetIntensity", [&]() {
            IndirectLight* indirectLight = (IndirectLight*) nativeIndirectLight;
            indirectLight->setIntensity(intensity);
    });
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_IndirectLight_nGetIntensity(JNIEnv* env, jclass,
        jlong nativeIndirectLight) {
    return filament::android::jniGuard<jfloat>(env, "Java_com_google_android_filament_IndirectLight_nGetIntensity", 0.0f, [&]() -> jfloat {
            IndirectLight* indirectLight = (IndirectLight*) nativeIndirectLight;
            return indirectLight->getIntensity();
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_IndirectLight_nSetRotation(JNIEnv* env, jclass,
        jlong nativeIndirectLight, jfloat v0, jfloat v1, jfloat v2,
        jfloat v3, jfloat v4, jfloat v5, jfloat v6, jfloat v7, jfloat v8) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_IndirectLight_nSetRotation", [&]() {
            IndirectLight *indirectLight = (IndirectLight *) nativeIndirectLight;
            indirectLight->setRotation(filament::math::mat3f{v0, v1, v2, v3, v4, v5, v6, v7, v8});
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_IndirectLight_nGetRotation(JNIEnv* env, jclass,
        jlong nativeIndirectLight, jfloatArray outRotation_) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_IndirectLight_nGetRotation", [&]() {
            IndirectLight *indirectLight = (IndirectLight *) nativeIndirectLight;
            jfloat *outRotation = env->GetFloatArrayElements(outRotation_, NULL);
            *reinterpret_cast<filament::math::mat3f*>(outRotation) = indirectLight->getRotation();
            env->ReleaseFloatArrayElements(outRotation_, outRotation, 0);

    });
}

extern "C" [[deprecated]] JNIEXPORT void JNICALL
Java_com_google_android_filament_IndirectLight_nGetDirectionEstimate(JNIEnv* env, jclass,
        jlong nativeIndirectLight, jfloatArray outDirection_) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_IndirectLight_nGetDirectionEstimate", [&]() {
            IndirectLight *indirectLight = (IndirectLight *) nativeIndirectLight;
            jfloat *outDirection = env->GetFloatArrayElements(outDirection_, NULL);
            *reinterpret_cast<filament::math::float3*>(outDirection) = indirectLight->getDirectionEstimate();
            env->ReleaseFloatArrayElements(outDirection_, outDirection, 0);
    });
}

extern "C" [[deprecated]] JNIEXPORT void JNICALL
Java_com_google_android_filament_IndirectLight_nGetColorEstimate(JNIEnv* env, jclass,
        jlong nativeIndirectLight, jfloatArray outColor_, jfloat x, jfloat y, jfloat z) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_IndirectLight_nGetColorEstimate", [&]() {
            IndirectLight *indirectLight = (IndirectLight *) nativeIndirectLight;
            jfloat *outColor = env->GetFloatArrayElements(outColor_, NULL);
            *reinterpret_cast<filament::math::float4*>(outColor) =
                    indirectLight->getColorEstimate(math::float3{x, y, z});
            env->ReleaseFloatArrayElements(outColor_, outColor, 0);
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_IndirectLight_nGetReflectionsTexture(JNIEnv* env, jclass,
        jlong nativeIndirectLight) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_IndirectLight_nGetReflectionsTexture", 0, [&]() -> jlong {
            IndirectLight *indirectLight = (IndirectLight *) nativeIndirectLight;
            Texture const *tex = indirectLight->getReflectionsTexture();
            return (jlong) tex;
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_IndirectLight_nGetIrradianceTexture(JNIEnv* env, jclass,
        jlong nativeIndirectLight) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_IndirectLight_nGetIrradianceTexture", 0, [&]() -> jlong {
            IndirectLight *indirectLight = (IndirectLight *) nativeIndirectLight;
            Texture const *tex = indirectLight->getIrradianceTexture();
            return (jlong) tex;
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_IndirectLight_nGetDirectionEstimateStatic(JNIEnv *env, jclass,
        jfloatArray sh_, jfloatArray outDirection_) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_IndirectLight_nGetDirectionEstimateStatic", [&]() {
            jfloat* sh = env->GetFloatArrayElements(sh_, NULL);
            jfloat *outDirection = env->GetFloatArrayElements(outDirection_, NULL);
            *reinterpret_cast<filament::math::float3*>(outDirection) = IndirectLight::getDirectionEstimate((filament::math::float3*)sh);
            env->ReleaseFloatArrayElements(outDirection_, outDirection, 0);
            env->ReleaseFloatArrayElements(sh_, sh, 0);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_IndirectLight_nGetColorEstimateStatic(JNIEnv *env, jclass,
        jfloatArray outColor_, jfloatArray sh_, jfloat x, jfloat y, jfloat z) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_IndirectLight_nGetColorEstimateStatic", [&]() {
            jfloat* sh = env->GetFloatArrayElements(sh_, NULL);
            jfloat *outColor = env->GetFloatArrayElements(outColor_, NULL);
            *reinterpret_cast<filament::math::float4*>(outColor) =
                    IndirectLight::getColorEstimate((filament::math::float3*)sh, math::float3{x, y, z});
            env->ReleaseFloatArrayElements(outColor_, outColor, 0);
            env->ReleaseFloatArrayElements(sh_, sh, 0);
    });
}
