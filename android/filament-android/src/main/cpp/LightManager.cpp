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

using namespace filament;
using namespace utils;

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_LightManager_nGetComponentCount(JNIEnv*, jclass,
        jlong nativeLightManager) {
    LightManager *lm = (LightManager *) nativeLightManager;
    return lm->getComponentCount();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_LightManager_nHasComponent(JNIEnv*, jclass,
        jlong nativeLightManager, jint entity) {
    LightManager *lm = (LightManager *) nativeLightManager;
    return (jboolean) lm->hasComponent((Entity &) entity);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_LightManager_nGetInstance(JNIEnv*, jclass,
        jlong nativeLightManager, jint entity) {
    LightManager *lm = (LightManager *) nativeLightManager;
    return lm->getInstance((Entity &) entity);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nDestroy(JNIEnv*, jclass,
        jlong nativeLightManager, jint entity) {
    LightManager *lm = (LightManager *) nativeLightManager;
    lm->destroy((Entity &) entity);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_LightManager_nCreateBuilder(JNIEnv*, jclass, jint lightType) {
    return (jlong) new LightManager::Builder((LightManager::Type) lightType);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nDestroyBuilder(JNIEnv*, jclass,
        jlong nativeBuilder) {
    LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
    delete builder;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderCastShadows(JNIEnv*, jclass,
        jlong nativeBuilder, jboolean enable) {
    LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
    builder->castShadows(enable);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderShadowOptions(JNIEnv*, jclass,
        jlong nativeBuilder, jint mapSize, jfloat constantBias, jfloat normalBias, jfloat shadowFar,
        jfloat shadowNearHint, jfloat shadowFarHint, jboolean stable,
        jboolean screenSpaceContactShadows, jint stepCount, jfloat maxShadowDistance) {
    LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
    builder->shadowOptions(
            LightManager::ShadowOptions{.mapSize = (uint32_t)mapSize,
                                        .constantBias = constantBias,
                                        .normalBias = normalBias,
                                        .shadowFar = shadowFar,
                                        .shadowNearHint = shadowNearHint,
                                        .shadowFarHint = shadowFarHint,
                                        .stable = (bool)stable,
                                        .screenSpaceContactShadows = (bool)screenSpaceContactShadows,
                                        .stepCount = uint8_t(stepCount),
                                        .maxShadowDistance = maxShadowDistance});
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderCastLight(JNIEnv*, jclass,
        jlong nativeBuilder, jboolean enabled) {
    LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
    builder->castLight(enabled);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderPosition(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat x, jfloat y, jfloat z) {
    LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
    builder->position({x, y, z});
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderDirection(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat x, jfloat y, jfloat z) {
    LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
    builder->direction({x, y, z});
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderColor(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat linearR, jfloat linearG, jfloat linearB) {
    LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
    builder->color({linearR, linearG, linearB});
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderIntensityCandela(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat intensity) {
    LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
    builder->intensityCandela(intensity);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderIntensity__JF(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat intensity) {
    LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
    builder->intensity(intensity);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderIntensity__JFF(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat watts, jfloat efficiency) {
    LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
    builder->intensity(watts, efficiency);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderFalloff(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat radius) {
    LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
    builder->falloff(radius);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderSpotLightCone(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat inner, jfloat outer) {
    LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
    builder->spotLightCone(inner, outer);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderAngularRadius(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat angularRadius) {
    LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
    builder->sunAngularRadius(angularRadius);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderHaloSize(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat haloSize) {
    LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
    builder->sunHaloSize(haloSize);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nBuilderHaloFalloff(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat haloFalloff) {
    LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
    builder->sunHaloFalloff(haloFalloff);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_LightManager_nBuilderBuild(JNIEnv*, jclass,
        jlong nativeBuilder, jlong nativeEngine, jint entity) {
    LightManager::Builder *builder = (LightManager::Builder *) nativeBuilder;
    Engine *engine = (Engine *) nativeEngine;
    return jboolean(builder->build(*engine, (Entity &) entity) == LightManager::Builder::Success);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_LightManager_nGetType(JNIEnv* env,
        jclass type, jlong nativeLightManager, jint i) {
    LightManager *lm = (LightManager *) nativeLightManager;
    return (jint)lm->getType((LightManager::Instance) i);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetPosition(JNIEnv*, jclass,
        jlong nativeLightManager, jint i, jfloat x, jfloat y, jfloat z) {
    LightManager *lm = (LightManager *) nativeLightManager;
    lm->setPosition((LightManager::Instance) i, {x, y, z});
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nGetPosition(JNIEnv* env, jclass,
        jlong nativeLightManager, jint i, jfloatArray out_) {
    LightManager *lm = (LightManager *) nativeLightManager;
    jfloat *out = env->GetFloatArrayElements(out_, nullptr);
    *reinterpret_cast<filament::math::float3 *>(out) = lm->getPosition((LightManager::Instance) i);
    env->ReleaseFloatArrayElements(out_, out, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetDirection(JNIEnv*, jclass,
        jlong nativeLightManager, jint i, jfloat x, jfloat y, jfloat z) {
    LightManager *lm = (LightManager *) nativeLightManager;
    lm->setDirection((LightManager::Instance) i, {x, y, z});
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nGetDirection(JNIEnv *env, jclass,
        jlong nativeLightManager, jint i, jfloatArray out_) {
    LightManager *lm = (LightManager *) nativeLightManager;
    jfloat *out = env->GetFloatArrayElements(out_, nullptr);
    *reinterpret_cast<filament::math::float3 *>(out) = lm->getDirection((LightManager::Instance) i);
    env->ReleaseFloatArrayElements(out_, out, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetColor(JNIEnv*, jclass,
        jlong nativeLightManager, jint i, jfloat linearR, jfloat linearG, jfloat linearB) {
    LightManager *lm = (LightManager *) nativeLightManager;
    lm->setColor((LightManager::Instance) i, {linearR, linearG, linearB});
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nGetColor(JNIEnv *env, jclass,
        jlong nativeLightManager, jint i, jfloatArray out_) {
    LightManager *lm = (LightManager *) nativeLightManager;
    jfloat *out = env->GetFloatArrayElements(out_, nullptr);
    *reinterpret_cast<filament::math::float3 *>(out) = lm->getColor((LightManager::Instance) i);
    env->ReleaseFloatArrayElements(out_, out, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetIntensity__JIF(JNIEnv*, jclass,
        jlong nativeLightManager, jint i, jfloat intensity) {
    LightManager *lm = (LightManager *) nativeLightManager;
    lm->setIntensity((LightManager::Instance) i, intensity);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetIntensity__JIFF(JNIEnv*, jclass,
        jlong nativeLightManager, jint i, jfloat watts, jfloat efficiency) {
    LightManager *lm = (LightManager *) nativeLightManager;
    lm->setIntensity((LightManager::Instance) i, watts, efficiency);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetIntensityCandela(JNIEnv*, jclass,
        jlong nativeLightManager, jint i, jfloat intensity) {
    LightManager *lm = (LightManager *) nativeLightManager;
    lm->setIntensityCandela((LightManager::Instance) i, intensity);
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_LightManager_nGetIntensity(JNIEnv*, jclass,
        jlong nativeLightManager, jint i) {
    LightManager *lm = (LightManager *) nativeLightManager;
    return lm->getIntensity((LightManager::Instance) i);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetFalloff(JNIEnv*, jclass,
        jlong nativeLightManager, jint i, jfloat falloff) {
    LightManager *lm = (LightManager *) nativeLightManager;
    lm->setFalloff((LightManager::Instance) i, falloff);
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_LightManager_nGetFalloff(JNIEnv*, jclass,
        jlong nativeLightManager, jint i) {
    LightManager *lm = (LightManager *) nativeLightManager;
    return lm->getFalloff((LightManager::Instance) i);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetSpotLightCone(JNIEnv*, jclass,
        jlong nativeLightManager, jint i, jfloat inner, jfloat outer) {
    LightManager *lm = (LightManager *) nativeLightManager;
    lm->setSpotLightCone((LightManager::Instance) i, inner, outer);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetSunAngularRadius(JNIEnv*, jclass,
        jlong nativeLightManager, jint i, jfloat angularRadius) {
    LightManager *lm = (LightManager *) nativeLightManager;
    lm->setSunAngularRadius((LightManager::Instance) i, angularRadius);
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_LightManager_nGetSunAngularRadius(JNIEnv*, jclass,
        jlong nativeLightManager, jint i) {
    LightManager *lm = (LightManager *) nativeLightManager;
    return lm->getSunAngularRadius((LightManager::Instance) i);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetSunHaloSize(JNIEnv*, jclass,
        jlong nativeLightManager, jint i, jfloat haloSize) {
    LightManager *lm = (LightManager *) nativeLightManager;
    lm->setSunHaloSize((LightManager::Instance) i, haloSize);
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_LightManager_nGetSunHaloSize(JNIEnv*, jclass,
        jlong nativeLightManager, jint i) {
    LightManager *lm = (LightManager *) nativeLightManager;
    return lm->getSunHaloSize((LightManager::Instance) i);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetSunHaloFalloff(JNIEnv*, jclass,
        jlong nativeLightManager, jint i, jfloat haloFalloff) {
    LightManager *lm = (LightManager *) nativeLightManager;
    lm->setSunHaloFalloff((LightManager::Instance) i, haloFalloff);
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_LightManager_nGetSunHaloFalloff(JNIEnv*, jclass,
        jlong nativeLightManager, jint i) {
    LightManager *lm = (LightManager *) nativeLightManager;
    return lm->getSunHaloFalloff((LightManager::Instance) i);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_LightManager_nSetShadowCaster(JNIEnv*, jclass,
        jlong nativeLightManager, jint i, jboolean shadowCaster) {
    LightManager *lm = (LightManager *) nativeLightManager;
    lm->setShadowCaster((LightManager::Instance) i, shadowCaster);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_LightManager_nIsShadowCaster(JNIEnv*, jclass,
        jlong nativeLightManager, jint i) {
    LightManager *lm = (LightManager *) nativeLightManager;
    return (jboolean)lm->isShadowCaster((LightManager::Instance) i);
}
