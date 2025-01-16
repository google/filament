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
#include "filament/View.h"


namespace filament {

void View::setScene(Scene* scene) {
    return downcast(this)->setScene(downcast(scene));
}

Scene* View::getScene() noexcept {
    return downcast(this)->getScene();
}


void View::setCamera(Camera* camera) noexcept {
    downcast(this)->setCameraUser(downcast(camera));
}

bool View::hasCamera() const noexcept {
    return downcast(this)->hasCamera();
}

Camera& View::getCamera() noexcept {
    return downcast(this)->getCameraUser();
}

void View::setViewport(Viewport const& viewport) noexcept {
    downcast(this)->setViewport(viewport);
}

Viewport const& View::getViewport() const noexcept {
    return downcast(this)->getViewport();
}

void View::setFrustumCullingEnabled(bool const culling) noexcept {
    downcast(this)->setFrustumCullingEnabled(culling);
}

bool View::isFrustumCullingEnabled() const noexcept {
    return downcast(this)->isFrustumCullingEnabled();
}

void View::setDebugCamera(Camera* camera) noexcept {
    downcast(this)->setViewingCamera(downcast(camera));
}

void View::setVisibleLayers(uint8_t const select, uint8_t const values) noexcept {
    downcast(this)->setVisibleLayers(select, values);
}

void View::setName(const char* name) noexcept {
    downcast(this)->setName(name);
}

const char* View::getName() const noexcept {
    return downcast(this)->getName();
}

utils::FixedCapacityVector<Camera const*> View::getDirectionalShadowCameras() const noexcept {
    return downcast(this)->getDirectionalShadowCameras();
}

void View::setShadowingEnabled(bool const enabled) noexcept {
    downcast(this)->setShadowingEnabled(enabled);
}

void View::setRenderTarget(RenderTarget* renderTarget) noexcept {
    downcast(this)->setRenderTarget(downcast(renderTarget));
}

RenderTarget* View::getRenderTarget() const noexcept {
    return downcast(this)->getRenderTarget();
}

void View::setSampleCount(uint8_t const count) noexcept {
    downcast(this)->setSampleCount(count);
}

uint8_t View::getSampleCount() const noexcept {
    return downcast(this)->getSampleCount();
}

void View::setAntiAliasing(AntiAliasing const type) noexcept {
    downcast(this)->setAntiAliasing(type);
}

View::AntiAliasing View::getAntiAliasing() const noexcept {
    return downcast(this)->getAntiAliasing();
}

void View::setTemporalAntiAliasingOptions(TemporalAntiAliasingOptions options) noexcept {
    downcast(this)->setTemporalAntiAliasingOptions(options);
}

const View::TemporalAntiAliasingOptions& View::getTemporalAntiAliasingOptions() const noexcept {
    return downcast(this)->getTemporalAntiAliasingOptions();
}

void View::setMultiSampleAntiAliasingOptions(MultiSampleAntiAliasingOptions const options) noexcept {
    downcast(this)->setMultiSampleAntiAliasingOptions(options);
}

const View::MultiSampleAntiAliasingOptions& View::getMultiSampleAntiAliasingOptions() const noexcept {
    return downcast(this)->getMultiSampleAntiAliasingOptions();
}

void View::setScreenSpaceReflectionsOptions(ScreenSpaceReflectionsOptions options) noexcept {
    downcast(this)->setScreenSpaceReflectionsOptions(options);
}

const View::ScreenSpaceReflectionsOptions& View::getScreenSpaceReflectionsOptions() const noexcept {
    return downcast(this)->getScreenSpaceReflectionsOptions();
}

void View::setGuardBandOptions(GuardBandOptions const options) noexcept {
    downcast(this)->setGuardBandOptions(options);
}

GuardBandOptions const& View::getGuardBandOptions() const noexcept {
    return downcast(this)->getGuardBandOptions();
}

void View::setColorGrading(ColorGrading* colorGrading) noexcept {
    return downcast(this)->setColorGrading(downcast(colorGrading));
}

const ColorGrading* View::getColorGrading() const noexcept {
    return downcast(this)->getColorGrading();
}

void View::setDithering(Dithering const dithering) noexcept {
    downcast(this)->setDithering(dithering);
}

View::Dithering View::getDithering() const noexcept {
    return downcast(this)->getDithering();
}

void View::setDynamicResolutionOptions(const DynamicResolutionOptions& options) noexcept {
    downcast(this)->setDynamicResolutionOptions(options);
}

View::DynamicResolutionOptions View::getDynamicResolutionOptions() const noexcept {
    return downcast(this)->getDynamicResolutionOptions();
}

void View::setRenderQuality(const RenderQuality& renderQuality) noexcept {
    downcast(this)->setRenderQuality(renderQuality);
}

View::RenderQuality View::getRenderQuality() const noexcept {
    return downcast(this)->getRenderQuality();
}

void View::setPostProcessingEnabled(bool const enabled) noexcept {
    downcast(this)->setPostProcessingEnabled(enabled);
}

bool View::isPostProcessingEnabled() const noexcept {
    return downcast(this)->hasPostProcessPass();
}

void View::setFrontFaceWindingInverted(bool const inverted) noexcept {
    downcast(this)->setFrontFaceWindingInverted(inverted);
}

bool View::isFrontFaceWindingInverted() const noexcept {
    return downcast(this)->isFrontFaceWindingInverted();
}

void View::setTransparentPickingEnabled(bool const enabled) noexcept {
    downcast(this)->setTransparentPickingEnabled(enabled);
}

bool View::isTransparentPickingEnabled() const noexcept {
    return downcast(this)->isTransparentPickingEnabled();
}

void View::setDynamicLightingOptions(float const zLightNear, float const zLightFar) noexcept {
    downcast(this)->setDynamicLightingOptions(zLightNear, zLightFar);
}

void View::setShadowType(ShadowType const shadow) noexcept {
    downcast(this)->setShadowType(shadow);
}

View::ShadowType View::getShadowType() const noexcept {
    return downcast(this)->getShadowType();
}

void View::setVsmShadowOptions(VsmShadowOptions const& options) noexcept {
    downcast(this)->setVsmShadowOptions(options);
}

View::VsmShadowOptions View::getVsmShadowOptions() const noexcept {
    return downcast(this)->getVsmShadowOptions();
}

void View::setSoftShadowOptions(SoftShadowOptions const& options) noexcept {
    downcast(this)->setSoftShadowOptions(options);
}

SoftShadowOptions View::getSoftShadowOptions() const noexcept {
    return downcast(this)->getSoftShadowOptions();
}

void View::setAmbientOcclusion(AmbientOcclusion const ambientOcclusion) noexcept {
    downcast(this)->setAmbientOcclusion(ambientOcclusion);
}

View::AmbientOcclusion View::getAmbientOcclusion() const noexcept {
    return downcast(this)->getAmbientOcclusion();
}

void View::setAmbientOcclusionOptions(AmbientOcclusionOptions const& options) noexcept {
    downcast(this)->setAmbientOcclusionOptions(options);
}

View::AmbientOcclusionOptions const& View::getAmbientOcclusionOptions() const noexcept {
    return downcast(this)->getAmbientOcclusionOptions();
}

void View::setBloomOptions(BloomOptions options) noexcept {
    downcast(this)->setBloomOptions(options);
}

View::BloomOptions View::getBloomOptions() const noexcept {
    return downcast(this)->getBloomOptions();
}

void View::setFogOptions(FogOptions options) noexcept {
    downcast(this)->setFogOptions(options);
}

View::FogOptions View::getFogOptions() const noexcept {
    return downcast(this)->getFogOptions();
}

void View::setDepthOfFieldOptions(DepthOfFieldOptions options) noexcept {
    downcast(this)->setDepthOfFieldOptions(options);
}

View::DepthOfFieldOptions View::getDepthOfFieldOptions() const noexcept {
    return downcast(this)->getDepthOfFieldOptions();
}

void View::setVignetteOptions(VignetteOptions options) noexcept {
    downcast(this)->setVignetteOptions(options);
}

View::VignetteOptions View::getVignetteOptions() const noexcept {
    return downcast(this)->getVignetteOptions();
}

void View::setBlendMode(BlendMode const blendMode) noexcept {
    downcast(this)->setBlendMode(blendMode);
}

View::BlendMode View::getBlendMode() const noexcept {
    return downcast(this)->getBlendMode();
}

uint8_t View::getVisibleLayers() const noexcept {
  return downcast(this)->getVisibleLayers();
}

bool View::isShadowingEnabled() const noexcept {
    return downcast(this)->isShadowingEnabled();
}

void View::setScreenSpaceRefractionEnabled(bool const enabled) noexcept {
    downcast(this)->setScreenSpaceRefractionEnabled(enabled);
}

bool View::isScreenSpaceRefractionEnabled() const noexcept {
    return downcast(this)->isScreenSpaceRefractionEnabled();
}

void View::setStencilBufferEnabled(bool const enabled) noexcept {
    downcast(this)->setStencilBufferEnabled(enabled);
}

bool View::isStencilBufferEnabled() const noexcept {
    return downcast(this)->isStencilBufferEnabled();
}

void View::setStereoscopicOptions(const StereoscopicOptions& options) noexcept {
    return downcast(this)->setStereoscopicOptions(options);
}

const View::StereoscopicOptions& View::getStereoscopicOptions() const noexcept {
    return downcast(this)->getStereoscopicOptions();
}

View::PickingQuery& View::pick(uint32_t const x, uint32_t const y, backend::CallbackHandler* handler,
        PickingQueryResultCallback const callback) noexcept {
    return downcast(this)->pick(x, y, handler, callback);
}

void View::setMaterialGlobal(uint32_t const index, math::float4 const& value) {
    downcast(this)->setMaterialGlobal(index, value);
}

math::float4 View::getMaterialGlobal(uint32_t const index) const {
    return downcast(this)->getMaterialGlobal(index);
}

utils::Entity View::getFogEntity() const noexcept {
    return downcast(this)->getFogEntity();
}

void View::clearFrameHistory(Engine& engine) noexcept {
    downcast(this)->clearFrameHistory(downcast(engine));
}

} // namespace filament
