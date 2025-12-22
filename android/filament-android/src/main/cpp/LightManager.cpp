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

#include <filament/LightManager.h>

#include <utils/Entity.h>

#include <algorithm>
#include "../../../../common/JniExceptionBridge.h"

using namespace filament;
using namespace utils;

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_LightManager_nGetComponentCount(JNIEnv* env, jclass,
        jlong nativeLightManager) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_LightManager_nGetComponentCount", 0, [&]() -> jint {
            LightManager *lm = (LightManager *) nativeLightManager;
            return lm->getComponentCount();
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_LightManager_nHasComponent(JNIEnv* env, jclass,
        jlong nativeLightManager, jint entity) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_LightManager_nHasComponent", JNI_FALSE, [&]() -> jboolean {
            LightManager *lm = (LightManager *) nativeLightManager;
            return (jboolean) lm->hasComponent((Entity &) entity);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_LightManager_nGetInstance(JNIEnv* env, jclass,
        jlong nativeLightManager, jint entity) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_LightManager_nGetInstance", 0, [&]() -> jint {
            LightManager *lm = (LightManager *) nativeLightManager;
            return lm->getInstance((Entity &) entity);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nDestroy(JNIEnv* env, jclass,
        jlong nativeLightManager, jint entity) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nDestroy", [&]() {
            LightManager *lm = (LightManager *) nativeLightManager;
            lm->destroy((Entity &) entity);
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_LightManager_nCreateBuilder(JNIEnv* env, jclass, jint lightType) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_LightManager_nCreateBuilder", 0, [&]() -> jlong {
            return (jlong) new LightManager::Builder((LightManager::Type) lightType);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nDestroyBuilder(JNIEnv* env, jclass,
        jlong nativeBuilder) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nDestroyBuilder", [&]() {
            LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
            delete builder;
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderCastShadows(JNIEnv* env, jclass,
        jlong nativeBuilder, jboolean enable) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nBuilderCastShadows", [&]() {
            LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
            builder->castShadows(enable);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderShadowOptions(JNIEnv* env, jclass,
        jlong nativeBuilder, jint mapSize, jint cascades, jfloatArray splitPositions,
        jfloat constantBias, jfloat normalBias, jfloat shadowFar, jfloat shadowNearHint,
        jfloat shadowFarHint, jboolean stable, jboolean lispsm,
        jfloat polygonOffsetConstant, jfloat polygonOffsetSlope,
        jboolean screenSpaceContactShadows, jint stepCount,
        jfloat maxShadowDistance, jboolean elvsm, jfloat blurWidth, jfloat shadowBulbRadius,
        jfloatArray transform) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nBuilderShadowOptions", [&]() {
            LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
            LightManager::ShadowOptions shadowOptions {
                    .mapSize = (uint32_t)mapSize,
                    .shadowCascades = (uint8_t)cascades,
                    .constantBias = constantBias,
                    .normalBias = normalBias,
                    .shadowFar = shadowFar,
                    .shadowNearHint = shadowNearHint,
                    .shadowFarHint = shadowFarHint,
                    .stable = (bool)stable,
                    .lispsm = (bool)lispsm,
                    .polygonOffsetConstant = polygonOffsetConstant,
                    .polygonOffsetSlope = polygonOffsetConstant,
                    .screenSpaceContactShadows = (bool)screenSpaceContactShadows,
                    .stepCount = uint8_t(stepCount),
                    .maxShadowDistance = maxShadowDistance,
                    .vsm = {
                            .elvsm = (bool)elvsm,
                            .blurWidth = blurWidth
                    },
                    .shadowBulbRadius = shadowBulbRadius
            };

            jfloat *nativeSplits = env->GetFloatArrayElements(splitPositions, NULL);
            const jsize splitCount = std::min((jsize) 3, env->GetArrayLength(splitPositions));
            std::copy_n(nativeSplits, splitCount, shadowOptions.cascadeSplitPositions);
            env->ReleaseFloatArrayElements(splitPositions, nativeSplits, 0);

            jfloat* nativeTransform = env->GetFloatArrayElements(transform, NULL);
            std::copy_n(nativeTransform,
                        std::min(4, env->GetArrayLength(transform)),
                        shadowOptions.transform.xyzw.v);
            env->ReleaseFloatArrayElements(transform, nativeTransform, 0);

            builder->shadowOptions(shadowOptions);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderCastLight(JNIEnv* env, jclass,
        jlong nativeBuilder, jboolean enabled) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nBuilderCastLight", [&]() {
            LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
            builder->castLight(enabled);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderPosition(JNIEnv* env, jclass,
        jlong nativeBuilder, jfloat x, jfloat y, jfloat z) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nBuilderPosition", [&]() {
            LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
            builder->position({x, y, z});
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderDirection(JNIEnv* env, jclass,
        jlong nativeBuilder, jfloat x, jfloat y, jfloat z) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nBuilderDirection", [&]() {
            LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
            builder->direction({x, y, z});
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderColor(JNIEnv* env, jclass,
        jlong nativeBuilder, jfloat linearR, jfloat linearG, jfloat linearB) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nBuilderColor", [&]() {
            LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
            builder->color({linearR, linearG, linearB});
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderIntensityCandela(JNIEnv* env, jclass,
        jlong nativeBuilder, jfloat intensity) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nBuilderIntensityCandela", [&]() {
            LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
            builder->intensityCandela(intensity);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderIntensity__JF(JNIEnv* env, jclass,
        jlong nativeBuilder, jfloat intensity) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nBuilderIntensity__JF", [&]() {
            LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
            builder->intensity(intensity);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderIntensity__JFF(JNIEnv* env, jclass,
        jlong nativeBuilder, jfloat watts, jfloat efficiency) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nBuilderIntensity__JFF", [&]() {
            LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
            builder->intensity(watts, efficiency);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderFalloff(JNIEnv* env, jclass,
        jlong nativeBuilder, jfloat radius) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nBuilderFalloff", [&]() {
            LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
            builder->falloff(radius);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderSpotLightCone(JNIEnv* env, jclass,
        jlong nativeBuilder, jfloat inner, jfloat outer) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nBuilderSpotLightCone", [&]() {
            LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
            builder->spotLightCone(inner, outer);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderAngularRadius(JNIEnv* env, jclass,
        jlong nativeBuilder, jfloat angularRadius) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nBuilderAngularRadius", [&]() {
            LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
            builder->sunAngularRadius(angularRadius);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderHaloSize(JNIEnv* env, jclass,
        jlong nativeBuilder, jfloat haloSize) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nBuilderHaloSize", [&]() {
            LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
            builder->sunHaloSize(haloSize);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderHaloFalloff(JNIEnv* env, jclass,
        jlong nativeBuilder, jfloat haloFalloff) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nBuilderHaloFalloff", [&]() {
            LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
            builder->sunHaloFalloff(haloFalloff);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderLightChannel(JNIEnv* env, jclass,
        jlong nativeBuilder, jint channel, jboolean enable) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nBuilderLightChannel", [&]() {
            LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
            builder->lightChannel(channel, (bool)enable);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_LightManager_nBuilderBuild(JNIEnv* env, jclass,
        jlong nativeBuilder, jlong nativeEngine, jint entity) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_LightManager_nBuilderBuild", JNI_FALSE, [&]() -> jboolean {
            LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
            Engine *engine = (Engine *) nativeEngine;
            return jboolean(builder->build(*engine, (Entity &) entity) == LightManager::Builder::Success);
    });
}

