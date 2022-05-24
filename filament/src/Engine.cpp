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

#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/Panic.h>

using namespace utils;

namespace filament {

using namespace math;
using namespace backend;

Engine* Engine::create(Backend backend, Platform* platform, void* sharedGLContext, const Config* config) {
    return FEngine::create(backend, platform, sharedGLContext, config);
}

void Engine::destroy(Engine* engine) {
    FEngine::destroy(upcast(engine));
}

#if UTILS_HAS_THREADING
void Engine::createAsync(Engine::CreateCallback callback, void* user, Backend backend,
        Platform* platform, void* sharedGLContext, const Config* config) {
    FEngine::createAsync(callback, user, backend, platform, sharedGLContext, config);
}

Engine* Engine::getEngine(void* token) {
    return FEngine::getEngine(token);
}
#endif

void Engine::destroy(Engine** pEngine) {
    if (pEngine) {
        Engine* engine = *pEngine;
        FEngine::destroy(upcast(engine));
        *pEngine = nullptr;
    }
}

// -----------------------------------------------------------------------------------------------
// Resource management
// -----------------------------------------------------------------------------------------------

const Material* Engine::getDefaultMaterial() const noexcept {
    return upcast(this)->getDefaultMaterial();
}

Backend Engine::getBackend() const noexcept {
    return upcast(this)->getBackend();
}

Platform* Engine::getPlatform() const noexcept {
    return upcast(this)->getPlatform();
}

Renderer* Engine::createRenderer() noexcept {
    return upcast(this)->createRenderer();
}

View* Engine::createView() noexcept {
    return upcast(this)->createView();
}

Scene* Engine::createScene() noexcept {
    return upcast(this)->createScene();
}

Camera* Engine::createCamera(Entity entity) noexcept {
    return upcast(this)->createCamera(entity);
}

Camera* Engine::getCameraComponent(utils::Entity entity) noexcept {
    return upcast(this)->getCameraComponent(entity);
}

void Engine::destroyCameraComponent(utils::Entity entity) noexcept {
    upcast(this)->destroyCameraComponent(entity);
}

Fence* Engine::createFence() noexcept {
    return upcast(this)->createFence(FFence::Type::SOFT);
}

SwapChain* Engine::createSwapChain(void* nativeWindow, uint64_t flags) noexcept {
    return upcast(this)->createSwapChain(nativeWindow, flags);
}

SwapChain* Engine::createSwapChain(uint32_t width, uint32_t height, uint64_t flags) noexcept {
    return upcast(this)->createSwapChain(width, height, flags);
}

bool Engine::destroy(const BufferObject* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const VertexBuffer* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const IndexBuffer* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const SkinningBuffer* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const MorphTargetBuffer* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const IndirectLight* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const Material* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const MaterialInstance* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const Renderer* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const View* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const Scene* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const Skybox* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const ColorGrading* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const Stream* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const Texture* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const RenderTarget* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const Fence* p) {
    return upcast(this)->destroy(upcast(p));
}

bool Engine::destroy(const SwapChain* p) {
    return upcast(this)->destroy(upcast(p));
}

void Engine::destroy(Entity e) {
    upcast(this)->destroy(e);
}

void Engine::flushAndWait() {
    upcast(this)->flushAndWait();
}

void Engine::flush() {
    upcast(this)->flush();
}

utils::EntityManager& Engine::getEntityManager() noexcept {
    return upcast(this)->getEntityManager();
}

RenderableManager& Engine::getRenderableManager() noexcept {
    return upcast(this)->getRenderableManager();
}

LightManager& Engine::getLightManager() noexcept {
    return upcast(this)->getLightManager();
}

TransformManager& Engine::getTransformManager() noexcept {
    return upcast(this)->getTransformManager();
}

void Engine::enableAccurateTranslations() noexcept  {
    getTransformManager().setAccurateTranslationsEnabled(true);
}

void* Engine::streamAlloc(size_t size, size_t alignment) noexcept {
    return upcast(this)->streamAlloc(size, alignment);
}

// The external-facing execute does a flush, and is meant only for single-threaded environments.
// It also discards the boolean return value, which would otherwise indicate a thread exit.
void Engine::execute() {
    ASSERT_PRECONDITION(!UTILS_HAS_THREADING, "Execute is meant for single-threaded platforms.");
    upcast(this)->flush();
    upcast(this)->execute();
}

utils::JobSystem& Engine::getJobSystem() noexcept {
    return upcast(this)->getJobSystem();
}

DebugRegistry& Engine::getDebugRegistry() noexcept {
    return upcast(this)->getDebugRegistry();
}

void Engine::pumpMessageQueues() {
    upcast(this)->pumpMessageQueues();
}

} // namespace filament
