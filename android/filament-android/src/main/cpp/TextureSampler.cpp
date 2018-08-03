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

#include <filament/TextureSampler.h>

using namespace filament;

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nCreateSampler(JNIEnv *env, jclass type, jint min,
        jint max, jint s, jint t, jint r) {
    return TextureSampler(static_cast<TextureSampler::MinFilter>(min),
            static_cast<TextureSampler::MagFilter>(max), static_cast<TextureSampler::WrapMode>(s),
            static_cast<TextureSampler::WrapMode>(t),
            static_cast<TextureSampler::WrapMode>(r)).getSamplerParams().u;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nCreateCompareSampler(JNIEnv *env, jclass type,
        jint mode, jint function) {
    return TextureSampler(static_cast<TextureSampler::CompareMode>(mode),
            static_cast<TextureSampler::CompareFunc>(function)).getSamplerParams().u;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nGetMinFilter(JNIEnv *env, jclass type,
        jint sampler_) {
    TextureSampler &sampler = reinterpret_cast<TextureSampler &>(sampler_);
    return static_cast<jint>(sampler.getMinFilter());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nSetMinFilter(JNIEnv *env, jclass type,
        jint sampler_, jint filter) {
    TextureSampler &sampler = reinterpret_cast<TextureSampler &>(sampler_);
    sampler.setMinFilter(static_cast<TextureSampler::MinFilter>(filter));
    return sampler.getSamplerParams().u;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nGetMagFilter(JNIEnv *env, jclass type,
        jint sampler_) {
    TextureSampler &sampler = reinterpret_cast<TextureSampler &>(sampler_);
    return static_cast<jint>(sampler.getMagFilter());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nSetMagFilter(JNIEnv *env, jclass type,
        jint sampler_, jint filter) {
    TextureSampler &sampler = reinterpret_cast<TextureSampler &>(sampler_);
    sampler.setMagFilter(static_cast<TextureSampler::MagFilter>(filter));
    return sampler.getSamplerParams().u;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nGetWrapModeS(JNIEnv *env, jclass type,
        jint sampler_) {
    TextureSampler &sampler = reinterpret_cast<TextureSampler &>(sampler_);
    return static_cast<jint>(sampler.getWrapModeS());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nSetWrapModeS(JNIEnv *env, jclass type,
        jint sampler_, jint mode) {
    TextureSampler &sampler = reinterpret_cast<TextureSampler &>(sampler_);
    sampler.setWrapModeS(static_cast<TextureSampler::WrapMode>(mode));
    return sampler.getSamplerParams().u;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nGetWrapModeT(JNIEnv *env, jclass type,
        jint sampler_) {
    TextureSampler &sampler = reinterpret_cast<TextureSampler &>(sampler_);
    return static_cast<jint>(sampler.getWrapModeT());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nSetWrapModeT(JNIEnv *env, jclass type,
        jint sampler_, jint mode) {
    TextureSampler &sampler = reinterpret_cast<TextureSampler &>(sampler_);
    sampler.setWrapModeT(static_cast<TextureSampler::WrapMode>(mode));
    return sampler.getSamplerParams().u;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nGetWrapModeR(JNIEnv *env, jclass type,
        jint sampler_) {
    TextureSampler &sampler = reinterpret_cast<TextureSampler &>(sampler_);
    return static_cast<jint>(sampler.getWrapModeR());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nSetWrapModeR(JNIEnv *env, jclass type,
        jint sampler_, jint mode) {
    TextureSampler &sampler = reinterpret_cast<TextureSampler &>(sampler_);
    sampler.setWrapModeR(static_cast<TextureSampler::WrapMode>(mode));
    return sampler.getSamplerParams().u;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nGetCompareMode(JNIEnv *env, jclass type,
        jint sampler_) {
    TextureSampler &sampler = reinterpret_cast<TextureSampler &>(sampler_);
    return static_cast<jint>(sampler.getCompareMode());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nSetCompareMode(JNIEnv *env, jclass type,
        jint sampler_, jint mode) {
    TextureSampler &sampler = reinterpret_cast<TextureSampler &>(sampler_);
    sampler.setCompareMode(static_cast<TextureSampler::CompareMode>(mode),
            sampler.getCompareFunc());
    return sampler.getSamplerParams().u;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nGetCompareFunction(JNIEnv *env, jclass type,
        jint sampler_) {
    TextureSampler &sampler = reinterpret_cast<TextureSampler &>(sampler_);
    return static_cast<jint>(sampler.getCompareFunc());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nSetCompareFunction(JNIEnv *env, jclass type,
        jint sampler_, jint function) {
    TextureSampler &sampler = reinterpret_cast<TextureSampler &>(sampler_);
    sampler.setCompareMode(sampler.getCompareMode(),
            static_cast<TextureSampler::CompareFunc>(function));
    return sampler.getSamplerParams().u;
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_TextureSampler_nGetAnisotropy(JNIEnv *env, jclass type,
        jint sampler_) {
    TextureSampler &sampler = reinterpret_cast<TextureSampler &>(sampler_);
    return sampler.getAnisotropy();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_TextureSampler_nSetAnisotropy(JNIEnv *env, jclass type,
        jint sampler_, jfloat anisotropy) {
    TextureSampler &sampler = reinterpret_cast<TextureSampler &>(sampler_);
    sampler.setAnisotropy(anisotropy);
    return sampler.getSamplerParams().u;
}
