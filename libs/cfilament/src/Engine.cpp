/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <filament/Engine.h>

#include "API.h"

using namespace filament;
using namespace utils;

Engine *Filament_Engine_Create(driver::Backend backend) {
    return Engine::create(backend);
}

Engine *Filament_Engine_CreateShared(driver::Backend backend, void *sharedContext) {
    return Engine::create(backend, nullptr, sharedContext);
}

void Filament_Engine_DestroyEngine(Engine *engine) {
    Engine::destroy(&engine);
}

// SwapChain

SwapChain *Filament_Engine_CreateSwapChain(Engine *engine, void *surface, uint64_t flags) {
    return engine->createSwapChain(surface, flags);
}

void Filament_Engine_DestroySwapChain(Engine *engine, SwapChain *swapChain) {
    engine->destroy(swapChain);
}

// View

View *Filament_Engine_CreateView(Engine *engine) {
    return engine->createView();
}

void Filament_Engine_DestroyView(Engine *engine, View *view) {
    engine->destroy(view);
}

// Renderer

Renderer *Filament_Engine_CreateRenderer(Engine *engine) {
    return engine->createRenderer();
}

void Filament_Engine_DestroyRenderer(Engine *engine, Renderer *renderer) {
    engine->destroy(renderer);
}

// Camera

Camera *Filament_Engine_CreateCamera(Engine *engine) {
    return engine->createCamera();
}

Camera *Filament_Engine_CreateCameraWithEntity(Engine *engine, Entity entity) {
    return engine->createCamera(entity);
}

void Filament_Engine_DestroyCamera(Engine *engine, Camera *camera) {
    engine->destroy(camera);
}

// Scene

Scene *Filament_Engine_CreateScene(Engine *engine) {
    return engine->createScene();
}

void Filament_Engine_DestroyScene(Engine *engine, Scene *scene) {
    engine->destroy(scene);
}

// Fence

Fence *Filament_Engine_CreateFence(Engine *engine, Fence::Type fenceType) {
    return engine->createFence(fenceType);
}

void Filament_Engine_DestroyFence(Engine *engine, Fence *fence) {
    engine->destroy(fence);
}

// Stream

void Filament_Engine_DestroyStream(Engine *engine, Stream *stream) {
    engine->destroy(stream);
}

// Others...

void Filament_Engine_DestroyIndexBuffer(Engine *engine, IndexBuffer *indexBuffer) {
    engine->destroy(indexBuffer);
}

void Filament_Engine_DestroyVertexBuffer(Engine *engine, VertexBuffer *vertexBuffer) {
    engine->destroy(vertexBuffer);
}

void
Filament_Engine_DestroyIndirectLight(Engine *engine, IndirectLight *indirectLight) {
    engine->destroy(indirectLight);
}

void Filament_Engine_DestroyMaterial(Engine *engine, Material *material) {
    engine->destroy(material);
}

void Filament_Engine_DestroyMaterialInstance(Engine *engine, MaterialInstance *materialInstance) {
    engine->destroy(materialInstance);
}

void Filament_Engine_DestroySkybox(Engine *engine, Skybox *skybox) {
    engine->destroy(skybox);
}

void Filament_Engine_DestroyTexture(Engine *engine, Texture *texture) {
    engine->destroy(texture);
}

// Managers...

TransformManager *Filament_Engine_GetTransformManager(Engine *engine) {
    return &engine->getTransformManager();
}

LightManager *Filament_Engine_GetLightManager(Engine *engine) {
    return &engine->getLightManager();
}

RenderableManager *Filament_Engine_GetRenderableManager(Engine *engine) {
    return &engine->getRenderableManager();
}
