/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "FFilamentAsset.h"

using namespace filament;
using namespace utils;

namespace gltfio {

using namespace details;

size_t FilamentAsset::getEntityCount() const noexcept {
    return upcast(this)->getEntityCount();
}

const Entity* FilamentAsset::getEntities() const noexcept {
    return upcast(this)->getEntities();
}

const Entity* FilamentAsset::getLightEntities() const noexcept {
    return upcast(this)->getLightEntities();
}

size_t FilamentAsset::getLightEntityCount() const noexcept {
    return upcast(this)->getLightEntityCount();
}

Entity FilamentAsset::getRoot() const noexcept {
    return upcast(this)->getRoot();
}

Entity FilamentAsset::popRenderable() noexcept {
    Entity result[1];
    const bool empty = !popRenderables(result, 1);
    return empty ? Entity() : result[0];
}

size_t FilamentAsset::popRenderables(Entity* result, size_t count) noexcept {
    return upcast(this)->popRenderables(result, count);
}

size_t FilamentAsset::getMaterialInstanceCount() const noexcept {
    return upcast(this)->getMaterialInstanceCount();
}

const MaterialInstance* const* FilamentAsset::getMaterialInstances() const noexcept {
    return upcast(this)->getMaterialInstances();
}

MaterialInstance* const* FilamentAsset::getMaterialInstances() noexcept {
    return upcast(this)->getMaterialInstances();
}

size_t FilamentAsset::getResourceUriCount() const noexcept {
    return upcast(this)->getResourceUriCount();
}

const char* const* FilamentAsset::getResourceUris() const noexcept {
    return upcast(this)->getResourceUris();
}

filament::Aabb FilamentAsset::getBoundingBox() const noexcept {
    return upcast(this)->getBoundingBox();
}

const char* FilamentAsset::getName(utils::Entity entity) const noexcept {
    return upcast(this)->getName(entity);
}

Animator* FilamentAsset::getAnimator() noexcept {
    return upcast(this)->getAnimator();
}

utils::Entity FilamentAsset::getWireframe() noexcept {
    return upcast(this)->getWireframe();
}

Engine* FilamentAsset::getEngine() const noexcept {
    return upcast(this)->getEngine();
}

void FilamentAsset::releaseSourceData() noexcept {
    return upcast(this)->releaseSourceData();
}

const void* FilamentAsset::getSourceAsset() noexcept {
    return upcast(this)->getSourceAsset();
}

} // namespace gltfio
