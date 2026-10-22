/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "components/CameraManager.h"

#include "details/Camera.h"
#include "details/Engine.h"

#include <utils/debug.h>
#include <utils/Entity.h>
#include <utils/Logger.h>
#include <utils/PagedArenaBitsetPool.h>

using namespace utils;
using namespace filament::math;

namespace filament {

FCameraManager::FCameraManager(FEngine& engine) noexcept
        : mManager(engine.getEntityManager()) {
}

FCameraManager::~FCameraManager() noexcept = default;

void FCameraManager::terminate(FEngine& engine) noexcept {
    auto& manager = mManager;
    if (!manager.empty()) {
        DLOG(INFO) << "cleaning up " << manager.getComponentCount() << " leaked Camera components";
        while (!manager.empty()) {
            Instance const ci = manager.end() - 1;
            destroy(manager.getEntity(ci), engine);
        }
    }
}

void FCameraManager::gc(FEngine& engine) noexcept {
    mManager.gc(this, &FCameraManager::destroyComponents, engine);
}

FCamera* FCameraManager::create(FEngine& engine, Entity entity) {
    auto& manager = mManager;

    Entity zombie;
    if (UTILS_UNLIKELY(manager.popPendingZombie(entity, zombie))) {
        destroy(zombie, engine);
    }

    // if this entity already has Camera component, destroy it.
    if (UTILS_UNLIKELY(manager.hasComponent(entity))) {
        destroy(entity, engine);
    }

    // add the Camera component to the entity
    Instance const i = manager.addComponent(entity);

    // For historical reasons, FCamera must not move. So the CameraManager stores a pointer.
    FCamera* const camera = engine.getHeapAllocator().make<FCamera>(engine, entity);
    manager.elementAt<CAMERA>(i) = camera;
    manager.elementAt<OWNS_TRANSFORM_COMPONENT>(i) = false;

    // Make sure we have a transform component
    FTransformManager& tcm = engine.getTransformManager();
    if (!tcm.hasComponent(entity)) {
        tcm.create(entity);
        manager.elementAt<OWNS_TRANSFORM_COMPONENT>(i) = true;
    }
    return camera;
}

void FCameraManager::destroyComponents(Entity const* entities, size_t const count, FEngine& engine) noexcept {
    auto& manager = mManager;
    for (size_t k = 0; k < count; ++k) {
        Entity const e = entities[k];
        if (Instance const i = manager.getInstance(e) ; i) {
            // destroy the FCamera object
            bool const ownsTransformComponent = manager.elementAt<OWNS_TRANSFORM_COMPONENT>(i);

            { // scope for camera -- it's invalid after this scope.
                FCamera* const camera = manager.elementAt<CAMERA>(i);
                assert_invariant(camera);
                engine.getHeapAllocator().destroy(camera);

                // Remove the camera component
                manager.removeComponent(e);
            }

            // if we added the transform component, remove it.
            if (ownsTransformComponent) {
                engine.getTransformManager().destroy(e);
            }
        }
    }
}

} // namespace filament
