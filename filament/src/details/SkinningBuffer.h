/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_SKINNINGBUFFER_H
#define TNT_FILAMENT_DETAILS_SKINNINGBUFFER_H

#include <filament/SkinningBuffer.h>

#include "downcast.h"

#include <private/filament/EngineEnums.h>
#include <private/filament/UibStructs.h>

#include <backend/DriverApiForward.h>
#include <backend/Handle.h>

#include <utils/FixedCapacityVector.h>

#include <math/mat4.h>
#include <math/vec2.h>

#include <stddef.h>
#include <stdint.h>

// for gtest
class FilamentTest_Bones_Test;

namespace filament {

class FEngine;
class FRenderableManager;

class FSkinningBuffer : public SkinningBuffer {
public:
    FSkinningBuffer(FEngine& engine, const Builder& builder);

    // frees driver resources, object becomes invalid
    void terminate(FEngine& engine);

    void setBones(FEngine& engine, RenderableManager::Bone const* transforms, size_t count, size_t offset);
    void setBones(FEngine& engine, math::mat4f const* transforms, size_t count, size_t offset);
    size_t getBoneCount() const noexcept { return mBoneCount; }

    // round count to the size of the UBO in the shader
    static size_t getPhysicalBoneCount(size_t count) noexcept {
        static_assert((CONFIG_MAX_BONE_COUNT & (CONFIG_MAX_BONE_COUNT - 1)) == 0);
        return (count + CONFIG_MAX_BONE_COUNT - 1) & ~(CONFIG_MAX_BONE_COUNT - 1);
    }

private:
    friend class ::FilamentTest_Bones_Test;
    friend class SkinningBuffer;
    friend class FRenderableManager;

    static void setBones(FEngine& engine, backend::Handle<backend::HwBufferObject> handle,
            RenderableManager::Bone const* transforms, size_t boneCount, size_t offset) noexcept;

    static void setBones(FEngine& engine, backend::Handle<backend::HwBufferObject> handle,
            math::mat4f const* transforms, size_t boneCount, size_t offset) noexcept;

    static PerRenderableBoneUib::BoneData makeBone(math::mat4f transform) noexcept;

    backend::Handle<backend::HwBufferObject> getHwHandle() const noexcept {
        return mHandle;
    }

    static backend::TextureHandle createIndicesAndWeightsHandle(FEngine& engine, size_t count);

    static void setIndicesAndWeightsData(FEngine& engine,
          backend::Handle<backend::HwTexture> textureHandle,
          const utils::FixedCapacityVector<math::float2>& pairs,
          size_t count);

    backend::Handle<backend::HwBufferObject> mHandle;
    uint32_t mBoneCount;
};

FILAMENT_DOWNCAST(SkinningBuffer)

} // namespace filament

#endif //TNT_FILAMENT_DETAILS_SKINNINGBUFFER_H
