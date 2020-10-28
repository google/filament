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

Animator* FFilamentInstance::getAnimator() noexcept {
    if (!animator) {
        if (!owner->mResourcesLoaded) {
            slog.e << "Cannot create animator before resource loading." << io::endl;
            return nullptr;
        }
        if (owner->mIsReleased) {
            slog.e << "Cannot create animator from frozen asset." << io::endl;
            return nullptr;
        }
        animator = new Animator(owner, this);
    }
    return animator;
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

Animator* FilamentInstance::getAnimator() noexcept {
    return upcast(this)->getAnimator();
}

} // namespace gltfio
