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

#include <filament/Skybox.h>

#include <math/vec4.h>
#include "../../../../common/JniExceptionBridge.h"

using namespace filament;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Skybox_nCreateBuilder(JNIEnv *env, jclass type) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_Skybox_nCreateBuilder", 0, [&]() -> jlong {
            return (jlong) new Skybox::Builder{};
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Skybox_nDestroyBuilder(JNIEnv *env, jclass type,
        jlong nativeSkyBoxBuilder) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Skybox_nDestroyBuilder", [&]() {
            Skybox::Builder *builder = (Skybox::Builder *) nativeSkyBoxBuilder;
            delete builder;
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Skybox_nBuilderEnvironment(JNIEnv *env, jclass type,
        jlong nativeSkyBoxBuilder, jlong nativeTexture) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Skybox_nBuilderEnvironment", [&]() {
            Skybox::Builder *builder = (Skybox::Builder *) nativeSkyBoxBuilder;
            Texture *texture = (Texture *) nativeTexture;
            builder->environment(texture);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Skybox_nBuilderShowSun(JNIEnv *env, jclass type,
        jlong nativeSkyBoxBuilder, jboolean show) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Skybox_nBuilderShowSun", [&]() {
            Skybox::Builder *builder = (Skybox::Builder *) nativeSkyBoxBuilder;
            builder->showSun(show);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Skybox_nBuilderIntensity(JNIEnv *env, jclass clazz,
        jlong nativeSkyBoxBuilder, jfloat intensity) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Skybox_nBuilderIntensity", [&]() {
            Skybox::Builder *builder = (Skybox::Builder *) nativeSkyBoxBuilder;
            builder->intensity(intensity);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Skybox_nBuilderColor(JNIEnv * env,  jclass,
        jlong nativeSkyBoxBuilder, jfloat r, jfloat g, jfloat b, jfloat a) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Skybox_nBuilderColor", [&]() {
            Skybox::Builder *builder = (Skybox::Builder *) nativeSkyBoxBuilder;
            builder->color({r, g, b, a});
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Skybox_nBuilderBuild(JNIEnv *env, jclass type,
        jlong nativeSkyBoxBuilder, jlong nativeEngine) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_Skybox_nBuilderBuild", 0, [&]() -> jlong {
            Skybox::Builder *builder = (Skybox::Builder *) nativeSkyBoxBuilder;
            Engine *engine = (Engine *) nativeEngine;
            return (jlong) builder->build(*engine);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Skybox_nSetLayerMask(JNIEnv *env, jclass type, jlong nativeSkybox,
        jint select, jint value) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Skybox_nSetLayerMask", [&]() {
            Skybox *skybox = (Skybox *) nativeSkybox;
            skybox->setLayerMask((uint8_t) select, (uint8_t) value);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Skybox_nGetLayerMask(JNIEnv *env, jclass type,
        jlong nativeSkybox) {
    return filament::android::jniGuard<jint>(env, "Java_com_google_android_filament_Skybox_nGetLayerMask", 0, [&]() -> jint {
            Skybox *skybox = (Skybox *) nativeSkybox;
            return static_cast<jint>(skybox->getLayerMask());
    });
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_Skybox_nGetIntensity(JNIEnv *env, jclass clazz,
        jlong nativeSkybox) {
    return filament::android::jniGuard<jfloat>(env, "Java_com_google_android_filament_Skybox_nGetIntensity", 0.0f, [&]() -> jfloat {
            Skybox *skybox = (Skybox *) nativeSkybox;
            return static_cast<jint>(skybox->getIntensity());
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Skybox_nSetColor(JNIEnv * env, jclass,
        jlong nativeSkybox, jfloat r, jfloat g, jfloat b, jfloat a) {
    filament::android::jniGuardVoid(env, "Java_com_google_android_filament_Skybox_nSetColor", [&]() {
            Skybox *skybox = (Skybox *) nativeSkybox;
            skybox->setColor({r, g, b, a});
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Skybox_nGetTexture(JNIEnv* env, jclass,
        jlong nativeSkybox) {
    return filament::android::jniGuard<jlong>(env, "Java_com_google_android_filament_Skybox_nGetTexture", 0, [&]() -> jlong {
            Skybox *skybox = (Skybox *) nativeSkybox;
            Texture const *tex = skybox->getTexture();
            return (jlong) tex;
    });
}
