/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_SKYBOX_H
#define TNT_FILAMENT_DETAILS_SKYBOX_H

#include "downcast.h"

#include <filament/Skybox.h>

#include <private/backend/DriverApi.h>

#include <utils/compiler.h>
#include <utils/Entity.h>

namespace filament {

class FEngine;
class FTexture;
class FMaterial;
class FMaterialInstance;
class FRenderableManager;

class FSkybox : public Skybox {
public:
    FSkybox(FEngine& engine, const Builder& builder) noexcept;

    static FMaterial const* createMaterial(FEngine& engine);

    void terminate(FEngine& engine) noexcept;

    utils::Entity getEntity() const noexcept { return mSkybox; }

    void setLayerMask(uint8_t select, uint8_t values) noexcept;
    uint8_t getLayerMask() const noexcept { return mLayerMask; }

    float getIntensity() const noexcept { return mIntensity; }

    FTexture const* getTexture() const noexcept { return mSkyboxTexture; }

    void setColor(math::float4 color) noexcept;

private:
    // we don't own these
    FTexture const* mSkyboxTexture = nullptr;

    // we own these
    FMaterialInstance* mSkyboxMaterialInstance = nullptr;
    utils::Entity mSkybox;
    FRenderableManager& mRenderableManager;
    float mIntensity = 0.0f;
    uint8_t mLayerMask = 0x1;
};

FILAMENT_DOWNCAST(Skybox)

} // namespace filament


#endif // TNT_FILAMENT_DETAILS_SKYBOX_H
