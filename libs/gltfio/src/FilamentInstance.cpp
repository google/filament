/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include "FFilamentInstance.h"
#include "FFilamentAsset.h"

#include <gltfio/Animator.h>

#include <utils/Log.h>

using namespace filament;
using namespace utils;

namespace gltfio {

Animator* FFilamentInstance::getAnimator() const noexcept {
    assert_invariant(animator);
    return animator;
}

void FFilamentInstance::createAnimator() {
    assert_invariant(animator == nullptr);
    animator = new Animator(owner, this);
}

size_t FFilamentInstance::getSkinCount() const noexcept {
    return skins.size();
}

const char* FFilamentInstance::getSkinNameAt(size_t skinIndex) const noexcept {
    if (skins.size() <= skinIndex) {
        return nullptr;
    }
    return skins[skinIndex].name.c_str();
}

size_t FFilamentInstance::getJointCountAt(size_t skinIndex) const noexcept {
    if (skins.size() <= skinIndex) {
        return 0;
    }
    return skins[skinIndex].joints.size();
}

const utils::Entity* FFilamentInstance::getJointsAt(size_t skinIndex) const noexcept {
    if (skins.size() <= skinIndex) {
        return nullptr;
    }
    return skins[skinIndex].joints.data();
}

void FFilamentInstance::applyMaterialVariant(size_t variantIndex) noexcept {
    if (variantIndex >= variants.size()) {
        return;
    }
    const auto& mappings = variants[variantIndex].mappings;
    RenderableManager& rm = owner->mEngine->getRenderableManager();
    for (const auto& mapping : mappings) {
        auto renderable = rm.getInstance(mapping.renderable);
        rm.setMaterialInstanceAt(renderable, mapping.primitiveIndex, mapping.material);
    }
}

FilamentAsset* FilamentInstance::getAsset() const noexcept {
    return upcast(this)->owner;
}

size_t FilamentInstance::getEntityCount() const noexcept {
    return upcast(this)->entities.size();
}

const Entity* FilamentInstance::getEntities() const noexcept {
    const auto& entities = upcast(this)->entities;
    return entities.empty() ? nullptr : entities.data();
}

Entity FilamentInstance::getRoot() const noexcept {
    return upcast(this)->root;
}

void FilamentInstance::applyMaterialVariant(size_t variantIndex) noexcept {
    return upcast(this)->applyMaterialVariant(variantIndex);
}

Animator* FilamentInstance::getAnimator() noexcept {
    return upcast(this)->getAnimator();
}

} // namespace gltfio
