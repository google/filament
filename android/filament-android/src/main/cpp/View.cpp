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

#include <filament/View.h>
#include <filament/Viewport.h>

using namespace filament;

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetName(JNIEnv* env, jclass, jlong nativeView, jstring name_) {
    View* view = (View*) nativeView;
    const char* name = env->GetStringUTFChars(name_, 0);
    view->setName(name);
    env->ReleaseStringUTFChars(name_, name);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetScene(JNIEnv*, jclass, jlong nativeView, jlong nativeScene) {
    View* view = (View*) nativeView;
    Scene* scene = (Scene*) nativeScene;
    view->setScene(scene);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetCamera(JNIEnv*, jclass, jlong nativeView, jlong nativeCamera) {
    View* view = (View*) nativeView;
    Camera* camera = (Camera*) nativeCamera;
    view->setCamera(camera);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetViewport(JNIEnv*, jclass,
        jlong nativeView, jint left, jint bottom, jint width, jint height) {
    View* view = (View*) nativeView;
    view->setViewport({left, bottom, (uint32_t) width, (uint32_t) height});
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetVisibleLayers(JNIEnv*, jclass, jlong nativeView, jint select, jint value) {
    View* view = (View*) nativeView;
    view->setVisibleLayers((uint8_t) select, (uint8_t) value);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetShadowsEnabled(JNIEnv*, jclass, jlong nativeView, jboolean enabled) {
    View* view = (View*) nativeView;
    view->setShadowsEnabled(enabled);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetRenderTarget(JNIEnv*, jclass,
        jlong nativeView, jlong nativeTarget) {
    View* view = (View*) nativeView;
    view->setRenderTarget((RenderTarget*) nativeTarget);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetSampleCount(JNIEnv*, jclass, jlong nativeView, jint count) {
    View* view = (View*) nativeView;
    view->setSampleCount((uint8_t) count);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_View_nGetSampleCount(JNIEnv*, jclass, jlong nativeView) {
    View* view = (View*) nativeView;
    return view->getSampleCount();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetAntiAliasing(JNIEnv*, jclass, jlong nativeView, jint type) {
    View* view = (View*) nativeView;
    view->setAntiAliasing(View::AntiAliasing(type));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_View_nGetAntiAliasing(JNIEnv*, jclass, jlong nativeView) {
    View* view = (View*) nativeView;
    return (jint) view->getAntiAliasing();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetToneMapping(JNIEnv*, jclass, jlong nativeView, jint type) {
    View* view = (View*) nativeView;
    view->setToneMapping(View::ToneMapping(type));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_View_nGetToneMapping(JNIEnv*, jclass, jlong nativeView) {
    View* view = (View*) nativeView;
    return (jint) view->getToneMapping();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetDithering(JNIEnv*, jclass, jlong nativeView, jint dithering) {
    View* view = (View*) nativeView;
    view->setDithering((View::Dithering) dithering);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_View_nGetDithering(JNIEnv*, jclass,
        jlong nativeView) {
    View* view = (View*) nativeView;
    return (jint)view->getDithering();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetDynamicResolutionOptions(JNIEnv*, jclass, jlong nativeView,
        jboolean enabled, jboolean homogeneousScaling,
        jfloat minScale, jfloat maxScale, jint quality) {
    View* view = (View*)nativeView;
    View::DynamicResolutionOptions options;
    options.enabled = enabled;
    options.homogeneousScaling = homogeneousScaling;
    options.minScale = filament::math::float2{ minScale };
    options.maxScale = filament::math::float2{ maxScale };
    options.quality = (View::QualityLevel)quality;
    view->setDynamicResolutionOptions(options);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetRenderQuality(JNIEnv*, jclass,
        jlong nativeView, jint hdrColorBufferQuality) {
    View* view = (View*) nativeView;
    View::RenderQuality renderQuality;
    renderQuality.hdrColorBuffer = View::QualityLevel(hdrColorBufferQuality);
    view->setRenderQuality(renderQuality);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetDynamicLightingOptions(JNIEnv*,
        jclass, jlong nativeView, jfloat zLightNear, jfloat zLightFar) {
    View* view = (View*) nativeView;
    view->setDynamicLightingOptions(zLightNear, zLightFar);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetPostProcessingEnabled(JNIEnv*,
        jclass, jlong nativeView, jboolean enabled) {
    View* view = (View*) nativeView;
    view->setPostProcessingEnabled(enabled);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_View_nIsPostProcessingEnabled(JNIEnv*,
        jclass, jlong nativeView) {
    View* view = (View*) nativeView;
    return static_cast<jboolean>(view->isPostProcessingEnabled());
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetFrontFaceWindingInverted(JNIEnv*,
        jclass, jlong nativeView, jboolean inverted) {
    View* view = (View*) nativeView;
    view->setFrontFaceWindingInverted(inverted);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_View_nIsFrontFaceWindingInverted(JNIEnv*,
        jclass, jlong nativeView) {
    View* view = (View*) nativeView;
    return static_cast<jboolean>(view->isFrontFaceWindingInverted());
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetAmbientOcclusion(JNIEnv*, jclass, jlong nativeView, jint ordinal) {
    View* view = (View*) nativeView;
    view->setAmbientOcclusion((View::AmbientOcclusion) ordinal);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_View_nGetAmbientOcclusion(JNIEnv*, jclass, jlong nativeView) {
    View* view = (View*) nativeView;
    return (jint)view->getAmbientOcclusion();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetAmbientOcclusionOptions(JNIEnv*, jclass,
    jlong nativeView, jfloat radius, jfloat bias, jfloat power, jfloat resolution, jfloat intensity,
    jint quality) {
    View* view = (View*) nativeView;
    View::AmbientOcclusionOptions options = {
            .radius = radius,
            .power = power,
            .bias = bias,
            .resolution = resolution,
            .intensity = intensity,
            .quality = (View::QualityLevel)quality
    };
    view->setAmbientOcclusionOptions(options);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetBloomOptions(JNIEnv*, jclass,
        jlong nativeView, jlong nativeTexture,
        jfloat dirtStrength, jfloat strength, jint resolution, jfloat anamorphism, jint levels,
        jint blendMode, jboolean threshold, jboolean enabled) {
    View* view = (View*) nativeView;
    Texture* dirt = (Texture*) nativeTexture;
    View::BloomOptions options = {
            .dirt = dirt,
            .dirtStrength = dirtStrength,
            .strength = strength,
            .resolution = (uint32_t)resolution,
            .anamorphism = anamorphism,
            .levels = (uint8_t)levels,
            .blendMode = (View::BloomOptions::BlendMode)blendMode,
            .threshold = (bool)threshold,
            .enabled = (bool)enabled
    };
    view->setBloomOptions(options);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetFogOptions(JNIEnv *, jclass , jlong nativeView,
        jfloat distance, jfloat maximumOpacity, jfloat height, jfloat heightFalloff, jfloat r,
        jfloat g, jfloat b, jfloat density, jfloat inScatteringStart,
        jfloat inScatteringSize, jboolean fogColorFromIbl, jboolean enabled) {
    View* view = (View*) nativeView;
    View::FogOptions options = {
             .distance = distance,
             .maximumOpacity = maximumOpacity,
             .height = height,
             .heightFalloff = heightFalloff,
             .color = math::float3{r, g, b},
             .density = density,
             .inScatteringStart = inScatteringStart,
             .inScatteringSize = inScatteringSize,
             .fogColorFromIbl = (bool)fogColorFromIbl,
             .enabled = (bool)enabled
    };
    view->setFogOptions(options);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetBlendMode(JNIEnv *, jclass , jlong nativeView, jint blendMode) {
    View* view = (View*) nativeView;
    view->setBlendMode((View::BlendMode)blendMode);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetDepthOfFieldOptions(JNIEnv *, jclass ,
        jlong nativeView, jfloat focusDistance, jfloat blurScale, jfloat maxApertureDiameter, jboolean enabled) {
    View* view = (View*) nativeView;
    view->setDepthOfFieldOptions({.focusDistance = focusDistance, .blurScale = blurScale,
            .maxApertureDiameter = maxApertureDiameter, .enabled = (bool)enabled});
}
