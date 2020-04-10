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

using namespace filament;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Skybox_nCreateBuilder(JNIEnv *env, jclass type) {
    return (jlong) new Skybox::Builder{};
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Skybox_nDestroyBuilder(JNIEnv *env, jclass type,
        jlong nativeSkyBoxBuilder) {
    Skybox::Builder *builder = (Skybox::Builder *) nativeSkyBoxBuilder;
    delete builder;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Skybox_nBuilderEnvironment(JNIEnv *env, jclass type,
        jlong nativeSkyBoxBuilder, jlong nativeTexture) {
    Skybox::Builder *builder = (Skybox::Builder *) nativeSkyBoxBuilder;
    Texture *texture = (Texture *) nativeTexture;
    builder->environment(texture);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Skybox_nBuilderShowSun(JNIEnv *env, jclass type,
        jlong nativeSkyBoxBuilder, jboolean show) {
    Skybox::Builder *builder = (Skybox::Builder *) nativeSkyBoxBuilder;
    builder->showSun(show);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Skybox_nBuilderIntensity(JNIEnv *env, jclass clazz,
        jlong nativeSkyBoxBuilder, jfloat intensity) {
    Skybox::Builder *builder = (Skybox::Builder *) nativeSkyBoxBuilder;
    builder->intensity(intensity);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Skybox_nBuilderColor(JNIEnv *,  jclass,
        jlong nativeSkyBoxBuilder, jfloat r, jfloat g, jfloat b, jfloat a) {
    Skybox::Builder *builder = (Skybox::Builder *) nativeSkyBoxBuilder;
    builder->color({r, g, b, a});
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_Skybox_nBuilderBuild(JNIEnv *env, jclass type,
        jlong nativeSkyBoxBuilder, jlong nativeEngine) {
    Skybox::Builder *builder = (Skybox::Builder *) nativeSkyBoxBuilder;
    Engine *engine = (Engine *) nativeEngine;
    return (jlong) builder->build(*engine);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Skybox_nSetLayerMask(JNIEnv *env, jclass type, jlong nativeSkybox,
        jint select, jint value) {
    Skybox *skybox = (Skybox *) nativeSkybox;
    skybox->setLayerMask((uint8_t) select, (uint8_t) value);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Skybox_nGetLayerMask(JNIEnv *env, jclass type,
        jlong nativeSkybox) {
    Skybox *skybox = (Skybox *) nativeSkybox;
    return static_cast<jint>(skybox->getLayerMask());
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_Skybox_nGetIntensity(JNIEnv *env, jclass clazz,
        jlong nativeSkybox) {
    Skybox *skybox = (Skybox *) nativeSkybox;
    return static_cast<jint>(skybox->getIntensity());
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Skybox_nSetColor(JNIEnv *, jclass,
        jlong nativeSkybox, jfloat r, jfloat g, jfloat b, jfloat a) {
    Skybox *skybox = (Skybox *) nativeSkybox;
    skybox->setColor({r, g, b, a});
}
