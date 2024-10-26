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

#include <filament/Color.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include "common/CallbackUtils.h"

#include "private/backend/VirtualMachineEnv.h"

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
Java_com_google_android_filament_View_nSetCamera(JNIEnv*, jclass,
        jlong nativeView, jlong nativeCamera) {
    View* view = (View*) nativeView;
    Camera* camera = (Camera*) nativeCamera;
    view->setCamera(camera);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_View_nHasCamera(JNIEnv*, jclass, jlong nativeView) {
    View* view = (View*) nativeView;
    return (jboolean)view->hasCamera();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetColorGrading(JNIEnv*, jclass,
        jlong nativeView, jlong nativeColorGrading) {
    View* view = (View*) nativeView;
    ColorGrading* colorGrading = (ColorGrading*) nativeColorGrading;
    view->setColorGrading(colorGrading);
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
Java_com_google_android_filament_View_nSetShadowingEnabled(JNIEnv*, jclass, jlong nativeView, jboolean enabled) {
    View* view = (View*) nativeView;
    view->setShadowingEnabled(enabled);
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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    view->setSampleCount((uint8_t) count);
#pragma clang diagnostic pop
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_View_nGetSampleCount(JNIEnv*, jclass, jlong nativeView) {
    View* view = (View*) nativeView;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    return view->getSampleCount();
#pragma clang diagnostic pop
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
        jfloat minScale, jfloat maxScale, jfloat sharpness, jint quality) {
    View* view = (View*)nativeView;
    View::DynamicResolutionOptions options;
    options.enabled = enabled;
    options.homogeneousScaling = homogeneousScaling;
    options.minScale = filament::math::float2{ minScale };
    options.maxScale = filament::math::float2{ maxScale };
    options.sharpness = sharpness;
    options.quality = (View::QualityLevel)quality;
    view->setDynamicResolutionOptions(options);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetShadowType(JNIEnv*, jclass, jlong nativeView, jint type) {
    View* view = (View*) nativeView;
    view->setShadowType((View::ShadowType) type);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetVsmShadowOptions(JNIEnv*, jclass, jlong nativeView,
        jint anisotropy, jboolean mipmapping, jboolean highPrecision, jfloat minVarianceScale,
        jfloat lightBleedReduction) {
    View* view = (View*) nativeView;
    View::VsmShadowOptions options;
    options.anisotropy = (uint8_t)anisotropy;
    options.mipmapping = (bool)mipmapping;
    options.highPrecision = (bool)highPrecision;
    options.minVarianceScale = minVarianceScale;
    options.lightBleedReduction = lightBleedReduction;
    view->setVsmShadowOptions(options);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetSoftShadowOptions(JNIEnv*, jclass, jlong nativeView,
        jfloat penumbraScale, jfloat penumbraRatioScale) {
    View* view = (View*) nativeView;
    View::SoftShadowOptions options;
    options.penumbraScale = penumbraScale;
    options.penumbraRatioScale = penumbraRatioScale;
    view->setSoftShadowOptions(options);
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
Java_com_google_android_filament_View_nSetTransparentPickingEnabled(JNIEnv*,
        jclass, jlong nativeView, jboolean enabled) {
    View* view = (View*) nativeView;
    view->setTransparentPickingEnabled(enabled);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_View_nIsTransparentPickingEnabled(JNIEnv*,
        jclass, jlong nativeView) {
    View* view = (View*) nativeView;
    return static_cast<jboolean>(view->isTransparentPickingEnabled());
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetAmbientOcclusion(JNIEnv*, jclass, jlong nativeView, jint ordinal) {
    View* view = (View*) nativeView;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    view->setAmbientOcclusion((View::AmbientOcclusion) ordinal);
#pragma clang diagnostic pop
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_View_nGetAmbientOcclusion(JNIEnv*, jclass, jlong nativeView) {
    View* view = (View*) nativeView;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    return (jint)view->getAmbientOcclusion();
#pragma clang diagnostic pop
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetAmbientOcclusionOptions(JNIEnv*, jclass,
    jlong nativeView, jfloat radius, jfloat bias, jfloat power, jfloat resolution, jfloat intensity,
    jfloat bilateralThreshold,
    jint quality, jint lowPassFilter, jint upsampling, jboolean enabled, jboolean bentNormals,
    jfloat minHorizonAngleRad) {
    View* view = (View*) nativeView;
    View::AmbientOcclusionOptions options = view->getAmbientOcclusionOptions();
    options.radius = radius;
    options.power = power;
    options.bias = bias;
    options.resolution = resolution;
    options.intensity = intensity;
    options.bilateralThreshold = bilateralThreshold;
    options.quality = (View::QualityLevel)quality;
    options.lowPassFilter = (View::QualityLevel)lowPassFilter;
    options.upsampling = (View::QualityLevel)upsampling;
    options.enabled = (bool)enabled;
    options.bentNormals = (bool)bentNormals;
    options.minHorizonAngleRad = minHorizonAngleRad;
    view->setAmbientOcclusionOptions(options);
}


extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetSSCTOptions(JNIEnv *, jclass, jlong nativeView,
        jfloat ssctLightConeRad, jfloat ssctShadowDistance, jfloat ssctContactDistanceMax,
        jfloat ssctIntensity, jfloat ssctLightDirX, jfloat ssctLightDirY, jfloat ssctLightDirZ,
        jfloat ssctDepthBias, jfloat ssctDepthSlopeBias, jint ssctSampleCount,
        jint ssctRayCount, jboolean ssctEnabled) {
    View* view = (View*) nativeView;
    View::AmbientOcclusionOptions options = view->getAmbientOcclusionOptions();
    options.ssct.lightConeRad = ssctLightConeRad;
    options.ssct.shadowDistance = ssctShadowDistance;
    options.ssct.contactDistanceMax = ssctContactDistanceMax;
    options.ssct.intensity = ssctIntensity;
    options.ssct.lightDirection = math::float3{ ssctLightDirX, ssctLightDirY, ssctLightDirZ };
    options.ssct.depthBias = ssctDepthBias;
    options.ssct.depthSlopeBias = ssctDepthSlopeBias;
    options.ssct.sampleCount = (uint8_t)ssctSampleCount;
    options.ssct.rayCount = (uint8_t)ssctRayCount;
    options.ssct.enabled = (bool)ssctEnabled;
    view->setAmbientOcclusionOptions(options);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetBloomOptions(JNIEnv*, jclass,
        jlong nativeView, jlong nativeTexture,
        jfloat dirtStrength, jfloat strength, jint resolution, jint levels,
        jint blendMode, jboolean threshold, jboolean enabled, jfloat highlight,
        jboolean lensFlare, jboolean starburst, jfloat chromaticAberration, jint ghostCount,
        jfloat ghostSpacing, jfloat ghostThreshold, jfloat haloThickness, jfloat haloRadius,
        jfloat haloThreshold) {
    View* view = (View*) nativeView;
    Texture* dirt = (Texture*) nativeTexture;
    View::BloomOptions options = {
            .dirt = dirt,
            .dirtStrength = dirtStrength,
            .strength = strength,
            .resolution = (uint32_t)resolution,
            .levels = (uint8_t)levels,
            .blendMode = (View::BloomOptions::BlendMode)blendMode,
            .threshold = (bool)threshold,
            .enabled = (bool)enabled,
            .highlight = highlight,
            .lensFlare = (bool)lensFlare,
            .starburst = (bool)starburst,
            .chromaticAberration = chromaticAberration,
            .ghostCount = (uint8_t)ghostCount,
            .ghostSpacing = ghostSpacing,
            .ghostThreshold = ghostThreshold,
            .haloThickness = haloThickness,
            .haloRadius = haloRadius,
            .haloThreshold = haloThreshold
    };
    view->setBloomOptions(options);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetFogOptions(JNIEnv *, jclass , jlong nativeView,
        jfloat distance, jfloat maximumOpacity, jfloat height, jfloat heightFalloff, jfloat cutOffDistance,
        jfloat r, jfloat g, jfloat b, jfloat density, jfloat inScatteringStart,
        jfloat inScatteringSize, jboolean fogColorFromIbl, jlong skyColorNativeObject, jboolean enabled) {
    View* view = (View*) nativeView;
    Texture* skyColor = (Texture*) skyColorNativeObject;
    View::FogOptions options = {
             .distance = distance,
             .cutOffDistance = cutOffDistance,
             .maximumOpacity = maximumOpacity,
             .height = height,
             .heightFalloff = heightFalloff,
             .color = math::float3{r, g, b},
             .density = density,
             .inScatteringStart = inScatteringStart,
             .inScatteringSize = inScatteringSize,
             .fogColorFromIbl = (bool)fogColorFromIbl,
             .skyColor = skyColor,
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
Java_com_google_android_filament_View_nSetDepthOfFieldOptions(JNIEnv *, jclass,
        jlong nativeView, jfloat cocScale, jfloat maxApertureDiameter, jboolean enabled, jint filter,
        jboolean nativeResolution, jint foregroundRingCount, jint backgroundRingCount, jint fastGatherRingCount,
        jint maxForegroundCOC, jint maxBackgroundCOC) {
    View* view = (View*) nativeView;
    View::DepthOfFieldOptions::Filter eFilter{};
    if (filter == 1) {
        // View::DepthOfFieldOptions::Filter::MEDIAN value is actually 2
        eFilter = View::DepthOfFieldOptions::Filter::MEDIAN;
    }
    view->setDepthOfFieldOptions({.cocScale = cocScale,
            .maxApertureDiameter = maxApertureDiameter, .enabled = (bool)enabled, .filter = eFilter,
            .nativeResolution = (bool)nativeResolution,
            .foregroundRingCount = (uint8_t)foregroundRingCount,
            .backgroundRingCount = (uint8_t)backgroundRingCount,
            .fastGatherRingCount = (uint8_t)fastGatherRingCount,
            .maxForegroundCOC = (uint8_t)maxForegroundCOC,
            .maxBackgroundCOC = (uint8_t)maxBackgroundCOC,
    });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetVignetteOptions(JNIEnv*, jclass, jlong nativeView, jfloat midPoint, jfloat roundness,
        jfloat feather, jfloat r, jfloat g, jfloat b, jfloat a, jboolean enabled) {
    View* view = (View*) nativeView;
    view->setVignetteOptions({.midPoint = midPoint, .roundness = roundness, .feather = feather,
            .color = LinearColorA{r, g, b, a}, .enabled = (bool)enabled});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetMultiSampleAntiAliasingOptions(JNIEnv* env, jclass clazz,
        jlong nativeView, jboolean enabled, jint sampleCount, jboolean customResolve) {
    View* view = (View*) nativeView;
    view->setMultiSampleAntiAliasingOptions({
            .enabled = (bool)enabled,
            .sampleCount = (uint8_t)sampleCount,
            .customResolve = (bool)customResolve});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetTemporalAntiAliasingOptions(JNIEnv *, jclass,
        jlong nativeView, jfloat feedback, jfloat filterWidth, jboolean enabled) {
    View* view = (View*) nativeView;
    view->setTemporalAntiAliasingOptions({
            .filterWidth = filterWidth, .feedback = feedback, .enabled = (bool) enabled});
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetScreenSpaceReflectionsOptions(JNIEnv*, jclass,
        jlong nativeView, jfloat thickness, jfloat bias, jfloat maxDistance, jfloat stride, jboolean enabled) {
    View* view = (View*) nativeView;
    view->setScreenSpaceReflectionsOptions({.thickness = thickness, .bias = bias,
            .maxDistance = maxDistance, .stride = stride, .enabled = (bool) enabled
    });
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_View_nIsShadowingEnabled(JNIEnv *, jclass, jlong nativeView) {
    View* view = (View*) nativeView;
    return (jboolean)view->isShadowingEnabled();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetScreenSpaceRefractionEnabled(JNIEnv *, jclass,
        jlong nativeView, jboolean enabled) {
    View* view = (View*) nativeView;
    view->setScreenSpaceRefractionEnabled((bool)enabled);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_View_nIsScreenSpaceRefractionEnabled(JNIEnv *, jclass,
        jlong nativeView) {
    View* view = (View*) nativeView;
    return (jboolean)view->isScreenSpaceRefractionEnabled();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nPick(JNIEnv* env, jclass,
        jlong nativeView,
        jint x, jint y, jobject handler, jobject internalCallback) {

    // jniState will be initialized the first time this method is called
    static const struct JniState {
        jclass internalOnPickCallbackClass;
        jfieldID renderableFieldId;
        jfieldID depthFieldId;
        jfieldID fragCoordXFieldId;
        jfieldID fragCoordYFieldId;
        jfieldID fragCoordZFieldId;
        explicit JniState(JNIEnv* env) noexcept {
            internalOnPickCallbackClass = env->FindClass("com/google/android/filament/View$InternalOnPickCallback");
            renderableFieldId = env->GetFieldID(internalOnPickCallbackClass, "mRenderable", "I");
            depthFieldId = env->GetFieldID(internalOnPickCallbackClass, "mDepth", "F");
            fragCoordXFieldId = env->GetFieldID(internalOnPickCallbackClass, "mFragCoordsX", "F");
            fragCoordYFieldId = env->GetFieldID(internalOnPickCallbackClass, "mFragCoordsY", "F");
            fragCoordZFieldId = env->GetFieldID(internalOnPickCallbackClass, "mFragCoordsZ", "F");
        }
    } jniState(env);

    View* view = (View*) nativeView;
    JniCallback *callback = JniCallback::make(env, handler, internalCallback);
    view->pick(x, y, [callback](View::PickingQueryResult const& result) {
        // this is executed on the backend/service thread
        jobject obj = callback->getCallbackObject();
        JNIEnv* env = filament::VirtualMachineEnv::get().getEnvironment();
        env->SetIntField(obj, jniState.renderableFieldId, (jint)result.renderable.getId());
        env->SetFloatField(obj, jniState.depthFieldId, result.depth);
        env->SetFloatField(obj, jniState.fragCoordXFieldId, result.fragCoords.x);
        env->SetFloatField(obj, jniState.fragCoordYFieldId, result.fragCoords.y);
        env->SetFloatField(obj, jniState.fragCoordZFieldId, result.fragCoords.z);
        JniCallback::postToJavaAndDestroy(callback);
    }, callback->getHandler());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetStencilBufferEnabled(JNIEnv *, jclass, jlong nativeView,
        jboolean enabled) {
    View* view = (View*) nativeView;
    view->setStencilBufferEnabled(enabled);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_View_nIsStencilBufferEnabled(JNIEnv *, jclass, jlong nativeView) {
    View* view = (View*) nativeView;
    return view->isStencilBufferEnabled();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetStereoscopicOptions(JNIEnv *, jclass, jlong nativeView,
        jboolean enabled) {
    View* view = (View*) nativeView;
    View::StereoscopicOptions options {
        .enabled = (bool) enabled
    };
    view->setStereoscopicOptions(options);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetGuardBandOptions(JNIEnv *, jclass,
        jlong nativeView, jboolean enabled) {
    View* view = (View*) nativeView;
    view->setGuardBandOptions({ .enabled = (bool)enabled });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nSetMaterialGlobal(JNIEnv * , jclass, jlong nativeView,
        jint index, jfloat x, jfloat y, jfloat z, jfloat w) {
    View *view = (View *) nativeView;
    view->setMaterialGlobal((uint32_t)index, { x, y, z, w });
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nGetMaterialGlobal(JNIEnv *env, jclass clazz,
        jlong nativeView, jint index, jfloatArray out_) {
    jfloat* out = env->GetFloatArrayElements(out_, nullptr);
    View *view = (View *) nativeView;
    auto result = view->getMaterialGlobal(index);
    std::copy_n(result.v, 4, out);
    env->ReleaseFloatArrayElements(out_, out, 0);
}

extern "C"
JNIEXPORT int JNICALL
Java_com_google_android_filament_View_nGetFogEntity(JNIEnv *env, jclass clazz,
        jlong nativeView) {
    View *view = (View *) nativeView;
    return (jint)view->getFogEntity().getId();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_View_nClearFrameHistory(JNIEnv *env, jclass clazz,
        jlong nativeView, jlong nativeEngine) {
    View *view = (View *) nativeView;
    Engine *engine = (Engine *) nativeEngine;
    view->clearFrameHistory(*engine);
}