// ------------------------------------------------------------------------------------------------

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nComputeUniformSplits(JNIEnv* env, jclass,
        jfloatArray splitPositions, jint cascades) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nComputeUniformSplits", [&]() {
            jfloat *nativeSplits = env->GetFloatArrayElements(splitPositions, NULL);
            LightManager::ShadowCascades::computeUniformSplits(nativeSplits, (uint8_t) cascades);
            env->ReleaseFloatArrayElements(splitPositions, nativeSplits, 0);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nComputeLogSplits(JNIEnv* env, jclass,
        jfloatArray splitPositions, jint cascades, jfloat near, jfloat far) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nComputeLogSplits", [&]() {
            jfloat *nativeSplits = env->GetFloatArrayElements(splitPositions, NULL);
            LightManager::ShadowCascades::computeLogSplits(nativeSplits, (uint8_t) cascades, near, far);
            env->ReleaseFloatArrayElements(splitPositions, nativeSplits, 0);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nComputePracticalSplits(JNIEnv* env, jclass,
        jfloatArray splitPositions, jint cascades, jfloat near, jfloat far, jfloat lambda) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nComputePracticalSplits", [&]() {
            jfloat *nativeSplits = env->GetFloatArrayElements(splitPositions, NULL);
            LightManager::ShadowCascades::computePracticalSplits(nativeSplits, (uint8_t) cascades, near, far, lambda);
            env->ReleaseFloatArrayElements(splitPositions, nativeSplits, 0);
    });
}

