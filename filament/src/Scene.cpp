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
    downcast(this)->setSkybox(downcast(skybox));
}

Skybox* Scene::getSkybox() const noexcept {
    return downcast(this)->getSkybox();
}

void Scene::setIndirectLight(IndirectLight* ibl) noexcept {
    downcast(this)->setIndirectLight(downcast(ibl));
}

IndirectLight* Scene::getIndirectLight() const noexcept {
    return downcast(this)->getIndirectLight();
}

void Scene::addEntity(Entity const entity) {
    downcast(this)->addEntity(entity);
}

void Scene::addEntities(const Entity* entities, size_t const count) {
    downcast(this)->addEntities(entities, count);
}

void Scene::remove(Entity const entity) {
    downcast(this)->remove(entity);
}

void Scene::removeEntities(const Entity* entities, size_t const count) {
    downcast(this)->removeEntities(entities, count);
}

void Scene::removeAllEntities() noexcept {
    downcast(this)->removeAllEntities();
}

size_t Scene::getEntityCount() const noexcept {
    return downcast(this)->getEntityCount();
}

size_t Scene::getRenderableCount() const noexcept {
    return downcast(this)->getRenderableCount();
}

size_t Scene::getLightCount() const noexcept {
    return downcast(this)->getLightCount();
}

bool Scene::hasEntity(Entity const entity) const noexcept {
    return downcast(this)->hasEntity(entity);
}

void Scene::forEach(Invocable<void(Entity)>&& functor) const noexcept {
    downcast(this)->forEach(std::move(functor));
}

} // namespace filament
