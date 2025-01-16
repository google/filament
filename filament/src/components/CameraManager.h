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

#ifndef TNT_FILAMENT_COMPONENTS_CAMERAMANAGER_H
#define TNT_FILAMENT_COMPONENTS_CAMERAMANAGER_H

#include "downcast.h"

#include <filament/FilamentAPI.h>

#include <utils/compiler.h>
#include <utils/SingleInstanceComponentManager.h>
#include <utils/Entity.h>

namespace filament {

class CameraManager : public FilamentAPI {
public:
    using Instance = utils::EntityInstance<CameraManager>;
};

class FEngine;
class FCamera;

class UTILS_PRIVATE FCameraManager : public CameraManager {
public:
    using Instance = Instance;

    explicit FCameraManager(FEngine& engine) noexcept;

    ~FCameraManager() noexcept;

    // free-up all resources
    void terminate(FEngine& engine) noexcept;

    void gc(FEngine& engine, utils::EntityManager& em) noexcept;

    /*
    * Component Manager APIs
    */

    bool hasComponent(utils::Entity const e) const noexcept {
        return mManager.hasComponent(e);
    }

    Instance getInstance(utils::Entity const e) const noexcept {
        return { mManager.getInstance(e) };
    }

    size_t getComponentCount() const noexcept {
        return mManager.getComponentCount();
    }

    bool empty() const noexcept {
        return mManager.empty();
    }

    utils::Entity getEntity(Instance const i) const noexcept {
        return mManager.getEntity(i);
    }

    utils::Entity const* getEntities() const noexcept {
        return mManager.getEntities();
    }

    FCamera* getCamera(Instance const i) noexcept {
        return mManager.elementAt<CAMERA>(i);
    }

    FCamera* create(FEngine& engine, utils::Entity entity);

    void destroy(FEngine& engine, utils::Entity e) noexcept;

private:

    enum {
        CAMERA,
        OWNS_TRANSFORM_COMPONENT
    };

    using Base = utils::SingleInstanceComponentManager<FCamera*, bool>;

    struct CameraManagerImpl : public Base {
        using Base::gc;
        using Base::swap;
        using Base::hasComponent;
    } mManager;
};

} // namespace filament

#endif // TNT_FILAMENT_COMPONENTS_CAMERAMANAGER_H
