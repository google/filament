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

#include <filament/ColorGrading.h>
#include <filament/ToneMapper.h>

#include <math/vec3.h>
#include <math/vec4.h>

using namespace filament;
using namespace math;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_ColorGrading_nCreateBuilder(JNIEnv*, jclass) {
    return (jlong) new ColorGrading::Builder();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nDestroyBuilder(JNIEnv*, jclass, jlong nativeBuilder) {
    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    delete builder;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderBuild(JNIEnv*, jclass, jlong nativeBuilder, jlong nativeEngine) {
    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    Engine *engine = (Engine *) nativeEngine;
    return (jlong) builder->build(*engine);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderQuality(JNIEnv*, jclass, jlong nativeBuilder, jint quality_) {
    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    ColorGrading::QualityLevel quality = (ColorGrading::QualityLevel) quality_;
    builder->quality(quality);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderFormat(JNIEnv*, jclass, jlong nativeBuilder, jint format_) {
    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    ColorGrading::LutFormat format = (ColorGrading::LutFormat) format_;
    builder->format(format);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderDimensions(JNIEnv*, jclass, jlong nativeBuilder, jint dim_) {
    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    builder->dimensions((uint8_t)dim_);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderToneMapper(JNIEnv*, jclass,
        jlong nativeBuilder, jlong toneMapper_) {
    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    const ToneMapper* toneMapper = (const ToneMapper*) toneMapper_;
    builder->toneMapper(toneMapper);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderToneMapping(JNIEnv*, jclass,
        jlong nativeBuilder, jint toneMapping_) {
    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    ColorGrading::ToneMapping toneMapping = (ColorGrading::ToneMapping) toneMapping_;
    builder->toneMapping(toneMapping);
}
#pragma clang diagnostic pop

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderLuminanceScaling(JNIEnv*, jclass,
        jlong nativeBuilder, jboolean luminanceScaling) {
    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    builder->luminanceScaling(luminanceScaling);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderGamutMapping(JNIEnv*, jclass,
        jlong nativeBuilder, jboolean gamutMapping) {
    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    builder->gamutMapping(gamutMapping);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderExposure(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat exposure) {
    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    builder->exposure(exposure);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderNightAdaptation(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat adaptation) {
    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    builder->nightAdaptation(adaptation);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderWhiteBalance(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat temperature, jfloat tint) {
    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    builder->whiteBalance(temperature, tint);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderChannelMixer(JNIEnv* env, jclass,
        jlong nativeBuilder, jfloatArray outRed_, jfloatArray outGreen_, jfloatArray outBlue_) {

    jfloat* outRed = env->GetFloatArrayElements(outRed_, nullptr);
    jfloat* outGreen = env->GetFloatArrayElements(outGreen_, nullptr);
    jfloat* outBlue = env->GetFloatArrayElements(outBlue_, nullptr);

    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    builder->channelMixer(
            *reinterpret_cast<float3*>(outRed),
            *reinterpret_cast<float3*>(outGreen),
            *reinterpret_cast<float3*>(outBlue));

    env->ReleaseFloatArrayElements(outRed_, outRed, JNI_ABORT);
    env->ReleaseFloatArrayElements(outGreen_, outGreen, JNI_ABORT);
    env->ReleaseFloatArrayElements(outBlue_, outBlue, JNI_ABORT);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderShadowsMidtonesHighlights(JNIEnv* env, jclass, jlong nativeBuilder,
        jfloatArray shadows_, jfloatArray midtones_, jfloatArray highlights_, jfloatArray ranges_) {

    jfloat* shadows = env->GetFloatArrayElements(shadows_, nullptr);
    jfloat* midtones = env->GetFloatArrayElements(midtones_, nullptr);
    jfloat* highlights = env->GetFloatArrayElements(highlights_, nullptr);
    jfloat* ranges = env->GetFloatArrayElements(ranges_, nullptr);

    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    builder->shadowsMidtonesHighlights(
            *reinterpret_cast<float4*>(shadows),
            *reinterpret_cast<float4*>(midtones),
            *reinterpret_cast<float4*>(highlights),
            *reinterpret_cast<float4*>(ranges));

    env->ReleaseFloatArrayElements(shadows_, shadows, JNI_ABORT);
    env->ReleaseFloatArrayElements(midtones_, midtones, JNI_ABORT);
    env->ReleaseFloatArrayElements(highlights_, highlights, JNI_ABORT);
    env->ReleaseFloatArrayElements(ranges_, ranges, JNI_ABORT);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderSlopeOffsetPower(JNIEnv* env, jclass,
        jlong nativeBuilder, jfloatArray slope_, jfloatArray offset_, jfloatArray power_) {

    jfloat* slope = env->GetFloatArrayElements(slope_, nullptr);
    jfloat* offset = env->GetFloatArrayElements(offset_, nullptr);
    jfloat* power = env->GetFloatArrayElements(power_, nullptr);

    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    builder->slopeOffsetPower(
            *reinterpret_cast<float3*>(slope),
            *reinterpret_cast<float3*>(offset),
            *reinterpret_cast<float3*>(power));

    env->ReleaseFloatArrayElements(slope_, slope, JNI_ABORT);
    env->ReleaseFloatArrayElements(offset_, offset, JNI_ABORT);
    env->ReleaseFloatArrayElements(power_, power, JNI_ABORT);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderContrast(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat contrast) {
    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    builder->contrast(contrast);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderVibrance(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat vibrance) {
    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    builder->vibrance(vibrance);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderSaturation(JNIEnv*, jclass,
        jlong nativeBuilder, jfloat saturation) {
    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    builder->saturation(saturation);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_ColorGrading_nBuilderCurves(JNIEnv* env, jclass,
        jlong nativeBuilder, jfloatArray gamma_, jfloatArray midPoint_, jfloatArray scale_) {

    jfloat* gamma = env->GetFloatArrayElements(gamma_, nullptr);
    jfloat* midPoint = env->GetFloatArrayElements(midPoint_, nullptr);
    jfloat* scale = env->GetFloatArrayElements(scale_, nullptr);

    ColorGrading::Builder* builder = (ColorGrading::Builder*) nativeBuilder;
    builder->curves(
            *reinterpret_cast<float3*>(gamma),
            *reinterpret_cast<float3*>(midPoint),
            *reinterpret_cast<float3*>(scale));

    env->ReleaseFloatArrayElements(gamma_, gamma, JNI_ABORT);
    env->ReleaseFloatArrayElements(midPoint_, midPoint, JNI_ABORT);
    env->ReleaseFloatArrayElements(scale_, scale, JNI_ABORT);
}
