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

#ifndef TNT_FILAMENT_CAMERAMANAGER_H
#define TNT_FILAMENT_CAMERAMANAGER_H

#include "upcast.h"

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
    using Instance = CameraManager::Instance;

    explicit FCameraManager(FEngine& engine) noexcept;

    ~FCameraManager() noexcept;

    // free-up all resources
    void terminate() noexcept;

    void gc(utils::EntityManager& em) noexcept;

    /*
    * Component Manager APIs
    */

    bool hasComponent(utils::Entity e) const noexcept {
        return mManager.hasComponent(e);
    }

    Instance getInstance(utils::Entity e) const noexcept {
        return Instance(mManager.getInstance(e));
    }

    FCamera* getCamera(Instance i) noexcept {
        return mManager.elementAt<CAMERA>(i);
    }

    FCamera* create(utils::Entity entity);

    void destroy(utils::Entity e) noexcept;

private:

    enum {
        CAMERA
    };

    using Base = utils::SingleInstanceComponentManager<FCamera *>;

    struct CameraManagerImpl : public Base {
        using Base::gc;
        using Base::swap;
        using Base::hasComponent;
    } mManager;

    FEngine& mEngine;
};

} // namespace filament

#endif // TNT_FILAMENT_CAMERAMANAGER_H
