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

#include "details/Engine.h"

#include "details/BufferObject.h"
#include "details/Camera.h"
#include "details/Fence.h"
#include "details/IndexBuffer.h"
#include "details/IndirectLight.h"
#include "details/Material.h"
#include "details/Renderer.h"
#include "details/Scene.h"
#include "details/SkinningBuffer.h"
#include "details/Skybox.h"
#include "details/Stream.h"
#include "details/SwapChain.h"
#include "details/Texture.h"
#include "details/VertexBuffer.h"
#include "details/View.h"

#include <filament/Engine.h>

#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/Panic.h>

#include <stddef.h>
#include <stdint.h>

using namespace utils;

namespace filament {

namespace backend {
class Platform;
}

using namespace math;
using namespace backend;

void Engine::destroy(Engine* engine) {
    FEngine::destroy(downcast(engine));
}

#if UTILS_HAS_THREADING
Engine* Engine::getEngine(void* token) {
    return FEngine::getEngine(token);
}
#endif

void Engine::destroy(Engine** pEngine) {
    if (pEngine) {
        Engine* engine = *pEngine;
        FEngine::destroy(downcast(engine));
        *pEngine = nullptr;
    }
}

// -----------------------------------------------------------------------------------------------
// Resource management
// -----------------------------------------------------------------------------------------------

const Material* Engine::getDefaultMaterial() const noexcept {
    return downcast(this)->getDefaultMaterial();
}

Backend Engine::getBackend() const noexcept {
    return downcast(this)->getBackend();
}

Platform* Engine::getPlatform() const noexcept {
    return downcast(this)->getPlatform();
}

Renderer* Engine::createRenderer() noexcept {
    return downcast(this)->createRenderer();
}

View* Engine::createView() noexcept {
    return downcast(this)->createView();
}

Scene* Engine::createScene() noexcept {
    return downcast(this)->createScene();
}

Camera* Engine::createCamera(Entity entity) noexcept {
    return downcast(this)->createCamera(entity);
}

Camera* Engine::getCameraComponent(utils::Entity entity) noexcept {
    return downcast(this)->getCameraComponent(entity);
}

void Engine::destroyCameraComponent(utils::Entity entity) noexcept {
    downcast(this)->destroyCameraComponent(entity);
}

Fence* Engine::createFence() noexcept {
    return downcast(this)->createFence();
}

SwapChain* Engine::createSwapChain(void* nativeWindow, uint64_t flags) noexcept {
    return downcast(this)->createSwapChain(nativeWindow, flags);
}

SwapChain* Engine::createSwapChain(uint32_t width, uint32_t height, uint64_t flags) noexcept {
    return downcast(this)->createSwapChain(width, height, flags);
}

bool Engine::destroy(const BufferObject* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const VertexBuffer* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const IndexBuffer* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const SkinningBuffer* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const MorphTargetBuffer* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const IndirectLight* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const Material* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const MaterialInstance* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const Renderer* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const View* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const Scene* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const Skybox* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const ColorGrading* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const Stream* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const Texture* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const RenderTarget* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const Fence* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const SwapChain* p) {
    return downcast(this)->destroy(downcast(p));
}

bool Engine::destroy(const InstanceBuffer* p) {
    return downcast(this)->destroy(downcast(p));
}

void Engine::destroy(Entity e) {
    downcast(this)->destroy(e);
}

bool Engine::isValid(const BufferObject* p) const {
    return downcast(this)->isValid(downcast(p));
}
bool Engine::isValid(const VertexBuffer* p) const {
    return downcast(this)->isValid(downcast(p));
}
bool Engine::isValid(const Fence* p) const {
    return downcast(this)->isValid(downcast(p));
}
bool Engine::isValid(const IndexBuffer* p) const {
    return downcast(this)->isValid(downcast(p));
}
bool Engine::isValid(const SkinningBuffer* p) const {
    return downcast(this)->isValid(downcast(p));
}
bool Engine::isValid(const MorphTargetBuffer* p) const {
    return downcast(this)->isValid(downcast(p));
}
bool Engine::isValid(const IndirectLight* p) const {
    return downcast(this)->isValid(downcast(p));
}
bool Engine::isValid(const Material* p) const {
    return downcast(this)->isValid(downcast(p));
}
bool Engine::isValid(const Material* m, const MaterialInstance* p) const {
    return downcast(this)->isValid(downcast(m), downcast(p));
}
bool Engine::isValidExpensive(const MaterialInstance* p) const {
    return downcast(this)->isValidExpensive(downcast(p));
}
bool Engine::isValid(const Renderer* p) const {
    return downcast(this)->isValid(downcast(p));
}
bool Engine::isValid(const Scene* p) const {
    return downcast(this)->isValid(downcast(p));
}
bool Engine::isValid(const Skybox* p) const {
    return downcast(this)->isValid(downcast(p));
}
bool Engine::isValid(const ColorGrading* p) const {
    return downcast(this)->isValid(downcast(p));
}
bool Engine::isValid(const SwapChain* p) const {
    return downcast(this)->isValid(downcast(p));
}
bool Engine::isValid(const Stream* p) const {
    return downcast(this)->isValid(downcast(p));
}
bool Engine::isValid(const Texture* p) const {
    return downcast(this)->isValid(downcast(p));
}
bool Engine::isValid(const RenderTarget* p) const {
    return downcast(this)->isValid(downcast(p));
}
bool Engine::isValid(const View* p) const {
    return downcast(this)->isValid(downcast(p));
}
bool Engine::isValid(const InstanceBuffer* p) const {
    return downcast(this)->isValid(downcast(p));
}

void Engine::flushAndWait() {
    downcast(this)->flushAndWait();
}

void Engine::flush() {
    downcast(this)->flush();
}

utils::EntityManager& Engine::getEntityManager() noexcept {
    return downcast(this)->getEntityManager();
}

RenderableManager& Engine::getRenderableManager() noexcept {
    return downcast(this)->getRenderableManager();
}

LightManager& Engine::getLightManager() noexcept {
    return downcast(this)->getLightManager();
}

TransformManager& Engine::getTransformManager() noexcept {
    return downcast(this)->getTransformManager();
}

void Engine::enableAccurateTranslations() noexcept  {
    getTransformManager().setAccurateTranslationsEnabled(true);
}

void* Engine::streamAlloc(size_t size, size_t alignment) noexcept {
    return downcast(this)->streamAlloc(size, alignment);
}

// The external-facing execute does a flush, and is meant only for single-threaded environments.
// It also discards the boolean return value, which would otherwise indicate a thread exit.
void Engine::execute() {
    FILAMENT_CHECK_PRECONDITION(!UTILS_HAS_THREADING)
            << "Execute is meant for single-threaded platforms.";
    downcast(this)->flush();
    downcast(this)->execute();
}

utils::JobSystem& Engine::getJobSystem() noexcept {
    return downcast(this)->getJobSystem();
}

bool Engine::isPaused() const noexcept {
    FILAMENT_CHECK_PRECONDITION(UTILS_HAS_THREADING)
            << "Pause is meant for multi-threaded platforms.";
    return downcast(this)->isPaused();
}

void Engine::setPaused(bool paused) {
    FILAMENT_CHECK_PRECONDITION(UTILS_HAS_THREADING)
            << "Pause is meant for multi-threaded platforms.";
    downcast(this)->setPaused(paused);
}

DebugRegistry& Engine::getDebugRegistry() noexcept {
    return downcast(this)->getDebugRegistry();
}

void Engine::pumpMessageQueues() {
    downcast(this)->pumpMessageQueues();
}

void Engine::setAutomaticInstancingEnabled(bool enable) noexcept {
    downcast(this)->setAutomaticInstancingEnabled(enable);
}

bool Engine::isAutomaticInstancingEnabled() const noexcept {
    return downcast(this)->isAutomaticInstancingEnabled();
}

FeatureLevel Engine::getSupportedFeatureLevel() const noexcept {
    return downcast(this)->getSupportedFeatureLevel();
}

FeatureLevel Engine::setActiveFeatureLevel(FeatureLevel featureLevel) {
    return downcast(this)->setActiveFeatureLevel(featureLevel);
}

FeatureLevel Engine::getActiveFeatureLevel() const noexcept {
    return downcast(this)->getActiveFeatureLevel();
}

size_t Engine::getMaxAutomaticInstances() const noexcept {
    return downcast(this)->getMaxAutomaticInstances();
}

const Engine::Config& Engine::getConfig() const noexcept {
    return downcast(this)->getConfig();
}

bool Engine::isStereoSupported(StereoscopicType stereoscopicType) const noexcept {
    return downcast(this)->isStereoSupported();
}

size_t Engine::getMaxStereoscopicEyes() noexcept {
    return FEngine::getMaxStereoscopicEyes();
}

#if defined(__EMSCRIPTEN__)
void Engine::resetBackendState() noexcept {
    downcast(this)->resetBackendState();
}
#endif

} // namespace filament
