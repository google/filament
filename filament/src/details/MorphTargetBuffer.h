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

#ifndef TNT_FILAMENT_DETAILS_MORPHTARGETBUFFER_H
#define TNT_FILAMENT_DETAILS_MORPHTARGETBUFFER_H

#include "upcast.h"

#include <filament/MorphTargetBuffer.h>

#include "private/backend/DriverApiForward.h"

#include "private/backend/SamplerGroup.h"

#include <backend/Handle.h>

#include <utils/Allocator.h>

#include <vector>

namespace filament {

class FEngine;

class FMorphTargetBuffer : public MorphTargetBuffer {
public:
    FMorphTargetBuffer(FEngine& engine, const Builder& builder);

    // frees driver resources, object becomes invalid
    void terminate(FEngine& engine);

    void setPositionsAt(FEngine& engine, size_t targetIndex, math::float3 const* positions, size_t count);
    void setPositionsAt(FEngine& engine, size_t targetIndex, math::float4 const* positions, size_t count);
    void setTangentsAt(FEngine& engine, size_t targetIndex, math::short4 const* tangents, size_t count);
    inline size_t getVertexCount() const noexcept { return mVertexCount; }
    inline size_t getCount() const noexcept { return mCount; }

private:
    friend class FView;
    friend class RenderPass;

    void commit(FEngine& engine) const noexcept;
    void bind(backend::DriverApi& driver) const noexcept;

    backend::Handle<backend::HwSamplerGroup> mSbHandle;
    backend::Handle<backend::HwTexture> mTbHandle;
    backend::SamplerGroup mSBuffer;
    size_t mVertexCount;
    size_t mCount;
};

FILAMENT_UPCAST(MorphTargetBuffer)

} // namespace filament

#endif //TNT_FILAMENT_DETAILS_MORPHTARGETBUFFER_H
