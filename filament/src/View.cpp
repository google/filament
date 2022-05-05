/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "details/View.h"

namespace filament {

void View::setScene(Scene* scene) {
    return upcast(this)->setScene(upcast(scene));
}

Scene* View::getScene() noexcept {
    return upcast(this)->getScene();
}


void View::setCamera(Camera* camera) noexcept {
    upcast(this)->setCameraUser(upcast(camera));
}

Camera& View::getCamera() noexcept {
    return upcast(this)->getCameraUser();
}

void View::setViewport(filament::Viewport const& viewport) noexcept {
    upcast(this)->setViewport(viewport);
}

filament::Viewport const& View::getViewport() const noexcept {
    return upcast(this)->getViewport();
}

void View::setFrustumCullingEnabled(bool culling) noexcept {
    upcast(this)->setFrustumCullingEnabled(culling);
}

bool View::isFrustumCullingEnabled() const noexcept {
    return upcast(this)->isFrustumCullingEnabled();
}

void View::setDebugCamera(Camera* camera) noexcept {
    upcast(this)->setViewingCamera(upcast(camera));
}

void View::setVisibleLayers(uint8_t select, uint8_t values) noexcept {
    upcast(this)->setVisibleLayers(select, values);
}

void View::setName(const char* name) noexcept {
    upcast(this)->setName(name);
}

const char* View::getName() const noexcept {
    return upcast(this)->getName();
}

Camera const* View::getDirectionalLightCamera() const noexcept {
    return upcast(this)->getDirectionalLightCamera();
}

void View::setShadowingEnabled(bool enabled) noexcept {
    upcast(this)->setShadowingEnabled(enabled);
}

void View::setRenderTarget(RenderTarget* renderTarget) noexcept {
    upcast(this)->setRenderTarget(upcast(renderTarget));
}

RenderTarget* View::getRenderTarget() const noexcept {
    return upcast(this)->getRenderTarget();
}

void View::setSampleCount(uint8_t count) noexcept {
    upcast(this)->setSampleCount(count);
}

uint8_t View::getSampleCount() const noexcept {
    return upcast(this)->getSampleCount();
}

void View::setAntiAliasing(AntiAliasing type) noexcept {
    upcast(this)->setAntiAliasing(type);
}

View::AntiAliasing View::getAntiAliasing() const noexcept {
    return upcast(this)->getAntiAliasing();
}

void View::setTemporalAntiAliasingOptions(TemporalAntiAliasingOptions options) noexcept {
    upcast(this)->setTemporalAntiAliasingOptions(options);
}

const View::TemporalAntiAliasingOptions& View::getTemporalAntiAliasingOptions() const noexcept {
    return upcast(this)->getTemporalAntiAliasingOptions();
}

void View::setMultiSampleAntiAliasingOptions(MultiSampleAntiAliasingOptions options) noexcept {
    upcast(this)->setMultiSampleAntiAliasingOptions(options);
}

const View::MultiSampleAntiAliasingOptions& View::getMultiSampleAntiAliasingOptions() const noexcept {
    return upcast(this)->getMultiSampleAntiAliasingOptions();
}

void View::setScreenSpaceReflectionsOptions(ScreenSpaceReflectionsOptions options) noexcept {
    upcast(this)->setScreenSpaceReflectionsOptions(options);
}

const View::ScreenSpaceReflectionsOptions& View::getScreenSpaceReflectionsOptions() const noexcept {
    return upcast(this)->getScreenSpaceReflectionsOptions();
}

void View::setGuardBandOptions(GuardBandOptions options) noexcept {
    upcast(this)->setGuardBandOptions(options);
}

GuardBandOptions const& View::getGuardBandOptions() const noexcept {
    return upcast(this)->getGuardBandOptions();
}

void View::setColorGrading(ColorGrading* colorGrading) noexcept {
    return upcast(this)->setColorGrading(upcast(colorGrading));
}

const ColorGrading* View::getColorGrading() const noexcept {
    return upcast(this)->getColorGrading();
}

void View::setDithering(Dithering dithering) noexcept {
    upcast(this)->setDithering(dithering);
}

View::Dithering View::getDithering() const noexcept {
    return upcast(this)->getDithering();
}

void View::setDynamicResolutionOptions(const DynamicResolutionOptions& options) noexcept {
    upcast(this)->setDynamicResolutionOptions(options);
}

View::DynamicResolutionOptions View::getDynamicResolutionOptions() const noexcept {
    return upcast(this)->getDynamicResolutionOptions();
}

void View::setRenderQuality(const RenderQuality& renderQuality) noexcept {
    upcast(this)->setRenderQuality(renderQuality);
}

View::RenderQuality View::getRenderQuality() const noexcept {
    return upcast(this)->getRenderQuality();
}

void View::setPostProcessingEnabled(bool enabled) noexcept {
    upcast(this)->setPostProcessingEnabled(enabled);
}

bool View::isPostProcessingEnabled() const noexcept {
    return upcast(this)->hasPostProcessPass();
}

void View::setFrontFaceWindingInverted(bool inverted) noexcept {
    upcast(this)->setFrontFaceWindingInverted(inverted);
}

bool View::isFrontFaceWindingInverted() const noexcept {
    return upcast(this)->isFrontFaceWindingInverted();
}

void View::setDynamicLightingOptions(float zLightNear, float zLightFar) noexcept {
    upcast(this)->setDynamicLightingOptions(zLightNear, zLightFar);
}

void View::setShadowType(View::ShadowType shadow) noexcept {
    upcast(this)->setShadowType(shadow);
}

void View::setVsmShadowOptions(VsmShadowOptions const& options) noexcept {
    upcast(this)->setVsmShadowOptions(options);
}

View::VsmShadowOptions View::getVsmShadowOptions() const noexcept {
    return upcast(this)->getVsmShadowOptions();
}

void View::setSoftShadowOptions(SoftShadowOptions const& options) noexcept {
    upcast(this)->setSoftShadowOptions(options);
}

SoftShadowOptions View::getSoftShadowOptions() const noexcept {
    return upcast(this)->getSoftShadowOptions();
}

void View::setAmbientOcclusion(View::AmbientOcclusion ambientOcclusion) noexcept {
    upcast(this)->setAmbientOcclusion(ambientOcclusion);
}

View::AmbientOcclusion View::getAmbientOcclusion() const noexcept {
    return upcast(this)->getAmbientOcclusion();
}

void View::setAmbientOcclusionOptions(View::AmbientOcclusionOptions const& options) noexcept {
    upcast(this)->setAmbientOcclusionOptions(options);
}

View::AmbientOcclusionOptions const& View::getAmbientOcclusionOptions() const noexcept {
    return upcast(this)->getAmbientOcclusionOptions();
}

void View::setBloomOptions(View::BloomOptions options) noexcept {
    upcast(this)->setBloomOptions(options);
}

View::BloomOptions View::getBloomOptions() const noexcept {
    return upcast(this)->getBloomOptions();
}

void View::setFogOptions(View::FogOptions options) noexcept {
    upcast(this)->setFogOptions(options);
}

View::FogOptions View::getFogOptions() const noexcept {
    return upcast(this)->getFogOptions();
}

void View::setDepthOfFieldOptions(DepthOfFieldOptions options) noexcept {
    upcast(this)->setDepthOfFieldOptions(options);
}

View::DepthOfFieldOptions View::getDepthOfFieldOptions() const noexcept {
    return upcast(this)->getDepthOfFieldOptions();
}

void View::setVignetteOptions(View::VignetteOptions options) noexcept {
    upcast(this)->setVignetteOptions(options);
}

View::VignetteOptions View::getVignetteOptions() const noexcept {
    return upcast(this)->getVignetteOptions();
}

void View::setBlendMode(BlendMode blendMode) noexcept {
    upcast(this)->setBlendMode(blendMode);
}

View::BlendMode View::getBlendMode() const noexcept {
    return upcast(this)->getBlendMode();
}

uint8_t View::getVisibleLayers() const noexcept {
  return upcast(this)->getVisibleLayers();
}

bool View::isShadowingEnabled() const noexcept {
    return upcast(this)->isShadowingEnabled();
}

void View::setScreenSpaceRefractionEnabled(bool enabled) noexcept {
    upcast(this)->setScreenSpaceRefractionEnabled(enabled);
}

bool View::isScreenSpaceRefractionEnabled() const noexcept {
    return upcast(this)->isScreenSpaceRefractionEnabled();
}

View::PickingQuery& View::pick(uint32_t x, uint32_t y, backend::CallbackHandler* handler,
        View::PickingQueryResultCallback callback) noexcept {
    return upcast(this)->pick(x, y, handler, callback);
}

} // namespace filament
