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

#ifndef TNT_FILAMENT_SHADOWMAPDESCRIPTORSET_H
#define TNT_FILAMENT_SHADOWMAPDESCRIPTORSET_H

#include "DescriptorSet.h"

#include "DescriptorSetLayout.h"

#include "private/filament/UibStructs.h"

#include <backend/DriverApiForward.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <math/vec4.h>

namespace filament {

struct CameraInfo;

class FEngine;
class LightManager;

/*
 * PerShadowMapUniforms manages the UBO needed to generate our shadow maps. Internally it just
 * holds onto a `PerViewUniform` UBO handle, but doesn't keep any shadow copy of it, instead it
 * writes the data directly into the CommandStream, for this reason partial update of the data
 * is not possible.
 */
class ShadowMapDescriptorSet {

public:
    class Transaction {
        friend ShadowMapDescriptorSet;
        PerViewUib* uniforms = nullptr;
        Transaction() = default; // disallow creation by the caller
    };

    explicit ShadowMapDescriptorSet(FEngine& engine) noexcept;

    void terminate(backend::DriverApi& driver);

    static void prepareCamera(Transaction const& transaction,
            backend::DriverApi& driver, const CameraInfo& camera) noexcept;

    static void prepareLodBias(Transaction const& transaction,
            float bias) noexcept;

    static void prepareViewport(Transaction const& transaction,
            backend::Viewport const& viewport) noexcept;

    static void prepareTime(Transaction const& transaction,
            FEngine const& engine, math::float4 const& userTime) noexcept;

    static void prepareShadowMapping(Transaction const& transaction,
            bool highPrecision) noexcept;

    static Transaction open(backend::DriverApi& driver) noexcept;

    // update local data into GPU UBO
    void commit(Transaction& transaction, FEngine& engine, backend::DriverApi& driver) noexcept;

    // bind this UBO
    void bind(backend::DriverApi& driver) noexcept;

private:
    static PerViewUib& edit(Transaction const& transaction) noexcept;
    backend::Handle<backend::HwBufferObject> mUniformBufferHandle;
    DescriptorSet mDescriptorSet;
};

} // namespace filament

#endif //TNT_FILAMENT_SHADOWMAPDESCRIPTORSET_H
