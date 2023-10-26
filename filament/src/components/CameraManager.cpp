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

#include "details/Engine.h"
#include "details/Camera.h"

#include <utils/Entity.h>
#include <utils/Log.h>
#include <utils/debug.h>

using namespace utils;
using namespace filament::math;

namespace filament {

FCameraManager::FCameraManager(FEngine&) noexcept {
}

FCameraManager::~FCameraManager() noexcept = default;

void FCameraManager::terminate(FEngine& engine) noexcept {
    auto& manager = mManager;
    if (!manager.empty()) {
#ifndef NDEBUG
        slog.d << "cleaning up " << manager.getComponentCount()
               << " leaked Camera components" << io::endl;
#endif
        utils::Slice<Entity> const entities{ manager.getEntities(), manager.getComponentCount() };
        for (Entity const e : entities) {
            destroy(engine, e);
        }
    }
}

void FCameraManager::gc(FEngine& engine, utils::EntityManager& em) noexcept {
    auto& manager = mManager;
    manager.gc(em, [this, &engine](Entity e) {
        destroy(engine, e);
    });
}

FCamera* FCameraManager::create(FEngine& engine, Entity entity) {
    auto& manager = mManager;

    // if this entity already has Camera component, destroy it.
    if (UTILS_UNLIKELY(manager.hasComponent(entity))) {
        destroy(engine, entity);
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

void FCameraManager::destroy(FEngine& engine, Entity e) noexcept {
    auto& manager = mManager;
    if (Instance const i = manager.getInstance(e) ; i) {
        // destroy the FCamera object
        bool const ownsTransformComponent = manager.elementAt<OWNS_TRANSFORM_COMPONENT>(i);

        { // scope for camera -- it's invalid after this scope.
            FCamera* const camera = manager.elementAt<CAMERA>(i);
            assert_invariant(camera);
            camera->terminate(engine);
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

} // namespace filament
