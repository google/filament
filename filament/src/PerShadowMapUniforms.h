/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef TNT_FILAMENT_PERSHADOWMAPUNIFORMS_H
#define TNT_FILAMENT_PERSHADOWMAPUNIFORMS_H

#include <filament/Viewport.h>

#include <private/filament/UibStructs.h>
#include <private/backend/SamplerGroup.h>

#include "TypedUniformBuffer.h"

#include <backend/Handle.h>

#include <utils/EntityInstance.h>

#include <random>

namespace filament {

struct CameraInfo;

class FEngine;
class LightManager;

/*
 * PerShadowMapUniforms manages the UBO needed to generate our shadow maps. Internally it just
 * holds onto a `PerViewUniform` UBO handle, but doesn't keep any shadow copy of it, instead it
 * writes the data directly into the commandstream, for this reason partial update of the data
 * is not possible.
 */
class PerShadowMapUniforms {

    using LightManagerInstance = utils::EntityInstance<LightManager>;

public:
    class Transaction {
        friend PerShadowMapUniforms;
        PerViewUib* uniforms = nullptr;
        Transaction() = default; // disallow creation by the caller
    };

    explicit PerShadowMapUniforms(FEngine& engine) noexcept;

    void terminate(backend::DriverApi& driver);

    static void prepareCamera(Transaction const& transaction,
            FEngine& engine, const CameraInfo& camera) noexcept;

    static void prepareLodBias(Transaction const& transaction,
            float bias) noexcept;

    static void prepareViewport(Transaction const& transaction,
            backend::Viewport const& viewport) noexcept;

    static void prepareTime(Transaction const& transaction,
            FEngine& engine, math::float4 const& userTime) noexcept;

    static void prepareShadowMapping(Transaction const& transaction,
            bool highPrecision) noexcept;

    static Transaction open(backend::DriverApi& driver) noexcept;

    // update local data into GPU UBO
    void commit(Transaction& transaction, backend::DriverApi& driver) noexcept;

    // bind this UBO
    void bind(backend::DriverApi& driver) noexcept;

private:
    static PerViewUib& edit(Transaction const& transaction) noexcept;
    backend::Handle<backend::HwBufferObject> mUniformBufferHandle;
};

} // namespace filament

#endif //TNT_FILAMENT_PERSHADOWMAPUNIFORMS_H
