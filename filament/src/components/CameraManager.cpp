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

#include <math/mat4.h>

using namespace utils;
using namespace filament::math;

namespace filament {

FCameraManager::FCameraManager(FEngine& engine) noexcept
        : mEngine(engine) {
}

FCameraManager::~FCameraManager() noexcept {
}

void FCameraManager::terminate() noexcept {
    auto& manager = mManager;
    if (!manager.empty()) {
#ifndef NDEBUG
        slog.d << "cleaning up " << manager.getComponentCount()
               << " leaked Camera components" << io::endl;
#endif
        while (!manager.empty()) {
            Instance ci = manager.end() - 1;
            destroy(manager.getEntity(ci));
        }
    }
}

void FCameraManager::gc(utils::EntityManager& em) noexcept {
    auto& manager = mManager;
    manager.gc(em, 4, [this](Entity e) {
        destroy(e);
    });
}

FCamera* FCameraManager::create(Entity entity) {
    FEngine& engine = mEngine;
    auto& manager = mManager;

    if (UTILS_UNLIKELY(manager.hasComponent(entity))) {
        destroy(entity);
    }
    Instance i = manager.addComponent(entity);

    FCamera* camera = engine.getHeapAllocator().make<FCamera>(engine, entity);
    manager.elementAt<CAMERA>(i) = camera;

    // Make sure we have a transform component
    FTransformManager& transformManager = engine.getTransformManager();
    if (!transformManager.hasComponent(entity)) {
        transformManager.create(entity);
    }
    return camera;
}

void FCameraManager::destroy(Entity e) noexcept {
    auto& manager = mManager;
    Instance i = manager.getInstance(e);
    if (i) {
        FCamera* camera = manager.elementAt<CAMERA>(i);
        assert(camera);
        camera->terminate(mEngine);
        mEngine.getHeapAllocator().destroy(camera);
        manager.removeComponent(e);
    }
}

} // namespace filament
