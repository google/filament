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

#include "details/Scene.h"

#include "details/IndirectLight.h"
#include "details/Skybox.h"

using namespace utils;

namespace filament {

void Scene::setSkybox(Skybox* skybox) noexcept {
    upcast(this)->setSkybox(upcast(skybox));
}

Skybox* Scene::getSkybox() noexcept {
    return upcast(this)->getSkybox();
}

Skybox const* Scene::getSkybox() const noexcept {
    return upcast(this)->getSkybox();
}

void Scene::setIndirectLight(IndirectLight const* ibl) noexcept {
    upcast(this)->setIndirectLight(upcast(ibl));
}

void Scene::addEntity(Entity entity) {
    upcast(this)->addEntity(entity);
}

void Scene::addEntities(const Entity* entities, size_t count) {
    upcast(this)->addEntities(entities, count);
}

void Scene::remove(Entity entity) {
    upcast(this)->remove(entity);
}

void Scene::removeEntities(const Entity* entities, size_t count) {
    upcast(this)->removeEntities(entities, count);
}

size_t Scene::getRenderableCount() const noexcept {
    return upcast(this)->getRenderableCount();
}

size_t Scene::getLightCount() const noexcept {
    return upcast(this)->getLightCount();
}

bool Scene::hasEntity(Entity entity) const noexcept {
    return upcast(this)->hasEntity(entity);
}

void Scene::forEach(Invocable<void(utils::Entity)>&& functor) const noexcept {
    upcast(this)->forEach(std::move(functor));
}

} // namespace filament