// ------------------------------------------------------------------------------------------------

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_LightManager_nGetType(JNIEnv* env,
        jclass type, jlong nativeLightManager, jint i) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_LightManager_nGetType", 0, [&]() -> jint {
            LightManager *lm = (LightManager *) nativeLightManager;
            return (jint)lm->getType((LightManager::Instance) i);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetPosition(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i, jfloat x, jfloat y, jfloat z) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nSetPosition", [&]() {
            LightManager *lm = (LightManager *) nativeLightManager;
            lm->setPosition((LightManager::Instance) i, {x, y, z});
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nGetPosition(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i, jfloatArray out_) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nGetPosition", [&]() {
            LightManager *lm = (LightManager *) nativeLightManager;
            jfloat *out = env->GetFloatArrayElements(out_, nullptr);
            *reinterpret_cast<filament::math::float3 *>(out) = lm->getPosition((LightManager::Instance) i);
            env->ReleaseFloatArrayElements(out_, out, 0);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetDirection(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i, jfloat x, jfloat y, jfloat z) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nSetDirection", [&]() {
            LightManager *lm = (LightManager *) nativeLightManager;
            lm->setDirection((LightManager::Instance) i, {x, y, z});
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nGetDirection(JNIEnv *env, jclass,
        jlong nativeLightManager, jint i, jfloatArray out_) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nGetDirection", [&]() {
            LightManager *lm = (LightManager *) nativeLightManager;
            jfloat *out = env->GetFloatArrayElements(out_, nullptr);
            *reinterpret_cast<filament::math::float3 *>(out) = lm->getDirection((LightManager::Instance) i);
            env->ReleaseFloatArrayElements(out_, out, 0);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetColor(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i, jfloat linearR, jfloat linearG, jfloat linearB) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nSetColor", [&]() {
            LightManager *lm = (LightManager *) nativeLightManager;
            lm->setColor((LightManager::Instance) i, {linearR, linearG, linearB});
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nGetColor(JNIEnv *env, jclass,
        jlong nativeLightManager, jint i, jfloatArray out_) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nGetColor", [&]() {
            LightManager *lm = (LightManager *) nativeLightManager;
            jfloat *out = env->GetFloatArrayElements(out_, nullptr);
            *reinterpret_cast<filament::math::float3 *>(out) = lm->getColor((LightManager::Instance) i);
            env->ReleaseFloatArrayElements(out_, out, 0);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetIntensity__JIF(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i, jfloat intensity) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nSetIntensity__JIF", [&]() {
            LightManager *lm = (LightManager *) nativeLightManager;
            lm->setIntensity((LightManager::Instance) i, intensity);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetIntensity__JIFF(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i, jfloat watts, jfloat efficiency) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nSetIntensity__JIFF", [&]() {
            LightManager *lm = (LightManager *) nativeLightManager;
            lm->setIntensity((LightManager::Instance) i, watts, efficiency);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetIntensityCandela(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i, jfloat intensity) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nSetIntensityCandela", [&]() {
            LightManager *lm = (LightManager *) nativeLightManager;
            lm->setIntensityCandela((LightManager::Instance) i, intensity);
    });
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_LightManager_nGetIntensity(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i) {
    return filament::android::jniGuard<jfloat>(env, "Java_com_google_android_filament_LightManager_nGetIntensity", 0.0f, [&]() -> jfloat {
            LightManager *lm = (LightManager *) nativeLightManager;
            return lm->getIntensity((LightManager::Instance) i);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetFalloff(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i, jfloat falloff) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nSetFalloff", [&]() {
            LightManager *lm = (LightManager *) nativeLightManager;
            lm->setFalloff((LightManager::Instance) i, falloff);
    });
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_LightManager_nGetFalloff(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i) {
    return filament::android::jniGuard<jfloat>(env, "Java_com_google_android_filament_LightManager_nGetFalloff", 0.0f, [&]() -> jfloat {
            LightManager *lm = (LightManager *) nativeLightManager;
            return lm->getFalloff((LightManager::Instance) i);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetSpotLightCone(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i, jfloat inner, jfloat outer) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nSetSpotLightCone", [&]() {
            LightManager *lm = (LightManager *) nativeLightManager;
            lm->setSpotLightCone((LightManager::Instance) i, inner, outer);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetSunAngularRadius(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i, jfloat angularRadius) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nSetSunAngularRadius", [&]() {
            LightManager *lm = (LightManager *) nativeLightManager;
            lm->setSunAngularRadius((LightManager::Instance) i, angularRadius);
    });
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_LightManager_nGetSunAngularRadius(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i) {
    return filament::android::jniGuard<jfloat>(env, "Java_com_google_android_filament_LightManager_nGetSunAngularRadius", 0.0f, [&]() -> jfloat {
            LightManager *lm = (LightManager *) nativeLightManager;
            return lm->getSunAngularRadius((LightManager::Instance) i);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetSunHaloSize(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i, jfloat haloSize) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nSetSunHaloSize", [&]() {
            LightManager *lm = (LightManager *) nativeLightManager;
            lm->setSunHaloSize((LightManager::Instance) i, haloSize);
    });
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_LightManager_nGetSunHaloSize(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i) {
    return filament::android::jniGuard<jfloat>(env, "Java_com_google_android_filament_LightManager_nGetSunHaloSize", 0.0f, [&]() -> jfloat {
            LightManager *lm = (LightManager *) nativeLightManager;
            return lm->getSunHaloSize((LightManager::Instance) i);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetSunHaloFalloff(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i, jfloat haloFalloff) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nSetSunHaloFalloff", [&]() {
            LightManager *lm = (LightManager *) nativeLightManager;
            lm->setSunHaloFalloff((LightManager::Instance) i, haloFalloff);
    });
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_LightManager_nGetSunHaloFalloff(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i) {
    return filament::android::jniGuard<jfloat>(env, "Java_com_google_android_filament_LightManager_nGetSunHaloFalloff", 0.0f, [&]() -> jfloat {
            LightManager *lm = (LightManager *) nativeLightManager;
            return lm->getSunHaloFalloff((LightManager::Instance) i);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetShadowCaster(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i, jboolean shadowCaster) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nSetShadowCaster", [&]() {
            LightManager *lm = (LightManager *) nativeLightManager;
            lm->setShadowCaster((LightManager::Instance) i, shadowCaster);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_LightManager_nIsShadowCaster(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_LightManager_nIsShadowCaster", JNI_FALSE, [&]() -> jboolean {
            LightManager *lm = (LightManager *) nativeLightManager;
            return (jboolean)lm->isShadowCaster((LightManager::Instance) i);
    });
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_LightManager_nGetOuterConeAngle(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i) {
    return filament::android::jniGuard<jfloat>(env, "Java_com_google_android_filament_LightManager_nGetOuterConeAngle", 0.0f, [&]() -> jfloat {
            LightManager *lm = (LightManager *) nativeLightManager;
            return (jfloat)lm->getSpotLightOuterCone((LightManager::Instance) i);
    });
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_LightManager_nGetInnerConeAngle(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i) {
    return filament::android::jniGuard<jfloat>(env, "Java_com_google_android_filament_LightManager_nGetInnerConeAngle", 0.0f, [&]() -> jfloat {
            LightManager *lm = (LightManager *) nativeLightManager;
            return (jfloat)lm->getSpotLightInnerCone((LightManager::Instance) i);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetLightChannel(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i, jint channel, jboolean enable) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_LightManager_nSetLightChannel", [&]() {
            LightManager *lm = (LightManager *) nativeLightManager;
            lm->setLightChannel((LightManager::Instance) i, channel, (bool)enable);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_LightManager_nGetLightChannel(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i, jint channel) {
    return filament::android::jniGuard<jboolean>(env, "Java_com_google_android_filament_LightManager_nGetLightChannel", JNI_FALSE, [&]() -> jboolean {
            LightManager const *lm = (LightManager const *) nativeLightManager;
            return lm->getLightChannel((LightManager::Instance) i, channel);
    });
}
